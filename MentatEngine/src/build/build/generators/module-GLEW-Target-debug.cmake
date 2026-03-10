# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glew_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(glew_FRAMEWORKS_FOUND_DEBUG "${glew_FRAMEWORKS_DEBUG}" "${glew_FRAMEWORK_DIRS_DEBUG}")

set(glew_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glew_DEPS_TARGET)
    add_library(glew_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glew_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${glew_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${glew_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:opengl::opengl;glu::glu>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glew_DEPS_TARGET to all of them
conan_package_library_targets("${glew_LIBS_DEBUG}"    # libraries
                              "${glew_LIB_DIRS_DEBUG}" # package_libdir
                              glew_DEPS_TARGET
                              glew_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "glew")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glew_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT GLEW::GLEW #############

        set(glew_GLEW_GLEW_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glew_GLEW_GLEW_FRAMEWORKS_FOUND_DEBUG "${glew_GLEW_GLEW_FRAMEWORKS_DEBUG}" "${glew_GLEW_GLEW_FRAMEWORK_DIRS_DEBUG}")

        set(glew_GLEW_GLEW_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glew_GLEW_GLEW_DEPS_TARGET)
            add_library(glew_GLEW_GLEW_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glew_GLEW_GLEW_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glew_GLEW_GLEW_DEPS_TARGET' to all of them
        conan_package_library_targets("${glew_GLEW_GLEW_LIBS_DEBUG}"
                                      "${glew_GLEW_GLEW_LIB_DIRS_DEBUG}"
                                      glew_GLEW_GLEW_DEPS_TARGET
                                      glew_GLEW_GLEW_LIBRARIES_TARGETS
                                      "_DEBUG"
                                      "glew_GLEW_GLEW")

        ########## TARGET PROPERTIES #####################################
        set_property(TARGET GLEW::GLEW
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_LIBRARIES_TARGETS}>
                     APPEND)

        if("${glew_GLEW_GLEW_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET GLEW::GLEW
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         glew_GLEW_GLEW_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glew_GLEW_GLEW_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_LINK_LIBRARIES GLEW::GLEW APPEND)

########## For the modules (FindXXX)
set(glew_LIBRARIES_DEBUG GLEW::GLEW)
