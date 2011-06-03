PROJECT(qSlicer${qt_module_name}ModuleWidgets)

#
# See CMake/SlicerMacroBuildModuleWidgets.cmake for details
#

SET(module_widgets_name "${PROJECT_NAME}")

# The header '${module_widgets_name}Export.h' will be automatically configured.
SET(module_widgets_export_directive "Q_SLICER_MODULE_EMSEGMENT_WIDGET_EXPORT")


# Additional directories to include
SET(module_widgets_include_directories
  )

# Source files
SET(module_widgets_SRCS
  
  qSlicerEMSegmentWidget.cxx  
  qSlicerEMSegmentWidget.h
  
  qSlicerEMSegmentInputChannelListWidget.h
  qSlicerEMSegmentInputChannelListWidget.cxx
  
  qSlicerEMSegmentGraphWidget.h
  qSlicerEMSegmentGraphWidget.cxx
  
  qSlicerEMSegmentAnatomicalTreeWidget.h
  qSlicerEMSegmentAnatomicalTreeWidget.cxx
  qSlicerEMSegmentAnatomicalTreeWidget_p.h
  
  vtkPlotGaussian.h
  vtkPlotGaussian.cxx
  )

# Headers that should run through moc
SET(module_widgets_MOC_SRCS
  qSlicerEMSegmentAnatomicalTreeWidget.h
  qSlicerEMSegmentAnatomicalTreeWidget_p.h
  qSlicerEMSegmentGraphWidget.h
  qSlicerEMSegmentInputChannelListWidget.h
  qSlicerEMSegmentWidget.h
  )
  
# UI files
SET(module_widgets_UI_SRCS
  Resources/UI/qSlicerEMSegmentAnatomicalTreeWidget.ui
  Resources/UI/qSlicerEMSegmentGraphWidget.ui  
  )

# Resources
SET(module_widgets_resources
  )

# Additional Target libraries
SET(module_widgets_target_libraries
  vtkSlicerEMSegmentModuleLogic
  )

#SlicerMacroBuildModuleWidgets(
#  NAME ${module_widgets_name}
#  EXPORT_DIRECTIVE ${module_widgets_export_directive}
#  INCLUDE_DIRECTORIES ${module_widgets_include_directories}
#  SRCS ${module_widgets_SRCS}
#  MOC_SRCS ${module_widgets_MOC_SRCS}
#  UI_SRCS ${module_widgets_UI_SRCS}
#  TARGET_LIBRARIES ${module_widgets_target_libraries}
#  RESOURCES ${module_widgets_resources}
#  )
  
#SET(lib_name ${PROJECT_NAME})

  
#SET(KIT_PYTHONQT_SRCS) # Clear variable
#ctkMacroWrapPythonQt("org.slicer.libs" ${lib_name}
#  KIT_PYTHONQT_SRCS "${module_widgets_SRCS}" FALSE)
#ADD_LIBRARY(${lib_name}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
#TARGET_LINK_LIBRARIES(${lib_name}PythonQt ${lib_name})
#IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
#  SET_TARGET_PROPERTIES(${lib_name}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
#ENDIF()  


















QT4_WRAP_CPP(module_widgets_MOC_CXX ${module_widgets_MOC_SRCS})
QT4_WRAP_UI(module_widgets_UI_CXX ${module_widgets_UI_SRCS})
QT4_ADD_RESOURCES(module_widgets_QRC_CXX ${module_widgets_QRC_SRCS})

SOURCE_GROUP("Resources" FILES
  ${module_widgets_UI_SRCS}
  ${module_widgets_QRC_SRCS}
  )

SOURCE_GROUP("Generated" FILES
  ${module_widgets_MOC_CXX}
  ${module_widgets_UI_CXX}
  ${module_widgets_QRC_CXX}
  )

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
INCLUDE_DIRECTORIES(${include_dirs})

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
SET(lib_name ${PROJECT_NAME})

ADD_LIBRARY(${lib_name}
  ${module_widgets_SRCS}
  ${module_widgets_MOC_CXX}
  ${module_widgets_UI_CXX}
  ${module_widgets_QRC_CXX}
  )

SET(module_widgets_LIBRARIES
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  ${CTK_EXTERNAL_LIBRARIES}
  vtkSlicerEMSegmentModuleLogic
  )

TARGET_LINK_LIBRARIES(${lib_name} ${module_widgets_LIBRARIES})


# --------------------------------------------------------------------------
# PythonQt wrapping
# --------------------------------------------------------------------------
SET(module_widgets_WRAP_PYTHON ON)
IF(module_widgets_WRAP_PYTHON)
  SET(KIT_PYTHONQT_SRCS) # Clear variable
  ctkMacroWrapPythonQt("org.slicer.libs" ${lib_name}
    KIT_PYTHONQT_SRCS "${module_widgets_SRCS}" FALSE)
  ADD_LIBRARY(${lib_name}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
  TARGET_LINK_LIBRARIES(${lib_name}PythonQt ${lib_name})
  IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    SET_TARGET_PROPERTIES(${lib_name}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
  ENDIF()
ENDIF()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  SET(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
ENDIF()
EXPORT(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )
















# Plugin
ADD_SUBDIRECTORY(Plugins)

if(BUILD_TESTING)
  SET(EMSegment_TESTING_DATA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Testing/Data)
  #ADD_SUBDIRECTORY(Testing)
endif()

