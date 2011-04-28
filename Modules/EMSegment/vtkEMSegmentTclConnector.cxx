#include "vtkObjectFactory.h"
#include "vtkEMSegmentTclConnector.h"

#include "vtkMRMLEMSGlobalParametersNode.h"

//----------------------------------------------------------------------------
vtkEMSegmentTclConnector* vtkEMSegmentTclConnector::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkEMSegmentTclConnector");
  if(ret)
    {
    return (vtkEMSegmentTclConnector*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentTclConnector;
}


//----------------------------------------------------------------------------
vtkEMSegmentTclConnector::vtkEMSegmentTclConnector() {
    this->SlicerApp =NULL;
    this->EMSLogic = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentTclConnector::~vtkEMSegmentTclConnector() {
  this->SetSlicerApp(NULL);
  this->SetEMSLogic(NULL);
}


//----------------------------------------------------------------------------
// New Task Specific Pipeline
//----------------------------------------------------------------------------

int vtkEMSegmentTclConnector::SourceTclFile(const char *tclFile)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3

  // Load Tcl File defining the setting
  if (!this->SlicerApp->LoadScript(tclFile))
    {
      vtkErrorMacro("Could not load in data for task. The following file does not exist: " << tclFile);
      return 1;
    }

#endif

  return 0;

}

//----------------------------------------------------------------------------
int vtkEMSegmentTclConnector::SourceTaskFiles() {
  vtkstd::string generalFile = this->DefineTclTaskFullPathName(vtkMRMLEMSGlobalParametersNode::GetDefaultTaskTclFileName());
  vtkstd::string specificFile = this->DefineTclTaskFileFromMRML();
  cout << "Sourcing general Task file : " << generalFile.c_str() << endl;
  // Have to first source the default file to set up the basic structure"
  if (this->SourceTclFile(generalFile.c_str()))
    {
      return 1;
    }
  // Now we overwrite anything from the default
  if (specificFile.compare(generalFile))
    {
      cout << "Sourcing task specific file: " <<   specificFile << endl;
      return this->SourceTclFile(specificFile.c_str());
    }
  return 0;
}


//----------------------------------------------------------------------------
std::string vtkEMSegmentTclConnector::DefineTclTaskFullPathName(const char* TclFileName)
{

//  std::string task_dir = this->GetTclTaskDirectory(app);
//  cout << "TEST 1" << task_dir << " " << vtksys::SystemTools::FileExists(task_dir.c_str()) << endl;

  vtkstd::string tmp_full_file_path = this->GetTclTaskDirectory() + vtkstd::string("/") + vtkstd::string(TclFileName);
//  vtkstd::string full_file_path = vtksys::SystemTools::ConvertToOutputPath(tmp_full_file_path.c_str());
  if (vtksys::SystemTools::FileExists(tmp_full_file_path.c_str()))
    {
      return tmp_full_file_path;
    }

  tmp_full_file_path = this->GetTemporaryTaskDirectory() + vtkstd::string("/") + vtkstd::string(TclFileName);
//  full_file_path = vtksys::SystemTools::ConvertToOutputPath(tmp_full_file_path.c_str());
  if (vtksys::SystemTools::FileExists(tmp_full_file_path.c_str()))
    {
       return tmp_full_file_path;
    }

  vtkErrorMacro("DefineTclTaskFullPathName : could not find tcl file with name  " << TclFileName );
  tmp_full_file_path = vtkstd::string("");
  return  tmp_full_file_path;
}

//----------------------------------------------------------------------------
std::string vtkEMSegmentTclConnector::DefineTclTaskFileFromMRML()
{
  std::string tclFile("");
  tclFile = this->DefineTclTaskFullPathName(this->EMSLogic->GetMRMLManager()->GetTclTaskFilename());

  if (vtksys::SystemTools::FileExists(tclFile.c_str()) && (!vtksys::SystemTools::FileIsDirectory(tclFile.c_str())) )
    {
      return tclFile;
    }

  cout << "vtkEMSegmentKWLogic::DefineTclTaskFileFromMRML: " << tclFile.c_str() << " does not exist - using default file" << endl;

  tclFile = this->DefineTclTaskFullPathName(vtkMRMLEMSGlobalParametersNode::GetDefaultTaskTclFileName());
  return tclFile;
}


//----------------------------------------------------------------------------
vtkstd::string  vtkEMSegmentTclConnector::GetTclTaskDirectory()
{
  //workaround for the mrml library, we need to have write access to this folder
  const char* tmp_dir = "";

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  tmp_dir = this->SlicerApp->GetTemporaryDirectory();

  //#else

  // TODO Slicer4

#endif

  if (tmp_dir)
    {
      vtkstd::string copied_task_dir(std::string(tmp_dir) + std::string("/EMSegmentTaskCopy"));

      /**
        * Copy content directory to another directory with all files and
        * sub-directories.  If the "always" argument is true all files are
        * always copied.  If it is false, only files that have changed or
        * are new are copied.
        */
       // copy not always, only new files
       // Later do automatically
      vtkstd::string orig_task_dir = "";

#ifdef Slicer3_USE_KWWIDGETS

      // Slicer3
      orig_task_dir = this->EMSLogic->GetModuleShareDirectory() + vtkstd::string("/Tasks");

//#else

      // TODO Slicer4

#endif

      if ( !vtksys::SystemTools::CopyADirectory(orig_task_dir.c_str(), copied_task_dir.c_str(), false) )
      {
          cout << "GetTclTaskDirectory:: Couldn't copy task directory " << orig_task_dir.c_str() << " to " << copied_task_dir.c_str() << endl;
          vtkErrorMacro("GetTclTaskDirectory:: Couldn't copy task directory " << orig_task_dir.c_str() << " to " << copied_task_dir.c_str());
          return vtksys::SystemTools::ConvertToOutputPath("");
      }
      return copied_task_dir;
    }
  else
    {
      // FIXME, make sure there is always a valid temporary directory
      vtkErrorMacro("GetTclTaskDirectory:: Tcl Task Directory was not found, set temporary directory first");
    }

  // return empty string if not found
  return vtksys::SystemTools::ConvertToOutputPath("");

}


//----------------------------------------------------------------------------
// This function is used for the UpdateButton in vtkEMSegmentParametersSetStep
vtkstd::string vtkEMSegmentTclConnector::GetTemporaryTaskDirectory()
{
  // FIXME, what happens if user has no write permission to this directory
  std::string taskDir("");
  if (!this->SlicerApp)
    {
      return taskDir;
    }

  const char* tmp_dir = "";

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  tmp_dir = this->SlicerApp->GetTemporaryDirectory();

  //#else

  // TODO Slicer4

#endif


  const char* svn_revision = "";


#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  svn_revision = this->SlicerApp->GetSvnRevision();

  //#else

  // TODO Slicer4

#endif



  if (tmp_dir)
    {
      std::string tmpTaskDir( std::string(tmp_dir) + "/" + std::string(svn_revision) + std::string("/EMSegmentTask") );
      taskDir = vtksys::SystemTools::ConvertToOutputPath(tmpTaskDir.c_str());
    }
  else
    {
      // FIXME, make sure there is always a valid temporary directory
      vtkErrorMacro("GetTemporaryTaskDirectory:: Temporary Directory was not defined");
    }
  return taskDir;
}
