
# --------------------------------------------------------------------------
# Module name (usually identique to the directory name)
SET(qt_module_name "EMSegment")

# Add subdirectories

# Logic
ADD_SUBDIRECTORY(Logic)

# Qt Widgets
ADD_SUBDIRECTORY(Widgets)

# The Python-based EMSegment wizard
ADD_SUBDIRECTORY(Py)

SET(qt_module_export_directive "Q_SLICER_QTMODULES_EMSEGMENT_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )


# Source files
SET(qt_module_SRCS
  qSlicerEMSegmentModule.cxx
  qSlicerEMSegmentModule.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerEMSegmentModule.h
  )

# UI files
SET(qt_module_UI_SRCS
  )

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerEMSegmentModuleLogic
  qSlicerEMSegmentModuleWidgets
  qSlicerEMSegmentModuleWidgetsPythonQt
  vtkCharts
  MRMLLogic
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerEMSegmentModule.qrc
  )

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

if(BUILD_TESTING)
  SET(EMSegment_TESTING_DATA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Testing/Data)

  #ADD_SUBDIRECTORY(Testing)
endif()

# --------------------------------------------------------------------------
# Module name
SET(qt_module_name "EMSegmentQuick")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_EMSEGMENTQUICK_EXPORT")

# Source files
SET(qt_module_SRCS
  qSlicerEMSegmentQuickModule.cxx
  qSlicerEMSegmentQuickModule.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerEMSegmentQuickModule.h
  )

# UI files
SET(qt_module_UI_SRCS
  )

# Resources
SET(qt_module_resources
  #Resources/qSlicerEMSegmentModule.qrc
  )
slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  NO_TITLE # See note below
  )

# Note: Usually, one module per directory is defined. Trying to build two modules from the same
#       directory will lead to a multiple definition warning related to QTMODULE_TITLE macro.
#       By passing the option NO_TITLE, the macro won't be re-define and we will avoid the issue.
#       Doing so implies that we directly hardcode the title in qSlicerEMSegmentQuickModule.h
