include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/erc_debug.cmake)

erc_debug_info(erc_resource.cmake)

function(erc_resource target asset_root)
    # 参数
    erc_debug("[erc_resource] called; argv: target=${target}, asset_root=${asset_root}")
    if(NOT TARGET ${target})
        message(FATAL_ERROR "[erc_resource] 目标 '${target}' 不存在")
    endif()

    # 获取 erc-compiler
    get_property(_erc_compiler_exe GLOBAL PROPERTY ERC_COMPILER_EXE)
    if(NOT _erc_compiler_exe)
        message(FATAL_ERROR "[erc_resource] ERC_COMPILER_EXE 未设置")
    endif()
    erc_debug("[erc_resource] compiler exe = ${_erc_compiler_exe}")

    # 创建 res_target 接口对象
    set(_res_target "${target}.erc_resource")
    if(TARGET ${_res_target})
        message(FATAL_ERROR "[erc_resource] 无法生成erc目标，期望目标 '${_res_target}' 已存在")
    endif()

    add_library(${_res_target} INTERFACE)
    target_link_libraries(${_res_target} INTERFACE erc)
    target_link_libraries(${target} PRIVATE ${_res_target})
    erc_debug("[erc_resource] 创建 ${_res_target}")

    # 收集 target 的源文件
    set(_src_list_dir "${CMAKE_BINARY_DIR}/erc/src.list")
    file(MAKE_DIRECTORY "${_src_list_dir}")
    set(_src_list_file "${_src_list_dir}/${target}.list")

    get_target_property(_sources ${target} SOURCES)
    if(NOT _sources)
        erc_debug("[erc_resource](${target}) target 不存在 SOURCES")
    endif()
    erc_debug("[erc_resource](${target}) target SOURCES = ${_sources}")

    get_target_property(_src_dir ${target} SOURCE_DIR)
    if(NOT _src_dir)
        erc_debug("[erc_resource](${target}) target 不存在 SOURCE_DIR")
        set(_src_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    erc_debug("[erc_resource](${target}) target SOURCE_DIR = ${_src_dir}")

    set(_filtered "")
    foreach(_s IN LISTS _sources)
        if(IS_ABSOLUTE "${_s}")
            set(_abs "${_s}")
        else()
            set(_abs "${_src_dir}/${_s}")
        endif()
        get_filename_component(_abs "${_abs}" ABSOLUTE)
        if(EXISTS "${_abs}" AND NOT IS_DIRECTORY "${_abs}")
            #[[
            get_filename_component(_ext "${_abs}" EXT)
            if(_ext MATCHES "^\\.(h|hh|hpp|hxx|h\\+\\+|c|cc|cpp|cxx|ixx|m|mm)$")
                list(APPEND _filtered "${_abs}")
                erc_debug("[erc_resource]   收集 ${_abs}")
            else()
                erc_debug("[erc_resource]   跳过(非源码) ${_abs}")
            endif()
            ]]
            list(APPEND _filtered "${_abs}")
            erc_debug("[erc_resource](${target}) 收集源文件 ${_abs}")
        else()
            erc_debug("[erc_resource](${target}) 跳过(不存在/不合法) ${_abs}")
        endif()
    endforeach()
    list(LENGTH _filtered _filtered_len)
    erc_debug("[erc_resource](${target}) 收集到 ${_filtered_len} 个文件")

    ## 写出
    file(WRITE "${_src_list_file}" "")
    foreach(_f IN LISTS _filtered)
        file(APPEND "${_src_list_file}" "${_f}\n")
    endforeach()
    erc_debug("[erc_resource](${target}) 写出列表到 ${_src_list_file}")

    # 运行 erc-compiler
    set(_erc_out_dir "${CMAKE_BINARY_DIR}/erc/autogen.src/${_res_target}")
    file(MAKE_DIRECTORY "${_erc_out_dir}")

    erc_debug("[erc_resource] >>> 执行:")
    erc_debug("[erc_resource] exe = ${_erc_compiler_exe}")
    erc_debug("[erc_resource]     --asset-root = ${asset_root}")
    erc_debug("[erc_resource]     --out-dir    = ${_erc_out_dir}")
    erc_debug("[erc_resource]     --src-list   = ${_src_list_file}")

    execute_process(
        COMMAND "${_erc_compiler_exe}"
            --asset-root "${asset_root}"
            --out-dir    "${_erc_out_dir}"
            --src-list   "${_src_list_file}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE  _err
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    erc_debug("[erc_resource] rc = ${_rc}")
    if(_out)
        erc_debug("[erc_resource] stdout:\n${_out}")
    endif()
    if(_err)
        erc_debug("[erc_resource] stderr:\n${_err}")
    endif()

    if(NOT _rc STREQUAL "0")
        message(FATAL_ERROR
            "[erc_resource] erc-compiler 执行失败 (rc=${_rc})\n"
            "  exe=${_erc_compiler_exe}\n"
            "  --asset-root=${asset_root}\n"
            "  --out-dir   =${_erc_out_dir}\n"
            "  --src-list  =${_src_list_file}\n"
            "  @stdout:\n${_out}\n"
            "  @stderr:\n${_err}"
        )
    endif()

    # 收集 erc-compiler 生成结果
    file(GLOB_RECURSE _autogen_files CONFIGURE_DEPENDS "${_erc_out_dir}/*")
    list(LENGTH _autogen_files _autogen_len)
    erc_debug("[erc_resource] 生成目录 = ${_erc_out_dir}")
    erc_debug("[erc_resource] 生成文件数 = ${_autogen_len}")
    foreach(_f IN LISTS _autogen_files)
        erc_debug("[erc_resource] 生成文件: ${_f}")
    endforeach()

    if(_autogen_files)
        target_sources(${_res_target} INTERFACE ${_autogen_files})
        erc_debug("[erc_resource] 已加入生成文件到 ${_res_target}")
    else()
        erc_debug("[erc_resource] 生成文件为空，跳过")
    endif()

    # 完成
    erc_debug("[erc_resource] 处理 ${target} 完成")
endfunction()
