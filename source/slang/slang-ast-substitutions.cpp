// slang-ast-substitutions.cpp
#include "slang-ast-builder.h"
#include <assert.h>

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
    auto substSubst = astBuilder->create<GenericSubstitution>();
    substSubst->genericDecl = genericDecl;
    substSubst->args = substArgs;
    substSubst->outer = substOuter;
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

    if (!diff) return this;

    (*ioDiff)++;
    auto substSubst = astBuilder->create<ThisTypeSubstitution>();
    substSubst->interfaceDecl = interfaceDecl;
    substSubst->witness = substWitness;
    substSubst->outer = substOuter;
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
    return witness->getHashCode();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GlobalGenericParamSubstitution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Substitutions* GlobalGenericParamSubstitution::_applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff)
{
    // if we find a GlobalGenericParamSubstitution in subst that references the same type_param decl
    // return a copy of that GlobalGenericParamSubstitution
    int diff = 0;

    if (substOuter != outer) diff++;

    auto substActualType = as<Type>(actualType->substituteImpl(astBuilder, substSet, &diff));

    List<ConstraintArg> substConstraintArgs;
    for (auto constraintArg : constraintArgs)
    {
        ConstraintArg substConstraintArg;
        substConstraintArg.decl = constraintArg.decl;
        substConstraintArg.val = constraintArg.val->substituteImpl(astBuilder, substSet, &diff);

        substConstraintArgs.add(substConstraintArg);
    }

    if (!diff)
        return this;

    (*ioDiff)++;

    GlobalGenericParamSubstitution* substSubst = astBuilder->create<GlobalGenericParamSubstitution>();
    substSubst->paramDecl = paramDecl;
    substSubst->actualType = substActualType;
    substSubst->constraintArgs = substConstraintArgs;
    substSubst->outer = substOuter;
    return substSubst;
}

bool GlobalGenericParamSubstitution::_equalsOverride(Substitutions* subst)
{
    if (!subst)
        return false;
    if (subst == this)
        return true;

    if (auto genSubst = as<GlobalGenericParamSubstitution>(subst))
    {
        if (paramDecl != genSubst->paramDecl)
            return false;
        if (!actualType->equalsVal(genSubst->actualType))
            return false;
        if (constraintArgs.getCount() != genSubst->constraintArgs.getCount())
            return false;
        for (Index i = 0; i < constraintArgs.getCount(); i++)
        {
            if (!constraintArgs[i].val->equalsVal(genSubst->constraintArgs[i].val))
                return false;
        }
        return true;
    }
    return false;
}

HashCode GlobalGenericParamSubstitution::_getHashCodeOverride() const
{
    HashCode rs = actualType->getHashCode();
    for (auto && a : constraintArgs)
    {
        rs = combineHash(rs, a.val->getHashCode());
    }
    return rs;
}


} // namespace Slang
