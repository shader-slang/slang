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
    if (const auto module = getModule())
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
    SLANG_UNUSED(builder);
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
        return getModule()->getFileDependencies();

    static List<SourceFile*> empty;
    return empty;
}

} // namespace Slang
