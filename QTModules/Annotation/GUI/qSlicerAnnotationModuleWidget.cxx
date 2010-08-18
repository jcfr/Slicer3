#include "GUI/qSlicerAnnotationModuleWidget.h"
#include "ui_qSlicerAnnotationModule.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// CTK includes
#include "ctkCollapsibleButton.h"
// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QTextBrowser>
#include <QFile>
#include <QLineEdit>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QBuffer>
#include <QImageWriter>

#include "vtkObserverManager.h"

// GUI includes
#include "GUI/qSlicerAnnotationModulePushButton.h"
#include "GUI/qSlicerAnnotationModuleReportDialog.h"
#include "GUI/qSlicerAnnotationModuleAnnotationPropertyDialog.h"
#include "GUI/qSlicerAnnotationModuleImageUtil.h"
#include "GUI/qSlicerAnnotationModuleScreenShotDialog.h"

//-----------------------------------------------------------------------------
class qSlicerAnnotationModuleWidgetPrivate: public ctkPrivate<qSlicerAnnotationModuleWidget> ,
    public Ui_qSlicerAnnotationModule
{
public:
  typedef enum
  {
    VisibleColumn = 0,
    LockColumn,
    TypeColumn,
    ValueColumn,
    TextColumn
  } TableColumnType;

  qSlicerAnnotationModuleWidgetPrivate();
  ~qSlicerAnnotationModuleWidgetPrivate();
  void
  setupUi(qSlicerWidget* widget);
  void
  updateAnnotation(int index,
                   const QString& label,
                   const QString& value,
                   const QString& text);
  void
  moveSelectedRow(bool up);
  vtkSlicerAnnotationModuleLogic*
  logic() const;
  void
  updateSelection(int totalitems);
  std::vector<int>
  updateSingleSelection();
  void
  removeAnnotation(int index);
  void
  setInvisibleItemStrikeout(bool isVisible);
  void
  setInvisibleItemIcon(bool isVisible);
  void
  updateValueItem(int index,
                  const QString& value);
  void
  updateTextItem(int index,
                 const QString& value);
  void
  setLockUnLockIcon(bool isLocked);
  void
  SetItemEditable(int row,
                  int col,
                  bool isEditable);

  //void updateValueItem(int index, double value);


protected slots:

protected:
  QList<QTableWidgetItem*>
  takeRow(int row);
  QList<QTableWidgetItem*>
  readRow(int row);
  void
  setRow(int row,
         const QList<QTableWidgetItem*>& rowItems);

private:
  qSlicerAnnotationModulePushButton* visibilitywidget;
  qSlicerAnnotationModulePushButton* typewidget;
  qSlicerAnnotationModulePushButton* lockwidget;
  bool isSelectAll;
  bool isLockAll;

};

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic*
qSlicerAnnotationModuleWidgetPrivate::logic() const
{
  CTK_P(const qSlicerAnnotationModuleWidget);
  return vtkSlicerAnnotationModuleLogic::SafeDownCast(
      p->logic());
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidgetPrivate::qSlicerAnnotationModuleWidgetPrivate()
{
  isSelectAll = true;
  isLockAll = true;
  visibilitywidget = NULL;
  typewidget = NULL;
  lockwidget = NULL;
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidgetPrivate::~qSlicerAnnotationModuleWidgetPrivate()
{
  if (visibilitywidget != NULL)
    {
    delete visibilitywidget;
    }
  if (typewidget != NULL)
    {
    delete typewidget;
    }
  if (lockwidget != NULL)
    {
    delete lockwidget;
    }
}

//-----------------------------------------------------------------------------
// qSlicerAnnotationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerAnnotationModule::setupUi(
      widget);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::updateAnnotation(int index,
                                                            const QString& label,
                                                            const QString& value,
                                                            const QString& text)
{
  // warning, this function can be reentrant.
  CTK_P(qSlicerAnnotationModuleWidget);

  if (index >= tableWidget->rowCount())
    {
    tableWidget->setRowCount(
        index + 1);
    }

  if (tableWidget->item(
      index,
      VisibleColumn) == 0)
    {
    tableWidget->setItem(
        index,
        VisibleColumn,
        new QTableWidgetItem(
            QString(
                "")));
    }
  else
    {
    tableWidget->item(
        index,
        VisibleColumn)->setData(
        Qt::DisplayRole,
        QString(
            ""));
    }

  visibilitywidget = qobject_cast<qSlicerAnnotationModulePushButton*> (
      tableWidget->cellWidget(
          index,
          VisibleColumn));
  if (visibilitywidget == 0)
    {
    visibilitywidget = new qSlicerAnnotationModulePushButton(
        index,
        tableWidget);
    QSize size = visibilitywidget->size();
    size.setWidth(
        size.height());
    visibilitywidget->resize(
        size);
    tableWidget->setCellWidget(
        index,
        VisibleColumn,
        visibilitywidget);
    visibilitywidget->setIcon(
        QIcon(
            ":/Icons/AnnotationVisibility.png"));
    tableWidget->resizeColumnToContents(
        VisibleColumn);
    p->connect(
        this->visibilitywidget,
        SIGNAL(buttonClickedWithIndex(int)),
        p,
        SLOT(selectRowByIndex(int)));
    p->connect(
        this->visibilitywidget,
        SIGNAL(clicked()),
        p,
        SLOT(visibleSelectedButtonClicked()));
    visibilitywidget->setToolTip(
        "Click to Hide/Unhide this annotation.");

    }
  else
    {
    visibilitywidget->setButtonIndex(
        index);
    tableWidget->resizeColumnToContents(
        VisibleColumn);
    }

  if (tableWidget->item(
      index,
      TypeColumn) == 0)
    {
    tableWidget->setItem(
        index,
        TypeColumn,
        new QTableWidgetItem(
            QString(
                "")));
    }
  else
    {
    tableWidget->item(
        index,
        TypeColumn)->setData(
        Qt::DisplayRole,
        QString(
            ""));
    }

  typewidget = qobject_cast<qSlicerAnnotationModulePushButton*> (
      tableWidget->cellWidget(
          index,
          TypeColumn));
  if (typewidget == 0)
    {
    typewidget = new qSlicerAnnotationModulePushButton(
        index,
        tableWidget);
    QSize size = typewidget->size();
    size.setWidth(
        size.height());
    typewidget->resize(
        size);
    tableWidget->setCellWidget(
        index,
        TypeColumn,
        typewidget);

    typewidget->setIcon(
        QIcon(
            p->getAnnotationIconName(
                index,
                true)));
    tableWidget->resizeColumnToContents(
        TypeColumn);
    p->connect(
        this->typewidget,
        SIGNAL(buttonClickedWithIndex(int)),
        p,
        SLOT(selectRowByIndex(int)));
    p->connect(
        this->typewidget,
        SIGNAL(clicked()),
        p,
        SLOT(propertyEditButtonClicked()));
    typewidget->setToolTip(
        "Click to edit the properties of this annotation.");

    }
  else
    {
    typewidget->setIcon(
        QIcon(
            p->getAnnotationIconName(
                index,
                true)));
    typewidget->setButtonIndex(
        index);
    tableWidget->resizeColumnToContents(
        TypeColumn);
    }

  ///////////////////////////// Lock Column
  if (tableWidget->item(
      index,
      LockColumn) == 0)
    {
    tableWidget->setItem(
        index,
        LockColumn,
        new QTableWidgetItem(
            QString(
                "")));
    }
  else
    {
    tableWidget->item(
        index,
        LockColumn)->setData(
        Qt::DisplayRole,
        QString(
            ""));
    }

  lockwidget = qobject_cast<qSlicerAnnotationModulePushButton*> (
      tableWidget->cellWidget(
          index,
          LockColumn));
  if (lockwidget == 0)
    {
    lockwidget = new qSlicerAnnotationModulePushButton(
        index,
        tableWidget);
    QSize size = lockwidget->size();
    size.setWidth(
        size.height());
    lockwidget->resize(
        size);
    tableWidget->setCellWidget(
        index,
        LockColumn,
        lockwidget);
    lockwidget->setIcon(
        QIcon(
            ":/Icons/AnnotationUnlock.png"));
    tableWidget->resizeColumnToContents(
        LockColumn);
    p->connect(
        this->lockwidget,
        SIGNAL(buttonClickedWithIndex(int)),
        p,
        SLOT(selectRowByIndex(int)));
    p->connect(
        this->lockwidget,
        SIGNAL(clicked()),
        p,
        SLOT(lockSelectedButtonClicked()));
    lockwidget->setToolTip(
        "Click to lock/unlock the edition of the properties.");

    }
  else
    {
    lockwidget->setButtonIndex(
        index);
    tableWidget->resizeColumnToContents(
        LockColumn);
    }

  if (tableWidget->item(
      index,
      ValueColumn) == 0)
    {
    tableWidget->setItem(
        index,
        ValueColumn,
        new QTableWidgetItem(
            value));
    }
  else
    {
    tableWidget->item(
        index,
        ValueColumn)->setData(
        Qt::DisplayRole,
        value);
    }

  tableWidget->resizeColumnToContents(
      ValueColumn);

  tableWidget->item(
      index,
      ValueColumn)->setFlags(
      Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsTristate | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

  QString t(
      "");
  if (tableWidget->item(
      index,
      TextColumn) == 0)
    {
    tableWidget->setItem(
        index,
        TextColumn,
        new QTableWidgetItem(
            text));
    p->connect(
        tableWidget,
        SIGNAL(cellChanged(int, int)),
        p,
        SLOT(updateAnnotationText(int, int)));
    }
  else
    {
    tableWidget->item(
        index,
        TextColumn)->setData(
        Qt::DisplayRole,
        text);
    }
  int expandedwidth = tableWidget->size().width() - tableWidget->columnWidth(
      VisibleColumn) - tableWidget->columnWidth(
      TypeColumn) - tableWidget->columnWidth(
      ValueColumn) - tableWidget->columnWidth(
      LockColumn);
  tableWidget->setColumnWidth(
      TextColumn,
      expandedwidth);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::updateValueItem(int index,
                                                           const QString& value)
{
  tableWidget->item(
      index,
      ValueColumn)->setData(
      Qt::DisplayRole,
      value);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::updateTextItem(int index,
                                                          const QString& value)
{
  tableWidget->item(
      index,
      TextColumn)->setData(
      Qt::DisplayRole,
      value);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::SetItemEditable(int row,
                                                           int col,
                                                           bool isEditable)
{
  if (isEditable)
    {
    tableWidget->item(
        row,
        col)->setFlags(
        Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsTristate | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
    }
  else
    {
    tableWidget->item(
        row,
        col)->setFlags(
        Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsTristate | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::moveSelectedRow(bool up)
{
  if (this->tableWidget->selectedItems().count() < 1)
    {
    QMessageBox::warning(
        this->tableWidget,
        QString(
            "Move Selected Row"),
        QString(
            "No annotation is selected!"));

    std::cerr << "No annotation is selected" << std::endl;
    return;
    }

  int columns = this->tableWidget->columnCount();

  if (this->tableWidget->selectedItems().count() > columns)
    {
    std::cerr << "Please only select one annotation to be moved in the table" << std::endl;
    QMessageBox::warning(
        this->tableWidget,
        QString(
            "Move Selected Row"),
        QString(
            "Please only select one annotation to be moved in the table"));
    return;
    }

  const int sourceRow = this->tableWidget->row(
      this->tableWidget->selectedItems().at(
          0));
  const int destRow = (up ? sourceRow - 1 : sourceRow + 1);

  if (destRow < 0 || destRow >= this->tableWidget->rowCount())
    {
    std::cerr << "qSlicerAnnotationModuleWidgetPrivate::moveSelectedRow: Nothing to move as selected entry is already on top/buttom of table" << std::endl;
    return;
    }
  // change TypeList and IDList for angles and rulers
  CTK_P(qSlicerAnnotationModuleWidget);

  const char* tempid = p->m_IDs[sourceRow];
  p->m_IDs[sourceRow] = p->m_IDs[destRow];
  p->m_IDs[destRow] = tempid;

  // Read whole rows
  QList<QTableWidgetItem*> sourceItems = this->takeRow(
      sourceRow);
  QTableWidgetItem* sourceItemsID = sourceItems[1];
  QList<QTableWidgetItem*> destItems = this->takeRow(
      destRow);

  // I do that so that ID does not change
  sourceItems[1] = destItems[1];
  destItems[1] = sourceItemsID;

  // set back in reverse order
  this->setRow(
      sourceRow,
      destItems);
  this->setRow(
      destRow,
      sourceItems);

  // change icons
  qSlicerAnnotationModulePushButton* button1 = (qSlicerAnnotationModulePushButton*) this->tableWidget->cellWidget(
      sourceRow,
      TypeColumn);
  qSlicerAnnotationModulePushButton* button2 = (qSlicerAnnotationModulePushButton*) this->tableWidget->cellWidget(
      destRow,
      TypeColumn);
  QIcon icon1 = button1->icon();
  QIcon icon2 = button2->icon();
  button1->setIcon(
      icon2);
  button2->setIcon(
      icon1);

  // change the current selected row
  for (int i = 0; i < columns; ++i)
    {
    this->tableWidget->item(
        sourceRow,
        i)->setSelected(
        false);
    this->tableWidget->item(
        destRow,
        i)->setSelected(
        true);
    }

}

// takes and returns the whole row
//-----------------------------------------------------------------------------
QList<QTableWidgetItem*> qSlicerAnnotationModuleWidgetPrivate::takeRow(int row)
{
  QList<QTableWidgetItem*> rowItems;
  for (int col = 0; col < this->tableWidget->columnCount(); ++col)
    {
    rowItems << this->tableWidget->takeItem(
        row,
        col);
    }
  return rowItems;
}

//-----------------------------------------------------------------------------
QList<QTableWidgetItem*> qSlicerAnnotationModuleWidgetPrivate::readRow(int row)
{
  QList<QTableWidgetItem*> rowItems;
  for (int col = 0; col < this->tableWidget->columnCount(); ++col)
    {
    rowItems << this->tableWidget->item(
        row,
        col);
    }
  return rowItems;
}

// sets the whole row
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::setRow(int row,
                                                  const QList<QTableWidgetItem*>& rowItems)
{
  for (int col = 0; col < this->tableWidget->columnCount(); ++col)
    {
    this->tableWidget->setItem(
        row,
        col,
        rowItems.at(
            col));
    }
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidget::qSlicerAnnotationModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(
      parent)
{
  CTK_INIT_PRIVATE(qSlicerAnnotationModuleWidget);
  m_index = -1;
  m_lastAddedIndex = -1;
  m_ReportDialog = NULL;
  m_ScreenShotDialog = NULL;
  m_report = "";

  this->m_CurrentAnnotationType = 0;
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidget::~qSlicerAnnotationModuleWidget()
{
  if (m_ReportDialog != NULL)
    {
    delete m_ReportDialog;
    m_ReportDialog = NULL;
    }
  if (m_ScreenShotDialog != NULL)
    {
    delete m_ScreenShotDialog;
    m_ScreenShotDialog = NULL;
    }

  std::map<std::string, qSlicerAnnotationModuleAnnotationPropertyDialog*>::iterator iter;
  for (iter = this->m_PropertyDialogs.begin(); iter != this->m_PropertyDialogs.end(); ++iter)
    {
    if (iter->second != NULL)
      {
      delete iter->second;
      }
    }
  this->m_PropertyDialogs.clear();
  m_IDs.clear();
  m_screenshotList.clear();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::setup()
{
  CTK_D(qSlicerAnnotationModuleWidget);
  d->setupUi(
      this);
  this->connect(
      d->fiducialTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onFiducialNodeButtonClicked()));
  this->connect(
      d->stickyTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onStickyNodeButtonClicked()));
  this->connect(
      d->textTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onTextNodeButtonClicked()));
  this->connect(
      d->angleTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onAngleNodeButtonClicked()));
  this->connect(
      d->roiTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onROINodeButtonClicked()));
  this->connect(
      d->bidimensionalTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onBidimensionalNodeButtonClicked()));
  this->connect(
      d->splineTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onSplineNodeButtonClicked()));
  this->connect(
      d->rulerTypeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onRulerNodeButtonClicked()));

  this->connect(
      d->moveDownSelectedButton,
      SIGNAL(clicked()),
      SLOT(moveDownSelected()));
  this->connect(
      d->moveUpSelectedButton,
      SIGNAL(clicked()),
      SLOT(moveUpSelected()));

  // Save Panel
  this->connect(
      d->saveScene,
      SIGNAL(clicked()),
      SLOT(onSaveMRMLSceneButtonClicked()));
  this->connect(
      d->selectedAllButton,
      SIGNAL(clicked()),
      SLOT(selectedAllButtonClicked()));
  this->connect(
      d->visibleSelectedButton,
      SIGNAL(clicked()),
      SLOT(visibleSelectedButtonClicked()));
  this->connect(
      d->deleteSelectedButton,
      SIGNAL(clicked()),
      SLOT(deleteSelectedButtonClicked()));
  this->connect(
      d->generateReport,
      SIGNAL(clicked()),
      this,
      SLOT(onGenerateReportButtonClicked()));
  this->connect(
      d->saveAnnotation,
      SIGNAL(clicked()),
      this,
      SLOT(onSaveAnnotationButtonClicked()));
  this->connect(
      d->screenShot,
      SIGNAL(clicked()),
      this,
      SLOT(onScreenShotButtonClicked()));
  this->connect(
      d->lockUnlockAllButton,
      SIGNAL(clicked()),
      this,
      SLOT(onLockUnlockAllButtonClicked()));
  this->connect(
      d->pauseButton,
      SIGNAL(clicked()),
      this,
      SLOT(onPauseButtonClicked()));
  this->connect(
      d->resumeButton,
      SIGNAL(clicked()),
      this,
      SLOT(onResumeButtonClicked()));
  this->connect(
      d->cancelButton,
      SIGNAL(clicked()),
      this,
      SLOT(onCancelButtonClicked()));
  this->connect(
      d->doneButton,
      SIGNAL(clicked()),
      this,
      SLOT(onDoneButtonClicked()));
  this->connect(
      d->tableWidget,
      SIGNAL(itemSelectionChanged()),
      this,
      SLOT(onItemSelectionChanged()));

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::updateAnnotationText(int row,
                                                         int col)
{
  CTK_D(qSlicerAnnotationModuleWidget);
  if (col != d->TextColumn)
    {
    return;
    }

  QString textString;

  textString = d->logic()->GetAnnotationTextProperty(
      d->logic()->GetMRMLScene()->GetNodeByID(
          m_IDs[row]));
  if (textString.toLatin1().data() == d->tableWidget->item(
      row,
      col)->text())
    {
    return;
    }

  QString text = d->tableWidget->item(
      row,
      col)->text();
  d->logic()->ModifyPropertiesAndWidget(
      d->logic()->GetMRMLScene()->GetNodeByID(
          m_IDs[row]),
      d->logic()->TEXT,
      text.toLatin1().data());

  if (this->GetPropertyDialog(
      m_IDs[row]) != NULL && this->GetPropertyDialog(
      m_IDs[row])->isVisible())
    {
    this->GetPropertyDialog(
        m_IDs[row])->updateTextFromTable(
        text);
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onFiducialNodeAdded(vtkObject* object)
{

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onFiducialNodeModified(vtkObject* object,
                                                           void* call_data)
{

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::updateFiducialTable(int index)
{

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveDownSelected()
{
  CTK_D(qSlicerAnnotationModuleWidget);
  d->moveSelectedRow(
      false);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveUpSelected()
{
  CTK_D(qSlicerAnnotationModuleWidget);
  d->moveSelectedRow(
      true);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::StartAddingFiducials()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->logic()->StartAddingFiducials();

  const char *newFiducialNodeID = d->logic()->AddFiducial();
  if (!newFiducialNodeID)
    {
    std::cerr << "Could not add Fiducial" << std::endl;
    return;
    }

  m_IDs.push_back(
      newFiducialNodeID);
  m_index++;
  /*
   //qvtkConnect(d->logic()->GetFiducialManager(),  vtkMRMLAnnotationFiducialDisplayableManager::AddFiducialCompletedEvent,
   //  this, SLOT(AddFiducialCompleted(vtkObject*, void*)) );
   */
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::StopAddingFiducials()
{
  //CTK_D(qSlicerAnnotationModuleWidget);
  //d->logic()->StopAddingFiducials();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onAddFiducialsButtonToggled(bool toggle)
{
  if (toggle)
    {
    //this->StartAddingFiducials();
    }
  else
    {
    //this->StopAddingFiducials();
    }
}

void qSlicerAnnotationModuleWidget::AddFiducialCompleted(vtkObject* object,
                                                         void* call_data)
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->fiducialTypeButton->setChecked(
      false);

  std::vector<vtkMRMLNode*> nodevector;
  d->logic()->GetMRMLScene()->GetNodesByClass(
      "vtkMRMLFiducialListNode",
      nodevector);

  m_IDs.push_back(
      nodevector[nodevector.size() - 1]->GetID());
  m_index++;

  std::vector<double> thevalue;
  thevalue.push_back(
      0.0);
  QString valueString = "";
  char format[4] = " ";
  this->updateAnnotationTable(
      m_index,
      thevalue,
      format);
  this->selectRowByIndex(
      m_index);
}

void qSlicerAnnotationModuleWidget::onSaveMRMLSceneButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->logic()->SaveMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::removeAnnotation(int num)
{
  if (num > tableWidget->rowCount() || num < 0)
    return;

  for (int i = 0; i < num; ++i)
    tableWidget->removeRow(
        i);

  return;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onFiducialNodeRemoved(vtkObject* object)
{

}

void qSlicerAnnotationModuleWidgetPrivate::updateSelection(int totalitems)
{
  if (isSelectAll)
    {
    for (int i = 0; i < totalitems; ++i)
      {
      for (int j = 0; j < this->tableWidget->columnCount(); ++j)
        {
        this->tableWidget->item(
            i,
            j)->setSelected(
            true);
        }
      }
    isSelectAll = false;
    this->Ui_qSlicerAnnotationModule::selectedAllButton->setIcon(
        QIcon(
            ":/Icons/AnnotationDeselectAll.png"));
    }
  else
    {
    for (int i = 0; i < totalitems; ++i)
      {
      for (int j = 0; j < this->tableWidget->columnCount(); ++j)
        {
        this->tableWidget->item(
            i,
            j)->setSelected(
            false);
        }
      }
    isSelectAll = true;
    this->Ui_qSlicerAnnotationModule::selectedAllButton->setIcon(
        QIcon(
            ":/Icons/AnnotationSelectAll.png"));
    }

}

std::vector<int> qSlicerAnnotationModuleWidgetPrivate::updateSingleSelection()
{
  std::vector<int> seletedRows;

  if (this->tableWidget->selectedItems().count() == 0)
    {
    return seletedRows;
    }

  int totalselected = this->tableWidget->selectedItems().count();
  int totalcolumns = this->tableWidget->columnCount();

  for (int i = 0; i < totalselected; i += totalcolumns)
    {
    int currentSelectedRow = this->tableWidget->row(
        this->tableWidget->selectedItems().at(
            i));
    seletedRows.push_back(
        currentSelectedRow);
    }

  return seletedRows;

}

void qSlicerAnnotationModuleWidget::selectedAllButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  int totalAnnotations = d->tableWidget->rowCount();

  if (totalAnnotations == 0)
    {
    return;
    }

  d->updateSelection(
      totalAnnotations);

}

void qSlicerAnnotationModuleWidget::onLockUnlockAllButtonClicked()
{
  this->selectedAllButtonClicked();
  this->lockSelectedButtonClicked();
  //this->selectedAllButtonClicked();
}

void qSlicerAnnotationModuleWidget::propertyEditButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  // if one and only one annotation is selected, go ahead to open the dialog
  std::vector<int> selectedRows;
  if (m_IDs.size() >= 1)
    {
    selectedRows = d->updateSingleSelection();
    }
  else
    {
    std::cerr << "No annotation is present" << std::endl;
    return;
    }
  if (selectedRows.size() != 1)
    {
    std::cerr << "Select only one annotation to change property" << std::endl;
    QMessageBox::warning(
        d->tableWidget,
        QString(
            "Property Modifier"),
        QString(
            "Please only select one annotation in the table"));

    return;
    }

  int index = selectedRows[0];

  vtkMRMLNode* mrmlnode = d->logic()->GetMRMLScene()->GetNodeByID(
      m_IDs[index]);

  // Create the property dialog if not exist
  if (this->GetPropertyDialog(
      mrmlnode->GetID()) == NULL)
    {
    qSlicerAnnotationModuleAnnotationPropertyDialog* propertyDialog = new qSlicerAnnotationModuleAnnotationPropertyDialog(
        mrmlnode,
        d->logic());

    this->m_PropertyDialogs[mrmlnode->GetID()] = propertyDialog;
    }
  else
    {
    this->m_PropertyDialogs[mrmlnode->GetID()]->Initialize(
        mrmlnode);

    }

  // Do not process if the property dialog is already visible
  if (this->GetPropertyDialog(
      mrmlnode->GetID())->isVisible())
    {
    std::cerr << "The property Dialog is already open" << std::endl;
    QMessageBox::warning(
        d->tableWidget,
        QString(
            "Property Modifier"),
        QString(
            "The property Dialog is already open"));

    return;
    }

  qSlicerAnnotationModuleAnnotationPropertyDialog* propertyDialog = this->m_PropertyDialogs[mrmlnode->GetID()];

  this->connect(
      propertyDialog,
      SIGNAL(dialogRejected(char*)),
      this,
      SLOT(propertyRestored(char*)));
  this->connect(
      propertyDialog,
      SIGNAL(dialogAccepted(char*, QString)),
      this,
      SLOT(propertyAccepted(char*, QString)));
  this->connect(
      propertyDialog,
      SIGNAL(coordinateChanged(QString, char*)),
      this,
      SLOT(annotationCoordinateChanged(QString, char*)));
  //this->connect(propertyDialog, SIGNAL(textChanged(QString, char*)), this, SLOT(annotationTextChanged(QString, char*)) );

  propertyDialog->setVisible(
      true);
  d->SetItemEditable(
      index,
      d->TextColumn,
      false);

}

void qSlicerAnnotationModuleWidget::propertyRestored(char* nodeID)
{
  CTK_D(qSlicerAnnotationModuleWidget);

  vtkMRMLNode* node = d->logic()->GetMRMLScene()->GetNodeByID(
      nodeID);
  d->updateTextItem(
      this->getIndexByNodeID(
          nodeID),
      QString(
          d->logic()->GetAnnotationTextProperty(
              node)));
  QString valueString;
  qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
      d->logic()->GetAnnotationTextFormatProperty(
          node),
      d->logic()->GetAnnotationMeasurement(
          node),
      valueString);
  d->updateValueItem(
      this->getIndexByNodeID(
          nodeID),
      valueString);
  d->SetItemEditable(
      this->getIndexByNodeID(
          nodeID),
      d->TextColumn,
      true);
  this->RemovePropertyDialog(
      nodeID);
}

void qSlicerAnnotationModuleWidget::propertyAccepted(char* nodeID,
                                                     QString text)
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->RemovePropertyDialog(
      nodeID);
  int index = this->getIndexByNodeID(
      nodeID);
  if (text.toLatin1().data() == d->tableWidget->item(
      index,
      d->TextColumn)->text())
    {
    return;
    }
  d->logic()->ModifyPropertiesAndWidget(
      d->logic()->GetMRMLScene()->GetNodeByID(
          nodeID),
      d->logic()->TEXT,
      text.toLatin1().data());

  d->updateTextItem(
      index,
      text);
  d->SetItemEditable(
      this->getIndexByNodeID(
          nodeID),
      d->TextColumn,
      true);
}

void qSlicerAnnotationModuleWidget::onSaveAnnotationButtonClicked()
{

  QString filename = QFileDialog::getSaveFileName(
      this,
      "Save Annotation",
      QDir::currentPath(),
      "Annotations (*.txt)");

  // save the documents...
  if (!filename.isNull())
    {
    QFile file(
        filename);
    if (!file.open(
        QFile::WriteOnly | QFile::Text))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(filename) << ": " << qPrintable(file.errorString()) << std::endl;
      return;
      }

    QTextStream out(
        &file);
    // m_report has the contents for output
    out << m_report;

    }

}

void qSlicerAnnotationModuleWidget::onGenerateReportButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  if (m_ReportDialog == NULL)
    {
    m_ReportDialog = new qSlicerAnnotationModuleReportDialog();

    }

  Ui::qSlicerAnnotationModuleReportDialog ui = m_ReportDialog->getReportDialogUi();

  QString report = "<html>\n"
    "<head><meta name=\"Author\" content=\"Daniel Haehn, Kilian Pohl, Yong Zhang\"><title>3D Slicer Report</title>\n"
    "<style type=\"text/css\">\n"
    "<!--\n"
    "body {\n"
    "font-family: Helvetica, Arial;\n"
    "  padding-left: 0px;\n"
    "  padding-right: 0px;\n"
    "  padding-bottom: 0em;\n"
    "  text-align: justify;\n"
    "  margin-left: 0px;\n"
    "  line-height: 110%; \n"
    "}\n"
    "\n"
    "table.annotation  {\n"
    "  cellpadding:2;\n"
    "  cellspacing:0;\n"
    "  width:700;\n"
    "}\n"
    "\n"
    "table.title {\n"
    "  cellpadding:0;\n"
    "  cellspacing:0;\n"
    "  width:700;\n"
    "}\n"
    "\n"
    "table.title TH {\n"
    "  font-size: 20.0pt; \n"
    "  background: #ffffff\n"
    "}\n"
    "\n"
    "table.annotation TH { \n"
    "  font-size: 14.0pt;   \n"
    "}\n"
    "\n"
    "table.annotation TD {\n"
    "  font-size: 12.0pt; \n"
    "  background: #eeeeee;\n"
    "  vertical-align: top;\n"
    "}\n"
    "\n"
    ".snapshot {\n"
    "border: 10px #eeeeee solid;\n"
    "}\n"
    "\n"
    ".textField {\n"
    "text-align: left\n"
    "}\n"
    "\n"
    "-->\n"
    "</style>\n"
    "</head>\n"
    "\n"
    "<body>\n"
    "<table class=\"title\" WIDTH=700>\n"
    "<tbody>\n"
    "<TR><TH><font size=20 face=\"Helvetica\">3D Slicer Report</font></TH>\n"
    "</TR>\n"
    "</tbody>\n"
    "</table>\n"
    "<BR>\n";
  // We define style sheet and old style bc QT does not interpret stylte sheets
  QString TD = "<td  align=center  bgcolor=\"#eeeeee\">";
  QString TDtext = "<td  align=left  bgcolor=\"#eeeeee\">";

  QString TDend = "</TD>";
  QString TH = "<TH bgcolor=\"#cccccc\" ";
  QString THend = "</TH>";

  report.append(
      "<table class=\"annotation\" cellspacing=2>\n<tbody>\n<tr>\n");
  report.append(
      TH).append(
      "width=100 >&nbsp;Type").append(
      THend);
  report.append(
      TH).append(
      "width=100 >Value").append(
      THend);
  report.append(
      TH).append(
      "width=496 >Text").append(
      THend).append(
      "\n</tr>\n");

  if (m_IDs.size() > 0)
    {
    for (unsigned int i = 0; i < m_IDs.size(); ++i)
      {
      QString labelString = QString(
          "Seed %1").arg(
          QString::number(
              i + 1));

      std::vector<double> thevalue;
      QString valueString, textString;
      report.append(
          "<tr>\n").append(
          TD);

      thevalue = d->logic()->GetAnnotationMeasurement(
          d->logic()->GetMRMLScene()->GetNodeByID(
              m_IDs[i]));

      const char* format = d->logic()->GetAnnotationTextFormatProperty(
          d->logic()->GetMRMLScene()->GetNodeByID(
              m_IDs[i]));
      qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
          format,
          thevalue,
          valueString);

      report.append(
          "<img src='") .append(
          d->logic()->GetIconName(
              d->logic()->GetMRMLScene()->GetNodeByID(
                  m_IDs[i]))) .append(
          "'>");
      textString = d->logic()->GetAnnotationTextProperty(
          d->logic()->GetMRMLScene()->GetNodeByID(
              m_IDs[i]));
      report.append(
          TDend).append(
          TD).append(
          valueString).append(
          TDend).append(
          TD).append(
          textString).append(
          TDend).append(
          "\n</tr>\n");
      }
    }
  else
    {
    report.append(
        "<tr>\n").append(
        "<td  ALIGN=center  bgcolor=\"#eeeeee\" colspan=3>There is no annotation information").append(
        TDend).append(
        "\n</tr>\n");
    }

  report.append(
      "</table>\n<BR><BR>\n");

  if (!m_screenshotList.isEmpty())
    {
    report.append(
        "<table class=\"annotation\" cellspacing=2>\n<tbody>\n<tr>\n");
    report.append(
        TH).append(
        " width=700>Screen Shots").append(
        THend).append(
        "\n</TR>\n");

    foreach(QString filename, m_screenshotList)
        {
        QFile file(
            filename);
        QImage img(
            filename);
        if (img.isNull())
          {
          std::cerr << "Error: Cannot open screen shot file " << std::endl;
          return;
          }
        report.append(
            "<TR>\n").append(
            TD).append(
            "<img width=680 class=\"snapshot\" src=\"").append(
            filename).append(
            "\">").append(
            TDend).append(
            "\n</TR>\n");
        }
    report.append(
        "</tbody>\n</TABLE>\n");
    }

  report.append(
      "</body>");

  ui.reportBrowser->setHtml(
      report);
  m_ReportDialog->setVisible(
      true);

  this->connect(
      m_ReportDialog,
      SIGNAL(filenameSelected()),
      this,
      SLOT(saveAnnotationReport()));
  this->m_report = report;

}

bool qSlicerAnnotationModuleWidget::saveAnnotationReport()
{
  QString filename = m_ReportDialog->getFileName();

  if ((!filename.endsWith(
      ".html")) && (!filename.endsWith(
      ".HTML")))
    {
    filename.append(
        ".html");
    }

  QString imgdir(
      filename);
  imgdir.remove(
      imgdir.size() - 5,
      5);
  imgdir.append(
      "_files");
  QDir currentdir = QDir::current();

  if (currentdir.exists())
    {
    if (!currentdir.mkdir(
        imgdir))
      {
      std::cerr << "Error: cannot make directory" << std::endl;
      }
    }

  QStringList list = imgdir.split(
      "/");
  QString imgshortdir = list[list.size() - 1];

  QFile file(
      filename);
  if (!file.open(
      QFile::WriteOnly | QFile::Text))
    {
    std::cerr << "Error: Cannot save file " << qPrintable(filename) << ": " << qPrintable(file.errorString()) << std::endl;
    return false;
    }
  QTextStream out(
      &file);

  if (m_report.contains(
      "<img src=':/Icons/AnnotationPoint.png'>"))
    {
    QString oldstring(
        "<img src=':/Icons/AnnotationPoint.png'>");
    QString newstring(
        "");
    newstring.append(
        "<img src=\"").append(
        imgshortdir).append(
        "/AnnotationPoint.png\"").append(
        ">");
    m_report.replace(
        oldstring,
        newstring);

    // save the image
    QImage img(
        ":/Icons/AnnotationPoint.png");
    QString imgname(
        imgdir);
    imgname.append(
        "/AnnotationPoint.png");

    QFile imgfile(
        imgname);
    if (!imgfile.open(
        QFile::WriteOnly))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": " << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QImageWriter writer(
        &imgfile,
        "PNG");
    writer.write(
        img);
    imgfile.close();

    }

  if (m_report.contains(
      "<img src=':/Icons/AnnotationAngle.png'>"))
    {
    QString oldstring(
        "<img src=':/Icons/AnnotationAngle.png'>");
    QString newstring(
        "");
    newstring.append(
        "<img src=\"").append(
        imgshortdir).append(
        "/AnnotationAngle.png\"").append(
        ">");
    m_report.replace(
        oldstring,
        newstring);

    // save the image
    QImage img(
        ":/Icons/AnnotationAngle.png");
    QString imgname(
        imgdir);
    imgname.append(
        "/AnnotationAngle.png");

    QFile imgfile(
        imgname);
    if (!imgfile.open(
        QFile::WriteOnly))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": " << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QImageWriter writer(
        &imgfile,
        "PNG");
    writer.write(
        img);
    imgfile.close();

    }

  if (m_report.contains(
      "<img src=':/Icons/AnnotationDistance.png'>"))
    {
    QString oldstring(
        "<img src=':/Icons/AnnotationDistance.png'>");
    QString newstring(
        "");
    newstring.append(
        "<img src=\"").append(
        imgshortdir).append(
        "/AnnotationDistance.png\"").append(
        ">");
    m_report.replace(
        oldstring,
        newstring);

    // save the image
    QImage img(
        ":/Icons/AnnotationDistance.png");
    QString imgname(
        imgdir);
    imgname.append(
        "/AnnotationDistance.png");

    QFile imgfile(
        imgname);
    if (!imgfile.open(
        QFile::WriteOnly))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": " << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QImageWriter writer(
        &imgfile,
        "PNG");
    writer.write(
        img);
    imgfile.close();

    }

  if (!m_screenshotList.isEmpty())
    {
    foreach(QString filename, m_screenshotList )
        {
        QStringList names;
        names = filename.split(
            "/");
        QString shortname = names[names.size() - 1];
        QString oldstring = filename;
        QString newstring(
            "");
        newstring.append(
            imgshortdir).append(
            "/").append(
            shortname);
        m_report.replace(
            oldstring,
            newstring);

        // save the image
        QImage img(
            filename);
        QString imgname(
            imgdir);
        imgname.append(
            "/").append(
            shortname);

        QFile imgfile(
            imgname);
        if (!imgfile.open(
            QFile::WriteOnly))
          {
          std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": " << qPrintable(file.errorString()) << std::endl;
          return false;
          }
        QImageWriter writer(
            &imgfile,
            "PNG");
        writer.write(
            img);
        imgfile.close();

        }
    }

  out << m_report;
  file.close();

  m_ReportDialog->close();

  return true;

}

void qSlicerAnnotationModuleWidget::visibleSelectedButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  std::vector<int> selectedRows;
  bool isVisible = false;

  if (m_index >= 0)
    {
    selectedRows = d->updateSingleSelection();
    }

  if (selectedRows.size() == 0)
    {
    return;
    }
/*
  foreach(int i, selectedRows)
      {
      vtkMRMLFiducialListNode* fNode = vtkMRMLFiducialListNode::SafeDownCast(
       d->logic()->GetMRMLScene()->GetNodeByID(
       m_IDs[i]));
       if (fNode != NULL && fNode->IsA(
       "vtkMRMLFiducialListNode"))
       {
       isVisible = !fNode->GetVisibility();
       fNode->SetVisibility(
       (int) isVisible);
       }
       else
       {
       vtkMRMLAnnotationNode * node = (vtkMRMLAnnotationNode*) d->logic()->GetMRMLScene()->GetNodeByID(
       m_IDs[i]);
       if (node)
       {
       isVisible = !node->GetVisible();
       node->SetVisible(
       (int) isVisible);
       }


      }*/

  d->setInvisibleItemIcon(
      isVisible);

}

void qSlicerAnnotationModuleWidget::lockSelectedButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  std::vector<int> selectedRows;
  bool isLocked = false;

  if (m_IDs.size() >= 1)
    {
    selectedRows = d->updateSingleSelection();
    }

  if (selectedRows.size() == 0)
    {
    return;
    }

  std::cout << "Selected:" << selectedRows.size() << std::endl;
/*
  foreach(int i, selectedRows)
      {
       vtkMRMLAnnotationNode* node = vtkMRMLAnnotationNode::SafeDownCast(
       d->logic()->GetMRMLScene()->GetNodeByID(
       m_IDs[i]));
       if (node)
       {

       node->SetLocked(
       !node->GetLocked());
       }
       else
       {
       vtkMRMLFiducialListNode* fNode = vtkMRMLFiducialListNode::SafeDownCast(
       d->logic()->GetMRMLScene()->GetNodeByID(
       m_IDs[i]));
       if (fNode != NULL && fNode->IsA(
       "vtkMRMLFiducialListNode"))
       {
       isLocked = !fNode->GetLocked();
       fNode->SetLocked(
       (int) isLocked);
       }
       }

       if (this->GetPropertyDialog(
       m_IDs[i]) != NULL && this->GetPropertyDialog(
       m_IDs[i])->isVisible())
       {
       this->GetPropertyDialog(
       m_IDs[i])->UpdateLockUnlockStatus(
       isLocked);
       }

      }*/

  d->setLockUnLockIcon(
      isLocked);

}

void qSlicerAnnotationModuleWidgetPrivate::setInvisibleItemIcon(bool isVisible)
{
  std::vector<int> selectedRows;

  selectedRows = updateSingleSelection();

  foreach(int i, selectedRows)
      {
      qSlicerAnnotationModulePushButton* widget = qobject_cast<qSlicerAnnotationModulePushButton*> (
          tableWidget->cellWidget(
              i,
              VisibleColumn));

      if (isVisible)
        {
        widget->setIcon(
            QIcon(
                ":/Icons/AnnotationVisibility.png"));
        }
      else
        {
        widget->setIcon(
            QIcon(
                ":/Icons/AnnotationInvisible.png"));
        }

      tableWidget->resizeColumnToContents(
          VisibleColumn);
      tableWidget->resizeColumnToContents(
          TypeColumn);
      }

}

void qSlicerAnnotationModuleWidgetPrivate::setLockUnLockIcon(bool isLocked)
{
  std::vector<int> selectedRows;

  selectedRows = updateSingleSelection();

  foreach(int i, selectedRows)
      {
      qSlicerAnnotationModulePushButton* widget = qobject_cast<qSlicerAnnotationModulePushButton*> (
          tableWidget->cellWidget(
              i,
              LockColumn));

      if (isLocked)
        {
        widget->setIcon(
            QIcon(
                ":/Icons/AnnotationLock.png"));
        }
      else
        {
        widget->setIcon(
            QIcon(
                ":/Icons/AnnotationUnlock.png"));
        }

      tableWidget->resizeColumnToContents(
          VisibleColumn);
      tableWidget->resizeColumnToContents(
          TypeColumn);
      }

}

void qSlicerAnnotationModuleWidget::annotationCoordinateChanged(QString valueString,
                                                                char* nodeId)
{
  CTK_D(qSlicerAnnotationModuleWidget);
  d->updateValueItem(
      this->getIndexByNodeID(
          nodeId),
      valueString);
}

void qSlicerAnnotationModuleWidget::selectRowByIndex(int index)
{
  CTK_D(qSlicerAnnotationModuleWidget);

  int totalitems = m_IDs.size();
  for (int i = 0; i < totalitems; ++i)
    {
    for (int j = 0; j < d->tableWidget->columnCount(); ++j)
      {
      d->tableWidget->item(
          i,
          j)->setSelected(
          false);
      }
    }

  for (int j = 0; j < d->tableWidget->columnCount(); ++j)
    {
    d->tableWidget->item(
        index,
        j)->setSelected(
        true);
    }

}

void qSlicerAnnotationModuleWidget::deleteSelectedButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  std::vector<int> selectedRows;

  selectedRows = d->updateSingleSelection();
  if (m_index < 0)
    {
    return;
    }

  if (selectedRows.size() == 0)
    return;

  switch (QMessageBox::question(
      this,
      tr(
          "Confirm the Deletion"),
      tr(
          "Delete Selected Annotations?"),
      QMessageBox::Yes | QMessageBox::No))
    {
    case QMessageBox::Yes:
      {
      for (int i = selectedRows.size() - 1; i >= 0; --i)
        {
        if (this->GetPropertyDialog(
            m_IDs[selectedRows[i]]) != NULL)
          {
          this->GetPropertyDialog(
              m_IDs[selectedRows[i]])->setVisible(
              false);
          this->RemovePropertyDialog(
              m_IDs[selectedRows[i]]);
          }
        d->logic()->RemoveAnnotationByID(
            m_IDs[selectedRows[i]]);
        m_IDs.erase(
            m_IDs.begin() + selectedRows[i]);
        m_index--;
        }

      d->removeAnnotation(
          m_index + 2);

      std::vector<double> thevalue;
      // This is just a hack right now for fiducials
      char _format[4] = "%d";
      const char* format = _format;
      if (m_index >= 0)
        {
        for (int i = 0; i <= m_index; ++i)
          {
          thevalue = d->logic()->GetAnnotationMeasurement(
              d->logic()->GetMRMLScene()->GetNodeByID(
                  m_IDs[i]));
          format = d->logic()->GetAnnotationTextFormatProperty(
              d->logic()->GetMRMLScene()->GetNodeByID(
                  m_IDs[i]));
          this->updateAnnotationTable(
              i,
              thevalue,
              format);
          QString valueString;
          qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
              format,
              thevalue,
              valueString);
          if (this->GetPropertyDialog(
              m_IDs[i]))
            {
            if (this->GetPropertyDialog(
                m_IDs[i])->isVisible())
              {
              this->GetPropertyDialog(
                  m_IDs[i])->updateValue(
                  valueString);
              }
            }
          }

        }

      }
      break;
    case QMessageBox::No:
      qDebug(
          "no");
      break;
    default:
      qDebug(
          "close");
      break;
    }

}

void qSlicerAnnotationModuleWidget::onCreateMeasurementAngleButtonToggled(bool toggle)
{
  /* CTK_D(qSlicerAnnotationModuleWidget);

   if ( toggle )
   {
   const char *newAngleNodeID = d->logic()->AddAngle();

   if (!newAngleNodeID)
   {
   std::cerr << "Could not add Angle" << std::endl;
   return;
   }

   // please make sure to initialize the variable in the constructor and empty it in the destructor
   m_IDs.push_back( newAngleNodeID );
   m_index++;

   qvtkConnect(d->logic(),  vtkSlicerAnnotationModuleLogic::AddAngleCompletedEvent,
   this, SLOT(AddAngleCompleted(vtkObject*, void*)) );


   }
   */
}

void qSlicerAnnotationModuleWidget::onCreateMeasurementRulerButtonClicked()
{/*
 CTK_D(qSlicerAnnotationModuleWidget);
 const char *newRulerNodeID = d->logic()->AddRuler();
 if (!newRulerNodeID)
 {
 std::cerr << "Could not add Ruler" << std::endl;
 return;
 }

 std::vector<double> thevalue = d->logic()->GetAnnotationMeasurement( d->logic()->GetMRMLScene()->GetNodeByID(newRulerNodeID) );

 m_IDs.push_back( newRulerNodeID );
 m_index++;

 //vtkMRMLAnnotationRulerNode* node = d->logic()->GetRulerNodeByID( newRulerNodeID );

 char* format = node->GetDistanceAnnotationFormat();
 QString valueString;
 qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(format, thevalue, valueString);
 this->updateAnnotationTable( m_index, thevalue, format );
 this->selectRowByIndex( m_index );

 // watch for the control points being modified
 //qvtkConnect(d->logic()->GetRulerNodeByID( newRulerNodeID),  vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, this, SLOT(updateValue(vtkObject*, void*)) );
 //qvtkConnect(d->logic()->GetRulerNodeByID( newRulerNodeID),  vtkMRMLAnnotationRulerNode::ValueModifiedEvent, this, SLOT(updateValue(vtkObject*, void*)) );
 // watch for transform modified events
 //qvtkConnect(d->logic()->GetRulerNodeByID( newRulerNodeID),  vtkMRMLTransformableNode::TransformModifiedEvent,
 ///   this, SLOT(updateValue(vtkObject*, void*)) );
 // watch for general modified events
 //qvtkConnect(d->logic()->GetRulerNodeByID( newRulerNodeID),  vtkCommand::ModifiedEvent,
 //  this, SLOT(updateValue(vtkObject*, void*)) );
 */
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::updateAnnotationTable(int index,
                                                          std::vector<double> thevalue,
                                                          const char* format)
{
  CTK_D(qSlicerAnnotationModuleWidget);

  QString textString = QString(
      d->logic()->GetTextOfNodeByID(
          this->m_IDs[index]));

  QString labelString = QString(
      "Seed %1").arg(
      QString::number(
          m_index));
  QString valueString;
  qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
      format,
      thevalue,
      valueString);

  d->updateAnnotation(
      index,
      labelString,
      valueString,
      textString);

}

void qSlicerAnnotationModuleWidget::updateAnnotationInTableByID(const char* id,
                                                                std::vector<double> value,
                                                                const char* format)
{

  std::cout << "ENTERING: " << value[0] << std::endl;

  CTK_D(qSlicerAnnotationModuleWidget);

  int index = this->getIndexByNodeID(
      id);


  if (index < 0)
    {
    this->addNodeToTable(id);
    index = this->getIndexByNodeID(
          id);
    }

  QString valueString;
  qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
      format,
      value,
      valueString);

  std::cout << "here: " << valueString.toStdString() << std::endl;


  d->updateValueItem(
      index,
      valueString);

  // update property dialog
  qSlicerAnnotationModuleAnnotationPropertyDialog* pdialog = this->GetPropertyDialog(id);
  if (pdialog != NULL)
    {
    if (pdialog->isVisible())
      {
        for (vtkIdType t = 0; t < d->logic()->GetNumberOfControlPointsByID(
            id); t++)
          {
          pdialog->updateCoordinates(
              d->logic()->GetAnnotationControlPointsCoordinateByID(
                  id,
                  t),
              t);
          }
      pdialog->updateValue(
          valueString);
      }
    }

}

int qSlicerAnnotationModuleWidget::getIndexByNodeID(const char* nodeID)
{
  int num = this->m_IDs.size();
  std::string id = std::string(
      nodeID);
  for (int n = 0; n < num; n++)
    {
    const char *thisID = this->m_IDs[n];
    if (id.compare(
        thisID) == 0)
      {
      return n;
      }
    }
  return -1;
}

void qSlicerAnnotationModuleWidget::annotationTextChanged(QString text,
                                                          char* nodeId)
{
  CTK_D(qSlicerAnnotationModuleWidget);
  d->logic()->ModifyPropertiesAndWidget(
      d->logic()->GetMRMLScene()->GetNodeByID(
          nodeId),
      d->logic()->TEXT,
      text.toLatin1().data());
  d->updateTextItem(
      this->getIndexByNodeID(
          nodeId),
      text);
}

void qSlicerAnnotationModuleWidget::onScreenShotButtonClicked()
{
  //CTK_D(qSlicerAnnotationModuleWidget);
  /*
   vtkImageData* image = vtkImageData::New();
   //image = d->logic()->SaveScreenShot();

   if (m_ScreenShotDialog == NULL)
   {
   m_ScreenShotDialog = new qSlicerAnnotationModuleScreenShotDialog();
   }

   Ui::qSlicerAnnotationModuleScreenShotDialog ui = m_ScreenShotDialog->getScreenShotDialogUi();

   QImage img;
   qSlicerAnnotationModuleImageUtil::fromImageData(
   image,
   img);
   m_screenshot = QPixmap::fromImage(
   img);

   ui.screenShotLabel->setPixmap(
   m_screenshot);
   ui.screenShotLabel->setScaledContents(
   true);

   m_ScreenShotDialog->setVisible(
   true);

   this->connect(
   m_ScreenShotDialog,
   SIGNAL(filenameSelected()),
   this,
   SLOT(saveScreenShot()));*/
}

bool qSlicerAnnotationModuleWidget::saveScreenShot()
{
  QString filename = m_ScreenShotDialog->getFileName();

  QFile file(
      filename);
  if (!file.open(
      QFile::WriteOnly))
    {
    std::cerr << "Error: Cannot save file " << qPrintable(filename) << ": " << qPrintable(file.errorString()) << std::endl;
    return false;
    }

  QImageWriter writer(
      &file,
      "PNG");
  writer.write(
      m_screenshot.toImage());
  file.close();

  m_ScreenShotDialog->close();

  m_screenshotList.push_back(
      filename);

  return true;

}

void qSlicerAnnotationModuleWidget::onItemSelectionChanged()
{/*
 CTK_D(qSlicerAnnotationModuleWidget);

 std::vector<const char*> seletedIDs;
 int totalselected = d->tableWidget->selectedItems().count();
 int totalcolumns = d->tableWidget->columnCount();

 for(int i=0; i<totalselected; i+=totalcolumns )
 {
 int index = d->tableWidget->row(d->tableWidget->selectedItems().at(i));
 seletedIDs.push_back( m_IDs[index] );
 }

 d->logic()->SetAnnotationSelectedByIDs( seletedIDs, m_IDs );
 */
}

void qSlicerAnnotationModuleWidget::AddAngleCompleted(vtkObject* object,
                                                      void* call_data)
{
  //CTK_D(qSlicerAnnotationModuleWidget);
  /*
   d->angleTypeButton->setChecked(
   false);
   vtkMRMLAnnotationAngleNode* node = vtkMRMLAnnotationAngleNode::SafeDownCast(
   d->logic()->GetMRMLScene()->GetNodeByID(
   m_IDs[m_index]));

   const char* newAngleNodeID = node->GetID();
   std::vector<double> thevalue;
   thevalue.push_back(
   0.0);
   thevalue = d->logic()->GetAnnotationMeasurement(
   d->logic()->GetMRMLScene()->GetNodeByID(
   newAngleNodeID));

   char* format = node->GetLabelFormat();
   QString valueString;
   qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
   format,
   thevalue,
   valueString);
   this->updateAnnotationTable(
   m_index,
   thevalue,
   format);
   this->selectRowByIndex(
   m_index);

   // watch for the control points being modified
   //qvtkConnect(d->logic()->GetAngleNodeByID( newAngleNodeID),  vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent, this, SLOT(updateValue(vtkObject*, void*)) );
   qvtkConnect(
   d->logic()->GetAngleNodeByID(
   newAngleNodeID),
   vtkMRMLAnnotationAngleNode::ValueModifiedEvent,
   this,
   SLOT(updateValue(vtkObject*, void*)));
   // watch for transform modified events
   qvtkConnect(
   d->logic()->GetAngleNodeByID(
   newAngleNodeID),
   vtkMRMLTransformableNode::TransformModifiedEvent,
   this,
   SLOT(updateValue(vtkObject*, void*)));
   // watch for general modified events
   qvtkConnect(
   d->logic()->GetAngleNodeByID(
   newAngleNodeID),
   vtkCommand::ModifiedEvent,
   this,
   SLOT(updateValue(vtkObject*, void*)));*/

}

QString qSlicerAnnotationModuleWidget::getAnnotationIconName(int index,
                                                             bool isEdit)
{
  CTK_D(qSlicerAnnotationModuleWidget);
  return d->logic()->GetIconName(
      d->logic()->GetMRMLScene()->GetNodeByID(
          m_IDs[index]),
      isEdit);
}

//-----------------------------------------------------------------------------
// Resume, Pause, Cancel and Done buttons
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::enableMouseModeButtons()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->pauseButton->setChecked(
      false);
  d->resumeButton->setChecked(
      false);
  d->cancelButton->setChecked(
      false);
  d->doneButton->setChecked(
      false);
  d->pauseButton->setEnabled(
      true);
  d->resumeButton->setEnabled(
      true);
  d->cancelButton->setEnabled(
      true);
  d->doneButton->setEnabled(
      true);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::disableMouseModeButtons()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->pauseButton->setChecked(
      false);
  d->resumeButton->setChecked(
      false);
  d->cancelButton->setChecked(
      false);
  d->doneButton->setChecked(
      false);
  d->pauseButton->setEnabled(
      false);
  d->resumeButton->setEnabled(
      false);
  d->cancelButton->setEnabled(
      false);
  d->doneButton->setEnabled(
      false);

  this->m_lastAddedIndex = -1;
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onResumeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->pauseButton->setChecked(
      false);
  d->resumeButton->setChecked(true);

  switch (this->m_CurrentAnnotationType)
    {
    case qSlicerAnnotationModuleWidget::TextNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationTextNode");
      break;
    case qSlicerAnnotationModuleWidget::AngleNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationAngleNode");
      break;
    case qSlicerAnnotationModuleWidget::FiducialNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationFiducialNode");
      break;
    case qSlicerAnnotationModuleWidget::StickyNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationStickyNode");
      break;
    case qSlicerAnnotationModuleWidget::SplineNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationSplineNode");
      break;
    case qSlicerAnnotationModuleWidget::RulerNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationRulerNode");
      break;
    case qSlicerAnnotationModuleWidget::BidimensionalNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationBidimensionalNode");
      break;
    case qSlicerAnnotationModuleWidget::ROINode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationROINode");
      break;
    }

  /*if ( toggle )
   {
   d->resumeButton->setChecked(false);
   vtkMRMLInteractionNode *interactionNode = NULL;
   interactionNode = d->logic()->GetApplicationLogic()->GetInteractionNode();
   //this->SetInteractionNode ( interactionNode );

   interactionNode->NormalizeAllMouseModes();
   interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
   interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
   }*/

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onPauseButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->resumeButton->setChecked(
      false);
  d->pauseButton->setChecked(true);
  d->logic()->StopPlaceMode();

  /*if ( toggle )
   {
   d->resumeButton->setChecked(false);
   vtkMRMLInteractionNode *interactionNode = NULL;
   interactionNode = d->logic()->GetApplicationLogic()->GetInteractionNode();
   //this->SetInteractionNode ( interactionNode );

   interactionNode->NormalizeAllMouseModes();
   interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
   interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
   }*/

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onCancelButtonClicked()
{

  this->cancelOrRemoveLastAddedAnnotationNode();

  this->enableAllAnnotationTools();
  this->resetAllAnnotationTools();
}

void qSlicerAnnotationModuleWidget::cancelOrRemoveLastAddedAnnotationNode()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  if (this->m_lastAddedIndex != -1)
    {

    d->logic()->CancelCurrentOrRemoveLastAddedAnnotationNode();

    // TODO
    this->m_IDs.pop_back();
    d->tableWidget->removeRow(
        this->m_lastAddedIndex);
    this->m_index--;
    this->m_lastAddedIndex = -1;
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onDoneButtonClicked()
{

  this->enableAllAnnotationTools();
  this->resetAllAnnotationTools();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::resetAllAnnotationTools()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = 0;

  d->textTypeButton->setChecked(false);
  d->angleTypeButton->setChecked(false);
  d->roiTypeButton->setChecked(false);
  d->fiducialTypeButton->setChecked(false);
  d->splineTypeButton->setChecked(false);
  d->stickyTypeButton->setChecked(false);
  d->rulerTypeButton->setChecked(false);
  d->bidimensionalTypeButton->setChecked(false);

  d->logic()->StopPlaceMode();

  this->disableMouseModeButtons();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::disableAllAnnotationTools()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->textTypeButton->setEnabled(false);
  d->angleTypeButton->setEnabled(false);
  d->roiTypeButton->setEnabled(false);
  d->fiducialTypeButton->setEnabled(false);
  d->splineTypeButton->setEnabled(false);
  d->stickyTypeButton->setEnabled(false);
  d->rulerTypeButton->setEnabled(false);
  d->bidimensionalTypeButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::enableAllAnnotationTools()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  d->textTypeButton->setEnabled(true);
  d->angleTypeButton->setEnabled(true);
  d->roiTypeButton->setEnabled(true);
  d->fiducialTypeButton->setEnabled(true);
  d->splineTypeButton->setEnabled(true);
  d->stickyTypeButton->setEnabled(true);
  d->rulerTypeButton->setEnabled(true);
  d->bidimensionalTypeButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleAnnotationPropertyDialog*
qSlicerAnnotationModuleWidget::GetPropertyDialog(const char* id)
{
  std::map<std::string, qSlicerAnnotationModuleAnnotationPropertyDialog*>::iterator iter;
  for (iter = this->m_PropertyDialogs.begin(); iter != this->m_PropertyDialogs.end(); iter++)
    {
    if (iter->first.c_str() && !strcmp(
        iter->first.c_str(),
        id))
      {
      return iter->second;
      }
    }
  return NULL;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::RemovePropertyDialog(const char* id)
{
  if (this->GetPropertyDialog(
      id) != NULL)
    {
    delete this->m_PropertyDialogs[id];
    this->m_PropertyDialogs.erase(
        id);
    }

}

//-----------------------------------------------------------------------------
// Sticky Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onStickyNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::StickyNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  // this is a hack to export the sticky note icon
  // *sigh*
  QIcon icon = QIcon(":/Icons/AnnotationNote.png");
  QPixmap pixmap = icon.pixmap(16,16);
  //QString tempdir = QString(std::getenv("TMPDIR"));
  QString tempdir = QString("/tmp/");
  tempdir.append("sticky.png");
  pixmap.save(tempdir);
  // end of hack

  d->stickyTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);

}

//-----------------------------------------------------------------------------
// Angle Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onAngleNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::AngleNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->angleTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}

//-----------------------------------------------------------------------------
// Text Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onTextNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::TextNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->textTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}

//-----------------------------------------------------------------------------
// Spline Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onSplineNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::SplineNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->splineTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}

//-----------------------------------------------------------------------------
// Ruler Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onRulerNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::RulerNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->rulerTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}

//-----------------------------------------------------------------------------
// Angle Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onFiducialNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::FiducialNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->fiducialTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}

//-----------------------------------------------------------------------------
// Angle Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onBidimensionalNodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::BidimensionalNode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->bidimensionalTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}

//-----------------------------------------------------------------------------
// ROI Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onROINodeButtonClicked()
{
  CTK_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::ROINode;

  d->logic()->SetAndObserveWidget(
      this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();


  this->disableAllAnnotationTools();

  d->roiTypeButton->setChecked(
      true);
  d->resumeButton->setChecked(
      true);
}





//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::addNodeToTable(const char* newNodeID)
{
  CTK_D(qSlicerAnnotationModuleWidget);

  int index = this->getIndexByNodeID(newNodeID);


  if (index >= 0)
    {
    // node already exists
    return;
    }

  m_IDs.push_back(
      newNodeID);
  m_index++;

  this->m_lastAddedIndex = this->m_index;

  std::vector<double> thevalue;
  thevalue.push_back(
      0.0);
  thevalue = d->logic()->GetAnnotationMeasurement(
      d->logic()->GetMRMLScene()->GetNodeByID(
          newNodeID));

  char* format = const_cast<char*> (" ");
  QString valueString;
  qSlicerAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(
      format,
      thevalue,
      valueString);
  this->updateAnnotationTable(
      m_index,
      thevalue,
      format);
  this->selectRowByIndex(
      m_index);

}
