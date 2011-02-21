/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkAtlasCreatorCxxModuleLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkAtlasCreatorCxxModuleLogic.h"
#include "vtkAtlasCreatorCxxModule.h"
#include "vtkSlicerApplication.h"
#include "vtkTcl.h" // Needed for Tcl_Interp


vtkAtlasCreatorCxxModuleLogic* vtkAtlasCreatorCxxModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkAtlasCreatorCxxModuleLogic");
  if(ret)
    {
      return (vtkAtlasCreatorCxxModuleLogic*)ret;
    }



  // If the factory was unable to create the object, then create it here.
  return new vtkAtlasCreatorCxxModuleLogic;
}


//----------------------------------------------------------------------------
vtkAtlasCreatorCxxModuleLogic::vtkAtlasCreatorCxxModuleLogic()
{

  Tcl_Interp *interp = NULL;
  vtkSlicerApplication *slicerApp = vtkSlicerApplication::GetInstance ( );
  interp = slicerApp->GetMainInterp();
  if (!interp)
    {
      std::cout << "Error: InitializeTcl failed" << std::endl;
    
    } else {

      // do something here
      return;

    }

}

//----------------------------------------------------------------------------
vtkAtlasCreatorCxxModuleLogic::~vtkAtlasCreatorCxxModuleLogic()
{

}

//----------------------------------------------------------------------------
void vtkAtlasCreatorCxxModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

void vtkAtlasCreatorCxxModuleLogic::Apply()
{

}
