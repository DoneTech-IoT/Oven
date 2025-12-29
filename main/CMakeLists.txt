file(GLOB SOURCES)
file(GLOB INCLUDES)

file(GLOB_RECURSE CPP_SOURCES "*.cpp")
list(APPEND SOURCES ${CPP_SOURCES})
                                      
idf_component_register(
    SRCS ${SOURCES} 
    INCLUDE_DIRS ${INCLUDES})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")