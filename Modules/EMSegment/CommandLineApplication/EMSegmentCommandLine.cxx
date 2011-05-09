#include "EMSegmentCommandLineCLP.h"
#include "EMSegmentAPIHelper.h"
#include "EMSegmentCommandLineFct.h"
#include "vtkSlicerCommonInterface.h"

//---------------------------------------------------------------------------
// Slicer3_USE_PYTHON
//
#ifdef Slicer3_USE_PYTHON
#include "slicerPython.h"

extern "C" {
  void init_mytkinter( Tcl_Interp* );
  void init_slicer(void );
}
#include "vtkTclUtil.h"

#endif

// =======================================================================
//  MAIN
// =======================================================================
int main(int argc, char** argv)
{

  // =======================================================================
  //
  //  Read and check Input
  //
  // =======================================================================

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

#if WIN32
#define PathSep ";"
#else
#define PathSep ":"
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
  //
  //  Initialize Enviornment
  //
  // =======================================================================

  // SLICER COMMON INTERFACE
  vtkSlicerCommonInterface *slicerCommon = vtkSlicerCommonInterface::New();

  vtksys_stl::string slicerHome;
  if ( !vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome) )
    {
    slicerHome = std::string(slicerCommon->GetBinDirectory()) + "/..";
    slicerHome = vtksys::SystemTools::CollapseFullPath(slicerHome.c_str());
    }

  // ================== Tcl  ==================
  Tcl_Interp *interp =CreateTclInterp(argc,argv,slicerCommon);
  if (!interp)
    {
      return EXIT_FAILURE;
    }

  // ================== Application  ==================
  vtkSlicerApplicationLogic* appLogic = InitializeApplication(interp,slicerCommon,argc,argv);
  if (!appLogic)
    {
      CleanUp(appLogic,slicerCommon);
      return EXIT_FAILURE;
    }


  // ================== MRMLScene  ==================

  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  appLogic->SetAndObserveMRMLScene(mrmlScene);

  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
  colorLogic->SetMRMLScene(mrmlScene);
  colorLogic->AddDefaultColorNodes();
  colorLogic->SetMRMLScene(NULL);
  colorLogic->Delete();


  // PYTHON
#ifdef Slicer3_USE_PYTHON
  // Initialize Python

  // Set up the search path
  std::string pythonEnv = "PYTHONPATH=";

  const char* existingPythonEnv = vtksys::SystemTools::GetEnv("PYTHONPATH");
  if ( existingPythonEnv )
    {
    pythonEnv += std::string ( existingPythonEnv ) + PathSep;
    }

  pythonEnv += slicerHome + "/" + Slicer3_INSTALL_LIB_DIR + "/SlicerBaseGUI/Python" + PathSep;
  pythonEnv += slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + PathSep;
  vtkKWApplication::PutEnv(const_cast <char *> (pythonEnv.c_str()));

  Py_Initialize();
  PySys_SetArgv(argc, argv);
  PyObject* PythonModule = PyImport_AddModule("__main__");
  if (PythonModule == NULL)
    {
    std::cout << "Warning: Failed to initialize python" << std::endl;
    }
  PyObject* PythonDictionary = PyModule_GetDict(PythonModule);

  // Intercept _tkinter, and use ours...
  init_mytkinter(interp);
  init_slicer();
  PyObject* v;

  std::vector<std::string> pythonInitStrings;

  pythonInitStrings.push_back(std::string("import _tkinter;"));
  pythonInitStrings.push_back(std::string("import Tkinter;"));
  pythonInitStrings.push_back(std::string("import sys;"));
  pythonInitStrings.push_back(std::string("from os.path import join as j;"));
  pythonInitStrings.push_back(std::string("tk = Tkinter.Tk();"));
  pythonInitStrings.push_back(std::string("sys.path.append ( j('" + slicerHome + "','" + Slicer3_INSTALL_LIB_DIR + "', 'SlicerBaseGUI', 'Python')" + " );"));
  pythonInitStrings.push_back(std::string("sys.path.append ( j('" + slicerHome + "','" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "') );"));

  /*
  std::string TkinitString = "import Tkinter, sys;"
    "from os.path import join as j;"
    "tk = Tkinter.Tk();"
    "sys.path.append ( j('"
    + slicerHome + "','" + Slicer3_INSTALL_LIB_DIR + "', 'SlicerBaseGUI', 'Python')"
    + " );\n"
    "sys.path.append ( j('"
    + slicerHome + "','" + Slicer3_INSTALL_PLUGINS_BIN_DIR
    + "') );\n";
    */

  std::vector<std::string>::iterator strIt;
  strIt = pythonInitStrings.begin();
  for (; strIt != pythonInitStrings.end(); strIt++)
    {
    v = PyRun_String( (*strIt).c_str(),
                      Py_file_input,
                      PythonDictionary,
                      PythonDictionary );
    if (v == NULL)
      {
      PyObject *exception, *v, *tb;
      PyObject *exception_s, *v_s, *tb_s;

      PyErr_Fetch(&exception, &v, &tb);
      if (exception != NULL)
        {
        PyErr_NormalizeException(&exception, &v, &tb);
        if (exception != NULL)
          {
          exception_s = PyObject_Str(exception);
          v_s = PyObject_Str(v);
          tb_s = PyObject_Str(tb);
          const char *e_string, *v_string, *tb_string;
          cout << "Running: " << (*strIt).c_str() << endl;
          e_string = PyString_AS_STRING(exception_s);
          cout << "Exception: " << e_string << endl;
          v_string = PyString_AS_STRING(v_s);
          cout << "V: " << v_string << endl;
          tb_string = PyString_AS_STRING(PyObject_Str(tb_s));
          cout << "TB: " << tb_string << endl;
          Py_DECREF ( exception_s );
          Py_DECREF ( v_s );
          Py_DECREF ( tb_s );
          Py_DECREF ( exception );
          Py_DECREF ( v );
          if ( tb )
            {
            Py_DECREF ( tb );
            }
          }
        }

      PyErr_Print();
      }
    else
      {
      if (Py_FlushLine())
        {
        PyErr_Clear();
        }
      }
    }

  // now load the atlascreator module
  vtkSlicerApplication::GetInstance()->InitializePython(
    (void*)PythonModule, (void*)PythonDictionary);

  std::string pythonCommand = "";

  pythonCommand += "import sys\n";
  pythonCommand += "import os\n";
  pythonCommand += "packageNames = []\n";

/*
  module_paths_it = modulePathsList.begin();
  for (; module_paths_it != module_paths_end; module_paths_it++)
    {
*/


    vtksys_stl::string module_path("/Users/daniel/SLICER/TRUNK/Slicer3-build/lib/Slicer3/Modules/AtlasCreator/");
    vtksys::SystemTools::ConvertToUnixSlashes(module_path);
    if (*module_path.c_str() &&
        vtksys::SystemTools::FileExists(module_path.c_str()))
      {
      pythonCommand += "modulePath = '" + module_path + "'\n";
      pythonCommand += "sys.path.append(modulePath)\n";
      pythonCommand += "for packageName in os.listdir(modulePath):\n";
      pythonCommand += "    if os.path.isfile(os.path.join(modulePath,packageName,'__init__.py')):\n";
      pythonCommand += "        packageNames.append(packageName)\n";
      }


//    }

  pythonCommand += "import Slicer\n";
  pythonCommand += "import SlicerScriptedModule\n";
  pythonCommand += "SlicerScriptedModuleInfo = SlicerScriptedModule.SlicerScriptedModuleImporter(packageNames)\n";
  pythonCommand += "SlicerScriptedModuleInfo.ScanAndInitModules()\n";
  pythonCommand += "Slicer.ScriptedModuleInfo = SlicerScriptedModuleInfo\n";

  v = PyRun_String( pythonCommand.c_str(),
                    Py_file_input,
                    PythonDictionary,PythonDictionary);
  if (v == NULL)
    {
    PyErr_Print();
    }

#endif

  // ================== EMSegmenter  ==================

  // create an instance of vtkEMSegmentLogic and connect it with theMRML scene
  vtkEMSegmentLogic* EMSLogic = vtkEMSegmentLogic::New();
  EMSLogic->SetModuleName("EMSegment");
  EMSLogic->SetAndObserveMRMLScene(mrmlScene);
  EMSLogic->RegisterMRMLNodesWithScene();
  std::string EMSLogicTcl = vtksys::SystemTools::DuplicateString(slicerCommon->GetTclNameFromPointer(EMSLogic));

  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  EMSLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

  // For the EMSegment logic, getting and setting of parameters in the
  // MRML scene is delegated to the EMSegment MRML manager.  Get a
  // shortcut to the manager.
  vtkEMSegmentMRMLManager* emMRMLManager = EMSLogic->GetMRMLManager();
  std::string emMRMLManagerTcl = vtksys::SystemTools::DuplicateString(slicerCommon->GetTclNameFromPointer(emMRMLManager));
  emMRMLManager->SetMRMLScene( mrmlScene );

  // ================== Data IO  ==================
  vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();

  slicerCommon->AddDataIOToScene(mrmlScene,appLogic,dataIOManagerLogic);

  //
  // global try block makes sure data is cleaned up if anything goes  wrong
  //
  try
    {

      // =======================================================================
      //
      //  Loading Data
      //
      // =======================================================================

      progressReporter.ReportProgress("Loading Data...", currentStep++ / totalSteps, 0.2f);

      // ================== MRML ==============
      mrmlScene->SetURL(mrmlSceneFileName.c_str());
      if (LoadMRMLScene(emMRMLManager, mrmlScene,  verbose))
        {
          throw std::runtime_error("ERROR: failed to import mrml scene.");
        }
      progressReporter.ReportProgress("Loading Data...", currentStep / totalSteps, 0.4f);

      // ================== EMS Template ==================
      if (DefineEMSTemplate(useDefaultParametersNode, parametersMRMLNodeName, emMRMLManager, verbose) )
        {
          throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
        }
      else {
        // don't use manual sampling because the target images might change this is a hack; do better !!!
        emMRMLManager->ChangeTreeNodeDistributionsFromManualSamplingToManual();

        // change the default tcl preprocessing parameter
        if (taskPreProcessingSetting.size())
          {
            emMRMLManager->GetGlobalParametersNode()->SetTaskPreProcessingSetting(taskPreProcessingSetting.c_str());
          }
      }
      progressReporter.ReportProgress("Loading Data...", currentStep / totalSteps,0.6f);

      // ================== Target Images ================
      if (DefineTargetVolume( useDefaultTarget, targetVolumeFileNames, EMSLogic , mrmlScene, appLogic, verbose))
        {
          throw std::runtime_error("ERROR: failed to define target image ");
        }

      progressReporter.ReportProgress("Loading Data...", currentStep / totalSteps, 0.8f);

      // =================== Atlas ====================
      if (!useDefaultAtlas)
        {
          if (LoadUserDefinedAtlas (atlasVolumeFileNames, EMSLogic, mrmlScene, appLogic, verbose))
            {
              throw std::runtime_error("ERROR: failed to load user specified atlas ");
            }
        }

      // ================== Final Result ================
      if (DefineFinalOutput(useDefaultOutput, resultVolumeFileName, emMRMLManager, mrmlScene, verbose))
        {
          throw std::runtime_error("ERROR: failed to define volume node for final output.");
        }

      // =======================================================================
      //
      //  Update Misc. Parameters
      //
      // =======================================================================
      progressReporter.ReportProgress("Updating Parameters...", currentStep++ / totalSteps, 0.0f);

      // ================== Registration  ==================

      if (!registrationPackage.empty()) {

        if (registrationPackage == "CMTK")
          {
            emMRMLManager->SetRegistrationPackageType(0); //CMTK
          }
        else if (registrationPackage == "BRAINS")
          {
            emMRMLManager->SetRegistrationPackageType(1); // BRAINS
          }
        else
          {
            throw std::runtime_error("ERROR: registration package not known.");
          }
      }

      if (verbose)
        std::cout << "Registration Package is " << registrationPackage << std::endl;

      if ( registrationAffineType != -2) {
        emMRMLManager->SetRegistrationAffineType(registrationAffineType);
      }

      if ( registrationDeformableType != -2) {
        emMRMLManager->SetRegistrationDeformableType(registrationDeformableType);
      }


      // ================== Multithreading  ==================
      if (disableMultithreading != -1) {
        emMRMLManager->SetEnableMultithreading(!disableMultithreading);
      }
      if (verbose)
        std::cout << "Multithreading is "
                  << (disableMultithreading ? "disabled." : "enabled.")
                  << std::endl;

      // ================== Intermediate Results  ==================
      if (dontUpdateIntermediateData != -1) {
        emMRMLManager->SetUpdateIntermediateData(!dontUpdateIntermediateData);
        if (verbose)
           std::cout << "Update intermediate data: "
                  << (dontUpdateIntermediateData ? "disabled." : "enabled.")
                  << std::endl;
      }

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

      // ================== Segmentation Boundary  ==================
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

      if (verbose) {
        std::cout << "=============== Print EMSegmentMRMLManager" << std::endl;
        // emMRMLManager->PrintInfo(std::cout);
      }

      // ================== Check Frinal Parameter Definition  ==================
      if (!emMRMLManager->CheckMRMLNodeStructureForProcessing())
        {
          throw std::runtime_error("ERROR: EMSegment invalid parameter node structure");
        }

      // =======================================================================
      //
      //  Process Data
      //
      // =======================================================================

      progressReporter.ReportProgress("Running Segmentation...",
                                      currentStep++ / totalSteps);

      try
        {
          // ================== Preprocessing ==================
          if (RunPreprocessing( EMSLogic, EMSLogicTcl,  emMRMLManagerTcl, slicerCommon, emMRMLManager, verbose) ) {
            throw std::runtime_error("");
          }

          // ================== Segmentation ==================
          if (verbose) std::cout << "EMSEG: Start Segmentation." << std::endl;
          if ( EMSLogic->StartSegmentationWithoutPreprocessing(appLogic) == 1 )
            {
              std::cerr << "ERROR: StartSegmentationWithoutPreprocessing failed." << std::endl;
              throw std::runtime_error("");
            }
          if (verbose) std::cout << "Segmentation complete." << std::endl;
        }
      catch (...)
        {
          throw std::runtime_error("ERROR: failed to run preprocessing,segmentation, or postprocessing.");
        }

      //
      // End of global try block
      //
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

  // ================== Write Out Results  ==================
  if (segmentationSucceeded && !dontWriteResults)
    {
      segmentationSucceeded = WriteResultsToFile(disableCompression,  emMRMLManager, verbose);
    }
  else
    {
      if (verbose) std::cout << "Skipping over saving segmentation results." << std::endl;
    }

  // ================== Compare To Standard==================
  if (segmentationSucceeded && !resultStandardVolumeFileName.empty())
    {
      segmentationSucceeded =  CompareResultsToStandard(resultStandardVolumeFileName,  disableCompression, emMRMLManager, mrmlScene, verbose);
    }

  // ==================Write Out MRML ==================
  if (segmentationSucceeded && !resultMRMLSceneFileName.empty())
    {
      if (verbose) std::cout << "Writing mrml scene...";
      mrmlScene->Commit(resultMRMLSceneFileName.c_str());
      if (verbose) std::cout << "DONE" << std::endl;
    }

  progressReporter.ReportProgress("Cleaning Up...", currentStep++ / totalSteps);


  // ================== Clean Up ==================
  if (verbose) std::cout << "Cleaning up...";


  slicerCommon->RemoveDataIOFromScene(mrmlScene,dataIOManagerLogic);

  dataIOManagerLogic->Delete();
  dataIOManagerLogic = NULL;



  EMSLogic->SetAndObserveMRMLScene(NULL);
  EMSLogic->Delete();

  mrmlScene->Clear(true);
  mrmlScene->Delete();

  CleanUp(appLogic,slicerCommon);

  if (verbose) std::cout << "DONE" << std::endl;

  return segmentationSucceeded ? EXIT_SUCCESS : EXIT_FAILURE;
}
