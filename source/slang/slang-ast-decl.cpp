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
        if (as<InterfaceDefaultImplDecl>(ancestor))
            return false;
        if (as<ExtensionDecl>(ancestor))
            return false;
    }
    return false;
}

//
// ContainerDeclDirectMemberDecls
//

void ContainerDeclDirectMemberDecls::_initForOnDemandDeserialization(
    RefObject* deserializationContext,
    void const* deserializationData,
    Count declCount)
{
    SLANG_ASSERT(deserializationContext);
    SLANG_ASSERT(deserializationData);

    SLANG_ASSERT(!decls.getCount());
    SLANG_ASSERT(!onDemandDeserialization.context);

    onDemandDeserialization.context = deserializationContext;
    onDemandDeserialization.data = deserializationData;

    for (Index i = 0; i < declCount; ++i)
        decls.add(nullptr);
}

void ContainerDeclDirectMemberDecls::_readAllSerializedDecls() const
{
    SLANG_ASSERT(isUsingOnDemandDeserialization());

    // We start by querying each of the contained decls
    // by index, which should cause the entire `decls`
    // array to be filled in.
    //
    auto declCount = getDeclCount();
    for (Index i = 0; i < declCount; ++i)
    {
        auto decl = getDecl(i);
        SLANG_UNUSED(decl);
    }

    // At this point, we have loaded all the information
    // that was in the serialized representation, and
    // don't need to keep doing on-demand loading.
    // Thus, we clear out the pointer to the serialized
    // data (which will cause later calls to
    // `isDoingOnDemandSerialization()` to return `false`).
    //
    // Note that we do *not* clear out the `context` pointer
    // used for on-demand deserialization, because in the
    // case where we are storing the members of a `ModuleDecl`,
    // that context will hold the additional state needed to
    // look up declarations by their mangled names, and we
    // want to retain that state. The
    // `isUsingOnDemandDeserializationForExports()` query
    // is based on the `context` pointer only, so it will
    // continue to return `true`.
    //
    onDemandDeserialization.data = nullptr;

    _invalidateLookupAccelerators();
}

List<Decl*> const& ContainerDeclDirectMemberDecls::getDecls() const
{
    if (isUsingOnDemandDeserialization())
    {
        _readAllSerializedDecls();
    }

    return decls;
}

Count ContainerDeclDirectMemberDecls::getDeclCount() const
{
    // Note: in the case of on-demand deserialization,
    // the number of elements in the `decls` list
    // will be correct, although one or more of the
    // pointers in it might be null.
    //
    return decls.getCount();
}

Decl* ContainerDeclDirectMemberDecls::getDecl(Index index) const
{
    auto decl = decls[index];
    if (!decl && isUsingOnDemandDeserialization())
    {
        decl = _readSerializedDeclAtIndex(index);
        decls[index] = decl;
    }
    return decl;
}

Decl* ContainerDeclDirectMemberDecls::findLastDeclOfName(Name* name) const
{
    if (isUsingOnDemandDeserialization())
    {
        if (auto found = accelerators.mapNameToLastDeclOfThatName.tryGetValue(name))
            return *found;

        Decl* decl = _readSerializedDeclsOfName(name);
        accelerators.mapNameToLastDeclOfThatName.add(name, decl);
        return decl;
    }
    else
    {
        _ensureLookupAcceleratorsAreValid();
        if (auto found = accelerators.mapNameToLastDeclOfThatName.tryGetValue(name))
            return *found;
    }
    return nullptr;
}

Dictionary<Name*, Decl*> ContainerDeclDirectMemberDecls::getMapFromNameToLastDeclOfThatName() const
{
    if (isUsingOnDemandDeserialization())
    {
        // If we have been using on-demand deserialization,
        // then the `mapNameToLastDeclOfThatName` dictionary
        // may not accurately reflect the contained declarations.
        // We need to force all of the declarations to be
        // deserialized immediately, which will also have
        // the effect of invalidating the accelerators so
        // that they can be rebuilt to contain complete information.
        //
        _readAllSerializedDecls();
    }

    _ensureLookupAcceleratorsAreValid();
    return accelerators.mapNameToLastDeclOfThatName;
}


List<Decl*> const& ContainerDeclDirectMemberDecls::getTransparentDecls() const
{
    if (isUsingOnDemandDeserialization())
    {
        if (accelerators.filteredListOfTransparentDecls.getCount() == 0)
        {
            _readSerializedTransparentDecls();
        }
    }
    else
    {
        _ensureLookupAcceleratorsAreValid();
    }
    return accelerators.filteredListOfTransparentDecls;
}

bool ContainerDeclDirectMemberDecls::isUsingOnDemandDeserialization() const
{
    return onDemandDeserialization.data != nullptr;
}

bool ContainerDeclDirectMemberDecls::_areLookupAcceleratorsValid() const
{
    return accelerators.declCountWhenLastUpdated == decls.getCount();
}

void ContainerDeclDirectMemberDecls::_invalidateLookupAccelerators() const
{
    accelerators.declCountWhenLastUpdated = -1;
}

void ContainerDeclDirectMemberDecls::_ensureLookupAcceleratorsAreValid() const
{
    if (_areLookupAcceleratorsValid())
        return;

    // If the `declCountWhenLastUpdated` is less than zero, it means that
    // the accelerators are entirely invalidated, and must be rebuilt
    // from scratch.
    //
    if (accelerators.declCountWhenLastUpdated < 0)
    {
        accelerators.declCountWhenLastUpdated = 0;
        accelerators.mapNameToLastDeclOfThatName.clear();
        accelerators.filteredListOfTransparentDecls.clear();
    }

    Count memberCount = decls.getCount();
    Count memberCountWhenLastUpdated = accelerators.declCountWhenLastUpdated;

    SLANG_ASSERT(memberCountWhenLastUpdated >= 0 && memberCountWhenLastUpdated <= memberCount);

    // are we a generic?
    GenericDecl* genericDecl = nullptr;
    if (memberCount > 0)
    {
        genericDecl = as<GenericDecl>(decls[0]->parentDecl);
    }

    for (Index i = memberCountWhenLastUpdated; i < memberCount; ++i)
    {
        Decl* memberDecl = decls[i];

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
            accelerators.filteredListOfTransparentDecls.add(memberDecl);
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
        accelerators.mapNameToLastDeclOfThatName.tryGetValue(memberName, prevMemberWithSameName);
        memberDecl->_prevInContainerWithSameName = prevMemberWithSameName;

        // Whether or not there was a previous declaration with this
        // name, the current `memberDecl` is the last member declaration
        // with that name encountered so far, and it is what we will
        // store in the lookup dictionary.
        //
        accelerators.mapNameToLastDeclOfThatName[memberName] = memberDecl;
    }

    accelerators.declCountWhenLastUpdated = memberCount;
    SLANG_ASSERT(_areLookupAcceleratorsValid());
}


//
// ContainerDecl
//

List<Decl*> const& ContainerDecl::getDirectMemberDecls()
{
    return _directMemberDecls.getDecls();
}

Count ContainerDecl::getDirectMemberDeclCount()
{
    return _directMemberDecls.getDeclCount();
}

Decl* ContainerDecl::getDirectMemberDecl(Index index)
{
    return _directMemberDecls.getDecl(index);
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
    return _directMemberDecls.findLastDeclOfName(name);
}

Decl* ContainerDecl::getPrevDirectMemberDeclWithSameName(Decl* decl)
{
    SLANG_ASSERT(decl);
    SLANG_ASSERT(decl->parentDecl == this);

    if (isUsingOnDemandDeserializationForDirectMembers())
    {
        // Note: in the case of on-demand deserialization,
        // we trust that the caller has previously
        // invoked `findLastDirectMemberDeclOfName()`
        // in order to get `decl` (or an earlier
        // entry in the same linked list), so that
        // the list threaded through the declarations
        // of the same name is already set up.
        //
        // If that is ever *not* the case, then this
        // query would end up returning the wrong results.

        return decl->_prevInContainerWithSameName;
    }
    else
    {
        _ensureLookupAcceleratorsAreValid();
        return decl->_prevInContainerWithSameName;
    }
}

void ContainerDecl::addDirectMemberDecl(Decl* decl)
{
    if (isUsingOnDemandDeserializationForDirectMembers())
    {
        SLANG_UNEXPECTED("this operation shouldn't be performed on deserialized declarations");
    }

    if (!decl)
        return;

    decl->parentDecl = this;
    _directMemberDecls.decls.add(decl);
}

List<Decl*> const& ContainerDecl::getTransparentDirectMemberDecls()
{
    return _directMemberDecls.getTransparentDecls();
}

bool ContainerDecl::isUsingOnDemandDeserializationForDirectMembers()
{
    return _directMemberDecls.isUsingOnDemandDeserialization();
}

bool ModuleDecl::isUsingOnDemandDeserializationForExports()
{
    return _directMemberDecls.onDemandDeserialization.context != nullptr;
}

bool ContainerDecl::_areLookupAcceleratorsValid()
{
    return _directMemberDecls._areLookupAcceleratorsValid();
}

void ContainerDecl::_invalidateLookupAccelerators()
{
    _directMemberDecls._invalidateLookupAccelerators();
}

void ContainerDecl::_ensureLookupAcceleratorsAreValid()
{
    _directMemberDecls._ensureLookupAcceleratorsAreValid();
}

void ContainerDecl::
    _removeDirectMemberConstructorDeclBecauseSynthesizedAnotherDefaultConstructorInstead(
        ConstructorDecl* decl)
{
    if (isUsingOnDemandDeserializationForDirectMembers())
    {
        SLANG_UNEXPECTED("this operation shouldn't be performed on deserialized declarations");
    }

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
    if (isUsingOnDemandDeserializationForDirectMembers())
    {
        SLANG_UNEXPECTED("this operation shouldn't be performed on deserialized declarations");
    }

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
    if (isUsingOnDemandDeserializationForDirectMembers())
    {
        SLANG_UNEXPECTED("this operation shouldn't be performed on deserialized declarations");
    }

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
    typeTags = (TypeTag)((int)typeTags | (int)tag);
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
