########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(tinyobjloader_COMPONENT_NAMES "")
set(tinyobjloader_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(tinyobjloader_PACKAGE_FOLDER_RELWITHDEBINFO "C:/Users/sergi/.conan/data/tinyobjloader/2.0.0-rc10/_/_/package/270dc4a2f77a5fc63ad24b950295fddb3a32f993")
set(tinyobjloader_BUILD_MODULES_PATHS_RELWITHDEBINFO )


set(tinyobjloader_INCLUDE_DIRS_RELWITHDEBINFO "${tinyobjloader_PACKAGE_FOLDER_RELWITHDEBINFO}/include")
set(tinyobjloader_RES_DIRS_RELWITHDEBINFO )
set(tinyobjloader_DEFINITIONS_RELWITHDEBINFO )
set(tinyobjloader_SHARED_LINK_FLAGS_RELWITHDEBINFO )
set(tinyobjloader_EXE_LINK_FLAGS_RELWITHDEBINFO )
set(tinyobjloader_OBJECTS_RELWITHDEBINFO )
set(tinyobjloader_COMPILE_DEFINITIONS_RELWITHDEBINFO )
set(tinyobjloader_COMPILE_OPTIONS_C_RELWITHDEBINFO )
set(tinyobjloader_COMPILE_OPTIONS_CXX_RELWITHDEBINFO )
set(tinyobjloader_LIB_DIRS_RELWITHDEBINFO "${tinyobjloader_PACKAGE_FOLDER_RELWITHDEBINFO}/lib")
set(tinyobjloader_LIBS_RELWITHDEBINFO tinyobjloader)
set(tinyobjloader_SYSTEM_LIBS_RELWITHDEBINFO )
set(tinyobjloader_FRAMEWORK_DIRS_RELWITHDEBINFO )
set(tinyobjloader_FRAMEWORKS_RELWITHDEBINFO )
set(tinyobjloader_BUILD_DIRS_RELWITHDEBINFO "${tinyobjloader_PACKAGE_FOLDER_RELWITHDEBINFO}/")

# COMPOUND VARIABLES
set(tinyobjloader_COMPILE_OPTIONS_RELWITHDEBINFO
    "$<$<COMPILE_LANGUAGE:CXX>:${tinyobjloader_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>"
    "$<$<COMPILE_LANGUAGE:C>:${tinyobjloader_COMPILE_OPTIONS_C_RELWITHDEBINFO}>")
set(tinyobjloader_LINKER_FLAGS_RELWITHDEBINFO
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${tinyobjloader_SHARED_LINK_FLAGS_RELWITHDEBINFO}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${tinyobjloader_SHARED_LINK_FLAGS_RELWITHDEBINFO}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${tinyobjloader_EXE_LINK_FLAGS_RELWITHDEBINFO}>")


set(tinyobjloader_COMPONENTS_RELWITHDEBINFO )