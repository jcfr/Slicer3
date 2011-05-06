#include "vtkObjectFactory.h"
#include "vtkSlicerCommonInterface.h"


#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
#include "vtkKWTkUtilities.h"
#include "../../Applications/GUI/Slicer3Helper.cxx"

#endif

//----------------------------------------------------------------------------
vtkSlicerCommonInterface* vtkSlicerCommonInterface::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkSlicerCommonInterface");
  if(ret)
    {
    return (vtkSlicerCommonInterface*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerCommonInterface;
}


//----------------------------------------------------------------------------
vtkSlicerCommonInterface::vtkSlicerCommonInterface() {
}

//----------------------------------------------------------------------------
vtkSlicerCommonInterface::~vtkSlicerCommonInterface() {
}

//----------------------------------------------------------------------------
Tcl_Interp* vtkSlicerCommonInterface::GetTclInterpeter(int argc, char *argv[], ostream *err)
{
#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::InitializeTcl(argc, argv, err);

#endif

  return 0;
}

//----------------------------------------------------------------------------

int vtkSlicerCommonInterface::SourceTclFile(const char *tclFile)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3

  // Load Tcl File defining the setting
  if (!vtkSlicerApplication::GetInstance()->LoadScript(tclFile))
    {
      return 1;
    }

#endif

  return 0;

}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::EvaluateTcl(const char* command)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->Script(command);
  //#else

  // TODO Slicer4

#endif

  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetTclNameFromPointer(vtkObject *obj)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkKWTkUtilities::GetTclNameFromPointer(vtkSlicerApplication::GetInstance()->GetMainInterp(),obj);
  //#else

  // TODO Slicer4

#endif

  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetApplicationTclName()
{
#ifdef Slicer3_USE_KWWIDGETS

  return this->GetTclNameFromPointer(vtkSlicerApplication::GetInstance());

#endif

  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetTemporaryDirectory()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->GetTemporaryDirectory();

#else

  // Slicer4
  return qSlicerApplication::application()->temporaryPath().toLatin1();

#endif

}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetRepositoryRevision()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->GetSvnRevision();

#else

  // Slicer4
  return qSlicerApplication::application()->repositoryRevision().toLatin1();

#endif

  return 0;

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::SetApplicationBinDir(const char* bindir)
{
#ifdef Slicer3_USE_KWWIDGETS

  vtkSlicerApplication::GetInstance()->SetBinDir(bindir);

#endif
}

//-----------------------------------------------------------------------------
vtkHTTPHandler* vtkSlicerCommonInterface::GetHTTPHandler(vtkMRMLScene* scene)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkHTTPHandler::SafeDownCast(scene->FindURIHandlerByName("HTTPHandler"));

#else

  // Slicer4
  // TODO
  return 0;

#endif

  return 0;

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::PromptBeforeExitOff()
{

#ifdef Slicer3_USE_KWWIDGETS

  vtkSlicerApplication::GetInstance()->PromptBeforeExitOff();

#endif

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::DestroySlicerApplication()
{

#ifdef Slicer3_USE_KWWIDGETS

  vtkSlicerApplication* app = vtkSlicerApplication::GetInstance();

  if (app)
    {
      app->Exit();
      app->Delete();
      app = NULL;
    }

#endif

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic *appLogic, vtkDataIOManagerLogic *dataIOManagerLogic)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  Slicer3Helper::AddDataIOToScene(mrmlScene,vtkSlicerApplication::GetInstance(),appLogic,dataIOManagerLogic);

#endif

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::RemoveDataIOFromScene(vtkMRMLScene* mrmlScene, vtkDataIOManagerLogic *dataIOManagerLogic)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  Slicer3Helper::RemoveDataIOFromScene(mrmlScene,dataIOManagerLogic);

#endif

}
