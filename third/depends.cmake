
file(GLOB_RECURSE third_cmake_files "${CMAKE_CURRENT_LIST_DIR}/*/cmake/*.cmake")

foreach(cmake_file ${third_cmake_files})
    if(NOT cmake_file STREQUAL CMAKE_CURRENT_LIST_FILE)
        include(${cmake_file})
    endif()
endforeach()

include(third/config.cmake)

function(create_individual_install_target target_name)
    # message(STATUS "Creating individual install target for: ${target_name}")
    
    set(INSTALL_TARGET_NAME "INSTALL_${target_name}")
    
    add_custom_target(${INSTALL_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${target_name} --config Debug
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${target_name} --config Release
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component ${target_name} --config Debug
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component ${target_name} --config Release
        COMMENT "Installing ${target_name} as third-party library (Debug + Release)"
        VERBATIM
    )
    
    add_dependencies(${INSTALL_TARGET_NAME} ${target_name})
    
    if(MSVC)
        set_target_properties(${INSTALL_TARGET_NAME} PROPERTIES
            FOLDER "Install"
        )
    endif()
    
    # message(STATUS "Created install target: ${INSTALL_TARGET_NAME}")
endfunction(create_individual_install_target)

function(add_third TARGET_NAME DEP_NAME)
    string(TOUPPER ${DEP_NAME} DEP_NAME_UPPER)
    set(DEP_TARGET_VAR "DEP_${DEP_NAME_UPPER}_TARGETS")

    if(NOT DEFINED ${DEP_TARGET_VAR})
        message(WARNING "Dependency [${DEP_NAME}] is not defined or found. Please check third/depends.cmake")
    endif()

    # message(STATUS "Linking dependency [${DEP_NAME}] to target [${TARGET_NAME}]")
    target_link_libraries(${TARGET_NAME} PUBLIC ${${DEP_TARGET_VAR}})
    if(MSVC)
        set(DEP_INCLUDE_VAR "DEP_${DEP_NAME_UPPER}_INCLUDE_PATHS")
        if(DEFINED ${DEP_INCLUDE_VAR})
            target_include_directories(${TARGET_NAME} PUBLIC ${${DEP_INCLUDE_VAR}})
            
            get_target_property(CURRENT_VS_PATHS ${TARGET_NAME} VS_GLOBAL_IncludePath)
            if(CURRENT_VS_PATHS)
                set_target_properties(${TARGET_NAME} PROPERTIES
                    VS_GLOBAL_IncludePath "${CURRENT_VS_PATHS};${${DEP_INCLUDE_VAR}}"
                )
            else()
                set_target_properties(${TARGET_NAME} PROPERTIES
                    VS_GLOBAL_IncludePath "${${DEP_INCLUDE_VAR}};$(IncludePath)"
                )
            endif()
            
            # message(STATUS "Added VS IntelliSense paths for [${DEP_NAME}]: ${${DEP_INCLUDE_VAR}}")
        endif()
    endif()
endfunction(add_third)

function(setup_algorithm_target_folder target_name)
    if(MSVC)
        set_target_properties(${target_name} PROPERTIES FOLDER "exec")
    endif()
endfunction(setup_algorithm_target_folder)

function(install_module_third_party_dlls target_name install_prefix)
    if(NOT BUILD_SHARED)
        return()
    endif()
    
    # message(STATUS "Setting up DLL copy for target: ${target_name}")
    
    get_target_property(LINKED_LIBS ${target_name} LINK_LIBRARIES)
    if(LINKED_LIBS)
        set(processed_libs "")
        
        foreach(lib ${LINKED_LIBS})
            if(TARGET ${lib})
                get_target_property(lib_type ${lib} TYPE)
                if(lib_type STREQUAL "INTERFACE_LIBRARY")
                    get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
                    if(lib_include_dirs)
                        foreach(include_dir ${lib_include_dirs})
                            string(REGEX MATCH "${CMAKE_SOURCE_DIR}/third/([^/]+)" match_result ${include_dir})
                            if(match_result)
                                set(third_lib_name ${CMAKE_MATCH_1})
                                
                                if(third_lib_name IN_LIST processed_libs)
                                    break()
                                endif()
                                list(APPEND processed_libs ${third_lib_name})
                                
                                set(third_lib_dir "${CMAKE_SOURCE_DIR}/third/${third_lib_name}")
                                
                                file(GLOB release_dlls "${third_lib_dir}/bin/*.dll")
                                if(release_dlls)
                                    install(FILES ${release_dlls}
                                        DESTINATION ${install_prefix}/bin
                                        CONFIGURATIONS Release RelWithDebInfo
                                        COMPONENT ${target_name}
                                    )
                                    # message(STATUS "[${target_name}] Will install Release DLLs from ${third_lib_name}: ${release_dlls}")
                                endif()
                                
                                file(GLOB debug_dlls "${third_lib_dir}/debug/bin/*.dll")
                                if(debug_dlls)
                                    install(FILES ${debug_dlls}
                                        DESTINATION ${install_prefix}/debug/bin
                                        CONFIGURATIONS Debug
                                        COMPONENT ${target_name}
                                    )
                                    # message(STATUS "[${target_name}] Will install Debug DLLs from ${third_lib_name}: ${debug_dlls}")
                                endif()
                                
                                break()
                            endif()
                        endforeach()
                    endif()
                endif()
            endif()
        endforeach()
    endif()
endfunction(install_module_third_party_dlls)

function(install_as_third_party target_name)
    # message(STATUS "Installing as third-party library: ${target_name}")
    
    set(INSTALL_PREFIX "${target_name}")
    
    install(DIRECTORY include/
        DESTINATION ${INSTALL_PREFIX}/include
        COMPONENT ${target_name}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${INSTALL_PREFIX}/include
        COMPONENT ${target_name}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    
    if(BUILD_SHARED)
        install(TARGETS ${target_name}
            RUNTIME DESTINATION ${INSTALL_PREFIX}/$<IF:$<CONFIG:Debug>,debug/bin,bin>
            COMPONENT ${target_name}
        )
        install(TARGETS ${target_name}
            LIBRARY DESTINATION ${INSTALL_PREFIX}/$<IF:$<CONFIG:Debug>,debug/lib,lib>
            COMPONENT ${target_name}
        )
        install(TARGETS ${target_name}
            ARCHIVE DESTINATION ${INSTALL_PREFIX}/$<IF:$<CONFIG:Debug>,debug/lib,lib>
            COMPONENT ${target_name}
        )
        
        if(MSVC)
            install(FILES $<TARGET_PDB_FILE:${target_name}>
                DESTINATION ${INSTALL_PREFIX}/$<IF:$<CONFIG:Debug>,debug/bin,bin>
                COMPONENT ${target_name}
                OPTIONAL
            )
        endif()
    else()
        install(TARGETS ${target_name}
            ARCHIVE DESTINATION ${INSTALL_PREFIX}/$<IF:$<CONFIG:Debug>,debug/lib,lib>
            COMPONENT ${target_name}
        )
    endif()
    
    install_module_third_party_dlls(${target_name} ${INSTALL_PREFIX})
    
    string(TOUPPER ${target_name} TARGET_NAME_UPPER)
    string(TOLOWER ${target_name} target_name_lower)
    
    set(cmake_template_content "# Auto-generated cmake file for ${target_name}
set(THIRDPARTY_DIR \${CMAKE_SOURCE_DIR}/third)
set(${TARGET_NAME_UPPER}_INCLUDE_DIR \${THIRDPARTY_DIR}/${target_name})

if(EXISTS \"\${${TARGET_NAME_UPPER}_INCLUDE_DIR}\")
    message(STATUS \"Found installed library: ${target_name}. Configuring...\")
    
    add_library(${target_name_lower} INTERFACE IMPORTED)
    target_include_directories(${target_name_lower} INTERFACE
        \"\${${TARGET_NAME_UPPER}_INCLUDE_DIR}/include\"
    )
    
    set(${TARGET_NAME_UPPER}_LIB_DEBUG_DIR \${${TARGET_NAME_UPPER}_INCLUDE_DIR}/debug/lib)
    set(${TARGET_NAME_UPPER}_LIB_RELEASE_DIR \${${TARGET_NAME_UPPER}_INCLUDE_DIR}/lib)
    
    file(GLOB release_libs \"\${${TARGET_NAME_UPPER}_LIB_RELEASE_DIR}/*.lib\")
    file(GLOB debug_libs \"\${${TARGET_NAME_UPPER}_LIB_DEBUG_DIR}/*.lib\")
    
    set(link_libs \"\")
    foreach(lib \${release_libs})
        list(APPEND link_libs \"\$<\$<NOT:\$<CONFIG:Debug>>:\${lib}>\")
    endforeach()
    foreach(lib \${debug_libs})
        list(APPEND link_libs \"\$<\$<CONFIG:Debug>:\${lib}>\")
    endforeach()
    
    if(link_libs)
        target_link_libraries(${target_name_lower} INTERFACE \${link_libs})
    endif()
    
    set(DEP_${TARGET_NAME_UPPER}_TARGETS ${target_name_lower})
    set(DEP_${TARGET_NAME_UPPER}_INCLUDE_PATHS \"\${${TARGET_NAME_UPPER}_INCLUDE_DIR}/include\")
    
    message(STATUS \"${target_name} library configured successfully\")
else()
    message(WARNING \"${target_name} library not found\")
endif()
")
    
    set(cmake_temp_file "${CMAKE_BINARY_DIR}/${target_name}.cmake")
    
    if(EXISTS "${cmake_temp_file}")
        file(REMOVE "${cmake_temp_file}")
    endif()
    
    file(WRITE "${cmake_temp_file}" "${cmake_template_content}")
    
    install(CODE "
        set(cmake_install_dir \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_PREFIX}/cmake\")
        set(cmake_file_path \"\${cmake_install_dir}/${target_name}.cmake\")
        
        file(MAKE_DIRECTORY \"\${cmake_install_dir}\")
        
        if(EXISTS \"\${cmake_file_path}\")
            file(REMOVE \"\${cmake_file_path}\")
            message(STATUS \"Removed existing cmake file: \${cmake_file_path}\")
        endif()
        
        execute_process(COMMAND \${CMAKE_COMMAND} -E sleep 0.1)
        
        file(COPY \"${cmake_temp_file}\" DESTINATION \"\${cmake_install_dir}\")
        message(STATUS \"Installed new cmake file: \${cmake_file_path}\")
        
        if(EXISTS \"\${cmake_file_path}\")
            message(STATUS \"Verification: cmake file successfully installed\")
        else()
            message(WARNING \"Failed to install cmake file: \${cmake_file_path}\")
        endif()
    " COMPONENT ${target_name})
    
    # message(STATUS "Third-party library ${target_name} install configuration completed")
    
    setup_algorithm_target_folder(${target_name})
    
    create_individual_install_target(${target_name})
endfunction(install_as_third_party)

function(copy_third_party_dlls_to_output target_name)
    if(NOT BUILD_SHARED)
        return()
    endif()
    
    # message(STATUS "Setting up DLL copy for target: ${target_name}")
    
    get_target_property(LINKED_LIBS ${target_name} LINK_LIBRARIES)
    if(LINKED_LIBS)
        set(processed_libs "")
        
        foreach(lib ${LINKED_LIBS})
            if(TARGET ${lib})
                get_target_property(lib_type ${lib} TYPE)
                if(lib_type STREQUAL "INTERFACE_LIBRARY")
                    get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
                    if(lib_include_dirs)
                        foreach(include_dir ${lib_include_dirs})
                            string(REGEX MATCH "${CMAKE_SOURCE_DIR}/third/([^/]+)" match_result ${include_dir})
                            if(match_result)
                                set(third_lib_name ${CMAKE_MATCH_1})
                                
                                if(third_lib_name IN_LIST processed_libs)
                                    break()
                                endif()
                                list(APPEND processed_libs ${third_lib_name})
                                
                                set(third_lib_dir "${CMAKE_SOURCE_DIR}/third/${third_lib_name}")
                                
                                if(EXISTS "${third_lib_dir}/bin")
                                    add_custom_command(TARGET ${target_name} POST_BUILD
                                        COMMAND $<$<NOT:$<CONFIG:Debug>>:${CMAKE_COMMAND}> $<$<NOT:$<CONFIG:Debug>>:-E> $<$<NOT:$<CONFIG:Debug>>:copy_directory> $<$<NOT:$<CONFIG:Debug>>:"${third_lib_dir}/bin"> $<$<NOT:$<CONFIG:Debug>>:"$<TARGET_FILE_DIR:${target_name}>">
                                        COMMENT "Recursively copying Release directory: $<$<NOT:$<CONFIG:Debug>>:${third_lib_dir}/bin>"
                                    )
                                endif()
                                
                                if(EXISTS "${third_lib_dir}/debug/bin")
                                    add_custom_command(TARGET ${target_name} POST_BUILD
                                        COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_directory> $<$<CONFIG:Debug>:"${third_lib_dir}/debug/bin"> $<$<CONFIG:Debug>:"$<TARGET_FILE_DIR:${target_name}>">
                                        COMMENT "Recursively copying Debug directory: $<$<CONFIG:Debug>:${third_lib_dir}/debug/bin>"
                                    )
                                endif()

                                break()
                            endif()
                        endforeach()
                    endif()
                endif()
            endif()
        endforeach()
    endif()
endfunction(copy_third_party_dlls_to_output) 