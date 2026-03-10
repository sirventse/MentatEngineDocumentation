# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(sol2_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(sol2_FRAMEWORKS_FOUND_DEBUG "${sol2_FRAMEWORKS_DEBUG}" "${sol2_FRAMEWORK_DIRS_DEBUG}")

set(sol2_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET sol2_DEPS_TARGET)
    add_library(sol2_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET sol2_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${sol2_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${sol2_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:lua::lua>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### sol2_DEPS_TARGET to all of them
conan_package_library_targets("${sol2_LIBS_DEBUG}"    # libraries
                              "${sol2_LIB_DIRS_DEBUG}" # package_libdir
                              sol2_DEPS_TARGET
                              sol2_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "sol2")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${sol2_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${sol2_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${sol2_LIBRARIES_TARGETS}>
                 APPEND)

    if("${sol2_LIBS_DEBUG}" STREQUAL "")
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
                 $<$<CONFIG:Debug>:${sol2_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${sol2_INCLUDE_DIRS_DEBUG}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${sol2_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${sol2_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(sol2_LIBRARIES_DEBUG sol2::sol2)
