import os
from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy


class SlangConan(ConanFile):
    name = "slang"
    version = "2025.9.2"
    description = "Slang is a shading language that makes it easier to build and maintain large shader codebases in a modular and extensible fashion."
    license = "MIT"
    topics = ("shader", "compiler", "slang", "hlsl", "glsl", "spirv", "dxil", "metal")
    homepage = "https://github.com/shader-slang/slang"
    url = "https://github.com/shader-slang/slang"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    
    # Minimal exports - only what's needed to build the compiler
    # Note: examples/CMakeLists.txt is needed because root CMakeLists.txt calls add_subdirectory(examples)
    #       but examples won't be built since SLANG_ENABLE_EXAMPLES=OFF
    # External dependencies are selectively included:
    #   - miniz, lz4, unordered_dense: from Conan packages (not exported)
    #   - glm, imgui, lua, slang-rhi, stb, tinyobjloader: only for examples/tests (not exported)
    #   - glslang, spirv-*, vulkan, mimalloc, metal-cpp, dxc: needed for core compiler
    exports_sources = (
        "CMakeLists.txt",
        "slang-tag-version.h.in",
        "cmake/*",
        "source/*",
        "include/*",
        "prelude/*",
        "tools/*",
        "examples/CMakeLists.txt",  # Only the CMakeLists, not the actual example sources
        "LICENSE",
        # External dependencies needed for core compiler (not from Conan)
        "external/CMakeLists.txt",
        "external/glslang/*",
        "external/glslang-generated/*",
        "external/spirv/*",
        "external/spirv-headers/*",
        "external/spirv-tools/*",
        "external/spirv-tools-generated/*",
        "external/vulkan/*",
        "external/mimalloc/*",
        "external/metal-cpp/*",
        "external/dxc/*",
        "external/slang-binaries/*",
        "external/slang-tint-headers/*",
        "external/optix-dev/*",
        "external/WindowsToolchain/*",
        "external/lua/*",               # Required by slang-fiddle tool
    )
    
    # Dependency versions - matched to Slang's bundled external versions or latest compatible
    _miniz_version = "3.0.2"            # Conan latest compatible with bundled
    _lz4_version = "1.10.0"             # external/lz4 has 1.10.0
    _unordered_dense_version = "4.5.0"  # external/unordered_dense has 4.5.0
    # NOTE: Slang uses spirv-headers 1.5.5 with newer SPIR-V opcodes (e.g., OpRayQueryGetIntersectionClusterIdNV)
    # that are NOT available in Conan's latest packages (1.4.313.0). These must use bundled versions.
    # The spirv-tools, glslang, spirv-headers, and vulkan-headers are all tightly coupled.

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        # Core module options
        "embed_core_module": [True, False],
        "embed_core_module_source": [True, False],
        # Build targets
        "enable_gfx": [True, False],
        "enable_slang_rhi": [True, False],
        "enable_slangd": [True, False],
        "enable_slangc": [True, False],
        "enable_slangi": [True, False],
        "enable_slangrt": [True, False],
        "enable_slang_glslang": [True, False],
        "enable_tests": [True, False],
        "enable_examples": [True, False],
        "enable_replayer": [True, False],
        # Code generation options
        "enable_dxil": [True, False],
        # CUDA/OptiX options
        "enable_cuda": [True, False],
        "enable_optix": [True, False],
        # Debug/validation options
        "enable_full_ir_validation": [True, False],
        "enable_ir_break_alloc": [True, False],
        "enable_asan": [True, False],
        "enable_coverage": [True, False],
        # Release build options
        "enable_release_debug_info": [True, False],
        "enable_release_lto": [True, False],
        "enable_split_debug_info": [True, False],
        # Prebuilt binaries
        "enable_prebuilt_binaries": [True, False],
        # LLVM flavor
        "slang_llvm_flavor": ["FETCH_BINARY", "FETCH_BINARY_IF_POSSIBLE", "USE_SYSTEM_LLVM", "DISABLE"],
        # System library options
        "use_system_miniz": [True, False],
        "use_system_lz4": [True, False],
        "use_system_vulkan_headers": [True, False],
        "use_system_spirv_headers": [True, False],
        "use_system_spirv_tools": [True, False],
        "use_system_glslang": [True, False],
        "use_system_unordered_dense": [True, False],
        # Performance options
        "enable_spirv_tools_mimalloc": [True, False],
        # Advanced options
        "exclude_dawn": [True, False],
        "exclude_tint": [True, False],
        "prototype_diagnostics": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
        # Core module options - default ON
        "embed_core_module": True,
        "embed_core_module_source": True,
        # Build targets - most OFF for minimal build
        "enable_gfx": False,
        "enable_slang_rhi": False,
        "enable_slangd": True,
        "enable_slangc": True,
        "enable_slangi": True,
        "enable_slangrt": True,
        "enable_slang_glslang": True,
        "enable_tests": False,
        "enable_examples": False,
        "enable_replayer": False,
        # Code generation - default ON
        "enable_dxil": True,
        # CUDA/OptiX - default OFF
        "enable_cuda": False,
        "enable_optix": False,
        # Debug/validation - default OFF
        "enable_full_ir_validation": False,
        "enable_ir_break_alloc": False,
        "enable_asan": False,
        "enable_coverage": False,
        # Release build options
        "enable_release_debug_info": True,
        "enable_release_lto": True,
        "enable_split_debug_info": True,
        # Prebuilt binaries - default ON
        "enable_prebuilt_binaries": True,
        # LLVM flavor - default to fetching binary if possible
        "slang_llvm_flavor": "FETCH_BINARY_IF_POSSIBLE",
        # System library options - use Conan packages where compatible
        "use_system_miniz": True,           # Use Conan package
        "use_system_lz4": True,             # Use Conan package
        "use_system_vulkan_headers": False, # Bundled (newer than Conan)
        "use_system_spirv_headers": False,  # Bundled (v1.5.5, newer than Conan's 1.4.313.0)
        "use_system_spirv_tools": False,    # Bundled (requires matching spirv-headers)
        "use_system_glslang": False,        # Bundled (requires matching spirv-tools)
        "use_system_unordered_dense": True, # Use Conan package
        # Performance options - platform dependent
        "enable_spirv_tools_mimalloc": False,
        # Advanced options - default OFF
        "exclude_dawn": True,
        "exclude_tint": True,
        "prototype_diagnostics": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        else:
            # mimalloc for spirv-tools is Windows-only by default
            self.options.enable_spirv_tools_mimalloc = False

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def requirements(self):
        # Add Conan package dependencies based on use_system_* options
        if self.options.use_system_miniz:
            self.requires(f"miniz/{self._miniz_version}")
        if self.options.use_system_lz4:
            self.requires(f"lz4/{self._lz4_version}")
        if self.options.use_system_unordered_dense:
            self.requires(f"unordered_dense/{self._unordered_dense_version}")
        # NOTE: spirv-headers, spirv-tools, glslang, and vulkan-headers are NOT available
        # as Conan packages with compatible versions. Slang requires spirv-headers 1.5.5+
        # but Conan only has up to 1.4.313.0. These must use bundled versions.

    def build_requirements(self):
        self.tool_requires("cmake/4.0.3")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        
        # Convert shared option to SLANG_LIB_TYPE
        lib_type = "SHARED" if self.options.shared else "STATIC"
        
        cmake.configure(
            variables={
                # Library type
                "SLANG_LIB_TYPE": lib_type,
                "SLANG_BUILD_STATIC": not self.options.shared,
                # Core module options
                "SLANG_EMBED_CORE_MODULE": self.options.embed_core_module,
                "SLANG_EMBED_CORE_MODULE_SOURCE": self.options.embed_core_module_source,
                # Build targets
                "SLANG_ENABLE_GFX": self.options.enable_gfx,
                "SLANG_ENABLE_SLANG_RHI": self.options.enable_slang_rhi,
                "SLANG_ENABLE_SLANGD": self.options.enable_slangd,
                "SLANG_ENABLE_SLANGC": self.options.enable_slangc,
                "SLANG_ENABLE_SLANGI": self.options.enable_slangi,
                "SLANG_ENABLE_SLANGRT": self.options.enable_slangrt,
                "SLANG_ENABLE_SLANG_GLSLANG": self.options.enable_slang_glslang,
                "SLANG_ENABLE_TESTS": self.options.enable_tests,
                "SLANG_ENABLE_EXAMPLES": self.options.enable_examples,
                "SLANG_ENABLE_REPLAYER": self.options.enable_replayer,
                # Code generation
                "SLANG_ENABLE_DXIL": self.options.enable_dxil,
                # CUDA/OptiX
                "SLANG_ENABLE_CUDA": self.options.enable_cuda,
                "SLANG_ENABLE_OPTIX": self.options.enable_optix,
                # Debug/validation
                "SLANG_ENABLE_FULL_IR_VALIDATION": self.options.enable_full_ir_validation,
                "SLANG_ENABLE_IR_BREAK_ALLOC": self.options.enable_ir_break_alloc,
                "SLANG_ENABLE_ASAN": self.options.enable_asan,
                "SLANG_ENABLE_COVERAGE": self.options.enable_coverage,
                # Release build options
                "SLANG_ENABLE_RELEASE_DEBUG_INFO": self.options.enable_release_debug_info,
                "SLANG_ENABLE_RELEASE_LTO": self.options.enable_release_lto,
                "SLANG_ENABLE_SPLIT_DEBUG_INFO": self.options.enable_split_debug_info,
                # Prebuilt binaries
                "SLANG_ENABLE_PREBUILT_BINARIES": self.options.enable_prebuilt_binaries,
                # LLVM flavor
                "SLANG_SLANG_LLVM_FLAVOR": str(self.options.slang_llvm_flavor),
                # System library options
                "SLANG_USE_SYSTEM_MINIZ": self.options.use_system_miniz,
                "SLANG_USE_SYSTEM_LZ4": self.options.use_system_lz4,
                "SLANG_USE_SYSTEM_VULKAN_HEADERS": self.options.use_system_vulkan_headers,
                "SLANG_USE_SYSTEM_SPIRV_HEADERS": self.options.use_system_spirv_headers,
                "SLANG_USE_SYSTEM_SPIRV_TOOLS": self.options.use_system_spirv_tools,
                "SLANG_USE_SYSTEM_GLSLANG": self.options.use_system_glslang,
                "SLANG_USE_SYSTEM_UNORDERED_DENSE": self.options.use_system_unordered_dense,
                # Performance options
                "SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC": self.options.enable_spirv_tools_mimalloc,
                # Advanced options
                "SLANG_EXCLUDE_DAWN": self.options.exclude_dawn,
                "SLANG_EXCLUDE_TINT": self.options.exclude_tint,
                "SLANG_PROTOTYPE_DIAGNOSTICS": self.options.prototype_diagnostics,
            }
        )
        cmake.build()

    def package(self):
        # Copy license
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        
        # Copy headers
        copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))
        copy(self, "slang.h", src=self.source_folder, dst=os.path.join(self.package_folder, "include"))
        copy(self, "slang-com-ptr.h", src=self.source_folder, dst=os.path.join(self.package_folder, "include"))
        copy(self, "slang-com-helper.h", src=self.source_folder, dst=os.path.join(self.package_folder, "include"))
        copy(self, "slang-gfx.h", src=self.source_folder, dst=os.path.join(self.package_folder, "include"))
        copy(self, "slang-tag-version.h", src=self.source_folder, dst=os.path.join(self.package_folder, "include"))
        
        # Copy prelude headers
        copy(self, "*.h", src=os.path.join(self.source_folder, "prelude"), dst=os.path.join(self.package_folder, "include", "prelude"))
        
        # Copy static libraries
        copy(self, "*.a", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.lib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        
        # Copy shared libraries
        copy(self, "*.so", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.so.*", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.dylib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.dll", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path=False)
        
        # Copy executables
        copy(self, "*.exe", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path=False)
        # Linux/macOS executables - copy specific binaries without extension
        if self.settings.os != "Windows":
            bin_folder = os.path.join(self.build_folder, "bin")
            if os.path.exists(bin_folder):
                for exe_name in ["slangc", "slangd", "slangi"]:
                    copy(self, exe_name, src=bin_folder, dst=os.path.join(self.package_folder, "bin"), keep_path=False)

    def package_info(self):
        # Main slang library
        self.cpp_info.libs = ["slang"]
        
        # Add runtime library if enabled
        if self.options.enable_slangrt:
            self.cpp_info.libs.append("slang-rt")
        
        # Add glslang wrapper if enabled
        if self.options.enable_slang_glslang:
            self.cpp_info.libs.append("slang-glslang")
        
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.bindirs = ["bin"]
        
        # Add slangc to PATH for consumers
        if self.options.enable_slangc:
            self.buildenv_info.prepend_path("PATH", os.path.join(self.package_folder, "bin"))
