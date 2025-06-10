// slang-ast-decl.cpp
#include "slang-ast-decl.h"

#include "slang-ast-builder.h"
#include "slang-ast-dispatch.h"
#include "slang-syntax.h"

#include <assert.h>

namespace Slang
{

const TypeExp& TypeConstraintDecl::getSup() const
{
    SLANG_AST_NODE_VIRTUAL_CALL(TypeConstraintDecl, getSup, ())
}

const TypeExp& TypeConstraintDecl::_getSupOverride() const
{
    SLANG_UNEXPECTED("TypeConstraintDecl::_getSupOverride not overridden");
    // return TypeExp::empty;
}

InterfaceDecl* findParentInterfaceDecl(Decl* decl)
{
    auto ancestor = decl->parentDecl;
    for (; ancestor; ancestor = ancestor->parentDecl)
    {
        if (auto interfaceDecl = as<InterfaceDecl>(ancestor))
            return interfaceDecl;

        if (as<ExtensionDecl>(ancestor))
            return nullptr;
    }
    return nullptr;
}

bool isInterfaceRequirement(Decl* decl)
{
    auto ancestor = decl->parentDecl;
    for (; ancestor; ancestor = ancestor->parentDecl)
    {
        if (as<InterfaceDecl>(ancestor))
            return true;

        if (as<ExtensionDecl>(ancestor))
            return false;
    }
    return false;
}

//
// ContainerDecl
//

List<Decl*> const& ContainerDecl::getDirectMemberDecls()
{
    return _directMemberDecls.decls;
}

Count ContainerDecl::getDirectMemberDeclCount()
{
    return _directMemberDecls.decls.getCount();
}

Decl* ContainerDecl::getDirectMemberDecl(Index index)
{
    return _directMemberDecls.decls[index];
}

Decl* ContainerDecl::getFirstDirectMemberDecl()
{
    if (getDirectMemberDeclCount() == 0)
        return nullptr;
    return getDirectMemberDecl(0);
}

DeclsOfNameList ContainerDecl::getDirectMemberDeclsOfName(Name* name)
{
    return DeclsOfNameList(findLastDirectMemberDeclOfName(name));
}

Decl* ContainerDecl::findLastDirectMemberDeclOfName(Name* name)
{
    _ensureLookupAcceleratorsAreValid();
    if (auto found = _directMemberDecls.accelerators.mapNameToLastDeclOfThatName.tryGetValue(name))
        return *found;
    return nullptr;
}

Decl* ContainerDecl::getPrevDirectMemberDeclWithSameName(Decl* decl)
{
    SLANG_ASSERT(decl);
    SLANG_ASSERT(decl->parentDecl == this);

    _ensureLookupAcceleratorsAreValid();
    return decl->_prevInContainerWithSameName;
}

void ContainerDecl::addDirectMemberDecl(Decl* decl)
{
    if (!decl)
        return;

    decl->parentDecl = this;
    _directMemberDecls.decls.add(decl);
}

List<Decl*> const& ContainerDecl::getTransparentDirectMemberDecls()
{
    _ensureLookupAcceleratorsAreValid();
    return _directMemberDecls.accelerators.filteredListOfTransparentDecls;
}

bool ContainerDecl::_areLookupAcceleratorsValid()
{
    return _directMemberDecls.accelerators.declCountWhenLastUpdated ==
           _directMemberDecls.decls.getCount();
}

void ContainerDecl::_invalidateLookupAccelerators()
{
    _directMemberDecls.accelerators.declCountWhenLastUpdated = -1;
}

void ContainerDecl::_ensureLookupAcceleratorsAreValid()
{
    if (_areLookupAcceleratorsValid())
        return;

    // If the `declCountWhenLastUpdated` is less than zero, it means that
    // the accelerators are entirely invalidated, and must be rebuilt
    // from scratch.
    //
    if (_directMemberDecls.accelerators.declCountWhenLastUpdated < 0)
    {
        _directMemberDecls.accelerators.declCountWhenLastUpdated = 0;
        _directMemberDecls.accelerators.mapNameToLastDeclOfThatName.clear();
        _directMemberDecls.accelerators.filteredListOfTransparentDecls.clear();
    }

    // are we a generic?
    GenericDecl* genericDecl = as<GenericDecl>(this);

    Count memberCount = _directMemberDecls.decls.getCount();
    Count memberCountWhenLastUpdated = _directMemberDecls.accelerators.declCountWhenLastUpdated;

    SLANG_ASSERT(memberCountWhenLastUpdated >= 0 && memberCountWhenLastUpdated <= memberCount);

    for (Index i = memberCountWhenLastUpdated; i < memberCount; ++i)
    {
        Decl* memberDecl = _directMemberDecls.decls[i];

        // Transparent member declarations will go into a separate list,
        // so that they can be conveniently queried later for lookup
        // operations.
        //
        // TODO: Rather than track these using a separate table, we
        // could design a scheme where transparent members are put into
        // the same lookup dictionary as everything else, just under
        // a pseudo-name that identifies transparent members.
        //
        if (memberDecl->hasModifier<TransparentModifier>())
        {
            _directMemberDecls.accelerators.filteredListOfTransparentDecls.add(memberDecl);
        }

        // Members that don't have a name don't go into the lookup dictionary.
        //
        auto memberName = memberDecl->getName();
        if (!memberName)
            continue;

        // As a special case, we ignore the `inner` member of a
        // `GenericDecl`, since it will always have the same name
        // as the outer generic, and should not be found by lookup.
        //
        // TODO: We really ought to change up our entire encoding
        // of generic declarations in the AST.
        //
        if (genericDecl && memberDecl == genericDecl->inner)
            continue;

        // It is possible that we have encountered previous declarations
        // that have the same name as `memberDecl`, and in that
        // case we want to wire them up into a singly-linked list.
        //
        // This list makes it easy for a lookup operation to find, e.g.,
        // all of the overloaded functions with a given name.
        //
        Decl* prevMemberWithSameName = nullptr;
        _directMemberDecls.accelerators.mapNameToLastDeclOfThatName.tryGetValue(
            memberName,
            prevMemberWithSameName);
        memberDecl->_prevInContainerWithSameName = prevMemberWithSameName;

        // Whether or not there was a previous declaration with this
        // name, the current `memberDecl` is the last member declaration
        // with that name encountered so far, and it is what we will
        // store in the lookup dictionary.
        //
        _directMemberDecls.accelerators.mapNameToLastDeclOfThatName[memberName] = memberDecl;
    }

    _directMemberDecls.accelerators.declCountWhenLastUpdated = memberCount;
    SLANG_ASSERT(_areLookupAcceleratorsValid());
}

void ContainerDecl::
    _invalidateLookupAcceleratorsBecauseUnscopedEnumAttributeWillBeTurnedIntoTransparentModifier(
        UnscopedEnumAttribute* unscopedEnumAttr,
        TransparentModifier* transparentModifier)
{
    SLANG_ASSERT(unscopedEnumAttr);
    SLANG_ASSERT(transparentModifier);

    SLANG_UNUSED(unscopedEnumAttr);
    SLANG_UNUSED(transparentModifier);

    _invalidateLookupAccelerators();
}

void ContainerDecl::
    _removeDirectMemberConstructorDeclBecauseSynthesizedAnotherDefaultConstructorInstead(
        ConstructorDecl* decl)
{
    SLANG_ASSERT(decl);

    _invalidateLookupAccelerators();
    _directMemberDecls.decls.remove(decl);
}

void ContainerDecl::
    _replaceDirectMemberBitFieldVariableDeclAtIndexWithPropertyDeclThatWasSynthesizedForIt(
        Index index,
        VarDecl* oldDecl,
        PropertyDecl* newDecl)
{
    SLANG_ASSERT(oldDecl);
    SLANG_ASSERT(newDecl);
    SLANG_ASSERT(index >= 0 && index < getDirectMemberDeclCount());
    SLANG_ASSERT(getDirectMemberDecl(index) == oldDecl);

    SLANG_UNUSED(oldDecl);
    _invalidateLookupAccelerators();
    _directMemberDecls.decls[index] = newDecl;
}

void ContainerDecl::_insertDirectMemberDeclAtIndexForBitfieldPropertyBackingMember(
    Index index,
    VarDecl* backingVarDecl)
{
    SLANG_ASSERT(backingVarDecl);
    SLANG_ASSERT(index >= 0 && index <= getDirectMemberDeclCount());

    _invalidateLookupAccelerators();
    _directMemberDecls.decls.insert(index, backingVarDecl);
}

//
//
//

bool isLocalVar(const Decl* decl)
{
    const auto varDecl = as<VarDecl>(decl);
    if (!varDecl)
        return false;
    const Decl* pp = varDecl->parentDecl;
    if (as<ScopeDecl>(pp))
        return true;
    while (auto genericDecl = as<GenericDecl>(pp))
        pp = genericDecl->inner;
    if (as<FunctionDeclBase>(pp))
        return true;

    return false;
}

ThisTypeDecl* InterfaceDecl::getThisTypeDecl()
{
    auto thisTypeDecl = findFirstDirectMemberDeclOfType<ThisTypeDecl>();
    if (!thisTypeDecl)
    {
        SLANG_UNEXPECTED("InterfaceDecl does not have a ThisType decl.");
    }
    return thisTypeDecl;
}

InterfaceDecl* ThisTypeConstraintDecl::getInterfaceDecl()
{
    return as<InterfaceDecl>(parentDecl->parentDecl);
}

void AggTypeDecl::addTag(TypeTag tag)
{
    typeTags = (TypeTag)((int)tag | (int)tag);
}

bool AggTypeDecl::hasTag(TypeTag tag)
{
    return ((int)typeTags & (int)tag) != 0;
}

void AggTypeDecl::unionTagsWith(TypeTag other)
{
    addTag(other);
}

} // namespace Slang
