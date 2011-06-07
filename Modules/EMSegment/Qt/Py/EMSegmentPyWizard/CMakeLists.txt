SET(EMSegmentPyWizard_PYTHON_SCRIPTS
  __init__.py
  Helper.py
  EMSegmentPyStep.py
  EMSegmentPyStepOne.py
  EMSegmentPyStepTwo.py
  EMSegmentPyStepThree.py 
  EMSegmentPyDummyStep.py  
  )

SET(EMSegmentPyWizard_PYTHON_RESOURCES
  )
  
ctkMacroCompilePythonScript(
  TARGET_NAME EMSegmentPyWizard
  SCRIPTS "${EMSegmentPyWizard_PYTHON_SCRIPTS}"
  RESOURCES "${EMSegmentPyWizard_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/EMSegmentPyWizard
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif(BUILD_TESTING)
