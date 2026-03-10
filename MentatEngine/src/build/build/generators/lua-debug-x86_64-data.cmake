########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(lua_COMPONENT_NAMES "")
set(lua_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(lua_PACKAGE_FOLDER_DEBUG "C:/Users/sergi/.conan/data/lua/5.4.6/_/_/package/164640aad040835ac89882393a96d89200694f04")
set(lua_BUILD_MODULES_PATHS_DEBUG )


set(lua_INCLUDE_DIRS_DEBUG "${lua_PACKAGE_FOLDER_DEBUG}/include")
set(lua_RES_DIRS_DEBUG )
set(lua_DEFINITIONS_DEBUG )
set(lua_SHARED_LINK_FLAGS_DEBUG )
set(lua_EXE_LINK_FLAGS_DEBUG )
set(lua_OBJECTS_DEBUG )
set(lua_COMPILE_DEFINITIONS_DEBUG )
set(lua_COMPILE_OPTIONS_C_DEBUG )
set(lua_COMPILE_OPTIONS_CXX_DEBUG )
set(lua_LIB_DIRS_DEBUG "${lua_PACKAGE_FOLDER_DEBUG}/lib")
set(lua_LIBS_DEBUG lua)
set(lua_SYSTEM_LIBS_DEBUG )
set(lua_FRAMEWORK_DIRS_DEBUG )
set(lua_FRAMEWORKS_DEBUG )
set(lua_BUILD_DIRS_DEBUG "${lua_PACKAGE_FOLDER_DEBUG}/")

# COMPOUND VARIABLES
set(lua_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${lua_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${lua_COMPILE_OPTIONS_C_DEBUG}>")
set(lua_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${lua_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${lua_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${lua_EXE_LINK_FLAGS_DEBUG}>")


set(lua_COMPONENTS_DEBUG )