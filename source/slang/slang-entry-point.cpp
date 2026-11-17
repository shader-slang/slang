// slang-entry-point.cpp
#include "slang-entry-point.h"

#include "slang-compiler.h"
#include "slang-mangle.h"

namespace Slang
{

//
// EntryPoint
//

ISlangUnknown* EntryPoint::getInterface(const Guid& guid)
{
    if (guid == slang::IEntryPoint::getTypeGuid())
        return static_cast<slang::IEntryPoint*>(this);

    return Super::getInterface(guid);
}

RefPtr<EntryPoint> EntryPoint::create(
    Linkage* linkage,
    DeclRef<FuncDecl> funcDeclRef,
    Profile profile)
{
    RefPtr<EntryPoint> entryPoint =
        new EntryPoint(linkage, funcDeclRef.getName(), profile, funcDeclRef);
    entryPoint->m_mangledName = getMangledName(linkage->getASTBuilder(), funcDeclRef);
    // Seed with the function declaration's own requirements; `validateEntryPoint` replaces this
    // with the fuller entry-point set (adding stage-dependent contributions like `SV_` semantic
    // capabilities). The seed keeps the field non-null for paths that skip validation.
    entryPoint->m_inferredCapabilityRequirements =
        funcDeclRef.getDecl()->inferredCapabilityRequirements;
    return entryPoint;
}

RefPtr<EntryPoint> EntryPoint::createDummyForPassThrough(
    Linkage* linkage,
    Name* name,
    Profile profile)
{
    RefPtr<EntryPoint> entryPoint = new EntryPoint(linkage, name, profile, DeclRef<FuncDecl>());
    return entryPoint;
}

RefPtr<EntryPoint> EntryPoint::createDummyForDeserialize(
    Linkage* linkage,
    Name* name,
    Profile profile,
    String mangledName)
{
    RefPtr<EntryPoint> entryPoint = new EntryPoint(linkage, name, profile, DeclRef<FuncDecl>());
    entryPoint->m_mangledName = mangledName;
    return entryPoint;
}

EntryPoint::EntryPoint(Linkage* linkage, Name* name, Profile profile, DeclRef<FuncDecl> funcDeclRef)
    : ComponentType(linkage), m_name(name), m_profile(profile), m_funcDeclRef(funcDeclRef)
{
    // Collect any specialization parameters used by the entry point
    //
    _collectShaderParams();
}

Module* EntryPoint::getModule()
{
    return Slang::getModule(getFuncDecl());
}

Index EntryPoint::getSpecializationParamCount()
{
    return m_genericSpecializationParams.getCount() + m_existentialSpecializationParams.getCount();
}

SpecializationParam const& EntryPoint::getSpecializationParam(Index index)
{
    auto genericParamCount = m_genericSpecializationParams.getCount();
    if (index < genericParamCount)
    {
        return m_genericSpecializationParams[index];
    }
    else
    {
        return m_existentialSpecializationParams[index - genericParamCount];
    }
}

Index EntryPoint::getRequirementCount()
{
    // The only requirement of an entry point is the module that contains it.
    //
    // TODO: We will eventually want to support the case of an entry
    // point nested in a `struct` type, in which case there should be
    // a single requirement representing that outer type (so that multiple
    // entry points nested under the same type can share the storage
    // for parameters at that scope).

    // Note: the defensive coding is here because the
    // "dummy" entry points we create for pass-through
    // compilation will not have an associated module.
    //
    if (const auto module = getModule(); module)
    {
        return 1;
    }
    return 0;
}

RefPtr<ComponentType> EntryPoint::getRequirement(Index index)
{
    SLANG_UNUSED(index);
    SLANG_ASSERT(index == 0);
    SLANG_ASSERT(getModule());
    return getModule();
}

String EntryPoint::getEntryPointMangledName(Index index)
{
    SLANG_UNUSED(index);
    SLANG_ASSERT(index == 0);

    return m_mangledName;
}

String EntryPoint::getEntryPointNameOverride(Index index)
{
    SLANG_UNUSED(index);
    SLANG_ASSERT(index == 0);

    if (m_entryPointNameOverride.getLength() != 0)
        return m_entryPointNameOverride;
    return m_name ? m_name->text : "";
}

void EntryPoint::acceptVisitor(
    ComponentTypeVisitor* visitor,
    SpecializationInfo* specializationInfo)
{
    visitor->visitEntryPoint(this, as<EntryPointSpecializationInfo>(specializationInfo));
}

void EntryPoint::buildHash(DigestBuilder<SHA1>& builder)
{
    // Hash the entry point's identity: its name, name override, profile (which carries
    // the stage), and mangled name. An entry point's parameters take part in program
    // layout — an earlier entry point's `ParameterBlock` shifts later entry points'
    // descriptor sets — so two linked programs that differ only in a sibling entry
    // point generate different code for their shared entry point, and a cache key
    // that omits the sibling's identity (as `getEntryPointHash` does, since it names
    // only the requested entry point) collides. Only value-like fields are hashed:
    // the entry point's parameter *types* are part of the module source, which
    // `Module::buildHash` already digests, and name plus stage uniquely identify an
    // entry point within a module; pointer-bearing state must stay out so the same
    // program hashes identically across processes.
    builder.append(m_name ? m_name->text : String());
    builder.append(m_entryPointNameOverride);
    builder.append(m_profile.raw);
    builder.append(m_mangledName);
}

List<Module*> const& EntryPoint::getModuleDependencies()
{
    if (auto module = getModule())
        return module->getModuleDependencies();

    static List<Module*> empty;
    return empty;
}

List<SourceFile*> const& EntryPoint::getFileDependencies()
{
    if (const auto module = getModule())
        return module->getFileDependencies();

    static List<SourceFile*> empty;
    return empty;
}

} // namespace Slang
