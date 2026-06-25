// slang-ast-clone.cpp

#include "slang-ast-clone.h"

#include "slang-ast-builder.h"
#include "slang-ast-dispatch.h"
#include "slang-lookup.h"
#include "slang-syntax.h"

namespace Slang
{

struct ASTCloneFieldAccess
{
#if 0 // FIDDLE TEMPLATE:
%for _,T in ipairs(Slang.NodeBase.subclasses) do
    static void cloneFields(ASTCloner& cloner, $T* dst, $T* src)
    {
        SLANG_UNUSED(cloner);
        SLANG_UNUSED(dst);
        SLANG_UNUSED(src);
%   if T.directSuperClass then
        cloneFields(cloner, static_cast<$(T.directSuperClass)*>(dst), src);
%   end
%   for _,f in ipairs(T.directFields) do
        cloner.cloneField(dst->$f, src->$f);
%   end
    }
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-ast-clone.cpp.fiddle"
#endif // FIDDLE END
};

ASTCloner::ASTCloner(ASTBuilder* astBuilder, SemanticsVisitor* semantics)
{
    m_context.astBuilder = astBuilder;
    m_context.semantics = semantics;
}

void ASTCloner::mapDecl(Decl* oldDecl, Decl* newDecl)
{
    if (!oldDecl || !newDecl || oldDecl == newDecl)
        return;

    if (auto existing = m_context.oldToNewDecls.tryGetValue(oldDecl))
    {
        SLANG_RELEASE_ASSERT(*existing == newDecl);
        return;
    }
    m_context.oldToNewDecls.add(oldDecl, newDecl);
    m_context.oldToNewVals.clear();
}

DeclRef<Decl> getSpecializedDeclRefWithParamsFromGeneric(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    GenericDecl* genericDeclToSpecialize,
    GenericDecl* genericDeclProvidingSpecializationArgs)
{
    auto defaultArgs =
        getDefaultSubstitutionArgs(astBuilder, semantics, genericDeclProvidingSpecializationArgs);
    return astBuilder->getGenericAppDeclRef(genericDeclToSpecialize, defaultArgs.getArrayView());
}

static void cloneNonReflectedDeclFields(Decl* dst, Decl* src)
{
    // `parameterIndex` is checked metadata rather than syntax, so FIDDLE does not clone it.
    // GenericSignatureCloner clones already-checked generic signatures, and later substitution
    // and generic argument solving index ordinary parameters by this field.
    if (auto srcTypeParam = as<GenericTypeParamDeclBase>(src))
        as<GenericTypeParamDeclBase>(dst)->parameterIndex = srcTypeParam->parameterIndex;
    else if (auto srcValPackParam = as<GenericValuePackParamDecl>(src))
        as<GenericValuePackParamDecl>(dst)->parameterIndex = srcValPackParam->parameterIndex;
    else if (auto srcValParam = as<GenericValueParamDecl>(src))
        as<GenericValueParamDecl>(dst)->parameterIndex = srcValParam->parameterIndex;
}

NodeBase* ASTCloner::cloneSyntaxNode(NodeBase* node)
{
    if (!node)
        return nullptr;

    auto clonedNode = m_context.astBuilder->createByNodeType(node->astNodeType);
    auto decl = as<Decl>(node);
    if (decl)
        mapDecl(decl, as<Decl>(clonedNode));

    cloneNodeFields(clonedNode, node);
    if (auto clonedDecl = as<Decl>(clonedNode))
    {
        cloneNonReflectedDeclFields(clonedDecl, decl);

        // Container membership is re-established by callers that invoke addDirectMemberDecl after
        // cloning. Drop the cloned lookup-accelerator chain so it cannot temporarily point back
        // into the source container.
        clonedDecl->_prevInContainerWithSameName = nullptr;
    }
    return clonedNode;
}

void ASTCloner::cloneNodeFields(NodeBase* dst, NodeBase* src)
{
    ASTNodeDispatcher<NodeBase, void>::dispatch(
        src,
        [&](auto typedSrc)
        {
            using T = std::remove_pointer_t<decltype(typedSrc)>;
            ASTCloneFieldAccess::cloneFields(*this, static_cast<T*>(dst), typedSrc);
        });

    if (auto srcStaticMemberExpr = as<StaticMemberExpr>(src))
    {
        // These fields are not reflected through FIDDLE, so the generated clone above cannot see
        // them. Preserve the lookup base because cloned generic-constraint type syntax may need it
        // if the type expression is checked again.
        auto dstStaticMemberExpr = as<StaticMemberExpr>(dst);
        dstStaticMemberExpr->baseExpression = cloneExpr(srcStaticMemberExpr->baseExpression);
        dstStaticMemberExpr->memberOperatorLoc = srcStaticMemberExpr->memberOperatorLoc;
    }
}

Decl* ASTCloner::cloneDecl(Decl* decl)
{
    if (!decl)
        return nullptr;

    if (auto existing = m_context.oldToNewDecls.tryGetValue(decl))
        return *existing;

    return as<Decl>(cloneSyntaxNode(decl));
}

Expr* ASTCloner::cloneExpr(Expr* expr)
{
    return as<Expr>(cloneSyntaxNode(expr));
}

Stmt* ASTCloner::cloneStmt(Stmt* stmt)
{
    return as<Stmt>(cloneSyntaxNode(stmt));
}

Modifier* ASTCloner::cloneModifier(Modifier* modifier)
{
    return as<Modifier>(cloneSyntaxNode(modifier));
}

Val* ASTCloner::rewriteVal(Val* val)
{
    if (!val)
        return nullptr;

    if (auto cached = m_context.oldToNewVals.tryGetValue(val))
        return *cached;

    // Canonical Val graphs can share substructure and can contain cycles. Seed the cache before
    // walking operands so a cyclic edge re-enters as an unchanged value instead of recursing
    // without bound; the final set below installs the fully rewritten result for later callers.
    m_context.oldToNewVals.add(val, val);
    auto rewritten = rewriteValImpl(val);
    m_context.oldToNewVals.set(val, rewritten);
    return rewritten;
}

Decl* ASTCloner::rewriteDecl(Decl* decl)
{
    if (auto newDecl = m_context.oldToNewDecls.tryGetValue(decl))
        return *newDecl;
    return decl;
}

Val* ASTCloner::rewriteValImpl(Val* val)
{
    ValNodeDesc desc;
    desc.type = val->getClass();

    bool changed = false;
    for (Index i = 0; i < val->getOperandCount(); i++)
    {
        auto operand = val->m_operands[i];
        switch (operand.kind)
        {
        case ValNodeOperandKind::ConstantValue:
            desc.operands.add(operand);
            break;

        case ValNodeOperandKind::ValNode:
            {
                auto oldOperandVal = operand.getVal();
                auto newOperandVal = rewriteVal(oldOperandVal);
                changed = changed || newOperandVal != oldOperandVal;
                desc.operands.add(ValNodeOperand(newOperandVal));
            }
            break;

        case ValNodeOperandKind::ASTNode:
            {
                auto oldOperandNode = operand.values.nodeOperand;
                auto newOperandNode = oldOperandNode;
                if (auto oldOperandDecl = as<Decl>(oldOperandNode))
                {
                    newOperandNode = rewriteDecl(oldOperandDecl);
                }
                changed = changed || newOperandNode != oldOperandNode;
                desc.operands.add(ValNodeOperand(newOperandNode));
            }
            break;
        }
    }

    if (!changed)
        return val;

    desc.init();
    return m_context.astBuilder->_getOrCreateImpl(_Move(desc));
}

QualType ASTCloner::rewriteQualType(QualType const& type)
{
    QualType result = type;
    result.type = rewriteType(type.type);
    return result;
}

TypeExp ASTCloner::cloneTypeExp(TypeExp const& typeExp)
{
    TypeExp result;
    result.exp = cloneExpr(typeExp.exp);
    result.type = rewriteType(typeExp.type);
    return result;
}

void ASTCloner::cloneField(QualType& dst, QualType const& src)
{
    dst = rewriteQualType(src);
}

void ASTCloner::cloneField(TypeExp& dst, TypeExp const& src)
{
    dst = cloneTypeExp(src);
}

void ASTCloner::cloneField(Modifiers& dst, Modifiers const& src)
{
    dst.first = nullptr;
    Modifier** link = &dst.first;
    auto modifier = src.first;
    while (modifier)
    {
        auto clonedModifier = cloneModifier(modifier);
        *link = clonedModifier;
        link = &clonedModifier->next;
        modifier = modifier->next;
    }
}

void ASTCloner::cloneField(ValNodeOperand& dst, ValNodeOperand const& src)
{
    switch (src.kind)
    {
    case ValNodeOperandKind::ConstantValue:
        dst = src;
        return;

    case ValNodeOperandKind::ValNode:
        dst = ValNodeOperand(rewriteVal(src.getVal()));
        return;

    case ValNodeOperandKind::ASTNode:
        {
            auto oldNode = src.values.nodeOperand;
            if (auto oldDecl = as<Decl>(oldNode))
            {
                if (auto newDecl = m_context.oldToNewDecls.tryGetValue(oldDecl))
                {
                    dst = ValNodeOperand(*newDecl);
                    return;
                }
            }
            dst = src;
            return;
        }
    }
}

void ASTCloner::cloneField(RefPtr<WitnessTable>& dst, RefPtr<WitnessTable> const& src)
{
    // Witness tables and generic-constraint path-resolution tables are derived from the checked
    // declaration graph. GenericSignatureCloner deliberately drops them and then asks
    // checkGenericConstraintConformances to rebuild path-resolution data for cloned constraints.
    // Other ASTCloner callers fail here so they do not produce checked declarations whose derived
    // witness data silently points at the original graph or disappears.
    SLANG_RELEASE_ASSERT(!src || m_context.allowDroppingWitnessTables);
    dst = nullptr;
}

void ASTCloner::cloneField(
    ContainerDeclDirectMemberDecls& dst,
    ContainerDeclDirectMemberDecls const& src)
{
    SLANG_RELEASE_ASSERT(!src.isUsingOnDemandDeserialization());
    SLANG_RELEASE_ASSERT(src.getDeclCount() == 0);
    dst = ContainerDeclDirectMemberDecls();
}

GenericSignatureCloner::GenericSignatureCloner(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    GenericDecl* sourceGenericDecl,
    GenericDecl* destGenericDecl,
    List<Expr*>* outGenericArgs)
    : m_astCloner(astBuilder, semantics)
    , m_sourceGenericDecl(sourceGenericDecl)
    , m_destGenericDecl(destGenericDecl)
    , m_outGenericArgs(outGenericArgs)
{
    m_astCloner.getContext().allowDroppingWitnessTables = true;
}

Expr* GenericSignatureCloner::createGenericArgExpr(Decl* decl)
{
    auto declRef = makeDeclRef(decl);
    auto expr = m_astCloner.getContext().astBuilder->create<VarExpr>();
    expr->declRef = declRef;
    if (auto varDecl = as<VarDeclBase>(decl))
        expr->type = QualType(varDecl->type.type);
    else
        expr->type = getTypeForDeclRef(m_astCloner.getContext().astBuilder, declRef, SourceLoc());
    return expr;
}

void GenericSignatureCloner::cloneParameterMembers()
{
    for (auto member : m_sourceGenericDecl->getDirectMemberDecls())
    {
        if (!isGenericParam(member))
            continue;

        auto clonedParam = m_astCloner.cloneDecl(member);
        // GenericSignatureCloner creates synthetic wrappers that preserve binder identity and
        // checked constraints, not user-facing default arguments. Keeping defaults here can make a
        // synthesized extension wrapper cloned from a defaulted generic type look user-written to
        // the extension-default diagnostic, but that diagnostic is only meant for source
        // declarations.
        if (auto typeParam = as<GenericTypeParamDecl>(clonedParam))
            typeParam->initType = TypeExp();
        else if (auto valueParam = as<GenericValueParamDecl>(clonedParam))
            valueParam->initExpr = nullptr;
        m_destGenericDecl->addDirectMemberDecl(clonedParam);

        if (m_outGenericArgs)
            m_outGenericArgs->add(createGenericArgExpr(clonedParam));
    }
}

static TypeExp substituteConstraintTypeExp(
    ASTBuilder* astBuilder,
    TypeExp const& sourceTypeExp,
    TypeExp const& clonedTypeExp,
    SubstitutionSet const& sourceToDestSubstitution)
{
    TypeExp result = clonedTypeExp;
    if (sourceTypeExp.type)
    {
        result.type =
            as<Type>(sourceTypeExp.type->substitute(astBuilder, sourceToDestSubstitution));
    }
    return result;
}

Decl* GenericSignatureCloner::cloneConstraintMember(
    Decl* member,
    SubstitutionSet const& sourceToDestSubstitution)
{
    if (auto packCountConstraint = as<GenericVariadicPackCountConstraintDecl>(member))
    {
        // Header checking owns the diagnostic for a non-foldable count expression. Requirement
        // witness synthesis can run during recovery too, so preserve the existing behavior of
        // skipping this proof when that earlier invariant failed.
        if (!packCountConstraint->expectedCountVal)
            return nullptr;
    }

    if (!isGenericConstraintParameterDecl(member))
        return nullptr;

    auto clonedConstraint = m_astCloner.cloneDecl(member);
    m_destGenericDecl->addDirectMemberDecl(clonedConstraint);

    // The source constraint's checked `TypeExp::type` can contain declared witnesses and
    // associated-type lookup paths that are tied to the source generic environment. The syntax
    // clone above preserves the written expressions, but the checked types must be rebuilt through
    // the source-to-destination generic decl-ref that `liftDeclFromGenericContainers` is
    // constructing; otherwise a cloned proof can keep the old parent path while naming the new
    // constraint decl.
    auto astBuilder = m_astCloner.getContext().astBuilder;
    if (auto sourceTypeConstraint = as<GenericTypeConstraintDecl>(member))
    {
        auto destTypeConstraint = as<GenericTypeConstraintDecl>(clonedConstraint);
        destTypeConstraint->sub = substituteConstraintTypeExp(
            astBuilder,
            sourceTypeConstraint->sub,
            destTypeConstraint->sub,
            sourceToDestSubstitution);
        destTypeConstraint->sup = substituteConstraintTypeExp(
            astBuilder,
            sourceTypeConstraint->sup,
            destTypeConstraint->sup,
            sourceToDestSubstitution);
    }
    else if (auto sourceCoercionConstraint = as<TypeCoercionConstraintDecl>(member))
    {
        auto destCoercionConstraint = as<TypeCoercionConstraintDecl>(clonedConstraint);
        destCoercionConstraint->fromType = substituteConstraintTypeExp(
            astBuilder,
            sourceCoercionConstraint->fromType,
            destCoercionConstraint->fromType,
            sourceToDestSubstitution);
        destCoercionConstraint->toType = substituteConstraintTypeExp(
            astBuilder,
            sourceCoercionConstraint->toType,
            destCoercionConstraint->toType,
            sourceToDestSubstitution);
    }
    else if (auto sourceHasDiffTypeInfoConstraint = as<HasDiffTypeInfoConstraintDecl>(member))
    {
        auto destHasDiffTypeInfoConstraint = as<HasDiffTypeInfoConstraintDecl>(clonedConstraint);
        destHasDiffTypeInfoConstraint->type = substituteConstraintTypeExp(
            astBuilder,
            sourceHasDiffTypeInfoConstraint->type,
            destHasDiffTypeInfoConstraint->type,
            sourceToDestSubstitution);
    }

    return clonedConstraint;
}

} // namespace Slang
