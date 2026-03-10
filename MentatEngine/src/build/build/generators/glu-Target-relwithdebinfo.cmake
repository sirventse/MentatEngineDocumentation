# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glu_FRAMEWORKS_FOUND_RELWITHDEBINFO "") # Will be filled later
conan_find_apple_frameworks(glu_FRAMEWORKS_FOUND_RELWITHDEBINFO "${glu_FRAMEWORKS_RELWITHDEBINFO}" "${glu_FRAMEWORK_DIRS_RELWITHDEBINFO}")

set(glu_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glu_DEPS_TARGET)
    add_library(glu_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glu_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:RelWithDebInfo>:${glu_FRAMEWORKS_FOUND_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:${glu_SYSTEM_LIBS_RELWITHDEBINFO}>
             $<$<CONFIG:RelWithDebInfo>:opengl::opengl>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glu_DEPS_TARGET to all of them
conan_package_library_targets("${glu_LIBS_RELWITHDEBINFO}"    # libraries
                              "${glu_LIB_DIRS_RELWITHDEBINFO}" # package_libdir
                              glu_DEPS_TARGET
                              glu_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELWITHDEBINFO"
                              "glu")    # package_name

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glu_BUILD_DIRS_RELWITHDEBINFO} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES RelWithDebInfo ########################################
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:RelWithDebInfo>:${glu_OBJECTS_RELWITHDEBINFO}>
                 $<$<CONFIG:RelWithDebInfo>:${glu_LIBRARIES_TARGETS}>
                 APPEND)

    if("${glu_LIBS_RELWITHDEBINFO}" STREQUAL "")
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
                 $<$<CONFIG:RelWithDebInfo>:${glu_LINKER_FLAGS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:RelWithDebInfo>:${glu_INCLUDE_DIRS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:RelWithDebInfo>:${glu_COMPILE_DEFINITIONS_RELWITHDEBINFO}> APPEND)
    set_property(TARGET glu::glu
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:RelWithDebInfo>:${glu_COMPILE_OPTIONS_RELWITHDEBINFO}> APPEND)

########## For the modules (FindXXX)
set(glu_LIBRARIES_RELWITHDEBINFO glu::glu)
