// slang-ast-substitutions.cpp
#include "slang-ast-builder.h"
#include <assert.h>
#include "slang-syntax.h"
#include "slang-generated-ast-macro.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Substitutions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Substitutions* Substitutions::applySubstitutionsShallow(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Substitutions, applySubstitutionsShallow, (astBuilder, substSet, substOuter, ioDiff))
}

bool Substitutions::equals(Substitutions* subst)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Substitutions, equals, (subst))
}

HashCode Substitutions::getHashCode() const
{
    SLANG_AST_NODE_CONST_VIRTUAL_CALL(Substitutions, getHashCode, ())
}

Substitutions* Substitutions::_applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(substSet);
    SLANG_UNUSED(substOuter);
    SLANG_UNUSED(ioDiff);
    SLANG_UNEXPECTED("Substitutions::_applySubstitutionsShallowOverride not overridden");
    //return Substitutions*();
}

bool Substitutions::_equalsOverride(Substitutions* subst)
{
    SLANG_UNUSED(subst);
    SLANG_UNEXPECTED("Substitutions::_equalsOverride not overridden");
    //return false;
}

HashCode Substitutions::_getHashCodeOverride() const
{
    SLANG_UNEXPECTED("Substitutions::_getHashCodeOverride not overridden");
    //return HashCode(0);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GenericSubstitution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Substitutions* GenericSubstitution::_applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff)
{
    int diff = 0;

    if (substOuter != outer) diff++;

    List<Val*> substArgs;
    for (auto a : args)
    {
        substArgs.add(a->substituteImpl(astBuilder, substSet, &diff));
    }

    if (!diff) return this;

    (*ioDiff)++;

    auto substSubst = astBuilder->getOrCreateGenericSubstitution(genericDecl, substArgs, substOuter);
    return substSubst;
}

bool GenericSubstitution::_equalsOverride(Substitutions* subst)
{
    // both must be NULL, or non-NULL
    if (subst == nullptr)
        return false;
    if (this == subst)
        return true;

    auto genericSubst = as<GenericSubstitution>(subst);
    if (!genericSubst)
        return false;
    if (genericDecl != genericSubst->genericDecl)
        return false;

    Index argCount = args.getCount();
    SLANG_RELEASE_ASSERT(args.getCount() == genericSubst->args.getCount());
    for (Index aa = 0; aa < argCount; ++aa)
    {
        if (!args[aa]->equalsVal(genericSubst->args[aa]))
            return false;
    }

    if (!outer)
        return !genericSubst->outer;

    if (!outer->equals(genericSubst->outer))
        return false;

    return true;
}

HashCode GenericSubstitution::_getHashCodeOverride() const 
{
    HashCode rs = 0;
    for (auto && v : args)
    {
        rs ^= v->getHashCode();
        rs *= 16777619;
    }
    return rs;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ThisTypeSubstitution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Substitutions* ThisTypeSubstitution::_applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff)
{
    int diff = 0;

    if (substOuter != outer) diff++;

    // NOTE: Must use .as because we must have a smart pointer here to keep in scope.
    auto substWitness = as<SubtypeWitness>(witness->substituteImpl(astBuilder, substSet, &diff));

    for (auto subst = substSet.substitutions; subst; subst = subst->outer)
    {
        if (auto otherSubst = as<ThisTypeSubstitution>(subst))
        {
            if (otherSubst->witness->sup->equals(substWitness->sub))
            {
                // We have a `ThisTypeSubstitution` from `substSet` that substitutes the sub type of
                // this `ThisTypeSubst` to a more concrete type. In this case, we want to construct
                // a transitive sub type witness to make our witness more concrete.
                auto transitiveWitness = astBuilder->create<TransitiveSubtypeWitness>();
                transitiveWitness->sub = otherSubst->witness->sub;
                transitiveWitness->sup = substWitness->sup;
                transitiveWitness->subToMid = otherSubst->witness;
                transitiveWitness->midToSup = substWitness;
                substWitness = transitiveWitness;
                diff++;
            }
        }
    }

    if (!diff) return this;

    (*ioDiff)++;
    ThisTypeSubstitution* substSubst;

    substSubst = astBuilder->getOrCreateThisTypeSubstitution(interfaceDecl, substWitness, substOuter);
    return substSubst;
}

bool ThisTypeSubstitution::_equalsOverride(Substitutions* subst)
{
    if (!subst)
        return false;
    if (subst == this)
        return true;

    if (auto thisTypeSubst = as<ThisTypeSubstitution>(subst))
    {
        // For our purposes, two this-type substitutions are
        // equivalent if they have the same type as `This`,
        // even if the specific witness values they use
        // might differ.
        //
        if (this->interfaceDecl != thisTypeSubst->interfaceDecl)
            return false;

        if (!this->witness->sub->equals(thisTypeSubst->witness->sub))
            return false;

        return true;
    }
    return false;
}

HashCode ThisTypeSubstitution::_getHashCodeOverride() const
{
    return witness->sub->getHashCode();
}

} // namespace Slang
