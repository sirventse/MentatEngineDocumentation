########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(glu_COMPONENT_NAMES "")
list(APPEND glu_FIND_DEPENDENCY_NAMES opengl_system)
list(REMOVE_DUPLICATES glu_FIND_DEPENDENCY_NAMES)
set(opengl_system_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(glu_PACKAGE_FOLDER_RELEASE "C:/Users/sergi/.conan/data/glu/system/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9")
set(glu_BUILD_MODULES_PATHS_RELEASE )


set(glu_INCLUDE_DIRS_RELEASE )
set(glu_RES_DIRS_RELEASE )
set(glu_DEFINITIONS_RELEASE )
set(glu_SHARED_LINK_FLAGS_RELEASE )
set(glu_EXE_LINK_FLAGS_RELEASE )
set(glu_OBJECTS_RELEASE )
set(glu_COMPILE_DEFINITIONS_RELEASE )
set(glu_COMPILE_OPTIONS_C_RELEASE )
set(glu_COMPILE_OPTIONS_CXX_RELEASE )
set(glu_LIB_DIRS_RELEASE )
set(glu_LIBS_RELEASE )
set(glu_SYSTEM_LIBS_RELEASE glu32)
set(glu_FRAMEWORK_DIRS_RELEASE )
set(glu_FRAMEWORKS_RELEASE )
set(glu_BUILD_DIRS_RELEASE "${glu_PACKAGE_FOLDER_RELEASE}/")

# COMPOUND VARIABLES
set(glu_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glu_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glu_COMPILE_OPTIONS_C_RELEASE}>")
set(glu_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glu_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glu_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glu_EXE_LINK_FLAGS_RELEASE}>")


set(glu_COMPONENTS_RELEASE )