########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(tinyobjloader_COMPONENT_NAMES "")
set(tinyobjloader_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(tinyobjloader_PACKAGE_FOLDER_RELEASE "C:/Users/sergi/.conan/data/tinyobjloader/2.0.0-rc10/_/_/package/5a61a86bb3e07ce4262c80e1510f9c05e9b6d48b")
set(tinyobjloader_BUILD_MODULES_PATHS_RELEASE )


set(tinyobjloader_INCLUDE_DIRS_RELEASE "${tinyobjloader_PACKAGE_FOLDER_RELEASE}/include")
set(tinyobjloader_RES_DIRS_RELEASE )
set(tinyobjloader_DEFINITIONS_RELEASE )
set(tinyobjloader_SHARED_LINK_FLAGS_RELEASE )
set(tinyobjloader_EXE_LINK_FLAGS_RELEASE )
set(tinyobjloader_OBJECTS_RELEASE )
set(tinyobjloader_COMPILE_DEFINITIONS_RELEASE )
set(tinyobjloader_COMPILE_OPTIONS_C_RELEASE )
set(tinyobjloader_COMPILE_OPTIONS_CXX_RELEASE )
set(tinyobjloader_LIB_DIRS_RELEASE "${tinyobjloader_PACKAGE_FOLDER_RELEASE}/lib")
set(tinyobjloader_LIBS_RELEASE tinyobjloader)
set(tinyobjloader_SYSTEM_LIBS_RELEASE )
set(tinyobjloader_FRAMEWORK_DIRS_RELEASE )
set(tinyobjloader_FRAMEWORKS_RELEASE )
set(tinyobjloader_BUILD_DIRS_RELEASE "${tinyobjloader_PACKAGE_FOLDER_RELEASE}/")

# COMPOUND VARIABLES
set(tinyobjloader_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${tinyobjloader_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${tinyobjloader_COMPILE_OPTIONS_C_RELEASE}>")
set(tinyobjloader_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${tinyobjloader_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${tinyobjloader_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${tinyobjloader_EXE_LINK_FLAGS_RELEASE}>")


set(tinyobjloader_COMPONENTS_RELEASE )