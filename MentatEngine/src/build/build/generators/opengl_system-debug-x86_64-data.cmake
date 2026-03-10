########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(opengl_COMPONENT_NAMES "")
set(opengl_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(opengl_PACKAGE_FOLDER_DEBUG "C:/Users/sergi/.conan/data/opengl/system/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9")
set(opengl_BUILD_MODULES_PATHS_DEBUG )


set(opengl_INCLUDE_DIRS_DEBUG )
set(opengl_RES_DIRS_DEBUG )
set(opengl_DEFINITIONS_DEBUG )
set(opengl_SHARED_LINK_FLAGS_DEBUG )
set(opengl_EXE_LINK_FLAGS_DEBUG )
set(opengl_OBJECTS_DEBUG )
set(opengl_COMPILE_DEFINITIONS_DEBUG )
set(opengl_COMPILE_OPTIONS_C_DEBUG )
set(opengl_COMPILE_OPTIONS_CXX_DEBUG )
set(opengl_LIB_DIRS_DEBUG )
set(opengl_LIBS_DEBUG )
set(opengl_SYSTEM_LIBS_DEBUG opengl32)
set(opengl_FRAMEWORK_DIRS_DEBUG )
set(opengl_FRAMEWORKS_DEBUG )
set(opengl_BUILD_DIRS_DEBUG "${opengl_PACKAGE_FOLDER_DEBUG}/")

# COMPOUND VARIABLES
set(opengl_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${opengl_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${opengl_COMPILE_OPTIONS_C_DEBUG}>")
set(opengl_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${opengl_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${opengl_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${opengl_EXE_LINK_FLAGS_DEBUG}>")


set(opengl_COMPONENTS_DEBUG )