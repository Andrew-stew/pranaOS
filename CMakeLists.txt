cmake_minimum_required(VERSION 3.16)
project(pranaOS C CXX ASM)

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(FATAL_ERROR
    ": Don't use CMAKE_BUILD_TYPE when building pranaOS.\n"
    "The default build type is optimized with debug info and asserts enabled,\n"
    "and that's all there is.")
endif()

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.2)
  message(FATAL_ERROR
      "A GCC version less than 10.2 was detected (${CMAKE_CXX_COMPILER_VERSION}), this is unsupported.\n"
      "Please re-read the build instructions documentation, and upgrade your host compiler.\n")
endif()

set(CMAKE_INSTALL_MESSAGE NEVER)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PRANAOS_ARCH "i686" CACHE STRING "Target architecture for pranaOS.")

option(ENABLE_ADDRESS_SANITIZER "Enable address sanitizer testing in gcc/clang" OFF)
option(ENABLE_KERNEL_ADDRESS_SANITIZER "Enable kernel address sanitizer testing in gcc/clang" OFF)
option(ENABLE_MEMORY_SANITIZER "Enable memory sanitizer testing in gcc/clang" OFF)
option(ENABLE_UNDEFINED_SANITIZER "Enable undefined behavior sanitizer testing in gcc/clang" OFF)
option(ENABLE_FUZZER_SANITIZER "Enable fuzzer sanitizer testing in clang" OFF)
option(ENABLE_EXTRA_KERNEL_DEBUG_SYMBOLS "Enable -Og and -ggdb3 options for Kernel code for easier debugging" OFF)
option(ENABLE_ALL_THE_DEBUG_MACROS "Enable all debug macros to validate they still compile" OFF)
option(ENABLE_ALL_DEBUG_FACILITIES "Enable all noisy debug symbols and options. Not recommended for normal developer use" OFF)
option(ENABLE_COMPILETIME_FORMAT_CHECK "Enable compiletime format string checks" ON)
option(ENABLE_PCI_IDS_DOWNLOAD "Enable download of the pci.ids database at build time" ON)
option(ENABLE_USB_IDS_DOWNLOAD "Enable download of the usb.ids database at build time" ON)
option(BUILD_LAGOM "Build parts of the system targeting the host OS for fuzzing/testing" OFF)
option(ENABLE_KERNEL_LTO "Build the kernel with link-time optimization" OFF)

include(Meta/CMake/wasm_spec_tests.cmake)

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E env "PRANAOS_ARCH=${PRANAOS_ARCH}" ${CMAKE_SOURCE_DIR}/Meta/run.sh
    USES_TERMINAL
)

add_custom_target(setup-and-run
    COMMAND ${CMAKE_MAKE_PROGRAM} install
    COMMAND ${CMAKE_MAKE_PROGRAM} image
    COMMAND ${CMAKE_MAKE_PROGRAM} run
    USES_TERMINAL
)

add_custom_target(image
    DEPENDS qemu-image
)
add_custom_target(qemu-image
    COMMAND ${CMAKE_COMMAND} -E env "PRANAOS_SOURCE_DIR=${CMAKE_SOURCE_DIR}" "PRANAOS_ARCH=${PRANAOS_ARCH}" ${CMAKE_SOURCE_DIR}/meta/build-image-qemu.sh
    BYPRODUCTS ${CMAKE_BINARY_DIR}/_disk_image
    USES_TERMINAL
)
add_custom_target(grub-image
    COMMAND ${CMAKE_COMMAND} -E env "PRANAOS_SOURCE_DIR=${CMAKE_SOURCE_DIR}" "PRANAOS_ARCH=${PRANAOS_ARCH}" ${CMAKE_SOURCE_DIR}/meta/build-image-grub.sh
    BYPRODUCTS ${CMAKE_BINARY_DIR}/grub_disk_image
    USES_TERMINAL
)
add_custom_target(extlinux-image
    COMMAND ${CMAKE_COMMAND} -E env "PRANAOS_SOURCE_DIR=${CMAKE_SOURCE_DIR}" "PRANAOS_ARCH=${PRANAOS_ARCH}" ${CMAKE_SOURCE_DIR}/meta/build-image-extlinux.sh
    BYPRODUCTS ${CMAKE_BINARY_DIR}/extlinux_disk_image
    USES_TERMINAL
)

add_custom_target(lint-shell-scripts
    COMMAND ${CMAKE_SOURCE_DIR}/meta/lint-shell-scripts.sh
    USES_TERMINAL
)
add_custom_target(check-style
    COMMAND ${CMAKE_SOURCE_DIR}/meta/check-style.sh
    USES_TERMINAL
)

add_custom_target(install-ports
    COMMAND ${CMAKE_COMMAND} -E env "PRANAOS_SOURCE_DIR=${CMAKE_SOURCE_DIR}" "PRANAOS_ARCH=${PRANAOS_ARCH}" ${CMAKE_SOURCE_DIR}/Meta/install-ports-tree.sh
    USES_TERMINAL
)

add_custom_target(configure-components
    COMMAND ConfigureComponents
    DEPENDS ConfigureComponents
    USES_TERMINAL
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wno-literal-suffix)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wno-literal-suffix)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang$")
    add_compile_options(-Wno-overloaded-virtual -Wno-user-defined-literals)
endif()

if (ENABLE_ALL_DEBUG_FACILITIES)
    set(ENABLE_ALL_THE_DEBUG_MACROS ON)
    set(ENABLE_EXTRA_KERNEL_DEBUG_SYMBOLS ON)


    set(KMALLOC_VERIFY_NO_SPINLOCK_HELD ON)
endif()

if (ENABLE_ALL_THE_DEBUG_MACROS)
    include(${CMAKE_SOURCE_DIR}/meta/CMake/all_the_debug_macros.cmake)
endif(ENABLE_ALL_THE_DEBUG_MACROS)

configure_file(base/Debug.h.in AK/Debug.h @ONLY)
configure_file(kernel/Debug.h.in Kernel/Debug.h @ONLY)

include_directories(userland/libraries)
include_directories(.)
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(meta/Lagom)

if (ENABLE_UNDEFINED_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

add_subdirectory(userland/devtools/ipccomipler)
add_subdirectory(userland/devtools/statemachinegenerator)
add_subdirectory(userland/libraries/libweb/CodeGenerators)
add_subdirectory(meta/CMake/ConfigureComponents)

set(write_if_different ${CMAKE_SOURCE_DIR}/meta/write-only-on-difference.sh)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

unset(CMAKE_SYSROOT)
set(CMAKE_STAGING_PREFIX ${CMAKE_BINARY_DIR}/Root)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/Root)
set(CMAKE_INSTALL_DATAROOTDIR ${CMAKE_BINARY_DIR}/Root/res)

set(GCC_VERSION 11.1.0)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES pranaOS)
    message("Good job on building cmake!")
else()
    set(TOOLCHAIN_ROOT ${CMAKE_SOURCE_DIR}/toolchain/Local/${PRANAOS_ARCH}/)
    set(TOOLCHAIN_PATH ${TOOLCHAIN_ROOT}/bin)
    set(TOOLCHAIN_PREFIX ${TOOLCHAIN_PATH}/${PRANAOS_ARCH}-pc-pranaos-)

    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
    set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}ld)
    set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}gcc-ranlib)
    set(CMAKE_STRIP ${TOOLCHAIN_PREFIX}strip)
    set(CMAKE_AR ${TOOLCHAIN_PREFIX}gcc-ar)
    set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
endif()

foreach(lang ASM C CXX OBJC OBJCXX)
    unset(CMAKE_${lang}_OSX_COMPATIBILITY_VERSION_FLAG)
    unset(CMAKE_${lang}_OSX_CURRENT_VERSION_FLAG)
    unset(CMAKE_${lang}_LINK_FLAGS)
    unset(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS)
    unset(CMAKE_SHARED_MODULE_CREATE_${lang}_FLAGS)
    unset(CMAKE_SHARED_MODULE_LOADER_${lang}_FLAG )
    unset(CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG)
    unset(CMAKE_${lang}_SYSROOT_FLAG)
    if (CMAKE_SYSTEM_NAME MATCHES Darwin)
        set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
            "<CMAKE_${lang}_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS> <LINK_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
        set(CMAKE_SHARED_LIBRARY_SONAME_${lang}_FLAG "-Wl,-soname,")
    endif()
endforeach()

set(CMAKE_INSTALL_NAME_TOOL "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-shared -Wl,--hash-style=gnu,-z,relro,-z,now,-z,noexecstack")
set(CMAKE_CXX_LINK_FLAGS "-Wl,--hash-style=gnu,-z,relro,-z,now,-z,noexecstack,-z,max-page-size=0x1000")

set(CMAKE_SKIP_RPATH TRUE)

add_compile_options(-Wformat=2)
add_compile_options(-fdiagnostics-color=always)

if (NOT ${CMAKE_HOST_SYSTEM_NAME} MATCHES pranaOS)
    add_compile_options(-Werror)
endif()

add_compile_options(-Wall)
add_compile_options(-Wextra)

add_compile_options(-Wno-address-of-packed-member)
add_compile_options(-Wcast-align)
add_compile_options(-Wcast-qual)
add_compile_options(-Wno-deprecated-copy)
add_compile_options(-Wduplicated-cond)
add_compile_options(-Wdouble-promotion)
add_compile_options(-Wno-expansion-to-defined)
add_compile_options(-Wformat=2)
add_compile_options(-Wimplicit-fallthrough)
add_compile_options(-Wlogical-op)
add_compile_options(-Wmisleading-indentation)
add_compile_options(-Wmissing-declarations)
add_compile_options(-Wno-nonnull-compare)
add_compile_options(-Wnon-virtual-dtor)
add_compile_options(-Wno-unknown-warning-option)
add_compile_options(-Wundef)
add_compile_options(-Wunused)
add_compile_options(-Wwrite-strings)
add_compile_options(-Wno-maybe-uninitialized)

add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)
add_compile_options(-fno-exceptions)
add_compile_options(-fstack-protector-strong)
add_compile_options(-g1)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fstack-clash-protection)
endif()

add_compile_definitions(SANITIZE_PTRS)
set(CMAKE_CXX_FLAGS_STATIC "${CMAKE_CXX_FLAGS} -static")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pie -fpic")

if (ENABLE_COMPILETIME_FORMAT_CHECK)
    add_compile_definitions(ENABLE_COMPILETIME_FORMAT_CHECK)
endif()

add_link_options(--sysroot ${CMAKE_BINARY_DIR}/Root)

include_directories(userland/libraries/libc)
include_directories(userland/libraries/libcrypt)
include_directories(userland/libraries/libm)
include_directories(userland/libraries/libpthread)
include_directories(userland/libraries/libsystem)
include_directories(userland/services)
include_directories(userland)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/userland/services)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/userland/libraries)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/userland)

if (ENABLE_UNDEFINED_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize=vptr")
endif()

add_custom_target(components ALL)
option(BUILD_EVERYTHING "Build all optional components" ON)

include(${CMAKE_SOURCE_DIR}/meta/CMake/utils.cmake)

pranaos_component(
    tests
    RECOMMENDED
)

add_subdirectory(base)
add_subdirectory(kernel)
add_subdirectory(userland)
add_subdirectory(tests)

export_components("${CMAKE_BINARY_DIR}/components.ini")

set(PCI_IDS_GZ_URL https://pci-ids.ucw.cz/v2.2/pci.ids.gz)
set(PCI_IDS_GZ_PATH ${CMAKE_BINARY_DIR}/pci.ids.gz)
set(PCI_IDS_PATH ${CMAKE_BINARY_DIR}/pci.ids)
set(PCI_IDS_INSTALL_PATH ${CMAKE_INSTALL_DATAROOTDIR}/pci.ids)

if(ENABLE_PCI_IDS_DOWNLOAD AND NOT EXISTS ${PCI_IDS_GZ_PATH})
    message(STATUS "Downloading PCI ID database from ${PCI_IDS_GZ_URL}...")
    file(DOWNLOAD ${PCI_IDS_GZ_URL} ${PCI_IDS_GZ_PATH} INACTIVITY_TIMEOUT 10)
endif()

if(EXISTS ${PCI_IDS_GZ_PATH} AND NOT EXISTS ${PCI_IDS_INSTALL_PATH})
    message(STATUS "Extracting PCI ID database from ${PCI_IDS_GZ_PATH}...")
    execute_process(COMMAND gzip -k -d ${PCI_IDS_GZ_PATH})
    file(MAKE_DIRECTORY ${CMAKE_INSTALL_DATAROOTDIR})
    file(RENAME ${PCI_IDS_PATH} ${PCI_IDS_INSTALL_PATH})
endif()

set(USB_IDS_GZ_URL http://www.linux-usb.org/usb.ids.gz)
set(USB_IDS_GZ_PATH ${CMAKE_BINARY_DIR}/usb.ids.gz)
set(USB_IDS_PATH ${CMAKE_BINARY_DIR}/usb.ids)
set(USB_IDS_INSTALL_PATH ${CMAKE_INSTALL_DATAROOTDIR}/usb.ids)

if(ENABLE_USB_IDS_DOWNLOAD AND NOT EXISTS ${USB_IDS_GZ_PATH})
    message(STATUS "Downloading USB ID database from ${USB_IDS_GZ_URL}...")
    file(DOWNLOAD ${USB_IDS_GZ_URL} ${USB_IDS_GZ_PATH} INACTIVITY_TIMEOUT 10)
endif()

if(EXISTS ${USB_IDS_GZ_PATH} AND NOT EXISTS ${USB_IDS_INSTALL_PATH})
    message(STATUS "Extracting USB ID database from ${USB_IDS_GZ_PATH}...")
    execute_process(COMMAND gzip -k -d ${USB_IDS_GZ_PATH})
    file(MAKE_DIRECTORY ${CMAKE_INSTALL_DATAROOTDIR})
    file(RENAME ${USB_IDS_PATH} ${USB_IDS_INSTALL_PATH})
endif()