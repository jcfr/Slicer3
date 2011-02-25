/*=auto=========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkExampleLoadableGuiLessModuleLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkAtlasCreatorCxxModuleLogic_h
#define __vtkAtlasCreatorCxxModuleLogic_h

#include "vtkSlicerModuleLogic.h"

#include "vtkAtlasCreatorCxxModule.h"

class VTK_ATLASCREATORCXXMODULE_EXPORT vtkAtlasCreatorCxxModuleLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkAtlasCreatorCxxModuleLogic *New();
  vtkTypeMacro(vtkAtlasCreatorCxxModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  void RegisterNodes();

  // The method that creates and runs VTK or ITK pipeline
  void Apply();
  
protected:
  vtkAtlasCreatorCxxModuleLogic();
  virtual ~vtkAtlasCreatorCxxModuleLogic();
  vtkAtlasCreatorCxxModuleLogic(const vtkAtlasCreatorCxxModuleLogic&);
  void operator=(const vtkAtlasCreatorCxxModuleLogic&);

};

#endif

