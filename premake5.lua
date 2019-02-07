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
-- buidl, then you may need to read up on Premake's approach and
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
-- % premake5 --os=linux gmake --build-location="build.linux"
--
-- % cd build.linux
-- % make config=release_x64
-- or 
-- % make config=debug_x64
--
-- From in the build directory you can use
-- % premake5 --file=../premake5.lua --os=linux gmake 

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

buildLocation = _OPTIONS["build-location"]
executeBinary = (_OPTIONS["execute-binary"] == "true")

overrideModule = {}
local overrideModulePath = _OPTIONS["override-module"]
if overrideModulePath then
    overrideModule = require(overrideModulePath)
end

targetName = "%{cfg.system}-%{cfg.platform:lower()}"

workspace "slang"
    -- We will support debug/release configuration and x86/x64 builds.
    configurations { "Debug", "Release" }
    platforms { "x86", "x64"}
    
    if os.target() == "linux" then
        platforms {"aarch64" }
    end
    
    if buildLocation then
        location(buildLocation)
    end
    
    -- The output binary directory will be derived from the OS
    -- and configuration options, e.g. `bin/windows-x64/debug/`
    targetdir("bin/" .. targetName .. "/%{cfg.buildcfg:lower()}")

    -- Statically link to the C/C++ runtime rather than create a DLL dependency.
    flags { "StaticRuntime" }

    -- Once we've set up the common settings, we will make some tweaks
    -- that only apply in a subset of cases. Each call to `filter()`
    -- changes the "active" filter for subsequent commands. In
    -- effect, those commands iwll be ignored when the conditions of
    -- the filter aren't satisfied.

    -- Our `x64` platform should (obviously) target the x64
    -- architecture and similarly for x86.
    filter { "platforms:x64" }
        architecture "x64"
    filter { "platforms:x86" }
        architecture "x86"
    filter { "platforms:aarch64"}
        architecture "ARM"

    filter { "toolset:clang or gcc*" }
        buildoptions { "-Wno-unused-parameter", "-Wno-type-limits", "-Wno-sign-compare", "-Wno-unused-variable", "-Wno-reorder", "-Wno-switch", "-Wno-return-type", "-Wno-unused-local-typedefs", "-Wno-parentheses",  "-std=c++11", "-fvisibility=hidden" , "-Wno-ignored-optimization-argument", "-Wno-unknown-warning-option"} 
    	
	filter { "toolset:gcc*"}
		buildoptions { "-Wno-unused-but-set-variable", "-Wno-implicit-fallthrough"  }
		
    filter { "toolset:clang" }
         buildoptions { "-Wno-deprecated-register", "-Wno-tautological-compare", "-Wno-missing-braces", "-Wno-undefined-var-template", "-Wno-unused-function"}
		
    -- When compiling the debug configuration, we want to turn
    -- optimization off, make sure debug symbols are output,
    -- and add the same preprocessor definition that VS
    -- would add by default.
    filter { "configurations:debug" }
        optimize "Off"
        symbols "On"
        defines { "_DEBUG" }

    -- For the release configuration we will turn optimizations on
    -- (we do not yet micro-manage the optimization settings)
    -- and set the preprocessor definition that VS would add by default.
    filter { "configurations:release" }
        optimize "On"
        defines { "NDEBUG" }
    		
    filter { "system:linux" }
        linkoptions{  "-Wl,-rpath,'$$ORIGIN',--no-as-needed", "-ldl"}
        
            
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
function addSourceDir(path)
    files
    {
        path .. "/*.cpp",       -- C++ source files
        path .. "/*.slang",     -- Slang files (for our stdlib)
        path .. "/*.h",         -- Header files
        path .. "/*.hpp",       -- C++ style headers (for glslang)
        path .. "/*.natvis",    -- Visual Studio debugger visualization files
    }
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
    uuid(os.uuid(name .. '|' .. sourceDir))

    -- Set the location where the project file will be placed.
    -- We set the project files to reside in their source
    -- directory, because in Visual Studio the default
    -- working directory when launching a project in the
    -- debugger is its project directory. This ensures that
    -- examples will work as expected for VS users.
    --
    -- TODO: consider only setting this for examples, since
    -- it is less relevant to other projects.
    --

    location(sourceDir)
    
    if os.target() == "windows" then
    else
        location "intermediate/project/%{prj.name}"
    end
    
    -- The intermediate ("object") directory will use a similar
    -- naming scheme to the output directory, but will also use
    -- the project name to avoid cases where multiple projects
    -- have source files with the same name.
    objdir("intermediate/" .. targetName .. "/%{cfg.buildcfg:lower()}/%{prj.name}")
    
    -- All of our projects are written in C++.
    --
    language "C++"

    -- Since we know the project directory, we can go ahead
    -- and add any source files locate there.
    --
    -- Note that we do *not* recurse into subdirectories,
    -- so projects that spread their source over multiple
    -- directories will need to take more steps.
    --
    addSourceDir(sourceDir)

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
       { ["Source Files"] = { "**.cpp", "**.slang", "**.natvis" } },
    }

    -- Override default options for a project if necessary

    if overrideModule.addBaseProjectOptions then
        overrideModule.addBaseProjectOptions()
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
end

-- "Standard" projects will be those that go to make the binary
-- packages for slang: the shared libraries and executables.
--
function standardProject(name)
    -- Because Premake is stateful, any `group()` call by another
    -- project would still be in effect when we create a project
    -- here (e.g., if somebody had called `tool()` before
    -- `standardProject()`), so we are careful here to set the
    -- group to an emptry string, which Premake treats as "no group."
    --
    group ""

    -- A standard project has its code under `source/`
    --
    baseSlangProject(name, "source/" .. name)
end

function toolSharedLibrary(name)
    group "test-tool"
    -- specifying that the project lives under the `tools/` path.
    --
    baseSlangProject(name .. "-tool", "tools/" .. name)
    
    defines { "SLANG_SHARED_LIBRARY_TOOL" }
   
    kind "SharedLib"
end

-- Finally we have the example programs that show how to use Slang.
--
function example(name)
    -- Example programs go into an "example" group
    group "examples"

    -- They have their source code under `examples/<project-name>/`
    baseSlangProject(name, "examples/" .. name)

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
    links { "slang", "core", "gfx" }
end

if os.target() == "windows" then
    --
    -- With all of these helper routines defined, we can now define the
    -- actual projects quite simply. For example, here is the entire
    -- declaration of the "Hello, World" example project:
    --
    example "hello-world"
    --
    -- Note how we are calling our custom `example()` subroutine with
    -- the same syntax sugar that Premake usually advocates for their
    -- `project()` function. This allows us to treat `example` as
    -- a kind of specialized "subclass" of `project`
    --

    -- Let's go ahead and set up the projects for our other example now.
    example "model-viewer"
end

-- Most of the other projects have more interesting configuration going
-- on, so let's walk through them in order of increasing complexity.
--
-- The `core` project is a static library that has all the basic types
-- and routines that get shared across both the Slang compiler/runtime
-- and the various tool projects. It's build is pretty simple:
--

standardProject "core"
    uuid "F9BE7957-8399-899E-0C49-E714FDDD4B65"
    kind "StaticLib"

    -- For our core implementation, we want to use the most
    -- aggressive warning level supported by the target, and
    -- to treat every warning as an error to make sure we
    -- keep our code free of warnings.
    --
    warnings "Extra"
    flags { "FatalWarnings" }
    
    -- We need the core library to be relocatable to be able to link with slang.so
    filter { "system:linux" }
        buildoptions{"-fPIC"}
    
--
-- `slang-generate` is a tool we use for source code generation on
-- the compiler. It depends on the `core` library, so we need to
-- declare that:
--

tool "slang-generate"
    uuid "66174227-8541-41FC-A6DF-4764FC66F78E"
    links { "core" }


--
-- The `slang-test` test driver also uses the `core` library, and it
-- currently relies on include paths being set up so that it can find
-- the core headers:
--

tool "slang-test"
    uuid "0C768A18-1D25-4000-9F37-DA5FE99E3B64"
    includedirs { "." }
    links { "core", "slang" }

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

if os.target() == "windows" then    
    toolSharedLibrary "render-test"
        uuid "61F7EB00-7281-4BF3-9470-7C2EA92620C3"
        
        includedirs { ".", "external", "source", "tools/gfx" }
        links { "core", "slang", "gfx" }
        
        systemversion "10.0.14393.0"

        -- For Windows targets, we want to copy d3dcompiler_47.dll,
        -- dxcompiler.dll, and dxil.dll from the Windows SDK redistributable
        -- directory into the output directory.
        postbuildcommands { '"$(SolutionDir)tools\\copy-hlsl-libs.bat" "$(WindowsSdkDir)Redist/D3D/%{cfg.platform:lower()}/" "%{cfg.targetdir}/"'}
       
end
            
--
-- `gfx` is a utility library for doing GPU rendering
-- and compute, which is used by both our testing and exmaples.
-- It depends on teh `core` library, so we need to declare that:
--

tool "gfx"
    uuid "222F7498-B40C-4F3F-A704-DDEB91A4484A"
    -- Unlike most of the code under `tools/`, this is a library
    -- rather than a stand-alone executable.
    kind "StaticLib"

    includedirs { ".", "external", "source", "external/imgui" }

    filter { "system:windows" }

        systemversion "10.0.14393.0"

        -- For Windows targets, we want to copy d3dcompiler_47.dll,
        -- dxcompiler.dll, and dxil.dll from the Windows SDK redistributable
        -- directory into the output directory.
        postbuildcommands { '"$(SolutionDir)tools\\copy-hlsl-libs.bat" "$(WindowsSdkDir)Redist/D3D/%{cfg.platform:lower()}/" "%{cfg.targetdir}/"'}

        filter { "system:not windows" }
            removefiles { "tools/gfx/circular-resource-heap-d3d12.cpp", "tools/gfx/d3d-util.cpp", "tools/gfx/descriptor-heap-d3d12.cpp", "tools/gfx/render-d3d11.cpp", "tools/gfx/render-d3d12.cpp", "tools/gfx/render-gl.cpp", "tools/gfx/resource-d3d12.cpp", "tools/gfx/render-vk.cpp", "tools/gfx/vk-swap-chain.cpp", "tools/gfx/window.cpp" }

--
-- The `slangc` command-line application is just a very thin wrapper
-- around the Slang dynamic library, so its build is extermely simple.
-- One windows `slangc` uses the the `core` library for some UTF-16
-- to UTF-8 string conversion before calling into `slang.dll`, so
-- it also depends on `core`:
--

standardProject "slangc"
    uuid "D56CBCEB-1EB5-4CA8-AEC4-48EA35ED61C7"
    kind "ConsoleApp"
    links { "core", "slang" }
    
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

standardProject "slang"
    uuid "DB00DA62-0533-4AFD-B59F-A67D5B3A0808"
    kind "SharedLib"
    links { "core" }
    warnings "Extra"
    flags { "FatalWarnings" }

    -- The way that we currently configure things through `slang.h`,
    -- we need to set a preprocessor definitions to ensure that
    -- we declare the Slang API functions for *export* and not *import*.
    --
    defines { "SLANG_DYNAMIC_EXPORT" }

    -- The `standardProject` operation already added all the code in
    -- `source/slang/*`, but we also want to incldue the umbrella
    -- `slang.h` header in this prject, so we do that manually here.
    files { "slang.h" }

    -- The most challenging part of building `slang` is that we need
    -- to invoke the `slang-generate` tool to generate the version
    -- of the Slang standard library that we embed into the compiler.
    --
    -- First, we need to ensure that `slang-generate` gets built
    -- before `slang`, so we declare a non-linking dependency between
    -- the projects here:
    --
    dependson { "slang-generate" }

    filter { "system:linux" }
	-- might be able to do pic(true)
        buildoptions{"-fPIC"}
       
    -- Next, we want to add a custom build rule for each of the
    -- files that makes up the standard library. Those are
    -- always named `*.meta.slang`, so we can select for them
    -- using a `filter` and then use Premake's support for
    -- defining custom build commands:
    --
    if executeBinary then
        filter "files:**.meta.slang"
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
            buildcommands { '"%{cfg.targetdir}/slang-generate" %{file.relpath}' }

            -- Given `foo.meta.slang` we woutput `foo.meta.slang.h`.
            -- This needs to be specified because the custom action will only
            -- run when this file needs to be generated.
            --
            buildoutputs { "%{file.relpath}.h" }

            -- We will specify an additional build input dependency on the `slang-generate`
            -- tool itself, so that changes to the code for the tool cause the generation
            -- step to be re-run.
            --
            -- In order to get the file name right, we need to know the executable suffix
            -- that the target platform will use. Premake might have a built-in way to
            -- query this, but I couldn't find it, so I am just winging it for now:
            --
            local executableSuffix = "";
            if(os.target() == "windows") then
                executableSuffix = ".exe";
            end
            --
            buildinputs { "%{cfg.targetdir}/slang-generate" .. executableSuffix }
    end

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
standardProject "slang-glslang"
    uuid "C495878A-832C-485B-B347-0998A90CC936"
    kind "SharedLib"
    includedirs { "external/glslang" }

    defines
    {
        -- `ENABLE_OPT` must be defined (to either zero or one) for glslang to compile at all
        "ENABLE_OPT=0",

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
    addSourceDir("external/glslang/glslang/OSDependent")
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

    filter { "system:windows" }
        -- On Windows we need to add the platform-specific sources and then
        -- remove the `main.cpp` file since it tries to define a `DllMain`
        -- and we don't want the default glslang one.
        addSourceDir( "external/glslang/glslang/OSDependent/Windows" )
        removefiles { "external/glslang/glslang/OSDependent/Windows/main.cpp" }

    filter { "system:linux" }
        links { "dl", "pthread" }
        addSourceDir("external/glslang/glslang/OSDependent/Unix")
        buildoptions{"-fPIC", "-pthread"}
        
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
