#pragma once
#include "slang-syntax.h"

namespace Slang
{
template <typename Callback>
struct ASTIterator
{
    const Callback& callback;
    UnownedStringSlice fileName;
    SourceManager* sourceManager;
    ASTIterator(const Callback& func, SourceManager* manager, UnownedStringSlice sourceFileName)
        : callback(func)
        , fileName(sourceFileName)
        , sourceManager(manager)
    {}

    void visitDecl(DeclBase* decl);
    void visitExpr(Expr* expr);
    void visitStmt(Stmt* stmt);

    void maybeDispatchCallback(SyntaxNode* node)
    {
        if (node)
        {
            callback(node);
        }
    }

    struct ASTIteratorExprVisitor : public ExprVisitor<ASTIteratorExprVisitor>
    {
    public:
        ASTIterator* iterator;
        ASTIteratorExprVisitor(ASTIterator* iter)
            : iterator(iter)
        {}
        void dispatchIfNotNull(Expr* expr)
        {
            if (!expr)
                return;
            expr->accept(this, nullptr);
        }
        bool visitExpr(Expr*) { return false; }
        void visitBoolLiteralExpr(BoolLiteralExpr* expr) { iterator->maybeDispatchCallback(expr); }
        void visitNullPtrLiteralExpr(NullPtrLiteralExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
        }
        void visitNoneLiteralExpr(NoneLiteralExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
        }
        void visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
        }
        void visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
        }
        void visitStringLiteralExpr(StringLiteralExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
        }
        void visitIncompleteExpr(IncompleteExpr* expr) { iterator->maybeDispatchCallback(expr); }
        void visitIndexExpr(IndexExpr* subscriptExpr)
        {
            iterator->maybeDispatchCallback(subscriptExpr);
            dispatchIfNotNull(subscriptExpr->baseExpression);
            for (auto arg : subscriptExpr->indexExprs)
                dispatchIfNotNull(arg);
        }

        void visitParenExpr(ParenExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
        }

        void visitAssignExpr(AssignExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->left);
            dispatchIfNotNull(expr->right);
        }

        void visitGenericAppExpr(GenericAppExpr* genericAppExpr)
        {
            iterator->maybeDispatchCallback(genericAppExpr);

            dispatchIfNotNull(genericAppExpr->functionExpr);
            for (auto arg : genericAppExpr->arguments)
                dispatchIfNotNull(arg);
        }

        void visitSharedTypeExpr(SharedTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base.exp);
        }

        void visitInvokeExpr(InvokeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);

            dispatchIfNotNull(expr->functionExpr);
            dispatchIfNotNull(expr->originalFunctionExpr);

            for (auto arg : expr->arguments)
                dispatchIfNotNull(arg);
        }

        void visitVarExpr(VarExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->originalExpr);
        }

        void visitTryExpr(TryExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
        }

        void visitTypeCastExpr(TypeCastExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);

            dispatchIfNotNull(expr->functionExpr);
            for (auto arg : expr->arguments)
                dispatchIfNotNull(arg);
        }

        void visitDerefExpr(DerefExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
        }
        void visitMatrixSwizzleExpr(MatrixSwizzleExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
        }
        void visitSwizzleExpr(SwizzleExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
        }
        void visitOverloadedExpr(OverloadedExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
        }
        void visitOverloadedExpr2(OverloadedExpr2* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base);
            for (auto candidate : expr->candidiateExprs)
            {
                dispatchIfNotNull(candidate);
            }
        }
        void visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base.exp);
            for (auto arg : expr->arguments)
            {
                dispatchIfNotNull(arg);
            }
        }
        void visitCastToSuperTypeExpr(CastToSuperTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->valueArg);
        }
        void visitModifierCastExpr(ModifierCastExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->valueArg);
        }
        void visitLetExpr(LetExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            iterator->visitDecl(expr->decl);
            dispatchIfNotNull(expr->body);
        }
        void visitExtractExistentialValueExpr(ExtractExistentialValueExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->originalExpr);
        }

        void visitDeclRefExpr(DeclRefExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->originalExpr);
        }

        void visitStaticMemberExpr(StaticMemberExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->baseExpression);
        }

        void visitMemberExpr(MemberExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->baseExpression);
        }

        void visitInitializerListExpr(InitializerListExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            for (auto arg : expr->args)
            {
                dispatchIfNotNull(arg);
            }
        }

        void visitThisExpr(ThisExpr* expr) { iterator->maybeDispatchCallback(expr); }
        void visitThisTypeExpr(ThisTypeExpr* expr) { iterator->maybeDispatchCallback(expr); }
        void visitReturnValExpr(ReturnValExpr* expr) { iterator->maybeDispatchCallback(expr); }

        void visitAndTypeExpr(AndTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->left.exp);
            dispatchIfNotNull(expr->right.exp);
        }
        void visitModifiedTypeExpr(ModifiedTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base.exp);
        }
        void visitFuncTypeExpr(FuncTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            for(const auto& t : expr->parameters)
                dispatchIfNotNull(t.exp);
            dispatchIfNotNull(expr->result.exp);
        }
        void visitTupleTypeExpr(TupleTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            for(auto t : expr->members)
                dispatchIfNotNull(t.exp);
        }
        void visitPointerTypeExpr(PointerTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->base.exp);
        }
        void visitAsTypeExpr(AsTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->value);
            dispatchIfNotNull(expr->typeExpr);
        }
        void visitIsTypeExpr(IsTypeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->value);
            dispatchIfNotNull(expr->typeExpr.exp);
        }
        void visitMakeOptionalExpr(MakeOptionalExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->value);
            dispatchIfNotNull(expr->typeExpr);
        }
        void visitPartiallyAppliedGenericExpr(PartiallyAppliedGenericExpr* expr)
        {
            dispatchIfNotNull(expr->originalExpr);
        }

        void visitHigherOrderInvokeExpr(HigherOrderInvokeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            dispatchIfNotNull(expr->baseFunction);
        }

        void visitTreatAsDifferentiableExpr(TreatAsDifferentiableExpr* expr)
        {
            dispatchIfNotNull(expr->innerExpr);
        }

        void visitSPIRVAsmExpr(SPIRVAsmExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);
            for(const auto& i : expr->insts)
            {
                dispatchIfNotNull(i.opcode.expr);
                for(const auto& o : i.operands)
                    dispatchIfNotNull(o.expr);
            }
        }
    };

    struct ASTIteratorStmtVisitor : public StmtVisitor<ASTIteratorStmtVisitor>
    {
        ASTIterator* iterator;
        ASTIteratorStmtVisitor(ASTIterator* iter)
            : iterator(iter)
        {}

        void dispatchIfNotNull(Stmt* stmt)
        {
            if (!stmt)
                return;
            stmt->accept(this, nullptr);
        }

        void visitDeclStmt(DeclStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitDecl(stmt->decl);
        }

        void visitBlockStmt(BlockStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            dispatchIfNotNull(stmt->body);
        }

        void visitSeqStmt(SeqStmt* seqStmt)
        {
            iterator->maybeDispatchCallback(seqStmt);
            for (auto stmt : seqStmt->stmts)
                dispatchIfNotNull(stmt);
        }

        void visitLabelStmt(LabelStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            dispatchIfNotNull(stmt->innerStmt);
        }

        void visitBreakStmt(BreakStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitContinueStmt(ContinueStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitDoWhileStmt(DoWhileStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->predicate);
            dispatchIfNotNull(stmt->statement);
        }

        void visitForStmt(ForStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            dispatchIfNotNull(stmt->initialStatement);
            iterator->visitExpr(stmt->predicateExpression);
            iterator->visitExpr(stmt->sideEffectExpression);
            dispatchIfNotNull(stmt->statement);
        }

        void visitCompileTimeForStmt(CompileTimeForStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
        }

        void visitSwitchStmt(SwitchStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->condition);
            dispatchIfNotNull(stmt->body);
        }

        void visitCaseStmt(CaseStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->expr);
        }

        void visitTargetSwitchStmt(TargetSwitchStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            for (auto c : stmt->targetCases)
                dispatchIfNotNull(c);
        }

        void visitTargetCaseStmt(TargetCaseStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitStmt(stmt->body);
        }

        void visitIntrinsicAsmStmt(IntrinsicAsmStmt*) {}

        void visitDefaultStmt(DefaultStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitIfStmt(IfStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->predicate);
            dispatchIfNotNull(stmt->positiveStatement);
            dispatchIfNotNull(stmt->negativeStatement);
        }

        void visitUnparsedStmt(UnparsedStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitEmptyStmt(EmptyStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitDiscardStmt(DiscardStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitReturnStmt(ReturnStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->expression);
        }

        void visitWhileStmt(WhileStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->predicate);
            dispatchIfNotNull(stmt->statement);
        }

        void visitGpuForeachStmt(GpuForeachStmt* stmt) { iterator->maybeDispatchCallback(stmt); }

        void visitExpressionStmt(ExpressionStmt* stmt)
        {
            iterator->maybeDispatchCallback(stmt);
            iterator->visitExpr(stmt->expression);
        }
    };
};

template <typename CallbackFunc>
void ASTIterator<CallbackFunc>::visitDecl(DeclBase* decl)
{
    // Don't look at the decl if it is defined in a different file.
    if (!as<ModuleDecl>(decl) && !sourceManager->getHumaneLoc(decl->loc, SourceLocType::Actual)
                                      .pathInfo.foundPath.getUnownedSlice()
                                      .endsWithCaseInsensitive(fileName))
        return;

    maybeDispatchCallback(decl);
    if (auto funcDecl = as<FunctionDeclBase>(decl))
    {
        visitStmt(funcDecl->body);
        visitExpr(funcDecl->returnType.exp);
    }
    else if (auto propertyDecl = as<PropertyDecl>(decl))
    {
        visitExpr(propertyDecl->type.exp);
    }
    else if (auto varDecl = as<VarDeclBase>(decl))
    {
        visitExpr(varDecl->type.exp);
        visitExpr(varDecl->initExpr);
    }
    else if (auto genericDecl = as<GenericDecl>(decl))
    {
        visitDecl(genericDecl->inner);
    }
    else if (auto typeConstraint = as<TypeConstraintDecl>(decl))
    {
        visitExpr(typeConstraint->getSup().exp);
    }
    else if (auto typedefDecl = as<TypeDefDecl>(decl))
    {
        visitExpr(typedefDecl->type.exp);
    }
    else if (auto extDecl = as<ExtensionDecl>(decl))
    {
        visitExpr(extDecl->targetType.exp);
    }
    if (auto container = as<ContainerDecl>(decl))
    {
        for (auto member : container->members)
        {
            visitDecl(member);
        }
    }
    for (auto modifier : decl->modifiers)
    {
        if (auto attr = as<Attribute>(modifier))
        {
            maybeDispatchCallback(attr);
            for (auto arg : attr->args)
                visitExpr(arg);
        }
    }
}
template <typename CallbackFunc>
void ASTIterator<CallbackFunc>::visitExpr(Expr* expr)
{
    ASTIteratorExprVisitor visitor(this);
    visitor.dispatchIfNotNull(expr);
}
template <typename CallbackFunc>
void ASTIterator<CallbackFunc>::visitStmt(Stmt* stmt)
{
    ASTIteratorStmtVisitor visitor(this);
    visitor.dispatchIfNotNull(stmt);
}

template <typename Func>
void iterateAST(
    UnownedStringSlice fileName, SourceManager* manager, SyntaxNode* node, const Func& f)
{
    ASTIterator<Func> iter(f, manager, fileName);
    if (auto decl = as<Decl>(node))
    {
        iter.visitDecl(decl);
    }
    else if (auto expr = as<Expr>(node))
    {
        iter.visitExpr(expr);
    }
    else if (auto stmt = as<Stmt>(node))
    {
        iter.visitStmt(stmt);
    }
}
} // namespace Slang
