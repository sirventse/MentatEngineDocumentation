########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(lua_COMPONENT_NAMES "")
set(lua_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(lua_PACKAGE_FOLDER_RELEASE "C:/Users/sergi/.conan/data/lua/5.4.6/_/_/package/5a61a86bb3e07ce4262c80e1510f9c05e9b6d48b")
set(lua_BUILD_MODULES_PATHS_RELEASE )


set(lua_INCLUDE_DIRS_RELEASE "${lua_PACKAGE_FOLDER_RELEASE}/include")
set(lua_RES_DIRS_RELEASE )
set(lua_DEFINITIONS_RELEASE )
set(lua_SHARED_LINK_FLAGS_RELEASE )
set(lua_EXE_LINK_FLAGS_RELEASE )
set(lua_OBJECTS_RELEASE )
set(lua_COMPILE_DEFINITIONS_RELEASE )
set(lua_COMPILE_OPTIONS_C_RELEASE )
set(lua_COMPILE_OPTIONS_CXX_RELEASE )
set(lua_LIB_DIRS_RELEASE "${lua_PACKAGE_FOLDER_RELEASE}/lib")
set(lua_LIBS_RELEASE lua)
set(lua_SYSTEM_LIBS_RELEASE )
set(lua_FRAMEWORK_DIRS_RELEASE )
set(lua_FRAMEWORKS_RELEASE )
set(lua_BUILD_DIRS_RELEASE "${lua_PACKAGE_FOLDER_RELEASE}/")

# COMPOUND VARIABLES
set(lua_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${lua_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${lua_COMPILE_OPTIONS_C_RELEASE}>")
set(lua_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${lua_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${lua_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${lua_EXE_LINK_FLAGS_RELEASE}>")


set(lua_COMPONENTS_RELEASE )