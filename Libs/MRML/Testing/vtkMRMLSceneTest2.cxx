/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScene.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLSceneTest2(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer< vtkMRMLScene > scene = vtkSmartPointer< vtkMRMLScene >::New();

  EXERCISE_BASIC_OBJECT_METHODS( scene );

  scene->SetURL( argv[1] );
  scene->Connect();

  return EXIT_SUCCESS;
}