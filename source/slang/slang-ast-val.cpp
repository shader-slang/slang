// slang-ast-type.cpp
#include "slang-ast-builder.h"
#include <assert.h>
#include <typeinfo>

#include "slang-generated-ast-macro.h"
#include "slang-diagnostics.h"
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

void Val::toText(StringBuilder& out)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Val, toText, (out))
}

String Val::toString()
{
    StringBuilder builder;
    toText(builder);
    return builder;
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

void Val::_toTextOverride(StringBuilder& out)
{
    SLANG_UNUSED(out);
    SLANG_UNEXPECTED("Val::_toStringOverride not overridden");
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

void ConstantIntVal::_toTextOverride(StringBuilder& out)
{
    out << value;
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

void GenericParamIntVal::_toTextOverride(StringBuilder& out)
{
    Name* name = declRef.getName();
    if (name)
    {
        out << name->text;
    }
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

void ErrorIntVal::_toTextOverride(StringBuilder& out)
{
    out << toSlice("<error>");
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

void TypeEqualityWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("TypeEqualityWitness(") << sub << toSlice(")");
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

void DeclaredSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("DeclaredSubtypeWitness(") << sub << toSlice(", ") << sup << toSlice(", ") << declRef << toSlice(")");
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

void TransitiveSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    // Note: we only print the constituent
    // witnesses, and rely on them to print
    // the starting and ending types.
    
    out << toSlice("TransitiveSubtypeWitness(") << subToMid << toSlice(", ") << midToSup << toSlice(")");
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

void ExtractExistentialSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("extractExistentialValue(") << declRef << toSlice(")");
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

void TaggedUnionSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("TaggedUnionSubtypeWitness(");
    bool first = true;
    for (auto caseWitness : caseWitnesses)
    {
        if (!first)
        {
            out << toSlice(", ");
        }
        first = false;

        out << caseWitness;
    }
    out << toSlice(")");
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

// ModifierVal

bool ModifierVal::_equalsValOverride(Val* val)
{
    // TODO: This is assuming we can fully deduplicate the values that represent
    // modifiers, which may not actually be the case if there are multiple modules
    // being combined that use different `ASTBuilder`s.
    //
    return this == val;
}

HashCode ModifierVal::_getHashCodeOverride()
{
    Hasher hasher;
    hasher.hashValue((void*) this);
    return hasher.getResult();
}

// UNormModifierVal

void UNormModifierVal::_toTextOverride(StringBuilder& out)
{
    out.append("unorm");
}

Val* UNormModifierVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

// SNormModifierVal

void SNormModifierVal::_toTextOverride(StringBuilder& out)
{
    out.append("snorm");
}

Val* SNormModifierVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

// PolynomialIntVal

bool PolynomialIntVal::_equalsValOverride(Val* val)
{
    if (auto genericParamVal = as<GenericParamIntVal>(val))
    {
        return constantTerm == 0 && terms.getCount() == 1 &&
               terms[0]->paramFactors.getCount() == 1 && terms[0]->constFactor == 1 &&
               terms[0]->paramFactors[0]->param->equalsVal(genericParamVal) &&
               terms[0]->paramFactors[0]->power == 1;
    }
    else if (auto otherPolynomial = as<PolynomialIntVal>(val))
    {
        if (constantTerm != otherPolynomial->constantTerm)
            return false;
        if (terms.getCount() != otherPolynomial->terms.getCount())
            return false;
        for (Index i = 0; i < terms.getCount(); i++)
        {
            auto& thisTerm = *(terms[i]);
            auto& thatTerm = *(otherPolynomial->terms[i]);
            if (thisTerm.constFactor != thatTerm.constFactor)
                return false;
            if (thisTerm.paramFactors.getCount() != thatTerm.paramFactors.getCount())
                return false;
            for (Index j = 0; j < thisTerm.paramFactors.getCount(); j++)
            {
                if (thisTerm.paramFactors[j]->power != thatTerm.paramFactors[j]->power)
                    return false;
                if (!thisTerm.paramFactors[j]->param->equalsVal(thatTerm.paramFactors[j]->param))
                    return false;
            }
        }
        return true;
    }
    return false;
}

void PolynomialIntVal::_toTextOverride(StringBuilder& out)
{
    for (Index i = 0; i < terms.getCount(); i++)
    {
        auto& term = *(terms[i]);
        if (term.constFactor > 0)
        {
            if (i > 0)
                out << "+";
        }
        else
            out << "-";
        bool isFirstFactor = true;
        if (abs(term.constFactor) != 1 || term.paramFactors.getCount() == 0)
        {
            out << abs(term.constFactor);
            isFirstFactor = false;
        }
        for (Index j = 0; j < term.paramFactors.getCount(); j++)
        {
            auto factor = term.paramFactors[j];
            if (isFirstFactor)
            {
                isFirstFactor = false;
            }
            else
            {
                out << "*";
            }
            factor->param->toText(out);
            if (factor->power != 1)
            {
                out << "^^" << factor->power;
            }
        }
    }
    if (constantTerm > 0)
    {
        if (terms.getCount() > 0)
        {
            out << "+";
        }
        out << constantTerm;
    }
    else if (constantTerm < 0)
    {
        out << constantTerm;
    }
}

HashCode PolynomialIntVal::_getHashCodeOverride()
{
    HashCode result = (HashCode)constantTerm;
    for (auto& term : terms)
    {
        if (!term) continue;
        result = combineHash(result, (HashCode)term->constFactor);
        for (auto& factor : term->paramFactors)
        {
            result = combineHash(result, factor->param->getHashCode());
            result = combineHash(result, (HashCode)factor->power);
        }
    }
    return result;
}

Val* PolynomialIntVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    IntegerLiteralValue evaluatedConstantTerm = constantTerm;
    List<PolynomialIntValTerm*> evaluatedTerms;
    for (auto& term : terms)
    {
        IntegerLiteralValue evaluatedTermConstFactor;
        List<PolynomialIntValFactor*> evaluatedTermParamFactors;
        evaluatedTermConstFactor = term->constFactor;
        for (auto& factor : term->paramFactors)
        {
            auto substResult = factor->param->substituteImpl(astBuilder, subst, &diff);
            
            if (auto constantVal = as<ConstantIntVal>(substResult))
            {
                evaluatedTermConstFactor *= constantVal->value;
            }
            else if (auto intResult = as<IntVal>(substResult))
            {
                auto newFactor = astBuilder->create<PolynomialIntValFactor>();
                newFactor->param = intResult;
                newFactor->power = factor->power;
                evaluatedTermParamFactors.add(newFactor);
            }
        }
        if (evaluatedTermParamFactors.getCount() == 0)
        {
            evaluatedConstantTerm += evaluatedTermConstFactor;
        }
        else
        {
            auto newTerm = astBuilder->create<PolynomialIntValTerm>();
            newTerm->paramFactors = _Move(evaluatedTermParamFactors);
            newTerm->constFactor = evaluatedTermConstFactor;
            evaluatedTerms.add(newTerm);
        }
    }

    *ioDiff += diff;

    if (evaluatedTerms.getCount() == 0)
        return astBuilder->create<ConstantIntVal>(evaluatedConstantTerm);
    if (diff != 0)
    {
        auto newPolynomial = astBuilder->create<PolynomialIntVal>();
        newPolynomial->constantTerm = evaluatedConstantTerm;
        newPolynomial->terms = _Move(evaluatedTerms);
        return newPolynomial->canonicalize(astBuilder);
    }
    return this;
}


// compute val += opreand*multiplier;
bool addToPolynomialTerm(ASTBuilder* astBuilder, PolynomialIntVal* val, IntVal* operand, IntegerLiteralValue multiplier)
{
    if (auto c = as<ConstantIntVal>(operand))
    {
        val->constantTerm += c->value * multiplier;
        return true;
    }
    else if (auto poly = as<PolynomialIntVal>(operand))
    {
        val->constantTerm += poly->constantTerm * multiplier;
        for (auto term : poly->terms)
        {
            auto newTerm = astBuilder->create<PolynomialIntValTerm>();
            newTerm->constFactor = multiplier * term->constFactor;
            newTerm->paramFactors = term->paramFactors;
            val->terms.add(newTerm);
        }
        return true;
    }
    else if (auto genVal = as<IntVal>(operand))
    {
        auto term = astBuilder->create<PolynomialIntValTerm>();
        term->constFactor = multiplier;
        auto factor = astBuilder->create<PolynomialIntValFactor>();
        factor->power = 1;
        factor->param = genVal;
        term->paramFactors.add(factor);
        val->terms.add(term);
        return true;
    }
    return false;
}

PolynomialIntVal* PolynomialIntVal::neg(ASTBuilder* astBuilder, IntVal* base)
{
    auto result = astBuilder->create<PolynomialIntVal>();
    if (!addToPolynomialTerm(astBuilder, result, base, -1))
        return nullptr;
    result->canonicalize(astBuilder);
    return result;
}

PolynomialIntVal* PolynomialIntVal::sub(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1)
{
    auto result = astBuilder->create<PolynomialIntVal>();
    if (!addToPolynomialTerm(astBuilder, result, op0, 1))
        return nullptr;
    if (!addToPolynomialTerm(astBuilder, result, op1, -1))
        return nullptr;
    result->canonicalize(astBuilder);
    return result;
}

PolynomialIntVal* PolynomialIntVal::add(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1)
{
    auto result = astBuilder->create<PolynomialIntVal>();
    if (!addToPolynomialTerm(astBuilder, result, op0, 1))
        return nullptr;
    if (!addToPolynomialTerm(astBuilder, result, op1, 1))
        return nullptr;
    result->canonicalize(astBuilder);
    return result;
}

PolynomialIntVal* PolynomialIntVal::mul(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1)
{
    if (auto poly0 = as<PolynomialIntVal>(op0))
    {
        if (auto poly1 = as<PolynomialIntVal>(op1))
        {
            auto result = astBuilder->create<PolynomialIntVal>();
            // add poly0.constant * poly1.constant
            result->constantTerm = poly0->constantTerm * poly1->constantTerm;
            // add poly0.constant * poly1.terms
            if (poly0->constantTerm != 0)
            {
                for (auto term : poly1->terms)
                {
                    auto newTerm = astBuilder->create<PolynomialIntValTerm>();
                    newTerm->constFactor = poly0->constantTerm * term->constFactor;
                    newTerm->paramFactors.addRange(term->paramFactors);
                    result->terms.add(newTerm);
                }
            }
            // add poly1.constant * poly0.terms
            if (poly1->constantTerm != 0)
            {
                for (auto term : poly0->terms)
                {
                    auto newTerm = astBuilder->create<PolynomialIntValTerm>();
                    newTerm->constFactor = poly1->constantTerm * term->constFactor;
                    newTerm->paramFactors.addRange(term->paramFactors);
                    result->terms.add(newTerm);
                }
            }
            // add poly1.terms * poly0.terms
            for (auto term0 : poly0->terms)
            {
                for (auto term1 : poly1->terms)
                {
                    auto newTerm = astBuilder->create<PolynomialIntValTerm>();
                    newTerm->constFactor = term0->constFactor * term1->constFactor;
                    newTerm->paramFactors.addRange(term0->paramFactors);
                    newTerm->paramFactors.addRange(term1->paramFactors);
                    result->terms.add(newTerm);
                }
            }
            result->canonicalize(astBuilder);
            return result;
        }
        else if (auto cVal1 = as<ConstantIntVal>(op1))
        {
            auto result = astBuilder->create<PolynomialIntVal>();
            result->constantTerm = poly0->constantTerm * cVal1->value;
            auto factor1 = astBuilder->create<PolynomialIntValFactor>();
            for (auto term : poly0->terms)
            {
                auto newTerm = astBuilder->create<PolynomialIntValTerm>();
                newTerm->constFactor = term->constFactor * cVal1->value;
                newTerm->paramFactors.addRange(term->paramFactors);
                newTerm->paramFactors.add(factor1);
                result->terms.add(newTerm);
            }
            result->canonicalize(astBuilder);
            return result;
        }
        else if (auto val1 = as<IntVal>(op1))
        {
            auto result = astBuilder->create<PolynomialIntVal>();
            result->constantTerm = 0;
            auto factor1 = astBuilder->create<PolynomialIntValFactor>();
            factor1->power = 1;
            factor1->param = val1;
            if (poly0->constantTerm != 0)
            {
                auto term0 = astBuilder->create<PolynomialIntValTerm>();
                term0->constFactor = poly0->constantTerm;
                term0->paramFactors.add(factor1);
                result->terms.add(term0);
            }
            for (auto term : poly0->terms)
            {
                auto newTerm = astBuilder->create<PolynomialIntValTerm>();
                newTerm->constFactor = term->constFactor;
                newTerm->paramFactors.addRange(term->paramFactors);
                newTerm->paramFactors.add(factor1);
                result->terms.add(newTerm);
            }
            result->canonicalize(astBuilder);
            return result;
        }
        else
            return nullptr;
    }
    else if (as<ConstantIntVal>(op0))
    {
        return mul(astBuilder, op1, op0);
    }
    else if (auto val0 = as<IntVal>(op0))
    {
        if (auto poly1 = as<PolynomialIntVal>(op1))
        {
            return mul(astBuilder, op1, op0);
        }
        else if (auto cVal1 = as<ConstantIntVal>(op1))
        {
            auto result = astBuilder->create<PolynomialIntVal>();
            auto term = astBuilder->create<PolynomialIntValTerm>();
            term->constFactor = cVal1->value;
            auto factor0 = astBuilder->create<PolynomialIntValFactor>();
            factor0->power = 1;
            factor0->param = val0;
            term->paramFactors.add(factor0);
            result->terms.add(term);
            result->canonicalize(astBuilder);
            return result;
        }
        else if (auto val1 = as<IntVal>(op1))
        {
            auto result = astBuilder->create<PolynomialIntVal>();
            auto term = astBuilder->create<PolynomialIntValTerm>();
            term->constFactor = 1;
            auto factor0 = astBuilder->create<PolynomialIntValFactor>();
            factor0->power = 1;
            factor0->param = val0;
            term->paramFactors.add(factor0);
            auto factor1 = astBuilder->create<PolynomialIntValFactor>();
            factor1->power = 1;
            factor1->param = val1;
            term->paramFactors.add(factor1);
            result->terms.add(term);
            result->canonicalize(astBuilder);
            return result;
        }
    }
    return nullptr;
}

IntVal* PolynomialIntVal::canonicalize(ASTBuilder* builder)
{
    List<PolynomialIntValTerm*> newTerms;
    IntegerLiteralValue newConstantTerm = constantTerm;
    auto addTerm = [&](PolynomialIntValTerm* newTerm)
    {
        for (auto term : newTerms)
        {
            if (term->canCombineWith(*newTerm))
            {
                term->constFactor += newTerm->constFactor;
                return;
            }
        }
        newTerms.add(newTerm);
    };
    for (auto term : terms)
    {
        if (term->constFactor == 0)
            continue;
        List<PolynomialIntValFactor*> newFactors;
        List<bool> factorIsDifferent;
        for (Index i = 0; i < term->paramFactors.getCount(); i++)
        {
            auto factor = term->paramFactors[i];
            bool factorFound = false;
            for (Index j = 0; j < newFactors.getCount(); j++)
            {
                auto& newFactor = newFactors[j];
                if (factor->param->equalsVal(newFactor->param))
                {
                    if (!factorIsDifferent[j])
                    {
                        factorIsDifferent[j] = true;
                        auto clonedFactor = builder->create<PolynomialIntValFactor>();
                        clonedFactor->param = newFactor->param;
                        clonedFactor->power = newFactor->power;
                        newFactor = clonedFactor;
                    }
                    newFactor->power += factor->power;
                    factorFound = true;
                    break;
                }
            }
            if (!factorFound)
            {
                newFactors.add(factor);
                factorIsDifferent.add(false);
            }
        }
        List<PolynomialIntValFactor*> newFactors2;
        for (auto factor : newFactors)
        {
            if (factor->power != 0)
                newFactors2.add(factor);
        }
        if (newFactors2.getCount() == 0)
        {
            newConstantTerm += term->constFactor;
            continue;
        }
        newFactors2.sort([](PolynomialIntValFactor* t1, PolynomialIntValFactor* t2) {return *t1 < *t2; });
        bool isDifferent = false;
        if (newFactors2.getCount() != term->paramFactors.getCount())
            isDifferent = true;
        if (!isDifferent)
        {
            for (Index i = 0; i < term->paramFactors.getCount(); i++)
                if (term->paramFactors[i] != newFactors2[i])
                {
                    isDifferent = true;
                    break;
                }
        }
        if (!isDifferent)
        {
            addTerm(term);
        }
        else
        {
            auto newTerm = builder->create<PolynomialIntValTerm>();
            newTerm->constFactor = term->constFactor;
            newTerm->paramFactors = _Move(newFactors2);
            addTerm(newTerm);
        }
    }
    List<PolynomialIntValTerm*> newTerms2;
    for (auto term : newTerms)
    {
        if (term->constFactor == 0)
            continue;
        newTerms2.add(term);
    }
    newTerms2.sort([](PolynomialIntValTerm* t1, PolynomialIntValTerm* t2) {return *t1 < *t2; });
    terms = _Move(newTerms2);
    constantTerm = newConstantTerm;
    if (terms.getCount() == 1 && constantTerm == 0 && terms[0]->constFactor == 1 && terms[0]->paramFactors.getCount() == 1 &&
        terms[0]->paramFactors[0]->power == 1)
    {
        return terms[0]->paramFactors[0]->param;
    }
    if (terms.getCount() == 0)
        return builder->create<ConstantIntVal>(constantTerm);
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FuncCallIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool FuncCallIntVal::_equalsValOverride(Val* val)
{
    if (auto funcCallIntVal = as<FuncCallIntVal>(val))
    {
        if (!funcDeclRef.equals(funcCallIntVal->funcDeclRef))
            return false;
        if (args.getCount() != funcCallIntVal->args.getCount())
            return false;
        for (Index i = 0; i < args.getCount(); i++)
        {
            if (!args[i]->equalsVal(funcCallIntVal->args[i]))
                return false;
        }
        return true;
    }
    return false;
}

void FuncCallIntVal::_toTextOverride(StringBuilder& out)
{
    auto argToText = [&](int index)
    {
        if (as<PolynomialIntVal>(args[index]) || as<FuncCallIntVal>(args[index]))
        {
            out << "(";
            args[index]->toText(out);
            out << ")";
        }
        else
        {
            args[index]->toText(out);
        }
    };
    Name* name = funcDeclRef.getName();
    if (args.getCount() == 2)
    {
        argToText(0);
        out << (name ? name->text : "");
        argToText(1);;
    }
    else if (args.getCount() == 1)
    {
        out << (name ? name->text : "");
        argToText(0);
    }
    else if (name && name->text == "?:")
    {
        argToText(0);
        out << "?";
        argToText(1);
        out << ":";
        argToText(2);
    }
    else
    {
        if (name)
        {
            out << name->text;
        }
        out << "(";
        for (Index i = 0; i < args.getCount(); i++)
        {
            if (i > 0) out << ", ";
            args[i]->toText(out);
        }
        out << ")";
    }
}

HashCode FuncCallIntVal::_getHashCodeOverride()
{
    HashCode result = funcDeclRef.getHashCode();
    for (auto arg : args)
    {
        result = combineHash(result, arg->getHashCode());
    }
    return result;
}

static bool nameIs(Name* name, const char* val)
{
    if (name && name->text.getUnownedSlice() == val)
        return true;
    return false;
}

Val* FuncCallIntVal::tryFoldImpl(ASTBuilder* astBuilder, DeclRef<Decl> newFuncDecl, List<IntVal*>& newArgs, DiagnosticSink* sink)
{
    // Are all args const now?
    List<ConstantIntVal*> constArgs;
    bool allConst = true;
    for (auto arg : newArgs)
    {
        if (auto c = as<ConstantIntVal>(arg))
        {
            constArgs.add(c);
        }
        else
        {
            allConst = false;
            break;
        }
    }
    if (allConst)
    {
        // Evaluate the function.
        auto opName = newFuncDecl.getName();
        IntegerLiteralValue resultValue = 0;
        if (nameIs(opName, "=="))
        {
            resultValue = constArgs[0]->value / constArgs[1]->value;
        }
#define BINARY_OPERATOR_CASE(op) \
        else if (nameIs(opName, #op)) \
        { \
            resultValue = constArgs[0]->value op constArgs[1]->value; \
        }
        BINARY_OPERATOR_CASE(>=)
        BINARY_OPERATOR_CASE(<=)
        BINARY_OPERATOR_CASE(>)
        BINARY_OPERATOR_CASE(<)
        BINARY_OPERATOR_CASE(!=)
        BINARY_OPERATOR_CASE(<<)
        BINARY_OPERATOR_CASE(>>)
        BINARY_OPERATOR_CASE(&)
        BINARY_OPERATOR_CASE(|)
        BINARY_OPERATOR_CASE(^)
#undef BINARY_OPERATOR_CASE
#define DIV_OPERATOR_CASE(op)                                                        \
        else if (nameIs(opName, #op))                                                \
        {                                                                            \
            if (constArgs[1]->value == 0)                                            \
            {                                                                        \
                if (sink)                                                            \
                    sink->diagnose(newFuncDecl.getLoc(), Diagnostics::divideByZero); \
                return nullptr;                                                      \
            }                                                                        \
            resultValue = constArgs[0]->value op constArgs[1]->value;                \
        }
        DIV_OPERATOR_CASE(/)
        DIV_OPERATOR_CASE(%)
#undef DIV_OPERATOR_CASE
#define LOGICAL_OPERATOR_CASE(op) \
        else if (nameIs(opName, #op)) \
        { \
            resultValue = (((constArgs[0]->value!=0) op (constArgs[1]->value!=0)) ? 1 : 0); \
        }
        LOGICAL_OPERATOR_CASE(&&)
        LOGICAL_OPERATOR_CASE(|| )
#undef LOGICAL_OPERATOR_CASE
        else if (nameIs(opName, "!"))
        {
            resultValue = ((constArgs[0]->value != 0) ? 1 : 0);
        }
        else if (nameIs(opName, "~"))
        {
            resultValue = ~constArgs[0]->value;
        }
        else if (nameIs(opName, "?:"))
        {
            resultValue = constArgs[0]->value != 0 ? constArgs[1]->value : constArgs[2]->value;
        }
        else
        {
            SLANG_UNREACHABLE("constant folding of FuncCallIntVal");
        }
        return astBuilder->create<ConstantIntVal>(resultValue);
    }
    return nullptr;
}

Val* FuncCallIntVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto newFuncDeclRef = funcDeclRef.substituteImpl(astBuilder, subst, &diff);
    List<IntVal*> newArgs;
    for (auto& arg : args)
    {
        auto substArg = arg->substituteImpl(astBuilder, subst, &diff);
        if (substArg != arg)
            diff++;
        newArgs.add(as<IntVal>(substArg));
    }
    *ioDiff += diff;
    if (diff)
    {
        // TODO: report diagnostics back.
        auto newVal = tryFoldImpl(astBuilder, newFuncDeclRef, newArgs, nullptr);
        if (newVal)
            return newVal;
        else
        {
            auto result = astBuilder->create<FuncCallIntVal>();
            result->args = _Move(newArgs);
            result->funcDeclRef = newFuncDeclRef;
            result->funcType = funcType;
            return result;
        }
    }
    // Nothing found: don't substitute.
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WitnessLookupIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool WitnessLookupIntVal::_equalsValOverride(Val* val)
{
    if (auto lookupIntVal = as<WitnessLookupIntVal>(val))
    {
        if (!witness->equalsVal(lookupIntVal->witness))
            return false;
        if (key != lookupIntVal->key)
            return false;
        return true;
    }
    return false;
}

void WitnessLookupIntVal::_toTextOverride(StringBuilder& out)
{
    witness->sub->toText(out);
    out << ".";
    out << (key->getName() ? key->getName()->text : "??");
}

HashCode WitnessLookupIntVal::_getHashCodeOverride()
{
    HashCode result = witness->getHashCode();
    result = combineHash(result, Slang::getHashCode(key));
    return result;
}
Val* WitnessLookupIntVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto newWitness = witness->substituteImpl(astBuilder, subst, &diff);
    *ioDiff += diff;
    if (diff)
    {
        auto witnessEntry = tryFoldOrNull(astBuilder, as<SubtypeWitness>(newWitness), key);
        if (witnessEntry)
            return witnessEntry;
    }
    // Nothing found: don't substitute.
    return this;
}

Val* WitnessLookupIntVal::tryFoldOrNull(ASTBuilder* astBuilder, SubtypeWitness* witness, Decl* key)
{
    auto witnessEntry = tryLookUpRequirementWitness(astBuilder, witness, key);
    switch (witnessEntry.getFlavor())
    {
    case RequirementWitness::Flavor::val:
        return witnessEntry.getVal();
        break;
    default:
        break;
    }
    return nullptr;
}

Val* WitnessLookupIntVal::tryFold(ASTBuilder* astBuilder, SubtypeWitness* witness, Decl* key, Type* type)
{
    if (auto result = tryFoldOrNull(astBuilder, witness, key))
        return result;
    auto witnessResult = astBuilder->create<WitnessLookupIntVal>();
    witnessResult->witness = witness;
    witnessResult->key = key;
    witnessResult->type = type;
    return witnessResult;
}

} // namespace Slang
