cmake_minimum_required ( VERSION 3.1.0 )

project ( mortimer
  VERSION 0.1.0
  DESCRIPTION "An interactive, structured timer for the command line"
  LANGUAGES CXX
)

set ( CMAKE_CXX_STANDARD 11 )
set ( CMAKE_CXX_STANDARD_REQUIRED True )

add_executable ( mortimer src/mortimer.cpp )
