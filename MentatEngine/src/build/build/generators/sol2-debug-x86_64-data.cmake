########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(sol2_COMPONENT_NAMES "")
list(APPEND sol2_FIND_DEPENDENCY_NAMES lua)
list(REMOVE_DUPLICATES sol2_FIND_DEPENDENCY_NAMES)
set(lua_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(sol2_PACKAGE_FOLDER_DEBUG "C:/Users/sergi/.conan/data/sol2/3.3.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9")
set(sol2_BUILD_MODULES_PATHS_DEBUG )


set(sol2_INCLUDE_DIRS_DEBUG "${sol2_PACKAGE_FOLDER_DEBUG}/include")
set(sol2_RES_DIRS_DEBUG )
set(sol2_DEFINITIONS_DEBUG )
set(sol2_SHARED_LINK_FLAGS_DEBUG )
set(sol2_EXE_LINK_FLAGS_DEBUG )
set(sol2_OBJECTS_DEBUG )
set(sol2_COMPILE_DEFINITIONS_DEBUG )
set(sol2_COMPILE_OPTIONS_C_DEBUG )
set(sol2_COMPILE_OPTIONS_CXX_DEBUG )
set(sol2_LIB_DIRS_DEBUG )
set(sol2_LIBS_DEBUG )
set(sol2_SYSTEM_LIBS_DEBUG )
set(sol2_FRAMEWORK_DIRS_DEBUG )
set(sol2_FRAMEWORKS_DEBUG )
set(sol2_BUILD_DIRS_DEBUG "${sol2_PACKAGE_FOLDER_DEBUG}/")

# COMPOUND VARIABLES
set(sol2_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${sol2_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${sol2_COMPILE_OPTIONS_C_DEBUG}>")
set(sol2_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${sol2_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${sol2_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${sol2_EXE_LINK_FLAGS_DEBUG}>")


set(sol2_COMPONENTS_DEBUG )