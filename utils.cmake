# utils.cmake

## 添加宏
function(add_macro targetname)
    ## 获取源文件
    get_target_property(sourcefile "${targetname}" SOURCES)

    #添相对路径的宏
    list(APPEND defs "__RELATIVE_PATH__=\"${sourcefile}\"")

    #重新设置源文件的编译参数
    set_property(
        SOURCE "${sourcefile}"
        PROPERTY COMPILE_DEFINITIONS ${defs}
    )
endfunction()


## 执行函数
function(ppcode_add_executable targetname srcs depends libs)
    ## 添加可行性文件
    add_executable(${targetname} ${srcs})
    ## 添加依赖
    add_dependencies(${targetname} ${depends})
    ## 添加宏
    add_macro(${targetname})
    ## 添加连接库
    target_link_libraries(${targetname} ${libs})
endfunction()
