cmake_minimum_required(VERSION 3.0)
project(ppcode)

include(./utils.cmake)

link_directories( /usr/local/lib64 )


execute_process(COMMAND "${PROJECT_SOURCE_DIR}/code/select_asm.sh" "${PROJECT_SOURCE_DIR}" "jump" OUTPUT_VARIABLE jump_asm_file)
execute_process(COMMAND "${PROJECT_SOURCE_DIR}/code/select_asm.sh" "${PROJECT_SOURCE_DIR}" "make" OUTPUT_VARIABLE make_asm_file)


set(CMAKE_VERBOSE_MAKEFILE ON)

## g++ 编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -fPIC -g -std=c++14 -Wall  -mcx16 -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


set(LIB_SRC

## config src
    src/config/config.cc

## fiber src
    src/fiber/hook.cc
    src/fiber/fiber.cc
    src/fiber/timer.cc
    src/fiber/processer.cc
    src/fiber/poller.cc
    src/fiber/scheduler.cc
    src/fiber/fdcontext.cc


##  log src
    src/log/log_appender.cc
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

## os srcs
    src/os/os_linux.cc
    src/os/os_window.cc


    ${make_asm_file}
    ${jump_asm_file}
)

ENABLE_LANGUAGE( ASM)

add_library(ppcode SHARED ${LIB_SRC})


set(LIBS 
    ppcode
    dl
    pthread
    yaml-cpp
    boost_thread
    boost_system
)

#add_definitions(-Wno-builtin-macro-redefined)

### 生成可执行文件自定义函数 

ppcode_add_executable(test_log "test/test_log.cc" ppcode "${LIBS}")
ppcode_add_executable(test_cmake "test/test_cmake.cc" ppcode "${LIBS}")
ppcode_add_executable(test_thread "test/test_thread.cc" ppcode "${LIBS}")
ppcode_add_executable(test_lockfreelist "test/test_lockfreelist.cc" ppcode "${LIBS}")
ppcode_add_executable(test_yaml "test/test_yaml.cc" ppcode "${LIBS}")
ppcode_add_executable(test_config "test/test_config.cc" ppcode "${LIBS}")
#ppcode_add_executable(test_coroutines "test/test_coroutines.cc" ppcode "${LIBS}")
ppcode_add_executable(test_sche "test/test_sche.cc" ppcode "${LIBS}")
ppcode_add_executable(test_hook "test/test_hook.cc" ppcode "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)