# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glu_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(glu_FRAMEWORKS_FOUND_RELEASE "${glu_FRAMEWORKS_RELEASE}" "${glu_FRAMEWORK_DIRS_RELEASE}")

set(glu_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glu_DEPS_TARGET)
    add_library(glu_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glu_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${glu_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${glu_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:opengl::opengl>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glu_DEPS_TARGET to all of them
conan_package_library_targets("${glu_LIBS_RELEASE}"    # libraries
                              "${glu_LIB_DIRS_RELEASE}" # package_libdir
                              glu_DEPS_TARGET
                              glu_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "glu")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glu_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${glu_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${glu_LIBRARIES_TARGETS}>
                 APPEND)

    if("${glu_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET glu::glu
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     glu_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${glu_LINKER_FLAGS_RELEASE}> APPEND)
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${glu_INCLUDE_DIRS_RELEASE}> APPEND)
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${glu_COMPILE_DEFINITIONS_RELEASE}> APPEND)
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${glu_COMPILE_OPTIONS_RELEASE}> APPEND)

########## For the modules (FindXXX)
set(glu_LIBRARIES_RELEASE glu::glu)
