conan = {}
configs = {'Debug','Release','RelWithDebInfo'}

for i = 1,3 do

    include("build/deps/"..configs[i].."/conanbuildinfo.premake.lua")
    conan[configs[i]] = {}
    local cfg = conan[configs[i]]
    cfg["build_type"] = conan_build_type
    cfg["arch"] = conan_arch
    cfg["includedirs"] = conan_includedirs
    cfg["libdirs"] = conan_libdirs
    cfg["bindirs"] = conan_bindirs
    cfg["libs"] = conan_libs
    cfg["system_libs"] = conan_system_libs
    cfg["defines"] = conan_defines
    cfg["cxxflags"] = conan_cxxflags
    cfg["cflags"] = conan_cflags
    cfg["sharedlinkflags"] = conan_sharedlinkflags
    cfg["exelinkflags"] = conan_exelinkflags
    cfg["frameworks"] = conan_frameworks

end

function conan_config_exec()
    configs = {'Debug','Release','RelWithDebInfo'}
    
    for i = 1,3 do
    
        local cfg = conan[configs[i]]
        filter("configurations:"..configs[i])
        
            linkoptions { cfg["exelinkflags"] }
            includedirs{ cfg["includedirs"] }
            libdirs{ cfg["libdirs"] }
            links{ cfg["libs"] }
            links{ cfg["system_libs"] }
            links{ cfg["frameworks"] }
            defines{ cfg["defines"] }
        
        filter{}
    
    end
    
end

function conan_config_lib()
    configs = {'Debug','Release','RelWithDebInfo'}
    
    for i = 1,3 do
    
        local cfg = conan[configs[i]]
        filter("configurations:"..configs[i])
        
            linkoptions { cfg["sharedlinkflags"] }
            includedirs{ cfg["includedirs"] }
            defines{ cfg["defines"] }
        
        filter{}
    
    end
    
end