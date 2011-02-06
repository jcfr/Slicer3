#include <iostream>
#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentMRMLManager.h"
#include "EMSegmentTransformToNewFormatCLP.h"
#include "vtkSlicerApplication.h"
#include "vtkDataIOManagerLogic.h"
#include "vtkSlicerColorLogic.h"
#include "vtkEMSegmentLogic.h"
#include "vtkKWTkUtilities.h"

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);
extern "C" int Mrmlcli_Init(Tcl_Interp *interp); 


int main(int argc, char** argv)
{
   // parse arguments using the CLP system; this creates variables.
   PARSE_ARGS;

  // make sure arguments are sufficient and unique
  bool argsOK = true;
  if (inputMRMLFileName.empty() || !vtksys::SystemTools::FileExists(inputMRMLFileName.c_str()) )
    {
    std::cerr << "Error: inputMRMLFileName must be specified or file does not exists." << std::endl;
    argsOK = false;
    }

   if (outputMRMLFileName.empty())
    {
    std::cerr << "Error: outputMRMLFileName must be specified." << std::endl;
    argsOK = false;
    }

   if (vtksys::SystemTools::GetParentDirectory(inputMRMLFileName.c_str()) != vtksys::SystemTools::GetParentDirectory(outputMRMLFileName.c_str()) ) 
     {
    std::cerr << "Error: inputMRMLFileName and outputMRMLFileName must be in the same directory." << std::endl;
    argsOK = false;
     } 

  if (!argsOK)
    {
    
    std::cerr << "Try --help for usage..." << std::endl;
    return EXIT_FAILURE;
    }
 
    // =======================================================================
    //
    //  Initialize 
    // 
    // =======================================================================

     Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cout);
     if (!interp)
       {
        cout << "Error: InitializeTcl failed" << endl;
        return EXIT_FAILURE; 
       }

     Slicerbasegui_Init(interp);
     Slicerbaselogic_Init(interp);
     Mrml_Init(interp);
     Mrmlcli_Init(interp); 


     vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
     app->PromptBeforeExitOff();
     std::string appTcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp, app));;
     app->Script ("namespace eval slicer3 set Application %s", appTcl.c_str());

     vtkSlicerApplicationLogic* appLogic = vtkSlicerApplicationLogic::New();     
     vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
     vtkMRMLScene::SetActiveScene(mrmlScene);
     appLogic->SetAndObserveMRMLScene(mrmlScene);
     std::string appLogicTcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp, appLogic));;
     app->Script ("namespace eval slicer3 set ApplicationLogic %s", appLogicTcl.c_str());
 
  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
    colorLogic->SetMRMLScene(mrmlScene);
    colorLogic->AddDefaultColorNodes();
    colorLogic->SetMRMLScene(NULL);
    colorLogic->Delete();

    vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
    emLogic->SetModuleName("EMSegment");
    emLogic->SetAndObserveMRMLScene(mrmlScene);
    emLogic->RegisterMRMLNodesWithScene();

    vtkIntArray *emsEvents                 = vtkIntArray::New();
    emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
    emsEvents->Delete();

    vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();
    emLogic->AddDataIOToScene(mrmlScene,app,appLogic,dataIOManagerLogic);
    vtkEMSegmentMRMLManager* emMRMLManager = emLogic->GetMRMLManager();

    // =======================================================================
    //
    //  Start Processing 
    // 
    // =======================================================================
    bool success = true;
     try 
     {
        try 
        {
          //
          // read the mrml scene
           mrmlScene->SetURL(inputMRMLFileName.c_str());
           std::cout << "Reading MRML scene..." << endl;
           // 
           mrmlScene->Connect();
           std::cout << "DONE" << std::endl;
        }
        catch (...)
        {
          throw std::runtime_error("ERROR: failed to import mrml scene.");
        }

        //
        //  Transform to new style
        // 
        int numParameterSets = emMRMLManager->GetNumberOfParameterSets();
        std::cerr << "Imported: " << mrmlScene->GetNumberOfNodes()
                           << (mrmlScene->GetNumberOfNodes() == 1 
                               ? " node" : " nodes")
                           << ", including " << numParameterSets 
                           << " EM parameter "
                           << (numParameterSets == 1 ? "node." : "nodes.")
                           << std::endl;
  
        // make sure there is at least one parameter set
        if (numParameterSets < 1)
        {
      throw std::runtime_error("ERROR: no EMSegment parameter nodes in scene.");
        }
        
        // Look at all path if they are correct

        //
        // Write results back
        // 
        if (taskFlag)
      {
        // As a Template
              std::cout << "Using default parameter set named: " 
                                      << emMRMLManager->GetNthParameterSetName(0) 
                                      << std::endl;    

              //
              // populate the mrmlManager with the parameters
              try
              {
                 emMRMLManager->SetLoadedParameterSetIndex(0);
              }
              catch (...)
              {
                  throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
              }

              emMRMLManager->SetSaveTemplateFilename(outputMRMLFileName.c_str());
              emMRMLManager->CreateTemplateFile();
      }
    else 
      {
        // Just save it back 
             mrmlScene->Commit(outputMRMLFileName.c_str());
      }
        //
        // Done 
    //
     }
  catch (...)
  {
    std::cerr << "Error: " << std::endl;
    success = false;
  }

  //
  // clean up
  //
  std::cout << "Cleaning up...";

  appLogic->Delete();
  appLogic = NULL;  

  app->Exit();
  app->Delete();
  app = NULL;

  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->RemoveDataIOFromScene(mrmlScene,dataIOManagerLogic);
  emLogic->Delete();

  dataIOManagerLogic->Delete();
  dataIOManagerLogic = NULL;

  mrmlScene->Clear(true);
  mrmlScene->Delete();
  std::cout << "DONE" << std::endl;
  
  return success ? EXIT_SUCCESS : EXIT_FAILURE;    
}

