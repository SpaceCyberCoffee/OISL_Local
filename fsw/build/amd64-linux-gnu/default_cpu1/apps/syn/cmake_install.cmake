# Install script for directory: /mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/components/syn/synopsis

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/amd64-linux-gnu/default_cpu1/apps/syn/json/cmake_install.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsynopsis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsynopsis.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsynopsis.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/amd64-linux-gnu/default_cpu1/apps/syn/libsynopsis.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsynopsis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsynopsis.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsynopsis.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/components/syn/synopsis/include/synopsis.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf" TYPE SHARED_LIBRARY FILES "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/amd64-linux-gnu/default_cpu1/apps/syn/libsynopsis.so")
  if(EXISTS "$ENV{DESTDIR}/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/libsynopsis.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf/synopsis.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/fsw/build/exe/cpu1/cf" TYPE FILE FILES "/mnt/extras/SSD/NOS3_RBT/nos3_local_OISL/components/syn/synopsis/include/synopsis.hpp")
endif()

