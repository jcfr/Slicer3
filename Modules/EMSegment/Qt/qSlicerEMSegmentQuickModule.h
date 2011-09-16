/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEMSegmentQuickModule_h
#define __qSlicerEMSegmentQuickModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerEMSegmentQuickModuleExport.h"

class qSlicerEMSegmentQuickModulePrivate;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_QTMODULES_EMSEGMENTQUICK_EXPORT qSlicerEMSegmentQuickModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerEMSegmentQuickModule(QObject *parent=0);
  virtual ~qSlicerEMSegmentQuickModule();

  qSlicerGetTitleMacro("EMSegment Easy");

  /// Return help text
  virtual QString helpText()const;

  /// Return acknowledgement text
  virtual QString acknowledgementText()const;

  /// Return the icon of the EMSegment module.
  virtual QIcon icon()const;

  virtual QString category()const;

  virtual QString contributor()const;

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerEMSegmentQuickModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentQuickModule);
  Q_DISABLE_COPY(qSlicerEMSegmentQuickModule);
};

#endif
