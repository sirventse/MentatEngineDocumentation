# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(sol2_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(sol2_FRAMEWORKS_FOUND_RELEASE "${sol2_FRAMEWORKS_RELEASE}" "${sol2_FRAMEWORK_DIRS_RELEASE}")

set(sol2_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET sol2_DEPS_TARGET)
    add_library(sol2_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET sol2_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${sol2_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${sol2_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:lua::lua>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### sol2_DEPS_TARGET to all of them
conan_package_library_targets("${sol2_LIBS_RELEASE}"    # libraries
                              "${sol2_LIB_DIRS_RELEASE}" # package_libdir
                              sol2_DEPS_TARGET
                              sol2_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "sol2")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${sol2_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${sol2_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${sol2_LIBRARIES_TARGETS}>
                 APPEND)

    if("${sol2_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET sol2::sol2
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     sol2_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${sol2_LINKER_FLAGS_RELEASE}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${sol2_INCLUDE_DIRS_RELEASE}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${sol2_COMPILE_DEFINITIONS_RELEASE}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${sol2_COMPILE_OPTIONS_RELEASE}> APPEND)

########## For the modules (FindXXX)
set(sol2_LIBRARIES_RELEASE sol2::sol2)
