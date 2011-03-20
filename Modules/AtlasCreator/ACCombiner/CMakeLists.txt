project(ACCombiner)
set(LOCAL_PROJECT_NAME ACCombiner)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

if(NOT ITK_FOUND)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

ADD_EXECUTABLE(ACCombiner Combiner.cxx )
TARGET_LINK_LIBRARIES(ACCombiner ITKIO)
