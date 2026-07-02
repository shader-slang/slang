// slang-target.cpp
#include "slang-target.h"

#include "../core/slang-type-text-util.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "slang-compiler.h"
#include "slang-target-program.h"
#include "slang-type-layout.h"

namespace Slang
{

bool isHeterogeneousTarget(CodeGenTarget target)
{
    return ArtifactDescUtil::makeDescForCompileTarget(asExternal(target)).style ==
           ArtifactStyle::Host;
}

void printDiagnosticArg(StringBuilder& sb, CodeGenTarget val)
{
    UnownedStringSlice name = TypeTextUtil::getCompileTargetName(asExternal(val));
    name = name.getLength() ? name : toSlice("<unknown>");
    sb << name;
}

//
// TargetRequest
//

TargetRequest::TargetRequest(Linkage* linkage, CodeGenTarget format)
    : linkage(linkage)
{
    optionSet = linkage->m_optionSet;
    optionSet.add(CompilerOptionName::Target, format);
}

TargetRequest::TargetRequest(const TargetRequest& other)
    : RefObject(), linkage(other.linkage), optionSet(other.optionSet)
{
}


Session* TargetRequest::getSession()
{
    return linkage->getSessionImpl();
}

HLSLToVulkanLayoutOptions* TargetRequest::getHLSLToVulkanLayoutOptions()
{
    // Layout code can ask for these options from multiple threads; initialize them once.
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!hlslToVulkanOptions)
    {
        hlslToVulkanOptions = new HLSLToVulkanLayoutOptions();
        hlslToVulkanOptions->loadFromOptionSet(optionSet);
    }
    return hlslToVulkanOptions.get();
}

void TargetRequest::setTargetCaps(CapabilitySet capSet)
{
    // Some callers precompute capabilities while others read them during layout/codegen.
    std::lock_guard<std::mutex> lock(m_mutex);
    cookedCapabilities = capSet;
}

CapabilitySet TargetRequest::getTargetCaps()
{
    // Capabilities are derived lazily and shared across entry-point compiles for the same target.
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!cookedCapabilities.isEmpty())
        return cookedCapabilities;

    // The full `CapabilitySet` for the target will be computed
    // from the combination of the code generation format, and
    // the profile.
    //
    // Note: the preofile might have been set in a way that is
    // inconsistent with the output code format of SPIR-V, but
    // a profile of Direct3D Shader Model 5.1. In those cases,
    // the format should always override the implications in
    // the profile.
    //
    // TODO: This logic isn't currently taking int account
    // the information in the profile, because the current
    // `CapabilityAtom`s that we support don't include any
    // of the details there (e.g., the shader model versions).
    //
    // Eventually, we'd want to have a rich set of capability
    // atoms, so that most of the information about what operations
    // are available where can be directly encoded on the declarations.

    List<CapabilityName> atoms;

    // If the user specified a explicit profile, we should pull
    // a corresponding atom representing the target version from the profile.
    CapabilitySet profileCaps = optionSet.getProfile().getCapabilityName();

    bool isGLSLTarget = false;
    switch (getTarget())
    {
    case CodeGenTarget::GLSL:
        isGLSLTarget = true;
        atoms.add(CapabilityName::glsl);
        break;
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        if (getOptionSet().shouldEmitSPIRVDirectly())
        {
            // Default to SPIRV 1.5 if the user has not specified a target version.
            bool hasTargetVersionAtom = false;
            if (!profileCaps.isEmpty())
            {
                profileCaps.join(CapabilitySet(CapabilityName::spirv_1_0));
                for (auto profileCapAtomSet : profileCaps.getAtomSets())
                {
                    for (auto atom : profileCapAtomSet)
                    {
                        if (isTargetVersionAtom(asAtom(atom)))
                        {
                            atoms.add((CapabilityName)atom);
                            hasTargetVersionAtom = true;
                        }
                    }
                }
            }
            if (!hasTargetVersionAtom)
            {
                atoms.add(CapabilityName::spirv_1_5);
            }
            // If the user specified any SPIR-V extensions in the profile,
            // pull them in.
            for (auto profileCapAtomSet : profileCaps.getAtomSets())
            {
                for (auto atom : profileCapAtomSet)
                {
                    if (isSpirvExtensionAtom(asAtom(atom)))
                    {
                        atoms.add((CapabilityName)atom);
                        hasTargetVersionAtom = true;
                    }
                }
            }
        }
        else
        {
            isGLSLTarget = true;
            atoms.add(CapabilityName::glsl);
            profileCaps.addSpirvVersionFromOtherAsGlslSpirvVersion(profileCaps);
        }
        break;

    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
        atoms.add(CapabilityName::hlsl);
        break;

    case CodeGenTarget::CSource:
        atoms.add(CapabilityName::c);
        break;

    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::PyTorchCppBinding:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::HostSharedLibrary:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::HostObjectCode:
    case CodeGenTarget::ShaderObjectCode:
    case CodeGenTarget::HostLLVMIR:
    case CodeGenTarget::ShaderLLVMIR:
        if (isCPUTargetViaLLVM(this))
        {
            atoms.add(CapabilityName::llvm);
        }
        else
        {
            atoms.add(CapabilityName::cpp);
        }
        break;

    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PTX:
        atoms.add(CapabilityName::cuda);
        break;

    case CodeGenTarget::Metal:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::MetalLibAssembly:
        atoms.add(CapabilityName::metal);
        break;

    case CodeGenTarget::WGSLSPIRV:
    case CodeGenTarget::WGSLSPIRVAssembly:
    case CodeGenTarget::WGSL:
        atoms.add(CapabilityName::wgsl);
        break;

    default:
        break;
    }

    CapabilitySet targetCap = CapabilitySet(atoms);

    if (profileCaps.atLeastOneSetImpliedInOther(targetCap) ==
        CapabilitySet::ImpliesReturnFlags::Implied)
        targetCap.join(profileCaps);

    for (auto atomVal : optionSet.getArray(CompilerOptionName::Capability))
    {
        CapabilitySet toAdd;
        switch (atomVal.kind)
        {
        case CompilerOptionValueKind::Int:
            toAdd = CapabilitySet(CapabilityName(atomVal.intValue));
            break;
        case CompilerOptionValueKind::String:
            toAdd = CapabilitySet(findCapabilityName(atomVal.stringValue.getUnownedSlice()));
            break;
        }

        if (isGLSLTarget)
            targetCap.addSpirvVersionFromOtherAsGlslSpirvVersion(toAdd);

        if (!targetCap.isIncompatibleWith(toAdd))
            targetCap.join(toAdd);
    }

    cookedCapabilities = targetCap;

    SLANG_ASSERT(!cookedCapabilities.isInvalid());

    return cookedCapabilities;
}


TypeLayout* TargetRequest::getTypeLayout(
    Type* type,
    slang::LayoutRules rules,
    ProgramLayout* programLayout)
{
    SLANG_AST_BUILDER_RAII(getLinkage()->getASTBuilder());

    // When a `ProgramLayout` is supplied, the layout context can resolve
    // `extern` declarations against their link-time definitions (via
    // `buildExternTypeMap`) and establish the global ordering of generic type
    // parameters that might be referenced from field types. The reflection
    // entry point `spReflection_GetTypeLayout` always has the `ProgramLayout`
    // in hand, so it threads it through here; that way a query such as
    // `getTypeLayout` for a struct with an `extern` member resolves the member
    // to its concrete linked type rather than laying out the bare, unresolved
    // `extern` declaration (which would report zero fields and size 0).
    //
    // `programLayout` may still be null for the genuinely program-less entry
    // point (`Linkage::getTypeLayout`); in that case the layout context leaves
    // `extern`/global-generic references unresolved, which is correct for a
    // type that does not reference them.
    auto layoutContext = getInitialLayoutContextForTarget(this, programLayout, rules);

    // Choose where to cache. When a `ProgramLayout` is supplied, the resulting
    // `TypeLayout` is computed against that specific program (resolved `extern`
    // members, global-generic indices), so it must be cached with the program's
    // lifetime — on the owning `TargetProgram`, not on this session-long
    // `TargetRequest`. Caching a program-scoped result under a raw
    // `ProgramLayout*` key here would let a freed program's address be reused
    // by a later program and alias a stale entry.
    //
    // The program-less path has no such hazard: `Type*` lives in the
    // linkage-owned `ASTBuilder` arena that outlives every program, so its
    // entries stay on this `TargetRequest`.
    auto& typeLayoutCache =
        programLayout ? programLayout->getTargetProgram()->getTypeLayouts() : getTypeLayouts();

    RefPtr<TypeLayout> result;
    auto key = TypeLayoutKey{type, rules};
    if (typeLayoutCache.tryGetValue(key, result))
        return result.Ptr();
    result = createTypeLayout(layoutContext, type);
    typeLayoutCache[key] = result;
    return result.Ptr();
}

} // namespace Slang
