#ifndef __qSlicerWidget_h
#define __qSlicerWidget_h 

#include <QWidget>

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLScene; 

class QScrollArea; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerWidget : public QWidget
{
  Q_OBJECT
  
public:
  
  typedef QWidget Superclass;
  qSlicerWidget(QWidget *parent=0);
  virtual ~qSlicerWidget();
  
  virtual void dumpObjectInfo(); 
  
  // Description:
  // Convenient windows to return parent widget or Null if any
  QWidget* parentWidget();
  
  // Description:
  // If possible, set the windowsFlags of the parent container.
  // Otherwise, set the ones of the current widget
  void setWindowFlags(Qt::WindowFlags type); 
  
  // Description:
  // Tell if the parent container is a QScrollArea
  bool isParentContainerScrollArea();
  
  // Description:
  // Convenient method to Set/Get the parent container as a QScrollArea
  // Note: Method mainly used while porting the application from KwWidget to Qt
  QScrollArea* getScrollAreaParentContainer();
  void setScrollAreaAsParentContainer(bool enable); 

  // Description:
  // If possible, set parent container geometry otherwise set widget geometry
  void setParentGeometry(int ax, int ay, int aw, int ah); 

public slots:
  
  // Description:
  // Overloaded method from QWidget
  virtual void setParentVisible(bool visible); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif