file(GLOB SOURCES)
file(GLOB INCLUDES)

# Collect all .cpp files but exclude example files
file(GLOB_RECURSE CPP_SOURCES "*.cpp")

# Only compile the ServiceManager variant selected via Kconfig
if(CONFIG_USE_GENERALIZED_SERVICE_MANAGER)
    # Generalized mode: exclude legacy implementation
    list(FILTER CPP_SOURCES EXCLUDE REGEX ".*/ServiceManager/ServiceMngr_Legacy\\.cpp$")
else()
    # Legacy mode: exclude generalized/factory sources
    list(FILTER CPP_SOURCES EXCLUDE REGEX ".*/ServiceManager/ServiceMngr_Generalized\\.cpp$")
    list(FILTER CPP_SOURCES EXCLUDE REGEX ".*/ServiceManager/ServiceFactory\\.cpp$")
    list(FILTER CPP_SOURCES EXCLUDE REGEX ".*/ServiceRegistration\\.cpp$")
endif()

list(APPEND SOURCES ${CPP_SOURCES})
list(APPEND INCLUDES "ServiceManager")
                                      
idf_component_register(
    SRCS ${SOURCES} 
    INCLUDE_DIRS ${INCLUDES})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")