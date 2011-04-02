#include <iostream>
#include <vector>
#include <string>

#include "vtkMRMLScene.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"

#include <vtksys/SystemTools.hxx>
#include <stdexcept>

#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"
#include "vtkDataIOManagerLogic.h"

#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"

// -============================
// This is necessary to load EMSegment package in TCL interp.
// -============================

#include "vtkSlicerApplication.h"
#include "vtkKWTkUtilities.h"

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Emsegment_Init(Tcl_Interp *interp);
extern "C" int Vtkteem_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);
extern "C" int Mrmlcli_Init(Tcl_Interp *interp); 
extern "C" int Commandlinemodule_Init(Tcl_Interp *interp);
extern "C" int Kwwidgets_Init(Tcl_Interp *interp);
//extern "C" int Atlascreatorcxxmodule_Init(Tcl_Interp *interp);

#define tgVtkCreateMacro(name,type)                                     \
  name  = type::New();                                                  \
  name##Tcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp, name)); 

#define tgVtkDefineMacro(name,type)             \
  type *name;                                   \
  std::string name##Tcl;                        \
  tgVtkCreateMacro(name,type); 

#define tgSetDataMacro(name,matrix)                             \
  virtual int Set##name(const char *fileName) {                 \
    if (strcmp(fileName,"None")) {                              \
      tgVtkCreateMacro(this->name,vtkImageData);                \
      this->matrix = vtkMatrix4x4::New();                       \
      return tgReadVolume(fileName,this->name,this->matrix);    \
    }                                                           \
    this->name = NULL;                                          \
    this->matrix = NULL;                                        \
    std::cout << "Here" << std::endl;                           \
    return 0;                                                   \
  }


vtksys_stl::string tgGetSLICER_HOME(char** argv)  
{ 
  vtksys_stl::string slicerHome = "";
  if ( !vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome) )
    {
      std::string programPath;
      std::string errorMessage;
      if ( !vtksys::SystemTools::FindProgramPath(argv[0], programPath, errorMessage) ) return slicerHome;

      slicerHome = vtksys::SystemTools::GetFilenamePath(programPath.c_str()) + "/../../../";
    } 
  return slicerHome;
}

int tgSetSLICER_HOME(char** argv)  
{ 
  vtksys_stl::string slicerHome = "";
  if ( !vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome) )
    {
      std::string programPath;
      std::string errorMessage;

      if ( !vtksys::SystemTools::FindProgramPath(argv[0], programPath, errorMessage) ) return 1;

      std::string homeEnv = "Slicer3_HOME=";
      homeEnv += vtksys::SystemTools::GetFilenamePath(programPath.c_str()) + "/../../../";
   
      cout << "Set environment: " << homeEnv.c_str() << endl;
      vtkKWApplication::PutEnv(const_cast <char *> (homeEnv.c_str()));
    } else {
    cout << "Slicer3_HOME found: " << slicerHome << endl;
  }
  return 0;
}

Tcl_Interp* CreateTclInterp(int argc, char** argv) 
{
  Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cout);
  if (!interp)
    {
      cout << "Error: InitializeTcl failed" << endl;
      return NULL; 
    }

  // This is necessary to load in EMSEgmenter package in TCL interp.
  Emsegment_Init(interp);
  Slicerbasegui_Init(interp);
  Slicerbaselogic_Init(interp);
  Mrml_Init(interp);
  Mrmlcli_Init(interp); 
  Vtkteem_Init(interp);
  Vtkitk_Init(interp);
  Commandlinemodule_Init(interp);
  Kwwidgets_Init(interp);
  // Atlascreatorcxxmodule_Init(interp);
  return interp;
}

#ifdef _WIN32
std::string StripBackslashes(const std::string& s)
{
  std::string outString;
  for (unsigned int i = 0; i < s.size(); ++i)
    {
      if (s[i] != '\\')
        {
          outString.push_back(s[i]);
        }
      else if (i > 0 && s[i-1] == '\\')
        {
          outString.push_back(s[i]);
        }
    }
  return outString;
}
#endif

vtkSlicerApplicationLogic* InitializeApplication(Tcl_Interp *interp, vtkSlicerApplication *app, int argc, char** argv) 
{
  // SLICER_HOME
  cout << "Setting SLICER home: " << endl;
  vtkstd::string slicerHome = tgGetSLICER_HOME(argv);
  if(!slicerHome.size())
    {
      cout << "Error: Cannot find executable" << endl;
      return NULL;
    }
  cout << "Slicer home is " << slicerHome << endl;

  app->PromptBeforeExitOff();
  std::string appTcl;
  appTcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp, app));;

  app->Script ("namespace eval slicer3 set Application %s", appTcl.c_str());

  tgVtkDefineMacro(appLogic,vtkSlicerApplicationLogic);

  app->Script ("namespace eval slicer3 set ApplicationLogic %s", appLogicTcl.c_str());

  // set BinDir to make functionality like GetSvnRevision available
  std::string slicerBinDir = slicerHome + "/bin";
  slicerBinDir = vtksys::SystemTools::CollapseFullPath(slicerBinDir.c_str());
  app->SetBinDir(slicerBinDir.c_str());

  // Make generic later 
  app->Script("set ::env(KILIS_MODULE) KilisSandbox");
  std::string CMD = std::string("set ::env(SLICER_HOME) ") + slicerHome + "/..";
  app->Script(CMD.c_str());

  CMD = "set argv { "; 
  for (int i = 2 ; i < argc ; i++) CMD += std::string(argv[i]) + " ";
  CMD += " }  "; 
  app->Script(CMD.c_str());
  return  appLogic;
}


void CleanUp(vtkSlicerApplication* app, vtkSlicerApplicationLogic* appLogic  )
{
  if (appLogic)
    {
      appLogic->Delete();
      appLogic = NULL;  
    }

  if (app)
    {
      app->Exit();
      app->Delete();
      app = NULL;
    }
}



void GenerateEmptyMRMLScene(const char* filename)
{
  // create scene, logic, and add parameters to scene
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL(filename);

  vtkEMSegmentLogic* emLogic             = vtkEMSegmentLogic::New();
  emLogic->SetModuleName("EMSegment");
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();

  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

  emLogic->GetMRMLManager()->CreateAndObserveNewParameterSet();

  // write the scene
  try
    {
      mrmlScene->Commit();
    }
  catch (...)
    {
      std::cerr << "ERROR: could not write mrml scene." << std::endl;
    }

  // clean up
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
}


//
// This function checks to see if the image stored in standardFilename
// differs from resultData.  True is returned if the images differ,
// false is returned if they are identical.
bool ImageDiff(vtkImageData* resultData, std::string standardFilename)
{
  bool imagesDiffer = false;

  //
  // read segmentation result standard
  vtkITKArchetypeImageSeriesReader* standardReader =
    vtkITKArchetypeImageSeriesScalarReader::New();
  standardReader->SetArchetype(standardFilename.c_str());
  standardReader->SetOutputScalarTypeToNative();
  standardReader->SetDesiredCoordinateOrientationToNative();
  standardReader->SetUseNativeOriginOn();
  try
    {
      standardReader->Update();
    }
  catch (...)
    {
      std::cerr << "Error: EMSegmenter: Error reading standard image: " << std::endl;
      standardReader->Delete();
      return true;
    }

  //
  // compare image origin and spacing
  for (unsigned int i = 0; i < 3; ++i)
    {
      if (resultData->GetSpacing()[i] !=
          standardReader->GetOutput()->GetSpacing()[i] ||
          resultData->GetOrigin()[i] !=
          standardReader->GetOutput()->GetOrigin()[i])
        {
          //
          // display spacing and origin info for resultData
          std::cerr << "Error: EMSegmenter: Image spacing and/or origin does not match standard!" 
                    << std::endl;
          std::cerr << "Error: EMSegmenter: result origin: " 
                    << resultData->GetOrigin()[0] << " "
                    << resultData->GetOrigin()[1] << " "
                    << resultData->GetOrigin()[2] << std::endl;
          std::cerr << "Error: EMSegmenter: result spacing: " 
                    << resultData->GetSpacing()[0] << " "
                    << resultData->GetSpacing()[1] << " "
                    << resultData->GetSpacing()[2] << std::endl;

          std::cerr << "Error: EMSegmenter: Standard origin: " 
                    << standardReader->GetOutput()->GetOrigin()[0] << " "
                    << standardReader->GetOutput()->GetOrigin()[1] << " "
                    << standardReader->GetOutput()->GetOrigin()[2] << std::endl;
          std::cerr << "Error: EMSegmenter: Standard spacing: " 
                    << standardReader->GetOutput()->GetSpacing()[0] << " "
                    << standardReader->GetOutput()->GetSpacing()[1] << " "
                    << standardReader->GetOutput()->GetSpacing()[2] << std::endl;
          imagesDiffer = true;
        }
    }
  if (!imagesDiffer)
    {
      std::cout << "Result image origin and spacing match." << std::endl;
    }

  //
  // compare image voxels
  vtkImageMathematics* imageDifference = vtkImageMathematics::New();
  imageDifference->SetOperationToSubtract();
  imageDifference->SetInput1(resultData);
  imageDifference->SetInput2(standardReader->GetOutput());

  vtkImageAccumulate* differenceAccumulator = vtkImageAccumulate::New();
  differenceAccumulator->SetInputConnection(imageDifference->GetOutputPort());
  //differenceAccumulator->IgnoreZeroOn();
  differenceAccumulator->Update();

  //imagesDiffer = differenceAccumulator->GetVoxelCount() > 0;
  imagesDiffer =
    differenceAccumulator->GetMin()[0] != 0.0 ||
    differenceAccumulator->GetMax()[0] != 0.0;

  if (imagesDiffer)
    {
      std::cerr << "Error: EMSegmenter: ((temporarily not) ignoring zero) Num / Min / Max / Mean difference = " 
                << differenceAccumulator->GetVoxelCount()  << " / "
                << differenceAccumulator->GetMin()[0]      << " / "
                << differenceAccumulator->GetMax()[0]      << " / "
                << differenceAccumulator->GetMean()[0]     << std::endl;
    }
  else
    {
      std::cout << "Result image voxels match." << std::endl;
    }

  standardReader->Delete();
  imageDifference->Delete();
  differenceAccumulator->Delete();

  return imagesDiffer;
}



// does not actually read an image from disk, this is intended for
// creating an image that you will later want to write to
vtkMRMLVolumeNode*
AddNewScalarArchetypeVolume(vtkMRMLScene* mrmlScene,
                            const char* filename,
                            int centerImage,
                            int labelMap,
                            const char* volname)
{
  vtkMRMLScalarVolumeNode  *scalarNode   = vtkMRMLScalarVolumeNode::New();
  scalarNode->SetLabelMap(labelMap);
  vtkMRMLVolumeNode        *volumeNode   = scalarNode;

  // i/o mechanism
  vtkMRMLVolumeArchetypeStorageNode *storageNode =
    vtkMRMLVolumeArchetypeStorageNode::New();
  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);

  // set the volume's name
  if (volname == NULL)
    {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
    }
  else
    {
      volumeNode->SetName(volname);
    }

  // add nodes to scene
  mrmlScene->AddNodeNoNotify(storageNode);
  mrmlScene->AddNodeNoNotify(volumeNode);

  volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());

  if (scalarNode)
    {
      scalarNode->Delete();
    }
  if (storageNode)
    {
      storageNode->Delete();
    }
  return volumeNode;
}

class ProgressReporter
{
 public:
  void ReportProgress(const std::string& message,
                      float totalProgress = 0.0f,
                      float stageProgress = 0.0f)
  {
    // unused variable
    (void)(message);
    (void)(totalProgress);
    (void)(stageProgress);
  }

};

