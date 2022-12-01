// slang-ast-decl.cpp
#include "slang-ast-builder.h"
#include "slang-syntax.h"
#include <assert.h>

#include "slang-generated-ast-macro.h"

namespace Slang {

const TypeExp& TypeConstraintDecl::getSup() const
{
    SLANG_AST_NODE_CONST_VIRTUAL_CALL(TypeConstraintDecl, getSup, ())
}

const TypeExp& TypeConstraintDecl::_getSupOverride() const
{
    SLANG_UNEXPECTED("TypeConstraintDecl::_getSupOverride not overridden");
    //return TypeExp::empty;
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

void ContainerDecl::buildMemberDictionary()
{
    // Don't rebuild if already built
    if (isMemberDictionaryValid())
        return;

    // If it's < 0 it means that the dictionaries are entirely invalid
    if (dictionaryLastCount < 0)
    {
        dictionaryLastCount = 0;
        memberDictionary.Clear();
        transparentMembers.clear();
    }

    // are we a generic?
    GenericDecl* genericDecl = as<GenericDecl>(this);

    const Index membersCount = members.getCount();

    SLANG_ASSERT(dictionaryLastCount >= 0 && dictionaryLastCount <= membersCount);

    for (Index i = dictionaryLastCount; i < membersCount; ++i)
    {
        Decl* m = members[i];

        auto name = m->getName();

        // Add any transparent members to a separate list for lookup
        if (m->hasModifier<TransparentModifier>())
        {
            TransparentMemberInfo info;
            info.decl = m;
            transparentMembers.add(info);
        }

        // Ignore members with no name
        if (!name)
            continue;

        // Ignore the "inner" member of a generic declaration
        if (genericDecl && m == genericDecl->inner)
            continue;

        m->nextInContainerWithSameName = nullptr;

        Decl* next = nullptr;
        if (memberDictionary.TryGetValue(name, next))
            m->nextInContainerWithSameName = next;

        memberDictionary[name] = m;
    }

    dictionaryLastCount = membersCount;
    SLANG_ASSERT(isMemberDictionaryValid());
}

} // namespace Slang
