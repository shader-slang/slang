-- premake5.lua

-- This file describes the build configuration for Slang so
-- that premake can generate platform-specific build files
-- using Premake 5 (https://premake.github.io/).
--
-- To update the build files that are checked in to the Slang repository,
-- run a `premake5` binary and specify the appropriate action, e.g.:
--
--      premake5.exe --os=windows vs2015
--
-- If you are trying to build Slang on another platform, then you
-- can try invoking `premake5` for your desired OS and build format
-- and see what happens.
--
-- If you are going to modify this file to change/customize the Slang
-- build, then you may need to read up on Premake's approach and
-- how it uses/abuses Lua syntax. A few important things to note:
--
-- * Everything that *looks* like a declarative (e.g., `kind "SharedLib"`)
-- is actually a Lua function call (e.g., `kind("SharedLib")`) that
-- modifies the behind-the-scenes state that describes the build.
--
-- * Many of these function calls are "sticky" and affect subsequent
-- calls, so ordering matters a *lot*. This file uses indentation to
-- represent some of the flow of state, but it is important to recognize
-- that the indentation is not semantically significant.
--
-- * Because the configuration logic is just executable Lua code, we
-- can capture and re-use bits of configuration logic in ordinary
-- Lua subroutines.
--
-- Now let's move on to the actual build:

-- The "workspace" represents the overall build (the "solution" in
-- Visual Studio terms). It sets up basic build settings that will
-- apply across all projects.
--

-- To output linux will output to linux
-- % premake5 --os=linux gmake2 --build-location="build.linux"
--
-- % cd build.linux
-- % make config=release_x64
-- or
-- % make config=debug_x64
--
-- From in the build directory you can use
-- % premake5 --file=../premake5.lua --os=linux gmake2

-- Fail if we try to use the deprecated 'gmake' generator
if _ACTION == "gmake" then
    premake.error "Please use the 'gmake2' generator instead of 'gmake'"
end

--
-- Add the package path for slang-pack/slang-util
-- The question mark is there the name of the module is inserted.
---

local modulePath = "external/slang-binaries/lua-modules/?.lua"

package.path = package.path .. ";" .. modulePath

-- Load the slack package manager module
slangPack = require("slang-pack")
slangUtil = require("slang-util")

-- Load the dependencies from the json file
deps = slangPack.loadDependencies("deps/target-deps.json")

newoption {
    trigger     = "override-module",
    description = "(Optional) Specify a lua file that can override functions",
    value       = "path"
}

newoption {
    trigger     = "build-location",
    description = "(Optional) Specifiy the location to place solution on root Makefile",
    value       = "path"
}

newoption {
    trigger     = "execute-binary",
    description = "(Optional) If true binaries used in build will be executed (disable on cross compilation)",
    value       = "bool",
    default     = "true",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "build-glslang",
    description = "(Optional) If true glslang and spirv-opt will be built",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-cuda",
    description = "(Optional) If true will enable cuda tests, if CUDA is found via CUDA_PATH",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-nvapi",
    description = "(Optional) If true will enable NVAPI, if NVAPI is found via external/nvapi",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "cuda-sdk-path",
    description = "(Optional) Path to the root of CUDA SDK. If set will enable CUDA in build (ie in effect sets enable-cuda=true too)",
    value       = "path"
}

newoption {
    trigger     = "enable-optix",
    description = "(Optional) If true will enable OptiX build/ tests (also implicitly enables CUDA)",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "optix-sdk-path",
    description = "(Optional) Path to the root of OptiX SDK. (Implicitly enabled OptiX and CUDA)",
    value       = "path"
}

newoption {
    trigger     = "enable-profile",
    description = "(Optional) If true will enable slang-profile tool - suitable for gprof usage on linux",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-embed-stdlib",
    description = "(Optional) If true build slang with an embedded version of the stdlib",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-xlib",
    description = "(Optional) If true build `gfx` and `platform` with xlib to support windowed apps on linux.",
    value       = "bool",
    default     = "true",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-experimental-projects",
    description = "(Optional) If true include experimental projects in build.",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "disable-stdlib-source",
    description = "(Optional) If true stdlib source will not be included in binary.",
    value       = "bool",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "skip-source-generation",
    description = "(Optional) If true will skip source generation steps.",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "deploy-slang-llvm",
    description = "(Optional) If true will copy slang-llvm to output directory.",
    value       = "bool",
    default     = "true",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "deploy-slang-glslang",
    description = "(Optional) If true will copy slang-glslang to output directory.",
    value       = "bool",
    default     = "true",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "full-debug-validation",
    description = "(Optional) If true will enable full IR validation in debug build. (SLOW!)",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-asan",
    description = "(Optional) If true will enable ASAN (address santizier).",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "dx-on-vk",
    description = "(Optional) If true will use dxvk and vkd3d-proton for DirectX support",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "enable-aftermath",
    description = "(Optional) Enable aftermath in GFX, and add aftermath crash example to project",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

newoption {
    trigger     = "default-spirv-direct",
    description = "(Optional) Development flag to make the default SPIR-V path generate directly rather than via GLSL",
    value       = "bool",
    default     = "false",
    allowed     = { { "true", "True"}, { "false", "False" } }
}

buildLocation = _OPTIONS["build-location"]
executeBinary = (_OPTIONS["execute-binary"] == "true")
buildGlslang = (_OPTIONS["build-glslang"] == "true")
enableCuda = not not (_OPTIONS["enable-cuda"] == "true" or _OPTIONS["cuda-sdk-path"])
enableProfile = (_OPTIONS["enable-profile"] == "true")
optixPath = _OPTIONS["optix-sdk-path"]
enableOptix = not not (_OPTIONS["enable-optix"] == "true" or optixPath)
enableProfile = (_OPTIONS["enable-profile"] == "true")
enableEmbedStdLib = (_OPTIONS["enable-embed-stdlib"] == "true")
enableXlib = (_OPTIONS["enable-xlib"] == "true")
skipSourceGeneration = (_OPTIONS["skip-source-generation"] == "true")
deployLLVM = (_OPTIONS["deploy-slang-llvm"] == "true")
deployGLSLang = (_OPTIONS["deploy-slang-glslang"] == "true")
fullDebugValidation = (_OPTIONS["full-debug-validation"] == "true")
enableAsan = (_OPTIONS["enable-asan"] == "true")
dxOnVk = (_OPTIONS["dx-on-vk"] == "true")
enableAftermath = (_OPTIONS["enable-aftermath"] == "true")
defaultSPIRVDirect = (_OPTIONS["default-spirv-direct"] == "true")

-- If stdlib embedding is enabled, disable stdlib source embedding by default
disableStdlibSource = enableEmbedStdLib

-- If embedding is enabled, and the setting `disable-stdlib-source` setting is set, use it's value
if enableEmbedStdLib and _OPTIONS["disable-stdlib-source"] ~= nil then
    disableStdlibSource = (_OPTIONS["disable-stdlib-source"] == "true")   
end

if enableAftermath then
    aftermathPath = "external/nv-aftermath"
    
    if not os.isfile(path.join(aftermathPath, "nsight-aftermath-usage-guidelines.txt")) then
        print("external/nv-aftermath directory must hold aftermath SDK")
        os.exit(0)
    end

    printf("Enabled aftermath")
end

-- Determine the target info

targetInfo = slangUtil.getTargetInfo()

--
-- Update the dependencies for the target
--

deps:update(targetInfo.name)

-- Get the target name that can be used as paths that generate for different configurations (ie contains premake Tokens)

targetName = targetInfo.tokenName

-- This is the path where nvapi is expected to be found

nvapiPath = "external/nvapi"

if enableOptix then
    optixPath = optixPath or "C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.0.0/"
    enableCuda = true
end

-- cudaPath is only set if cuda is enabled, and CUDA_PATH enviromental variable is set
cudaPath = nil
if enableCuda then
    -- Get the CUDA path. Use the value set on cuda-sdk-path by default, if not set use the environment variable.
    cudaPath = (_OPTIONS["cuda-sdk-path"] or os.getenv("CUDA_PATH"))
end

-- TODO(JS): What's the point in the enable-xlib command line option if it's just overridden here?

if targetInfo.isWindows or os.target() == "macosx" then
    enableXlib = false
end

-- Even if we have the nvapi path, we only want to currently enable on windows targets

enableNvapi = not not (os.isdir(nvapiPath) and targetInfo.isWindows and _OPTIONS["enable-nvapi"] == "true")

if enableNvapi then
    printf("Enabled NVAPI")
end

overrideModule = {}
local overrideModulePath = _OPTIONS["override-module"]
if overrideModulePath then
    overrideModule = require(overrideModulePath)
end


-- This is needed for gcc, for the 'fileno' functions on cygwin
-- _GNU_SOURCE makes realpath available in gcc
if targetInfo.os == "cygwin" then
    buildoptions { "-D_POSIX_SOURCE" }
    filter { "toolset:gcc*" }
        buildoptions { "-D_GNU_SOURCE" }
end

function getPlatforms(targetInfo)
    return { "x86", "x64", "aarch64" }
end

workspace "slang"
    -- We will support debug/release configuration and x86/x64 builds.
    configurations { "Debug", "Release" }
    platforms(getPlatforms(targetInfo))

    if buildLocation then
        location(buildLocation)
    end

    flags "MultiProcessorCompile"

    --
    -- Make slang-test the startup project.
    --
    -- https://premake.github.io/docs/startproject
    startproject "slang-test"

    -- The output binary directory will be derived from the OS
    -- and configuration options, e.g. `bin/windows-x64/debug/`
    targetdir("bin/" .. targetName .. "/%{cfg.buildcfg:lower()}")

    cppdialect "C++17"
    -- Statically link to the C/C++ runtime rather than create a DLL dependency.
    staticruntime "On"
    -- Turn off edit and continue for all projects. This is needed to avoid
    -- linking warnings.
    editandcontinue "Off"

    -- Once we've set up the common settings, we will make some tweaks
    -- that only apply in a subset of cases. Each call to `filter()`
    -- changes the "active" filter for subsequent commands. In
    -- effect, those commands iwll be ignored when the conditions of
    -- the filter aren't satisfied.

    -- Our `x64` platform should (obviously) target the x64
    -- architecture and similarly for x86.
    --
    -- https://premake.github.io/docs/architecture/
    -- 
    filter { "platforms:x64" }
        architecture "x64"
    filter { "platforms:x86" }
        architecture "x86"
    filter { "platforms:aarch64" }
        architecture "ARM64"
    filter { "platforms:aarch64", "toolset:clang" }
        buildoptions { "-arch arm64" }
        linkoptions { "-arch arm64" }

    filter { "toolset:clang or gcc*" }
        -- Makes all symbols hidden by default unless explicitly 'exported'
        buildoptions { "-fvisibility=hidden" } 

    filter { "toolset:clang or gcc*", "files:source/compiler-core/slang-dxc-compiler.cpp" }
        -- For the DXC headers
        buildoptions { "-fms-extensions" }

    -- Disable some warnings
    filter { "toolset:clang or gcc*" }
        buildoptions {
          "-Wno-switch",
          "-Wno-parentheses",
          "-Wno-unused-local-typedefs",
        }
    filter { "toolset:gcc*", "language:C++" }
        buildoptions { "-Wno-class-memaccess" }
        -- If a function returns an address/reference to a local, we want it to produce an error, because 
        -- it probably means something very bad.
        buildoptions { "-Werror=return-local-addr" }
    filter { "toolset:clang", "language:C++" }
        buildoptions { "-Wno-assume" }
    filter { "toolset:clang or gcc*", "language:C++" }
        buildoptions { "-Wno-reorder", "-Wno-invalid-offsetof" }

    -- Enable some warnings on clang/gcc which are on by default in MSVC
    filter { "toolset:clang or gcc*", "language:C++" }
        buildoptions { "-Wnarrowing" }

    -- When compiling the debug configuration, we want to turn
    -- optimization off, make sure debug symbols are output,
    -- and add the same preprocessor definition that VS
    -- would add by default.
    filter { "configurations:debug" }
        optimize "Off"
        symbols "On"
        defines { "_DEBUG" }
        -- staticruntime "Off"
    -- For the release configuration we will turn optimizations on
    -- (we do not yet micro-manage the optimization settings)
    -- and set the preprocessor definition that VS would add by default.
    filter { "configurations:release" }
        optimize "On"
        defines { "NDEBUG" }

    filter { "system:linux" }
        links { "dl" }
        --
        -- `--no-undefined` - by default if a symbol is not found in a link it will assume it will be resolved at runtime (!)
        --                    this option ensures that all the referenced symbols exist
        --
        linkoptions{ "-Wl,-rpath,'$$ORIGIN',--no-as-needed,--no-undefined" }
        -- allow libraries to be listed in any order (do not require dependency order)
        linkgroups "On"

    filter {}
        -- For including windows.h in a way that minimized namespace pollution.
        -- Although we define these here, we still set them manually in any header
        -- files which may be included by another project
        defines { "WIN32_LEAN_AND_MEAN", "VC_EXTRALEAN", "NOMINMAX", "_ITERATOR_DEBUG_LEVEL=0" }

        if dxOnVk then
            defines { "SLANG_CONFIG_DX_ON_VK" }
        end

        if defaultSPIRVDirect then
            defines { "SLANG_CONFIG_DEFAULT_SPIRV_DIRECT" }
        end

function dump(o)
    if type(o) == 'table' then
        local s = '{ '
        for k,v in pairs(o) do
            if type(k) ~= 'number' then k = '"'..k..'"' end
            s = s .. '['..k..'] = ' .. dump(v) .. ','
        end
        return s .. '} '
    else
        return tostring(o)
    end
end

function dumpTable(o)
    local s = '{ '
    for k,v in pairs(o) do
        if type(k) ~= 'number' then k = '"'..k..'"' end
        s = s .. '['..k..'] = ' .. tostring(v) .. ',\n'
    end
    return s .. '} '
end

function getExecutableSuffix()
    if(os.target() == "windows") then
        return ".exe"
    end
    return ""
end
--
-- We are now going to start defining the projects, where
-- each project builds some binary artifact (an executable,
-- library, etc.).
--
-- All of our projects follow a common structure, so rather
-- than reiterate a bunch of build settings, we define
-- some subroutines that make the configuration as concise
-- as possible.
--
-- First, we will define a helper routine for adding all
-- the relevant files from a given directory path:
--
-- Note that this does not work recursively
-- so projects that spread their source over multiple
-- directories will need to take more steps.
function addSourceDir(path)
    files
    {
        path .. "/*.cpp",       -- C++ source files
        path .. "/*.slang",     -- Slang files (for our stdlib)
        path .. "/*.h",         -- Header files
        path .. "/*.hpp",       -- C++ style headers (for glslang)
        path .. "/*.natvis",    -- Visual Studio debugger visualization files
        path .. "/*.natstepfilter", -- Visual Studio debugger step filter files
        path .. "/*.natjmc", -- Visual Studio debugger step filter files
    }
    if os.target() == "macosx" then
        files { path .. "/*.mm" } -- Objective-C++ files
        filter { "files:**.mm" }
            compileas "Objective-C++"
        filter {}
    end
    removefiles
    {
        "**/*.meta.slang.h",
        "**/slang-*generated*.h",
        "**/gfx-unit-test/test-tmp*"
    }
end

-- Adds CUDA dependency to a project
function addCUDAIfEnabled()
    if type(cudaPath) == "string" and targetInfo.isWindows then
        filter {}
        includedirs { cudaPath .. "/include" }
        includedirs { cudaPath .. "/include", cudaPath .. "/common/inc" }
        links { "cuda", "cudart" }
        if optixPath then
            defines { "RENDER_TEST_OPTIX" }
            includedirs { optixPath .. "include/" }
        end

        filter { "platforms:x86" }
            libdirs { cudaPath .. "/lib/Win32/" }

        filter { "platforms:x64" }
            libdirs { cudaPath .. "/lib/x64/" }
        filter {}
        return true
    elseif enableCuda then
        filter {}
        if type(cudaPath) == "string" then
            includedirs { cudaPath .. "/include" }
            includedirs { cudaPath .. "/include" }
            if optixPath then
                defines { "GFX_OPTIX" }
                includedirs { optixPath .. "include/" }
            end
            filter { "platforms:x86" }
            libdirs { cudaPath .. "/lib32/" }
            filter { "platforms:x64" }
            libdirs { cudaPath .. "/lib64/" }
            filter {}
            links { "cuda", "cudart" }
        else
            print "Error: CUDA is enabled but --cuda-sdk-path is not specified."
        end
        return true
    end
    return false
end

--
-- Next we will define a helper routine that all of our
-- projects will bottleneck through. Here `name` is
-- the name for the project (and the base name for
-- whatever output file it produces), while `sourceDir`
-- is the directory that holds the source.
--
-- E.g., for the `slangc` project, the source code
-- is nested in `source/`, so we'd (indirectly) call:
--
--      baseSlangProject("slangc", "source/slangc")
--
-- NOTE! This function will add any source from the sourceDir, *if* it's specified.
-- Pass nil if adding files is not wanted.
function baseSlangProject(name, sourceDir)

    -- Start a new project in premake. This switches
    -- the "current" project over to the newly created
    -- one, so that subsequent commands affect this project.
    --
    project(name)

    -- We need every project to have a stable UUID for
    -- output formats (like Visual Studio and XCode projects)
    -- that use UUIDs rather than names to uniquely identify
    -- projects. If we don't have a stable UUID, then the
    -- output files might have spurious diffs whenever we
    -- re-run premake generation.

    if sourceDir then
        uuid(os.uuid(name .. '|' .. sourceDir))
    else
        -- If we don't have a sourceDir, the name will have to be enough
        uuid(os.uuid(name))
    end

    -- Location could do with a better name than 'other' - but it seems as if %{cfg.buildcfg:lower()} and similar variables
    -- is not available for location to expand.
    location("build/" .. slangUtil.getBuildLocationName(targetInfo) .. "/" .. name)


    -- The intermediate ("object") directory will use a similar
    -- naming scheme to the output directory, but will also use
    -- the project name to avoid cases where multiple projects
    -- have source files with the same name.
    --
    objdir("intermediate/" .. targetName .. "/%{cfg.buildcfg:lower()}/%{prj.name}")

    -- Treat C++ as the default language, projects in other languages can
    -- override this later
    --
    language "C++"

    -- By default, Premake generates VS project files that
    -- reflect the directory structure of the source code.
    -- While this is nice in principle, it creates messy
    -- results in practice for our projects.
    --
    -- Instead, we will use the `vpaths` feature to imitate
    -- the default VS behavior of grouping files into
    -- virtual subdirectories (VS calls them "filters") for
    -- header and source files respectively.
    --
    -- Note: We are setting `vpaths` using a list of key/value
    -- tables instead of just a key/value table, since this
    -- appears to be an (undocumented) way to fix the order
    -- in which the filters are tested. Otherwise we have
    -- issues where premake will nondeterministically decide
    -- the check something against the `**.cpp` filter first,
    -- and decide that a `foo.cpp.h` file should go into
    -- the `"Source Files"` vpath. That behavior seems buggy,
    -- but at least we appear to have a workaround.
    --
    vpaths {
        { ["Header Files"] = { "**.h", "**.hpp"} },
        { ["Source Files"] = { "**.cpp", "**.slang", "**.natvis", "**.natjmc" } },
    }

    -- Override default options for a project if necessary

    if overrideModule.addBaseProjectOptions then
        overrideModule.addBaseProjectOptions()
    end

    --
    -- Add the files in the sourceDir
    -- NOTE! This doesn't recursively add files in subdirectories
    --

    if not not sourceDir then
        addSourceDir(sourceDir)
    end

    --
    -- Enable ASAN (address sanitizer) if requested.
    --

    if enableAsan then
        if (targetInfo.isWindows) then
            buildoptions { "/fsanitize=address" }
            flags { "NoIncrementalLink" }
        end
    end

end


-- We can now use the `baseSlangProject()` subroutine to
-- define helpers for the different categories of project
-- in our source tree.
--
-- For example, the Slang project has several tools that
-- are used during building/testing, but don't need to
-- be distributed. These always have their source code in
-- `tools/<project-name>/`.
--
function tool(name)
    -- We use the `group` command here to specify that the
    -- next project we create shold be placed into a group
    -- named "tools" in a generated IDE solution/workspace.
    --
    -- This is used in the generated Visual Studio solution
    -- to group all the tools projects together in a logical
    -- sub-directory of the solution.
    --
    group "tools"

    -- Now we invoke our shared project configuration logic,
    -- specifying that the project lives under the `tools/` path.
    --
    baseSlangProject(name, "tools/" .. name)

    -- Finally, we set the project "kind" to produce a console
    -- application. This is a reasonable default for tools,
    -- and it can be overriden because Premake is stateful,
    -- and a subsequent call to `kind()` would overwrite this
    -- default.
    --
    kind "ConsoleApp"

    if not targetInfo.isWindows then
        links { "pthread" }
    end
end

-- "Standard" projects will be those that go to make the binary
-- packages for slang: the shared libraries and executables.
--
function standardProject(name, sourceDir)
    -- Because Premake is stateful, any `group()` call by another
    -- project would still be in effect when we create a project
    -- here (e.g., if somebody had called `tool()` before
    -- `standardProject()`), so we are careful here to set the
    -- group to an emptry string, which Premake treats as "no group."
    --
    group ""

    baseSlangProject(name, sourceDir)
end


function toolSharedLibrary(name)
    group "test-tool"
    -- specifying that the project lives under the `tools/` path.
    --
    baseSlangProject(name .. "-tool", "tools/" .. name)

    defines { "SLANG_SHARED_LIBRARY_TOOL" }

    kind "SharedLib"

    if not targetInfo.isWindows then
        links { "pthread" }
    end
end

function exampleLibrary(name)
    group "examples"
    baseSlangProject(name, "examples/"..name)
    kind "StaticLib"
    includedirs { ".", "tools" }
    links { "gfx", "slang", "platform", "gfx-util", "core"}
    addCUDAIfEnabled();
end

exampleLibrary "example-base"

-- Finally we have the example programs that show how to use Slang.
--
function example(name)
    -- Example programs go into an "example" group
    group "examples"

    -- They have their source code under `examples/<project-name>/`
    baseSlangProject(name, "examples/" .. name)

    -- Set up working directory to be the source directory
    debugdir("examples/" .. name)

    -- By default, all of our examples are GUI applications. One some
    -- platforms there is no meaningful distinction between GUI and
    -- command-line applications, but it is significant on Windows and MacOS
    --
    kind "WindowedApp"

    -- Every example needs to be able to include the `slang.h` header
    -- if it is going to use Slang, so we might as well set up a suitable
    -- include path here rather than make each example do it.
    --
    -- Most of the examples also need the `gfx` library,
    -- which lives under `tools/`, so we will add that to the path as well.
    --
    includedirs { ".", "tools" }

    -- The examples also need to link against the slang library,
    -- and the `gfx` abstraction layer (which in turn
    -- depends on the `core` library). We specify all of that here,
    -- rather than in each example.
    links { "example-base", "slang", "gfx", "gfx-util", "platform", "core" }

    if not targetInfo.isWindows then
        links { "pthread" }
    end

    if targetInfo.isWindows then
    else
        if enableXlib then
            defines { "SLANG_ENABLE_XLIB" }
            libdirs { "/usr/X11/lib" }
            links {"X11"}
        end
    end

    addCUDAIfEnabled();
end

--
-- Create a project that is used as a build step, typically to
-- build items needed for other dependencies
---

function generatorProject(name, sourcePath, projectKind)
    -- We use the `group` command here to specify that the
    -- next project we create shold be placed into a group
    -- named "generator" in a generated IDE solution/workspace.
    --
    -- This is used in the generated Visual Studio solution
    -- to group all the tools projects together in a logical
    -- sub-directory of the solution.
    --
    group "generator"

    -- Set up the project, but do NOT add any source files.
    baseSlangProject(name, sourcePath)

    -- By default the generator projects run a custom tool and don't
    -- require any premake machinery to compile a binary for them.
    if projectKind == nil then
        kind "Utility"
    else
        kind (projectKind)
    end

    if not targetInfo.isWindows then
        links { "pthread" }
    end
end

--
-- With all of these helper routines defined, we can now define the
-- actual projects quite simply. For example, here is the entire
-- declaration of the "Hello, World" example project:
--
example "hello-world"
    kind "ConsoleApp"
    includedirs {"external/vulkan/include"}

-- Note how we are calling our custom `example()` subroutine with
-- the same syntax sugar that Premake usually advocates for their
-- `project()` function. This allows us to treat `example` as
-- a kind of specialized "subclass" of `project`
--

-- Let's go ahead and set up the projects for our other example now.
example "platform-test"

example "triangle"

example "ray-tracing"
example "ray-tracing-pipeline"

example "autodiff-texture"

example "gpu-printing"
    kind "ConsoleApp"

example "shader-toy"

example "model-viewer"

example "shader-object"
    kind "ConsoleApp"

example "cpu-com-example"
    kind "ConsoleApp"

example "cpu-hello-world"
    kind "ConsoleApp"

if enableAftermath then
    example "nv-aftermath-example"
        filter {}
        
        local aftermathIncludePath = path.join(aftermathPath, "include") 
        local aftermathLibPath = path.join(aftermathPath, "lib") 
        
        -- Add the aftermath includes 
        
        includedirs { aftermathIncludePath }
        
        -- Add the libs directory.
        -- Additionally we need to copy dlls that are needed for aftermath usage such that they 
        -- are available from the executable.
        
        filter { "platforms:x86" }
            local libPath = path.join(aftermathLibPath, "x86")
            libdirs { libPath }
            links { "GFSDK_Aftermath_Lib.x86" }

            postbuildcommands {
                    '{COPY} "$(SolutionDir)"' .. libPath .. '/*.* "%{cfg.targetdir}"'
                }

        filter { "platforms:x64" }
            local libPath = path.join(aftermathLibPath, "x64")
            libdirs { libPath }
            links { "GFSDK_Aftermath_Lib.x64" }
            
            postbuildcommands {
                    '{COPY} "$(SolutionDir)"' .. libPath .. '/*.* "%{cfg.targetdir}"'
                }
end

-- Most of the other projects have more interesting configuration going
-- on, so let's walk through them in order of increasing complexity.
--
-- The `core` project is a static library that has all the basic types
-- and routines that get shared across both the Slang compiler/runtime
-- and the various tool projects. It's build is pretty simple:
--

standardProject("core", "source/core")
    uuid "F9BE7957-8399-899E-0C49-E714FDDD4B65"
    kind "StaticLib"
    -- We need the core library to be relocatable to be able to link with slang.so
    pic "On"

    -- For our core implementation, we want to use the most
    -- aggressive warning level supported by the target, and
    -- to treat every warning as an error to make sure we
    -- keep our code free of warnings.
    --
    warnings "Extra"

    includedirs { "external/miniz" }

    if targetInfo.isWindows then
        addSourceDir "source/core/windows"
    else
        addSourceDir "source/core/unix"
    end

standardProject("compiler-core", "source/compiler-core")
    uuid "12C1E89D-F5D0-41D3-8E8D-FB3F358F8126"
    kind "StaticLib"
    -- We need the compiler-core library to be relocatable to be able to link with slang.so
    pic "On"

    links { "core" }

    -- For our core implementation, we want to use the most
    -- aggressive warning level supported by the target, and
    -- to treat every warning as an error to make sure we
    -- keep our code free of warnings.
    --
    warnings "Extra"

    if targetInfo.isWindows then
        addSourceDir "source/compiler-core/windows"
    else
        addSourceDir "source/compiler-core/unix"
    end

standardProject("slang-rt", "source/slang-rt")
    uuid "DFC79D72-91DE-434C-871B-B3943B488BEB"
    kind "SharedLib"
    pic "On"
    warnings "Extra"
    links {"miniz", "lz4"}
    includedirs { "external/miniz" }
    defines { "SLANG_RT_DYNAMIC", "SLANG_RT_DYNAMIC_EXPORT" }
    addSourceDir "source/core"
    if targetInfo.isWindows then
        addSourceDir "source/core/windows"
    else
        addSourceDir "source/core/unix"
        links { "pthread" }
    end

--
-- The cpp extractor is a tool that scans C++ header files to extract
-- reflection like information, and generate files to handle
-- RTTI fast/simply
---

tool "slang-cpp-extractor"
    uuid "CA8A30D1-8FA9-4330-B7F7-84709246D8DC"
    includedirs { "." }

    links { "compiler-core", "core" }

tool "slang-spirv-embed-generator"
    uuid "8da787cc-0e04-450f-8e29-88eac5ebe9bb"
    includedirs { "." }

    links { "compiler-core", "core" }

tool "slang-lookup-generator"
    uuid "3242baa7-fc4c-4f76-83bc-e4403099dc1d"
    includedirs { "." }

    links { "compiler-core", "core" }

tool "slang-capability-generator"
    uuid "FD16CA29-C66A-430A-822C-C09655088611"
    includedirs { "." }

    links { "compiler-core", "core" }

tool "test-process"
    uuid "BE412850-4BB9-429A-877C-BFBC4B34186C"
    includedirs { "." }

    links { "compiler-core", "core" }

tool "test-server"
    uuid "23149706-C12F-4329-B6AA-8266407C32D3"
    includedirs { "." }

    links { "compiler-core", "core", "slang" }
    
tool "slangd"
    uuid "B2D63B45-92B0-40F7-B242-CCA4DFD64341"
    includedirs { "." }
    links { "compiler-core", "core", "slang" }
  
tool "vk-pipeline-create"
    uuid "9ED23316-AE8B-4018-9407-271F4DDB8CBA"
    includedirs { ".", "external/vulkan/include"}
    files { "examples/hello-world/vulkan-api.cpp", "examples/hello-world/vulkan-api.h" }
    links { "slang", "gfx", "gfx-util", "platform", "core" }

--
--
-- `slang-generate` is a tool we use for source code generation on
-- the compiler. It depends on the `core` library, so we need to
-- declare that:
--

tool "slang-generate"
    uuid "66174227-8541-41FC-A6DF-4764FC66F78E"
    links { "core" }

tool "slang-embed"
    uuid "7F773DD9-EB8F-2403-B43C-B49C2014B99C"
    links { "core" }

--
-- The `slang-test` test driver also uses the `core` library, and it
-- currently relies on include paths being set up so that it can find
-- the core headers:
--

tool "slang-test"
    uuid "0C768A18-1D25-4000-9F37-DA5FE99E3B64"
    includedirs { "." }
    links { "compiler-core", "slang", "core", "miniz", "lz4" }
    dependson { "slang-reflection-test-tool", "render-test-tool", "slang-unit-test-tool", "gfx-unit-test-tool" }
    -- We want to set to the root of the project, but that doesn't seem to work with '.'.
    -- So set a path that resolves to the same place.
    debugdir("source/..")
    if not targetInfo.isWindows then
        links { "pthread" }
    end
    
--
-- The reflection test harness `slang-reflection-test` is pretty
-- simple, in that it only needs to link against the slang library
-- to do its job:
--

toolSharedLibrary "slang-reflection-test"
    uuid "C5ACCA6E-C04D-4B36-8516-3752B3C13C2F"

    includedirs { "." }

    kind "SharedLib"
    links { "core", "slang" }

--
-- The most complex testing tool we have is `render-test`, but from
-- a build perspective the most interesting thing about it is that for
-- our Windows build it requires a Windows 10 SDK.
--
-- TODO: Try to make the build not require a fixed version of the Windows SDK.
-- Ideally we should just specify a *minimum* version.
--
-- This test also requires Vulkan headers which we've placed in the
-- `external/` directory, and it also includes files from the `core`
-- library in ways that require us to set up `source/` as an include path.
--
-- TODO: Fix that requirement.
--

toolSharedLibrary "render-test"
    uuid "61F7EB00-7281-4BF3-9470-7C2EA92620C3"

    includedirs { ".", "external", "external/vulkan/include", "source", "tools/gfx", "tools/platform" }
    links { "core", "compiler-core", "slang", "gfx", "gfx-util", "platform" }
    if targetInfo.isWindows then
        addSourceDir "tools/render-test/windows"

        systemversion "latest"

        -- For Windows targets, we want to copy
        -- dxcompiler.dll, and dxil.dll from the Windows SDK redistributable
        -- directory into the output directory.
        -- d3dcompiler_47.dll is copied from the external/slang-binaries submodule.
        postbuildcommands { '"$(SolutionDir)tools\\copy-hlsl-libs.bat" "$(WindowsSdkDir)Redist/D3D/%{cfg.platform:lower()}/" "%{cfg.targetdir}/" "windows-%{cfg.platform:lower()}"'}
        if (type(cudaPath) == "string") then
            addSourceDir "tools/render-test/cuda"
        end
    end
    if addCUDAIfEnabled() then
        defines { "RENDER_TEST_CUDA" }
    end

--
-- `gfx` is a abstraction layer for different GPU platforms.
--

tool "gfx"
    uuid "222F7498-B40C-4F3F-A704-DDEB91A4484A"
    -- Unlike most of the code under `tools/`, this is a library
    -- rather than a stand-alone executable.
    kind "SharedLib"
    links { "core", "slang" }
    pic "On"

    defines { "SLANG_GFX_DYNAMIC", "SLANG_GFX_DYNAMIC_EXPORT" }

    includedirs { ".", "external", "external/vulkan/include", "source" }

    files {"slang-gfx.h"}

    -- Will compile across targets
    addSourceDir "tools/gfx/apple"
    addSourceDir "tools/gfx/cpu"
    addSourceDir "tools/gfx/nvapi"
    addSourceDir "tools/gfx/cuda"
    addSourceDir "tools/gfx/debug-layer"
    if targetInfo.isWindows then
        postbuildcommands {
            '{COPY} "$(SolutionDir)tools/gfx/gfx.slang" "%{cfg.targetdir}"',
            '{COPY} "$(SolutionDir)tools/gfx/slang.slang" "%{cfg.targetdir}"'
        }
    else
        postbuildcommands {
            '{COPY} "' .. path.getabsolute("tools/gfx/gfx.slang") .. '" "%{cfg.targetdir}"',
            '{COPY} "' .. path.getabsolute("tools/gfx/slang.slang") .. '" "%{cfg.targetdir}"',
        }
    end
    
    -- If aftermath is enabled we need a define to turn on debugging features withing GFX
    
    if enableAftermath then
        defines { "GFX_NV_AFTERMATH" }
        
        local aftermathIncludePath = path.join(aftermathPath, "include") 
        local aftermathLibPath = path.join(aftermathPath, "lib") 
        
        -- Add the aftermath includes         
        includedirs { aftermathIncludePath }
        
        -- Add the libs
        -- 
        -- We don't copy the dlls as that is something the application should do.
        
        filter { "platforms:x86" }
            local libPath = path.join(aftermathLibPath, "x86")
            libdirs { libPath }
            links { "GFSDK_Aftermath_Lib.x86" }

        filter { "platforms:x64" }
            local libPath = path.join(aftermathLibPath, "x64")
            libdirs { libPath }
            links { "GFSDK_Aftermath_Lib.x64" }
        
    end
    
    -- To special case that we may be building using cygwin on windows. If 'true windows' we build for dx12/vk and run the script
    -- If not we assume it's a cygwin/mingw type situation and remove files that aren't appropriate
    if targetInfo.isWindows then
        systemversion "latest"

        -- For Windows targets, we want to copy
        -- dxcompiler.dll, and dxil.dll from the Windows SDK redistributable
        -- directory into the output directory.
        -- d3dcompiler_47.dll is copied from the external/slang-binaries submodule.
        postbuildcommands { '"$(SolutionDir)tools\\copy-hlsl-libs.bat" "$(WindowsSdkDir)Redist/D3D/%{cfg.platform:lower()}/" "%{cfg.targetdir}/"'}

        addSourceDir "tools/gfx/vulkan"
        addSourceDir "tools/gfx/open-gl"
        addSourceDir "tools/gfx/d3d"
        addSourceDir "tools/gfx/d3d11"
        addSourceDir "tools/gfx/d3d12"
    elseif targetInfo.os == "mingw" or targetInfo.os == "cygwin" then
        -- Don't support any render techs...
    elseif os.target() == "macosx" then
        addSourceDir "tools/gfx/vulkan"
    else
        -- Linux like
        addSourceDir "tools/gfx/vulkan"
        if dxOnVk then
            addSourceDir "tools/gfx/d3d"
            addSourceDir "tools/gfx/d3d11"
            addSourceDir "tools/gfx/d3d12"
        end
        --addSourceDir "tools/gfx/open-gl"
    end

    if os.target() == "macosx" then
        links { "Cocoa.framework", "QuartzCore.framework" }
    end

    if enableXlib then
        defines { "SLANG_ENABLE_XLIB" }
        libdirs { "/usr/X11/lib" }
        links {"X11"}
    end

    -- If NVAPI is enabled
    if enableNvapi then
        -- Add the include path
        includedirs { nvapiPath }

        -- Add a define so that render-test code can check if nvapi is available
        defines { "GFX_NVAPI" }

        -- Set the nvapi libs directory
        filter { "platforms:x86" }
            libdirs { nvapiPath .. "/x86" }
            links { "nvapi" }

        filter { "platforms:x64" }
            libdirs { nvapiPath .. "/amd64" }
            links { "nvapi64" }

    end
    if addCUDAIfEnabled() then
        defines { "GFX_ENABLE_CUDA" }
    end
    
--
-- `gfx-util` is a static library containing utilities and helpers for using
-- the `gfx` library.
--
tool "gfx-util"
    uuid "F5ADB74E-02A7-44FB-AA3B-FC02F8AC7A4B"
    kind "StaticLib"
    pic "On"

    includedirs { ".", "source" }

    addSourceDir "tools/gfx-util"
--
-- `platform` contains all the platform abstractions for a GUI application.
--
tool "platform"
    uuid "3565fe5e-4fa3-11eb-ae93-0242ac130002"
    kind "SharedLib"
    pic "On"
    links {"core", "slang", "gfx" }
    defines { "SLANG_PLATFORM_DYNAMIC", "SLANG_PLATFORM_DYNAMIC_EXPORT" }
    includedirs { ".", "external", "source", "external/imgui", "tools/gfx" }
    addSourceDir "tools/platform"
    addSourceDir "tools/platform/linux"
    addSourceDir "tools/platform/windows"
    addSourceDir "tools/platform/apple"
    addSourceDir "tools/platform/placeholder"
    -- Include windowing support on Windows.
    if targetInfo.isWindows then
        systemversion "latest"
    elseif os.target() == "macosx" then
        links { "Cocoa.framework", "QuartzCore.framework" }
    else
        if enableXlib then
            defines { "SLANG_ENABLE_XLIB" }
            libdirs { "/usr/X11/lib" }
            links {"X11"}
        end
    end

--
-- The `slangc` command-line application is just a very thin wrapper
-- around the Slang dynamic library, so its build is extermely simple.
-- One windows `slangc` uses the the `core` library for some UTF-16
-- to UTF-8 string conversion before calling into `slang.dll`, so
-- it also depends on `core`:
--

standardProject("slangc", "source/slangc")
    uuid "D56CBCEB-1EB5-4CA8-AEC4-48EA35ED61C7"
    kind "ConsoleApp"
    links { "core", "slang" }
    if not targetInfo.isWindows then
        links { "pthread" }
    end

function getBuildDir(isArm64)
    return "%{cfg.targetdir}"
end

function astReflectGenerator(isArm64)
    local builddir = getBuildDir()

    filter("files:**/slang-ast-reflect.h")

    buildmessage "C++ Extractor %{file.relpath}"

    local sourcePath = "%{file.directory}"

    -- Work out the output files

    local outputTypes = { "obj", "ast", "value" };

    local outputTable = {}

    for key, outputType in ipairs(outputTypes) do
        table.insert(outputTable, sourcePath .. "/slang-generated-" .. outputType .. ".h")
        table.insert(outputTable, sourcePath .. "/slang-generated-" .. outputType .. "-macro.h")
    end

    -- List all of the input files to be scanned

    local inputFiles = { "slang-ast-support-types.h", "slang-ast-base.h", "slang-ast-decl.h", "slang-ast-expr.h", "slang-ast-modifier.h", "slang-ast-stmt.h", "slang-ast-type.h", "slang-ast-val.h" }

    local options = { "-strip-prefix", "slang-", "-o", "slang-generated", "-output-fields", "-mark-suffix", "_CLASS"}

    -- Specify the actual command to run for this action.
    --
    -- Note that we use a single-quoted Lua string and wrap the path
    -- to the `slang-cpp-extractor` command in double quotes to avoid
    -- confusing the Windows shell. It seems that Premake outputs that
    -- path with forward slashes, which confused the shell if we don't
    -- quote the executable path.

    local buildcmd = '"' .. builddir .. '/slang-cpp-extractor" -d ' .. sourcePath .. " " .. table.concat(inputFiles, " ") .. " " .. table.concat(options, " ")

    buildcommands { buildcmd }

    -- Specify the files output by the extactor - so custom action will run when these files are needed.
    --
    buildoutputs(outputTable)

    -- Make it depend on the extractor tool itself
    local buildInputTable = { builddir .. "/slang-cpp-extractor" .. getExecutableSuffix() }
    for key, inputFile in ipairs(inputFiles) do
        table.insert(buildInputTable, sourcePath .. "/" .. inputFile)
    end

    --
    buildinputs(buildInputTable)
end

function metaSlangGenerator()
    local builddir = getBuildDir()

    filter("files:**.meta.slang")

    -- Specify the "friendly" message that should print to the build log for the action
    buildmessage "slang-generate %{file.relpath}"

    -- Specify the actual command to run for this action.
    --
    -- Note that we use a single-quoted Lua string and wrap the path
    -- to the `slang-generate` command in double quotes to avoid
    -- confusing the Windows shell. It seems that Premake outputs that
    -- path with forward slashes, which confused the shell if we don't
    -- quote the executable path.
    --
    buildcommands { '"' .. builddir .. '/slang-generate" %{file.relpath}' }

    -- Given `foo.meta.slang` we woutput `foo.meta.slang.h`.
    -- This needs to be specified because the custom action will only
    -- run when this file needs to be generated.
    --
    -- Note the use of abspath here, this ensures windows tests the correct file, otherwise
    -- triggering doesn't work. The problem still remains on linux, because abspath *isn't* an
    -- absolute path, it remains relative.
    --
    -- TODO(JS):
    -- It's not clear how to determine how to create the absolute path on linux, using
    -- path.absolutepath, requires knowing the path to be relative to, and it's neither
    -- the current path, the source path or the targetpath.
    buildoutputs { "%{file.abspath}.h" }

    -- We will specify an additional build input dependency on the `slang-generate`
    -- tool itself, so that changes to the code for the tool cause the generation
    -- step to be re-run.
    --
    -- In order to get the file name right, we need to know the executable suffix
    -- that the target platform will use. Premake might have a built-in way to
    -- query this, but I couldn't find it, so I am just winging it for now:
    --
    --
    buildinputs { builddir .. "/slang-generate" .. getExecutableSuffix() }
end

function preludeGenerator()
    local builddir = getBuildDir()

    filter("files:prelude/*-prelude.h")

    buildmessage "slang-embed %{file.relpath}"
    buildcommands { '"' .. builddir .. '/slang-embed" %{file.relpath}' }
    buildoutputs { "%{file.abspath}.cpp" }
    buildinputs { builddir .. "/slang-embed" .. getExecutableSuffix() }
end

function capabilityGenerator()
    filter("files:source/slang/*.capdef")

    dependson { "slang-capability-generator" }
    local inputFile = "%{file.abspath}"
    local builddir = getBuildDir()
    local outputHeaderFile = "%{wks.location}/source/slang/slang-generated-capability-defs.h"
    local outputCppFile = "%{wks.location}/source/slang/slang-generated-capability-defs-impl.h"
    local outputLookupFile = "%{wks.location}/source/slang/slang-lookup-capability-defs.cpp"
    if executeBinary then
        buildmessage ("slang-capability-generator %{file.relpath}")
        local buildcmd = '"' .. builddir .. '/slang-capability-generator" "%{file.abspath}"'
        buildcommands { buildcmd }
        buildinputs { "%{file.abspath}", builddir .. "/slang-capability-generator" .. getExecutableSuffix() }
        buildoutputs (outputHeaderFile, outputCppFile, outputLookupFile)
    end

    filter { }
end

if not skipSourceGeneration then

generatorProject("run-generators", nil)

    -- We make 'source/slang' the location of the source, to make paths to source
    -- relative to that

    -- We include these, even though they are not really part of the dummy
    -- build, so that the filters below can pick up the appropriate locations.

    files
    {
        "source/slang/*.meta.slang",                -- The stdlib files
        "source/slang/slang-ast-reflect.h",         -- C++ reflection
        "prelude/*.h",                              -- The prelude files
        "source/slang/*.capdef",

        --
        -- To build we need to have some source! It has to be a source file that
        -- does not depend on anything that is generated, so we take something
        -- from core that will compile without any generation.
        --

        "source/core/slang-string.cpp",
    }

    -- First, we need to ensure that various source-generation tools
    -- get built before `slang`, so we declare a non-linking dependency between
    -- the projects here:
    dependson { "slang-cpp-extractor", "slang-generate", "slang-embed", "slang-capability-generator" }

    local executableSuffix = getExecutableSuffix()

    -- We need to run the C++ extractor to generate some include files
    if executeBinary then
        astReflectGenerator()
    end

    -- Next, we want to add a custom build rule for each of the
    -- files that makes up the standard library. Those are
    -- always named `*.meta.slang`, so we can select for them
    -- using a `filter` and then use Premake's support for
    -- defining custom build commands:
    --
    if executeBinary then
        metaSlangGenerator()
        preludeGenerator()
        capabilityGenerator()
    end

    filter { }

generatorProject("generate-spirv-embed")
    tables = {
        {
            json = "external/spirv-headers/include/spirv/unified1/spirv.core.grammar.json",
        },
    }
    for _, t in pairs(tables) do
        files {t.json}
    end

    dependson { "slang-spirv-embed-generator" }

    local builddir = getBuildDir()
    if executeBinary then
        for _, t in pairs(tables) do
            filter("files:" .. t.json)

            local inJson = "%{file.abspath}"
            local cppFilename = "slang-spirv-core-grammar-embed.cpp"
            local cppPath = "%{wks.location}/source/slang/" .. cppFilename
            local buildcmd = '"' .. builddir .. '/slang-spirv-embed-generator" '
                .. inJson .. " "
                .. cppPath

            buildmessage ("slang-spirv-embed-generator for " .. cppFilename)
            buildcommands { buildcmd }
            buildinputs { inJson, builddir .. "/slang-spirv-embed-generator" .. getExecutableSuffix() }
            buildoutputs (cppPath)
        end
    end

    filter { }

generatorProject("generate-lookup-tables")
    tables = {
        {
            json = "external/spirv-headers/include/spirv/unified1/extinst.glsl.std.450.grammar.json",
            header = "spirv/unified1/GLSL.std.450.h",
            prefix = "GLSLstd450",
            type = "GLSLstd450"
        },
    }
    for _, t in pairs(tables) do
        files {t.json}
    end

    dependson { "slang-lookup-generator" }

    local builddir = getBuildDir()
    if executeBinary then
        for _, t in pairs(tables) do
            filter("files:" .. t.json)

            local inJson = "%{file.abspath}"
            local cppFilename = "slang-lookup-" .. t.type:lower() .. ".cpp"
            local cppPath = "%{wks.location}/source/slang/" .. cppFilename
            local buildcmd = '"' .. builddir .. '/slang-lookup-generator" '
                .. inJson .. " "
                .. cppPath .. " "
                .. t.type .. " "
                .. t.prefix .. " "
                .. t.header

            buildmessage ("slang-lookup-generator for " .. cppFilename)
            buildcommands { buildcmd }
            buildinputs { inJson, builddir .. "/slang-lookup-generator" .. getExecutableSuffix() }
            buildoutputs (cppPath)
        end
    end

    filter { }

if enableEmbedStdLib then
    standardProject("slangc-bootstrap", "source/slangc")
        uuid "6339BF31-AC99-4819-B719-679B63451EF0"
        kind "ConsoleApp"
        links { "core", "compiler-core", "miniz", "lz4" }

        -- We need to run all the generators to be able to build the main
        -- slang source in source/slang

        dependson { "run-generators" }

        defines {
            -- We are going statically link Slang compiler with the slangc command line
            "SLANG_STATIC",
            -- This is the bootstrap to produce the embedded stdlib, so we disable to be able to bootstrap
            "SLANG_WITHOUT_EMBEDDED_STD_LIB"
        }

        includedirs { "external/spirv-headers/include", "source" }

        -- Add all of the slang source
        addSourceDir "source/slang"

        -- On some tests with MSBuild disabling these made build work.
        -- flags { "NoIncrementalLink", "NoPCH", "NoMinimalRebuild" }

        -- The `standardProject` operation already added all the code in
        -- `source/slang/*`, but we also want to incldue the umbrella
        -- `slang.h` header in this prject, so we do that manually here.
        files { "slang.h" }

        files { "source/core/core.natvis" }

        -- We explicitly name the prelude file(s) that we need to
        -- compile for their embedded code, since they will not
        -- exist at the time projects/makefiles are generated,
        -- and thus a glob would not match anything.
        files {
            "prelude/slang-cuda-prelude.h.cpp",
            "prelude/slang-hlsl-prelude.h.cpp",
            "prelude/slang-cpp-prelude.h.cpp",
            "prelude/slang-cpp-host-prelude.h.cpp",
            "prelude/slang-torch-prelude.h.cpp",
            "source/slang/slang-lookup-glslstd450.cpp",
            "source/slang/slang-lookup-capability-defs.cpp"
        }
        if not targetInfo.isWindows then
            links { "pthread" }
        end
end

if enableEmbedStdLib then
    generatorProject("embed-stdlib-generator", nil, "SharedLib")

        -- We include these, even though they are not really part of the dummy
        -- build, so that the filters below can pick up the appropriate locations.

        files
        {
            --
            -- To build we need to have some source! It has to be a source file that
            -- does not depend on anything that is generated, so we take something
            -- from core that will compile without any generation.
            --

            "source/slang/slang-stdlib-api.cpp",
        }

        -- Only produce the embedded stdlib if that option is enabled

        local executableSuffix = getExecutableSuffix()

        -- We need slangc-bootstrap to build the embedded stdlib
        dependson { "slangc-bootstrap" }

        local absDirectory = path.getabsolute("source/slang")
        local absOutputPath = absDirectory .. "/slang-stdlib-generated.h"

        -- I don't know why I need a filter, but without it nothing works (!)
        filter("files:source/slang/slang-stdlib-api.cpp")
        -- Note! Has to be an absolute path else doesn't work(!)
        buildoutputs { absOutputPath }
        filter("files:source/slang/slang-stdlib-api.cpp")
    
        filter(f)
            buildinputs { "%{cfg.targetdir}/slangc-bootstrap" .. executableSuffix }
            buildcommands { '"%{cfg.targetdir}/slangc-bootstrap" -archive-type riff-lz4 -save-stdlib-bin-source "%{file.directory}/slang-stdlib-generated.h"' }
end
end -- not skipSourceGeneration

--
-- TODO: Slang's current `Makefile` build does some careful incantations
-- to make sure that the binaries it generates use a "relative `RPATH`"
-- for loading shared libraries, so that Slang is not dependent on
-- being installed to a fixed path on end-user machines. Before we
-- can use Premake for the Linux build (or eventually MacOS) we would
-- need to figure out how to replicate this incantation in premake.
--

--
-- Now that we've gotten all the simple projects out of the way, it is time
-- to get into the more serious build steps.
--
-- First up is the `slang` dynamic library project:
--

standardProject("slang", "source/slang")
    uuid "DB00DA62-0533-4AFD-B59F-A67D5B3A0808"
    kind "SharedLib"
    links { "core", "compiler-core", "miniz", "lz4"}
    warnings "Extra"
    pic "On"
    
    -- The way that we currently configure things through `slang.h`,
    -- we need to set a preprocessor definitions to ensure that
    -- we declare the Slang API functions for *export* and not *import*.
    --
    defines { "SLANG_DYNAMIC_EXPORT" }

    if disableStdlibSource then
        defines { "SLANG_DISABLE_STDLIB_SOURCE" }
    end

    if fullDebugValidation then
        defines { "SLANG_ENABLE_FULL_IR_VALIDATION" }
    end

    if enableEmbedStdLib then
        -- We only have this dependency if we are embedding stdlib
        if not skipSourceGeneration then
            dependson { "embed-stdlib-generator" }
        end
    else
        -- Disable StdLib embedding
        defines { "SLANG_WITHOUT_EMBEDDED_STD_LIB" }
    end

    includedirs { "external/spirv-headers/include", "source" }

    -- On some tests with MSBuild disabling these made build work.
    -- flags { "NoIncrementalLink", "NoPCH", "NoMinimalRebuild" }

    -- The `standardProject` operation already added all the code in
    -- `source/slang/*`, but we also want to incldue the umbrella
    -- `slang.h` header in this prject, so we do that manually here.
    files { "slang.h" }

    files { "source/core/core.natvis" }

    -- We explicitly name the prelude file(s) that we need to
    -- compile for their embedded code, since they will not
    -- exist at the time projects/makefiles are generated,
    -- and thus a glob would not match anything.
    files {
        "prelude/slang-cuda-prelude.h.cpp",
        "prelude/slang-hlsl-prelude.h.cpp",
        "prelude/slang-cpp-prelude.h.cpp",
        "prelude/slang-cpp-host-prelude.h.cpp",
        "prelude/slang-torch-prelude.h.cpp"
    }

    -- Similarly for any generated lookup tables
    files {
        "source/slang/slang-lookup-glslstd450.cpp",
        "source/slang/slang-lookup-capability-defs.cpp",
        "source/slang/slang-generated-capability-defs.h",
        "source/slang/slang-generated-capability-defs-impl.h",
    }

    --
    -- The most challenging part of building `slang` is that we need
    -- to invoke generators such as slang-cpp-extractor and slang-generate
    -- to generate. We do this by executing the run-generators 'dummy' project
    -- which produces the appropriate source

    if not skipSourceGeneration then
        dependson { "run-generators" }
        dependson { "generate-lookup-tables" }
        dependson { "generate-capabilities" }
        dependson { "generate-spirv-embed" }
    end

    -- If we have slang-llvm copy it
    local slangLLVMPath = deps:getProjectRelativePath("slang-llvm", "../../..")
    
    if slangLLVMPath and deployLLVM then
        filter { "system:linux or macosx or windows" }
            local sharedLibName = slangUtil.getSharedLibraryFileName(targetInfo, "slang-llvm")            
            postbuildcommands {
                "{COPY} " .. slangLLVMPath .."/bin/" .. targetName .. "/release/" .. sharedLibName .. " %{cfg.targetdir}"
            }
    end

    local slangGlslangPath = deps:getProjectRelativePath("slang-glslang", "../../..")

    -- If we are not building glslang from source, then be
    -- sure to copy a binary copy over to the output directory
    if not buildGlslang and slangGlslangPath~=nil and deployGLSLang then
        filter { "system:linux or macosx or windows" }
            local sharedLibName = slangUtil.getSharedLibraryFileName(targetInfo, "slang-glslang")            
            postbuildcommands {
                "{COPY} " .. slangGlslangPath .. "/bin/" .. targetName .. "/release/" .. sharedLibName .. " %{cfg.targetdir}"
            }        
    end

    filter {"configurations:debug"}
        defines { "SLANG_ENABLE_IR_BREAK_ALLOC=1" }
    filter {}

    if not targetInfo.isWindows then
        links { "pthread" }
    end

toolSharedLibrary "gfx-unit-test"
    uuid "092DAB9F-1DA5-4538-ADD7-1A8D1DBFD519"
    includedirs { "." }
    addSourceDir "tools/unit-test"
    links {  "core", "slang", "gfx", "gfx-util", "platform" }

toolSharedLibrary "slang-unit-test"
    uuid "0162864E-7651-4B5E-9105-C571105276EA"
    includedirs { "." }
    addSourceDir "tools/unit-test"
    links { "lz4", "miniz", "core", "compiler-core",  "slang" }
    if not targetInfo.isWindows then
        links { "pthread" }
    end

if enableProfile then
    tool "slang-profile"
        uuid "375CC87D-F34A-4DF1-9607-C5C990FD6227"

        -- gprof needs symbols
        symbols "On"

        dependson { "slang" }

        includedirs { "external/spirv-headers/include" }

        defines { "SLANG_STATIC",
            -- Disable StdLib embedding
            "SLANG_WITHOUT_EMBEDDED_STD_LIB"
        }

        -- The `standardProject` operation already added all the code in
        -- `source/slang/*`, but we also want to incldue the umbrella
        -- `slang.h` header in this prject, so we do that manually here.
        files { "slang.h" }

        files { "source/core/core.natvis" }

        -- We explicitly name the prelude file(s) that we need to
        -- compile for their embedded code, since they will not
        -- exist at the time projects/makefiles are generated,
        -- and thus a glob would not match anything.
        files {
            "prelude/slang-cuda-prelude.h.cpp",
            "prelude/slang-hlsl-prelude.h.cpp",
            "prelude/slang-cpp-prelude.h.cpp",
            "prelude/slang-cpp-host-prelude.h.cpp",
            "prelude/slang-torch-prelude.h.cpp"
        }

        -- Add the slang source
        addSourceDir "source/slang"

        includedirs { "." }
        links { "core", "compiler-core", "miniz", "lz4"}

        filter { "system:linux" }
            linkoptions{  "-pg" }
            buildoptions{ "-pg" }

end

standardProject("miniz", nil)
    uuid "E76ACB11-4A12-4F0A-BE1E-CE0B8836EB7F"
    kind "StaticLib"
    pic "On"

    -- Add the files explicitly
    language "C"
    files
    {
        "external/miniz/miniz.c",
        "external/miniz/miniz_tdef.c",
        "external/miniz/miniz_tinfl.c",
        "external/miniz/miniz_zip.c"
    }

    -- A file usually created by miniz's build system
    io.writefile("external/miniz/miniz_export.h", "#define MINIZ_EXPORT")
    -- Miniz redefines this, so remove it for this project
    removedefines "WIN32_LEAN_AND_MEAN"

    filter { "system:linux" }
        defines { "_LARGEFILE64_SOURCE" }

    filter { "system:linux or macosx" }
        links { "dl"}

standardProject("lz4", nil)
    uuid "E1EC8075-823E-46E5-BC38-C124CCCDF878"
    kind "StaticLib"
    pic "On"

    -- Add the files explicitly
    language "C"
    files
    {
        "external/lz4/lib/lz4.c",
        "external/lz4/lib/lz4.h",
    }

    filter { "system:linux or macosx" }
        links { "dl"}

if buildGlslang then

standardProject("slang-spirv-tools", nil)
    uuid "C36F6185-49B3-467E-8388-D0E9BF5F7BB8"
    kind "StaticLib"
    pic "On"

    includedirs { "external/spirv-tools", "external/spirv-tools/include", "external/spirv-headers/include",  "external/spirv-tools-generated"}

    addSourceDir("external/spirv-tools/source")
    addSourceDir("external/spirv-tools/source/opt")
    addSourceDir("external/spirv-tools/source/util")
    addSourceDir("external/spirv-tools/source/val")

    filter { "system:linux or macosx" }
        links { "dl"}
--
-- The single most complicated part of our build is our custom version of glslang.
-- Is not really set up to produce a shared library with a usable API, so we have
-- our own custom shim API around it to invoke GLSL->SPIRV compilation.
--
-- Glslang normally relies on a CMake-based build process, and its code is spread
-- across multiple directories with implicit dependencies on certain command-line
-- definitions.
--
-- The following is a tailored build of glslang that pulls in the pieces we care
-- about whle trying to leave out the rest:
--
standardProject("slang-glslang", "source/slang-glslang")
    uuid "C495878A-832C-485B-B347-0998A90CC936"
    kind "SharedLib"
    pic "On"
    cppdialect "C++17"
    includedirs { "external/glslang", "external/spirv-tools", "external/spirv-tools/include", "external/spirv-headers/include",  "external/spirv-tools-generated", "external/glslang-generated" }

    defines
    {
        -- `ENABLE_OPT` must be defined (to either zero or one) for glslang to compile at all
        "ENABLE_OPT=1",

        -- We want to build a version of glslang that supports every feature possible,
        -- so we will enable all of the supported vendor-specific extensions so
        -- that they can be used in Slang-generated GLSL code when required.
        --
        "AMD_EXTENSIONS",
        "NV_EXTENSIONS",
    }

    -- We will add source code from every directory that is required to get a
    -- minimal GLSL->SPIR-V compilation path working.
    addSourceDir("external/glslang/glslang/GenericCodeGen")
    addSourceDir("external/glslang/glslang/MachineIndependent")
    addSourceDir("external/glslang/glslang/MachineIndependent/preprocessor")
    addSourceDir("external/glslang/OGLCompilersDLL")
    addSourceDir("external/glslang/SPIRV")
    addSourceDir("external/glslang/StandAlone")

    -- Unfortunately, blindly adding files like that also pulled in a declaration
    -- of a main entry point that we do *not* want, so we will specifically
    -- exclude that file from our build.
    removefiles { "external/glslang/StandAlone/StandAlone.cpp" }

    -- Glslang includes some platform-specific code around DLL setup/teardown
    -- and handling of thread-local storage for its multi-threaded mode. We
    -- don't really care about *any* of that, but we can't remove it from the
    -- build so we need to include the appropriate platform-specific sources.

    links { "slang-spirv-tools"}

    filter { "system:windows" }
        -- On Windows we need to add the platform-specific sources and then
        -- remove the `main.cpp` file since it tries to define a `DllMain`
        -- and we don't want the default glslang one.
        addSourceDir( "external/glslang/glslang/OSDependent/Windows")
        removefiles { "external/glslang/glslang/OSDependent/Windows/main.cpp" }

    filter { "system:linux or macosx" }
        addSourceDir( "external/glslang/glslang/OSDependent/Unix")
        links { "dl", "pthread" }

--
-- With glslang's build out of the way, we've now covered everything we have
-- to build to get Slang and its tools/examples built.
--
-- What is not included in this file yet is support for any custom `make`
-- targets for:
--
-- * Invoking the test runner
-- * Packaging up binaries
-- * "Installing" Slang on a user's machine
--

end
