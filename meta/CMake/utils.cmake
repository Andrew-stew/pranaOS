function(pranaos_install_headers target_name)
    file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
    foreach(header ${headers})
        get_filename_component(subdirectory ${header} DIRECTORY)
        install(FILES ${header} DESTINATION usr/include/${target_name}/${subdirectory} OPTIONAL)
    endforeach()
endfunction()

function(pranaos_install_sources target_name)
    file(GLOB_RECURSE sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.cpp")
    foreach(source ${sources})
        get_filename_component(subdirectory ${source} DIRECTORY)
        install(FILES ${source} DESTINATION usr/src/pranaos/${target_name}/${subdirectory} OPTIONAL)
    endforeach()
endfunction()

function(pranaos_generated_sources target_name)
    if(DEFINED GENERATED_SOURCES)
        set_source_files_properties(${GENERATED_SOURCES} PROPERTIES GENERATED 1)
        foreach(generated ${GENERATED_SOURCES})
            get_filename_component(generated_name ${generated} NAME)
            add_dependencies(${target_name} generate_${generated_name})
        endforeach()
    endif()
endfunction()

function(pranaos_lib target_name fs_name)
    pranaos_install_headers(${target_name})
    pranaos_install_sources("userland/libraries/${target_name}")
    add_library(${target_name} SHARED ${SOURCES} ${GENERATED_SOURCES})
    set_target_properties(${target_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    install(TARGETS ${target_name} DESTINATION usr/lib OPTIONAL)
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${fs_name})
    pranaos_generated_sources(${target_name})
endfunction()

function(pranaos_shared_lib target_name fs_name)
    pranaos_install_headers(${target_name})
    pranaos_install_sources("userland/libraries/${target_name}")
    add_library(${target_name} SHARED ${SOURCES} ${GENERATED_SOURCES})
    set_target_properties(${target_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    install(TARGETS ${target_name} DESTINATION usr/lib OPTIONAL)
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${fs_name})
    pranaos_generated_sources(${target_name})
endfunction()

function(pranaos_libc target_name fs_name)
    pranaos_install_headers("")
    pranaos_install_sources("userland/libraries/LibC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdlib -fpic")
    add_library(${target_name} SHARED ${SOURCES})
    install(TARGETS ${target_name} DESTINATION usr/lib)
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${fs_name})
    target_link_directories(LibC PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    pranaos_generated_sources(${target_name})
endfunction()

function(pranaos_libc_static target_name fs_name)
    pranaos_install_headers("")
    pranaos_install_sources("userland/libraries/LibC")
    add_library(${target_name} ${SOURCES})
    set_target_properties(${target_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    install(TARGETS ${target_name} ARCHIVE DESTINATION usr/lib OPTIONAL)
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${fs_name})
    target_link_directories(${target_name} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    pranaos_generated_sources(${target_name})
endfunction()

function(pranaos_bin target_name)
    add_executable(${target_name} ${SOURCES})
    set_target_properties(${target_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    install(TARGETS ${target_name} RUNTIME DESTINATION bin OPTIONAL)
    pranaos_generated_sources(${target_name})
endfunction()

function(pranaos_test test_src sub_dir)
    cmake_parse_arguments(PRANAOS_TEST "MAIN_ALREADY_DEFINED" "CUSTOM_MAIN" "LIBS" ${ARGN})
    set(TEST_SOURCES ${test_src})
    if ("${PRANAOS_TEST_CUSTOM_MAIN}" STREQUAL "")
        set(PRANAOS_TEST_CUSTOM_MAIN
            "${CMAKE_SOURCE_DIR}/userland/libraries/LibTest/TestMain.cpp")
    endif()
    if (NOT ${PRANAOS_TEST_MAIN_ALREADY_DEFINED})
        list(PREPEND TEST_SOURCES "${PRANAOS_TEST_CUSTOM_MAIN}")
    endif()
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${TEST_SOURCES})
    add_dependencies(ComponentTests ${test_name})
    set_target_properties(${test_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_link_libraries(${test_name} LibTest libcore)
    foreach(lib ${PRANAOS_TEST_LIBS})
        target_link_libraries(${test_name} ${lib})
    endforeach()
    install(TARGETS ${test_name} RUNTIME DESTINATION usr/Tests/${sub_dir} OPTIONAL)
endfunction()


function(pranaos_testjs_test test_src sub_dir)
    cmake_parse_arguments(PRANAOS_TEST "" "CUSTOM_MAIN" "LIBS" ${ARGN})
    if ("${PRANAOS_TEST_CUSTOM_MAIN}" STREQUAL "")
        set(PRANAOS_TEST_CUSTOM_MAIN
            "${CMAKE_SOURCE_DIR}/userland/libraries/libtest/JavaScriptTestRunnerMain.cpp")
    endif()
    list(APPEND PRANAOS_TEST_LIBS libjs libcore)
    pranaos_test(${test_src} ${sub_dir}
        CUSTOM_MAIN "${PRANAOS_TEST_CUSTOM_MAIN}"
        LIBS ${PRANAOS_TEST_LIBS})
endfunction()

function(pranaos_app target_name)
    cmake_parse_arguments(PRANAOS_APP "" "ICON" "" ${ARGN})

    pranaos_bin("${target_name}")
    set(small_icon "${CMAKE_SOURCE_DIR}/Base/res/icons/16x16/${PRANAOS_APP_ICON}.png")
    set(medium_icon "${CMAKE_SOURCE_DIR}/Base/res/icons/32x32/${PRANAOS_APP_ICON}.png")

    if (EXISTS "${small_icon}")
        embed_resource("${target_name}" pranaos_icon_s "${small_icon}")
    else()
        message(FATAL_ERROR "Missing small app icon: ${small_icon}")
    endif()

    if (EXISTS "${medium_icon}")
        embed_resource("${target_name}" pranaos_icon_m "${medium_icon}")
    else()
        list(APPEND allowed_missing_medium_icons "audio-volume-high")
        list(APPEND allowed_missing_medium_icons "edit-copy")

        if (NOT ${PRANAOS_APP_ICON} IN_LIST allowed_missing_medium_icons)
            message(FATAL_ERROR "Missing medium app icon: ${medium_icon}")
        endif()
    endif()
endfunction()

define_property(TARGET PROPERTY PRANAOS_COMPONENT_NAME BRIEF_DOCS "pranaOS component name" FULL_DOCS "-")
define_property(TARGET PROPERTY PRANAOS_COMPONENT_DESCRIPTION BRIEF_DOCS "pranaOS component description" FULL_DOCS "-")
define_property(TARGET PROPERTY PRANAOS_COMPONENT_RECOMMENDED BRIEF_DOCS "pranaOS component recommended (flag)" FULL_DOCS "-")
define_property(TARGET PROPERTY PRANAOS_COMPONENT_REQUIRED BRIEF_DOCS "pranaOS component required (flag)" FULL_DOCS "-")
define_property(TARGET PROPERTY PRANAOS_COMPONENT_DEPENDS BRIEF_DOCS "pranaOS component dependencies" FULL_DOCS "-")

function(pranaos_component name)
    cmake_parse_arguments(PRANAOS_COMPONENT "RECOMMENDED;REQUIRED" "DESCRIPTION" "TARGETS;DEPENDS" ${ARGN})
    string(TOUPPER "${name}" NAME_UPPER)
    option("BUILD_${NAME_UPPER}" "Build ${name}" ${PRANAOS_COMPONENT_RECOMMENDED})
    add_custom_target("Component${name}")
    set_property(TARGET "Component${name}" PROPERTY PRANAOS_COMPONENT_NAME ${name})
    set_property(TARGET "Component${name}" PROPERTY PRANAOS_COMPONENT_DESCRIPTION ${PRANAOS_COMPONENT_DESCRIPTION})
    set_property(TARGET "Component${name}" PROPERTY PRANAOS_COMPONENT_RECOMMENDED ${PRANAOS_COMPONENT_RECOMMENDED})
    set_property(TARGET "Component${name}" PROPERTY PRANAOS_COMPONENT_REQUIRED ${PRANAOS_COMPONENT_REQUIRED})
    set_property(TARGET "Component${name}" PROPERTY PRANAOS_COMPONENT_DEPENDS ${PRANAOS_COMPONENT_DEPENDS})
    if(PRANAOS_COMPONENT_TARGETS)
        add_dependencies("Component${name}" ${PRANAOS_COMPONENT_TARGETS})
    endif()
    if(BUILD_EVERYTHING OR BUILD_${NAME_UPPER} OR PRANAOS_COMPONENT_REQUIRED)
        add_dependencies(components "Component${name}")
    endif()
    foreach(dependency ${PRANAOS_COMPONENT_DEPENDS})
        add_dependencies("Component${name}" "Component${dependency}")
    endforeach()
endfunction()

macro(export_components_helper file_name current_dir)
    get_property(sub_dirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)
    foreach(sub_dir ${sub_dirs})
        export_components_helper(${file_name} ${sub_dir})
    endforeach()

    get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target ${targets})
        get_property(component_name TARGET ${target} PROPERTY PRANAOS_COMPONENT_NAME)
        if(component_name)
            get_property(component_name TARGET ${target} PROPERTY PRANAOS_COMPONENT_NAME)
            get_property(component_description TARGET ${target} PROPERTY PRANAOS_COMPONENT_DESCRIPTION)
            get_property(component_recommended TARGET ${target} PROPERTY PRANAOS_COMPONENT_RECOMMENDED)
            get_property(component_required TARGET ${target} PROPERTY PRANAOS_COMPONENT_REQUIRED)
            get_property(component_depends TARGET ${target} PROPERTY PRANAOS_COMPONENT_DEPENDS)
            file(APPEND ${file_name} "[${component_name}]\n")
            file(APPEND ${file_name} "description=${component_description}\n")
            if(component_recommended)
                file(APPEND ${file_name} "recommended=1\n")
            else()
                file(APPEND ${file_name} "recommended=0\n")
            endif()
            if(component_required)
                file(APPEND ${file_name} "required=1\n")
            else()
                file(APPEND ${file_name} "required=0\n")
            endif()
            string(TOUPPER "${component_name}" component_name_upper)
            if(BUILD_${component_name_upper})
                file(APPEND ${file_name} "user_selected=1\n")
            else()
                file(APPEND ${file_name} "user_selected=0\n")
            endif()

            file(APPEND ${file_name} "depends=${component_depends}\n")
            file(APPEND ${file_name} "\n")
        endif()
    endforeach()
endmacro()

function(export_components file_name)
    file(WRITE ${file_name} "[Global]\n")
    if(BUILD_EVERYTHING)
        file(APPEND ${file_name} "build_everything=1\n")
    else()
        file(APPEND ${file_name} "build_everything=0\n")
    endif()
    file(APPEND ${file_name} "\n")
    export_components_helper(${file_name} ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

function(compile_gml source output string_name)
    set(source ${CMAKE_CURRENT_SOURCE_DIR}/${source})
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${write_if_different} ${output} ${CMAKE_SOURCE_DIR}/Meta/text-to-cpp-string.sh ${string_name} ${source}
        VERBATIM
        DEPENDS ${CMAKE_SOURCE_DIR}/Meta/text-to-cpp-string.sh
        MAIN_DEPENDENCY ${source}
    )
    get_filename_component(output_name ${output} NAME)
    add_custom_target(generate_${output_name} DEPENDS ${output})
endfunction()


function(compile_ipc source output)
    set(source ${CMAKE_CURRENT_SOURCE_DIR}/${source})
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${write_if_different} ${output} ${CMAKE_BINARY_DIR}/userland/DevTools/IPCCompiler/IPCCompiler ${source}
        VERBATIM
        DEPENDS IPCCompiler
        MAIN_DEPENDENCY ${source}
    )
    get_filename_component(output_name ${output} NAME)
    add_custom_target(generate_${output_name} DEPENDS ${output})
endfunction()

function(embed_resource target section file)
    get_filename_component(asm_file "${file}" NAME)
    set(asm_file "${CMAKE_CURRENT_BINARY_DIR}/${target}-${section}.s")
    get_filename_component(input_file "${file}" ABSOLUTE)
    file(SIZE "${input_file}" file_size)
    add_custom_command(
        OUTPUT "${asm_file}"
        COMMAND "${CMAKE_SOURCE_DIR}/Meta/generate-embedded-resource-assembly.sh" "${asm_file}" "${section}" "${input_file}" "${file_size}"
        DEPENDS "${input_file}" "${CMAKE_SOURCE_DIR}/Meta/generate-embedded-resource-assembly.sh"
        COMMENT "Generating ${asm_file}"
    )
    target_sources("${target}" PRIVATE "${asm_file}")
endfunction()

function(generate_state_machine source header)
    get_filename_component(header_name ${header} NAME)
    set(target_name "generate_${header_name}")
    if(NOT TARGET ${target_name})
        set(source ${CMAKE_CURRENT_SOURCE_DIR}/${source})
        set(output ${CMAKE_CURRENT_BINARY_DIR}/${header})
        add_custom_command(
            OUTPUT ${output}
            COMMAND ${write_if_different} ${output} ${CMAKE_BINARY_DIR}/userland/DevTools/StateMachineGenerator/StateMachineGenerator ${source}
            VERBATIM
            DEPENDS StateMachineGenerator
            MAIN_DEPENDENCY ${source}
        )
        add_custom_target(${target_name} DEPENDS ${output})
    endif()
endfunction()