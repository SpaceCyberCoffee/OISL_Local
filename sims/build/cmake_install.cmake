# Install script for directory: /home/jstar/Desktop/github-nos3/sims

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/jstar/Desktop/github-nos3/sims/build")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xconfigx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE FILES
    "/home/jstar/Desktop/github-nos3/sims/../cfg/build/sims/nos_engine_server_config.json"
    "/home/jstar/Desktop/github-nos3/sims/../cfg/build/sims/nos3-simulator.xml"
    "/home/jstar/Desktop/github-nos3/sims/../cfg/build/sims/sim_log_config.xml"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/jstar/Desktop/github-nos3/sims/build/sim_common/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/nos_time_driver/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/sim_terminal/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/truth_42_sim/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/arducam/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_css/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_eps/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_fss/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_imu/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_mag/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_radio/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_reaction_wheel/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_star_tracker/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/generic_torquer/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/novatel_oem615/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/oisl/cmake_install.cmake")
  include("/home/jstar/Desktop/github-nos3/sims/build/sample/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/jstar/Desktop/github-nos3/sims/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
