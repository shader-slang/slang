// slang-ast-type.cpp
#include "slang-ast-builder.h"
#include <assert.h>
#include <typeinfo>

#include "slang-generated-ast-macro.h"

#include "slang-syntax.h"

namespace Slang {

Val* Val::substitute(ASTBuilder* astBuilder, SubstitutionSet subst)
{
    if (!subst) return this;
    int diff = 0;
    return substituteImpl(astBuilder, subst, &diff);
}

Val* Val::substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Val, substituteImpl, (astBuilder, subst, ioDiff))
}

bool Val::equalsVal(Val* val)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Val, equalsVal, (val))
}

String Val::toString()
{
    SLANG_AST_NODE_VIRTUAL_CALL(Val, toString, ())
}

HashCode Val::getHashCode()
{
    SLANG_AST_NODE_VIRTUAL_CALL(Val, getHashCode, ())
}

Val* Val::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    // Default behavior is to not substitute at all
    return this;
}

bool Val::_equalsValOverride(Val* val)
{
    SLANG_UNUSED(val);
    SLANG_UNEXPECTED("Val::_equalsValOverride not overridden");
    //return false;
}

String Val::_toStringOverride()
{
    SLANG_UNEXPECTED("Val::_toStringOverride not overridden");
    //return String();
}

HashCode Val::_getHashCodeOverride()
{
    SLANG_UNEXPECTED("Val::_getHashCodeOverride not overridden");
    //return HashCode(0);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ConstantIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool ConstantIntVal::_equalsValOverride(Val* val)
{
    if (auto intVal = as<ConstantIntVal>(val))
        return value == intVal->value;
    return false;
}

String ConstantIntVal::_toStringOverride()
{
    return String(value);
}

HashCode ConstantIntVal::_getHashCodeOverride()
{
    return (HashCode)value;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GenericParamIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool GenericParamIntVal::_equalsValOverride(Val* val)
{
    if (auto genericParamVal = as<GenericParamIntVal>(val))
    {
        return declRef.equals(genericParamVal->declRef);
    }
    return false;
}

String GenericParamIntVal::_toStringOverride()
{
    return getText(declRef.getName());
}

HashCode GenericParamIntVal::_getHashCodeOverride()
{
    return declRef.getHashCode() ^ HashCode(0xFFFF);
}

Val* GenericParamIntVal::_substituteImplOverride(ASTBuilder* /* astBuilder */, SubstitutionSet subst, int* ioDiff)
{
    // search for a substitution that might apply to us
    for (auto s = subst.substitutions; s; s = s->outer)
    {
        auto genSubst = as<GenericSubstitution>(s);
        if (!genSubst)
            continue;

        // the generic decl associated with the substitution list must be
        // the generic decl that declared this parameter
        auto genericDecl = genSubst->genericDecl;
        if (genericDecl != declRef.getDecl()->parentDecl)
            continue;

        int index = 0;
        for (auto m : genericDecl->members)
        {
            if (m == declRef.getDecl())
            {
                // We've found it, so return the corresponding specialization argument
                (*ioDiff)++;
                return genSubst->args[index];
            }
            else if (auto typeParam = as<GenericTypeParamDecl>(m))
            {
                index++;
            }
            else if (auto valParam = as<GenericValueParamDecl>(m))
            {
                index++;
            }
            else
            {
            }
        }
    }

    // Nothing found: don't substitute.
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ErrorIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool ErrorIntVal::_equalsValOverride(Val* val)
{
    if (auto errorIntVal = as<ErrorIntVal>(val))
    {
        return true;
    }
    return false;
}

String ErrorIntVal::_toStringOverride()
{
    return "<error>";
}

HashCode ErrorIntVal::_getHashCodeOverride()
{
    return HashCode(typeid(this).hash_code());
}

Val* ErrorIntVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ErrorIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// TODO: should really have a `type.cpp` and a `witness.cpp`

bool TypeEqualityWitness::_equalsValOverride(Val* val)
{
    auto otherWitness = as<TypeEqualityWitness>(val);
    if (!otherWitness)
        return false;
    return sub->equals(otherWitness->sub);
}

Val* TypeEqualityWitness::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int * ioDiff)
{
    TypeEqualityWitness* rs = astBuilder->create<TypeEqualityWitness>();
    rs->sub = as<Type>(sub->substituteImpl(astBuilder, subst, ioDiff));
    rs->sup = as<Type>(sup->substituteImpl(astBuilder, subst, ioDiff));
    return rs;
}

String TypeEqualityWitness::_toStringOverride()
{
    return "TypeEqualityWitness(" + sub->toString() + ")";
}

HashCode TypeEqualityWitness::_getHashCodeOverride()
{
    return sub->getHashCode();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclaredSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool DeclaredSubtypeWitness::_equalsValOverride(Val* val)
{
    auto otherWitness = as<DeclaredSubtypeWitness>(val);
    if (!otherWitness)
        return false;

    return sub->equals(otherWitness->sub)
        && sup->equals(otherWitness->sup)
        && declRef.equals(otherWitness->declRef);
}

Val* DeclaredSubtypeWitness::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int * ioDiff)
{
    if (auto genConstraintDeclRef = declRef.as<GenericTypeConstraintDecl>())
    {
        auto genConstraintDecl = genConstraintDeclRef.getDecl();

        // search for a substitution that might apply to us
        for (auto s = subst.substitutions; s; s = s->outer)
        {
            if (auto genericSubst = as<GenericSubstitution>(s))
            {
                // the generic decl associated with the substitution list must be
                // the generic decl that declared this parameter
                auto genericDecl = genericSubst->genericDecl;
                if (genericDecl != genConstraintDecl->parentDecl)
                    continue;

                bool found = false;
                Index index = 0;
                for (auto m : genericDecl->members)
                {
                    if (auto constraintParam = as<GenericTypeConstraintDecl>(m))
                    {
                        if (constraintParam == declRef.getDecl())
                        {
                            found = true;
                            break;
                        }
                        index++;
                    }
                }
                if (found)
                {
                    (*ioDiff)++;
                    auto ordinaryParamCount = genericDecl->getMembersOfType<GenericTypeParamDecl>().getCount() +
                        genericDecl->getMembersOfType<GenericValueParamDecl>().getCount();
                    SLANG_ASSERT(index + ordinaryParamCount < genericSubst->args.getCount());
                    return genericSubst->args[index + ordinaryParamCount];
                }
            }
            else if (auto globalGenericSubst = as<GlobalGenericParamSubstitution>(s))
            {
                // check if the substitution is really about this global generic type parameter
                if (globalGenericSubst->paramDecl != genConstraintDecl->parentDecl)
                    continue;

                for (auto constraintArg : globalGenericSubst->constraintArgs)
                {
                    if (constraintArg.decl != genConstraintDecl)
                        continue;

                    (*ioDiff)++;
                    return constraintArg.val;
                }
            }
        }
    }

    // Perform substitution on the constituent elements.
    int diff = 0;
    auto substSub = as<Type>(sub->substituteImpl(astBuilder, subst, &diff));
    auto substSup = as<Type>(sup->substituteImpl(astBuilder, subst, &diff));
    auto substDeclRef = declRef.substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;

    (*ioDiff)++;

    // If we have a reference to a type constraint for an
    // associated type declaration, then we can replace it
    // with the concrete conformance witness for a concrete
    // type implementing the outer interface.
    //
    // TODO: It is a bit gross that we use `GenericTypeConstraintDecl` for
    // associated types, when they aren't really generic type *parameters*,
    // so we'll need to change this location in the code if we ever clean
    // up the hierarchy.
    //
    if (auto substTypeConstraintDecl = as<GenericTypeConstraintDecl>(substDeclRef.decl))
    {
        if (auto substAssocTypeDecl = as<AssocTypeDecl>(substTypeConstraintDecl->parentDecl))
        {
            if (auto interfaceDecl = as<InterfaceDecl>(substAssocTypeDecl->parentDecl))
            {
                // At this point we have a constraint decl for an associated type,
                // and we nee to see if we are dealing with a concrete substitution
                // for the interface around that associated type.
                if (auto thisTypeSubst = findThisTypeSubstitution(substDeclRef.substitutions, interfaceDecl))
                {
                    // We need to look up the declaration that satisfies
                    // the requirement named by the associated type.
                    Decl* requirementKey = substTypeConstraintDecl;
                    RequirementWitness requirementWitness = tryLookUpRequirementWitness(astBuilder, thisTypeSubst->witness, requirementKey);
                    switch (requirementWitness.getFlavor())
                    {
                        default:
                            break;

                        case RequirementWitness::Flavor::val:
                        {
                            auto satisfyingVal = requirementWitness.getVal();
                            return satisfyingVal;
                        }
                    }
                }
            }
        }
    }

    DeclaredSubtypeWitness* rs = astBuilder->create<DeclaredSubtypeWitness>();
    rs->sub = substSub;
    rs->sup = substSup;
    rs->declRef = substDeclRef;
    return rs;
}

String DeclaredSubtypeWitness::_toStringOverride()
{
    StringBuilder sb;
    sb << "DeclaredSubtypeWitness(";
    sb << this->sub->toString();
    sb << ", ";
    sb << this->sup->toString();
    sb << ", ";
    sb << this->declRef.toString();
    sb << ")";
    return sb.ProduceString();
}

HashCode DeclaredSubtypeWitness::_getHashCodeOverride()
{
    return declRef.getHashCode();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TransitiveSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool TransitiveSubtypeWitness::_equalsValOverride(Val* val)
{
    auto otherWitness = as<TransitiveSubtypeWitness>(val);
    if (!otherWitness)
        return false;

    return sub->equals(otherWitness->sub)
        && sup->equals(otherWitness->sup)
        && subToMid->equalsVal(otherWitness->subToMid)
        && midToSup->equalsVal(otherWitness->midToSup);
}

Val* TransitiveSubtypeWitness::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int * ioDiff)
{
    int diff = 0;

    Type* substSub = as<Type>(sub->substituteImpl(astBuilder, subst, &diff));
    Type* substSup = as<Type>(sup->substituteImpl(astBuilder, subst, &diff));
    SubtypeWitness* substSubToMid = as<SubtypeWitness>(subToMid->substituteImpl(astBuilder, subst, &diff));
    SubtypeWitness* substMidToSup = as<SubtypeWitness>(midToSup->substituteImpl(astBuilder, subst, &diff));

    // If nothing changed, then we can bail out early.
    if (!diff)
        return this;

    // Something changes, so let the caller know.
    (*ioDiff)++;

    // TODO: are there cases where we can simplify?
    //
    // In principle, if either `subToMid` or `midToSub` turns into
    // a reflexive subtype witness, then we could drop that side,
    // and just return the other one (this would imply that `sub == mid`
    // or `mid == sup` after substitutions).
    //
    // In the long run, is it also possible that if `sub` gets resolved
    // to a concrete type *and* we decide to flatten out the inheritance
    // graph into a linearized "class precedence list" stored in any
    // aggregate type, then we could potentially just redirect to point
    // to the appropriate inheritance decl in the original type.
    //
    // For now I'm going to ignore those possibilities and hope for the best.

    // In the simple case, we just construct a new transitive subtype
    // witness, and we move on with life.
    TransitiveSubtypeWitness* result = astBuilder->create<TransitiveSubtypeWitness>();
    result->sub = substSub;
    result->sup = substSup;
    result->subToMid = substSubToMid;
    result->midToSup = substMidToSup;
    return result;
}

String TransitiveSubtypeWitness::_toStringOverride()
{
    // Note: we only print the constituent
    // witnesses, and rely on them to print
    // the starting and ending types.
    StringBuilder sb;
    sb << "TransitiveSubtypeWitness(";
    sb << this->subToMid->toString();
    sb << ", ";
    sb << this->midToSup->toString();
    sb << ")";
    return sb.ProduceString();
}

HashCode TransitiveSubtypeWitness::_getHashCodeOverride()
{
    auto hash = sub->getHashCode();
    hash = combineHash(hash, sup->getHashCode());
    hash = combineHash(hash, subToMid->getHashCode());
    hash = combineHash(hash, midToSup->getHashCode());
    return hash;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExtractExistentialSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool ExtractExistentialSubtypeWitness::_equalsValOverride(Val* val)
{
    if (auto extractWitness = as<ExtractExistentialSubtypeWitness>(val))
    {
        return declRef.equals(extractWitness->declRef);
    }
    return false;
}

String ExtractExistentialSubtypeWitness::_toStringOverride()
{
    String result;
    result.append("extractExistentialValue(");
    result.append(declRef.toString());
    result.append(")");
    return result;
}

HashCode ExtractExistentialSubtypeWitness::_getHashCodeOverride()
{
    return declRef.getHashCode();
}

Val* ExtractExistentialSubtypeWitness::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substDeclRef = declRef.substituteImpl(astBuilder, subst, &diff);
    auto substSub = as<Type>(sub->substituteImpl(astBuilder, subst, &diff));
    auto substSup = as<Type>(sup->substituteImpl(astBuilder, subst, &diff));

    if (!diff)
        return this;

    (*ioDiff)++;

    ExtractExistentialSubtypeWitness* substValue = astBuilder->create<ExtractExistentialSubtypeWitness>();
    substValue->declRef = declRef;
    substValue->sub = substSub;
    substValue->sup = substSup;
    return substValue;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TaggedUnionSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool TaggedUnionSubtypeWitness::_equalsValOverride(Val* val)
{
    auto taggedUnionWitness = as<TaggedUnionSubtypeWitness>(val);
    if (!taggedUnionWitness)
        return false;

    auto caseCount = caseWitnesses.getCount();
    if (caseCount != taggedUnionWitness->caseWitnesses.getCount())
        return false;

    for (Index ii = 0; ii < caseCount; ++ii)
    {
        if (!caseWitnesses[ii]->equalsVal(taggedUnionWitness->caseWitnesses[ii]))
            return false;
    }

    return true;
}

String TaggedUnionSubtypeWitness::_toStringOverride()
{
    String result;
    result.append("TaggedUnionSubtypeWitness(");
    bool first = true;
    for (auto caseWitness : caseWitnesses)
    {
        if (!first) result.append(", ");
        first = false;

        result.append(caseWitness->toString());
    }
    return result;
}

HashCode TaggedUnionSubtypeWitness::_getHashCodeOverride()
{
    HashCode hash = 0;
    for (auto caseWitness : caseWitnesses)
    {
        hash = combineHash(hash, caseWitness->getHashCode());
    }
    return hash;
}

Val* TaggedUnionSubtypeWitness::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substSub = as<Type>(sub->substituteImpl(astBuilder, subst, &diff));
    auto substSup = as<Type>(sup->substituteImpl(astBuilder, subst, &diff));

    List<Val*> substCaseWitnesses;
    for (auto caseWitness : caseWitnesses)
    {
        substCaseWitnesses.add(caseWitness->substituteImpl(astBuilder, subst, &diff));
    }

    if (!diff)
        return this;

    (*ioDiff)++;

    TaggedUnionSubtypeWitness* substWitness = astBuilder->create<TaggedUnionSubtypeWitness>();
    substWitness->sub = substSub;
    substWitness->sup = substSup;
    substWitness->caseWitnesses.swapWith(substCaseWitnesses);
    return substWitness;
}



} // namespace Slang
