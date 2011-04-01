#include <iostream>
#include <vector>
#include <string>

#include "vtkMRMLScene.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"

#include "EMSegmentCommandLineCLP.h"

#include <vtksys/SystemTools.hxx>
#include <stdexcept>

#include "../../../Applications/GUI/Slicer3Helper.cxx"
#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"
#include "vtkDataIOManagerLogic.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkMRMLEMSAtlasNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkMRMLEMSTemplateNode.h"

// -============================
// This is necessary to load EMSegment package in TCL interp.
// -============================

#include "vtkSlicerApplication.h"
#include "vtkKWTkUtilities.h"
#include "EMSegmentHelper.h"


int main(int argc, char** argv)
{
  // parse arguments using the CLP system; this creates variables.
  PARSE_ARGS;

  ProgressReporter progressReporter;
  float currentStep = 0.0f;
  float totalSteps  = 6.0f;

  progressReporter.ReportProgress("Parsing Arguments...",
                                   currentStep++ / totalSteps);

  bool useDefaultParametersNode = parametersMRMLNodeName.empty();
  bool useDefaultTarget         = targetVolumeFileNames.empty();
  bool useDefaultAtlas          = atlasVolumeFileNames.empty();
  bool useDefaultOutput         = resultVolumeFileName.empty();
  bool writeIntermediateResults = !intermediateResultsDirectory.empty();
  bool segmentationSucceeded    = true;

  if (verbose) std::cout << "Starting EMSegment Command Line." << std::endl;

  if (!generateEmptyMRMLSceneAndQuit.empty())
    {
    GenerateEmptyMRMLScene(generateEmptyMRMLSceneAndQuit.c_str());
    return EXIT_SUCCESS;
    }

#ifdef _WIN32
  //
  // strip backslashes from parameter node name (present if spaces were used)
  parametersMRMLNodeName = StripBackslashes(parametersMRMLNodeName);
#endif

  if (verbose) std::cout << "Setting taskPreProcessingSetting: " << taskPreProcessingSetting << std::endl;

  //
  // make sure arguments are sufficient and unique
  bool argsOK = true;
  if (mrmlSceneFileName.empty())
    {
    std::cerr << "Error: mrmlSceneFileName must be specified." << std::endl;
    argsOK = false;
    }
  if (!argsOK)
    {
    std::cerr << "Try --help for usage..." << std::endl;
    return EXIT_FAILURE;
    }

  //
  // make sure files exist
  if (writeIntermediateResults &&
      !vtksys::SystemTools::FileExists(intermediateResultsDirectory.c_str()))
    {
      std::cout << "Warning: intermediate results directory does not exist. "
                << "We will try to create it for you."
                << std::endl;
      std::cout << intermediateResultsDirectory << std::endl;
    }

  if (!vtksys::SystemTools::FileExists(mrmlSceneFileName.c_str()))
    {
      std::cerr << "Error: MRML scene file does not exist." << std::endl;
      std::cerr << mrmlSceneFileName << std::endl;
      return EXIT_FAILURE;
    }

  if (!resultStandardVolumeFileName.empty() &&
      !vtksys::SystemTools::FileExists(resultStandardVolumeFileName.c_str()))
    {
      std::cerr << "Error: result standard volume file does not exist."
                << std::endl;
      std::cerr << resultStandardVolumeFileName << std::endl;
      return EXIT_FAILURE;
    }

  // the gui uses <image>, the command line uses actual files
  for (unsigned int i = 0; i < targetVolumeFileNames.size(); ++i)
    {
      if (!vtksys::SystemTools::
          FileExists(targetVolumeFileNames[i].c_str()))
        {
          std::cerr << "Error: target volume file " << i << " does not exist."
                    << std::endl;
          std::cerr << targetVolumeFileNames[i] << std::endl;
          return EXIT_FAILURE;
        }
    }

  for (unsigned int i = 0; i < atlasVolumeFileNames.size(); ++i)
    {
      if (!vtksys::SystemTools::
          FileExists(atlasVolumeFileNames[i].c_str()))
        {
          std::cerr << "Error: atlas volume file " << i << " does not exist."
                    << std::endl;
          std::cerr << atlasVolumeFileNames[i] << std::endl;
          return EXIT_FAILURE;
        }
    }

  // =======================================================================
  //  Initialize TCL
  // =======================================================================

  // interp has to be set to initialize vtkSlicer
  Tcl_Interp *interp =CreateTclInterp(argc,argv);
  if (!interp)
    {
      return EXIT_FAILURE;
    }

  vtkSlicerApplication* app = vtkSlicerApplication::GetInstance();
  vtkSlicerApplicationLogic* appLogic = InitializeApplication(interp,app,argc,argv);
  if (!appLogic)
    {
      CleanUp(app,appLogic);
      return EXIT_FAILURE;
    }


    // =======================================================================
    //  Setting up initial Scene
    // =======================================================================

    //
    // create a mrml scene that will hold the parameters and data
    vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
    vtkMRMLScene::SetActiveScene(mrmlScene);
    appLogic->SetAndObserveMRMLScene(mrmlScene);

    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
    colorLogic->SetMRMLScene(mrmlScene);
    colorLogic->AddDefaultColorNodes();
    colorLogic->SetMRMLScene(NULL);
    colorLogic->Delete();

    //
    // create an instance of vtkEMSegmentLogic and connect it with the
    // MRML scene
    vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
    emLogic->SetModuleName("EMSegment");
    emLogic->SetAndObserveMRMLScene(mrmlScene);
    emLogic->RegisterMRMLNodesWithScene();
    std::string emLogicTcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp,emLogic));

    vtkIntArray *emsEvents                 = vtkIntArray::New();
    emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
    emsEvents->Delete();

    vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();
    Slicer3Helper::AddDataIOToScene(mrmlScene,app,appLogic,dataIOManagerLogic);

    //
    // For the EMSegment logic, getting and setting of parameters in the
    // MRML scene is delegated to the EMSegment MRML manager.  Get a
    // shortcut to the manager.
    vtkEMSegmentMRMLManager* emMRMLManager = emLogic->GetMRMLManager();

    std::string emMRMLManagerTcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp,emMRMLManager));

    // =======================================================================
    //
    //  Loading Data
    //
    // =======================================================================

    progressReporter.ReportProgress("Loading Data...",
                                   currentStep++ / totalSteps,
                                   0.2f);

    mrmlScene->SetURL(mrmlSceneFileName.c_str());

  //
  // global try block makes sure data is cleaned up if anything goes
  // wrong
  try
    {
    //
    // read the mrml scene
    try
      {
      if (verbose) std::cout << "Reading MRML scene...";
      //
      mrmlScene->Connect();
      if (verbose) std::cout << "DONE" << std::endl;
      }
    catch (...)
      {
      throw std::runtime_error("ERROR: failed to import mrml scene.");
      }

    if (verbose) {
        std::cout << "=============== Print EMSegmentMRMLManager" << std::endl;
        emMRMLManager->Print(std::cout);

        std::cout << "=============== Print PrintVolumeInfo(mrmlScene)" << std::endl;
        emMRMLManager->PrintVolumeInfo(mrmlScene);

        //std::cout << "=============== Print Tree" << std::endl;
        //emMRMLManager->PrintTree();
    }

    int numParameterSets = emMRMLManager->GetNumberOfParameterSets();
    if (verbose) std::cout << "Imported: " << mrmlScene->GetNumberOfNodes()
                           << (mrmlScene->GetNumberOfNodes() == 1
                               ? " node" : " nodes")
                           << ", including " << numParameterSets
                           << " EM parameter "
                           << (numParameterSets == 1 ? "node." : "nodes.")
                           << std::endl;

    progressReporter.ReportProgress("Loading Data...",
                                     currentStep / totalSteps,
                                     0.4f);


    // =======================================================================
    // Define EMS Template
    // =======================================================================

    //
    // make sure there is at least one parameter set
    //
    if (numParameterSets < 1)
      {
      throw std::
        runtime_error("ERROR: no EMSegment parameter nodes in scene.");
      }

    //
    // find the parameter set in the MRML scene
    int parameterNodeIndex = 0;
    if (useDefaultParametersNode)
      {
      if (verbose) std::cout << "Using default parameter set named: "
                             << emMRMLManager->GetNthParameterSetName(0)
                             << std::endl;
      }
    else
      {
      // search for the named parameter set
      bool foundParameters = false;
      if (verbose) std::cout << "Searching for an EM parameter node named: "
                             << parametersMRMLNodeName << std::endl;

      for (int i = 0; i < numParameterSets; ++i)
        {
        std::string currentNodeName(emMRMLManager->GetNthParameterSetName(i));
        if (verbose) std::cout << "Node " << i
                               << " name: " << currentNodeName << std::endl;
        if (parametersMRMLNodeName == currentNodeName)
          {
          parameterNodeIndex = i;
          foundParameters = true;
          break;
          }
        else
          {
          if (verbose) std::cerr << "Found non-matching EM parameters node: "
                                 << currentNodeName << std::endl;
          }
        }

      // make sure the parameters were found
      if (!foundParameters)
        {
        std::stringstream ss;
        ss << "ERROR: no EMSegment parameters found in scene with name "
           << parametersMRMLNodeName;
        throw std::
          runtime_error(ss.str());
        }
      }

    //
    // populate the mrmlManager with the parameters
    try
      {
      emMRMLManager->SetLoadedParameterSetIndex(parameterNodeIndex);
      }
    catch (...)
      {
      throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
      }

    //
    // don't use manual sampling because the target images might change
    // this is a hack; do better !!!
    emMRMLManager->ChangeTreeNodeDistributionsFromManualSamplingToManual();

    progressReporter.ReportProgress("Loading Data...",
                                     currentStep / totalSteps,
                                     0.6f);
    //
    // change the default tcl preprocessing parameter
    try
      {
      emMRMLManager->GetGlobalParametersNode()->SetTaskPreProcessingSetting(taskPreProcessingSetting.c_str());
      }
    catch (...)
      {
      throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
      }

    // =======================================================================
    // Define Target Images
    // =======================================================================
    if (!useDefaultTarget)
      {
      try
        {
        if (verbose)
          std::cout << "Adding a target node...";

        // create target node
        vtkMRMLEMSVolumeCollectionNode* targetNode = vtkMRMLEMSVolumeCollectionNode::New();
        mrmlScene->AddNodeNoNotify(targetNode);

        // remove default target node
        mrmlScene->RemoveNode(emMRMLManager->GetTargetInputNode());

        // connect target node to segmenter
        emMRMLManager->GetWorkingDataNode()->SetInputTargetNodeID(targetNode->GetID());

        if (verbose)
          std::cout << targetNode->GetID() << " DONE" << std::endl;

        targetNode->Delete();

        if (verbose)
          std::cout << "Segmenter's target node is now: "
                    << emMRMLManager->GetTargetInputNode()->GetID()
                    << std::endl;
        }
      catch (...)
        {
        throw std::runtime_error("ERROR: failed to add target node.");
        }

      if (verbose)std::cout << "Adding " << targetVolumeFileNames.size()  << " target images..." << std::endl;
      for (unsigned int imageIndex = 0; imageIndex < targetVolumeFileNames.size(); ++imageIndex)
        {
        if (verbose) std::cout << "Loading target image " << imageIndex
                               << "..." << std::endl;
        try
          {
          // load image into scene
            vtkMRMLVolumeNode* volumeNode = AddArchetypeScalarVolume(targetVolumeFileNames[imageIndex].c_str(),targetVolumeFileNames[imageIndex].c_str(), appLogic, mrmlScene);

          if (!volumeNode)
            {
            throw std::runtime_error("failed to load image.");
            }

           // set volume name and ID in map
           emMRMLManager->GetTargetInputNode()->AddVolume(volumeNode->GetID(), volumeNode->GetID());
          }
        catch(...)
          {
          vtkstd::stringstream ss;
          ss << "ERROR: failed to load target image " << targetVolumeFileNames[imageIndex];
          throw std::runtime_error(ss.str());
          }
        }
      }

    //
    // make sure the number of input channels matches the expected
    // value in the parameters
    if (emMRMLManager->GetGlobalParametersNode()->
        GetNumberOfTargetInputChannels() !=
        emMRMLManager->GetTargetInputNode()->GetNumberOfVolumes())
      {
      vtkstd::stringstream ss;
      ss << "ERROR: Number of input channels (" <<
        emMRMLManager->GetTargetInputNode()->GetNumberOfVolumes()
         << ") does not match expected value from parameters (" <<
        emMRMLManager->GetGlobalParametersNode()->
        GetNumberOfTargetInputChannels()
         << ")";
      throw std::runtime_error(ss.str());
      }
    else
      {
      if (verbose)
        std::cout << "Number of input channels (" <<
          emMRMLManager->GetTargetInputNode()->GetNumberOfVolumes()
                  << ") matches expected value from parameters (" <<
          emMRMLManager->GetGlobalParametersNode()->
          GetNumberOfTargetInputChannels()
                  << ")" << std::endl;
      }

    progressReporter.ReportProgress("Loading Data...",
                                     currentStep / totalSteps,
                                     0.8f);
    // =======================================================================
    // Define Atlas Images
    // =======================================================================

    // cout << "useDefaultAtlas " << useDefaultAtlas << " " << atlasVolumeFileNames.empty() << endl;

    if (!useDefaultAtlas)
      {
        vtkMRMLEMSAtlasNode* atlasNode = emMRMLManager->GetAtlasInputNode();
        if (!atlasNode)
          {
            throw std::runtime_error("ERROR: parameters must already contain an atlas node if you wish to specify atlas volumes.");
          }

        if (int(atlasNode->GetNumberOfVolumes()) != int(atlasVolumeFileNames.size()))
          {
            std::stringstream ss;
            ss << "ERROR: number of volumes defined by atlasVolumeFileNames ("<< int(atlasVolumeFileNames.size())
               << ") does not match number of atlas volumes originally defined by template (" << atlasNode->GetNumberOfVolumes() << ")";
            throw std::runtime_error(ss.str());
          }

        if (verbose)
          {
            std::cerr << "Adding " << atlasVolumeFileNames.size() << " atlas images..." << std::endl;
          }

        for (unsigned int imageIndex = 0; imageIndex < atlasVolumeFileNames.size(); ++imageIndex)
          {
            if (verbose) std::cout << "Loading atlas image " << imageIndex << "..." << std::endl;
            try
              {
                // load image into scene
                vtkMRMLVolumeNode* volumeNode = AddArchetypeScalarVolume( atlasVolumeFileNames[imageIndex].c_str(), atlasVolumeFileNames[imageIndex].c_str(), appLogic, mrmlScene);
                if (!volumeNode)
                  {
                    throw std::runtime_error("failed to load image.");
                  }

                // set volume name and ID in map
                // cout << "Updated Node :" << atlasNode->AddVolume( atlasNode->GetNthKey(imageIndex),  volumeNode->GetID() ) << endl;
                atlasNode->SetNthNodeID(imageIndex,volumeNode->GetID());
              }
            catch(...)
              {
                vtkstd::stringstream ss;
                ss << "ERROR: failed to load atlas image " << atlasVolumeFileNames[imageIndex];
                throw std::runtime_error(ss.str());
              }
          }

        if (verbose)
          {
            cout << "Done downloading atlases - here is the assignment between class and atlas volumes " << endl;
             // iterate over tree nodes
            for (unsigned int imageIndex = 0; imageIndex < atlasVolumeFileNames.size(); ++imageIndex)
              {
                // This assumes that the keys are defined by the EMSTree node ids - which they were so far !
                // if needed can be made more fancy
                const char* volumeNodeID =  atlasNode->GetNthNodeID(imageIndex);
                vtkMRMLNode* vNode =  mrmlScene->GetNodeByID(volumeNodeID );
                if (vNode)
                  {
                     const char* treeNodeID =  atlasNode->GetNthKey(imageIndex);
                     vtkMRMLNode* tNode = mrmlScene->GetNodeByID(treeNodeID);
                      if (tNode)
                        {
                          cout << "Class: " << setw(30) << std::left << tNode->GetName() <<  " " << vNode->GetName() << " " << volumeNodeID << endl;
                        }
                      else
                        {
                           cout << "AtlasTemplateID: " << setw(30) << std::left << treeNodeID <<  " " << vNode->GetName() <<  " " << volumeNodeID << endl;
                        }
                  }
                else
                  {
                    // then just make the method more fancy so that you first go through all the nodes in the tree - and then look where the index of the corresponding key defined by the spatialnode ID
                    cout << "Could not retrieve class name or volume for: " << atlasVolumeFileNames[imageIndex].c_str() << endl;
                  }
              }
          }
      }

    // =======================================================================
    // Define Generated Segmentation Volume
    // =======================================================================
    //
    // set the result labelmap image
    //
    if (useDefaultOutput)
      {
      if (!emMRMLManager->GetOutputVolumeNode())
        {
        throw std::
          runtime_error("ERROR: no default output volume node available.");
        }
      if (verbose)
        std::cout << "Using default output volume node named: "
                  <<
          emMRMLManager->GetOutputVolumeNode()->GetName()
                  << std::endl;
      }
    else
      {
      try
        {
        // create volume node
        if (verbose) std::cout << "Creating output volume node...";

        vtkstd::string absolutePath = resultVolumeFileName;

        // the gui uses <image>, the command line uses actual files
        //
        // Set up the filename so that a relative filename will be
        // relative to the current directory, not relative to the mrml
        // scene's path.
        if (!vtksys::SystemTools::FileIsFullPath(resultVolumeFileName.c_str()))
          {
          absolutePath = vtksys::SystemTools::
            CollapseFullPath(resultVolumeFileName.c_str());
          }

        vtkMRMLVolumeNode* outputNode =
          AddNewScalarArchetypeVolume(mrmlScene,
                                      absolutePath.c_str(),
                                      true,
                                      true,
                                      NULL);

        if (!outputNode)
          {
          throw std::runtime_error("failed to create output image");
          }

        // connect output volume node to segmenter
        emMRMLManager->SetOutputVolumeMRMLID(outputNode->GetID());

        if (verbose)
          std::cout << "DONE" << std::endl;
        }
      catch (...)
        {
        throw std::runtime_error("ERROR: failed to add result volume node.");
        }
      }



    // =======================================================================
    // Update Misc. Parameters
    // =======================================================================
    progressReporter.ReportProgress("Updating Parameters...",
                                     currentStep++ / totalSteps,
                                     0.0f);


    //
    // set registrationMethod from command line
    if (registrationPackage=="CMTK")
      {
        emMRMLManager->SetRegistrationPackageType(0); //CMTK
      }
    else
      {
        emMRMLManager->SetRegistrationPackageType(1); // BRAINS
      }
    if (verbose)
      std::cout << "Registration Package is "
                << registrationPackage
                << std::endl;

    //TODO, review
    emMRMLManager->SetRegistrationAffineType(atoi(registrationAffineType.c_str()));
    emMRMLManager->SetRegistrationDeformableType(atoi(registrationDeformableType.c_str()));

    //
    // update logic parameters from command line
    emMRMLManager->SetEnableMultithreading(!disableMultithreading);
    if (verbose)
      std::cout << "Multithreading is "
                << (disableMultithreading ? "disabled." : "enabled.")
                << std::endl;

    emMRMLManager->SetUpdateIntermediateData(!dontUpdateIntermediateData);
    if (verbose)
      std::cout << "Update intermediate data: "
                << (dontUpdateIntermediateData ? "disabled." : "enabled.")
                << std::endl;

    int segmentationBoundaryMin[3];
    int segmentationBoundaryMax[3];
    emMRMLManager->GetSegmentationBoundaryMin(segmentationBoundaryMin);
    emMRMLManager->GetSegmentationBoundaryMax(segmentationBoundaryMax);
    if (verbose) std::cout
      << "Default ROI is ["
      << segmentationBoundaryMin[0] << ", "
      << segmentationBoundaryMin[1] << ", "
      << segmentationBoundaryMin[2] << "] -->> ["
      << segmentationBoundaryMax[0] << ", "
      << segmentationBoundaryMax[1] << ", "
      << segmentationBoundaryMax[2] << "]" << std::endl;

    //
    // set intermediate results directory
    if (writeIntermediateResults)
      {
      emMRMLManager->SetSaveIntermediateResults(true);
      vtkstd::string absolutePath = vtksys::SystemTools::
        CollapseFullPath(intermediateResultsDirectory.c_str());
      emMRMLManager->
        SetSaveWorkingDirectory(absolutePath.c_str());
      std::cout << "Intermediate results will be written to: "
                << absolutePath << std::endl;
      }
    else
      {
      emMRMLManager->SetSaveIntermediateResults(false);
      }

    // Disable registration
    if (disableRegistration)
      {
        // Set both affine and deformable to off - and so preprocessing will jump over
    emMRMLManager->GetGlobalParametersNode()->SetRegistrationAffineType(0);
    emMRMLManager->GetGlobalParametersNode()->SetRegistrationDeformableType(0);
      }

    if (verbose) {
        std::cout << "=============== Print EMSegmentMRMLManager" << std::endl;
        emMRMLManager->PrintInfo(std::cout);
    }

    //
    // check parameters' node structure
    if (!emMRMLManager->CheckMRMLNodeStructureForProcessing())
      {
        throw std::runtime_error("ERROR: EMSegment invalid parameter node structure");
      }



    // =======================================================================
    //
    //  Start processing
    //
    // =======================================================================

    progressReporter.ReportProgress("Running Segmentation...",
                                     currentStep++ / totalSteps);

    try
       {
         if (verbose) std::cout << "Starting preprocessing ..." << std::endl;

         emMRMLManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);
         emMRMLManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);


         if (emLogic->SourcePreprocessingTclFiles(app))
           {
             throw std::runtime_error("ERROR: could not source tcl files. ");
           }

        // Have to init variables again bc first time EMLogic was not fully set up
        std::string CMD = "::EMSegmenterPreProcessingTcl::InitVariables " + emLogicTcl + " " + emMRMLManagerTcl + " NULL";
        if (atoi(app->Script(CMD.c_str())))
        {
           throw std::runtime_error("ERROR: could not init files. ");
        }


         if (atoi(app->Script("::EMSegmenterPreProcessingTcl::Run")))
           {
             throw std::runtime_error("ERROR: Pre-processing did not execute correctly");
           }


         emMRMLManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
         emMRMLManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

          if (verbose) std::cout << "EMSEG: Preprocessing complete." << std::endl;

          if (verbose) std::cout << "EMSEG: Start Segmentation." << std::endl;

          int return_value;
          return_value = emLogic->StartSegmentationWithoutPreprocessing(app,appLogic);
          if ( return_value == 1 )
          {
            std::cerr << "ERROR: StartSegmentationWithoutPreprocessing failed." << std::endl;
            throw std::runtime_error("");
          }

          if (verbose) std::cout << "Segmentation complete." << std::endl;
          std::cout << "============ End of New Pipeline =========================" << std::endl;

       }
     catch (...)
       {
         throw std::runtime_error("ERROR: failed to run preprocessing/segmentation.");
       }

    }
  catch (std::runtime_error& e)
    {
      std::cerr << "EMSegmentCommandline.cxx: Segmentation failed: " << e.what() << std::endl;
      segmentationSucceeded = false;
    }
  catch (...)
    {
      std::cerr << "EMSegmentCommandline.cxx: Unknown error detected.  Segmentation failed." << std::endl;
      segmentationSucceeded = false;
    }

  progressReporter.ReportProgress("Updating Results...",
                                   currentStep++ / totalSteps);

   // =======================================================================
   //
   //  Write out results and clean up
   //
   // =======================================================================

  if (segmentationSucceeded && !dontWriteResults)
    {

    //
    // save the results
    if (verbose) std::cout << "Saving segmentation results..." << std::endl;
    try
      {
      vtkstd::cout << "Writing segmentation result: " <<
        emMRMLManager->GetOutputVolumeNode()->GetStorageNode()->GetFileName()
                   << vtkstd::endl;
      emMRMLManager->GetOutputVolumeNode()->GetStorageNode()->
        SetUseCompression(!disableCompression);
      emMRMLManager->GetOutputVolumeNode()->GetStorageNode()->
        WriteData(emMRMLManager->GetOutputVolumeNode());
      }
    catch (std::runtime_error& e)
      {
      std::cerr << e.what() << std::endl;
      std::cerr << "Errors detected.  Writing failed." << std::endl;
      segmentationSucceeded = false;
      }
    catch (...)
      {
      std::cerr << "Unknown error detected.  Writing failed." << std::endl;
      segmentationSucceeded = false;
      }
    if (verbose) std::cout << "DONE" << std::endl;
    }
  else
    {
    if (verbose)
      std::cout << "Skipping over saving segmentation results." << std::endl;
    }

  //
  // compare results to standard image
  if (segmentationSucceeded && !resultStandardVolumeFileName.empty())
    {
    if (verbose)
      cout << "Comparing results with standard..." << std::endl;

    try
      {
      //
      // get a pointer to the results
      std::string resultMRMLID = emMRMLManager->GetOutputVolumeMRMLID();
      std::cout << "Extracting results from mrml node: "
                << resultMRMLID << std::endl;
      vtkImageData* resultImage = NULL;
      vtkMRMLVolumeNode* node   = vtkMRMLVolumeNode::
        SafeDownCast(mrmlScene->GetNodeByID(resultMRMLID.c_str()));
      resultImage = node->GetImageData();
      resultImage->SetSpacing(node->GetSpacing());
      resultImage->SetOrigin(node->GetOrigin());

      //
      // compare result with standard image
      bool imagesDiffer = ImageDiff(resultImage, resultStandardVolumeFileName);
      if (imagesDiffer)
        {
        if (verbose)
          if (verbose) cerr << "Result DOES NOT match standard!" << std::endl;
        segmentationSucceeded = false;
        }
      else
        {
        if (verbose) cout << "Result matches standard!" << std::endl;
        segmentationSucceeded = true;
        }
      }
    catch (std::runtime_error& e)
      {
      std::cerr << e.what() << std::endl;
      std::cerr << "Errors detetected.  Comparison failed." << std::endl;
      segmentationSucceeded = false;
      }
    catch (...)
      {
      std::cerr << "Unknown error detected.  Comparison failed." << std::endl;
      segmentationSucceeded = false;
      }
    }

  //
  // write the final mrml scene file
  if (segmentationSucceeded && !resultMRMLSceneFileName.empty())
    {
    if (verbose) std::cout << "Writing mrml scene...";
    mrmlScene->Commit(resultMRMLSceneFileName.c_str());
    if (verbose) std::cout << "DONE" << std::endl;
    }

  progressReporter.ReportProgress("Cleaning Up...",
                                  currentStep++ / totalSteps);

  //
  // clean up
  //
  if (verbose) std::cout << "Cleaning up...";

  CleanUp(app,appLogic);

  emLogic->SetAndObserveMRMLScene(NULL);
  Slicer3Helper::RemoveDataIOFromScene(mrmlScene,dataIOManagerLogic);
  emLogic->Delete();

  dataIOManagerLogic->Delete();
  dataIOManagerLogic = NULL;

  mrmlScene->Clear(true);
  mrmlScene->Delete();
  if (verbose) std::cout << "DONE" << std::endl;

  return segmentationSucceeded ? EXIT_SUCCESS : EXIT_FAILURE;
}
