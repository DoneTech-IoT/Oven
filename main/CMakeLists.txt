file(GLOB SOURCES)
file(GLOB INCLUDES)

# Dependencies needed by main headers
# main includes headers from Utilities, UserInterface2, Matter, and MQTT components
set(MAIN_REQUIRES Utilities UserInterface2 Matter MQTT)

# Only build main component when NOT building pre-built libraries
# When building pre-built libraries, main should not be built to avoid component tracking conflicts
if(NOT CONFIG_BUILD_PREBUILT_UTILITIES AND 
   NOT CONFIG_BUILD_PREBUILT_MATTER AND 
   NOT CONFIG_BUILD_PREBUILT_MQTT AND 
   NOT CONFIG_BUILD_PREBUILT_UI2)
    file(GLOB_RECURSE CPP_SOURCES "*.cpp")
    list(APPEND SOURCES ${CPP_SOURCES})
                                          
    idf_component_register(
        SRCS ${SOURCES} 
        INCLUDE_DIRS ${INCLUDES}
        REQUIRES ${MAIN_REQUIRES})

    set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
    target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
else()
    # Register empty component when building pre-built libraries
    # Create a dummy source file to ensure a library target is created
    # This is needed because esp_matter expects __idf_main to be a library
    # Include REQUIRES so ESP-IDF dependency checker doesn't complain about headers in source files
    set(DUMMY_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/dummy_main.cpp")
    file(WRITE "${DUMMY_SOURCE}" "// Dummy source for pre-built library build\n")
    
    idf_component_register(
        SRCS "${DUMMY_SOURCE}"
        INCLUDE_DIRS ${INCLUDES}
        REQUIRES ${MAIN_REQUIRES})
endif()
