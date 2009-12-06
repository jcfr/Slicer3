#ifndef __qSlicerCamerasModuleWidget_h
#define __qSlicerCamerasModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesWin32Header.h"

class vtkMRMLNode;
class vtkMRMLViewNode;
class qSlicerCamerasModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCamerasModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

public slots:
  // Description:
  // Inherited from qSlicerWidget. Reimplemented for refresh issues.
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  virtual void setup();
  void synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode);

protected slots:
  void onCurrentViewNodeChanged(vtkMRMLNode*);
  void onCurrentCameraNodeChanged(vtkMRMLNode*);
  void onCameraNodeAdded(vtkMRMLNode*);
  void onCameraNodeRemoved(vtkMRMLNode*);
  void synchronizeCameraWithView(void * currentView= 0);

private:
  QCTK_DECLARE_PRIVATE(qSlicerCamerasModuleWidget);
};

#endif