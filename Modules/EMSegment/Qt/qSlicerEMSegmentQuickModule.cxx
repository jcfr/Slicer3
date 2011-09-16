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

// Qt includes
#include <QFileInfo>
#include <QScopedPointer>
#include <QtPlugin>

// PythonQT includes
#include <PythonQt.h>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerScriptedLoadableModuleWidget.h>

// EMSegment Logic includes
#include <vtkEMSegmentLogic.h>

// EMSegment QTModule includes
#include "qSlicerEMSegmentQuickModule.h"

void PythonQt_init_org_slicer_module_qSlicerEMSegmentModuleWidgets(PyObject*);

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerEMSegmentQuickModule, qSlicerEMSegmentQuickModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegmentQuick
class qSlicerEMSegmentQuickModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentQuickModule::qSlicerEMSegmentQuickModule(QObject* _parent)
  :Superclass(_parent)
  , d_ptr(new qSlicerEMSegmentQuickModulePrivate)
{

}

//-----------------------------------------------------------------------------
qSlicerEMSegmentQuickModule::~qSlicerEMSegmentQuickModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentQuickModule::setup()
{
  this->Superclass::setup();

  PythonQt_init_org_slicer_module_qSlicerEMSegmentModuleWidgets(0);
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::helpText()const
{
  QString help =
      "<b>EMSegment Easy Module:</b>  This module provides EM segmentation without an atlas.<br>"
      "<br>"
      "It is possible to segment different structures by manual sampling.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::acknowledgementText()const
{
  return QLatin1String(
        "<img src=':/Icons/UPenn_logo.png'><br>"
        "<br>"
        "This module is currently maintained by Daniel Haehn"
        " and Kilian Pohl (SBIA,UPenn). The work is currently supported by an ARRA "
        "supplement to NAC and the Slicer Community (see also <a>http://www.slicer.org</a>). <br>"
        "<br>"
        "The work was reported in  <br>"
        "K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. "
        "IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007.");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEMSegmentQuickModule::createWidgetRepresentation()
{
  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  QString classNameToLoad = "qSlicerEMSegmentQuickModuleWidget";
  bool ret = widget->setPythonSource(
        QFileInfo(this->path()).path() + "/Python/" + classNameToLoad + ".py", classNameToLoad);
  if (!ret)
    {
    return 0;
    }
  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerEMSegmentQuickModule::createLogic()
{
  return vtkEMSegmentLogic::New();
}

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentQuickModule::icon() const
{
  return QIcon(":/Icons/EMSegmentQuick.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::category()const
{
  return QLatin1String("Segmentation");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::contributor()const
{
  return QLatin1String("Daniel Haehn");
}
