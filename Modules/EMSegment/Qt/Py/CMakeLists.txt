
SET(EMS_PYTHON_SCRIPTS
  EMSegmentPy.py
  EMSegmentQuickPy.py
  EMSegmentWizard/__init__.py
  EMSegmentWizard/Helper.py
  EMSegmentWizard/EMSegmentDynamicFrame.py
  EMSegmentWizard/EMSegmentStep.py
  EMSegmentWizard/EMSegmentSelectTaskStep.py                        # step 1
  EMSegmentWizard/EMSegmentDefineInputChannelsStep.py               # step 2
  EMSegmentWizard/EMSegmentDefineAnatomicalTreeStep.py              # step 3
  EMSegmentWizard/EMSegmentDefineAtlasStep.py                       # step 4
  EMSegmentWizard/EMSegmentEditRegistrationParametersStep.py        # step 5
  EMSegmentWizard/EMSegmentDefinePreprocessingStep.py               # step 6
  EMSegmentWizard/EMSegmentSpecifyIntensityDistributionStep.py      # step 7
  EMSegmentWizard/EMSegmentEditNodeBasedParametersStep.py           # step 8
  EMSegmentWizard/EMSegmentDefineMiscParametersStep.py              # step 9
  EMSegmentWizard/EMSegmentStartSegmentationStep.py
  EMSegmentWizard/EMSegmentDummyStep.py
  )

SET(EMS_PYTHON_RESOURCES
  Resources/qSlicerEMSegmentModule.qrc
  )

ctkMacroCompilePythonScript(
  TARGET_NAME qSlicerEMSegmentModuleWidget
  SCRIPTS "${EMS_PYTHON_SCRIPTS}"
  RESOURCES "${EMS_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_PYTHON_LIB_DIR}
  NO_INSTALL_SUBDIR
  )

ctkMacroCompilePythonScript(
  TARGET_NAME EMSegmentQuickPy
  SCRIPTS "${EMS_PYTHON_SCRIPTS}"
  RESOURCES "${EMS_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_PYTHON_LIB_DIR}
  NO_INSTALL_SUBDIR
  )
