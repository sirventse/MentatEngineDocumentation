# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glew_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(glew_FRAMEWORKS_FOUND_RELEASE "${glew_FRAMEWORKS_RELEASE}" "${glew_FRAMEWORK_DIRS_RELEASE}")

set(glew_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glew_DEPS_TARGET)
    add_library(glew_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glew_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${glew_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${glew_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:opengl::opengl;glu::glu>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glew_DEPS_TARGET to all of them
conan_package_library_targets("${glew_LIBS_RELEASE}"    # libraries
                              "${glew_LIB_DIRS_RELEASE}" # package_libdir
                              glew_DEPS_TARGET
                              glew_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "glew")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glew_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT GLEW::glew_s #############

        set(glew_GLEW_glew_s_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(glew_GLEW_glew_s_FRAMEWORKS_FOUND_RELEASE "${glew_GLEW_glew_s_FRAMEWORKS_RELEASE}" "${glew_GLEW_glew_s_FRAMEWORK_DIRS_RELEASE}")

        set(glew_GLEW_glew_s_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glew_GLEW_glew_s_DEPS_TARGET)
            add_library(glew_GLEW_glew_s_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glew_GLEW_glew_s_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_DEPENDENCIES_RELEASE}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glew_GLEW_glew_s_DEPS_TARGET' to all of them
        conan_package_library_targets("${glew_GLEW_glew_s_LIBS_RELEASE}"
                                      "${glew_GLEW_glew_s_LIB_DIRS_RELEASE}"
                                      glew_GLEW_glew_s_DEPS_TARGET
                                      glew_GLEW_glew_s_LIBRARIES_TARGETS
                                      "_RELEASE"
                                      "glew_GLEW_glew_s")

        ########## TARGET PROPERTIES #####################################
        set_property(TARGET GLEW::glew_s
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_LIBRARIES_TARGETS}>
                     APPEND)

        if("${glew_GLEW_glew_s_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET GLEW::glew_s
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         glew_GLEW_glew_s_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET GLEW::glew_s PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_LINKER_FLAGS_RELEASE}> APPEND)
        set_property(TARGET GLEW::glew_s PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_INCLUDE_DIRS_RELEASE}> APPEND)
        set_property(TARGET GLEW::glew_s PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_COMPILE_DEFINITIONS_RELEASE}> APPEND)
        set_property(TARGET GLEW::glew_s PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${glew_GLEW_glew_s_COMPILE_OPTIONS_RELEASE}> APPEND)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_LINK_LIBRARIES GLEW::glew_s APPEND)

########## For the modules (FindXXX)
set(glew_LIBRARIES_RELEASE GLEW::GLEW)
