# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)

# Automatically sync BSP dependency from main/idf_component.yml to all module idf_component.yml files
# This must run before ESP-IDF's component manager processes idf_component.yml files
set(SYNC_BSP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/sync_bsp_to_modules.sh")
if(EXISTS "${SYNC_BSP_SCRIPT}")
    execute_process(
        COMMAND bash "${SYNC_BSP_SCRIPT}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE SYNC_RESULT
    )
    if(NOT SYNC_RESULT EQUAL 0)
        message(WARNING "BSP sync script failed with exit code ${SYNC_RESULT}")
    endif()
endif()

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS    
    ${IDF_PATH}/examples/common_components/protocol_examples_common
    ${extra_components_dirs_append})

project(DoneDeskHub)
if(CONFIG_IDF_TARGET_ESP32C2)
    include(relinker)
endif()

idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H" APPEND)#SEZ@Done 14->17
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# For RISCV chips, project_include.cmake sets -Wno-format, but does not clear various
# flags that depend on -Wformat
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
