# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(sol2_FRAMEWORKS_FOUND_RELWITHDEBINFO "") # Will be filled later
conan_find_apple_frameworks(sol2_FRAMEWORKS_FOUND_RELWITHDEBINFO "${sol2_FRAMEWORKS_RELWITHDEBINFO}" "${sol2_FRAMEWORK_DIRS_RELWITHDEBINFO}")

set(sol2_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET sol2_DEPS_TARGET)
    add_library(sol2_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET sol2_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:RelWithDebInfo>:${sol2_FRAMEWORKS_FOUND_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:${sol2_SYSTEM_LIBS_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:lua::lua>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### sol2_DEPS_TARGET to all of them
conan_package_library_targets("${sol2_LIBS_RELWITHDEBINFO}"    # libraries
                              "${sol2_LIB_DIRS_RELWITHDEBINFO}" # package_libdir
                              sol2_DEPS_TARGET
                              sol2_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELWITHDEBINFO"
                              "sol2")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${sol2_BUILD_DIRS_RELWITHDEBINFO} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES RelWithDebInfo ########################################
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:RelWithDebInfo>:${sol2_OBJECTS_RELWITHDEBINFO}>
                 $<$<CONFIG:RelWithDebInfo>:${sol2_LIBRARIES_TARGETS}>
                 APPEND)

    if("${sol2_LIBS_RELWITHDEBINFO}" STREQUAL "")
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
                 $<$<CONFIG:RelWithDebInfo>:${sol2_LINKER_FLAGS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:RelWithDebInfo>:${sol2_INCLUDE_DIRS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:RelWithDebInfo>:${sol2_COMPILE_DEFINITIONS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET sol2::sol2
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:RelWithDebInfo>:${sol2_COMPILE_OPTIONS_RELWITHDEBINFO}> APPEND)

########## For the modules (FindXXX)
set(sol2_LIBRARIES_RELWITHDEBINFO sol2::sol2)
