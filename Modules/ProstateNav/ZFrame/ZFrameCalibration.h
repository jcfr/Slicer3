/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   ZFrame Calibration
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLBase.h $
  Date:      $Date: 2009-01-05 13:28:20 -0500 (Mon, 05 Jan 2009) $
  Version:   $Revision: 8267 $

==========================================================================*/

#ifndef __ZFrameCalibration_h
#define __ZFrameCalibration_h

#include "ZLinAlg.h"
#include "newmatap.h"
#include "newmat.h"

namespace zf {

typedef float  Matrix4x4[4][4];

void PrintMatrix(Matrix4x4 &matrix);
void QuaternionToMatrix(float* q, Matrix4x4& m);
void MatrixToQuaternion(Matrix4x4& m, float* q);
void Cross(float *a, float *b, float *c);
void IdentityMatrix(Matrix4x4 &matrix);

class ZFrameCalibration
{

public:

  // Methods related to finding the fiducial artifacts in the MR images.
  ZFrameCalibration();
  ~ZFrameCalibration();


public:
  int ZFrameRegistration(Matrix4x4& imageTransform,
                         float ZquaternionBase[4], int range[2],
                         short* inputImage, int dimensions[3],
                         float Zposition[3], float Zorientation[4]);
  void Init(int xsize, int ysize);
  int  ZFrameRegistrationQuaternion(float position[3], float quaternion[4],
                                    float ZquaternionBase[4],
                                    Matrix& srcImage, int dimension[3], float spacing[3]);
  bool LocateFiducials(Matrix &image, int xsize, int ysize, 
                       int Zcoordinates[7][2], float tZcoordinates[7][2]);
  void FindSubPixelPeak(int Zcoordinate[2], float tZcoordinate[2],
                        Real Y0, Real Yx1, Real Yx2, Real Yy1, Real Yy2);
  bool CheckFiducialGeometry(int Zcoordinates[7][2], int xsize, int ysize);
  void FindFidCentre(float points[7][2], float &rmid, float &cmid);
  void FindFidCorners(float points[7][2], float *pmid);
  void OrderFidPoints(float points[7][2], float rmid, float cmid);
  
  // Methods related to solving for the frame pose w.r.t. the imaging plane.
  bool LocalizeFrame(float Zcoordinates[7][2], Column3Vector &Zposition,
                     Quaternion &Zorientation);
  void SolveZ(Column3Vector P1, Column3Vector P2, Column3Vector P3,
              Column3Vector Oz, Column3Vector Vz, Column3Vector &P2f);
  
  // Methods for finding matrix maxima.
  Real ComplexMax(Matrix &realmat, Matrix &imagmat);
  Real RealMax(Matrix &realmat);
  Real FindMax(Matrix &inmatrix, int &row, int &col);
  float CoordDistance(float *p1, float *p2);
  //ETX

protected:

  int xsize;
  int ysize;

  //BTX
  Matrix SourceImage, MaskImage;
  Matrix IFreal, IFimag, MFreal, MFimag, zeroimag;
  Matrix PFreal, PFimag;
  Matrix PIreal, PIimag;
  //ETX

};

}


#endif // __ZFrameCalibration_h


