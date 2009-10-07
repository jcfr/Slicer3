#ifndef __qMRMLNodeSelector_h
#define __qMRMLNodeSelector_h

#include "qCTKComboBoxAddRemove.h"
#include "qVTKObject.h"

#include <QString>

#include "qMRMLWidgetsWin32Header.h"

class vtkMRMLScene; 
class vtkMRMLNode; 

class QMRML_WIDGETS_EXPORT qMRMLNodeSelector : public qCTKComboBoxAddRemove
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString NodeType READ nodeType WRITE setNodeType)
  
public:
  // Superclass typedef
  typedef qCTKComboBoxAddRemove Superclass;
  
  // Constructors
  qMRMLNodeSelector(QWidget* parent = 0);
  virtual ~qMRMLNodeSelector();
  
  // Description:
  // Return the node currently selected
  vtkMRMLNode* getSelectedNode(); 
  
  // Description:
  // Set/Get node type 
  QString nodeType(); 
  void setNodeType(const QString& nodeType); 
  
  // Description:
  // Set/Get MRML scene
  vtkMRMLScene* getMRMLScene();
  void setMRMLScene(vtkMRMLScene* scene);

signals:
  // Description:
  void nodeSelected(vtkMRMLNode* node); 

protected slots:
  // Description:
  // Triggered upon MRML scene updates
  void onMRMLNodeAdded(vtkObject * node); 
  void onMRMLNodeRemoved(vtkObject * node); 
  void onMRMLNodeModified(void* call_data, vtkObject * caller);
  
  // Description:
  // Handle when NodeSelector 'add' button is pressed
  void onAddButtonPressed(); 
  
  // Description:
  // Handle when NodeSelector 'remove' button is pressed
  void onRemoveButtonPressed(const QString & selectedItemName);
  
  // Description:
  // Handle when NodeSelector item is selected
  void onSelectorItemSelected(const QString & itemName); 
  
  // Description:
  // Handle when NodeSelector 'edit' button is pressed
  void onSelectorItemEditRequested(const QString & itemName); 
  
protected:
  
  void setSelected(const QString& itemName);
  
private:
  class qInternal; 
  qInternal * Internal;

};

#endif
