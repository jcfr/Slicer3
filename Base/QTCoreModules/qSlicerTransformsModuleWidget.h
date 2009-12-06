#ifndef __qSlicerTransformsModuleWidget_h
#define __qSlicerTransformsModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesWin32Header.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTransformsModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerTransformsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTransformsModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

public slots:
  void loadTransform();
  void loadTransform(const QString& fileName);

protected:
  virtual void setup();

protected slots:
  void onCoordinateReferenceButtonPressed(int id);
  void onIdentityButtonPressed();
  void onInvertButtonPressed();
  void onNodeSelected(vtkMRMLNode* node);

  // Description:
  // Triggered upon MRML transform node updates
  void onMRMLTransformNodeModified(void* call_data, vtkObject* caller);

protected:
  // Description:
  // Fill the 'minmax' array with the min/max translation value of the matrix.
  // Parameter expand allows to specify (using a value between 0 and 1)
  // which percentage of the found min/max value should be substracted/added
  // to the min/max value found.
  void extractMinMaxTranslationValue(vtkMatrix4x4 * mat, double& min, double& max);

  // Description:
  // Convenient method to return the coordinate system currently selected
  int coordinateReference();

private:
  QCTK_DECLARE_PRIVATE(qSlicerTransformsModuleWidget);
};

#endif