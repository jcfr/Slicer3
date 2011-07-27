#
# automatically detect if we build for Slicer3 or Slicer4
#

#
# as a result the CMake variable EM_Slicer4_FOUND and EM_Slicer3_FOUND will be set accordingly
# also, a message stating for which application we configure is printed
#

if(Slicer3_SOURCE_DIR)
  project(EMSegment)
  # this is Slicer3
  SET(EM_Slicer3_FOUND ON)
  SET(EM_Slicer4_FOUND OFF)
  MESSAGE(STATUS "Configuring EMSegment for Slicer3")
else()
  # this is Slicer4
  SET(EM_Slicer4_FOUND ON)
  SET(EM_Slicer3_FOUND OFF)
  MESSAGE(STATUS "Configuring EMSegment for Slicer4")
endif()

