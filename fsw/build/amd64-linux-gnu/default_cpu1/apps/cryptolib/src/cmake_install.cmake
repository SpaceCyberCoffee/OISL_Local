# Install script for directory: /home/jstar/Desktop/github-nos3/components/cryptolib/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/jstar/Desktop/github-nos3/fsw/build/exe")
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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so"
         RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpucpu1/cf")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cpu1/cf" TYPE MODULE FILES "/home/jstar/Desktop/github-nos3/fsw/build/amd64-linux-gnu/default_cpu1/apps/cryptolib/src/crypto.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so"
         OLD_RPATH "::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpucpu1/cf")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/crypto.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so"
         RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpucpu1/cf")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf" TYPE MODULE FILES "/home/jstar/Desktop/github-nos3/fsw/build/amd64-linux-gnu/default_cpu1/apps/cryptolib/src/crypto.so")
  if(EXISTS "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so"
         OLD_RPATH "::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpucpu1/cf")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/crypto.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto_config.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto_config_structs.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto_error.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto_events.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto_print.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/crypto_structs.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/cryptography_interface.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/key_interface.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/mc_interface.h;/home/jstar/Desktop/github-nos3/fsw/build/exe/host/sa_interface.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jstar/Desktop/github-nos3/fsw/build/exe/host" TYPE FILE FILES
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto_config.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto_config_structs.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto_error.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto_events.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto_print.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/crypto_structs.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/cryptography_interface.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/key_interface.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/mc_interface.h"
    "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../include/sa_interface.h"
    )
endif()

