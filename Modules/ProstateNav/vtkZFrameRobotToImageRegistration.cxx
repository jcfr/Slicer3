/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Modules/OpenIGTLinkIF/vtkRobotRegistration.cxx $
  Date:      $Date: 2009-01-05 13:28:20 -0500 (Mon, 05 Jan 2009) $
  Version:   $Revision: 8267 $

==========================================================================*/

/*
 * ZFrame Fiducial Registration Algorithm
 * The method and code is originally developoed by Simon DiMaio from BWH.
 */

#include "vtkObjectFactory.h"

#include "vtkZFrameRobotToImageRegistration.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include <string.h>

// Test
#include <fstream>

#include "ZFrameCalibration.h"

#define MEPSILON        (1e-10)

vtkStandardNewMacro(vtkZFrameRobotToImageRegistration);
vtkCxxRevisionMacro(vtkZFrameRobotToImageRegistration, "$Revision: 8267 $");


//#define DEBUG_ZFRAME_REGISTRATION 1

//---------------------------------------------------------------------------
vtkZFrameRobotToImageRegistration::vtkZFrameRobotToImageRegistration()
{
  this->SliceRangeLow = -1;
  this->SliceRangeHigh = -1;
  this->ZFrameBaseOrientation = vtkMatrix4x4::New();
  this->ZFrameBaseOrientation->Identity();
}


//---------------------------------------------------------------------------
vtkZFrameRobotToImageRegistration::~vtkZFrameRobotToImageRegistration()
{
  this->ZFrameBaseOrientation->Delete();
  this->ZFrameBaseOrientation = NULL;
}


//---------------------------------------------------------------------------
void vtkZFrameRobotToImageRegistration::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
void vtkZFrameRobotToImageRegistration::SetSliceRange(int param1, int param2)
{
  this->SliceRangeLow = param1;
  this->SliceRangeHigh = param2;
}


//---------------------------------------------------------------------------
int vtkZFrameRobotToImageRegistration::DoRegistration()
{
  
  if (this->FiducialVolume && this->RobotToImageTransform)
    {
    //ZFrameRegistration(this->FiducialVolume, this->RobotToImageTransform, 0);
    if (ZFrameRegistration(this->FiducialVolume, this->RobotToImageTransform, this->SliceRangeLow, this->SliceRangeHigh))
      {
      return 1;
      }
    else
      {
      return 0;
      }
    }
  else
    {
    return 0;
    }

}


//----------------------------------------------------------------------------
// Z-frame registration algorithm
//----------------------------------------------------------------------------

/*----------------------------------------------------------------------------*/
int vtkZFrameRobotToImageRegistration::ZFrameRegistration(vtkMRMLScalarVolumeNode* volumeNode,
                                                          vtkMRMLLinearTransformNode* transformNode, int slindex_s, int slindex_e)
{
  int           xsize;
  int           ysize;
  int           zsize;
  //Image         image_attrib;
  int           i,j;

  //simond debug frame delay
  // Get the image size attributes from the event.

  vtkImageData* image = volumeNode->GetImageData();
  int dimensions[3];
  image->GetDimensions(dimensions);
  xsize = dimensions[0];
  ysize = dimensions[1];
  zsize = dimensions[2];
  
  // Get image position and orientation
  vtkMatrix4x4* rtimgTransform = vtkMatrix4x4::New(); 
  zf::Matrix4x4 matrix;
  volumeNode->GetIJKToRASMatrix(rtimgTransform);

  float tx = rtimgTransform->GetElement(0, 0);
  float ty = rtimgTransform->GetElement(1, 0);
  float tz = rtimgTransform->GetElement(2, 0);
  float sx = rtimgTransform->GetElement(0, 1);
  float sy = rtimgTransform->GetElement(1, 1);
  float sz = rtimgTransform->GetElement(2, 1);
  float nx = rtimgTransform->GetElement(0, 2);
  float ny = rtimgTransform->GetElement(1, 2);
  float nz = rtimgTransform->GetElement(2, 2);
  float px = rtimgTransform->GetElement(0, 3);
  float py = rtimgTransform->GetElement(1, 3);
  float pz = rtimgTransform->GetElement(2, 3);

  // normalize
  float psi = sqrt(tx*tx + ty*ty + tz*tz);
  float psj = sqrt(sx*sx + sy*sy + sz*sz);
  float psk = sqrt(nx*nx + ny*ny + nz*nz);
  float ntx = tx / psi;
  float nty = ty / psi;
  float ntz = tz / psi;
  float nsx = sx / psj;
  float nsy = sy / psj;
  float nsz = sz / psj;
  float nnx = nx / psk;
  float nny = ny / psk;
  float nnz = nz / psk;

  // Here we calculate 'average' quaternion from registration results from
  // multiple slices. The average quaternion here is defined as the eigenvector
  // corresponding to the largest eigenvalue of the sample moment of inertia
  // matrix given as:
  //            ____
  //         1  \   |
  //    T = ---  |     qi qi'
  //         n  /___|
  //              i

  int n = 0;
  SymmetricMatrix T;
  T.ReSize(4);
  float P[3]={0,0,0};
  for (i = 0; i < 4; i ++)
    for (j = 0; j < 4; j ++)
      T.element(i, j) = 0.0;

  float position[3];
  float quaternion[4];

  // Default orientation of Z-frame
  float ZquaternionBase[4];
  zf::Matrix4x4 ZmatrixBase;
  
  ZmatrixBase[0][0] = (float) this->ZFrameBaseOrientation->GetElement(0, 0);
  ZmatrixBase[1][0] = (float) this->ZFrameBaseOrientation->GetElement(1, 0);
  ZmatrixBase[2][0] = (float) this->ZFrameBaseOrientation->GetElement(2, 0);
  ZmatrixBase[0][1] = (float) this->ZFrameBaseOrientation->GetElement(0, 1);
  ZmatrixBase[1][1] = (float) this->ZFrameBaseOrientation->GetElement(1, 1);
  ZmatrixBase[2][1] = (float) this->ZFrameBaseOrientation->GetElement(2, 1);
  ZmatrixBase[0][2] = (float) this->ZFrameBaseOrientation->GetElement(0, 2);
  ZmatrixBase[1][2] = (float) this->ZFrameBaseOrientation->GetElement(1, 2);
  ZmatrixBase[2][2] = (float) this->ZFrameBaseOrientation->GetElement(2, 2);
  ZmatrixBase[0][3] = (float) this->ZFrameBaseOrientation->GetElement(0, 3);
  ZmatrixBase[1][3] = (float) this->ZFrameBaseOrientation->GetElement(1, 3);
  ZmatrixBase[2][3] = (float) this->ZFrameBaseOrientation->GetElement(2, 3);

  zf::MatrixToQuaternion(ZmatrixBase, ZquaternionBase);


  zf::ZFrameCalibration * calibration;
  calibration = new zf::ZFrameCalibration(256, 256);

  for (int slindex = slindex_s; slindex < slindex_e; slindex ++)
    {
    // Shift the center
    // NOTE: The center of the image should be shifted due to different
    // definitions of image origin between VTK (Slicer) and OpenIGTLink;
    // OpenIGTLink image has its origin at the center, while VTK image
    // has one at the corner.

    float hfovi = psi * (dimensions[0]-1) / 2.0;
    float hfovj = psj * (dimensions[1]-1) / 2.0;
    //float hfovk = psk * (dimensions[2]-1) / 2.0;
    
    // For slice (k) direction, we calculate slice offset based on
    // the slice index.
    float offsetk = psk * slindex;
    
    float cx = ntx * hfovi + nsx * hfovj + nnx * offsetk;
    float cy = nty * hfovi + nsy * hfovj + nny * offsetk;
    float cz = ntz * hfovi + nsz * hfovj + nnz * offsetk;
    
    matrix[0][0] = ntx;
    matrix[1][0] = nty;
    matrix[2][0] = ntz;
    matrix[0][1] = nsx;
    matrix[1][1] = nsy;
    matrix[2][1] = nsz;
    matrix[0][2] = nnx;
    matrix[1][2] = nny;
    matrix[2][2] = nnz;
    matrix[0][3] = px + cx;
    matrix[1][3] = py + cy;
    matrix[2][3] = pz + cz;
    
    zf::MatrixToQuaternion(matrix, quaternion);
    position[0] = matrix[0][3];
    position[1] = matrix[1][3];
    position[2] = matrix[2][3];
    
#ifdef DEBUG_ZFRAME_REGISTRATION
    std::cerr << "=== Image position ===" << std::endl;
    std::cerr << "x = " << position[0] << std::endl; 
    std::cerr << "y = " << position[1] << std::endl;
    std::cerr << "z = " << position[2] << std::endl;
#endif // DEBUG_ZFRAME_REGISTRATION
    
    // Get a pointer to the image array.
    image->SetScalarTypeToShort();
    image->Modified();
    InputImage=(short*)image->GetScalarPointer();
    if (slindex >= 0 && slindex < zsize)
      {
      InputImage = &InputImage[xsize*ysize*slindex];
      }
    else
      {
      delete calibration;
      return 0;
      }
    
    // Transfer image to a Matrix.
    SourceImage.ReSize(xsize,ysize);

    for(i=0; i<xsize; i++)
      for(j=0; j<ysize; j++)
        SourceImage.element(i,j) = InputImage[j*xsize+i];
    
    // if Z-frame position is determined from the slice
    float spacing[3];
    spacing[0] = psi;
    spacing[1] = psj;
    spacing[2] = psk;

    if (calibration->ZFrameRegistrationQuaternion(position, quaternion,
                                                  ZquaternionBase,
                                                  SourceImage, dimensions, spacing))
      {
      P[0] += position[0];
      P[1] += position[1];
      P[2] += position[2];

#ifdef DEBUG_ZFRAME_REGISTRATION
      std::cerr << "position = ("
                << position[0] << ", "
                << position[1] << ", "
                << position[2] << ")" << std::endl;
#endif // DEBUG_ZFRAME_REGISTRATION
      
      // Note that T is defined as SymmetricMatrix class 
      // and upper triangular part is updated.
      T.element(0, 0) = T.element(0, 0) + quaternion[0]*quaternion[0];
      T.element(0, 1) = T.element(0, 1) + quaternion[0]*quaternion[1];
      T.element(0, 2) = T.element(0, 2) + quaternion[0]*quaternion[2];
      T.element(0, 3) = T.element(0, 3) + quaternion[0]*quaternion[3];
      T.element(1, 1) = T.element(1, 1) + quaternion[1]*quaternion[1];
      T.element(1, 2) = T.element(1, 2) + quaternion[1]*quaternion[2];
      T.element(1, 3) = T.element(1, 3) + quaternion[1]*quaternion[3];
      T.element(2, 2) = T.element(2, 2) + quaternion[2]*quaternion[2];
      T.element(2, 3) = T.element(2, 3) + quaternion[2]*quaternion[3];
      T.element(3, 3) = T.element(3, 3) + quaternion[3]*quaternion[3];
      n ++;

#ifdef DEBUG_ZFRAME_REGISTRATION
      std::cerr << "quaternion = ("
                << quaternion[0] << ", "
                << quaternion[1] << ", "
                << quaternion[2] << ", "
                << quaternion[3] << ")" << std::endl;
#endif // DEBUG_ZFRAME_REGISTRATION
      }
    }

  delete calibration;


  if (n <= 0)
    {
    return 0;
    }

  float fn = (float) n;
  for (i = 0; i < 3; i ++)
    {
    P[i] /= fn;
    }

  T.element(0, 0) = T.element(0, 0) / fn;
  T.element(0, 1) = T.element(0, 1) / fn;
  T.element(0, 2) = T.element(0, 2) / fn;
  T.element(0, 3) = T.element(0, 3) / fn;

  T.element(1, 1) = T.element(1, 1) / fn;
  T.element(1, 2) = T.element(1, 2) / fn;
  T.element(1, 3) = T.element(1, 3) / fn;

  T.element(2, 2) = T.element(2, 2) / fn;
  T.element(2, 3) = T.element(2, 3) / fn;
  T.element(3, 3) = T.element(3, 3) / fn;

  
  // calculate eigenvalues of T matrix
  DiagonalMatrix D;
  Matrix V;
  D.ReSize(4);
  V.ReSize(4, 4);
  eigenvalues(T, D, V);

#ifdef DEBUG_ZFRAME_REGISTRATION
  for (i = 0; i < 4; i ++)
    {
    std::cerr << "T[" << i << ", 0] = ("
              <<  T.element(i, 0) << ", "
              <<  T.element(i, 1) << ", "
              <<  T.element(i, 2) << ", "
              <<  T.element(i, 3) << ")" << std::endl;
    }

  std::cerr << "D[" << i << ", 0] = ("
            <<  D.element(0) << ", "
            <<  D.element(1) << ", "
            <<  D.element(2) << ", "
            <<  D.element(3) << ")" << std::endl;

  for (i = 0; i < 4; i ++)
    {
    std::cerr << "V[" << i << ", 0] = ("
              <<  V.element(i, 0) << ", "
              <<  V.element(i, 1) << ", "
              <<  V.element(i, 2) << ", "
              <<  V.element(i, 3) << ")" << std::endl;
    }
#endif // DEBUG_ZFRAME_REGISTRATION

  // find the maximum eigen value
  int maxi = 0;
  float maxv = D.element(0);
  for (i = 1; i < 4; i ++)
    {
    if (D.element(i) > maxv)
      {
      maxi = i;
      }
    }

  // Substitute 'average' position and quaternion.
  position[0] = P[0];
  position[1] = P[1];
  position[2] = P[2];
  quaternion[0] = V.element(0, maxi);
  quaternion[1] = V.element(1, maxi);
  quaternion[2] = V.element(2, maxi);
  quaternion[3] = V.element(3, maxi);

#ifdef DEBUG_ZFRAME_REGISTRATION
  std::cerr << "average position = ("
            << position[0] << ", "
            << position[1] << ", "
            << position[2] << ")" << std::endl;

  std::cerr << "average quaternion = ("
            << quaternion[0] << ", "
            << quaternion[1] << ", "
            << quaternion[2] << ", "
            << quaternion[3] << ")" << std::endl;
#endif // DEBUG_ZFRAME_REGISTRATION

  zf::QuaternionToMatrix(quaternion, matrix);
  matrix[0][3] = position[0];
  matrix[1][3] = position[1];
  matrix[2][3] = position[2];

#ifdef DEBUG_ZFRAME_REGISTRATION
  std::cerr << "Result matrix:" << std::endl;
  zf::PrintMatrix(matrix);
#endif // DEBUG_ZFRAME_REGISTRATION

  //// For experiment
  //std::ofstream fout;
  //fout.open("zframe_output.csv", std::ios::out | std::ios::app);
  //
  //fout << volumeNode->GetName() << ", "
  //     << n << ", "
  //     << matrix[0][0] << ", " << matrix[0][1] << ", " << matrix[0][2] << ", " << matrix[0][3] << ", "
  //     << matrix[1][0] << ", " << matrix[1][1] << ", " << matrix[1][2] << ", " << matrix[1][3] << ", "
  //     << matrix[2][0] << ", " << matrix[2][1] << ", " << matrix[2][2] << ", " << matrix[2][3]
  //     << std::endl;
  //fout.close();

  vtkMatrix4x4* zMatrix = vtkMatrix4x4::New();
  zMatrix->Identity();

  zMatrix->SetElement(0, 0, matrix[0][0]);
  zMatrix->SetElement(1, 0, matrix[1][0]);
  zMatrix->SetElement(2, 0, matrix[2][0]);
  zMatrix->SetElement(0, 1, matrix[0][1]);
  zMatrix->SetElement(1, 1, matrix[1][1]);
  zMatrix->SetElement(2, 1, matrix[2][1]);
  zMatrix->SetElement(0, 2, matrix[0][2]);
  zMatrix->SetElement(1, 2, matrix[1][2]);
  zMatrix->SetElement(2, 2, matrix[2][2]);
  zMatrix->SetElement(0, 3, matrix[0][3]);
  zMatrix->SetElement(1, 3, matrix[1][3]);
  zMatrix->SetElement(2, 3, matrix[2][3]);

  if (transformNode != NULL)
    {
    vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();
    transformToParent->DeepCopy(zMatrix);
    zMatrix->Delete();
    transformNode->Modified();
    return 1;
    }
  else
    {
    zMatrix->Delete();
    return 0;
    }
}


