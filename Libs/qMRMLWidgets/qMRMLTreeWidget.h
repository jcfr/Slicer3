#ifndef __qMRMLTreeWidget_h
#define __qMRMLTreeWidget_h

// Qt includes
#include <QTreeView>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLSortFilterProxyModel;
class qMRMLTreeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLTreeWidget : public QTreeView
{
  Q_OBJECT
  Q_PROPERTY (bool listenNodeModifiedEvent READ listenNodeModifiedEvent WRITE setListenNodeModifiedEvent)
public:
  qMRMLTreeWidget(QWidget *parent=0);
  virtual ~qMRMLTreeWidget();

  vtkMRMLScene* mrmlScene()const;

  /// \sa qMRMLSceneModel::setListenNodeModifiedEvent
  void setListenNodeModifiedEvent(bool listen);
  /// \sa qMRMLSceneModel::listenNodeModifiedEvent
  bool listenNodeModifiedEvent()const;

  ///
  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes
  qMRMLSortFilterProxyModel* sortFilterProxyModel()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

signals:
  void currentNodeChanged(vtkMRMLNode* node);

protected slots:
  void onActivated(const QModelIndex& index);
protected:
  virtual void updateGeometries();
private:
  CTK_DECLARE_PRIVATE(qMRMLTreeWidget);
};

#endif
