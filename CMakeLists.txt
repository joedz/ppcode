cmake_minimum_required(VERSION 3.0)
project(ppcode)

include(./utils.cmake)



set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

set(LIB_SRC

##  log src
    src/log/log_appender.cc
    src/log/log_event.cc
    src/log/log_level.cc
    src/log/log_format.cc
    src/log/logger.cc
    src/log/log_manager.cc

## thread src
    src/thread/barrier.cc
    src/thread/condition_cariable.cc
    src/thread/mutex.cc
    src/thread/rwmutex.cc
    src/thread/semaphore.cc
    src/thread/spinlock.cc
    src/thread/thread.cc
    src/thread/thread_attributes.cc

## util src
    src/util/util.cc


## os src
    src/os/os_linux.cc
    src/os/os_window.cc
)

add_library(ppcode SHARED ${LIB_SRC})

set(LIBS 
    ppcode
    pthread
)

add_definitions(-Wno-builtin-macro-redefined)

ppcode_add_executable(test_log "test/test_log.cc" ppcode "${LIBS}")
ppcode_add_executable(test_cmake "test/test_cmake.cc" ppcode "${LIBS}")
ppcode_add_executable(test_thread "test/test_thread.cc" ppcode "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)