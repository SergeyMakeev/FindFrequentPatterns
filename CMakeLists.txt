cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJ_NAME FindFrequentPatterns)
project(${PROJ_NAME})

set (CMAKE_CXX_STANDARD 17)


set(BENCH_SOURCES 
  main.cpp
)
set (EXE_NAME ${PROJ_NAME})
add_executable(${EXE_NAME} ${BENCH_SOURCES})

if(MSVC)
  target_compile_options(${EXE_NAME} PRIVATE "$<$<CONFIG:Release>:/Zi>")
  target_link_options(${EXE_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
  target_link_options(${EXE_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
  target_link_options(${EXE_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})
endif()


