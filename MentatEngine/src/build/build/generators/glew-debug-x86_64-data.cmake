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
set(glew_PACKAGE_FOLDER_DEBUG "C:/Users/sergi/.conan/data/glew/2.2.0/_/_/package/187c86e330f71652b242085ff4304d46863f17ab")
set(glew_BUILD_MODULES_PATHS_DEBUG )


set(glew_INCLUDE_DIRS_DEBUG "${glew_PACKAGE_FOLDER_DEBUG}/include")
set(glew_RES_DIRS_DEBUG )
set(glew_DEFINITIONS_DEBUG "-DGLEW_STATIC")
set(glew_SHARED_LINK_FLAGS_DEBUG )
set(glew_EXE_LINK_FLAGS_DEBUG )
set(glew_OBJECTS_DEBUG )
set(glew_COMPILE_DEFINITIONS_DEBUG "GLEW_STATIC")
set(glew_COMPILE_OPTIONS_C_DEBUG )
set(glew_COMPILE_OPTIONS_CXX_DEBUG )
set(glew_LIB_DIRS_DEBUG "${glew_PACKAGE_FOLDER_DEBUG}/lib")
set(glew_LIBS_DEBUG libglew32d)
set(glew_SYSTEM_LIBS_DEBUG )
set(glew_FRAMEWORK_DIRS_DEBUG )
set(glew_FRAMEWORKS_DEBUG )
set(glew_BUILD_DIRS_DEBUG )

# COMPOUND VARIABLES
set(glew_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${glew_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${glew_COMPILE_OPTIONS_C_DEBUG}>")
set(glew_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glew_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glew_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glew_EXE_LINK_FLAGS_DEBUG}>")


set(glew_COMPONENTS_DEBUG GLEW::glew_s)
########### COMPONENT GLEW::glew_s VARIABLES ############################################

set(glew_GLEW_glew_s_INCLUDE_DIRS_DEBUG "${glew_PACKAGE_FOLDER_DEBUG}/include")
set(glew_GLEW_glew_s_LIB_DIRS_DEBUG "${glew_PACKAGE_FOLDER_DEBUG}/lib")
set(glew_GLEW_glew_s_RES_DIRS_DEBUG )
set(glew_GLEW_glew_s_DEFINITIONS_DEBUG "-DGLEW_STATIC")
set(glew_GLEW_glew_s_OBJECTS_DEBUG )
set(glew_GLEW_glew_s_COMPILE_DEFINITIONS_DEBUG "GLEW_STATIC")
set(glew_GLEW_glew_s_COMPILE_OPTIONS_C_DEBUG "")
set(glew_GLEW_glew_s_COMPILE_OPTIONS_CXX_DEBUG "")
set(glew_GLEW_glew_s_LIBS_DEBUG libglew32d)
set(glew_GLEW_glew_s_SYSTEM_LIBS_DEBUG )
set(glew_GLEW_glew_s_FRAMEWORK_DIRS_DEBUG )
set(glew_GLEW_glew_s_FRAMEWORKS_DEBUG )
set(glew_GLEW_glew_s_DEPENDENCIES_DEBUG opengl::opengl glu::glu)
set(glew_GLEW_glew_s_SHARED_LINK_FLAGS_DEBUG )
set(glew_GLEW_glew_s_EXE_LINK_FLAGS_DEBUG )
# COMPOUND VARIABLES
set(glew_GLEW_glew_s_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glew_GLEW_glew_s_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glew_GLEW_glew_s_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glew_GLEW_glew_s_EXE_LINK_FLAGS_DEBUG}>
)
set(glew_GLEW_glew_s_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${glew_GLEW_glew_s_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${glew_GLEW_glew_s_COMPILE_OPTIONS_C_DEBUG}>")