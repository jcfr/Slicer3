#include "qCTKCollapsibleWidget.h"

#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

qCTKCollapsibleWidget::qCTKCollapsibleWidget(QWidget* parent)
  :QFrame(parent)
{
  this->Header = new QPushButton("CollapsibleWidget",this);
  this->Header->setCheckable(true);
  this->StackWidget = new QStackedWidget(this);
  this->StackWidget->addWidget(new QWidget);
  
  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(this->Header);
  layout->addWidget(this->StackWidget);
  this->setLayout(layout);
  
  connect(this->Header, SIGNAL(toggled(bool)),
          this, SLOT(collapse(bool)));

  this->CollapseChildren = true;
  this->CollapsedHeight = 0;

  this->MaxHeight = this->maximumHeight();

  // Customization
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  this->Header->setStyleSheet("text-align: left;");
  
  QIcon headerIcon;
  headerIcon.addFile(":/Icons/expand-up.png",QSize(), QIcon::Normal, QIcon::Off);
  headerIcon.addFile(":/Icons/expand-down.png",QSize(), QIcon::Normal, QIcon::On);
  this->Header->setIcon(headerIcon);
  
  // Slicer Custom
  this->CollapsedHeight = 10;
  this->setContentsFrameShape(QFrame::Box);
}

qCTKCollapsibleWidget::~qCTKCollapsibleWidget()
{
  
}


QWidget *qCTKCollapsibleWidget::widget()
{
  return this->StackWidget->widget(0);
}

void qCTKCollapsibleWidget::setWidget(QWidget *newWidget)
{
  this->StackWidget->removeWidget(this->StackWidget->widget(0));
  newWidget->setParent(this->StackWidget);
  this->StackWidget->addWidget(newWidget);
}

void qCTKCollapsibleWidget::setTitle(QString t)
{
  this->Header->setText(t);
}

QString qCTKCollapsibleWidget::title()const
{
  return this->Header->text();
}

void qCTKCollapsibleWidget::setCollapsed(bool c)
{
  this->Header->setChecked(c);
}

bool qCTKCollapsibleWidget::collapsed()const
{
  return this->Header->isChecked();
}

void qCTKCollapsibleWidget::setCollapseChildren(bool c)
{
  this->CollapseChildren = c;
}

bool qCTKCollapsibleWidget::collapseChildren()const
{
  return this->CollapseChildren;
}

void qCTKCollapsibleWidget::setCollapsedHeight(int h)
{
  this->CollapsedHeight = h;
}

int qCTKCollapsibleWidget::collapsedHeight()const
{
  return this->CollapsedHeight;
}

void qCTKCollapsibleWidget::collapse(bool c)
{
  if (c)
    {
    this->OldSize = this->size();
    }
  QSize newSize = this->OldSize;
  if (this->CollapseChildren)
    {
    this->widget()->setHidden(c);
    }
  else
    {
    this->StackWidget->setHidden(c);
    }
  if (c)
    {
    if (this->CollapseChildren)
      {
      int top, bottom;
      this->StackWidget->getContentsMargins(0, &top, 0, &bottom);
      int stackWidgetHeight = top + bottom + this->CollapsedHeight;
      this->StackWidget->setMaximumHeight(stackWidgetHeight);
      int top2, bottom2;
      this->getContentsMargins(0, &top2, 0, &bottom2);
      this->MaxHeight = this->maximumHeight();
      this->setMaximumHeight(top2 + this->Header->size().height() + 
                             this->layout()->spacing() + stackWidgetHeight + 
                             bottom2);
      }
    else
      {
      newSize.setHeight(this->sizeHint().height());
      this->resize(newSize);
      }
    }
  else
    {
    this->StackWidget->setMaximumHeight(QWIDGETSIZE_MAX);
    this->setMaximumHeight(this->MaxHeight);
    this->resize(newSize);
    }
}

QFrame::Shape qCTKCollapsibleWidget::contentsFrameShape() const
{
  return this->StackWidget->frameShape();
}

void qCTKCollapsibleWidget::setContentsFrameShape(QFrame::Shape s)
{
  this->StackWidget->setFrameShape(s);
}

QFrame::Shadow qCTKCollapsibleWidget::contentsFrameShadow() const
{
  return this->StackWidget->frameShadow();
}

void qCTKCollapsibleWidget::setContentsFrameShadow(QFrame::Shadow s)
{
  this->StackWidget->setFrameShadow(s);
}
  
int qCTKCollapsibleWidget:: contentsLineWidth() const
{
  return this->StackWidget->lineWidth();
}

void qCTKCollapsibleWidget::setContentsLineWidth(int w)
{
  this->StackWidget->setLineWidth(w);
}

int qCTKCollapsibleWidget::contentsMidLineWidth() const
{
  return this->StackWidget->midLineWidth();
}

void qCTKCollapsibleWidget::setContentsMidLineWidth(int w)
{
  this->StackWidget->setMidLineWidth(w);
}