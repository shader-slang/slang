// slang-ast-type.cpp
#include "slang-ast-val.h"

#include "core/slang-uint-set.h"
#include "slang-ast-builder.h"
#include "slang-ast-dispatch.h"
#include "slang-ast-natural-layout.h"
#include "slang-ast-substitution.h"
#include "slang-check-impl.h"
#include "slang-diagnostics.h"
#include "slang-mangle.h"
#include "slang-rich-diagnostics.h"
#include "slang-syntax.h"

#include <assert.h>
#include <typeinfo>

namespace Slang
{

void ValNodeDesc::init()
{
    Hasher hasher;
    hasher.hashValue(type.getTag());
    for (Index i = 0; i < operands.getCount(); ++i)
    {
        // Note: we are hashing the raw pointer value rather
        // than the content of the value node. This is done
        // to match the semantics implemented for `==` on
        // `NodeDesc`.
        //
        hasher.hashValue(operands[i].values.intOperand);
    }
    hashCode = hasher.getResult();
}

Val* Val::substitute(ASTBuilder* astBuilder, SubstitutionSet subst)
{
    if (!subst)
        return this;
    int diff = 0;
    return substituteImpl(astBuilder, subst, &diff);
}

Val* Val::substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    return substituteValWithCache(
        this,
        astBuilder,
        subst,
        ioDiff,
        [&](SubstitutionSet cachedSubst, int* cachedDiff) -> Val*
        {
            return ASTNodeDispatcher<Val, Val*>::dispatch(
                this,
                [&](auto value) -> Val*
                { return value->_substituteImplOverride(astBuilder, cachedSubst, cachedDiff); });
        });
}

void Val::toText(StringBuilder& out){SLANG_AST_NODE_VIRTUAL_CALL(Val, toText, (out))}

Val* Val::_resolveImplOverride()
{
    SLANG_UNEXPECTED("Val::_resolveImplOverride not overridden");
}

Val* Val::resolveImpl()
{
    SLANG_AST_NODE_VIRTUAL_CALL(Val, resolveImpl, ());
}

Val* Val::resolve()
{
    auto astBuilder = getCurrentASTBuilder();
    // If we are not in a proper checking context, just return the previously resolved val.
    if (!astBuilder)
        return m_resolvedVal ? m_resolvedVal : this;
    if (m_resolvedVal && m_resolvedValEpoch == astBuilder->getEpoch())
    {
        SLANG_ASSERT(as<Val>(m_resolvedVal));
        return m_resolvedVal;
    }
    // Update epoch now to avoid infinite recursion.
    m_resolvedValEpoch = astBuilder->getEpoch();
    m_resolvedVal = resolveImpl();
#ifdef _DEBUG
    if (m_resolvedVal->_debugUID > 0 && this->_debugUID < 0)
    {
        SLANG_ASSERT_FAILURE(
            "should not be modifying the core module vals outside of the core module checking.");
    }
#endif
    return m_resolvedVal;
}

void Val::_setUnique()
{
    m_resolvedVal = this;
    m_resolvedValEpoch = getCurrentASTBuilder()->getEpoch();
}

Val* Val::defaultResolveImpl()
{
    // Default resolve implementation is to recursively resolve all operands, and lookup in
    // deduplication cache.
    ValNodeDesc newDesc;
    newDesc.type = SyntaxClass<NodeBase>(astNodeType);
    bool diff = false;
    for (auto operand : m_operands)
    {
        if (operand.kind == ValNodeOperandKind::ValNode)
        {
            auto valOperand = as<Val>(operand.values.nodeOperand);
            if (valOperand)
            {
                auto newOperand = valOperand->resolve();
                if (newOperand != valOperand)
                {
                    diff = true;
                    operand.values.nodeOperand = newOperand;
                }
            }
        }
        newDesc.operands.add(operand);
    }

    if (!diff)
        return this;

    newDesc.init();
    auto astBuilder = getCurrentASTBuilder();
    return astBuilder->_getOrCreateImpl(_Move(newDesc));
}

String Val::toString()
{
    StringBuilder builder;
    toText(builder);
    return builder;
}

HashCode Val::getHashCode()
{
    return Slang::getHashCode(resolve());
}

Val* Val::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    // Default behavior is to not substitute at all
    return this;
}

void Val::_toTextOverride(StringBuilder& out)
{
    SLANG_UNUSED(out);
    SLANG_UNEXPECTED("Val::_toTextOverride not overridden");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ConstantIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ConstantIntVal::_toTextOverride(StringBuilder& out)
{
    if (auto enumTypeDecl = isDeclRefTypeOf<EnumDecl>(getType()))
    {
        // If this is an enum type, then we want to print the name of the
        // corresponding enum case, instead of the raw integer value, if possible.
        //
        // We will look up the enum case that corresponds to the value, and
        // print its name if we can find one.
        //
        for (auto enumCase : enumTypeDecl.getDecl()->getMembersOfType<EnumCaseDecl>())
        {
            if (auto constVal = as<ConstantIntVal>(enumCase->tagVal))
            {
                if (constVal->getValue() == getValue())
                {
                    out << DeclRef(enumCase);
                    return;
                }
            }
        }

        // Fallback to explicit cast to the enum type.
        out << getType() << "(" << getValue() << ")";
        return;
    }
    out << getValue();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclRefIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void DeclRefIntVal::_toTextOverride(StringBuilder& out)
{
    Name* name = getDeclRef().getName();
    if (name)
    {
        out << name->text;
    }
}

Val* maybeSubstituteGenericParam(Val* paramVal, Decl* paramDecl, SubstitutionSet subst, int* ioDiff)
{
    // search for a substitution that might apply to us
    auto outerGeneric = as<GenericDecl>(paramDecl->parentDecl);
    if (!outerGeneric)
        return paramVal;

    GenericAppDeclRef* genAppArgs = subst.findGenericAppDeclRef(outerGeneric);
    if (!genAppArgs)
    {
        return paramVal;
    }

    auto args = genAppArgs->getArgs();

    // In some cases, we construct a `DeclRef` to a `GenericDecl`
    // (or a declaration under one) that only includes argument
    // values for a prefix of the parameters of the generic.
    //
    // If we aren't careful, we could end up indexing into the
    // argument list past the available range.
    //
    Count argCount = args.getCount();

    Count argIndex = 0;
    for (auto m : outerGeneric->getDirectMemberDecls())
    {
        // If we have run out of arguments, then we can stop
        // iterating over the parameters, because `this`
        // parameter will not be replaced with anything by
        // the substituion.
        //
        if (argIndex >= argCount)
        {
            return paramVal;
        }


        if (m == paramDecl)
        {
            // We've found it, so return the corresponding specialization argument
            (*ioDiff)++;
            return args[argIndex];
        }
        else if (const auto typeParam = as<GenericTypeParamDeclBase>(m); typeParam)
        {
            argIndex++;
        }
        else if (const auto valPackParam = as<GenericValuePackParamDecl>(m); valPackParam)
        {
            argIndex++;
        }
        else if (const auto valParam = as<GenericValueParamDecl>(m); valParam)
        {
            argIndex++;
        }
        else
        {
        }
    }

    // Nothing found: don't substitute.
    return paramVal;
}

Val* DeclRefIntVal::_substituteImplOverride(
    ASTBuilder* /* astBuilder */,
    SubstitutionSet subst,
    int* ioDiff)
{
    if (auto result = maybeSubstituteGenericParam(this, getDeclRef().getDecl(), subst, ioDiff))
        return result;

    return this;
}

bool DeclRefIntVal::_isLinkTimeValOverride()
{
    return getDeclRef().getDecl()->hasModifier<ExternModifier>();
}

Val* DeclRefIntVal::_linkTimeResolveOverride(Dictionary<String, IntVal*>& map)
{
    auto name = getMangledName(getCurrentASTBuilder(), getDeclRef().declRefBase);
    IntVal* v;
    if (map.tryGetValue(name, v))
        return v;
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ErrorIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ErrorIntVal::_toTextOverride(StringBuilder& out)
{
    out << toSlice("<error>");
}

Val* ErrorIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

Val* TypeEqualityWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    auto type = as<Type>(getSub()->substituteImpl(astBuilder, subst, ioDiff));
    TypeEqualityWitness* rs = astBuilder->getOrCreate<TypeEqualityWitness>(type, type);
    return rs;
}

void TypeEqualityWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("TypeEqualityWitness(") << getSub() << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TypePackSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* TypePackSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    ShortList<SubtypeWitness*> newWitnesses;
    for (Index i = 0; i < getCount(); i++)
    {
        auto witness = getWitness(i);
        auto newWitness = as<SubtypeWitness>(witness->substituteImpl(astBuilder, subst, &diff));
        newWitnesses.add(newWitness);
    }
    auto newSub = as<Type>(getSub()->substituteImpl(astBuilder, subst, &diff));
    auto newSup = as<Type>(getSup()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    (*ioDiff)++;
    return getCurrentASTBuilder()->getSubtypeWitnessPack(
        newSub,
        newSup,
        newWitnesses.getArrayView().arrayView);
}

Val* TypePackSubtypeWitness::_resolveImplOverride()
{
    int diff = 0;
    ShortList<SubtypeWitness*> newWitnesses;
    for (Index i = 0; i < getCount(); i++)
    {
        auto witness = getWitness(i);
        auto newWitness = as<SubtypeWitness>(witness->resolve());
        if (witness != newWitness)
            diff++;
        newWitnesses.add(newWitness);
    }
    auto newSub = as<Type>(getSub()->resolve());
    if (newSub != getSub())
        diff++;
    auto newSup = as<Type>(getSup()->resolve());
    if (newSup != getSup())
        diff++;

    if (!diff)
        return this;
    return getCurrentASTBuilder()->getSubtypeWitnessPack(
        newSub,
        newSup,
        newWitnesses.getArrayView().arrayView);
}

void TypePackSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("Pack(");
    for (Index i = 0; i < getCount(); i++)
    {
        if (i != 0)
            out << toSlice(", ");
        getWitness(i)->toText(out);
    }
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExpandSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* ExpandSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newSub = as<Type>(getSub()->substituteImpl(astBuilder, subst, &diff));
    auto newSup = as<Type>(getSup()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    if (auto subTypePack = as<ConcreteTypePack>(newSub))
    {
        // If sub is substituted into a concrete type pack, we should return a
        // TypePackSubtypeWitness.
        ShortList<SubtypeWitness*> newWitnesses;
        for (int i = 0; i < (int)subTypePack->getTypeCount(); i++)
        {
            auto elementType = subTypePack->getElementType(i);
            subst.packExpansionIndex = i;
            auto elementWitness = as<SubtypeWitness>(
                getPatternTypeWitness()->substituteImpl(astBuilder, subst, &diff));
            auto newWitness = getCurrentASTBuilder()->getExpandSubtypeWitness(
                elementType,
                newSup,
                elementWitness);
            newWitnesses.add(as<SubtypeWitness>(newWitness));
        }
        (*ioDiff)++;
        return getCurrentASTBuilder()->getSubtypeWitnessPack(
            newSub,
            newSup,
            newWitnesses.getArrayView().arrayView);
    }

    (*ioDiff)++;
    auto newPatternWitness =
        as<SubtypeWitness>(getPatternTypeWitness()->substituteImpl(astBuilder, subst, &diff));
    return getCurrentASTBuilder()->getExpandSubtypeWitness(newSub, newSup, newPatternWitness);
}

Val* ExpandSubtypeWitness::_resolveImplOverride()
{
    int diff = 0;
    auto newPatternWitness = as<SubtypeWitness>(getPatternTypeWitness()->resolve());
    if (newPatternWitness != getPatternTypeWitness())
        diff++;
    auto newSub = as<Type>(getSub()->resolve());
    if (newSub != getSub())
        diff++;
    auto newSup = as<Type>(getSup()->resolve());
    if (newSup != getSup())
        diff++;
    if (!diff)
        return this;
    return getCurrentASTBuilder()->getExpandSubtypeWitness(newSub, newSup, newPatternWitness);
}

void ExpandSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("ExpandWitness(");
    getPatternTypeWitness()->toText(out);
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! EachSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* EachSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newPatternWitness =
        as<SubtypeWitness>(getPatternTypeWitness()->substituteImpl(astBuilder, subst, &diff));
    if (auto witnessPack = as<TypePackSubtypeWitness>(newPatternWitness))
    {
        if (subst.packExpansionIndex >= 0 && subst.packExpansionIndex < witnessPack->getCount())
        {
            auto newWitness = witnessPack->getWitness(subst.packExpansionIndex);
            (*ioDiff)++;
            return newWitness;
        }
    }
    auto newSub = as<Type>(getSub()->substituteImpl(astBuilder, subst, &diff));
    auto newSup = as<Type>(getSup()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    (*ioDiff)++;
    return getCurrentASTBuilder()->getEachSubtypeWitness(newSub, newSup, newPatternWitness);
}

Val* EachSubtypeWitness::_resolveImplOverride()
{
    int diff = 0;
    auto newPatternWitness = as<SubtypeWitness>(getPatternTypeWitness()->resolve());
    if (newPatternWitness != getPatternTypeWitness())
        diff++;
    auto newSub = as<Type>(getSub()->resolve());
    if (newSub != getSub())
        diff++;
    auto newSup = as<Type>(getSup()->resolve());
    if (newSup != getSup())
        diff++;
    if (!diff)
        return this;
    return getCurrentASTBuilder()->getEachSubtypeWitness(newSub, newSup, newPatternWitness);
}

void EachSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("EachWitness(");
    getPatternTypeWitness()->toText(out);
    out << toSlice(")");
}

namespace
{
template<typename TWitness>
Val* _substituteExtractPackSubtypeWitness(
    TWitness* witness,
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff,
    bool useLast)
{
    int diff = 0;
    auto newPatternWitness = as<SubtypeWitness>(
        witness->getPatternTypeWitness()->substituteImpl(astBuilder, subst, &diff));
    if (auto witnessPack = as<TypePackSubtypeWitness>(newPatternWitness))
    {
        if (witnessPack->getCount() > 0)
        {
            auto index = useLast ? witnessPack->getCount() - 1 : 0;
            (*ioDiff)++;
            return witnessPack->getWitness(index);
        }
    }
    auto newSub = as<Type>(witness->getSub()->substituteImpl(astBuilder, subst, &diff));
    auto newSup = as<Type>(witness->getSup()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return witness;
    (*ioDiff)++;
    return useLast
               ? getCurrentASTBuilder()->getLastSubtypeWitness(newSub, newSup, newPatternWitness)
               : getCurrentASTBuilder()->getFirstSubtypeWitness(newSub, newSup, newPatternWitness);
}

template<typename TWitness>
Val* _resolveExtractPackSubtypeWitness(TWitness* witness, bool useLast)
{
    int diff = 0;
    auto newPatternWitness = as<SubtypeWitness>(witness->getPatternTypeWitness()->resolve());
    if (newPatternWitness != witness->getPatternTypeWitness())
        diff++;
    if (auto witnessPack = as<TypePackSubtypeWitness>(newPatternWitness))
    {
        if (witnessPack->getCount() > 0)
        {
            auto index = useLast ? witnessPack->getCount() - 1 : 0;
            return witnessPack->getWitness(index);
        }
    }
    auto newSub = as<Type>(witness->getSub()->resolve());
    if (newSub != witness->getSub())
        diff++;
    auto newSup = as<Type>(witness->getSup()->resolve());
    if (newSup != witness->getSup())
        diff++;
    if (!diff)
        return witness;
    return useLast
               ? getCurrentASTBuilder()->getLastSubtypeWitness(newSub, newSup, newPatternWitness)
               : getCurrentASTBuilder()->getFirstSubtypeWitness(newSub, newSup, newPatternWitness);
}

template<typename TWitness>
Val* _substituteTrimPackSubtypeWitness(
    TWitness* witness,
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff,
    bool trimLast)
{
    int diff = 0;
    auto newPatternWitness = as<SubtypeWitness>(
        witness->getPatternTypeWitness()->substituteImpl(astBuilder, subst, &diff));
    auto newSub = as<Type>(witness->getSub()->substituteImpl(astBuilder, subst, &diff));
    auto newSup = as<Type>(witness->getSup()->substituteImpl(astBuilder, subst, &diff));
    if (auto witnessPack = as<TypePackSubtypeWitness>(newPatternWitness))
    {
        List<SubtypeWitness*> newWitnesses;
        Index end = trimLast ? witnessPack->getCount() - 1 : witnessPack->getCount();
        Index start = trimLast ? 0 : 1;
        for (Index i = start; i < end; i++)
            newWitnesses.add(witnessPack->getWitness(i));
        (*ioDiff)++;
        return getCurrentASTBuilder()->getSubtypeWitnessPack(
            newSub,
            newSup,
            newWitnesses.getArrayView());
    }
    if (!diff)
        return witness;
    (*ioDiff)++;
    return trimLast ? getCurrentASTBuilder()->getTrimLastSubtypeWitness(
                          newSub,
                          newSup,
                          newPatternWitness)
                    : getCurrentASTBuilder()->getTrimFirstSubtypeWitness(
                          newSub,
                          newSup,
                          newPatternWitness);
}

template<typename TWitness>
Val* _resolveTrimPackSubtypeWitness(TWitness* witness, bool trimLast)
{
    int diff = 0;
    auto newPatternWitness = as<SubtypeWitness>(witness->getPatternTypeWitness()->resolve());
    if (newPatternWitness != witness->getPatternTypeWitness())
        diff++;
    auto newSub = as<Type>(witness->getSub()->resolve());
    if (newSub != witness->getSub())
        diff++;
    auto newSup = as<Type>(witness->getSup()->resolve());
    if (newSup != witness->getSup())
        diff++;
    if (auto witnessPack = as<TypePackSubtypeWitness>(newPatternWitness))
    {
        List<SubtypeWitness*> newWitnesses;
        Index end = trimLast ? witnessPack->getCount() - 1 : witnessPack->getCount();
        Index start = trimLast ? 0 : 1;
        for (Index i = start; i < end; i++)
            newWitnesses.add(witnessPack->getWitness(i));
        return getCurrentASTBuilder()->getSubtypeWitnessPack(
            newSub,
            newSup,
            newWitnesses.getArrayView());
    }
    if (!diff)
        return witness;
    return trimLast ? getCurrentASTBuilder()->getTrimLastSubtypeWitness(
                          newSub,
                          newSup,
                          newPatternWitness)
                    : getCurrentASTBuilder()->getTrimFirstSubtypeWitness(
                          newSub,
                          newSup,
                          newPatternWitness);
}
} // namespace

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FirstSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* FirstSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    return _substituteExtractPackSubtypeWitness(this, astBuilder, subst, ioDiff, false);
}

Val* FirstSubtypeWitness::_resolveImplOverride()
{
    return _resolveExtractPackSubtypeWitness(this, false);
}

void FirstSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("FirstWitness(");
    getPatternTypeWitness()->toText(out);
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LastSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* LastSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    return _substituteExtractPackSubtypeWitness(this, astBuilder, subst, ioDiff, true);
}

Val* LastSubtypeWitness::_resolveImplOverride()
{
    return _resolveExtractPackSubtypeWitness(this, true);
}

void LastSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("LastWitness(");
    getPatternTypeWitness()->toText(out);
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TrimFirstSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* TrimFirstSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    return _substituteTrimPackSubtypeWitness(this, astBuilder, subst, ioDiff, false);
}

Val* TrimFirstSubtypeWitness::_resolveImplOverride()
{
    return _resolveTrimPackSubtypeWitness(this, false);
}

void TrimFirstSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("TrimFirstWitness(");
    getPatternTypeWitness()->toText(out);
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TrimLastSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* TrimLastSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    return _substituteTrimPackSubtypeWitness(this, astBuilder, subst, ioDiff, true);
}

Val* TrimLastSubtypeWitness::_resolveImplOverride()
{
    return _resolveTrimPackSubtypeWitness(this, true);
}

void TrimLastSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("TrimLastWitness(");
    getPatternTypeWitness()->toText(out);
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PackBranchSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* PackBranchSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newSub = as<Type>(getSub()->substituteImpl(astBuilder, subst, &diff));
    auto newSup = as<Type>(getSup()->substituteImpl(astBuilder, subst, &diff));
    auto newPackOperand = getPackOperand()->substituteImpl(astBuilder, subst, &diff);
    auto newEmptyWitness =
        as<SubtypeWitness>(getEmptyWitness()->substituteImpl(astBuilder, subst, &diff));
    auto newNonEmptyWitness =
        as<SubtypeWitness>(getNonEmptyWitness()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getPackBranchSubtypeWitness(
        newSub,
        newSup,
        newPackOperand,
        newEmptyWitness,
        newNonEmptyWitness);
}

Val* PackBranchSubtypeWitness::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();
    auto newSub = as<Type>(getSub()->resolve());
    auto newSup = as<Type>(getSup()->resolve());
    auto newPackOperand = getPackOperand()->resolve();
    auto newEmptyWitness = as<SubtypeWitness>(getEmptyWitness()->resolve());
    auto newNonEmptyWitness = as<SubtypeWitness>(getNonEmptyWitness()->resolve());
    if (newSub == getSub() && newSup == getSup() && newPackOperand == getPackOperand() &&
        newEmptyWitness == getEmptyWitness() && newNonEmptyWitness == getNonEmptyWitness())
        return this;
    return astBuilder->getPackBranchSubtypeWitness(
        newSub,
        newSup,
        newPackOperand,
        newEmptyWitness,
        newNonEmptyWitness);
}

void PackBranchSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("PackBranchWitness(");
    getPackOperand()->toText(out);
    out << toSlice(", ");
    getEmptyWitness()->toText(out);
    out << toSlice(", ");
    getNonEmptyWitness()->toText(out);
    out << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclaredSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* DeclaredSubtypeWitness::_resolveImplOverride()
{
    auto resolvedDeclRef = getDeclRef().declRefBase->resolve();
    if (auto resolvedVal = as<SubtypeWitness>(resolvedDeclRef))
        return resolvedVal;

    auto newSub = as<Type>(getSub()->resolve());
    auto newSup = as<Type>(getSup()->resolve());

    // If we are trying to lookup for a witness that A<:B from a witness(A<:B), we
    // can just return the witness itself.
    if (auto lookupDeclRef = as<LookupDeclRef>(resolvedDeclRef))
    {
        auto witnessToLookupFrom = lookupDeclRef->getWitness();
        if (witnessToLookupFrom->getSub()->equals(newSub) &&
            witnessToLookupFrom->getSup()->equals(newSup))
            return witnessToLookupFrom;
    }
    auto newDeclRef = as<DeclRefBase>(resolvedDeclRef);
    if (!newDeclRef)
        newDeclRef = getDeclRef().declRefBase;
    if (newSub != getSub() || newSup != getSup() || newDeclRef != getDeclRef())
    {
        return getCurrentASTBuilder()->getDeclaredSubtypeWitness(newSub, newSup, newDeclRef);
    }
    return this;
}

ConversionCost DeclaredSubtypeWitness::_getOverloadResolutionCostOverride()
{
    if (auto nestedLookup = as<LookupDeclRef>(getDeclRef().declRefBase))
        return nestedLookup->getWitness()->getOverloadResolutionCost() +
               kConversionCost_GenericParamUpcast;
    return kConversionCost_None;
}

Val* DeclaredSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    if (auto genConstraintDeclRef = getDeclRef().as<GenericTypeConstraintDecl>())
    {
        auto genericDecl = as<GenericDecl>(getDeclRef().getDecl()->parentDecl);
        if (!genericDecl)
            goto breakLabel;

        // search for a substitution that might apply to us
        auto args = tryGetGenericArguments(subst, genericDecl);
        if (args.getCount() == 0)
            goto breakLabel;

        bool found = false;
        Index index = 0;
        for (auto member : genericDecl->getDirectMemberDecls())
        {
            if (auto constraintParam = as<GenericTypeConstraintDecl>(member))
            {
                if (constraintParam == getDeclRef().getDecl())
                {
                    found = true;
                    break;
                }
                index++;
            }
            else if (as<TypeCoercionConstraintDecl>(member))
            {
                index++;
            }
            else if (as<NonEmptyPackConstraintDecl>(member))
            {
                index++;
            }
            else if (as<GenericVariadicPackCountConstraintDecl>(member))
            {
                index++;
            }
            else if (as<HasDiffTypeInfoConstraintDecl>(member))
            {
                index++;
            }
        }
        if (found)
        {
            auto ordinaryParamCount =
                genericDecl->getMembersOfType<GenericTypeParamDeclBase>().getCount() +
                genericDecl->getMembersOfType<GenericValueParamDecl>().getCount() +
                genericDecl->getMembersOfType<GenericValuePackParamDecl>().getCount();
            if (index + ordinaryParamCount < args.getCount())
            {
                (*ioDiff)++;
                return args[index + ordinaryParamCount];
            }
            else
            {
                // When the `subst` represents a partial substitution, we may not have a
                // corresponding argument. In this case we just return the original witness.
                //
                goto breakLabel;
            }
        }
    }
    else if (auto thisTypeConstraintDeclRef = getDeclRef().as<ThisTypeConstraintDecl>())
    {
        auto lookupSubst = subst.findLookupDeclRef();
        if (lookupSubst &&
            lookupSubst->getSupDecl() == thisTypeConstraintDeclRef.getDecl()->getInterfaceDecl())
        {
            (*ioDiff)++;
            return lookupSubst->getWitness();
        }
    }

breakLabel:;

    // Perform substitution on the constituent elements.
    int diff = 0;
    auto substSub = as<Type>(getSub()->substituteImpl(astBuilder, subst, &diff));
    auto substSup = as<Type>(getSup()->substituteImpl(astBuilder, subst, &diff));

    auto substDeclRef = getDeclRef().substituteImpl(astBuilder, subst, &diff);
    auto rs = astBuilder->getDeclaredSubtypeWitness(substSub, substSup, substDeclRef);

    if (!diff)
        return this;

    (*ioDiff)++;

    return rs;
}

void DeclaredSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("DeclaredSubtypeWitness(") << getSub() << toSlice(", ") << getSup()
        << toSlice(", ") << getDeclRef() << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TransitiveSubtypeWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Val* TransitiveSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;

    SubtypeWitness* substSubToMid =
        as<SubtypeWitness>(getSubToMid()->substituteImpl(astBuilder, subst, &diff));
    SubtypeWitness* substMidToSup =
        as<SubtypeWitness>(getMidToSup()->substituteImpl(astBuilder, subst, &diff));

    // If nothing changed, then we can bail out early.
    if (!diff)
        return this;

    // Something changes, so let the caller know.
    (*ioDiff)++;

    // If it possible that substitution could have led to either of the
    // constituent witnesses being simplified, and such simplification could
    // (in principle) lead to opportunities to simplify this transitive witness.
    // As such, we do not simply create a fresh `TransitiveSubtypeWitness` here,
    // and instead go through a bottleneck routine in the `ASTBuilder` that will
    // detect and handle any possible simplifications.
    //
    return astBuilder->getTransitiveSubtypeWitness(substSubToMid, substMidToSup);
}

ConversionCost TransitiveSubtypeWitness::_getOverloadResolutionCostOverride()
{
    return getSubToMid()->getOverloadResolutionCost() + getMidToSup()->getOverloadResolutionCost() +
           kConversionCost_GenericParamUpcast;
}

void TransitiveSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    // Note: we only print the constituent
    // witnesses, and rely on them to print
    // the starting and ending types.

    out << toSlice("TransitiveSubtypeWitness(") << getSubToMid() << toSlice(", ") << getMidToSup()
        << toSlice(")");
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExtractExistentialSubtypeWitness
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExtractExistentialSubtypeWitness::_toTextOverride(StringBuilder& out)
{
    out << toSlice("extractExistentialValue(") << getDeclRef() << toSlice(")");
}

Val* ExtractExistentialSubtypeWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;

    auto substDeclRef = getDeclRef().substituteImpl(astBuilder, subst, &diff);
    auto substSub = as<Type>(getSub()->substituteImpl(astBuilder, subst, &diff));
    auto substSup = as<Type>(getSup()->substituteImpl(astBuilder, subst, &diff));

    if (!diff)
        return this;

    (*ioDiff)++;

    ExtractExistentialSubtypeWitness* substValue =
        astBuilder->getOrCreate<ExtractExistentialSubtypeWitness>(substSub, substSup, substDeclRef);
    return substValue;
}

void BuiltinTypeCoercionWitness::_toTextOverride(StringBuilder& out)
{
    out << "BuiltinTypeCoercionWitness(";
    if (getFromType())
        out << getFromType();
    else
        out << "<null>";
    out << ",";
    if (getToType())
        out << getToType();
    else
        out << "<null>";
    out << ")";
}

Val* BuiltinTypeCoercionWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;

    auto substFrom = as<Type>(getFromType()->substituteImpl(astBuilder, subst, &diff));
    auto substTo = as<Type>(getToType()->substituteImpl(astBuilder, subst, &diff));

    if (!diff)
        return this;

    (*ioDiff)++;

    BuiltinTypeCoercionWitness* substValue =
        astBuilder->getBuiltinTypeCoercionWitness(substFrom, substTo);
    return substValue;
}

Val* BuiltinTypeCoercionWitness::_resolveImplOverride()
{
    auto newFrom = as<Type>(getFromType()->resolve());
    auto newTo = as<Type>(getToType()->resolve());

    if (newFrom != getFromType() || newTo != getToType())
    {
        return getCurrentASTBuilder()->getBuiltinTypeCoercionWitness(newFrom, newTo);
    }
    return this;
}

void DeclRefTypeCoercionWitness::_toTextOverride(StringBuilder& out)
{
    out << "DeclRefTypeCoercionWitness(";
    if (getFromType())
        out << getFromType();
    else
        out << "<null>";
    out << ",";
    if (getToType())
        out << getToType();
    else
        out << "<null>";
    out << ",";
    if (getDeclRef())
        out << getDeclRef();
    else
        out << "<null>";
    out << ")";
}

Val* DeclRefTypeCoercionWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;

    auto substDeclRef = getDeclRef().substituteImpl(astBuilder, subst, &diff);
    auto substFrom = as<Type>(getFromType()->substituteImpl(astBuilder, subst, &diff));
    auto substTo = as<Type>(getToType()->substituteImpl(astBuilder, subst, &diff));

    if (!diff)
        return this;

    (*ioDiff)++;

    DeclRefTypeCoercionWitness* substValue =
        astBuilder->getDeclRefTypeCoercionWitness(substFrom, substTo, substDeclRef);
    return substValue;
}

Val* DeclRefTypeCoercionWitness::_resolveImplOverride()
{
    Val* resolvedDeclRef = nullptr;
    if (getDeclRef())
        resolvedDeclRef = getDeclRef().declRefBase->resolve();
    if (auto resolvedVal = as<Witness>(resolvedDeclRef))
        return resolvedVal;

    auto newFrom = as<Type>(getFromType()->resolve());
    auto newTo = as<Type>(getToType()->resolve());

    auto newDeclRef = as<DeclRefBase>(resolvedDeclRef);
    if (!newDeclRef)
        newDeclRef = getDeclRef().declRefBase;
    if (newFrom != getFromType() || newTo != getToType() || newDeclRef != getDeclRef())
    {
        return getCurrentASTBuilder()->getDeclRefTypeCoercionWitness(newFrom, newTo, newDeclRef);
    }
    return this;
}

// DiffTypeInfoWitness
void DiffTypeInfoWitness::_toTextOverride(StringBuilder& out)
{
    out << "DiffTypeInfoWitness(";
    getOperand(0)->toText(out);
    out << ")";
}

Val* DiffTypeInfoWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    List<Val*> newOperands;

    int diff = 0;
    for (Index ii = 0; ii < (Index)getOperandCount(); ii++)
    {
        auto operand = getOperand(ii);
        auto newOperand = operand ? operand->substituteImpl(astBuilder, subst, &diff) : nullptr;
        newOperands.add(newOperand);
    }

    if (diff)
    {
        (*ioDiff)++;
        return astBuilder->getOrCreate<DiffTypeInfoWitness>(newOperands);
    }
    else
    {
        return this;
    }
}

Val* DiffTypeInfoWitness::_resolveImplOverride()
{
    List<Val*> newOperands;
    int diff = 0;
    for (Index ii = 0; ii < (Index)getOperandCount(); ii++)
    {
        auto operand = getOperand(ii);
        auto newOperand = operand ? operand->resolve() : nullptr;
        if (newOperand != operand)
            diff++;
        newOperands.add(newOperand);
    }

    if (diff)
    {
        return getCurrentASTBuilder()->getOrCreate<DiffTypeInfoWitness>(newOperands);
    }
    else
    {
        return this;
    }
}

void HigherOrderDiffTypeTranslationWitness::_toTextOverride(StringBuilder& out)
{
    out << "HigherOrderDiffTypeTranslationWitness(";
    getOperand(0)->toText(out);
    out << ")";
}

Val* HigherOrderDiffTypeTranslationWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substWitness = getBaseWitness()->substituteImpl(astBuilder, subst, &diff);

    if (diff)
    {
        (*ioDiff)++;
        return astBuilder->getOrCreate<HigherOrderDiffTypeTranslationWitness>(substWitness);
    }
    else
    {
        return this;
    }
}

Val* HigherOrderDiffTypeTranslationWitness::_resolveImplOverride()
{
    auto resolvedWitness = getBaseWitness()->resolve();

    if (auto diffTypeInfoWitness = as<DiffTypeInfoWitness>(resolvedWitness))
    {
        // Generate higher order diff-type-info witness.
        //
        // This is a little bit of a hack for the fact that diff-type-info witness
        // only gets the witness for ParamType : IDifferentiable/IDifferentiablePtrType, but
        // during higher-order autodiff, we may also need the witness for
        // DifferentialPair<ParamType> : IDifferentiable or
        // DifferentialPtrPair<ParamType> : IDifferentiablePtrType.
        //
        // Unfortunately, there could be arbitrary number of nesting levels, so it's not tractable
        // to store all of them on the DiffTypeInfoWitness.
        //
        // Technically, this requires storing a higher-rank witness (i.e.
        // a witness of the form: forall T : IDifferentiable . DifferentialPair<T> :
        // IDifferentiable, and the corresponding pointer-pair form) but our type (and decl-ref)
        // system does not support this at the moment.
        //
        // Fortunately, in practice these pair conformances are the only higher-rank witnesses we
        // need to handle, so we'll manually construct the pair witness here by looking up the
        // matching InheritanceDecl in the pair declaration, and forming a specialized
        // member-decl-ref to it.
        //

        auto astBuilder = getCurrentASTBuilder();
        auto diffPairGenericDecl = as<GenericDecl>(
            astBuilder->getSharedASTBuilder()->findMagicDecl("DifferentialPairType"));
        SLANG_ASSERT(diffPairGenericDecl);
        GenericDecl* diffPtrPairGenericDecl = nullptr;

        auto differentiableInterfaceType = astBuilder->getDifferentiableInterfaceType();
        auto differentiablePtrInterfaceType = astBuilder->getDifferentiableRefInterfaceType();

        auto findInheritanceDecl = [&](GenericDecl* pairGenericDecl,
                                       Type* interfaceType) -> InheritanceDecl*
        {
            for (auto inheritanceDecl : getMembersOfType<InheritanceDecl>(
                     astBuilder,
                     as<ContainerDecl>(pairGenericDecl->inner)->getDefaultDeclRef()))
            {
                if (inheritanceDecl.getDecl()->base.type == interfaceType)
                    return inheritanceDecl.getDecl();
            }
            return nullptr;
        };

        InheritanceDecl* diffInheritanceDecl =
            findInheritanceDecl(diffPairGenericDecl, differentiableInterfaceType);
        SLANG_ASSERT(diffInheritanceDecl);
        InheritanceDecl* diffPtrInheritanceDecl = nullptr;

        auto ensureDiffPtrPairDecls = [&]()
        {
            // Higher-order witnesses for value types can resolve while the core module is still
            // being compiled. Only require the pointer-pair declaration when the base witness
            // actually targets IDifferentiablePtrType.
            if (!diffPtrPairGenericDecl)
            {
                diffPtrPairGenericDecl = as<GenericDecl>(
                    astBuilder->getSharedASTBuilder()->findMagicDecl("DifferentialPtrPairType"));
                SLANG_ASSERT(diffPtrPairGenericDecl);
            }
            if (!diffPtrInheritanceDecl)
            {
                diffPtrInheritanceDecl =
                    findInheritanceDecl(diffPtrPairGenericDecl, differentiablePtrInterfaceType);
                SLANG_ASSERT(diffPtrInheritanceDecl);
            }
        };

        // The base witness determines the pair flavor. For pointer-differentiable `this`, reusing
        // `DifferentialPairType` would form
        // DiffPair(DifferentialPtrPair<T>, T:IDifferentiablePtrType), and IR lowering would later
        // query an IDifferentiablePtrType witness table with IDifferentiable's value-pair
        // requirement keys.
        auto makeDiffPairType = [&](Type* baseType, SubtypeWitness* baseWitness) -> Type*
        {
            if (baseWitness->getSup() == differentiableInterfaceType)
                return astBuilder->getDifferentialPairType(baseType, baseWitness);
            if (baseWitness->getSup() == differentiablePtrInterfaceType)
                return astBuilder->getDifferentialPtrPairType(baseType, baseWitness);

            SLANG_UNEXPECTED("unsupported diff witness for higher-order diff pair type");
            UNREACHABLE_RETURN(baseType);
        };

        auto makeDiffPairWitness = [&](Type* baseType,
                                       SubtypeWitness* baseWitness) -> SubtypeWitness*
        {
            GenericDecl* pairGenericDecl = nullptr;
            InheritanceDecl* pairInheritanceDecl = nullptr;
            Type* pairInterfaceType = nullptr;
            if (baseWitness->getSup() == differentiableInterfaceType)
            {
                pairGenericDecl = diffPairGenericDecl;
                pairInheritanceDecl = diffInheritanceDecl;
                pairInterfaceType = differentiableInterfaceType;
            }
            else if (baseWitness->getSup() == differentiablePtrInterfaceType)
            {
                ensureDiffPtrPairDecls();
                pairGenericDecl = diffPtrPairGenericDecl;
                pairInheritanceDecl = diffPtrInheritanceDecl;
                pairInterfaceType = differentiablePtrInterfaceType;
            }
            else
            {
                SLANG_UNEXPECTED("unsupported diff witness for higher-order diff pair witness");
                UNREACHABLE_RETURN(baseWitness);
            }

            Val* args[] = {baseType, baseWitness};
            auto diffPairDeclRef =
                astBuilder->getGenericAppDeclRef(pairGenericDecl, makeArrayView(args));

            auto inheritanceDeclRef =
                astBuilder->getMemberDeclRef(diffPairDeclRef, pairInheritanceDecl);

            return astBuilder->getDeclaredSubtypeWitness(
                makeDiffPairType(baseType, baseWitness),
                pairInterfaceType,
                inheritanceDeclRef);
        };

        Type* thisParamType = diffTypeInfoWitness->getThisParamType();
        auto thisDiffWitness = diffTypeInfoWitness->getThisTypeDiffWitness();


        if (thisParamType && thisDiffWitness)
        {
            auto originalThisParamType = thisParamType;
            auto originalThisDiffWitness = thisDiffWitness;
            thisParamType = makeDiffPairType(originalThisParamType, originalThisDiffWitness);
            thisDiffWitness = makeDiffPairWitness(originalThisParamType, originalThisDiffWitness);
        }

        SubtypeWitness* resultDiffWitness = diffTypeInfoWitness->getReturnTypeDiffWitness();
        if (resultDiffWitness)
            resultDiffWitness = makeDiffPairWitness(resultDiffWitness->getSub(), resultDiffWitness);

        List<SubtypeWitness*> pairDiffWitnesses;
        for (UIndex ii = 0; ii < diffTypeInfoWitness->getParamTypeCount(); ii++)
        {
            auto diffWitness = diffTypeInfoWitness->getParamTypeDiffWitness(ii);
            if (diffWitness)
                pairDiffWitnesses.add(makeDiffPairWitness(diffWitness->getSub(), diffWitness));
            else
                pairDiffWitnesses.add(diffWitness);
        }

        return getCurrentASTBuilder()->getOrCreate<DiffTypeInfoWitness>(
            thisParamType,
            thisDiffWitness,
            resultDiffWitness,
            pairDiffWitnesses);
    }

    return getCurrentASTBuilder()->getOrCreate<HigherOrderDiffTypeTranslationWitness>(
        resolvedWitness);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NoneWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NoneWitness::_toTextOverride(StringBuilder& out)
{
    out.append("none");
}

Val* NoneWitness::_resolveImplOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! HasDiffTypeInfoWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void HasDiffTypeInfoWitness::_toTextOverride(StringBuilder& out)
{
    out << "has_diff_type_info_witness(" << getDeclRef() << ")";
}

Val* HasDiffTypeInfoWitness::_resolveImplOverride()
{
    return this;
}

Val* HasDiffTypeInfoWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    auto constraintDeclRef = getDeclRef();
    auto genericDecl = as<GenericDecl>(constraintDeclRef.getDecl()->parentDecl);
    if (genericDecl)
    {
        auto args = tryGetGenericArguments(subst, genericDecl);
        if (args.getCount())
        {
            bool found = false;
            Index index = 0;
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (isGenericConstraintParameterDecl(member))
                {
                    if (member == constraintDeclRef.getDecl())
                    {
                        found = true;
                        break;
                    }
                    index++;
                }
            }

            if (found)
            {
                auto ordinaryParamCount =
                    genericDecl->getMembersOfType<GenericTypeParamDeclBase>().getCount() +
                    genericDecl->getMembersOfType<GenericValueParamDecl>().getCount() +
                    genericDecl->getMembersOfType<GenericValuePackParamDecl>().getCount();
                if (index + ordinaryParamCount < args.getCount())
                {
                    (*ioDiff)++;
                    return args[index + ordinaryParamCount];
                }
            }
        }
    }

    int diff = 0;
    auto substDeclRef = constraintDeclRef.substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;

    (*ioDiff)++;
    auto substConstraintDeclRef = substDeclRef.as<HasDiffTypeInfoConstraintDecl>();
    if (!substConstraintDeclRef)
        return this;
    return astBuilder->getHasDiffTypeInfoWitness(substConstraintDeclRef);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclaredVariadicPackCountWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!

void DeclaredVariadicPackCountWitness::_toTextOverride(StringBuilder& out)
{
    out << "declared_pack_count_witness(" << getDeclRef() << ")";
}

Val* DeclaredVariadicPackCountWitness::_resolveImplOverride()
{
    auto resolvedDeclRef = getDeclRef().declRefBase->resolve();
    if (auto resolvedVal = as<Witness>(resolvedDeclRef))
        return resolvedVal;

    auto newDeclRef = as<DeclRefBase>(resolvedDeclRef);
    if (!newDeclRef)
        newDeclRef = getDeclRef().declRefBase;
    if (newDeclRef != getDeclRef().declRefBase)
    {
        auto newPackCountDeclRef =
            DeclRef<Decl>(newDeclRef).as<GenericVariadicPackCountConstraintDecl>();
        if (newPackCountDeclRef)
            return getCurrentASTBuilder()->getDeclaredVariadicPackCountWitness(newPackCountDeclRef);
    }
    return this;
}

Val* DeclaredVariadicPackCountWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    auto constraintDeclRef = getDeclRef();
    auto genericDecl = as<GenericDecl>(constraintDeclRef.getDecl()->parentDecl);
    if (genericDecl)
    {
        auto args = tryGetGenericArguments(subst, genericDecl);
        if (args.getCount())
        {
            // Generic application arguments store ordinary parameters first and
            // then one witness argument for each source generic constraint, in
            // the same member order used by `getDefaultSubstitutionArgs` and
            // `_lowerSubstitutionEnv`. When substituting a declared pack-count
            // witness through `foo<5, int, ...>`, return that hidden witness
            // argument directly if it is present in the substitution.
            bool found = false;
            Index index = 0;
            for (auto member : genericDecl->getDirectMemberDecls())
            {
                if (isGenericConstraintParameterDecl(member))
                {
                    if (member == constraintDeclRef.getDecl())
                    {
                        found = true;
                        break;
                    }
                    index++;
                }
            }

            if (found)
            {
                auto ordinaryParamCount =
                    genericDecl->getMembersOfType<GenericTypeParamDeclBase>().getCount() +
                    genericDecl->getMembersOfType<GenericValueParamDecl>().getCount() +
                    genericDecl->getMembersOfType<GenericValuePackParamDecl>().getCount();
                if (index + ordinaryParamCount < args.getCount())
                {
                    (*ioDiff)++;
                    return args[index + ordinaryParamCount];
                }
            }
        }
    }

    int diff = 0;
    auto substDeclRef = constraintDeclRef.substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;

    auto substConstraintDeclRef = substDeclRef.as<GenericVariadicPackCountConstraintDecl>();
    if (!substConstraintDeclRef)
        return this;

    (*ioDiff)++;
    return astBuilder->getDeclaredVariadicPackCountWitness(substConstraintDeclRef);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ConcreteVariadicPackCountWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!

void ConcreteVariadicPackCountWitness::_toTextOverride(StringBuilder& out)
{
    out.append("pack_count_witness(");
    if (auto actualCount = getActualCount())
        actualCount->toText(out);
    else
        out.append("<null>");
    out.append(", ");
    if (auto count = getExpectedCount())
        count->toText(out);
    else
        out.append("<null>");
    out.append(")");
}

Val* ConcreteVariadicPackCountWitness::_resolveImplOverride()
{
    auto resolvedActualCount = getActualCount() ? as<IntVal>(getActualCount()->resolve()) : nullptr;
    auto resolvedExpectedCount =
        getExpectedCount() ? as<IntVal>(getExpectedCount()->resolve()) : nullptr;
    if (resolvedActualCount != getActualCount() || resolvedExpectedCount != getExpectedCount())
        return getCurrentASTBuilder()->getConcreteVariadicPackCountWitness(
            resolvedActualCount,
            resolvedExpectedCount);
    return this;
}

Val* ConcreteVariadicPackCountWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    // Concrete witnesses are proof-only values, but both count operands still
    // participate in specialization. Substituting them keeps serialized/lowered
    // witnesses tied to the exact specialized `countof(pack) == count` fact
    // that the checker validated.
    int diff = 0;
    auto substActualCount =
        getActualCount() ? as<IntVal>(getActualCount()->substituteImpl(astBuilder, subst, &diff))
                         : nullptr;
    auto substExpectedCount =
        getExpectedCount()
            ? as<IntVal>(getExpectedCount()->substituteImpl(astBuilder, subst, &diff))
            : nullptr;
    if (!diff)
        return this;

    (*ioDiff)++;
    return astBuilder->getConcreteVariadicPackCountWitness(substActualCount, substExpectedCount);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NonEmptyPackWitness !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NonEmptyPackWitness::_toTextOverride(StringBuilder& out)
{
    out.append("nonempty_witness(");
    if (auto pack = getPack())
        pack->toText(out);
    else
        out.append("<null>");
    out.append(")");
}

Val* NonEmptyPackWitness::_resolveImplOverride()
{
    auto resolvedPack = getPack() ? getPack()->resolve() : nullptr;
    if (resolvedPack != getPack())
        return getCurrentASTBuilder()->getNonEmptyPackWitness(resolvedPack);
    return this;
}

Val* NonEmptyPackWitness::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substPack = getPack() ? getPack()->substituteImpl(astBuilder, subst, &diff) : nullptr;
    if (!diff)
        return this;

    (*ioDiff)++;
    return astBuilder->getNonEmptyPackWitness(substPack);
}

// UNormModifierVal

void UNormModifierVal::_toTextOverride(StringBuilder& out)
{
    out.append("unorm");
}

Val* UNormModifierVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
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

Val* SNormModifierVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

// NoDiffModifierVal
void NoDiffModifierVal::_toTextOverride(StringBuilder& out)
{
    out.append("no_diff");
}

Val* NoDiffModifierVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

// PolynomialIntVal

void PolynomialIntVal::_toTextOverride(StringBuilder& out)
{
    auto constantTerm = getConstantTerm();
    auto terms = getTerms();
    for (Index i = 0; i < terms.getCount(); i++)
    {
        auto& term = *(terms[i]);
        if (term.getConstFactor() > 0)
        {
            if (i > 0)
                out << "+";
        }
        else
            out << "-";
        bool isFirstFactor = true;
        if (abs(term.getConstFactor()) != 1 || term.getParamFactors().getCount() == 0)
        {
            out << abs(term.getConstFactor());
            isFirstFactor = false;
        }
        for (Index j = 0; j < term.getParamFactors().getCount(); j++)
        {
            auto factor = term.getParamFactors()[j];
            if (isFirstFactor)
            {
                isFirstFactor = false;
            }
            else
            {
                out << "*";
            }
            factor->getParam()->toText(out);
            if (factor->getPower() != 1)
            {
                out << "^^" << factor->getPower();
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

struct PolynomialIntValBuilder
{
    ASTBuilder* astBuilder;

    IntegerLiteralValue constantTerm = 0;
    List<PolynomialIntValTerm*> terms;

    PolynomialIntValBuilder(ASTBuilder* inAstBuilder)
        : astBuilder(inAstBuilder)
    {
    }

    // compute val += opreand*multiplier;
    bool addToPolynomialTerm(IntVal* operand, IntegerLiteralValue multiplier)
    {
        if (auto c = as<ConstantIntVal>(operand))
        {
            constantTerm += c->getValue() * multiplier;
            return true;
        }
        else if (auto poly = as<PolynomialIntVal>(operand))
        {
            constantTerm += poly->getConstantTerm() * multiplier;
            for (auto term : poly->getTerms())
            {
                auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                    multiplier * term->getConstFactor(),
                    term->getParamFactors());
                terms.add(newTerm);
            }
            return true;
        }
        else if (auto genVal = as<IntVal>(operand))
        {
            auto factor = astBuilder->getOrCreate<PolynomialIntValFactor>(genVal, 1);
            auto term = astBuilder->getOrCreate<PolynomialIntValTerm>(
                multiplier,
                makeArrayViewSingle(factor));
            terms.add(term);
            return true;
        }
        return false;
    }

    IntVal* canonicalize(Type* type)
    {
        List<PolynomialIntValTerm*> newTerms;
        IntegerLiteralValue newConstantTerm = constantTerm;
        auto addTerm = [&](PolynomialIntValTerm* newTerm)
        {
            for (auto& term : newTerms)
            {
                if (term->canCombineWith(*newTerm))
                {
                    term = astBuilder->getOrCreate<PolynomialIntValTerm>(
                        term->getConstFactor() + newTerm->getConstFactor(),
                        term->getParamFactors());
                    return;
                }
            }
            newTerms.add(newTerm);
        };
        for (auto term : terms)
        {
            if (term->getConstFactor() == 0)
                continue;
            List<PolynomialIntValFactor*> newFactors;
            List<bool> factorIsDifferent;
            for (Index i = 0; i < term->getParamFactors().getCount(); i++)
            {
                auto factor = term->getParamFactors()[i];
                bool factorFound = false;
                for (Index j = 0; j < newFactors.getCount(); j++)
                {
                    auto& newFactor = newFactors[j];
                    if (factor->getParam()->equals(newFactor->getParam()))
                    {
                        if (!factorIsDifferent[j])
                        {
                            factorIsDifferent[j] = true;
                            auto clonedFactor = astBuilder->getOrCreate<PolynomialIntValFactor>(
                                newFactor->getParam(),
                                newFactor->getPower());
                            newFactor = clonedFactor;
                        }
                        newFactor = astBuilder->getOrCreate<PolynomialIntValFactor>(
                            newFactor->getParam(),
                            newFactor->getPower() + factor->getPower());
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
            // Remove zero-powered factors.
            for (auto factor : newFactors)
            {
                if (factor->getPower() != 0)
                    newFactors2.add(factor);
            }
            if (newFactors2.getCount() == 0)
            {
                newConstantTerm += term->getConstFactor();
                continue;
            }
            newFactors2.sort([](PolynomialIntValFactor* t1, PolynomialIntValFactor* t2)
                             { return *t1 < *t2; });
            bool isDifferent = false;
            if (newFactors2.getCount() != term->getParamFactors().getCount())
                isDifferent = true;
            if (!isDifferent)
            {
                for (Index i = 0; i < term->getParamFactors().getCount(); i++)
                    if (term->getParamFactors()[i] != newFactors2[i])
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
                auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                    term->getConstFactor(),
                    newFactors2.getArrayView());
                addTerm(newTerm);
            }
        }
        List<PolynomialIntValTerm*> newTerms2;
        for (auto term : newTerms)
        {
            if (term->getConstFactor() == 0)
                continue;
            newTerms2.add(term);
        }
        newTerms2.sort([](PolynomialIntValTerm* t1, PolynomialIntValTerm* t2)
                       { return *t1 < *t2; });
        terms = _Move(newTerms2);
        constantTerm = newConstantTerm;
        if (terms.getCount() == 1 && constantTerm == 0 && terms[0]->getConstFactor() == 1 &&
            terms[0]->getParamFactors().getCount() == 1 &&
            terms[0]->getParamFactors()[0]->getPower() == 1)
        {
            return terms[0]->getParamFactors()[0]->getParam();
        }
        if (terms.getCount() == 0)
            return astBuilder->getIntVal(type, constantTerm);
        return nullptr;
    }

    IntVal* getIntVal(Type* type)
    {
        if (auto canVal = canonicalize(type))
            return canVal;
        return astBuilder->getOrCreate<PolynomialIntVal>(type, constantTerm, terms.getArrayView());
    }
};

Val* PolynomialIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    PolynomialIntValBuilder builder(astBuilder);
    builder.constantTerm = getConstantTerm();
    for (auto& term : getTerms())
    {
        IntegerLiteralValue evaluatedTermConstFactor;
        List<PolynomialIntValFactor*> evaluatedTermParamFactors;
        evaluatedTermConstFactor = term->getConstFactor();
        for (auto& factor : term->getParamFactors())
        {
            auto substResult = factor->getParam()->substituteImpl(astBuilder, subst, &diff);

            if (auto constantVal = as<ConstantIntVal>(substResult))
            {
                auto power = factor->getPower();
                for (IntegerLiteralValue i = 0; i < power; i++)
                    evaluatedTermConstFactor *= constantVal->getValue();
            }
            else if (auto intResult = as<IntVal>(substResult))
            {
                auto newFactor =
                    astBuilder->getOrCreate<PolynomialIntValFactor>(intResult, factor->getPower());
                evaluatedTermParamFactors.add(newFactor);
            }
        }
        if (evaluatedTermParamFactors.getCount() == 0)
        {
            builder.constantTerm += evaluatedTermConstFactor;
        }
        else
        {
            if (evaluatedTermParamFactors.getCount() == 1 &&
                evaluatedTermParamFactors[0]->getPower() == 1)
            {
                if (auto polyTerm = as<PolynomialIntVal>(evaluatedTermParamFactors[0]->getParam()))
                {
                    builder.addToPolynomialTerm(polyTerm, evaluatedTermConstFactor);
                    continue;
                }
            }
            auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                evaluatedTermConstFactor,
                evaluatedTermParamFactors.getArrayView());
            builder.terms.add(newTerm);
        }
    }

    *ioDiff += diff;

    if (builder.terms.getCount() == 0)
        return astBuilder->getIntVal(getType(), builder.constantTerm);
    if (diff != 0)
    {
        return builder.getIntVal(getType());
    }
    return this;
}

IntVal* PolynomialIntVal::neg(ASTBuilder* astBuilder, IntVal* base)
{
    PolynomialIntValBuilder builder(astBuilder);
    builder.addToPolynomialTerm(base, -1);
    return builder.getIntVal(base->getType());
}

IntVal* PolynomialIntVal::sub(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1)
{
    PolynomialIntValBuilder builder(astBuilder);
    builder.addToPolynomialTerm(op0, 1);
    builder.addToPolynomialTerm(op1, -1);
    return builder.getIntVal(op0->getType());
}

IntVal* PolynomialIntVal::add(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1)
{
    PolynomialIntValBuilder builder(astBuilder);
    builder.addToPolynomialTerm(op0, 1);
    builder.addToPolynomialTerm(op1, 1);
    return builder.getIntVal(op0->getType());
}

IntVal* PolynomialIntVal::mul(ASTBuilder* astBuilder, IntVal* op0, IntVal* op1)
{
    if (auto poly0 = as<PolynomialIntVal>(op0))
    {
        if (auto poly1 = as<PolynomialIntVal>(op1))
        {
            PolynomialIntValBuilder builder(astBuilder);
            // add poly0.constant * poly1.constant
            builder.constantTerm = poly0->getConstantTerm() * poly1->getConstantTerm();
            // add poly0.constant * poly1.terms
            if (poly0->getConstantTerm() != 0)
            {
                for (auto term : poly1->getTerms())
                {
                    auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                        poly0->getConstantTerm() * term->getConstFactor(),
                        term->getParamFactors());
                    builder.terms.add(newTerm);
                }
            }
            // add poly1.constant * poly0.terms
            if (poly1->getConstantTerm() != 0)
            {
                for (auto term : poly0->getTerms())
                {
                    auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                        poly1->getConstantTerm() * term->getConstFactor(),
                        term->getParamFactors());
                    builder.terms.add(newTerm);
                }
            }
            // add poly1.terms * poly0.terms
            for (auto term0 : poly0->getTerms())
            {
                for (auto term1 : poly1->getTerms())
                {
                    List<PolynomialIntValFactor*> newFactors;
                    for (auto f : term0->getParamFactors())
                        newFactors.add(f);
                    for (auto f : term1->getParamFactors())
                        newFactors.add(f);
                    auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                        term0->getConstFactor() * term1->getConstFactor(),
                        newFactors.getArrayView());
                    builder.terms.add(newTerm);
                }
            }
            return builder.getIntVal(op0->getType());
        }
        else if (auto cVal1 = as<ConstantIntVal>(op1))
        {
            PolynomialIntValBuilder builder(astBuilder);
            builder.constantTerm = poly0->getConstantTerm() * cVal1->getValue();
            for (auto term : poly0->getTerms())
            {
                auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                    term->getConstFactor() * cVal1->getValue(),
                    term->getParamFactors());
                builder.terms.add(newTerm);
            }
            return builder.getIntVal(poly0->getType());
        }
        else if (auto val1 = as<IntVal>(op1))
        {
            PolynomialIntValBuilder builder(astBuilder);
            auto factor1 = astBuilder->getOrCreate<PolynomialIntValFactor>(val1, 1);
            if (poly0->getConstantTerm() != 0)
            {
                auto term0 = astBuilder->getOrCreate<PolynomialIntValTerm>(
                    poly0->getConstantTerm(),
                    makeArrayViewSingle(factor1));
                builder.terms.add(term0);
            }
            for (auto term : poly0->getTerms())
            {
                List<PolynomialIntValFactor*> newFactors;
                for (auto f : term->getParamFactors())
                    newFactors.add(f);
                newFactors.add(factor1);
                auto newTerm = astBuilder->getOrCreate<PolynomialIntValTerm>(
                    term->getConstFactor(),
                    newFactors.getArrayView());
                builder.terms.add(newTerm);
            }
            return builder.getIntVal(poly0->getType());
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
        if (const auto poly1 = as<PolynomialIntVal>(op1); poly1)
        {
            return mul(astBuilder, op1, op0);
        }
        else if (auto cVal1 = as<ConstantIntVal>(op1))
        {
            PolynomialIntValBuilder builder(astBuilder);
            auto factor0 = astBuilder->getOrCreate<PolynomialIntValFactor>(val0, 1);
            auto term = astBuilder->getOrCreate<PolynomialIntValTerm>(
                cVal1->getValue(),
                makeArrayView(&factor0, 1));
            builder.terms.add(term);
            return builder.getIntVal(val0->getType());
        }
        else if (auto val1 = as<IntVal>(op1))
        {
            PolynomialIntValBuilder builder(astBuilder);
            auto factor0 = astBuilder->getOrCreate<PolynomialIntValFactor>(val0, 1);
            auto factor1 = astBuilder->getOrCreate<PolynomialIntValFactor>(val1, 1);
            PolynomialIntValFactor* newFactors[] = {factor0, factor1};
            auto term = astBuilder->getOrCreate<PolynomialIntValTerm>(1, makeArrayView(newFactors));
            builder.terms.add(term);
            return builder.getIntVal(val0->getType());
        }
    }
    return nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TypeCastIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TypeCastIntVal::_toTextOverride(StringBuilder& out)
{
    getType()->toText(out);
    out << "(";
    getBase()->toText(out);
    out << ")";
}

Val* TypeCastIntVal::tryFoldImpl(
    ASTBuilder* astBuilder,
    Type* resultType,
    Val* base,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    auto convertValue = [&](BasicExpressionType* baseType, IntegerLiteralValue& resultValue) -> bool
    {
        switch (baseType->getBaseType())
        {
        case BaseType::Int:
            resultValue = (int)resultValue;
            return true;
        case BaseType::UInt:
            resultValue = (unsigned int)resultValue;
            return true;
        case BaseType::Int64:
        case BaseType::IntPtr:
            resultValue = (Int64)resultValue;
            return true;
        case BaseType::UInt64:
        case BaseType::UIntPtr:
            resultValue = (UInt64)resultValue;
            return true;
        case BaseType::Int16:
            resultValue = (int16_t)resultValue;
            return true;
        case BaseType::UInt16:
            resultValue = (uint16_t)resultValue;
            return true;
        case BaseType::Int8:
            resultValue = (int8_t)resultValue;
            return true;
        case BaseType::UInt8:
            resultValue = (uint8_t)resultValue;
            return true;
        default:
            return false;
        }
    };
    if (auto c = as<ConstantIntVal>(base))
    {
        IntegerLiteralValue resultValue = c->getValue();
        auto baseType = as<BasicExpressionType>(resultType);
        if (baseType)
        {
            if (!convertValue(baseType, resultValue))
                return nullptr;
        }
        else if (auto enumDecl = isEnumType(resultType))
        {
            baseType = as<BasicExpressionType>(enumDecl->tagType);
            if (!baseType || !convertValue(baseType, resultValue))
                return nullptr;
        }
        return astBuilder->getIntVal(resultType, resultValue);
    }
    return nullptr;
}

Val* TypeCastIntVal::_linkTimeResolveOverride(Dictionary<String, IntVal*>& map)
{
    auto intValBase = as<IntVal>(getBase());
    if (!intValBase)
        return this;
    auto resolvedBase = intValBase->linkTimeResolve(map);
    return tryFoldImpl(getCurrentASTBuilder(), getType(), resolvedBase, nullptr);
}

Val* TypeCastIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substBase = getBase()->substituteImpl(astBuilder, subst, &diff);
    if (substBase != getBase())
        diff++;
    auto substType = as<Type>(getType()->substituteImpl(astBuilder, subst, &diff));
    if (substType != getType())
        diff++;
    *ioDiff += diff;
    if (diff)
    {
        auto newVal = tryFoldImpl(astBuilder, substType, substBase, nullptr);
        if (newVal)
            return newVal;
        else
        {
            auto result = astBuilder->getTypeCastIntVal(substType, substBase);
            return result;
        }
    }
    // Nothing found: don't substitute.
    return this;
}

Val* TypeCastIntVal::_resolveImplOverride()
{
    if (auto resolved = tryFoldImpl(getCurrentASTBuilder(), getType(), getBase(), nullptr))
        return resolved;
    return this;
}

// Fold a constant integer shift. Negative counts leave the value symbolic (returns false);
// out-of-range counts are masked to the operand width, matching common hardware behavior and
// avoiding C++ undefined behavior. Shared by every constant-folding path so `x << y` produces
// the same constant regardless of which path reaches it.
static bool _tryFoldConstantShift(
    IntegerLiteralValue base,
    IntegerLiteralValue count,
    bool isLeftShift,
    IntegerLiteralValue& out)
{
    if (count < 0)
        return false;
    const auto width = std::numeric_limits<std::make_unsigned_t<IRIntegerValue>>::digits;
    const auto shiftCount = static_cast<std::make_unsigned_t<IRIntegerValue>>(count) % width;
    out = isLeftShift
              ? static_cast<IntegerLiteralValue>(
                    static_cast<std::make_unsigned_t<IntegerLiteralValue>>(base) << shiftCount)
              : (base >> shiftCount);
    return true;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BuiltinOperationIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

UnownedStringSlice getBuiltinOperationOpText(BuiltinOperationKind op)
{
    switch (op)
    {
    case BuiltinOperationKind::Add:
        return toSlice("+");
    case BuiltinOperationKind::Sub:
        return toSlice("-");
    case BuiltinOperationKind::Mul:
        return toSlice("*");
    case BuiltinOperationKind::Div:
        return toSlice("/");
    case BuiltinOperationKind::Mod:
        return toSlice("%");
    case BuiltinOperationKind::Neg:
        return toSlice("-");
    case BuiltinOperationKind::Eql:
        return toSlice("==");
    case BuiltinOperationKind::Neq:
        return toSlice("!=");
    case BuiltinOperationKind::Less:
        return toSlice("<");
    case BuiltinOperationKind::Greater:
        return toSlice(">");
    case BuiltinOperationKind::Leq:
        return toSlice("<=");
    case BuiltinOperationKind::Geq:
        return toSlice(">=");
    case BuiltinOperationKind::BitAnd:
        return toSlice("&");
    case BuiltinOperationKind::BitOr:
        return toSlice("|");
    case BuiltinOperationKind::BitXor:
        return toSlice("^");
    case BuiltinOperationKind::BitNot:
        return toSlice("~");
    case BuiltinOperationKind::Lsh:
        return toSlice("<<");
    case BuiltinOperationKind::Rsh:
        return toSlice(">>");
    case BuiltinOperationKind::Not:
        return toSlice("!");
    case BuiltinOperationKind::Conditional:
        return toSlice("?:");
    case BuiltinOperationKind::And:
        return toSlice("&&");
    case BuiltinOperationKind::Or:
        return toSlice("||");
    case BuiltinOperationKind::Unknown:
        break;
    }
    // Every real `BuiltinOperationKind` must have op text (it feeds `toText` and mangling); a
    // missing case (or the `Unknown` sentinel reaching here) is a bug, not a silently-"?"
    // operator.
    SLANG_UNEXPECTED("unhandled BuiltinOperationKind in getBuiltinOperationOpText");
    UNREACHABLE_RETURN(toSlice("?"));
}

BuiltinOperationKind getBuiltinOperationKindFromString(
    UnownedStringSlice opText,
    OperatorArity arity)
{
    const bool isUnary = (arity == OperatorArity::Unary);
    if (opText == toSlice("+"))
        return BuiltinOperationKind::Add;
    if (opText == toSlice("-"))
        return isUnary ? BuiltinOperationKind::Neg : BuiltinOperationKind::Sub;
    if (opText == toSlice("*"))
        return BuiltinOperationKind::Mul;
    if (opText == toSlice("/"))
        return BuiltinOperationKind::Div;
    if (opText == toSlice("%"))
        return BuiltinOperationKind::Mod;
    if (opText == toSlice("~"))
        return BuiltinOperationKind::BitNot;
    if (opText == toSlice("!"))
        return BuiltinOperationKind::Not;
    if (opText == toSlice("=="))
        return BuiltinOperationKind::Eql;
    if (opText == toSlice("!="))
        return BuiltinOperationKind::Neq;
    if (opText == toSlice("<"))
        return BuiltinOperationKind::Less;
    if (opText == toSlice(">"))
        return BuiltinOperationKind::Greater;
    if (opText == toSlice("<="))
        return BuiltinOperationKind::Leq;
    if (opText == toSlice(">="))
        return BuiltinOperationKind::Geq;
    if (opText == toSlice("&"))
        return BuiltinOperationKind::BitAnd;
    if (opText == toSlice("|"))
        return BuiltinOperationKind::BitOr;
    if (opText == toSlice("^"))
        return BuiltinOperationKind::BitXor;
    if (opText == toSlice("<<"))
        return BuiltinOperationKind::Lsh;
    if (opText == toSlice(">>"))
        return BuiltinOperationKind::Rsh;
    // `?:`/`&&`/`||` are not rewritten by the fast path (`?:` is not an infix operator and
    // `&&`/`||` short-circuit), but a *resolved* operator call on them can still fold to a
    // constant `BuiltinOperationIntVal`; the fast path naturally ignores these kinds because
    // they are neither arithmetic, comparison, nor bitwise.
    if (opText == toSlice("?:"))
        return BuiltinOperationKind::Conditional;
    if (opText == toSlice("&&"))
        return BuiltinOperationKind::And;
    if (opText == toSlice("||"))
        return BuiltinOperationKind::Or;
    return BuiltinOperationKind::Unknown;
}

void BuiltinOperationIntVal::_toTextOverride(StringBuilder& out)
{
    auto args = getArgs();
    auto argToText = [&](Index index)
    {
        if (as<PolynomialIntVal>(args[index]) || as<BuiltinOperationIntVal>(args[index]))
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
    if (getOp() == BuiltinOperationKind::Conditional && args.getCount() == 3)
    {
        argToText(0);
        out << "?";
        argToText(1);
        out << ":";
        argToText(2);
        return;
    }
    auto opText = getBuiltinOperationOpText(getOp());
    if (args.getCount() == 2)
    {
        argToText(0);
        out << opText;
        argToText(1);
    }
    else if (args.getCount() == 1)
    {
        out << opText;
        argToText(0);
    }
}

Val* BuiltinOperationIntVal::tryFoldImpl(
    ASTBuilder* astBuilder,
    Type* resultType,
    BuiltinOperationKind op,
    List<IntVal*>& newArgs,
    DiagnosticSink* sink,
    SourceLoc loc)
{
    List<ConstantIntVal*> constArgs;
    for (auto arg : newArgs)
    {
        auto c = as<ConstantIntVal>(arg);
        if (!c)
            return nullptr; // still symbolic
        constArgs.add(c);
    }

    // Reject a malformed node whose argument count does not match the operation's arity, so a
    // unary op never reads a missing second operand and a binary op never folds with a
    // defaulted one.
    const bool isUnary =
        (op == BuiltinOperationKind::Neg || op == BuiltinOperationKind::BitNot ||
         op == BuiltinOperationKind::Not);
    const Index expectedArgs = (op == BuiltinOperationKind::Conditional) ? 3 : (isUnary ? 1 : 2);
    if (constArgs.getCount() != expectedArgs)
        return nullptr;

    const IntegerLiteralValue a0 = constArgs[0]->getValue();
    const IntegerLiteralValue a1 = (constArgs.getCount() > 1) ? constArgs[1]->getValue() : 0;
    const IntegerLiteralValue a2 = (constArgs.getCount() > 2) ? constArgs[2]->getValue() : 0;
    // Do the wrapping arithmetic (negate/add/sub/mul) through the unsigned type: signed
    // overflow is UB (and `-INT64_MIN` / `INT64_MIN / -1` even trap on real hardware), while
    // unsigned wraps two's-complement, matching what the target IR ops compute at runtime.
    using UInt = std::make_unsigned_t<IntegerLiteralValue>;
    const UInt u0 = (UInt)a0;
    const UInt u1 = (UInt)a1;
    IntegerLiteralValue r = 0;
    switch (op)
    {
    case BuiltinOperationKind::Neg:
        r = (IntegerLiteralValue)(UInt(0) - u0);
        break;
    case BuiltinOperationKind::BitNot:
        r = ~a0;
        break;
    case BuiltinOperationKind::Not:
        r = (a0 == 0);
        break;
    case BuiltinOperationKind::Eql:
        r = (a0 == a1);
        break;
    case BuiltinOperationKind::Neq:
        r = (a0 != a1);
        break;
    case BuiltinOperationKind::Less:
        r = (a0 < a1);
        break;
    case BuiltinOperationKind::Greater:
        r = (a0 > a1);
        break;
    case BuiltinOperationKind::Leq:
        r = (a0 <= a1);
        break;
    case BuiltinOperationKind::Geq:
        r = (a0 >= a1);
        break;
    case BuiltinOperationKind::BitAnd:
        r = a0 & a1;
        break;
    case BuiltinOperationKind::BitOr:
        r = a0 | a1;
        break;
    case BuiltinOperationKind::BitXor:
        r = a0 ^ a1;
        break;
    case BuiltinOperationKind::Add:
        r = (IntegerLiteralValue)(u0 + u1);
        break;
    case BuiltinOperationKind::Sub:
        r = (IntegerLiteralValue)(u0 - u1);
        break;
    case BuiltinOperationKind::Mul:
        r = (IntegerLiteralValue)(u0 * u1);
        break;
    case BuiltinOperationKind::Div:
    case BuiltinOperationKind::Mod:
        if (a1 == 0)
        {
            if (sink)
                sink->diagnose(Diagnostics::DivideByZero{.location = loc});
            return nullptr;
        }
        // `INT64_MIN / -1` overflows and traps (SIGFPE) on real hardware, so handle the
        // `/ -1` case without dividing: `x / -1 == -x` (wrapping) and `x % -1 == 0`.
        if (a1 == -1)
            r = (op == BuiltinOperationKind::Div) ? (IntegerLiteralValue)(UInt(0) - u0) : 0;
        else
            r = (op == BuiltinOperationKind::Div) ? (a0 / a1) : (a0 % a1);
        break;
    case BuiltinOperationKind::Lsh:
    case BuiltinOperationKind::Rsh:
        if (!_tryFoldConstantShift(a0, a1, /*isLeftShift*/ op == BuiltinOperationKind::Lsh, r))
            return nullptr;
        break;
    case BuiltinOperationKind::And:
        r = ((a0 != 0) && (a1 != 0)) ? 1 : 0;
        break;
    case BuiltinOperationKind::Or:
        r = ((a0 != 0) || (a1 != 0)) ? 1 : 0;
        break;
    case BuiltinOperationKind::Conditional:
        r = (a0 != 0) ? a1 : a2;
        break;
    default:
        return nullptr;
    }
    return astBuilder->getIntVal(resultType, r);
}

Val* BuiltinOperationIntVal::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();
    bool diff = false;
    List<IntVal*> newArgs;
    for (auto arg : getArgs())
    {
        auto newArg = as<IntVal>(arg->resolve());
        if (!newArg)
            return this;
        newArgs.add(newArg);
        if (newArg != arg)
            diff = true;
    }
    if (auto resolved = tryFoldImpl(astBuilder, getType(), getOp(), newArgs, nullptr))
        return resolved;
    if (diff)
        return astBuilder->getOrCreate<BuiltinOperationIntVal>(
            getType(),
            getOp(),
            newArgs.getArrayView());
    return this;
}

Val* BuiltinOperationIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    List<IntVal*> newArgs;
    for (auto& arg : getArgs())
    {
        auto substArg = arg->substituteImpl(astBuilder, subst, &diff);
        if (substArg != arg)
            diff++;
        newArgs.add(as<IntVal>(substArg));
    }
    *ioDiff += diff;
    if (diff)
    {
        if (auto newVal = tryFoldImpl(astBuilder, getType(), getOp(), newArgs, nullptr))
            return newVal;
        return astBuilder->getOrCreate<BuiltinOperationIntVal>(
            getType(),
            getOp(),
            newArgs.getArrayView());
    }
    return this;
}

Val* BuiltinOperationIntVal::_linkTimeResolveOverride(Dictionary<String, IntVal*>& map)
{
    List<IntVal*> newArgs;
    for (auto arg : getArgs())
        newArgs.add(as<IntVal>(arg->linkTimeResolve(map)));
    return tryFoldImpl(getCurrentASTBuilder(), getType(), getOp(), newArgs, nullptr);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SizeOfIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void SizeOfIntVal::_toTextOverride(StringBuilder& out)
{
    out << "sizeof(";
    getValArg()->toText(out);
    out << ")";
}

Val* SizeOfIntVal::tryFoldOrNull(ASTBuilder* astBuilder, Type* intType, Type* newType)
{
    ASTNaturalLayoutContext context(astBuilder, nullptr);
    const auto size = context.calcSize(newType);

    if (!size)
        return nullptr;

    return astBuilder->getIntVal(intType, size.size);
}

Val* SizeOfIntVal::tryFold(ASTBuilder* astBuilder, Type* intType, Type* newType)
{
    if (auto result = tryFoldOrNull(astBuilder, intType, newType))
        return result;
    auto result = astBuilder->getOrCreate<SizeOfIntVal>(intType, newType);
    return result;
}

Val* SizeOfIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newType = as<Type>(getValArg()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;

    (*ioDiff)++;
    return tryFold(astBuilder, getType(), newType);
}

Val* SizeOfIntVal::_resolveImplOverride()
{
    auto resolvedArg = getValArg()->resolve();
    if (resolvedArg == getValArg())
        return this;
    return tryFold(getCurrentASTBuilder(), getType(), as<Type>(resolvedArg));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AlignOfIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void AlignOfIntVal::_toTextOverride(StringBuilder& out)
{
    out << "alignof(";
    getValArg()->toText(out);
    out << ")";
}

Val* AlignOfIntVal::tryFoldOrNull(ASTBuilder* astBuilder, Type* intType, Type* newType)
{
    ASTNaturalLayoutContext context(astBuilder, nullptr);
    const auto size = context.calcSize(newType);

    if (!size)
        return nullptr;

    return astBuilder->getIntVal(intType, size.alignment);
}

Val* AlignOfIntVal::tryFold(ASTBuilder* astBuilder, Type* intType, Type* newType)
{
    if (auto result = tryFoldOrNull(astBuilder, intType, newType))
        return result;
    auto result = astBuilder->getOrCreate<AlignOfIntVal>(intType, newType);
    return result;
}

Val* AlignOfIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newType = as<Type>(getValArg()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;

    (*ioDiff)++;
    return tryFold(astBuilder, getType(), newType);
}

Val* AlignOfIntVal::_resolveImplOverride()
{
    auto resolvedArg = getValArg()->resolve();
    if (resolvedArg == getValArg())
        return this;
    return tryFold(getCurrentASTBuilder(), getType(), as<Type>(resolvedArg));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CountOfIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void CountOfIntVal::_toTextOverride(StringBuilder& out)
{
    out << "countof(";
    getValArg()->toText(out);
    out << ")";
}

Val* CountOfIntVal::tryFoldOrNull(ASTBuilder* astBuilder, Type* intType, Val* newVal)
{
    if (auto typePack = as<ConcreteTypePack>(newVal))
    {
        bool anyAbstract = false;
        for (Index i = 0; i < typePack->getTypeCount(); i++)
        {
            if (isAbstractTypePack(typePack->getElementType(i)))
            {
                anyAbstract = true;
                break;
            }
        }
        if (!anyAbstract)
        {
            return astBuilder->getIntVal(intType, typePack->getTypeCount());
        }
    }
    else if (auto tupleType = as<TupleType>(newVal))
    {
        return tryFoldOrNull(astBuilder, intType, tupleType->getTypePack());
    }
    else if (auto valPack = as<ConcreteIntValPack>(newVal))
    {
        return astBuilder->getIntVal(intType, valPack->getCount());
    }
    return nullptr;
}

Val* CountOfIntVal::tryFold(ASTBuilder* astBuilder, Type* intType, Val* newVal)
{
    if (auto result = tryFoldOrNull(astBuilder, intType, newVal))
        return result;
    return astBuilder->getOrCreate<CountOfIntVal>(intType, newVal);
}

Val* CountOfIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    if (!getValArg())
        return this;
    int diff = 0;
    auto newVal = getValArg()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;

    (*ioDiff)++;
    return tryFold(astBuilder, getType(), newVal);
}

Val* CountOfIntVal::_resolveImplOverride()
{
    if (!getValArg())
        return this;
    auto resolvedArg = getValArg()->resolve();
    if (resolvedArg == getValArg())
        return this;
    return tryFold(getCurrentASTBuilder(), getType(), resolvedArg);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FirstIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void FirstIntVal::_toTextOverride(StringBuilder& out)
{
    out << "__first(";
    getBasePack()->toText(out);
    out << ")";
}

Val* FirstIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substBase = getBasePack()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getFirstElement(substBase);
}

Val* FirstIntVal::_resolveImplOverride()
{
    auto resolvedArg = getBasePack()->resolve();
    if (resolvedArg == getBasePack())
        return this;
    return getCurrentASTBuilder()->getFirstElement(resolvedArg);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LastIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void LastIntVal::_toTextOverride(StringBuilder& out)
{
    out << "__last(";
    getBasePack()->toText(out);
    out << ")";
}

Val* LastIntVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto substBase = getBasePack()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getLastElement(substBase);
}

Val* LastIntVal::_resolveImplOverride()
{
    auto resolvedArg = getBasePack()->resolve();
    if (resolvedArg == getBasePack())
        return this;
    return getCurrentASTBuilder()->getLastElement(resolvedArg);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ConcreteIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ConcreteIntValPack::_toTextOverride(StringBuilder& out)
{
    for (Index i = 0; i < getCount(); i++)
    {
        if (i != 0)
            out << ", ";
        getElement(i)->toText(out);
    }
}

static void _appendShapePackOperandText(StringBuilder& out, Val* val)
{
    auto concretePack = as<ConcreteIntValPack>(val);
    if (concretePack)
        out << "(";
    val->toText(out);
    if (concretePack)
        out << ")";
}

Val* ConcreteIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    ShortList<IntVal*> substElements;
    for (Index i = 0; i < getCount(); i++)
    {
        auto substVal = getElement(i)->substituteImpl(astBuilder, subst, &diff);
        if (auto innerPack = as<ConcreteIntValPack>(substVal))
        {
            for (Index j = 0; j < innerPack->getCount(); ++j)
                substElements.add(innerPack->getElement(j));
        }
        else
        {
            substElements.add(as<IntVal>(substVal));
        }
    }
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getIntValPack(substElements.getArrayView().arrayView);
}

Val* ConcreteIntValPack::_resolveImplOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TrimFirstIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TrimFirstIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "__trimFirst(";
    getBasePack()->toText(out);
    out << ")";
}

Val* TrimFirstIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substBase = getBasePack()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getTrimFirstPack(substBase);
}

Val* TrimFirstIntValPack::_resolveImplOverride()
{
    auto resolvedArg = getBasePack()->resolve();
    if (resolvedArg == getBasePack())
        return this;
    return getCurrentASTBuilder()->getTrimFirstPack(resolvedArg);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TrimLastIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TrimLastIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "__trimLast(";
    getBasePack()->toText(out);
    out << ")";
}

Val* TrimLastIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substBase = getBasePack()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getTrimLastPack(substBase);
}

Val* TrimLastIntValPack::_resolveImplOverride()
{
    auto resolvedArg = getBasePack()->resolve();
    if (resolvedArg == getBasePack())
        return this;
    return getCurrentASTBuilder()->getTrimLastPack(resolvedArg);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ShapeConcatIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ShapeConcatIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "__shapeConcat(";
    _appendShapePackOperandText(out, getLeftPack());
    out << ", ";
    _appendShapePackOperandText(out, getRightPack());
    out << ", ";
    getAxis()->toText(out);
    out << ")";
}

Val* ShapeConcatIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substLeftPack = getLeftPack()->substituteImpl(astBuilder, subst, &diff);
    auto substRightPack = getRightPack()->substituteImpl(astBuilder, subst, &diff);
    auto substAxis = as<IntVal>(getAxis()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getShapeConcatIntValPack(substLeftPack, substRightPack, substAxis);
}

Val* ShapeConcatIntValPack::_resolveImplOverride()
{
    auto resolvedLeftPack = getLeftPack()->resolve();
    auto resolvedRightPack = getRightPack()->resolve();
    auto resolvedAxis = as<IntVal>(getAxis()->resolve());
    if (resolvedLeftPack == getLeftPack() && resolvedRightPack == getRightPack() &&
        resolvedAxis == getAxis())
        return this;
    return getCurrentASTBuilder()->getShapeConcatIntValPack(
        resolvedLeftPack,
        resolvedRightPack,
        resolvedAxis);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ShapePermuteIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ShapePermuteIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "__shapePermute(";
    _appendShapePackOperandText(out, getValuePack());
    out << ", ";
    _appendShapePackOperandText(out, getOrderPack());
    out << ")";
}

Val* ShapePermuteIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substValuePack = getValuePack()->substituteImpl(astBuilder, subst, &diff);
    auto substOrderPack = getOrderPack()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getShapePermuteIntValPack(substValuePack, substOrderPack);
}

Val* ShapePermuteIntValPack::_resolveImplOverride()
{
    auto resolvedValuePack = getValuePack()->resolve();
    auto resolvedOrderPack = getOrderPack()->resolve();
    if (resolvedValuePack == getValuePack() && resolvedOrderPack == getOrderPack())
        return this;
    return getCurrentASTBuilder()->getShapePermuteIntValPack(resolvedValuePack, resolvedOrderPack);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ShapeSwapIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ShapeSwapIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "__shapeSwap(";
    _appendShapePackOperandText(out, getValuePack());
    out << ", ";
    getDim0()->toText(out);
    out << ", ";
    getDim1()->toText(out);
    out << ")";
}

Val* ShapeSwapIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substValuePack = getValuePack()->substituteImpl(astBuilder, subst, &diff);
    auto substDim0 = as<IntVal>(getDim0()->substituteImpl(astBuilder, subst, &diff));
    auto substDim1 = as<IntVal>(getDim1()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getShapeSwapIntValPack(substValuePack, substDim0, substDim1);
}

Val* ShapeSwapIntValPack::_resolveImplOverride()
{
    auto resolvedValuePack = getValuePack()->resolve();
    auto resolvedDim0 = as<IntVal>(getDim0()->resolve());
    auto resolvedDim1 = as<IntVal>(getDim1()->resolve());
    if (resolvedValuePack == getValuePack() && resolvedDim0 == getDim0() &&
        resolvedDim1 == getDim1())
        return this;
    return getCurrentASTBuilder()->getShapeSwapIntValPack(
        resolvedValuePack,
        resolvedDim0,
        resolvedDim1);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ShapeReduceIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ShapeReduceIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "__shapeReduce(";
    _appendShapePackOperandText(out, getValuePack());
    out << ", ";
    getAxis()->toText(out);
    out << ")";
}

Val* ShapeReduceIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substValuePack = getValuePack()->substituteImpl(astBuilder, subst, &diff);
    auto substAxis = as<IntVal>(getAxis()->substituteImpl(astBuilder, subst, &diff));
    if (!diff)
        return this;
    (*ioDiff)++;
    return astBuilder->getShapeReduceIntValPack(substValuePack, substAxis);
}

Val* ShapeReduceIntValPack::_resolveImplOverride()
{
    auto resolvedValuePack = getValuePack()->resolve();
    auto resolvedAxis = as<IntVal>(getAxis()->resolve());
    if (resolvedValuePack == getValuePack() && resolvedAxis == getAxis())
        return this;
    return getCurrentASTBuilder()->getShapeReduceIntValPack(resolvedValuePack, resolvedAxis);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExpandIntValPack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExpandIntValPack::_toTextOverride(StringBuilder& out)
{
    out << "expand ";
    getPatternVal()->toText(out);
}

Val* ExpandIntValPack::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    ShortList<Val*> capturedPacks;
    ShortList<ConcreteIntValPack*> concreteValPacks;
    for (Index i = 0; i < getCapturedPackCount(); i++)
    {
        auto substPack = getCapturedPack(i)->substituteImpl(astBuilder, subst, &diff);
        capturedPacks.add(substPack);
        if (auto pack = as<ConcreteIntValPack>(substPack))
            concreteValPacks.add(pack);
    }

    if (!diff || concreteValPacks.getCount() != capturedPacks.getCount())
    {
        auto substPattern = getPatternVal()->substituteImpl(astBuilder, subst, &diff);
        if (!diff)
            return this;
        (*ioDiff)++;
        return astBuilder->getExpandIntValPack(
            substPattern,
            capturedPacks.getArrayView().arrayView);
    }
    else
    {
        ShortList<IntVal*> expandedVals;
        SLANG_ASSERT(capturedPacks.getCount() != 0);

        for (int i = 0; i < (int)concreteValPacks[0]->getCount(); i++)
        {
            subst.packExpansionIndex = i;
            auto substElement = getPatternVal()->substituteImpl(astBuilder, subst, &diff);
            expandedVals.add(as<IntVal>(substElement));
        }
        if (!diff)
            return this;
        (*ioDiff)++;
        return astBuilder->getIntValPack(expandedVals.getArrayView().arrayView);
    }
}

Val* ExpandIntValPack::_resolveImplOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! EachIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void EachIntVal::_toTextOverride(StringBuilder& out)
{
    out << "each ";
    getBasePack()->toText(out);
}

Val* EachIntVal::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto substBase = getBasePack()->substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;
    if (auto valPack = as<ConcreteIntValPack>(substBase))
    {
        if (subst.packExpansionIndex >= 0 && subst.packExpansionIndex < valPack->getCount())
        {
            (*ioDiff)++;
            return valPack->getElement(subst.packExpansionIndex);
        }
    }
    (*ioDiff)++;
    return astBuilder->getEachIntVal(getType(), substBase);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! isValuePack helpers !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool isAbstractValuePack(Val* val)
{
    if (as<ExpandIntValPack>(val))
        return true;
    if (as<TrimFirstIntValPack>(val))
        return true;
    if (as<TrimLastIntValPack>(val))
        return true;
    if (as<ShapeTransformIntValPack>(val))
        return true;
    if (auto declRefIntVal = as<DeclRefIntVal>(val))
    {
        if (as<GenericValuePackParamDecl>(declRefIntVal->getDeclRef().getDecl()))
            return true;
    }
    return false;
}

bool isValuePack(Val* val)
{
    if (as<ConcreteIntValPack>(val))
        return true;
    return isAbstractValuePack(val);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WitnessLookupIntVal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void WitnessLookupIntVal::_toTextOverride(StringBuilder& out)
{
    getWitness()->getSub()->toText(out);
    out << ".";
    out << (getKey()->getName() ? getKey()->getName()->text : "??");
}

Val* WitnessLookupIntVal::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();

    auto newWitness = as<SubtypeWitness>(getWitness()->resolve());
    if (!newWitness)
        return this;

    if (auto val = tryFoldOrNull(astBuilder, newWitness, getKey()))
    {
        return val;
    }

    auto newType = as<Type>(getType()->resolve());
    if (!newType)
        return this;

    if (newWitness != getWitness() || newType != getType())
    {
        return astBuilder->getOrCreate<WitnessLookupIntVal>(newType, newWitness, getKey());
    }

    return this;
}

Val* WitnessLookupIntVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newWitness = getWitness()->substituteImpl(astBuilder, subst, &diff);
    if (diff)
    {
        *ioDiff += diff;
        auto witnessEntry = tryFoldOrNull(astBuilder, as<SubtypeWitness>(newWitness), getKey());
        if (witnessEntry)
        {
            return witnessEntry;
        }
        else
        {
            return astBuilder->getOrCreate<WitnessLookupIntVal>(getType(), newWitness, getKey());
        }
    }
    return this;
}

Val* WitnessLookupIntVal::tryFoldOrNull(ASTBuilder* astBuilder, SubtypeWitness* witness, Decl* key)
{
    // Check if we can find an entry for this key.
    auto unspecializedEntry = getUnspecializedLookupRec(astBuilder, key, witness);

    // If we found a relevant entry, try to specialize it.
    switch (unspecializedEntry.getFlavor())
    {
    case RequirementWitness::Flavor::val:
        {
            auto specializedEntry = specializeLookedUpRec(astBuilder, witness, unspecializedEntry);
            SLANG_ASSERT(specializedEntry.getFlavor() == RequirementWitness::Flavor::val);
            return specializedEntry.getVal();
        }
        break;
    default:
        break;
    }
    return nullptr;
}

Val* WitnessLookupIntVal::tryFold(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    Decl* key,
    Type* type)
{
    if (auto result = tryFoldOrNull(astBuilder, witness, key))
        return result;
    auto witnessResult = astBuilder->getOrCreate<WitnessLookupIntVal>(type, witness, key);
    return witnessResult;
}

void DifferentiateVal::_toTextOverride(StringBuilder& out)
{
    out << "DifferentiateVal(";
    out << getFunc();
    out << ")";
}

Val* DifferentiateVal::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto newFunc = getFunc().substituteImpl(astBuilder, subst, &diff);
    *ioDiff += diff;
    if (diff)
    {
        auto result = as<DifferentiateVal>(astBuilder->createByNodeType(astNodeType));
        result->getFunc() = newFunc;
        return result;
    }
    // Nothing found: don't substitute.
    return this;
}

Val* DifferentiateVal::_resolveImplOverride()
{
    return this;
}

Val* PolynomialIntValFactor::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();

    auto newParam = as<IntVal>(getParam()->resolve());
    if (newParam && newParam != getParam())
        return astBuilder->getOrCreate<PolynomialIntValFactor>(newParam, getPower());

    return this;
}

Val* PolynomialIntValTerm::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();

    bool diff = false;
    List<PolynomialIntValFactor*> newFactors;
    for (auto factor : getParamFactors())
    {
        auto newFactor = as<PolynomialIntValFactor>(factor->resolve());
        if (!newFactor)
            return this;

        if (newFactor != factor)
            diff = true;
        newFactors.add(newFactor);
    }

    if (diff)
        return astBuilder->getOrCreate<PolynomialIntValTerm>(
            getConstFactor(),
            newFactors.getArrayView());

    return this;
}

Val* PolynomialIntVal::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();

    bool diff = false;
    PolynomialIntValBuilder builder(astBuilder);
    builder.constantTerm = getConstantTerm();
    for (auto term : getTerms())
    {
        auto newTerm = as<PolynomialIntValTerm>(term->resolve());
        if (!newTerm)
            return this;

        if (newTerm != term)
            diff = true;
        builder.terms.add(newTerm);
    }

    if (diff)
        return builder.getIntVal(getType());

    return this;
}

bool IntVal::isLinkTimeVal()
{
    SLANG_AST_NODE_VIRTUAL_CALL(IntVal, isLinkTimeVal, ());
}

Val* IntVal::linkTimeResolve(Dictionary<String, IntVal*>& mapMangledNameToVal)
{
    SLANG_AST_NODE_VIRTUAL_CALL(IntVal, linkTimeResolve, (mapMangledNameToVal));
}

//
// UIntSetVal
//

UIntSet UIntSetVal::toUIntSet() const
{
    UIntSet result;

    // Convert each bitmask operand back to UIntSet::Element
    result.resizeBackingBufferDirectly(getBitmaskCount());
    for (Index i = 0; i < getBitmaskCount(); i++)
    {
        result.m_buffer[i] = getBitmask(i);
    }

    return result;
}

void UIntSet::unionWith(const UIntSetVal& set)
{
    // UIntSetVal has getBitmask accessor that returns Elements
    const Index setCount = set.getBitmaskCount();
    const Index minCount = Math::Min(setCount, m_buffer.getCount());
    m_buffer.reserve(setCount);

    for (Index i = 0; i < minCount; ++i)
    {
        m_buffer[i] |= set.getBitmask(i);
    }

    // Add remaining elements from the UIntSetVal if it's larger
    if (setCount > m_buffer.getCount())
    {
        for (Index i = m_buffer.getCount(); i < setCount; ++i)
        {
            m_buffer.add(set.getBitmask(i));
        }
    }
}

UIntSetVal* UIntSetVal::fromUIntSet(ASTBuilder* astBuilder, const UIntSet& uintSet)
{
    return astBuilder->getUIntSetVal(uintSet);
}

void UIntSetVal::_toTextOverride(StringBuilder& out)
{
    out << "UIntSetVal{";
    for (Index i = 0; i < getBitmaskCount(); i++)
    {
        if (i > 0)
            out << ", ";
        out << getBitmask(i);
    }
    out << "}";
}

} // namespace Slang
