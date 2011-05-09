project(EMSegmentCommandLineExecutable)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

#-----------------------------------------------------------------------------
## Python
if(Slicer3_USE_PYTHON)
  find_package(PythonLibs)
endif(Slicer3_USE_PYTHON)

if(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)
  set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH})
  if(WIN32)
    set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH}/../PC)
  endif(WIN32)
endif(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)

include_directories(${dir_list})

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${EMSegment_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}/KWWidgets
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/CommandLineApplication
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )


# --------------------------------------------------------------------------
# Sources

set(CLP_MODULE EMSegmentCommandLine)
set(${CLP_MODULE}_SOURCE
  ${CLP_MODULE}.cxx
  )
generateclp(
  ${CLP_MODULE}_SOURCE 
  ${CLP_MODULE}.xml)

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  EMSegment
  vtkCommon
  AtlasCreatorCxxModule   
  SlicerBaseGUIPython
  )

slicer3_install_plugins(${CLP_MODULE})

# ==================================
# Default install prefix
# slicer3_set_default_install_prefix_for_external_projects()

set(CLP_MODULE EMSegmentTclInterpreter)
set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)

add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  EMSegment
  vtkCommon
  AtlasCreatorCxxModule 
  SlicerBaseGUIPython
  )

set(CLP_MODULE EMSegmentTransformToNewFormat)
set(${CLP_MODULE}_SOURCE ${CLP_MODULE}.cxx )
generateclp( ${CLP_MODULE}_SOURCE  ${CLP_MODULE}.xml)
add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  EMSegment
  vtkCommon
  AtlasCreatorCxxModule
  SlicerBaseGUIPython   
  )

# --------------------------------------------------------------------------
# Build and install the exe



