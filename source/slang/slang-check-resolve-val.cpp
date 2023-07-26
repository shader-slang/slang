// slang-check-resolve-val.cpp

// Logic for resolving/simplifying Types and DeclRefs.

#include "slang-check-impl.h"

#include "slang-lookup.h"
#include "slang-syntax.h"
#include "slang-ast-synthesis.h"
#include "slang-ast-reflect.h"

namespace Slang
{

Substitutions* SemanticsVisitor::resolveSubstDeprecated(Substitutions* subst)
{
    if (!subst)
        return nullptr;
    if (auto genericSubst = as<GenericSubstitutionDeprecated>(subst))
    {
        List<Val*> newArgs;
        for (auto arg : genericSubst->getArgs())
            newArgs.add(resolveVal(arg));
        auto outerSubst = resolveSubstDeprecated(subst->getOuter());
        return subst->getASTBuilder()->getOrCreateGenericSubstitution(outerSubst, genericSubst->getGenericDecl(), newArgs);
    }
    else if (auto thisSubst = as<ThisTypeSubstitution>(subst))
    {
        auto witness = as<SubtypeWitness>(resolveVal(thisSubst->witness));
        auto outerSubst = resolveSubstDeprecated(subst->getOuter());
        return subst->getASTBuilder()->getOrCreateThisTypeSubstitution(thisSubst->interfaceDecl, witness, outerSubst);
    }
    SLANG_UNREACHABLE("unhandled case of resolveSubst");
}

Val* DeclRefBase::_resolveOverride(SemanticsVisitor* semantics)
{
    if (m_astBuilder->getEpoch() == m_resolvedValEpoch)
        return m_resolvedVal;

    auto resolvedSubst = semantics ? semantics->resolveSubstDeprecated(substitutions) : substitutions;
    m_resolvedVal = m_astBuilder->getSpecializedDeclRef(decl, resolvedSubst);
    if (semantics)
        m_resolvedValEpoch = m_astBuilder->getEpoch();
    return m_resolvedVal;
}

Type* Type::createCanonicalType(SemanticsVisitor* semantics)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Type, createCanonicalType, (semantics));
}

Val* Type::_resolveOverride(SemanticsVisitor* semantics)
{
    auto val = const_cast<Type*>(this);

    if (val->m_resolvedValEpoch == m_astBuilder->getEpoch())
        return val->m_resolvedVal;

    Val* resolvedVal = createCanonicalType(semantics);
    val->m_resolvedVal = resolvedVal;

    if (semantics)
        val->m_resolvedValEpoch = m_astBuilder->getEpoch();

    return resolvedVal;
}

Type* DeclRefType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    // A declaration reference is already canonical
    declRef.substitute(m_astBuilder, this);
    auto resolvedDeclRef = declRef;
    static int cc = 0;
    cc++;
    if (semantics)
        resolvedDeclRef = as<DeclRefBase>(declRef.declRefBase->resolve(semantics));
    if (resolvedDeclRef.declRefBase->getSubst() && (UInt)(resolvedDeclRef.declRefBase->getSubst()->astNodeType) >= UInt(ASTNodeType::CountOf))
        printf("break");
    if (auto satisfyingVal = _tryLookupConcreteAssociatedTypeFromThisTypeSubst(m_astBuilder, resolvedDeclRef))
        return as<Type>(satisfyingVal);
    if (resolvedDeclRef != declRef)
        return DeclRefType::create(m_astBuilder, resolvedDeclRef);
    return this;
}


Val* SubtypeWitness::_resolveOverride(SemanticsVisitor* semantics)
{
    auto subtypeWitness = this;

    if (subtypeWitness->m_resolvedValEpoch == m_astBuilder->getEpoch())
        return subtypeWitness->m_resolvedVal;

    subtypeWitness->m_resolvedVal = this;
    if (semantics)
    {
        auto subType = as<Type>(subtypeWitness->sub->resolve(semantics));
        auto supType = as<Type>(subtypeWitness->sup->resolve(semantics));
        if (subType && supType)
        {
            if (subType != subtypeWitness->sub || supType != subtypeWitness->sup)
            {
                auto newVal = semantics->tryGetSubtypeWitness(as<Type>(subType), as<Type>(supType));
                if (newVal)
                    subtypeWitness->m_resolvedVal = newVal;
            }
        }
        subtypeWitness->m_resolvedValEpoch = m_astBuilder->getEpoch();
    }
    return subtypeWitness->m_resolvedVal;
}

}
