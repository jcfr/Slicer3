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
#include "qSlicerEMSegmentModule.h"

void PythonQt_init_org_slicer_module_qSlicerEMSegmentModuleWidgets(PyObject*);

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerEMSegmentModule, qSlicerEMSegmentModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentModule::qSlicerEMSegmentModule(QObject* _parent)
  :Superclass(_parent)
  , d_ptr(new qSlicerEMSegmentModulePrivate)
{

}

//-----------------------------------------------------------------------------
qSlicerEMSegmentModule::~qSlicerEMSegmentModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModule::setup()
{
  this->Superclass::setup();

  PythonQt_init_org_slicer_module_qSlicerEMSegmentModuleWidgets(0);
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentModule::helpText()const
{
  QString help =
      "<b>EMSegment Module:</b>  Segment a set of set of images (target images) using"
      " the tree-based EM segmentation algorithm<br>"
      "<br>"
      "Use the pull down menu to select from a collection of tasks or create a new one.<br>"
      "Use the 'Back' and 'Next' to navigate through the stages of filling in the algorithm "
      "parameters.\n\n"
      "When all parameters are specified, use the 'segmentation' button. \n\n"
      "For latest updates, new tasks, and detail help please visit "
      "<a>%1/Modules:EMSegmenter-3.6</a> <br>"
      "<br>"
      " <b>The work was reported in:</b> <br>"
      "K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. "
      "IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007.";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentModule::acknowledgementText()const
{
  return QLatin1String(
        "<img src=':/Icons/UPenn_logo.png'><br>"
        "<br>"
        "This module is currently maintained by Daniel Haehn, Dominique Belhachemi,"
        " and Kilian Pohl (SBIA,UPenn). The work is currently supported by an ARRA "
        "supplement to NAC and the Slicer Community (see also <a>http://www.slicer.org</a>). <br>"
        "<br>"
        "The work was reported in  <br>"
        "K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. "
        "IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007.");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEMSegmentModule::createWidgetRepresentation()
{
  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  QString classNameToLoad = "qSlicerEMSegmentModuleWidget";
  bool ret = widget->setPythonSource(
        QFileInfo(this->path()).path() + "/Python/" + classNameToLoad + ".py", classNameToLoad);
  if (!ret)
    {
    return 0;
    }
  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerEMSegmentModule::createLogic()
{
  return vtkEMSegmentLogic::New();
}

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentModule::icon() const
{
  return QIcon(":/Icons/EMSegment.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentModule::category()const
{
  return QLatin1String("Segmentation");
}
