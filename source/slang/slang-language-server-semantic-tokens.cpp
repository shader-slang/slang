#include "slang-language-server-semantic-tokens.h"
#include "slang-visitor.h"
#include "slang-ast-support-types.h"
#include <algorithm>

namespace Slang
{
template<typename Callback>
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
            dispatchIfNotNull(subscriptExpr->indexExpression);
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

        void visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* expr) { iterator->maybeDispatchCallback(expr); }

        void visitInvokeExpr(InvokeExpr* expr)
        {
            iterator->maybeDispatchCallback(expr);

            dispatchIfNotNull(expr->functionExpr);
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
        }

        void visitDeclRefExpr(DeclRefExpr* expr) { iterator->maybeDispatchCallback(expr); }

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
    if (!as<ModuleDecl>(decl) &&
        !sourceManager->getHumaneLoc(decl->loc, SourceLocType::Actual)
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
    if (auto container = as<ContainerDecl>(decl))
    {
        for (auto member : container->members)
        {
            visitDecl(member);
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
void iterateAST(UnownedStringSlice fileName, SourceManager* manager, SyntaxNode* node, const Func& f)
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

const char* kSemanticTokenTypes[] = {
    "type", "enumMember", "variable", "parameter", "function", "property", "namespace"};

static_assert(SLANG_COUNT_OF(kSemanticTokenTypes) == (int)SemanticTokenType::NormalText, "kSemanticTokenTypes must match SemanticTokenType");

SemanticToken _createSemanticToken(SourceManager* manager, SourceLoc loc, Name* name)
{
    SemanticToken token;
    auto humaneLoc = manager->getHumaneLoc(loc, SourceLocType::Actual);
    token.line = (int)(humaneLoc.line - 1);
    token.col = (int)(humaneLoc.column - 1);
    token.length =
        name ? (int)(name->text.getLength()) : 0;
    token.type = SemanticTokenType::NormalText;
    return token;
}

List<SemanticToken> getSemanticTokens(Linkage* linkage, Module* module, UnownedStringSlice fileName)
{
    auto manager = linkage->getSourceManager();

    List<SemanticToken> result;
    auto maybeInsertToken = [&](const SemanticToken& token)
    {
        if (token.line >= 0 && token.col >= 0 && token.length > 0 &&
            token.type != SemanticTokenType::NormalText)
            result.add(token);
    };

    iterateAST(
        fileName,
        manager,
        module->getModuleDecl(),
        [&](SyntaxNode* node)
        {
            if (auto declRef = as<DeclRefExpr>(node))
            {
                if (declRef->name)
                {
                    // Don't look at the expr if it is defined in a different file.
                    if (!manager->getHumaneLoc(declRef->loc, SourceLocType::Actual)
                             .pathInfo.foundPath.getUnownedSlice()
                             .endsWithCaseInsensitive(fileName))
                        return;

                    SemanticToken token =
                        _createSemanticToken(manager, declRef->loc, declRef->name);
                    auto target = declRef->declRef.decl;
                    if (as<AggTypeDecl>(target))
                    {
                        if (target->hasModifier<BuiltinTypeModifier>())
                            return;
                        token.type = SemanticTokenType::Type;
                    }
                    else if (as<SimpleTypeDecl>(target))
                    {
                        token.type = SemanticTokenType::Type;
                    }
                    else if (as<PropertyDecl>(target))
                    {
                        token.type = SemanticTokenType::Property;
                    }
                    else if (as<ParamDecl>(target))
                    {
                        token.type = SemanticTokenType::Parameter;
                    }
                    else if (as<VarDecl>(target))
                    {
                        token.type = SemanticTokenType::Variable;
                    }
                    else if (as<FunctionDeclBase>(target))
                    {
                        token.type = SemanticTokenType::Function;
                    }
                    else if (as<EnumCaseDecl>(target))
                    {
                        token.type = SemanticTokenType::EnumMember;
                    }
                    else if (as<NamespaceDecl>(target))
                    {
                        token.type = SemanticTokenType::Namespace;
                    }

                    if (as<CallableDecl>(target))
                    {
                        if (target->hasModifier<ImplicitConversionModifier>())
                            return;
                    }
                    maybeInsertToken(token);
                }

            }
            else if (auto typeDecl = as<SimpleTypeDecl>(node))
            {
                if (typeDecl->getName())
                {
                    SemanticToken token =
                        _createSemanticToken(manager, typeDecl->getNameLoc(), typeDecl->getName());
                    token.type = SemanticTokenType::Type;
                    maybeInsertToken(token);
                }
            }
            else if (auto aggTypeDecl = as<AggTypeDeclBase>(node))
            {
                if (aggTypeDecl->getName())
                {
                    SemanticToken token = _createSemanticToken(
                        manager, aggTypeDecl->getNameLoc(), aggTypeDecl->getName());
                    token.type = SemanticTokenType::Type;
                    maybeInsertToken(token);
                }
            }
            else if (auto enumCase = as<EnumCaseDecl>(node))
            {
                if (enumCase->getName())
                {
                    SemanticToken token = _createSemanticToken(
                        manager, enumCase->getNameLoc(), enumCase->getName());
                    token.type = SemanticTokenType::EnumMember;
                    maybeInsertToken(token);
                }
            }
            else if (auto propertyDecl = as<PropertyDecl>(node))
            {
                if (propertyDecl->getName())
                {
                    SemanticToken token = _createSemanticToken(
                        manager, propertyDecl->getNameLoc(), propertyDecl->getName());
                    token.type = SemanticTokenType::Property;
                    maybeInsertToken(token);
                }
            }
            else if (auto funcDecl = as<FuncDecl>(node))
            {
                if (funcDecl->getName())
                {
                    SemanticToken token = _createSemanticToken(
                        manager, funcDecl->getNameLoc(), funcDecl->getName());
                    token.type = SemanticTokenType::Function;
                    maybeInsertToken(token);
                }
            }
            else if (auto varDecl = as<VarDeclBase>(node))
            {
                if (varDecl->getName())
                {
                    SemanticToken token = _createSemanticToken(
                        manager, varDecl->getNameLoc(), varDecl->getName());
                    token.type = SemanticTokenType::Variable;
                    maybeInsertToken(token);
                }
            }
        });
    return result;
}

List<uint32_t> getEncodedTokens(List<SemanticToken>& tokens)
{
    List<uint32_t> result;
    if (tokens.getCount() == 0)
        return result;

    std::sort(tokens.begin(), tokens.end());

    // Encode the first token as is.
    result.add((uint32_t)tokens[0].line);
    result.add((uint32_t)tokens[0].col);
    result.add((uint32_t)tokens[0].length);
    result.add((uint32_t)tokens[0].type);
    result.add(0);

    // Encode the rest tokens as deltas.
    uint32_t prevLine = (uint32_t)tokens[0].line;
    uint32_t prevCol = (uint32_t)tokens[0].col;
    for (Index i = 1; i < tokens.getCount(); i++)
    {
        uint32_t thisLine = (uint32_t)tokens[i].line;
        uint32_t thisCol = (uint32_t)tokens[i].col;
        if (thisLine == prevLine && thisCol == prevCol)
            continue;

        uint32_t deltaLine = thisLine - prevLine;
        uint32_t deltaCol = deltaLine == 0 ? thisCol - prevCol : thisCol;

        result.add(deltaLine);
        result.add(deltaCol);
        result.add((uint32_t)tokens[i].length);
        result.add((uint32_t)tokens[i].type);
        result.add(0);

        prevLine = thisLine;
        prevCol = thisCol;
    }

    return result;
}

} // namespace Slang
