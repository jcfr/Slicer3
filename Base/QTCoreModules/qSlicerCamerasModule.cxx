#include "qSlicerCamerasModule.h"

// SlicerQT includes
#include "qSlicerCamerasModuleWidget.h"

//-----------------------------------------------------------------------------
struct qSlicerCamerasModulePrivate: public qCTKPrivate<qSlicerCamerasModule>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerCamerasModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerCamerasModule::createWidgetRepresentation()
{
  return new qSlicerCamerasModuleWidget;
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::helpText()const
{
  QString help =
    "Create new views and cameras.<br>"
    "The view pulldown menu below can be used to create new views and select "
    "the active view. Switch the layout to \"Tabbed 3D Layout\" from the "
    "layout icon in the toolbar to access multiple views. The view selected in "
    "\"Tabbed 3D Layout\" becomes the active view and replaces the 3D view in "
    "all other layouts. The camera pulldown menu below can be used to set the "
    "active camera for the selected view. <br>"
    "WARNING: this is rather experimental at the moment (fiducials, IO/data, "
    "closing the scene are probably broken for new views).";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::acknowledgementText()const
{
  QString about =
    "To be updated %1";

  return about.arg(this->slicerWikiUrl());
}