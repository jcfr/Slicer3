#include "qMRMLLinearTransformSlider.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
struct qMRMLLinearTransformSliderPrivate: public qCTKPrivate<qMRMLLinearTransformSlider>
{
  qMRMLLinearTransformSliderPrivate()
    {
    this->TypeOfTransform = qMRMLLinearTransformSlider::TRANSLATION_LR;
    this->CoordinateReference = qMRMLLinearTransformSlider::GLOBAL;
    this->MRMLTransformNode = 0;
    this->PreviousPosition = 0;
    }
  qMRMLLinearTransformSlider::TransformType            TypeOfTransform;
  qMRMLLinearTransformSlider::CoordinateReferenceType  CoordinateReference;
  vtkMRMLLinearTransformNode*                          MRMLTransformNode;
  double                                               PreviousPosition;
};

// --------------------------------------------------------------------------
qMRMLLinearTransformSlider::qMRMLLinearTransformSlider(QWidget* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qMRMLLinearTransformSlider);
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::setTypeOfTransform(TransformType typeOfTransform)
{
  qctk_d()->TypeOfTransform = typeOfTransform;
}

// --------------------------------------------------------------------------
qMRMLLinearTransformSlider::TransformType qMRMLLinearTransformSlider::typeOfTransform() const
{
  return qctk_d()->TypeOfTransform;
}

// --------------------------------------------------------------------------
bool qMRMLLinearTransformSlider::isRotation()
{
  return (this->typeOfTransform() == ROTATION_LR ||
          this->typeOfTransform() == ROTATION_PA ||
          this->typeOfTransform() == ROTATION_IS);
}

// --------------------------------------------------------------------------
bool qMRMLLinearTransformSlider::isTranslation()
{
  return (this->typeOfTransform() == TRANSLATION_LR ||
          this->typeOfTransform() == TRANSLATION_PA ||
          this->typeOfTransform() == TRANSLATION_IS);
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::setCoordinateReference(CoordinateReferenceType coordinateReference)
{
  this->reset();
  qctk_d()->CoordinateReference = coordinateReference;
}

// --------------------------------------------------------------------------
qMRMLLinearTransformSlider::CoordinateReferenceType qMRMLLinearTransformSlider::coordinateReference() const
{
  return qctk_d()->CoordinateReference;
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  QCTK_D(qMRMLLinearTransformSlider);
  
  if (d->MRMLTransformNode == transformNode) { return; }

  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
    vtkMRMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onMRMLTransformNodeModified(void*,vtkObject*)));

  d->MRMLTransformNode = transformNode;
  this->onMRMLTransformNodeModified(0, transformNode);
  // If the node is NULL, any action on the widget is meaningless, this is why
  // the widget is disabled
  this->setEnabled(transformNode != 0);
}

// --------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qMRMLLinearTransformSlider::mrmlTransformNode()const
{
  return qctk_d()->MRMLTransformNode;
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::onMRMLTransformNodeModified(void* /*call_data*/, vtkObject* caller)
{
  QCTK_D(qMRMLLinearTransformSlider);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }
  Q_ASSERT(d->MRMLTransformNode == transformNode);

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(d->MRMLTransformNode,
    d->CoordinateReference == Self::GLOBAL, transform);

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  double value = 0.0;
  if (this->typeOfTransform() == TRANSLATION_LR)
    {
    value = matrix->GetElement(0,3);
    }
  else if (this->typeOfTransform() == TRANSLATION_PA)
    {
    value = matrix->GetElement(1,3);
    }
  else if (this->typeOfTransform() == TRANSLATION_IS)
    {
    value = matrix->GetElement(2,3);
    }

  if (this->isTranslation())
    {
    this->setValue(value);
    }
  else if (this->isRotation())
    {
    this->setValue(this->sliderPosition());
    }
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::applyTransformation(double sliderPosition)
{
  QCTK_D(qMRMLLinearTransformSlider);
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(d->MRMLTransformNode,
    d->CoordinateReference == Self::GLOBAL, transform);

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  if (this->typeOfTransform() == ROTATION_LR)
    {
    transform->RotateX(sliderPosition - this->previousSliderPosition());
    }
  else if (this->typeOfTransform() == ROTATION_PA)
    {
    transform->RotateY(sliderPosition - this->previousSliderPosition());
    }
  else if (this->typeOfTransform() == ROTATION_IS)
    {
    transform->RotateZ(sliderPosition - this->previousSliderPosition());
    }
  else if (this->typeOfTransform() == TRANSLATION_LR)
    {
    double position[3] = {0, 0, 0};
    position[0] = sliderPosition - matrix->GetElement(0,3);
    transform->Translate(position);
    }
  else if (this->typeOfTransform() == TRANSLATION_PA)
    {
    double position[3] = {0, 0, 0};
    position[1] = sliderPosition - matrix->GetElement(1,3);
    transform->Translate(position);
    }
  else if (this->typeOfTransform() == TRANSLATION_IS)
    {
    double position[3] = {0, 0, 0};
    position[2] = sliderPosition - matrix->GetElement(2,3);
    transform->Translate(position);
    }

  d->MRMLTransformNode->GetMatrixTransformToParent()->DeepCopy(
    transform->GetMatrix());
}
