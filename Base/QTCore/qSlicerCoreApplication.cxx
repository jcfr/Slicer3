#include "qSlicerCoreApplication.h"

// SlicerQT includes
#include "qSlicerCoreModuleManager.h"
#include "qSlicerModuleFactory.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// QT includes
#include <QVector>
#include <QStringList>
#include <QDebug>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkEventBroker.h"

// VTK includes
#include "vtkSmartPointer.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
struct qSlicerCoreApplicationPrivate: public qCTKPrivate<qSlicerCoreApplication>
{
  QCTK_DECLARE_PUBLIC(qSlicerCoreApplication);
  typedef qSlicerCoreApplicationPrivate Self; 
  qSlicerCoreApplicationPrivate()
    {
    this->AppLogic = 0;
    this->MRMLScene = 0;
    this->ModuleManager = 0; 
    }

  ~qSlicerCoreApplicationPrivate()
    {
    if (this->ModuleManager)
      {
      this->ModuleManager->factory()->uninstantiateAll();
      delete this->ModuleManager; 
      }
    if (this->AppLogic) { this->AppLogic->Delete(); }
    if (this->MRMLScene) { this->MRMLScene->Delete(); }
    }

  // Description:
  // Given the program name, should return Slicer Home Directory
  void discoverSlicerHomeDirectory(const QString& programName);

  // Description:
  // Given the program name, attempt to return the corresponding binary directory
  QString discoverSlicerBinDirectory(const QString& programName);

  // Description:
  // Accept argument of the form "FOO=BAR" and update the process environment
  int putEnv(const QString& value);

  // Description:
  // MRMLScene and AppLogic pointers
  // Note: Since the logic and the scene should be deleted before the EventBroker,
  // they are not SmartPointer.
  vtkMRMLScene*                        MRMLScene;
  vtkSlicerApplicationLogic*           AppLogic;
  
  vtkSmartPointer<vtkEventBroker>      EventBroker;

  QString                              SlicerHome;

  // Description:
  // ModuleManager - It should exist only one instance of the factory
  qSlicerCoreModuleManager*            ModuleManager;

  // For ::PutEnv
  // See http://groups.google.com/group/comp.unix.wizards/msg/f0915a043bf259fa?dmode=source
  struct DeletingCharVector : public QVector<char*>
  {
    ~DeletingCharVector()
      {
      for (int i = 0; i < this->size(); ++i)
        {
        delete []this->at(i); 
        }
      }
  };

  // On windows plateform, after the method 'discoverSlicerBinDirectory' has been called,
  // HasIntDir should be set to true and IntDir should be set to either Debug,
  // Release, RelWithDebInfo, MinSizeRel or any other custom build type.
  bool               HasIntDir;
  QString            IntDir; 
  
};

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  QCTK_INIT_PRIVATE(qSlicerCoreApplication);
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication::~qSlicerCoreApplication()
{
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication* qSlicerCoreApplication::application()
{
  qSlicerCoreApplication* app = qobject_cast<qSlicerCoreApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initialize()
{
  QCTK_D(qSlicerCoreApplication);
  d->discoverSlicerHomeDirectory(this->arguments().at(0));
  
  // Take ownership of the vtkEventBroker instance.
  // Note: Since EventBroker is a SmartPointer, the object will be deleted when
  // qSlicerCoreApplicationPrivate will be deleted
  d->EventBroker.TakeReference(vtkEventBroker::GetInstance());
  
  // Create MRML scene
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtksys_stl::string root = vtksys::SystemTools::GetCurrentWorkingDirectory();
  scene->SetRootDirectory(root.c_str());
  vtkMRMLScene::SetActiveScene( scene );

  // Create the application Logic object,
  vtkSmartPointer<vtkSlicerApplicationLogic> appLogic =
    vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  appLogic->SetMRMLScene(scene);

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  appLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  appLogic->SetAndObserveMRMLScene(scene);
  appLogic->CreateProcessingThread();

  // --- First scene needs a crosshair to be added manually
  vtkSmartPointer<vtkMRMLCrosshairNode> crosshair = vtkSmartPointer<vtkMRMLCrosshairNode>::New();
  crosshair->SetCrosshairName("default");
  scene->AddNode( crosshair );

  this->setMRMLScene(scene);
  this->setAppLogic(appLogic);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  QCTK_D(qSlicerCoreApplication);
  if (d->MRMLScene == mrmlScene)
    {
    return;
    }
  d->MRMLScene = mrmlScene;
  d->MRMLScene->Register(NULL);
  emit this->currentMRMLSceneChanged(mrmlScene);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setAppLogic(vtkSlicerApplicationLogic* appLogic)
{
  QCTK_D(qSlicerCoreApplication);
  d->AppLogic = appLogic;
  d->AppLogic->Register(NULL);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerHome()
{
  // TODO Use QCoreApplication::applicationDirPath
  return qctk_d()->SlicerHome;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, const QString&, setSlicerHome, SlicerHome);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initializeLoadableModulesPaths()
{
  QCTK_D(qSlicerCoreApplication);
  
  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  QString defaultQTModulePaths = this->slicerHome() + "/"
                                                    + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR;

//   if (hasIntDir)
//     {
//     defaultQTModulePaths = defaultQTModulePaths + PathSep +
//       slicerHome + "/" + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR + "/" + intDir;
//     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QString qtModulePaths = /*userModulePaths + PathSep + */defaultQTModulePaths;
  this->addLibraryPath(defaultQTModulePaths);

//   foreach (QString path, this->libraryPaths())
//     {
//     qDebug() << "libraryPath:" << path;
//     }

  QStringList paths;
  paths << qtModulePaths;
  d->ModuleManager->factory()->setLoadableModuleSearchPaths(paths);
  //qDebug() << "initializeLoadableModulesPaths - qtModulePaths:" << qtModulePaths;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initializeCmdLineModulesPaths()
{
  QCTK_D(qSlicerCoreApplication);
  
  QString defaultCmdLineModulePaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultCmdLineModulePaths = this->slicerHome() + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR;
//   if (hasIntDir)
//     {
//     defaultQTModulePaths = defaultCmdLineModulePaths + PathSep +
//       slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "/" + intDir;
//     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QString cmdLineModulePaths = /*userModulePaths + PathSep +*/ defaultCmdLineModulePaths;
  this->addLibraryPath(defaultCmdLineModulePaths);

  QStringList paths;
  paths << cmdLineModulePaths;
  d->ModuleManager->factory()->setCmdLineModuleSearchPaths(paths);
  //cout << "cmdLineModulePaths:" << cmdLineModulePaths << endl;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, qSlicerCoreModuleManager*, setModuleManager, ModuleManager);
QCTK_GET_CXX(qSlicerCoreApplication, qSlicerCoreModuleManager*, coreModuleManager, ModuleManager);

//-----------------------------------------------------------------------------
// qSlicerCoreApplicationPrivate methods

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverSlicerHomeDirectory(const QString& programName)
{
   // set the Slicer3_HOME variable if it doesn't already exist from the launcher 
  const char* homeFromEnv = getenv("Slicer3_HOME");
  if (!homeFromEnv)
    {
    QString slicerBinDir = this->discoverSlicerBinDirectory(programName);
    if (slicerBinDir.isEmpty())
      {
      return;
      }
      
    std::string home = vtksys::SystemTools::CollapseFullPath((slicerBinDir + "/..").toLatin1());

    this->SlicerHome = QString::fromStdString(home); 

    // Update env
    QString homeEnv = "Slicer3_HOME=%1";
    qDebug() << "Set environment: " << homeEnv.arg(this->SlicerHome);
    this->putEnv(homeEnv.arg(this->SlicerHome));
    //vtkKWApplication::PutEnv(const_cast <char *> (homeEnv.c_str()));
    }
  else
    {
    this->SlicerHome = QString::fromLatin1(homeFromEnv);
    }
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplicationPrivate::discoverSlicerBinDirectory(const QString& programName)
{
  std::string programPath;
  std::string errorMessage;
  if ( !vtksys::SystemTools::FindProgramPath(programName.toLatin1(), programPath, errorMessage) )
    {
    qCritical() << "Cannot find Slicer3 executable - " << errorMessage.c_str();
    return "";
    }

  std::string slicerBinDir = vtksys::SystemTools::GetFilenamePath(programPath.c_str());

  std::string intDir = "";
  
  // If the path: [slicerBinDir + Slicer3_INSTALL_LIB_DIR] isn't valid, try to
  // discover the appropriate one
  std::string tmpName = slicerBinDir + "/../" + Slicer3_INSTALL_LIB_DIR;
  if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
    {
    // Handle Visual Studio IntDir
    std::vector<std::string> pathComponents;
    vtksys::SystemTools::SplitPath(slicerBinDir.c_str(), pathComponents);

    slicerBinDir = slicerBinDir + "/..";
    tmpName = slicerBinDir + "/../" + Slicer3_INSTALL_LIB_DIR;
    if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
      {
      qCritical() << "Cannot find Slicer3 libraries";
      return "";
      }

    if (pathComponents.size() > 0)
      {
      this->HasIntDir = true;
      this->IntDir = QString::fromStdString(pathComponents[pathComponents.size()-1]);
      }
    }

  slicerBinDir = vtksys::SystemTools::CollapseFullPath(slicerBinDir.c_str());

  return QString::fromStdString(slicerBinDir);
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplicationPrivate::putEnv(const QString& value)
{ 
  static Self::DeletingCharVector local_environment;
  char *env_var = new char[value.size() + 1];
  strcpy(env_var, value.toLatin1());
  int ret = putenv(env_var);
  // Save the pointer in the static vector so that it can be deleted on exit
  // See http://groups.google.com/group/comp.unix.wizards/msg/f0915a043bf259fa?dmode=source
  local_environment << env_var;
  return ret == 0;
}