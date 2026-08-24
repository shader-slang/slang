// slang-target.cpp
#include "slang-target.h"

#include "compiler-core/slang-artifact-desc-util.h"
#include "core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-rich-diagnostics.h"
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

// Returns true if this target uses GLSL-flavored capability semantics: either the target
// literally emits GLSL, or it emits SPIRV via the GLSL-SPIRV pipeline (`-emit-spirv-via-glsl`,
// the default when direct SPIRV emission is off). Both paths run source through glslang,
// so both classify SPIRV version/extension capabilities the same way -- as `glsl_spirv_*`
// atoms rather than the `spirv_*` atoms a direct-SPIRV target uses (see
// `getTargetCaps()`, which performs that atom conversion for exactly the targets this
// function returns true for). A target emitting SPIRV directly skips glslang entirely and
// so keeps the plain `spirv_*` atoms.
bool TargetRequest::isGLSLBasedTarget()
{
    switch (getTarget())
    {
    case CodeGenTarget::GLSL:
        return true;
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        return !optionSet.shouldEmitSPIRVDirectly();
    default:
        return false;
    }
}

// static
CapabilitySet TargetRequest::decodeCapabilityOption(
    const CompilerOptionValue& atomVal,
    String* outName)
{
    switch (atomVal.kind)
    {
    case CompilerOptionValueKind::Int:
        if (outName)
            *outName = capabilityNameToString(CapabilityName(atomVal.intValue));
        return CapabilitySet(CapabilityName(atomVal.intValue));
    case CompilerOptionValueKind::String:
        if (outName)
            *outName = atomVal.stringValue;
        return CapabilitySet(findCapabilityName(atomVal.stringValue.getUnownedSlice()));
    default:
        return CapabilitySet();
    }
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

    bool isGLSLTarget = isGLSLBasedTarget();
    switch (getTarget())
    {
    case CodeGenTarget::GLSL:
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
                        // SPIR-V code-gen path: pull only SPIR-V version atoms from the profile.
                        if (isSpirvVersionAtom(asAtom(atom)))
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
        CapabilitySet toAdd = decodeCapabilityOption(atomVal);

        if (isGLSLTarget)
            targetCap.addSpirvVersionFromOtherAsGlslSpirvVersion(toAdd);

        if (!targetCap.isIncompatibleWith(toAdd))
            targetCap.join(toAdd);
    }

    cookedCapabilities = targetCap;

    SLANG_ASSERT(!cookedCapabilities.isInvalid());

    return cookedCapabilities;
}

void TargetRequest::checkCapabilities(DiagnosticSink* sink)
{
    // Every call site (currently only `FrontEndCompileRequest::checkEntryPoints()`) has a
    // live sink to diagnose into; a null sink here would mean a caller silently wants no
    // diagnostics, which is not a supported way to call this function.
    SLANG_RELEASE_ASSERT(sink);

    // A target's own `-capability` requests are fixed at target-construction time (see
    // Linkage::addTarget()) and never change afterward, but checkEntryPoints() -- and
    // therefore this function -- runs once per FrontEndCompileRequest, i.e. once per
    // module load, not once per target's lifetime. Without this latch, a persistent
    // session that loads many modules against the same target would re-derive and
    // re-diagnose the identical incompatibility on every single load.
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_capabilitiesChecked)
            return;
        m_capabilitiesChecked = true;
    }

    bool isGLSLTarget = isGLSLBasedTarget();
    auto cookedCaps = getTargetCaps();

    // Use the user-specified CodeGenTarget for the diagnostic so the error says e.g.
    // "incompatible with compilation target 'spirv'" even when the GLSL-SPIRV pipeline
    // is in use (where cookedCaps.getCompileTarget() would return 'glsl').
    auto userTargetName = TypeTextUtil::getCompileTargetName(asExternal(getTarget()));

    // Gather every explicitly requested `-capability` for this target as a
    // `SourcedCapabilityRequirement`. `atomVal.capabilitySource` was set when the entry was
    // produced (`CompilerOptionSet::load`/`addCapabilityAtom`) and survives `inheritFrom`
    // unchanged, so it already says whether this specific entry was requested at session
    // scope or for this target -- no need to reconstruct that by comparing option sets.
    List<SourcedCapabilityRequirement> requirements;
    for (auto atomVal : optionSet.getArray(CompilerOptionName::Capability))
    {
        // Decode the option value into a CapabilitySet and a display name in one place
        // (decodeCapabilityOption is shared with getTargetCaps()). An unknown/invalid
        // entry (e.g. SLANG_CAPABILITY_UNKNOWN, or a string that doesn't name a known
        // capability) decodes to an empty CapabilitySet, which the isEmpty() check below
        // skips -- there's no need for a separate up-front validity check.
        String requestedCapName;
        CapabilitySet toAdd = decodeCapabilityOption(atomVal, &requestedCapName);
        if (toAdd.isEmpty())
            continue;

        // For GLSL-SPIRV pipeline targets, SPIRV version caps are intentionally converted
        // to their glsl_spirv_* equivalents by getTargetCaps() (see isGLSLBasedTarget()'s
        // comment), so they are not an error here.
        //
        // TODO(https://github.com/shader-slang/slang/issues/12703): this exemption is
        // broader than the conversion it is meant to mirror. getTargetCaps() only
        // converts SPIRV *version* atoms, but this test also exempts SPIRV *extension*
        // atoms via the same "belongs to the spirv target family" check, so a SPIRV
        // extension capability requested on a GLSL-based target is silently dropped by
        // both getTargetCaps() and this check instead of being flagged. That mismatch
        // predates this function (getTargetCaps() has always silently dropped
        // incompatible explicit capability requests) and needs its own fix to
        // getTargetCaps()'s conversion/exemption logic; narrowing just this test would
        // just move where the silent drop happens.
        if (isGLSLTarget && toAdd.getCapabilityTargetSets().containsKey(CapabilityAtom::spirv))
            continue;

        requirements.add({toAdd, atomVal.capabilitySource, requestedCapName});
    }

    for (auto& incompatible : findIncompatibleCapabilityRequirements(cookedCaps, requirements))
    {
        // A session-level capability request is a "use this if it applies" broadcast to
        // every target in the session (see CapabilitySource::SessionOption), not a binding
        // requirement on this particular target, so it is expected -- not an error -- for
        // it to be incompatible here. A target-level request, by contrast, names this exact
        // target, so an incompatible one is a real mistake (most likely a mismatched
        // -target/-capability pairing) and must be diagnosed.
        if (incompatible.source == CapabilitySource::SessionOption)
            continue;

        maybeDiagnose(
            sink,
            getLinkage()->m_optionSet,
            DiagnosticCategory::Capability,
            Diagnostics::RequestedCapabilityIncompatibleWithTarget{
                .requestedCap = incompatible.label,
                .target = String(userTargetName)});
    }
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
