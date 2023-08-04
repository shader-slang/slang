// slang-check-resolve-val.cpp

// Logic for resolving/simplifying Types and DeclRefs.

#include "slang-check-impl.h"

#include "slang-lookup.h"
#include "slang-syntax.h"
#include "slang-ast-synthesis.h"
#include "slang-ast-reflect.h"

namespace Slang
{

Type* Type::createCanonicalType(SemanticsVisitor* semantics)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Type, createCanonicalType, (semantics));
}

Val* Type::_resolveImplOverride(SemanticsVisitor* semantics)
{
    Val* resolvedVal = createCanonicalType(semantics);
    return resolvedVal;
}

DeclRefBase* _resolveAsDeclRef(DeclRefBase* declRefToResolve, SemanticsVisitor* semantics);

Type* DeclRefType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    auto astBuilder = getCurrentASTBuilder();

    // A declaration reference is already canonical
    auto resolvedDeclRef = getDeclRef();
    resolvedDeclRef = _resolveAsDeclRef(getDeclRef().declRefBase, semantics);
    if (auto satisfyingVal = _tryLookupConcreteAssociatedTypeFromThisTypeSubst(astBuilder, resolvedDeclRef))
        return as<Type>(satisfyingVal);
    if (resolvedDeclRef != getDeclRef())
        return DeclRefType::create(astBuilder, resolvedDeclRef);
    return this;
}


Val* SubtypeWitness::_resolveImplOverride(SemanticsVisitor*)
{
    return as<SubtypeWitness>(defaultResolveImpl());
}

}
