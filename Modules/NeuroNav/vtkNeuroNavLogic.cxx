/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: $
Date:      $Date: $
Version:   $Revision: $

==========================================================================*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkNeuroNavLogic.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerColorLogic.h"

#include "vtkCylinderSource.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"
#include "vtkMultiThreader.h"

#include "vnl/vnl_float_3.h"


vtkCxxRevisionMacro(vtkNeuroNavLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkNeuroNavLogic);

//---------------------------------------------------------------------------
vtkNeuroNavLogic::vtkNeuroNavLogic()
{

  this->SliceDriver[0] = vtkNeuroNavLogic::SLICE_DRIVER_USER;
  this->SliceDriver[1] = vtkNeuroNavLogic::SLICE_DRIVER_USER;
  this->SliceDriver[2] = vtkNeuroNavLogic::SLICE_DRIVER_USER;

  // If the following code doesn't work, slice nodes should be obtained from application GUI
  this->SliceNode[0] = NULL;
  this->SliceNode[1] = NULL;
  this->SliceNode[2] = NULL;

  this->NeedRealtimeImageUpdate0 = 0;
  this->NeedRealtimeImageUpdate1 = 0;
  this->NeedRealtimeImageUpdate2 = 0;
  this->ImagingControl = 0;

  this->EnableOblique = false;
  this->TransformNodeName = NULL;
  this->SliceNo1Last = 1;
  this->SliceNo2Last = 1;
  this->SliceNo3Last = 1;
  this->OriginalTrackerNode = NULL;
  this->UseRegistration = false;

  this->Pat2ImgReg = vtkIGTPat2ImgRegistration::New();
  this->UpdatedTrackerNode = NULL;
}


//---------------------------------------------------------------------------
vtkNeuroNavLogic::~vtkNeuroNavLogic()
{
  if (this->Pat2ImgReg)
    {
    this->Pat2ImgReg->Delete();
    this->Pat2ImgReg = NULL;
    }

  if (this->UpdatedTrackerNode)
    {
    this->UpdatedTrackerNode->Delete();
    this->UpdatedTrackerNode = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkNeuroNavLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkNeuroNavLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
int vtkNeuroNavLogic::EnableLocatorDriver(int sw)
{
  if (sw == 1)  // turn on
    {
    vtkMRMLModelNode* mnode = SetVisibilityOfLocatorModel("NeuroNavLocator", 1);
    if (!UpdatedTrackerNode || UpdatedTrackerNode == NULL)
      {
      mnode->Delete();
      return 0;
      }
    mnode->SetAndObserveTransformNodeID(UpdatedTrackerNode->GetID());
    mnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    mnode->Delete();
    }
  else  // turn off
    {
    vtkMRMLModelNode* mnode = SetVisibilityOfLocatorModel("NeuroNavLocator", 0);
    mnode->Delete();
    }
  this->GetMRMLScene()->Modified();
  return 1;
}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkNeuroNavLogic::SetVisibilityOfLocatorModel(const char* nodeName, int v)
{
  vtkMRMLModelNode*   locatorModel;
  vtkMRMLDisplayNode* locatorDisp;

  // Check if any node with the specified name exists
  vtkMRMLScene*  scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(nodeName);

  if (collection != NULL && collection->GetNumberOfItems() == 0)
    {
    // if a node doesn't exist
    locatorModel = AddLocatorModel(nodeName, 0.0, 1.0, 1.0);
    }
  else
    {
    locatorModel = vtkMRMLModelNode::SafeDownCast(collection->GetItemAsObject(0));
    }

  if (locatorModel)
    {
    locatorDisp = locatorModel->GetDisplayNode();
    locatorDisp->SetVisibility(v);
    locatorModel->Modified();
    this->GetApplicationLogic()->GetMRMLScene()->Modified();
    }

  return locatorModel;
}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkNeuroNavLogic::AddLocatorModel(const char* nodeName, double r, double g, double b)
{

  vtkMRMLModelNode           *locatorModel;
  vtkMRMLModelDisplayNode    *locatorDisp;

  locatorModel = vtkMRMLModelNode::New();
  locatorDisp = vtkMRMLModelDisplayNode::New();

  GetMRMLScene()->SaveStateForUndo();
  GetMRMLScene()->AddNode(locatorDisp);
  GetMRMLScene()->AddNode(locatorModel);  

  locatorDisp->SetScene(this->GetMRMLScene());

  locatorModel->SetName(nodeName);
  locatorModel->SetScene(this->GetMRMLScene());
  locatorModel->SetAndObserveDisplayNodeID(locatorDisp->GetID());
  locatorModel->SetHideFromEditors(0);

  // Cylinder represents the locator stick
  vtkCylinderSource *cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.5);
  cylinder->SetHeight(100);
  cylinder->SetCenter(0, 0, 0);
  cylinder->Update();

  // Rotate cylinder
  vtkTransformPolyDataFilter *tfilter = vtkTransformPolyDataFilter::New();
  vtkTransform* trans =   vtkTransform::New();
  trans->RotateX(-90.0);
  trans->Translate(0.0, -50.0, 0.0);
  trans->Update();
  tfilter->SetInput(cylinder->GetOutput());
  tfilter->SetTransform(trans);
  tfilter->Update();

  // Sphere represents the locator tip 
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetRadius(3.0);
  sphere->SetCenter(0, 0, 0);
  sphere->Update();

  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(sphere->GetOutput());
  //apd->AddInput(cylinder->GetOutput());
  apd->AddInput(tfilter->GetOutput());
  apd->Update();

  locatorModel->SetAndObservePolyData(apd->GetOutput());

  double color[3];
  color[0] = r;
  color[1] = g;
  color[2] = b;
  locatorDisp->SetPolyData(locatorModel->GetPolyData());
  locatorDisp->SetColor(color);

  trans->Delete();
  tfilter->Delete();
  cylinder->Delete();
  sphere->Delete();
  apd->Delete();

  //locatorModel->Delete();
  locatorDisp->Delete();

  return locatorModel;
}



void vtkNeuroNavLogic::GetCurrentPosition(double *px, double *py, double *pz)
{
  *px = 0.0;
  *py = 0.0;
  *pz = 0.0;

  if (! this->OriginalTrackerNode)
    {
    return;
    }

  vtkMatrix4x4* transform;
  //transform = transformNode->GetMatrixTransformToParent();
  transform = this->OriginalTrackerNode->GetMatrixTransformToParent();

  if (transform)
    {
    // set volume orientation
    *px = transform->GetElement(0, 3);
    *py = transform->GetElement(1, 3);
    *pz = transform->GetElement(2, 3);
    }
}



void vtkNeuroNavLogic::UpdateTransformNodeByName(const char *name)
{
  if (name)
    {
    this->SetTransformNodeName(name);

    vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
    vtkCollection* collection = scene->GetNodesByName(this->TransformNodeName);

    if (collection != NULL && collection->GetNumberOfItems() == 0)
      {
      // the node name does not exist in the MRML tree
      return;
      }

    this->OriginalTrackerNode = vtkMRMLLinearTransformNode::SafeDownCast(collection->GetItemAsObject(0));
    if (this->Pat2ImgReg && this->UseRegistration)
      {
      this->UpdateLocatorTransform();
      }
    }
}


void vtkNeuroNavLogic::UpdateFiducialSeeding(const char *name, double offset)
{
  if (name)
    {
    // The following line causes memory leaking.
    // this->GetApplicationLogic()->GetMRMLScene()->SaveStateForUndo();

    vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
    vtkCollection* collection = scene->GetNodesByName(name);

    if (collection != NULL && collection->GetNumberOfItems() == 0)
      {
      vtkErrorMacro("NeuroNavLogic: The node name: (" << name << ") does not exist in the MRML tree");
      return;
      }

    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(collection->GetItemAsObject(0));
    if (flist == NULL)
      {
      vtkErrorMacro("NeuroNavLogic: The fiducial list node doesn't exist.");
      return;
      }

    double x, y, z;
    this->GetCurrentPosition(&x, &y, &z);
    if (offset != 0.0)
      {
      double len = sqrt(x*x + y*y + z*z);
      double r = (len + offset) / len;
      x = r*x;
      y = r*y;
      z = r*z;
      }

    flist->SetNthFiducialXYZ(0, x, y, z); 
    }
}


void vtkNeuroNavLogic::UpdateDisplay(int sliceNo1, int sliceNo2, int sliceNo3)
{
  if ((! this->OriginalTrackerNode) || (! this->UpdatedTrackerNode))
    {
    return;
    }

  vtkMatrix4x4* transform;
  vtkMatrix4x4* updated_transform;
  //transform = transformNode->GetMatrixTransformToParent();
  transform = this->OriginalTrackerNode->GetMatrixTransformToParent();
  updated_transform = this->UpdatedTrackerNode->GetMatrixTransformToParent();

  if (transform && updated_transform)
    {
    // set volume orientation
    float tx = transform->GetElement(0, 0);
    float ty = transform->GetElement(1, 0);
    float tz = transform->GetElement(2, 0);
    float nx = transform->GetElement(0, 2);
    float ny = transform->GetElement(1, 2);
    float nz = transform->GetElement(2, 2);
    float px = transform->GetElement(0, 3);
    float py = transform->GetElement(1, 3);
    float pz = transform->GetElement(2, 3);
    float ux = updated_transform->GetElement(0, 3);
    float uy = updated_transform->GetElement(1, 3);
    float uz = updated_transform->GetElement(2, 3);

    UpdateSliceNode(sliceNo1, sliceNo2, sliceNo3, 
                    nx, ny, nz, 
                    tx, ty, tz, 
                    px, py, pz,
                    ux, uy, uz);
    }
}


void vtkNeuroNavLogic::UpdateSliceNode(int sliceNo1, int sliceNo2, int sliceNo3,
                                       float nx, float ny, float nz,
                                       float tx, float ty, float tz,
                                       float px, float py, float pz,
                                       float ux, float uy, float uz)
{
  CheckSliceNodes();

  if (sliceNo1) // axial driven by User 
    {
    if (sliceNo1 != this->SliceNo1Last)
      {
      this->SliceNode[0]->SetOrientationToAxial();
      this->SliceNode[0]->JumpSlice(0, 0, 0);
      this->SliceNode[0]->UpdateMatrices();
      this->SliceNo1Last = sliceNo1;
      }
    }
  else
    {
    this->SliceNo1Last = sliceNo1;
    if (this->EnableOblique) // perpendicular
      {
      this->SliceNode[0]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
      this->SliceNode[0]->UpdateMatrices();
      }
    else
      {
      this->SliceNode[0]->SetOrientationToAxial();
      this->SliceNode[0]->JumpSlice(ux, uy, uz);
      this->SliceNode[0]->UpdateMatrices();
      }
    }

  if (sliceNo2) // sagittal driven by User 
    {
    if (sliceNo2 != this->SliceNo2Last)
      {
      this->SliceNode[1]->SetOrientationToSagittal();
      this->SliceNode[1]->JumpSlice(0, 0, 0);
      this->SliceNode[1]->UpdateMatrices();
      this->SliceNo2Last = sliceNo2;
      }
    }
  else
    {
    this->SliceNo2Last = sliceNo2;
    if (this->EnableOblique) // In-Plane
      {
      this->SliceNode[1]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
      this->SliceNode[1]->UpdateMatrices();
      }
    else
      {
      this->SliceNode[1]->SetOrientationToSagittal();
      this->SliceNode[1]->JumpSlice(ux, uy, uz);
      this->SliceNode[1]->UpdateMatrices();
      }
    }


  if (sliceNo3) // coronal driven by User 
    {
    if (sliceNo3 != this->SliceNo3Last)
      {
      this->SliceNode[2]->SetOrientationToCoronal();
      this->SliceNode[2]->JumpSlice(0, 0, 0);
      this->SliceNode[2]->UpdateMatrices();
      this->SliceNo3Last = sliceNo3;
      }
    }
  else
    {
    this->SliceNo3Last = sliceNo3;
    if (this->EnableOblique)  // In-Plane 90
      {
      this->SliceNode[2]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
      this->SliceNode[2]->UpdateMatrices();
      }
    else
      {
      this->SliceNode[2]->SetOrientationToCoronal();
      this->SliceNode[2]->JumpSlice(ux, uy, uz);
      this->SliceNode[2]->UpdateMatrices();
      }
    }
}



void vtkNeuroNavLogic::CheckSliceNodes()
{
  if (this->SliceNode[0] == NULL)
    {
    this->SliceNode[0] = this->GetApplicationLogic()
      ->GetSliceLogic("Red")->GetSliceNode();
    }
  if (this->SliceNode[1] == NULL)
    {
    this->SliceNode[1] = this->GetApplicationLogic()
      ->GetSliceLogic("Yellow")->GetSliceNode();
    }
  if (this->SliceNode[2] == NULL)
    {
    this->SliceNode[2] = this->GetApplicationLogic()
      ->GetSliceLogic("Green")->GetSliceNode();
    }
}



void vtkNeuroNavLogic::UpdateLocatorTransform()
{

  if (! this->OriginalTrackerNode)
    {
    return;
    }

  vtkMatrix4x4* original_tfm = vtkMatrix4x4::New();
  original_tfm->DeepCopy(this->OriginalTrackerNode->GetMatrixTransformToParent());
  vtkMatrix4x4 *updated_tfm = vtkMatrix4x4::New();

  // apply transform (updated_transform = registration_transform * original_transform)
  vtkMatrix4x4::Multiply4x4(this->Pat2ImgReg->GetLandmarkTransformMatrix(), original_tfm, updated_tfm);

  if (! this->UpdatedTrackerNode)
    {
    this->UpdatedTrackerNode = vtkMRMLLinearTransformNode::New();
    this->UpdatedTrackerNode->SetName("NeuroNavTracker");
    this->UpdatedTrackerNode->SetDescription("Tracker after patient to image registration.");
    GetMRMLScene()->AddNode(this->UpdatedTrackerNode);
    }

  vtkMatrix4x4 *matrix = this->UpdatedTrackerNode->GetMatrixTransformToParent();
  matrix->DeepCopy(updated_tfm);
  this->UpdatedTrackerNode->Modified();

  original_tfm->Delete();
  updated_tfm->Delete();
}

int vtkNeuroNavLogic::PerformPatientToImageRegistration()
{
  int error = this->GetPat2ImgReg()->DoRegistration();
  if (error)
    {
    this->SetUseRegistration(0);
    return error;
    }

  this->SetUseRegistration(1);
  return 0;
}


