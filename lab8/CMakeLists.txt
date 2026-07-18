cmake_minimum_required(VERSION 3.22)

project(
  Lab8
  VERSION 1.0
  DESCRIPTION "This is an example for the lab."
  LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)

# Note: ASan and TSan are incompatible
# so one of them has to be commented out

# Address sanitization:
# add_compile_options(-fsanitize=address) 
# add_link_options(-fsanitize=address)
# --------- Task 3 ---------
# Enable the thread sanitizer 
# TODO
# whenever CMake configure this project, thread sanitizer is enabled which detects data races and
# gives userfule source-line info
# also, sanitizer needs to appear at both compile and link stage
add_compile_options(-fsanitize=thread -g -O1)
add_link_options(-fsanitize=thread -g -O1)

#these are not sanitizer, but for POSIX thread support, they are required since we use functions
#such as pthread_create, pthread_join
add_compile_options(-pthread)
add_link_options(-pthread)

#the target name is main, and the source file is src/lab8.c
add_executable(main src/lab8.c)
#means add this include dir specifically to main, while include_directories(include) means addthis
#include dir broadly for targets in this CMake dir, e.g.  if we have add_exec(tests
#tests/test.c),both main and tests receive the include search path
#PRIVATE means the include dir is needed when compiling main, but it should not be passed to another
#target that links against main, i.e. used only to build the main target
target_include_directories(main PRIVATE include)

