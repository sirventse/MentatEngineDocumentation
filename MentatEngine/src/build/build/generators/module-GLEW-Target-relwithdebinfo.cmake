# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glew_FRAMEWORKS_FOUND_RELWITHDEBINFO "") # Will be filled later
conan_find_apple_frameworks(glew_FRAMEWORKS_FOUND_RELWITHDEBINFO "${glew_FRAMEWORKS_RELWITHDEBINFO}" "${glew_FRAMEWORK_DIRS_RELWITHDEBINFO}")

set(glew_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glew_DEPS_TARGET)
    add_library(glew_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glew_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:RelWithDebInfo>:${glew_FRAMEWORKS_FOUND_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:${glew_SYSTEM_LIBS_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:opengl::opengl;glu::glu>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glew_DEPS_TARGET to all of them
conan_package_library_targets("${glew_LIBS_RELWITHDEBINFO}"    # libraries
                              "${glew_LIB_DIRS_RELWITHDEBINFO}" # package_libdir
                              glew_DEPS_TARGET
                              glew_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELWITHDEBINFO"
                              "glew")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glew_BUILD_DIRS_RELWITHDEBINFO} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES RelWithDebInfo ########################################

    ########## COMPONENT GLEW::GLEW #############

        set(glew_GLEW_GLEW_FRAMEWORKS_FOUND_RELWITHDEBINFO "")
        conan_find_apple_frameworks(glew_GLEW_GLEW_FRAMEWORKS_FOUND_RELWITHDEBINFO "${glew_GLEW_GLEW_FRAMEWORKS_RELWITHDEBINFO}" "${glew_GLEW_GLEW_FRAMEWORK_DIRS_RELWITHDEBINFO}")

        set(glew_GLEW_GLEW_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glew_GLEW_GLEW_DEPS_TARGET)
            add_library(glew_GLEW_GLEW_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glew_GLEW_GLEW_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_FRAMEWORKS_FOUND_RELWITHDEBINFO}>
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_SYSTEM_LIBS_RELWITHDEBINFO}>
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_DEPENDENCIES_RELWITHDEBINFO}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glew_GLEW_GLEW_DEPS_TARGET' to all of them
        conan_package_library_targets("${glew_GLEW_GLEW_LIBS_RELWITHDEBINFO}"
                                      "${glew_GLEW_GLEW_LIB_DIRS_RELWITHDEBINFO}"
                                      glew_GLEW_GLEW_DEPS_TARGET
                                      glew_GLEW_GLEW_LIBRARIES_TARGETS
                                      "_RELWITHDEBINFO"
                                      "glew_GLEW_GLEW")

        ########## TARGET PROPERTIES #####################################
        set_property(TARGET GLEW::GLEW
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_OBJECTS_RELWITHDEBINFO}>
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_LIBRARIES_TARGETS}>
                     APPEND)

        if("${glew_GLEW_GLEW_LIBS_RELWITHDEBINFO}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET GLEW::GLEW
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         glew_GLEW_GLEW_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_LINKER_FLAGS_RELWITHDEBINFO}> APPEND)
        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_INCLUDE_DIRS_RELWITHDEBINFO}> APPEND)
        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_COMPILE_DEFINITIONS_RELWITHDEBINFO}> APPEND)
        set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:RelWithDebInfo>:${glew_GLEW_GLEW_COMPILE_OPTIONS_RELWITHDEBINFO}> APPEND)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_LINK_LIBRARIES GLEW::GLEW APPEND)

########## For the modules (FindXXX)
set(glew_LIBRARIES_RELWITHDEBINFO GLEW::GLEW)
