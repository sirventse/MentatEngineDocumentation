# Load the debug and release variables
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB DATA_FILES "${_DIR}/tinyobjloader-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${tinyobjloader_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${tinyobjloader_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET tinyobjloader::tinyobjloader)
    add_library(tinyobjloader::tinyobjloader INTERFACE IMPORTED)
    message(${tinyobjloader_MESSAGE_MODE} "Conan: Target declared 'tinyobjloader::tinyobjloader'")
endif()
if(NOT TARGET tinyobjloader)
    add_library(tinyobjloader INTERFACE IMPORTED)
    set_property(TARGET tinyobjloader PROPERTY INTERFACE_LINK_LIBRARIES tinyobjloader::tinyobjloader)
else()
    message(WARNING "Target name 'tinyobjloader' already exists.")
endif()
# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/tinyobjloader-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()