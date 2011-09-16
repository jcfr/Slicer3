# --------------------------------------------------------------------------
#
# EM Segmenter for 3D Slicer3 and 3D Slicer4
#
# --------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
if(Slicer3_SOURCE_DIR)
  include(CMake/EM_Slicer3_FindPackageSlicer.cmake)
else()
  include(CMake/EM_Slicer4_FindPackageSlicer.cmake)
endif()

# --------------------------------------------------------------------------
include(CMake/EM_BaseSources.cmake)

# --------------------------------------------------------------------------
if("${Slicer3_VERSION_MAJOR}" EQUAL "3")
  include(CMake/EM_Slicer3_IncludeSlicerPython.cmake)
  include(CMake/EM_Slicer3_GenerateConfigureFile.cmake)
endif()

# --------------------------------------------------------------------------
ADD_SUBDIRECTORY(CommandLineApplication)

# --------------------------------------------------------------------------
if("${Slicer3_VERSION_MAJOR}" EQUAL "3")
  add_subdirectory(KWWidgets)
else()
  add_subdirectory(Qt)
endif()

# --------------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif()

# --------------------------------------------------------------------------
include(CMake/EM_InstallTasks.cmake)

# --------------------------------------------------------------------------
add_custom_target(copytesting ALL
  ${CMAKE_COMMAND}
  -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/Testing/TestData
  -DDST=${CMAKE_BINARY_DIR}/${EM_SHARE_DIR}/${PROJECT_NAME}/Testing
  -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/EM_InstallTasksSubScript.cmake
  )
install(DIRECTORY
  Testing/TestData
  DESTINATION ${EM_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Testing
  FILES_MATCHING PATTERN "*"
  )

# --------------------------------------------------------------------------
if("${Slicer_MAJOR_VERSION}" EQUAL "4")
  include(CMake/EM_Slicer4_GenerateExtensionFile.cmake)
endif()
