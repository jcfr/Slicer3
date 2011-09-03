project(EMSegmentCommandLineExecutable)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find appropriate package so that this project can be built standalone
if(NOT Slicer3_SOURCE_DIR AND NOT Slicer_SOURCE_DIR)
  if(EXISTS "${Slicer3_DIR}")
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  elseif(EXISTS "${SlicerExecutionModel_DIR}")
    find_package(SlicerExecutionModel REQUIRED GenerateCLP)
  endif()
endif()

# --------------------------------------------------------------------------
set(CommandLineApplication_LIBRARIES ${VTK_LIBRARIES})

# --------------------------------------------------------------------------
set(CommandLineApplication_INCLUDE_DIRS
  ${EMSegment_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/CommandLineApplication
  )

# --------------------------------------------------------------------------
if("${Slicer3_VERSION_MAJOR}" EQUAL "3")
  list(APPEND CommandLineApplication_LIBRARIES
    EMSegment
    AtlasCreatorCxxModule
    )
  list(APPEND CommandLineApplication_INCLUDE_DIRS
    ${Slicer3_Libs_INCLUDE_DIRS}
    ${Slicer3_Base_INCLUDE_DIRS}
    )
else()
  list(APPEND CommandLineApplication_LIBRARIES
    vtkSlicerAtlasCreatorModuleLogic
    vtkSlicerEMSegmentModuleLogic
    )
  list(APPEND CommandLineApplication_INCLUDE_DIRS
    ${vtkSlicerAtlasCreatorModuleLogic_SOURCE_DIR}
    ${vtkSlicerAtlasCreatorModuleLogic_BINARY_DIR}
    )
endif()


# --------------------------------------------------------------------------
#
# EMSegmentCommandline
#

set(CLP_MODULE EMSegmentCommandLine)

if("${Slicer3_VERSION_MAJOR}" EQUAL "3")

  # Slicer3 version
  set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
  generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
  add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE} EMSegmentCommandLineFct.cxx)
  slicer3_set_plugins_output_path(${CLP_MODULE})
  target_link_libraries(
    ${CLP_MODULE}
    ${CommandLineApplication_LIBRARIES}
    )
  slicer3_install_plugins(${CLP_MODULE})

else()

  slicerMacroBuildCLI(
    NAME ${CLP_MODULE}
    TARGET_LIBRARIES ${CommandLineApplication_LIBRARIES}
    EXECUTABLE_ONLY
    INCLUDE_DIRECTORIES ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ADDITIONAL_SRCS
      EMSegmentCommandLineFct.cxx
    )

endif()

# --------------------------------------------------------------------------
#
# EMSegmentTclInterpreter
#

set(CLP_MODULE EMSegmentTclInterpreter)

if("${Slicer3_VERSION_MAJOR}" EQUAL "3")

  set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
  generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
  add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE} EMSegmentCommandLineFct.cxx)
  slicer3_set_plugins_output_path(${CLP_MODULE})
  target_link_libraries(
    ${CLP_MODULE}
    ${CommandLineApplication_LIBRARIES}
    )

  slicer3_install_plugins(${CLP_MODULE})

# TODO
#else()

#slicerMacroBuildCLI(
#  NAME ${CLP_MODULE}
#  TARGET_LIBRARIES ${CommandLineApplication_LIBRARIES}
#  EXECUTABLE_ONLY
#  ADDITIONAL_SRCS
#    EMSegmentCommandLineFct.cxx
#  )

endif()

# --------------------------------------------------------------------------
#
# EMSegmentTransformToNewFormat
#

set(CLP_MODULE EMSegmentTransformToNewFormat)

if("${Slicer3_VERSION_MAJOR}" EQUAL "3")

  set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
  generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
  add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE} EMSegmentCommandLineFct.cxx)
  slicer3_set_plugins_output_path(${CLP_MODULE})
  target_link_libraries(
    ${CLP_MODULE}
    ${CommandLineApplication_LIBRARIES}
    )
  slicer3_install_plugins(${CLP_MODULE})

else()

  slicerMacroBuildCLI(
    NAME ${CLP_MODULE}
    TARGET_LIBRARIES ${CommandLineApplication_LIBRARIES}
    EXECUTABLE_ONLY
    ADDITIONAL_SRCS
      EMSegmentCommandLineFct.cxx
    )

endif()
