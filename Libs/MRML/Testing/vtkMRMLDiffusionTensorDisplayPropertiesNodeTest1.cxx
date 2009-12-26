/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkAbstractTransform.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

class vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1 : public vtkMRMLDiffusionTensorDisplayPropertiesNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1 *New(){return new vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1;};
  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* transform) { return; }

  virtual int ReadData(vtkMRMLNode *refNode) { return 0; }
  virtual int WriteData(vtkMRMLNode *refNode) { return 0; }
};
 
int vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1(int argc, char * argv [] )
{
  vtkSmartPointer< vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  return EXIT_SUCCESS;
}