# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(lua_FRAMEWORKS_FOUND_RELWITHDEBINFO "") # Will be filled later
conan_find_apple_frameworks(lua_FRAMEWORKS_FOUND_RELWITHDEBINFO "${lua_FRAMEWORKS_RELWITHDEBINFO}" "${lua_FRAMEWORK_DIRS_RELWITHDEBINFO}")

set(lua_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET lua_DEPS_TARGET)
    add_library(lua_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET lua_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:RelWithDebInfo>:${lua_FRAMEWORKS_FOUND_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:${lua_SYSTEM_LIBS_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### lua_DEPS_TARGET to all of them
conan_package_library_targets("${lua_LIBS_RELWITHDEBINFO}"    # libraries
                              "${lua_LIB_DIRS_RELWITHDEBINFO}" # package_libdir
                              lua_DEPS_TARGET
                              lua_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELWITHDEBINFO"
                              "lua")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${lua_BUILD_DIRS_RELWITHDEBINFO} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES RelWithDebInfo ########################################
    set_property(TARGET lua::lua
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:RelWithDebInfo>:${lua_OBJECTS_RELWITHDEBINFO}>
                 $<$<CONFIG:RelWithDebInfo>:${lua_LIBRARIES_TARGETS}>
                 APPEND)

    if("${lua_LIBS_RELWITHDEBINFO}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET lua::lua
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     lua_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET lua::lua
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:RelWithDebInfo>:${lua_LINKER_FLAGS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET lua::lua
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:RelWithDebInfo>:${lua_INCLUDE_DIRS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET lua::lua
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:RelWithDebInfo>:${lua_COMPILE_DEFINITIONS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET lua::lua
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:RelWithDebInfo>:${lua_COMPILE_OPTIONS_RELWITHDEBINFO}> APPEND)

########## For the modules (FindXXX)
set(lua_LIBRARIES_RELWITHDEBINFO lua::lua)
