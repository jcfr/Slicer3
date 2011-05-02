#include "vtkObjectFactory.h"
#include "vtkEMSegmentKWLogic.h"
#include "vtkEMSegmentMRMLManager.h"
#include "vtkSlicerApplication.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkImageEMLocalSegmenter.h"
#include "vtkKWTkUtilities.h"
#include "vtkDataIOManagerLogic.h"
#include "../../Applications/GUI/Slicer3Helper.h"




//----------------------------------------------------------------------------
vtkEMSegmentKWLogic* vtkEMSegmentKWLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkEMSegmentKWLogic");
  if(ret)
    {
    return (vtkEMSegmentKWLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentKWLogic;
}


//----------------------------------------------------------------------------
vtkEMSegmentKWLogic::vtkEMSegmentKWLogic() { 
    this->SlicerApp =NULL;
    this->EMSLogic = NULL;

    this->TclConnector = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentKWLogic::~vtkEMSegmentKWLogic() { 
  this->SetSlicerApp(NULL);
  this->SetEMSLogic(NULL);

  if (this->TclConnector)
    {
    this->TclConnector->Delete();
    this->TclConnector = NULL;
    }
}

//----------------------------------------------------------------------------
vtkEMSegmentTclConnector* vtkEMSegmentKWLogic::GetTclConnector()
{

  if (!this->TclConnector)
    {
    this->TclConnector = vtkEMSegmentTclConnector::New();
    this->TclConnector->SetSlicerApp(this->SlicerApp);
    this->TclConnector->SetEMSLogic(this->GetEMSLogic());
    }

  return this->TclConnector;

}

//----------------------------------------------------------------------------
bool
vtkEMSegmentKWLogic::SaveIntermediateResults(vtkSlicerApplicationLogic *appLogic)
{
  return this->GetEMSLogic()->SaveIntermediateResults(appLogic);
}

//----------------------------------------------------------------------------
// New Task Specific Pipeline
//----------------------------------------------------------------------------

int vtkEMSegmentKWLogic::SourceTclFile(const char *tclFile)
{
  return this->GetTclConnector()->SourceTclFile(tclFile);
}

//----------------------------------------------------------------------------
int vtkEMSegmentKWLogic::SourceTaskFiles()
{
  return this->GetTclConnector()->SourceTaskFiles();
}

//----------------------------------------------------------------------------  
int vtkEMSegmentKWLogic::SourcePreprocessingTclFiles() 
{
  if (this->SourceTaskFiles())
    {
      return 1;
    }
   // Source all files here as we otherwise sometimes do not find the function as Tcl did not finish sourcing but our cxx file is already trying to call the function 
   vtkstd::string tclFile =  this->EMSLogic->GetModuleShareDirectory();
#ifdef _WIN32
   tclFile.append("\\Tcl\\EMSegmentAutoSample.tcl");
#else
   tclFile.append("/Tcl/EMSegmentAutoSample.tcl");
#endif
   return this->SourceTclFile(tclFile.c_str());
}

//----------------------------------------------------------------------------
int vtkEMSegmentKWLogic::StartSegmentationWithoutPreprocessing(vtkSlicerApplicationLogic *appLogic)
{
  int flag = this->EMSLogic->StartSegmentationWithoutPreprocessingAndSaving() ;
  ErrorMsg = this->EMSLogic->GetErrorMessage();
  if (flag == EXIT_FAILURE)
    {
         return EXIT_FAILURE;
    }

  //
  // save intermediate results
  if (this->EMSLogic->GetMRMLManager()->GetSaveIntermediateResults())
    {
    vtkstd::cout << "[Start] Saving intermediate results..." << vtkstd::endl;
    bool savedResults = this->SaveIntermediateResults(appLogic);
    vtkstd::cout << "[Done]  Saving intermediate results." << vtkstd::endl;
    if (!savedResults)
      {
    std::string msg = "Error writing intermediate results"; 
        ErrorMsg += msg + "\n";
        vtkErrorMacro( << msg);
        return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
vtkstd::string  vtkEMSegmentKWLogic::GetTclTaskDirectory()
{
  return this->GetTclConnector()->GetTclTaskDirectory();
}

//----------------------------------------------------------------------------
std::string vtkEMSegmentKWLogic::DefineTclTaskFileFromMRML()
{
  return this->GetTclConnector()->DefineTclTaskFileFromMRML();
}

//----------------------------------------------------------------------------
// Make sure you source EMSegmentAutoSample.tcl
int vtkEMSegmentKWLogic::ComputeIntensityDistributionsFromSpatialPrior()
{
  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;

  this->EMSLogic->GetMRMLManager()->GetListOfTreeNodeIDs(this->EMSLogic->GetMRMLManager()->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
      if (this->EMSLogic->GetMRMLManager()->GetTreeNodeIsLeaf(*i)) 
        {      
      this->UpdateIntensityDistributionAuto(*i);
        }
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkEMSegmentKWLogic::UpdateIntensityDistributionAuto(vtkIdType nodeID)
{

  if (!this->EMSLogic->GetMRMLManager()->GetTreeNodeSpatialPriorVolumeID(nodeID)) {
    vtkWarningMacro("Nothing to update for " << nodeID << " as atlas is not defined");
    return ;
  }

  vtkMRMLVolumeNode*  atlasNode = this->EMSLogic->GetMRMLManager()->GetAlignedSpatialPriorFromTreeNodeID(nodeID);
  if (!atlasNode)  {
    vtkErrorMacro("No aligned atlas defined for node with ID" << nodeID << " and name " << this->EMSLogic->GetMRMLManager()->GetTreeNodeName(nodeID) << "! ");
    return ;
  }

  if (!this->EMSLogic->GetMRMLManager()->GetTreeNodeSpatialPriorVolumeID(nodeID)) 
  {
    vtkErrorMacro("No atlas defined for node " << nodeID << " ! ");
    return ;
  }

  // get working node 
  vtkMRMLEMSVolumeCollectionNode* workingTarget = NULL;
  if (this->EMSLogic->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNode() &&
      this->EMSLogic->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNodeIsValid())
    {
    workingTarget = this->EMSLogic->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNode();
    }
  else 
    {
       vtkErrorMacro("Cannot update intensity distribution bc Aligned Target is not correctly defined for node " << nodeID);
       return ;
    }

  int numTargetImages = workingTarget->GetNumberOfVolumes();
  
   // Sample
  {
    vtkstd::stringstream CMD ;
    CMD <<  "::EMSegmenterAutoSampleTcl::EMSegmentGaussCurveCalculationFromID " << vtkKWTkUtilities::GetTclNameFromPointer(this->SlicerApp->GetMainInterp(),this) << " 0.95 1 { " ;
    for (int i = 0 ; i < numTargetImages; i++) {
      CMD << workingTarget->GetNthVolumeNodeID(i) << " " ;
    }
    CMD << " } ";
    CMD << atlasNode->GetID() << " {" <<  this->EMSLogic->GetMRMLManager()->GetTreeNodeName(nodeID) << "} \n";
    // cout << CMD.str().c_str() << endl;
    if (atoi(this->SlicerApp->Script(CMD.str().c_str()))) { return; }
  }
  

  //
  // propagate data to mrml node
  //

  vtkMRMLEMSTreeParametersLeafNode* leafNode = this->EMSLogic->GetMRMLManager()->GetTreeParametersLeafNode(nodeID);  
  for (int r = 0; r < numTargetImages; ++r)
    {
      {
        double value = atof(this->SlicerApp->Script("expr $::EMSegment(GaussCurveCalc,Mean,%d)",r));
        leafNode->SetLogMean(r, value);
      }
      for (int c = 0; c < numTargetImages; ++c)
      {
        double value = atof(this->SlicerApp->Script("expr $::EMSegment(GaussCurveCalc,Covariance,%d,%d)",r,c));
        leafNode->SetLogCovariance(r, c, value);
      }
    }
}


//----------------------------------------------------------------------------
// This function is used for the UpdateButton in vtkEMSegmentParametersSetStep
vtkstd::string vtkEMSegmentKWLogic::GetTemporaryTaskDirectory()
{
  return this->GetTclConnector()->GetTemporaryTaskDirectory();
} 

//----------------------------------------------------------------------------
// cannot be moved to vtkEMSEgmentGUI bc of command line interface !
std::string vtkEMSegmentKWLogic::DefineTclTaskFullPathName(const char* TclFileName)
{
  return this->GetTclConnector()->DefineTclTaskFullPathName(TclFileName);
}


//----------------------------------------------------------------------------
// Updates the .tcl Tasks from an external website and replaces the content
// in $tmpDir/EMSegmentTask (e.g. /home/Slicer3USER/EMSegmentTask)
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int vtkEMSegmentKWLogic::UpdateTasks()
{

  { // we want our own scope in this one :D
  
  //
  // ** THE URL **
  //
  // the url to the EMSegment task repository
  //std::string taskRepository = "http://people.csail.mit.edu/pohl/EMSegmentUpdates/";
  std::string taskRepository = "http://slicer.org/EMSegmentUpdates/3.6.3/";

  //
  // ** PATH MANAGEMENT **
  //
  // we need the slicer application to query the $tmp_dir path
  const char* tmpDir = this->SlicerApp->GetTemporaryDirectory();
  if (!tmpDir)
    {
      vtkErrorMacro("UpdateTasksCallback: Temporary directory is not defined!");
      return 0;
    }
  // also add the manifest filename
  std::string tmpManifestFilename( std::string(tmpDir) + "/" + std::string(this->SlicerApp->GetSvnRevision()) + std::string("/EMSegmentTasksManifest.html") );
  std::string manifestFilename = vtksys::SystemTools::ConvertToOutputPath(tmpManifestFilename.c_str());

  // and add the EMSegmentTask directory
  std::string taskDir = this->GetTemporaryTaskDirectory();
  //
  // ** HTTP ACCESS **
  //
  // our HTTP handler
  //vtkHTTPHandler* httpHandler = vtkHTTPHandler::New();
  vtkHTTPHandler* httpHandler = vtkHTTPHandler::SafeDownCast(this->SlicerApp->GetMRMLScene()->FindURIHandlerByName("HTTPHandler"));


  // prevent funny behavior on windows with the side-effect of more network resources are used
  // (o_o) who cares about traffic or the tcp/ip ports? *g
  httpHandler->SetForbidReuse(1);

  // safe-check if the handler can really handle the hardcoded uri protocol
  if (!httpHandler->CanHandleURI(taskRepository.c_str()))
    {
    vtkErrorMacro("UpdateTasksCallback: Invalid URI specified and you can't do anything about it bcuz it is *hardcoded*!")
    return 0;
    }

  //
  // ** THE ACTION STARTS **
  //
  // make sure we can access the task repository
  // TODO: this function will be provided by Wendy sooner or later :D
  //       for now, we just assume that we are on-line!
  

  // get the directory listing of the EMSegment task repository and save it as $tmpDir/EMSegmentTasksManifest.html.
  // the manifest always gets overwritten, but nobody should care
  httpHandler->StageFileRead(taskRepository.c_str(),manifestFilename.c_str());
  
  // sanity checks: if manifestFilename does not exist or size<1, exit here before it is too late!
  if (!vtksys::SystemTools::FileExists(manifestFilename.c_str()) || vtksys::SystemTools::FileLength(manifestFilename.c_str())<1)
    {
    vtkErrorMacro("UpdateTasksCallback: Could not get the manifest! Try again later..")
    return 0;
    }
  
  
  // what happens now? answer: a three-step-processing chain!!!
  // (1) now we got the manifest and we can parse it for filenames of EMSegment tasks.
  // (2) then, download these files and copy them to our $tmpDir.
  //     after we are sure we got the files (we can not be really sure but we can check if some files where downloaded), 
  // (3) we delete all old files in $taskDir and then copy our newly downloaded EMSegment tasks from $tmpDir to $taskDir.
  // sounds good!
  
  // 1) open the manifest and get the filenames
  std::ifstream fileStream(manifestFilename.c_str());
  std::string htmlManifestAsString;
  if (!fileStream.fail())
    {
    fileStream.seekg(0,std::ios::end);
    size_t length = fileStream.tellg();
    fileStream.seekg(0,std::ios::beg);
    char* htmlManifest = new char[length+1];
    fileStream.read(htmlManifest, length);
    htmlManifest[length] = '\n';
    htmlManifestAsString = std::string(htmlManifest);
    delete[] htmlManifest;
    }
    
  fileStream.close();
    
  // when C++0x is released, we could easily do something like this to filter out the .tcl and .mrml filenames:
  //  cmatch regexResult;
  //  regex tclExpression("(\w*-*)+.tcl(?!\")");  
  //  regex_search(htmlManifest, regexResult, tclExpression); 
  //  regex mrmlExpression("(\w*-*)+.mrml(?!\")");  
  //  regex_search(htmlManifest, regexResult, mrmlExpression);   
  // but right now, we have to manually parse the string.
  // at least we can use std::string methods :D
  //
  // Fix for recent webservers does not include a space after HTML tags
  std::string beginTaskFilenameTag(".tcl\">");
  std::string endTaskFilenameTag(".tcl</a>");
  std::string beginMrmlFilenameTag(".mrml\">");
  std::string endMrmlFilenameTag(".mrml</a>");  
  
  bool tclFilesExist = false;
  bool mrmlFilesExist = false;
  
  std::vector<std::string> taskFilenames;
  std::vector<std::string> mrmlFilenames;

  std::string::size_type beginTaskFilenameIndex = htmlManifestAsString.find(beginTaskFilenameTag,0);
  
  // the loop for .tcl files
  while(beginTaskFilenameIndex!=std::string::npos)
    {
    // as long as we find the beginning of a filename, do the following..
    
    // find the corresponding end
    std::string::size_type endTaskFilenameIndex = htmlManifestAsString.find(endTaskFilenameTag,beginTaskFilenameIndex);
  
    if (endTaskFilenameIndex==std::string::npos)
      {
      vtkErrorMacro("UpdateTasksCallback: Error during parsing! There was no end *AAAAAAAAAAAAAAAAAAAAHHHH*")
      return 0;
      }
    
    // now get the string between begin and end, then add it to the vector
    taskFilenames.push_back(htmlManifestAsString.substr(beginTaskFilenameIndex+beginTaskFilenameTag.size(),endTaskFilenameIndex-(beginTaskFilenameIndex+beginTaskFilenameTag.size())));
    
    // and try to find the next beginTag
    beginTaskFilenameIndex = htmlManifestAsString.find(beginTaskFilenameTag,endTaskFilenameIndex);
    }
    
  // enable copying of .tcl files if they exist
  if (taskFilenames.size()!=0)
    {
    tclFilesExist = true;
    }
    
  std::string::size_type beginMrmlFilenameIndex = htmlManifestAsString.find(beginMrmlFilenameTag,0);
  
  // the loop for .mrml files
  while(beginMrmlFilenameIndex!=std::string::npos)
    {
    // as long as we find the beginning of a filename, do the following..
    
    // find the corresponding end
    std::string::size_type endMrmlFilenameIndex = htmlManifestAsString.find(endMrmlFilenameTag,beginMrmlFilenameIndex);
  
    if (endMrmlFilenameIndex==std::string::npos)
      {
      vtkErrorMacro("UpdateTasksCallback: Error during parsing! There was no end *AAAAAAAAAAAAAAAAAAAAHHHH*")
      return 0;
      }
    
    // now get the string between begin and end, then add it to the vector
    mrmlFilenames.push_back(htmlManifestAsString.substr(beginMrmlFilenameIndex+beginMrmlFilenameTag.size(),endMrmlFilenameIndex-(beginMrmlFilenameIndex+beginMrmlFilenameTag.size())));
    
    // and try to find the next beginTag
    beginMrmlFilenameIndex = htmlManifestAsString.find(beginMrmlFilenameTag,endMrmlFilenameIndex);
    }
    
  // enable copying of .mrml files if they exist
  if (mrmlFilenames.size()!=0)
    {
    mrmlFilesExist = true;
    }    
    
  // 2) loop through the vector and download the task files and the mrml files to the $tmpDir
  std::string currentTaskUrl;
  std::string currentTaskName;
  std::string currentTaskFilepath;

  std::string currentMrmlUrl;
  std::string currentMrmlName;
  std::string currentMrmlFilepath;
  
  if (tclFilesExist)
    {
    // loop for .tcl
    for (std::vector<std::string>::iterator i = taskFilenames.begin(); i != taskFilenames.end(); ++i)
      {
      
      currentTaskName = *i;
      
      // sanity checks: if the filename is "", exit here before it is too late!
      if (!strcmp(currentTaskName.c_str(),""))
        {
        vtkErrorMacro("UpdateTasksCallback: At least one filename was empty, get outta here NOW! *AAAAAAAAAAAAAAAAAHHH*")
          return 0;
        }
      
      // generate the url of this task
      currentTaskUrl = std::string(taskRepository + currentTaskName + std::string(".tcl"));
      
      // generate the destination filename of this task in $tmpDir
      currentTaskFilepath = std::string(tmpDir + std::string("/") + currentTaskName + std::string(".tcl"));
      
      // and get the content and save it to $tmpDir
      httpHandler->StageFileRead(currentTaskUrl.c_str(),currentTaskFilepath.c_str());
      
      // sanity checks: if the downloaded file does not exist or size<1, exit here before it is too late!
      if (!vtksys::SystemTools::FileExists(currentTaskFilepath.c_str()) || vtksys::SystemTools::FileLength(currentTaskFilepath.c_str())<1)
        {
        vtkErrorMacro("UpdateTasksCallback: At least one file was not downloaded correctly! Aborting.. *beepbeepbeep*")
          return 0;
        }
     
      }
    }
  
  if (mrmlFilesExist)
    {  
    // loop for .mrml
    for (std::vector<std::string>::iterator i = mrmlFilenames.begin(); i != mrmlFilenames.end(); ++i)
      {
      
      currentMrmlName = *i;
      
      // sanity checks: if the filename is "", exit here before it is too late!
      if (!strcmp(currentMrmlName.c_str(),""))
        {
        vtkErrorMacro("UpdateTasksCallback: At least one filename was empty, get outta here NOW! *AAAAAAAAAAAAAAAAAHHH*")
          return 0;
        }
      
      // generate the url of this mrml file
      currentMrmlUrl = std::string(taskRepository + currentMrmlName + std::string(".mrml"));
      
      // generate the destination filename of this task in $tmpDir
      currentMrmlFilepath = std::string(tmpDir + std::string("/") + currentMrmlName + std::string(".mrml"));
      
      // and get the content and save it to $tmpDir
      httpHandler->StageFileRead(currentMrmlUrl.c_str(),currentMrmlFilepath.c_str());
      
      // sanity checks: if the downloaded file does not exist or size<1, exit here before it is too late!
      if (!vtksys::SystemTools::FileExists(currentMrmlFilepath.c_str()) || vtksys::SystemTools::FileLength(currentMrmlFilepath.c_str())<1)
        {
        vtkErrorMacro("UpdateTasksCallback: At least one file was not downloaded correctly! Aborting.. *beepbeepbeep*")
         return 0;
        }
     
      }    
    }

  // we got the .tcl files and the .mrml files now at a safe location and they have at least some content :P
  // this makes it safe to delete all old EMSegment tasks and activate the new one :D
  
  // OMG did you realize that this is a kind of backdoor to take over your home directory?? the
  // downloaded .tcl files get sourced later and can do whatever they want to do!! but pssst let's keep it a secret
  // option for a Slicer backdoor :) on the other hand, the EMSegment tasks repository will be monitored closely and is not
  // public, but what happens if someone changes the URL to the repository *evilgrin*
  
  // 3) copy the $taskDir to a backup folder, delete the $taskDir. and create it again. then, move our downloaded files to it
  
  // purge, NOW!! but only if the $taskDir exists..
  if (vtksys::SystemTools::FileExists(taskDir.c_str()))
  {
    // create a backup of the old taskDir
    std::string backupTaskDir(taskDir + std::string("_old"));
    if (!vtksys::SystemTools::CopyADirectory(taskDir.c_str(),backupTaskDir.c_str()))
      {
      vtkErrorMacro("UpdateTasksCallback: Could not create backup " << backupTaskDir.c_str() << "! This is very bad, we abort the update..")
       return 0;
      }
    
    if (!vtksys::SystemTools::RemoveADirectory(taskDir.c_str()))
      {
      vtkErrorMacro("UpdateTasksCallback: Could not delete " << taskDir.c_str() << "! This is very bad, we abort the update..")
       return 0 ;
      }
  }
  
  // check if the taskDir is gone now!
  if (!vtksys::SystemTools::FileExists(taskDir.c_str()))
    {
    // the $taskDir does not exist, so create it
    bool couldCreateTaskDir = vtksys::SystemTools::MakeDirectory(taskDir.c_str());

    // sanity checks: if the directory could not be created, something is wrong!
    if (!couldCreateTaskDir)
      {
      vtkErrorMacro("UpdateTasksCallback: Could not (re-)create the EMSegmentTask directory: " << taskDir.c_str())
      return 0;
      }
    }
    
  std::string currentTaskDestinationFilepath;
  std::string currentMrmlDestinationFilepath;
  
  if (tclFilesExist)
    {
    // now move the downloaded .tcl files to the $taskDir
    for (std::vector<std::string>::iterator i = taskFilenames.begin(); i != taskFilenames.end(); ++i)
      {
      
      currentTaskName = *i;    
      
      // generate the destination filename of this task in $tmpDir
      currentTaskFilepath = std::string(tmpDir + std::string("/") + currentTaskName + std::string(".tcl"));
      
      // generate the destination filename of this task in $taskDir
      currentTaskDestinationFilepath = std::string(taskDir + std::string("/") + currentTaskName + std::string(".tcl"));    
      
      if (!vtksys::SystemTools::CopyFileAlways(currentTaskFilepath.c_str(),currentTaskDestinationFilepath.c_str()))
        {
        vtkErrorMacro("UpdateTasksCallback: Could not copy at least one downloaded task file. Everything is lost now! Sorry :( Just kidding: there was a backup in " << taskDir << "!")
          return 0;
        }
      }
    }
    
  if (mrmlFilesExist)
    {
    // now move the downloaded .mrml files to the $taskDir
    for (std::vector<std::string>::iterator i = mrmlFilenames.begin(); i != mrmlFilenames.end(); ++i)
      {
      
      currentMrmlName = *i;    
      
      // generate the destination filename of this task in $tmpDir
      currentMrmlFilepath = std::string(tmpDir + std::string("/") + currentMrmlName + std::string(".mrml"));
      
      // generate the destination filename of this task in $taskDir
      currentMrmlDestinationFilepath = std::string(taskDir + std::string("/") + currentMrmlName + std::string(".mrml"));    
      
      if (!vtksys::SystemTools::CopyFileAlways(currentMrmlFilepath.c_str(),currentMrmlDestinationFilepath.c_str()))
        {
        vtkErrorMacro("UpdateTasksCallback: Could not copy at least one downloaded mrml file. Everything is lost now! Sorry :( Just kidding: there was a backup in " << taskDir << "!")
           return 0;
        }
      }
    }
    
 
  //
  // ** ALL DONE, NOW CLEANUP **
  //
  
  } // now go for destruction, donkey!!

  return 1;  
}

//-----------------------------------------------------------------------------
void vtkEMSegmentKWLogic::CreateDefaultTasksList(std::vector<std::string> & DefaultTasksName,  std::vector<std::string> & DefaultTasksFile, 
                                                                                                                         std::vector<std::string> & DefinePreprocessingTasksName, std::vector<std::string> &  DefinePreprocessingTasksFile)
{
  DefaultTasksName.clear();
  DefaultTasksFile.clear();
  DefinePreprocessingTasksName.clear();
  DefinePreprocessingTasksFile.clear();

  this->EMSLogic->AddDefaultTasksToList(this->GetTclTaskDirectory().c_str(), DefaultTasksName,DefaultTasksFile, DefinePreprocessingTasksName, DefinePreprocessingTasksFile);
  this->EMSLogic->AddDefaultTasksToList(this->GetTemporaryTaskDirectory().c_str(), DefaultTasksName,DefaultTasksFile, DefinePreprocessingTasksName, DefinePreprocessingTasksFile);
}

