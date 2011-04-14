cmake_minimum_required (VERSION 2.8)
project (Congealing4Slicer)
include(ExternalProject)

#http://cmake.org/gitweb?p=cmake.git;a=blob;f=Modules/ExternalProject.cmake

message("APPLE :                         ${APPLE}")
message("BORLAND :                       ${BORLAND}")
message("CMAKE_CL_64 :                   ${CMAKE_CL_64}")
message("CMAKE_COMPILER_2005 :           ${CMAKE_COMPILER_2005}")
message("CMAKE_HOST_APPLE :              ${CMAKE_HOST_APPLE}")
message("CMAKE_HOST_SYSTEM :             ${CMAKE_HOST_SYSTEM}")
message("CMAKE_HOST_SYSTEM_NAME :        ${CMAKE_HOST_SYSTEM_NAME}")
message("CMAKE_HOST_SYSTEM_PROCESSOR :   ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message("CMAKE_HOST_SYSTEM_VERSION :     ${CMAKE_HOST_SYSTEM_VERSION}")
message("CMAKE_HOST_UNIX :               ${CMAKE_HOST_UNIX}")
message("CMAKE_HOST_WIN32 :              ${CMAKE_HOST_WIN32}")
message("CMAKE_SYSTEM :                  ${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME :             ${CMAKE_SYSTEM_NAME}")
message("CMAKE_SYSTEM_PROCESSOR(target): ${CMAKE_SYSTEM_PROCESSOR}")
message("CMAKE_SYSTEM_VERSION :          ${CMAKE_SYSTEM_VERSION}")
message("CYGWIN :                        ${CYGWIN}")
message("UNIX :                          ${UNIX}")
message("WIN32 :                         ${WIN32}")
message("MSVC :                          ${MSVC}")


# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

set( MYPROJECT_SOURCE_DIR  Congealing4Slicer )
set( MYPROJECT_BINARY_DIR  Congealing4Slicer-build )
set( MYPROJECT_INSTALL_DIR Congealing4Slicer-install )

message("######### MYPROJECT_BINARY_DIR:                 ${MYPROJECT_BINARY_DIR}")
message("######### MYPROJECT_SOURCE_DIR:                 ${MYPROJECT_SOURCE_DIR}")
message("######### MYPROJECT_INSTALL_DIR:                ${MYPROJECT_INSTALL_DIR}")

message("######### Slicer3_INSTALL_PLUGINS_BIN_DIR:      ${Slicer3_INSTALL_PLUGINS_BIN_DIR}")
message("######### CMAKE_CURRENT_BINARY_DIR:             ${CMAKE_CURRENT_BINARY_DIR}")
message("######### CMAKE_CURRENT_SOURCE_DIR:             ${CMAKE_CURRENT_SOURCE_DIR}")

message("######### LOCAL_CMAKE_BUILD_OPTIONS:            ${LOCAL_CMAKE_BUILD_OPTIONS}")







if (NOT WIN32)

  set( PROJECTNAME CongealingExternal )

  ExternalProject_Add(${PROJECTNAME}
#    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    SVN_REPOSITORY "http://svn.github.com/domibel/Congealing"
    CMAKE_ARGS
    ${LOCAL_CMAKE_BUILD_OPTIONS}
    -DCMAKE_INSTALL_PREFIX:PATH=${MYPROJECT_INSTALL_DIR}
#    -DCMAKE_BUILD_TYPE:STRING=Release
    -DCMAKE_BUILD_TYPE:STRING=Debug
#   INSTALL_COMMAND ""
    )

install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECTNAME}-prefix/src/${PROJECTNAME}-build/${PROJECTNAME}-install/bin/
  DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
  FILES_MATCHING PATTERN "*"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
              GROUP_EXECUTE GROUP_READ
              WORLD_EXECUTE WORLD_READ
)

install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECTNAME}-prefix/src/${PROJECTNAME}-build/${PROJECTNAME}-install/lib/
  DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
  FILES_MATCHING PATTERN "*"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
              GROUP_EXECUTE GROUP_READ
              WORLD_EXECUTE WORLD_READ
)

endif(NOT WIN32)

