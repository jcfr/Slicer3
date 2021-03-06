project(IGT)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
#find_package(KWWidgets REQUIRED)
#include(${KWWidgets_USE_FILE})
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

foreach(package 
    MRML
    )
  # This is mostly to test building this lib as standalone outside Slicer3
  if(NOT DEFINED ${package}_SOURCE_DIR)
    set(${package}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${package}")
  endif(NOT DEFINED ${package}_SOURCE_DIR)
  if(NOT DEFINED ${package}_BINARY_DIR)
    string(REPLACE ${PROJECT_NAME} ${package} 
      ${package}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    link_directories(${${package}_BINARY_DIR})
  endif(NOT DEFINED ${package}_BINARY_DIR)

  set(include_dirs ${include_dirs}
    ${${package}_SOURCE_DIR}
    ${${package}_BINARY_DIR}
    )
endforeach(package)

include_directories(${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkIGTConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkIGTConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkIGTConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(IGT_SRCS
#The following two lines are excluded by NH on Jan 10 in an
#anticipation that these have not been used in Modules.
  vtkIGTDataManager.cxx
  vtkIGTDataStream.cxx
  vtkIGTMatrixState.cxx
  vtkIGTPat2ImgRegistration.cxx
  )
set(TRACKING_INTERFACE_LIBRARY "")


if(IGT_USE_IGSTK)
  set(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTPat2ImgRegistration.cxx
    vtkIGTIGSTKStream.cxx
    )
  set(TRACKING_INTERFACE_INCLUDE_DIR
    ${IGSTK_INCLUDE_DIRS}
    )
  set(TRACKING_INTERFACE_LIB  
    ${IGSTK_LIBRARIES}
    )
endif(IGT_USE_IGSTK)

# --------------------------------------------------------------------------
# Wrapping
IF (VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(IGT IGT_TCL_SRCS "${IGT_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name IGT)
SET(srcs
  ${IGT_SRCS} 
  )
IF (VTK_WRAP_TCL)
  SET(srcs
    ${srcs}
    ${IGT_TCL_SRCS}
    )
ENDIF (VTK_WRAP_TCL)
add_library(${lib_name}
  ${srcs}
  )

SET(libs
  MRML
  ${TRACKING_INTERFACE_LIB}
)
IF (VTK_WRAP_TCL)
  # Complain if KWWidget is not available
  IF(NOT KWWidgets_FOUND)
    MESSAGE(FATAL_ERROR "IGT required KWWidgets to build")
  ELSE(NOT KWWidgets_FOUND)
    ADD_DEFINITIONS(-DKWWidgets_FOUND)
  ENDIF(NOT KWWidgets_FOUND)
  SET(libs
    ${libs}
    ${KWWidgets_LIBRARIES}
    )
ENDIF (VTK_WRAP_TCL)
target_link_libraries(${lib_name}
  ${libs}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)


