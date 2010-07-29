/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QAction>
#include <QIcon>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::qSlicerAbstractModuleWidget(QWidget* parentWidget)
  :qSlicerWidget(parentWidget)
{
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setup()
{
  const qSlicerAbstractModule* m =
    qobject_cast<const qSlicerAbstractModule*>(this->module());
  this->setWindowTitle(m->title());
  this->setWindowIcon(m->icon());
}
