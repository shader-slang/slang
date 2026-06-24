// slang-ast-copy.cpp

#include "slang-ast-copy.h"

#include "slang-ast-builder.h"
#include "slang-ast-dispatch.h"
#include "slang-lookup.h"
#include "slang-syntax.h"

namespace Slang
{

struct ASTCopyFieldAccess
{
#if 0 // FIDDLE TEMPLATE:
%for _,T in ipairs(Slang.NodeBase.subclasses) do
    static void copyFields(ASTCopier& copier, $T* dst, $T* src)
    {
        SLANG_UNUSED(copier);
        SLANG_UNUSED(dst);
        SLANG_UNUSED(src);
%   if T.directSuperClass then
        copyFields(copier, static_cast<$(T.directSuperClass)*>(dst), src);
%   end
%   for _,f in ipairs(T.directFields) do
        copier.copyField(dst->$f, src->$f);
%   end
    }
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-ast-copy.cpp.fiddle"
#endif // FIDDLE END
};

ASTCopier::ASTCopier(ASTBuilder* astBuilder, SemanticsVisitor* semantics)
{
    m_context.astBuilder = astBuilder;
    m_context.semantics = semantics;
}

void ASTCopier::mapDecl(Decl* oldDecl, Decl* newDecl)
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

static void copyNonReflectedDeclFields(Decl* dst, Decl* src)
{
    // `parameterIndex` is checked metadata rather than syntax, so FIDDLE does not copy it.
    // GenericSignatureCopier copies already-checked generic signatures, and later substitution
    // and generic argument solving index ordinary parameters by this field.
    if (auto srcTypeParam = as<GenericTypeParamDeclBase>(src))
        as<GenericTypeParamDeclBase>(dst)->parameterIndex = srcTypeParam->parameterIndex;
    else if (auto srcValPackParam = as<GenericValuePackParamDecl>(src))
        as<GenericValuePackParamDecl>(dst)->parameterIndex = srcValPackParam->parameterIndex;
    else if (auto srcValParam = as<GenericValueParamDecl>(src))
        as<GenericValueParamDecl>(dst)->parameterIndex = srcValParam->parameterIndex;
}

NodeBase* ASTCopier::copySyntaxNode(NodeBase* node)
{
    if (!node)
        return nullptr;

    auto copiedNode = m_context.astBuilder->createByNodeType(node->astNodeType);
    auto decl = as<Decl>(node);
    if (decl)
        mapDecl(decl, as<Decl>(copiedNode));

    copyNodeFields(copiedNode, node);
    if (auto copiedDecl = as<Decl>(copiedNode))
    {
        copyNonReflectedDeclFields(copiedDecl, decl);

        // Container membership is re-established by callers such as GenericSignatureCopier through
        // addDirectMemberDecl. Drop the copied lookup-accelerator chain so it cannot temporarily
        // point back into the source container.
        copiedDecl->_prevInContainerWithSameName = nullptr;
    }
    return copiedNode;
}

void ASTCopier::copyNodeFields(NodeBase* dst, NodeBase* src)
{
    ASTNodeDispatcher<NodeBase, void>::dispatch(
        src,
        [&](auto typedSrc)
        {
            using T = std::remove_pointer_t<decltype(typedSrc)>;
            ASTCopyFieldAccess::copyFields(*this, static_cast<T*>(dst), typedSrc);
        });

    if (auto srcStaticMemberExpr = as<StaticMemberExpr>(src))
    {
        // These fields are not reflected through FIDDLE, so the generated copy above cannot see
        // them. Preserve the lookup base because copied generic-constraint type syntax may need it
        // if the type expression is checked again.
        auto dstStaticMemberExpr = as<StaticMemberExpr>(dst);
        dstStaticMemberExpr->baseExpression = copyExpr(srcStaticMemberExpr->baseExpression);
        dstStaticMemberExpr->memberOperatorLoc = srcStaticMemberExpr->memberOperatorLoc;
    }
}

Decl* ASTCopier::copyDecl(Decl* decl)
{
    if (!decl)
        return nullptr;

    if (auto existing = m_context.oldToNewDecls.tryGetValue(decl))
        return *existing;

    return as<Decl>(copySyntaxNode(decl));
}

Expr* ASTCopier::copyExpr(Expr* expr)
{
    return as<Expr>(copySyntaxNode(expr));
}

Stmt* ASTCopier::copyStmt(Stmt* stmt)
{
    return as<Stmt>(copySyntaxNode(stmt));
}

Modifier* ASTCopier::copyModifier(Modifier* modifier)
{
    return as<Modifier>(copySyntaxNode(modifier));
}

Val* ASTCopier::rewriteVal(Val* val)
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

Decl* ASTCopier::rewriteDecl(Decl* decl)
{
    if (auto newDecl = m_context.oldToNewDecls.tryGetValue(decl))
        return *newDecl;
    return decl;
}

Val* ASTCopier::rewriteValImpl(Val* val)
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

QualType ASTCopier::rewriteQualType(QualType const& type)
{
    QualType result = type;
    result.type = rewriteType(type.type);
    return result;
}

TypeExp ASTCopier::copyTypeExp(TypeExp const& typeExp)
{
    TypeExp result;
    result.exp = copyExpr(typeExp.exp);
    result.type = rewriteType(typeExp.type);
    return result;
}

void ASTCopier::copyField(QualType& dst, QualType const& src)
{
    dst = rewriteQualType(src);
}

void ASTCopier::copyField(TypeExp& dst, TypeExp const& src)
{
    dst = copyTypeExp(src);
}

void ASTCopier::copyField(Modifiers& dst, Modifiers const& src)
{
    dst.first = nullptr;
    Modifier** link = &dst.first;
    auto modifier = src.first;
    while (modifier)
    {
        auto copiedModifier = copyModifier(modifier);
        *link = copiedModifier;
        link = &copiedModifier->next;
        modifier = modifier->next;
    }
}

void ASTCopier::copyField(ValNodeOperand& dst, ValNodeOperand const& src)
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

void ASTCopier::copyField(RefPtr<WitnessTable>& dst, RefPtr<WitnessTable> const& src)
{
    // Witness tables and generic-constraint path-resolution tables are derived from the checked
    // declaration graph. GenericSignatureCopier deliberately drops them and then asks
    // checkGenericConstraintConformances to rebuild path-resolution data for copied constraints.
    // Other ASTCopier callers fail here so they do not produce checked declarations whose derived
    // witness data silently points at the original graph or disappears.
    SLANG_RELEASE_ASSERT(!src || m_context.allowDroppingWitnessTables);
    dst = nullptr;
}

void ASTCopier::copyField(
    ContainerDeclDirectMemberDecls& dst,
    ContainerDeclDirectMemberDecls const& src)
{
    SLANG_RELEASE_ASSERT(!src.isUsingOnDemandDeserialization());
    SLANG_RELEASE_ASSERT(src.getDeclCount() == 0);
    dst = ContainerDeclDirectMemberDecls();
}

GenericSignatureCopier::GenericSignatureCopier(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    GenericDecl* sourceGenericDecl,
    GenericDecl* destGenericDecl,
    List<Expr*>* outGenericArgs)
    : m_astCopier(astBuilder, semantics)
    , m_sourceGenericDecl(sourceGenericDecl)
    , m_destGenericDecl(destGenericDecl)
    , m_outGenericArgs(outGenericArgs)
{
    m_astCopier.getContext().allowDroppingWitnessTables = true;
}

Expr* GenericSignatureCopier::createGenericArgExpr(Decl* decl)
{
    auto declRef = makeDeclRef(decl);
    auto expr = m_astCopier.getContext().astBuilder->create<VarExpr>();
    expr->declRef = declRef;
    if (auto varDecl = as<VarDeclBase>(decl))
        expr->type = QualType(varDecl->type.type);
    else
        expr->type = getTypeForDeclRef(m_astCopier.getContext().astBuilder, declRef, SourceLoc());
    return expr;
}

void GenericSignatureCopier::copyParameterMembers()
{
    for (auto member : m_sourceGenericDecl->getDirectMemberDecls())
    {
        if (!isGenericParam(member))
            continue;

        auto copiedParam = m_astCopier.copyDecl(member);
        m_destGenericDecl->addDirectMemberDecl(copiedParam);

        if (m_outGenericArgs)
            m_outGenericArgs->add(createGenericArgExpr(copiedParam));
    }
}

static TypeExp substituteConstraintTypeExp(
    ASTBuilder* astBuilder,
    TypeExp const& sourceTypeExp,
    TypeExp const& copiedTypeExp,
    SubstitutionSet const& sourceToDestSubstitution)
{
    TypeExp result = copiedTypeExp;
    if (sourceTypeExp.type)
    {
        result.type =
            as<Type>(sourceTypeExp.type->substitute(astBuilder, sourceToDestSubstitution));
    }
    return result;
}

Decl* GenericSignatureCopier::copyConstraintMember(
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

    auto copiedConstraint = m_astCopier.copyDecl(member);
    m_destGenericDecl->addDirectMemberDecl(copiedConstraint);

    // The source constraint's checked `TypeExp::type` can contain declared witnesses and
    // associated-type lookup paths that are tied to the source generic environment. The syntax copy
    // above preserves the written expressions, but the checked types must be rebuilt through the
    // source-to-destination generic decl-ref that `liftDeclFromGenericContainers` is constructing;
    // otherwise a copied proof can keep the old parent path while naming the new constraint decl.
    auto astBuilder = m_astCopier.getContext().astBuilder;
    if (auto sourceTypeConstraint = as<GenericTypeConstraintDecl>(member))
    {
        auto destTypeConstraint = as<GenericTypeConstraintDecl>(copiedConstraint);
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
        auto destCoercionConstraint = as<TypeCoercionConstraintDecl>(copiedConstraint);
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
        auto destHasDiffTypeInfoConstraint = as<HasDiffTypeInfoConstraintDecl>(copiedConstraint);
        destHasDiffTypeInfoConstraint->type = substituteConstraintTypeExp(
            astBuilder,
            sourceHasDiffTypeInfoConstraint->type,
            destHasDiffTypeInfoConstraint->type,
            sourceToDestSubstitution);
    }

    return copiedConstraint;
}

} // namespace Slang
