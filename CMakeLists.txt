cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(shhuttled)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/)

set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 1)
set(${PROJECT_NAME}_PATCH_LEVEL 0)

set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")

set(SSHUTTLED_CONF_DIR "/etc/sshuttled")
set(SYSTEMD_UNIT_DIR "/usr/lib/systemd/system/")

file(GLOB_RECURSE common_src
  "src/daemonize.c"
  "src/log.c"
  "src/pid.c"
  "src/dirs.c"
  "src/fifo.c"
)

file(GLOB_RECURSE sshuttled_src
  "src/sshuttled.c"
)

add_executable(sshuttled ${sshuttled_src} ${common_src})
set_property(TARGET sshuttled PROPERTY C_STANDARD 11)
target_include_directories(sshuttled PRIVATE include/)

install(TARGETS sshuttled
  RUNTIME
  DESTINATION bin)

file(GLOB_RECURSE sshuttlectl_src
  "src/sshuttlectl.c"
)

add_executable(sshuttlectl ${sshuttlectl_src} ${common_src})
set_property(TARGET sshuttlectl PROPERTY C_STANDARD 11)
target_include_directories(sshuttlectl PRIVATE include/)

install(TARGETS sshuttlectl
  RUNTIME
  DESTINATION bin)

if(CMAKE_COMPILER_IS_GNUCC)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_C_FLAGS "-D_REETRANT -ggdb -fPIC -Wall -Wextra -pedantic -O0")
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set (CMAKE_C_FLAGS "-D_REETRANT -DNDEBUG -fPIC -Wall -Wextra -pedantic -O3 -flto")
  endif ()
endif(CMAKE_COMPILER_IS_GNUCC)

function(install_conf src dest)
  if(NOT IS_ABSOLUTE "${src}")
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
  endif()

  get_filename_component(src_name "${src}" NAME)

  if (NOT IS_ABSOLUTE "${dest}")
    set(dest "${CMAKE_INSTALL_PREFIX}/${dest}")
  endif()

  install(CODE "
    if(NOT EXISTS \"\$ENV{DESTDIR}${dest}/${src_name}\")
      #file(INSTALL \"${src}\" DESTINATION \"${dest}\")
      message(STATUS \"Installing: \$ENV{DESTDIR}${dest}/${src_name}\")
      execute_process(COMMAND \${CMAKE_COMMAND} -E copy \"${src}\"
                      \"\$ENV{DESTDIR}${dest}/${src_name}\"
                      RESULT_VARIABLE copy_result
                      ERROR_VARIABLE error_output)
      if(copy_result)
        message(FATAL_ERROR \${error_output})
      endif()
    else()
      message(STATUS \"Skipping  : \$ENV{DESTDIR}${dest}/${src_name}\")
    endif()
  ")
endfunction(install_conf)

install_conf(./config/sshutled.json ${SSHUTTLED_CONF_DIR})
install_conf(./systemd/sshuttled.service ${SYSTEMD_UNIT_DIR})

