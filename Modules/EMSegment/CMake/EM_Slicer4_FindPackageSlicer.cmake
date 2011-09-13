
set(EXTENSION_NAME EMSegment)

#
# Note: If we are outside Slicer4's source-tree we assume that we build an extension.
#

if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
  set(EXTENSION_CATEGORY "Segmentation")
  set(EXTENSION_STATUS "Beta")
  set(EXTENSION_DESCRIPTION "This is an example of Qt loadable module built as an extension")
  set(EXTENSION_DEPENDS NA) # Specified as a space separated list or 'NA' if any

  find_package(Slicer REQUIRED)

  # Additional C/CXX flags - Should be defined before including Slicer_USE_FILE
  set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} C Flags")
  set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} CXX Flags")

  include(${Slicer_USE_FILE})

  set(EXTENSION_LICENSE_FILE ${Slicer_LICENSE_FILE})
  set(EXTENSION_README_FILE ${Slicer_README_FILE})

  include(SlicerEnableExtensionTesting)
else()
  set(EMSegment_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  set(EMSegment_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

SET(EM_SHARE_DIR ${Slicer_QTLOADABLEMODULES_SHARE_DIR})
SET(EM_INSTALL_SHARE_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR})  
