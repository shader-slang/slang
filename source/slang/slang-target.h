// slang-target.h
#pragma once

//
// This file declares the `TargetRequest` class, which is
// the primary way that the Slang compiler groups together
// a compilation target and options that affect output
// code generation and/or layout for that target.
//

#include "../core/slang-string.h"
#include "slang-ast-base.h"
#include "slang-compiler-fwd.h"
#include "slang-compiler-options.h"
#include "slang-hlsl-to-vulkan-layout-options.h"

#include <slang.h>

namespace Slang
{

enum class CodeGenTarget : SlangCompileTargetIntegral
{
    Unknown = SLANG_TARGET_UNKNOWN,
    None = SLANG_TARGET_NONE,
    GLSL = SLANG_GLSL,
    HLSL = SLANG_HLSL,
    SPIRV = SLANG_SPIRV,
    SPIRVAssembly = SLANG_SPIRV_ASM,
    DXBytecode = SLANG_DXBC,
    DXBytecodeAssembly = SLANG_DXBC_ASM,
    DXIL = SLANG_DXIL,
    DXILAssembly = SLANG_DXIL_ASM,
    CSource = SLANG_C_SOURCE,
    CPPSource = SLANG_CPP_SOURCE,
    PyTorchCppBinding = SLANG_CPP_PYTORCH_BINDING,
    HostCPPSource = SLANG_HOST_CPP_SOURCE,
    HostExecutable = SLANG_HOST_EXECUTABLE,
    HostSharedLibrary = SLANG_HOST_SHARED_LIBRARY,
    ShaderSharedLibrary = SLANG_SHADER_SHARED_LIBRARY,
    ShaderHostCallable = SLANG_SHADER_HOST_CALLABLE,
    CUDASource = SLANG_CUDA_SOURCE,
    PTX = SLANG_PTX,
    CUDAObjectCode = SLANG_CUDA_OBJECT_CODE,
    ObjectCode = SLANG_OBJECT_CODE,
    HostHostCallable = SLANG_HOST_HOST_CALLABLE,
    Metal = SLANG_METAL,
    MetalLib = SLANG_METAL_LIB,
    MetalLibAssembly = SLANG_METAL_LIB_ASM,
    WGSL = SLANG_WGSL,
    WGSLSPIRVAssembly = SLANG_WGSL_SPIRV_ASM,
    WGSLSPIRV = SLANG_WGSL_SPIRV,
    HostVM = SLANG_HOST_VM,
    CountOf = SLANG_TARGET_COUNT_OF,
};

bool isHeterogeneousTarget(CodeGenTarget target);

void printDiagnosticArg(StringBuilder& sb, CodeGenTarget val);

class TargetRequest;

/// Are we generating code for a D3D API?
bool isD3DTarget(TargetRequest* targetReq);

// Are we generating code for Metal?
bool isMetalTarget(TargetRequest* targetReq);

/// Are we generating code for a Khronos API (OpenGL or Vulkan)?
bool isKhronosTarget(TargetRequest* targetReq);
bool isKhronosTarget(CodeGenTarget target);

/// Are we generating code where SPIRV is the target?
bool isSPIRV(CodeGenTarget codeGenTarget);

/// Are we generating code for a CUDA API (CUDA / OptiX)?
bool isCUDATarget(TargetRequest* targetReq);

// Are we generating code for a CPU target
bool isCPUTarget(TargetRequest* targetReq);

/// Are we generating code for the WebGPU API?
bool isWGPUTarget(TargetRequest* targetReq);
bool isWGPUTarget(CodeGenTarget target);

// Are we generating code for a Kernel-style target (as opposed to host-style target)
bool isKernelTarget(CodeGenTarget codeGenTarget);

/// A request to generate output in some target format.
class TargetRequest : public RefObject
{
public:
    TargetRequest(Linkage* linkage, CodeGenTarget format);

    TargetRequest(const TargetRequest& other);

    Linkage* getLinkage() { return linkage; }

    Session* getSession();

    CodeGenTarget getTarget()
    {
        return optionSet.getEnumOption<CodeGenTarget>(CompilerOptionName::Target);
    }

    // TypeLayouts created on the fly by reflection API
    struct TypeLayoutKey
    {
        Type* type;
        slang::LayoutRules rules;
        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher.hashValue(type);
            hasher.hashValue(rules);
            return hasher.getResult();
        }
        bool operator==(TypeLayoutKey other) const
        {
            return type == other.type && rules == other.rules;
        }
    };
    Dictionary<TypeLayoutKey, RefPtr<TypeLayout>> typeLayouts;

    Dictionary<TypeLayoutKey, RefPtr<TypeLayout>>& getTypeLayouts() { return typeLayouts; }

    TypeLayout* getTypeLayout(Type* type, slang::LayoutRules rules);

    CompilerOptionSet& getOptionSet() { return optionSet; }

    CapabilitySet getTargetCaps();

    void setTargetCaps(CapabilitySet capSet);

    HLSLToVulkanLayoutOptions* getHLSLToVulkanLayoutOptions();

private:
    Linkage* linkage = nullptr;
    CompilerOptionSet optionSet;
    CapabilitySet cookedCapabilities;
    RefPtr<HLSLToVulkanLayoutOptions> hlslToVulkanOptions;
};

/// Are resource types "bindless" (implemented as ordinary data) on the given `target`?
bool areResourceTypesBindlessOnTarget(TargetRequest* target);

} // namespace Slang
