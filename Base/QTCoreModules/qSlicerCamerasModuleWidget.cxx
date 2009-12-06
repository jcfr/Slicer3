#include "qSlicerCamerasModuleWidget.h"
#include "ui_qSlicerCamerasModule.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"

#include <vector>

//-----------------------------------------------------------------------------
struct qSlicerCamerasModuleWidgetPrivate: public qCTKPrivate<qSlicerCamerasModuleWidget>,
                                    public Ui_qSlicerCamerasModule
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerCamerasModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::setup()
{
  //this->Superclass::setup();
  
  QCTK_D(qSlicerCamerasModuleWidget);
  d->setupUi(this);

  connect(d->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentViewNodeChanged(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentCameraNodeChanged(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onCameraNodeAdded(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
          this, SLOT(onCameraNodeRemoved(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
QAction* qSlicerCamerasModuleWidget::showModuleAction()
{
  return new QAction(QIcon(":/Icons/Cameras.png"), tr("Show Cameras module"), this);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCurrentViewNodeChanged(vtkMRMLNode* mrmlNode)
{
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(mrmlNode);
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::synchronizeCameraWithView(void *)
{
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(
    qctk_d()->ViewNodeSelector->currentNode());
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode)
{
  if (!currentViewNode)
    {
    return;
    }
  vtkMRMLCameraNode *found_camera_node = NULL;
  std::vector<vtkMRMLNode*> cameraNodes;
  int nnodes = this->mrmlScene()->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLCameraNode *cameraNode = vtkMRMLCameraNode::SafeDownCast(cameraNodes[n]);
    if (cameraNode &&
        cameraNode->GetActiveTag() &&
        !strcmp(cameraNode->GetActiveTag(), currentViewNode->GetID()))
      {
      found_camera_node = cameraNode;
      break;
      }
    }
  qctk_d()->CameraNodeSelector->setCurrentNode(found_camera_node);
}


//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCurrentCameraNodeChanged(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *currentCameraNode =
        vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!currentCameraNode)
    {// if the camera list is empty, there is no current camera
    return;
    }
  vtkMRMLViewNode *currentViewNode = vtkMRMLViewNode::SafeDownCast(
    qctk_d()->ViewNodeSelector->currentNode());
  if (currentViewNode)
    {
    currentCameraNode->SetActiveTag(currentViewNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCameraNodeAdded(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *cameraNode =
    vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!cameraNode)
    {
    Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkConnect(cameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
                    this, SLOT(synchronizeCameraWithView(void*)));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCameraNodeRemoved(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *cameraNode =
    vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!cameraNode)
    {
    Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkDisconnect(cameraNode,
    vtkMRMLCameraNode::ActiveTagModifiedEvent,
    this, SLOT(synchronizeCameraWithView(void*)));
}

//-----------------------------------------------------------------------------
void  qSlicerCamerasModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);

  // When the view and camera selectors populate their items, the view might populate
  // its items before the camera, and the synchronizeCameraWithView() might do nothing.
  // Let's resync here.
  this->synchronizeCameraWithView();
}