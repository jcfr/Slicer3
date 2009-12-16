/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreIOManager.h"
#include "vtkMRMLScene.h"

#include <stdlib.h>
#include <iostream>
#include <QString>

int qSlicerCoreIOManagerTest1(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Missing arguments" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCoreIOManager manager;

  manager.printAdditionalInfo();

  vtkMRMLScene * scene1 = vtkMRMLScene::New();

  QString filename = argv[1];

  manager.loadScene( scene1, filename );

  vtkMRMLScene * scene2 = vtkMRMLScene::New();

  manager.loadScene( scene2, filename );

  manager.closeScene( scene1 );

  QString extension = "mrml";

  QString fileType = manager.fileTypeFromExtension( extension );

  std::cout << "File Type from extension " << qPrintable(extension);
  std::cout << " is " << qPrintable(fileType) << std::endl;

  return EXIT_SUCCESS;
}