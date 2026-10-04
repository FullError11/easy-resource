include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/erc_debug.cmake)

erc_debug_info(erc_resource.cmake)

if (NOT TARGET erc-compiler)
    set(_erc_compiler_src   "${CMAKE_CURRENT_LIST_DIR}/../compiler")
    set(_erc_compiler_build "${CMAKE_BINARY_DIR}/erc_compiler")

    erc_debug("[erc] erc-compiler 源目录 : ${_erc_compiler_src}")
    erc_debug("[erc] erc-compiler 构建目录: ${_erc_compiler_build}")

    if (NOT EXISTS "${_erc_compiler_src}/CMakeLists.txt")
        message(FATAL_ERROR "[erc] 找不到 ${_erc_compiler_src}/CMakeLists.txt")
    endif()

    # cmake 配置
    set(_erc_cfg_args
        -S "${_erc_compiler_src}"
        -B "${_erc_compiler_build}"
    )
    if (ERC_DEBUG)
        list(APPEND _erc_cfg_args -DCMAKE_BUILD_TYPE="Debug")
    else ()
        list(APPEND _erc_cfg_args -DCMAKE_BUILD_TYPE="Release")
    endif()

    erc_debug("[erc] >>> configure: ${CMAKE_COMMAND} ${_erc_cfg_args}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${_erc_cfg_args}
        RESULT_VARIABLE _erc_cfg_rc
        OUTPUT_VARIABLE _erc_cfg_out
        ERROR_VARIABLE  _erc_cfg_err
    )

    erc_debug("[erc] configure rc = ${_erc_cfg_rc}")
    if(_erc_cfg_out)
        message("[erc] configure stdout:\n${_erc_cfg_out}")
    endif()
    if(_erc_cfg_err)
        message("[erc] configure stderr:\n${_erc_cfg_err}")
    endif()

    if(NOT _erc_cfg_rc STREQUAL "0")
        message(FATAL_ERROR "[erc] erc-compiler 配置失败 (rc=${_erc_cfg_rc})")
    endif()

    # cmake 编译
    set(_erc_build_args --build "${_erc_compiler_build}" --target erc-compiler)
    erc_debug("[erc] >>> build: ${CMAKE_COMMAND} ${_erc_build_args}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${_erc_build_args}
        RESULT_VARIABLE _erc_build_rc
        OUTPUT_VARIABLE _erc_build_out
        ERROR_VARIABLE  _erc_build_err
    )

    erc_debug("[erc] build rc = ${_erc_build_rc}")
    if(_erc_build_out)
        message("[erc] build stdout:\n${_erc_build_out}")
    endif()
    if(_erc_build_err)
        message("[erc] build stderr:\n${_erc_build_err}")
    endif()

    if(NOT _erc_build_rc STREQUAL "0")
        message(FATAL_ERROR "[erc] erc-compiler 构建失败 (rc=${_erc_build_rc})")
    endif()

    # 定位 erc-compiler 可执行文件
    set(_erc_compiler_exe
        "${_erc_compiler_build}/bin/erc-compiler${CMAKE_EXECUTABLE_SUFFIX}"
    )

    if(NOT EXISTS "${_erc_compiler_exe}")
        # 兜底：在整个构建目录里搜
        file(GLOB_RECURSE _erc_candidates "${_erc_compiler_build}/erc-compiler*")
        erc_debug("[erc] 兜底搜索候选: ${_erc_candidates}")
        foreach(_c IN LISTS _erc_candidates)
            if(EXISTS "${_c}" AND NOT IS_DIRECTORY "${_c}")
                set(_erc_compiler_exe "${_c}")
                break()
            endif()
        endforeach()
    endif()

    if(NOT EXISTS "${_erc_compiler_exe}")
        message(FATAL_ERROR "[erc] 找不到 erc-compiler 可执行文件")
    endif()
    erc_debug("[erc] erc-compiler exe = ${_erc_compiler_exe}")

    # 导出 erc-compiler 路径到cmake全局属性
    set_property(GLOBAL PROPERTY ERC_COMPILER_EXE "${_erc_compiler_exe}")

    add_executable(erc-compiler IMPORTED GLOBAL)
    set_target_properties(erc-compiler PROPERTIES IMPORTED_LOCATION "${_erc_compiler_exe}")
else()
    erc_debug("[erc] cmake target(erc-compiler) 已存在，跳过构建")
endif()
