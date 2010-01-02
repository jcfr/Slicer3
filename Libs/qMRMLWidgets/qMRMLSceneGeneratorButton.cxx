#include "qMRMLSceneGeneratorButton.h"

// QT includes
#include <QDebug>

// MRML includes 
#include <vtkMRMLScene.h>

// qMRML includes
#include "qMRMLNodeFactory.h"

class qMRMLSceneGeneratorButtonPrivate: public qCTKPrivate<qMRMLSceneGeneratorButton>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLTreeWidget);
  void init();
  
  vtkMRMLScene*  MRMLScene;
};

// --------------------------------------------------------------------------
void qMRMLSceneGeneratorButtonPrivate::init()
{
  QCTK_P(qMRMLSceneGeneratorButton);
  this->MRMLScene = 0;
  p->connect(p, SIGNAL(clicked()), p, SLOT(generateScene()));
}

// --------------------------------------------------------------------------
// qMRMLSceneGeneratorButton methods

// --------------------------------------------------------------------------
qMRMLSceneGeneratorButton::qMRMLSceneGeneratorButton(QWidget* parent)
  :QPushButton("Generate New Scene", parent)
{
  QCTK_INIT_PRIVATE(qMRMLSceneGeneratorButton);
  qctk_d()->init();
}

// --------------------------------------------------------------------------
qMRMLSceneGeneratorButton::~qMRMLSceneGeneratorButton()
{
  this->clear();
}

// --------------------------------------------------------------------------
void qMRMLSceneGeneratorButton::clear()
{
  QCTK_D(qMRMLSceneGeneratorButton);
  if (d->MRMLScene)
    {
    emit mrmlSceneSet(0);
    d->MRMLScene->Delete();
    d->MRMLScene = 0;
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneGeneratorButton::generateScene()
{
  QCTK_D(qMRMLSceneGeneratorButton);
  
  this->clear();
  d->MRMLScene = vtkMRMLScene::New();
  
  qMRMLNodeFactory factory(this);
  factory.setMRMLScene(d->MRMLScene);
  
  int numClasses = d->MRMLScene->GetNumberOfRegisteredNodeClasses();
  int numNodes = 15;
  QList<QString> nodeNames;
  for (int i = 0; i < numNodes ; ++i)
    {
    int classNumber = rand() % numClasses; 
    vtkMRMLNode* node = d->MRMLScene->GetNthRegisteredNodeClass(classNumber);
    QString classname = QLatin1String(node->GetClassName()); 
    if (classname.isEmpty())
      {
      qWarning() << "Class registered (#" << classNumber << "):"
                 << node << " has an empty classname";
      continue;
      }
    nodeNames << QLatin1String(node->GetClassName());
    factory.createNode(node->GetClassName());
    }

  emit this->randomMRMLNodeType(nodeNames.at(rand() % nodeNames.size()));

  qDebug() << "Scene generated; Number of nodes: " << d->MRMLScene->GetNumberOfNodes();
  emit mrmlSceneSet(d->MRMLScene);
}

vtkMRMLScene* qMRMLSceneGeneratorButton::mrmlScene()const
{
  QCTK_D(const qMRMLSceneGeneratorButton);
  return d->MRMLScene;
}