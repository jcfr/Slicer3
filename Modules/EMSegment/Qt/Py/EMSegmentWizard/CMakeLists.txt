SET(EMSegmentWizard_PYTHON_SCRIPTS
  __init__.py
  Helper.py
  EMSegmentStep.py
  EMSegmentSelectTaskStep.py
  EMSegmentDefineInputChannelsStep.py
  EMSegmentDefineAnatomicalTreeStep.py
  EMSegmentDefineAtlasStep.py 
  EMSegmentEditRegistrationParametersStep.py
  EMSegmentDummyStep.py  
  )

SET(EMSegmentWizard_PYTHON_RESOURCES
  )
  
ctkMacroCompilePythonScript(
  TARGET_NAME EMSegmentWizard
  SCRIPTS "${EMSegmentWizard_PYTHON_SCRIPTS}"
  RESOURCES "${EMSegmentWizard_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/EMSegmentWizard
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}/Python
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif(BUILD_TESTING)
