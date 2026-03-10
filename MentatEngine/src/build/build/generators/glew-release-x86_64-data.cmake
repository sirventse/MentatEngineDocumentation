########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND glew_COMPONENT_NAMES GLEW::glew_s)
list(REMOVE_DUPLICATES glew_COMPONENT_NAMES)
list(APPEND glew_FIND_DEPENDENCY_NAMES opengl_system glu)
list(REMOVE_DUPLICATES glew_FIND_DEPENDENCY_NAMES)
set(opengl_system_FIND_MODE "NO_MODULE")
set(glu_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(glew_PACKAGE_FOLDER_RELEASE "C:/Users/sergi/.conan/data/glew/2.2.0/_/_/package/876ec6183b0dda30dcb41826d66ad276a23d1ade")
set(glew_BUILD_MODULES_PATHS_RELEASE )


set(glew_INCLUDE_DIRS_RELEASE "${glew_PACKAGE_FOLDER_RELEASE}/include")
set(glew_RES_DIRS_RELEASE )
set(glew_DEFINITIONS_RELEASE "-DGLEW_STATIC")
set(glew_SHARED_LINK_FLAGS_RELEASE )
set(glew_EXE_LINK_FLAGS_RELEASE )
set(glew_OBJECTS_RELEASE )
set(glew_COMPILE_DEFINITIONS_RELEASE "GLEW_STATIC")
set(glew_COMPILE_OPTIONS_C_RELEASE )
set(glew_COMPILE_OPTIONS_CXX_RELEASE )
set(glew_LIB_DIRS_RELEASE "${glew_PACKAGE_FOLDER_RELEASE}/lib")
set(glew_LIBS_RELEASE libglew32)
set(glew_SYSTEM_LIBS_RELEASE )
set(glew_FRAMEWORK_DIRS_RELEASE )
set(glew_FRAMEWORKS_RELEASE )
set(glew_BUILD_DIRS_RELEASE )

# COMPOUND VARIABLES
set(glew_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glew_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glew_COMPILE_OPTIONS_C_RELEASE}>")
set(glew_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glew_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glew_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glew_EXE_LINK_FLAGS_RELEASE}>")


set(glew_COMPONENTS_RELEASE GLEW::glew_s)
########### COMPONENT GLEW::glew_s VARIABLES ############################################

set(glew_GLEW_glew_s_INCLUDE_DIRS_RELEASE "${glew_PACKAGE_FOLDER_RELEASE}/include")
set(glew_GLEW_glew_s_LIB_DIRS_RELEASE "${glew_PACKAGE_FOLDER_RELEASE}/lib")
set(glew_GLEW_glew_s_RES_DIRS_RELEASE )
set(glew_GLEW_glew_s_DEFINITIONS_RELEASE "-DGLEW_STATIC")
set(glew_GLEW_glew_s_OBJECTS_RELEASE )
set(glew_GLEW_glew_s_COMPILE_DEFINITIONS_RELEASE "GLEW_STATIC")
set(glew_GLEW_glew_s_COMPILE_OPTIONS_C_RELEASE "")
set(glew_GLEW_glew_s_COMPILE_OPTIONS_CXX_RELEASE "")
set(glew_GLEW_glew_s_LIBS_RELEASE libglew32)
set(glew_GLEW_glew_s_SYSTEM_LIBS_RELEASE )
set(glew_GLEW_glew_s_FRAMEWORK_DIRS_RELEASE )
set(glew_GLEW_glew_s_FRAMEWORKS_RELEASE )
set(glew_GLEW_glew_s_DEPENDENCIES_RELEASE opengl::opengl glu::glu)
set(glew_GLEW_glew_s_SHARED_LINK_FLAGS_RELEASE )
set(glew_GLEW_glew_s_EXE_LINK_FLAGS_RELEASE )
# COMPOUND VARIABLES
set(glew_GLEW_glew_s_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glew_GLEW_glew_s_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glew_GLEW_glew_s_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glew_GLEW_glew_s_EXE_LINK_FLAGS_RELEASE}>
)
set(glew_GLEW_glew_s_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glew_GLEW_glew_s_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glew_GLEW_glew_s_COMPILE_OPTIONS_C_RELEASE}>")