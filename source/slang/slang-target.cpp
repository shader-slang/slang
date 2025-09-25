// slang-target.cpp
#include "slang-target.h"

#include "../core/slang-type-text-util.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "slang-compiler.h"
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
    if (!hlslToVulkanOptions)
    {
        hlslToVulkanOptions = new HLSLToVulkanLayoutOptions();
        hlslToVulkanOptions->loadFromOptionSet(optionSet);
    }
    return hlslToVulkanOptions.get();
}

void TargetRequest::setTargetCaps(CapabilitySet capSet)
{
    cookedCapabilities = capSet;
}

CapabilitySet TargetRequest::getTargetCaps()
{
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
    case CodeGenTarget::PyTorchCppBinding:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::HostSharedLibrary:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::ShaderHostCallable:
        atoms.add(CapabilityName::cpp);
        break;

    case CodeGenTarget::CUDASource:
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


TypeLayout* TargetRequest::getTypeLayout(Type* type, slang::LayoutRules rules)
{
    SLANG_AST_BUILDER_RAII(getLinkage()->getASTBuilder());

    // TODO: We are not passing in a `ProgramLayout` here, although one
    // is nominally required to establish the global ordering of
    // generic type parameters, which might be referenced from field types.
    //
    // The solution here is to make sure that the reflection data for
    // uses of global generic/existential types does *not* include any
    // kind of index in that global ordering, and just refers to the
    // parameter instead (leaving the user to figure out how that
    // maps to the ordering via some API on the program layout).
    //
    auto layoutContext = getInitialLayoutContextForTarget(this, nullptr, rules);

    RefPtr<TypeLayout> result;
    auto key = TypeLayoutKey{type, rules};
    if (getTypeLayouts().tryGetValue(key, result))
        return result.Ptr();
    result = createTypeLayout(layoutContext, type);
    getTypeLayouts()[key] = result;
    return result.Ptr();
}

} // namespace Slang
