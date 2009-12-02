
#include "qCTKAddRemoveComboBox.h"

#include "ui_qCTKAddRemoveComboBox.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
struct qCTKAddRemoveComboBoxPrivate : public qCTKPrivate<qCTKAddRemoveComboBox>,
                                      public Ui_qCTKAddRemoveComboBox
{
  qCTKAddRemoveComboBoxPrivate()
    {
    this->HasEmptyItem = false; 
    this->AddingEmptyItem = false;
    this->RemovingEmptyItem = false;
    this->EmptyText = "None";

    this->AddEnabled = true;
    this->RemoveEnabled = true;
    this->EditEnabled = true;
    }

  bool    HasEmptyItem; 
  bool    AddingEmptyItem; 
  bool    RemovingEmptyItem; 
  QString EmptyText;
  
  bool    AddEnabled;
  bool    RemoveEnabled;
  bool    EditEnabled; 
};

// --------------------------------------------------------------------------
qCTKAddRemoveComboBox::qCTKAddRemoveComboBox(QWidget* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qCTKAddRemoveComboBox);
  QCTK_D(qCTKAddRemoveComboBox);
  d->setupUi(this);
  
  // connect 
  this->connectComboBox(d->ComboBox);
    
  this->connect(d->AddPushButton, SIGNAL(pressed()), SLOT(onAdd()));
  this->connect(d->RemovePushButton, SIGNAL(pressed()), SLOT(onRemove()));
  this->connect(d->EditPushButton, SIGNAL(pressed()), SLOT(onEdit()));

  // Add default 'empty item'
  d->AddingEmptyItem = true;
  d->ComboBox->addItem(d->EmptyText);
  d->AddingEmptyItem = false;
  d->HasEmptyItem = true;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::connectComboBox(QComboBox* comboBox)
{
  this->connect(comboBox, 
                SIGNAL(activated(int)), 
                SIGNAL(activated(int)));
  this->connect(comboBox, 
                SIGNAL(currentIndexChanged(int)), 
                SIGNAL(currentIndexChanged(int)));
  /*
  this->connect(qctk_d()->ComboBox->model(),
  SIGNAL(rowsAboutToBeInserted(const QModelIndex & parent, int start, int end )),
  SLOT(onRowsAboutToBeInserted(const QModelIndex & parent, int start, int end )));
  */
  this->connect(comboBox->model(), 
                SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(onRowsAboutToBeRemoved(const QModelIndex & , int , int  )));  
  
  this->connect(comboBox->model(), 
                SIGNAL(rowsInserted(const QModelIndex &, int, int )),
                SLOT(onRowsInserted(const QModelIndex &, int, int)));
  this->connect(comboBox->model(), 
                SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                SLOT(onRowsRemoved(const QModelIndex &, int, int )));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBox(QComboBox* comboBox)
{
  QCTK_D(qCTKAddRemoveComboBox);
  if ((comboBox == d->ComboBox) ||
      comboBox->count())
    {
    return;
    }
  
  QLayoutItem* oldComboBoxItem = this->layout()->takeAt(0);
  QComboBox* oldComboBox = qobject_cast<QComboBox*>(oldComboBoxItem->widget());
  comboBox->setSizePolicy(oldComboBox->sizePolicy());
  comboBox->setEnabled(this->comboBoxEnabled());

  delete oldComboBoxItem;

  dynamic_cast<QBoxLayout*>(this->layout())->insertWidget(0, comboBox);
  this->connectComboBox(comboBox);
  d->ComboBox = comboBox;
  delete oldComboBox;

  // Add default 'empty item'
  d->AddingEmptyItem = true;
  d->ComboBox->addItem(d->EmptyText);
  d->AddingEmptyItem = false;
  d->HasEmptyItem = true;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEmptyText(const QString& text)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    Q_ASSERT(d->ComboBox->count() == 1);
    this->setItemText(0, text);
    }
  d->EmptyText = text;
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qCTKAddRemoveComboBox, QString, emptyText, EmptyText);

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsInserted(const QModelIndex & parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }
    
  if (d->HasEmptyItem && !d->AddingEmptyItem)
    {
    // Remove the Empty item as some real items have been added
    d->HasEmptyItem = false;
    d->RemovingEmptyItem = true;
    d->ComboBox->removeItem(start == 0 ? end + 1 : 0);
    d->RemovingEmptyItem = false;
    
    if (d->RemoveEnabled)
      {
      d->RemovePushButton->setEnabled(true);
      }
    if (d->EditEnabled)
      {
      d->EditPushButton->setEnabled(true);
      }
    // as we removed the empty item, we need to shift the start/end items if needed
    if (start > 0 )
      {
      --start;
      --end;
      }
    }

  // don't emit signal if we are adding the Empty item
  if (!d->AddingEmptyItem)
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemAdded(i);
      }
    }
 }

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  // if the user try to remove the Empty item, don't send event
  if (d->RemovingEmptyItem)
    {
    return;
    }
  for (int i = start; i <= end; ++i)
    {
    emit this->itemAboutToBeRemoved(i);
    }
}


// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsRemoved(const QModelIndex & parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  if (d->ComboBox->count() == 0)
    {
    d->HasEmptyItem = true;
    d->AddingEmptyItem = true;
    d->ComboBox->addItem(d->EmptyText);
    d->AddingEmptyItem = false;
    if (d->RemoveEnabled)
      {
      d->RemovePushButton->setEnabled(false);
      }
    if (d->EditEnabled)
      {
      d->EditPushButton->setEnabled(false);
      }
    }  

  if (!d->RemovingEmptyItem)
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemRemoved(i);
      }
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBoxEnabled(bool enable)
{
  qctk_d()->ComboBox->setEnabled(enable);
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::comboBoxEnabled()const
{
  //const cast as I'm not sure why isEnabledTo doesn't take a const
  return qctk_d()->ComboBox->isEnabledTo(const_cast<qCTKAddRemoveComboBox*>(this));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setAddEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  d->AddPushButton->setEnabled(enable);
  d->AddEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::addEnabled()const
{
  return qctk_d()->AddEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRemoveEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (this->count() > 0)
    {
    d->RemovePushButton->setEnabled(enable);
    }
  d->RemoveEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::removeEnabled()const
{
  return qctk_d()->RemoveEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEditEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (this->count() > 0)
    { 
    d->EditPushButton->setEnabled(enable);
    }
  d->EditEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::editEnabled()const
{
  return qctk_d()->EditEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onAdd()
{

}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRemove()
{
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onEdit()
{
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::count()const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  return (d->HasEmptyItem ? 0 : d->ComboBox->count());
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qCTKAddRemoveComboBox, bool, empty, HasEmptyItem);

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setCurrentIndex(int index)
{
  return qctk_d()->ComboBox->setCurrentIndex(index);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QString &text, const QVariant &userData)
{
  //qDebug() << __FUNCTION__ << " " << index <<  " " << text << " " << userData ;
  qctk_d()->ComboBox->insertItem(index, text, userData);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData)
{
  qctk_d()->ComboBox->insertItem(index, icon, text, userData);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItems(int index, const QStringList &texts)
{
  qctk_d()->ComboBox->insertItems(index, texts);
}
  
// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findText(const QString & text, Qt::MatchFlags flags)const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    { // if the scene is empty, don't even try to find the text (it could be the
      // one of the EmptyText prop.
    return -1;
    }
  return d->ComboBox->findText(text, flags);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags)const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    { // if the scene is empty, don't even try to find the data 
    return -1;
    }
  return d->ComboBox->findData(data, role, flags);
}

// --------------------------------------------------------------------------
QString qCTKAddRemoveComboBox::itemText(int index) const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return QString();
    }
  return d->ComboBox->itemText(index);
}
  
// --------------------------------------------------------------------------
QVariant qCTKAddRemoveComboBox::itemData(int index, int role) const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return QVariant();
    }
  return d->ComboBox->itemData(index,role);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemText(int index, const QString& text)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  return d->ComboBox->setItemText(index, text);
}
  
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemData(int index, const QVariant& data, int role)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  d->ComboBox->setItemData(index, data, role);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::currentIndex() const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  return d->HasEmptyItem ? -1 : d->ComboBox->currentIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(int index)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return; 
    }
  d->ComboBox->removeItem(index);
}

// --------------------------------------------------------------------------
QModelIndex qCTKAddRemoveComboBox::rootModelIndex()const
{
  return qctk_d()->ComboBox->rootModelIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRootModelIndex(const QModelIndex& root)
{
  qctk_d()->ComboBox->setRootModelIndex(root);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::modelColumn()const
{
  return qctk_d()->ComboBox->modelColumn();
}

// --------------------------------------------------------------------------
QAbstractItemModel* qCTKAddRemoveComboBox::model()const
{
  return qctk_d()->ComboBox->model();
}
