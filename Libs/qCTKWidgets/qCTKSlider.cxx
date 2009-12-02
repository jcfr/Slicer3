
#include "qCTKSlider.h"

// QT includes
#include <QDebug>

// STL includes
#include <math.h>

// Helper Macro
#define qCTKSlider_IS_DBL(X) (!(X == static_cast<double>(static_cast<int>(X))))

//-----------------------------------------------------------------------------
struct qCTKSliderPrivate: public qCTKPrivate<qCTKSlider>
{
  qCTKSliderPrivate()
    {
    this->SingleStep = 1;
    this->HandleDouble = false;
    this->RangeMinimum = 0;
    this->RangeMaximum = 100;
    this->PreviousPosition = 0;
    }
  double      SingleStep;
  bool        HandleDouble;
  double      RangeMinimum;
  double      RangeMaximum;
  double      PreviousPosition;
};

// --------------------------------------------------------------------------
qCTKSlider::qCTKSlider(QWidget* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qCTKSlider);

  this->connect(this, SIGNAL(valueChanged(int)), SLOT(onValueChanged(int)));
  this->connect(this, SIGNAL(sliderMoved(int)), SLOT(onSliderMoved(int)));
}

// --------------------------------------------------------------------------
void qCTKSlider::setMinimumRange(double min)
{
  this->setRange(min, this->maximumAsDbl());
}

// --------------------------------------------------------------------------
void qCTKSlider::setMaximumRange(double max)
{
  this->setRange(this->minimumAsDbl(), max);
}

// --------------------------------------------------------------------------
void qCTKSlider::setRange(double min, double max)
{
  QCTK_D(qCTKSlider);
  
  if (d->HandleDouble)
    {
    min = min / d->SingleStep;
    max = max / d->SingleStep;
    }
  this->Superclass::setRange(static_cast<int>(min), static_cast<int>(max));
}

// --------------------------------------------------------------------------
double qCTKSlider::minimumAsDbl()
{
  return this->Superclass::minimum() * qctk_d()->SingleStep;
}

// --------------------------------------------------------------------------
double qCTKSlider::maximumAsDbl()
{
  return this->Superclass::maximum() * qctk_d()->SingleStep;
}

// --------------------------------------------------------------------------
double qCTKSlider::sliderPositionAsDbl()
{
  return this->Superclass::sliderPosition() * qctk_d()->SingleStep;
}

// --------------------------------------------------------------------------
void qCTKSlider::setSliderPosition(double position)
{
  this->Superclass::setSliderPosition(this->fromDoubleToInteger(position));
}

// -------------------------------------------------------------------------
QCTK_GET_CXX(qCTKSlider, double, previousSliderPosition, PreviousPosition);

// --------------------------------------------------------------------------
double qCTKSlider::valueAsDbl()const
{
  return this->Superclass::value() * qctk_d()->SingleStep;
}

// --------------------------------------------------------------------------
void qCTKSlider::setValue(double value)
{
  this->Superclass::setValue(this->fromDoubleToInteger(value));
  emit this->valueChanged(value);
  qctk_d()->PreviousPosition = value;
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qCTKSlider, double, singleStepAsDbl, SingleStep);

// --------------------------------------------------------------------------
void qCTKSlider::setSingleStep(double step)
{
  QCTK_D(qCTKSlider);
  
  d->HandleDouble = qCTKSlider_IS_DBL(step);
  d->SingleStep = step;
}

// --------------------------------------------------------------------------
double qCTKSlider::tickIntervalAsDbl()const
{
  return this->Superclass::tickInterval() * qctk_d()->SingleStep;
}

// --------------------------------------------------------------------------
void qCTKSlider::setTickInterval(double ti)
{
  this->Superclass::setTickInterval(this->fromDoubleToInteger(ti));
}

// --------------------------------------------------------------------------
int qCTKSlider::fromDoubleToInteger(double value)
{
  QCTK_D(qCTKSlider);
  
  int res = static_cast<int>(value);
  if (d->HandleDouble)
    {
    double tmp = value / d->SingleStep;
    res = static_cast<int>( (tmp > 0) ? floor(tmp + 0.5) : ceil(tmp - 0.5) );

    }
  return res;
}

// --------------------------------------------------------------------------
void qCTKSlider::onValueChanged(int value)
{
  emit this->valueChanged(value * qctk_d()->SingleStep);
}

// --------------------------------------------------------------------------
void qCTKSlider::onSliderMoved(int position)
{
  emit this->sliderMoved(position * qctk_d()->SingleStep);
}

// --------------------------------------------------------------------------
void qCTKSlider::reset()
{
  QCTK_D(qCTKSlider);
  
  this->setValue(0.0);
  this->setSliderPosition(0.0);
  d->PreviousPosition = 0.0;
}

#undef qCTKSlider_IS_DBL
