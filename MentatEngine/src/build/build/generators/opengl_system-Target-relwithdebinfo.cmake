# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(opengl_FRAMEWORKS_FOUND_RELWITHDEBINFO "") # Will be filled later
conan_find_apple_frameworks(opengl_FRAMEWORKS_FOUND_RELWITHDEBINFO "${opengl_FRAMEWORKS_RELWITHDEBINFO}" "${opengl_FRAMEWORK_DIRS_RELWITHDEBINFO}")

set(opengl_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET opengl_DEPS_TARGET)
    add_library(opengl_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET opengl_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:RelWithDebInfo>:${opengl_FRAMEWORKS_FOUND_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:${opengl_SYSTEM_LIBS_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### opengl_DEPS_TARGET to all of them
conan_package_library_targets("${opengl_LIBS_RELWITHDEBINFO}"    # libraries
                              "${opengl_LIB_DIRS_RELWITHDEBINFO}" # package_libdir
                              opengl_DEPS_TARGET
                              opengl_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELWITHDEBINFO"
                              "opengl")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${opengl_BUILD_DIRS_RELWITHDEBINFO} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES RelWithDebInfo ########################################
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:RelWithDebInfo>:${opengl_OBJECTS_RELWITHDEBINFO}>
                 $<$<CONFIG:RelWithDebInfo>:${opengl_LIBRARIES_TARGETS}>
                 APPEND)

    if("${opengl_LIBS_RELWITHDEBINFO}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET opengl::opengl
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     opengl_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:RelWithDebInfo>:${opengl_LINKER_FLAGS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:RelWithDebInfo>:${opengl_INCLUDE_DIRS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:RelWithDebInfo>:${opengl_COMPILE_DEFINITIONS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:RelWithDebInfo>:${opengl_COMPILE_OPTIONS_RELWITHDEBINFO}> APPEND)

########## For the modules (FindXXX)
set(opengl_LIBRARIES_RELWITHDEBINFO opengl::opengl)
