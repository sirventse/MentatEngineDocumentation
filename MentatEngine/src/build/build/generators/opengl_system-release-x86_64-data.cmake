########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(opengl_COMPONENT_NAMES "")
set(opengl_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(opengl_PACKAGE_FOLDER_RELEASE "C:/Users/sergi/.conan/data/opengl/system/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9")
set(opengl_BUILD_MODULES_PATHS_RELEASE )


set(opengl_INCLUDE_DIRS_RELEASE )
set(opengl_RES_DIRS_RELEASE )
set(opengl_DEFINITIONS_RELEASE )
set(opengl_SHARED_LINK_FLAGS_RELEASE )
set(opengl_EXE_LINK_FLAGS_RELEASE )
set(opengl_OBJECTS_RELEASE )
set(opengl_COMPILE_DEFINITIONS_RELEASE )
set(opengl_COMPILE_OPTIONS_C_RELEASE )
set(opengl_COMPILE_OPTIONS_CXX_RELEASE )
set(opengl_LIB_DIRS_RELEASE )
set(opengl_LIBS_RELEASE )
set(opengl_SYSTEM_LIBS_RELEASE opengl32)
set(opengl_FRAMEWORK_DIRS_RELEASE )
set(opengl_FRAMEWORKS_RELEASE )
set(opengl_BUILD_DIRS_RELEASE "${opengl_PACKAGE_FOLDER_RELEASE}/")

# COMPOUND VARIABLES
set(opengl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${opengl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${opengl_COMPILE_OPTIONS_C_RELEASE}>")
set(opengl_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${opengl_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${opengl_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${opengl_EXE_LINK_FLAGS_RELEASE}>")


set(opengl_COMPONENTS_RELEASE )