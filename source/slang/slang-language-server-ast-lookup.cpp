#include "slang-language-server-ast-lookup.h"
#include "slang-visitor.h"

namespace Slang
{
struct Loc
{
    Int line;
    Int col;
    bool operator<(const Loc& other)
    {
        return line < other.line || line == other.line && col < other.col;
    }
    bool operator<=(const Loc& other)
    {
        return line < other.line || line == other.line && col <= other.col;
    }
};
struct ASTLookupContext
{
    SourceManager* sourceManager;
    List<SyntaxNode*> nodePath;
    ASTLookupType findType;
    Int line;
    Int col;
    Loc cursorLoc;
    UnownedStringSlice sourceFileName;
    List<ASTLookupResult> results;

    Loc getLoc(SourceLoc loc, String* outFileName)
    {
        auto humaneLoc = sourceManager->getHumaneLoc(loc, SourceLocType::Actual);
        if (outFileName)
            *outFileName = humaneLoc.pathInfo.foundPath;
        return Loc{humaneLoc.line, humaneLoc.column};
    }
};
struct PushNode
{
    ASTLookupContext* context;
    PushNode(ASTLookupContext* ctx, SyntaxNode* node)
    {
        context = ctx;
        context->nodePath.add(node);
    }
    ~PushNode() { if (context) context->nodePath.removeLast(); }
};

static Index _getDeclNameLength(Name* name)
{
    if (!name)
        return 0;
    if (name->text.startsWith("$"))
        return 0;
    // HACK: our __subscript functions currently have a name "operator[]".
    // Since this isn't the name that actually appears in user's code,
    // we need to shorten its reported length to 1 for now.
    if (name->text.startsWith("operator"))
    {
        return 1;
    }
    return name->text.getLength();
}

bool _isLocInRange(ASTLookupContext* context, SourceLoc loc, Int length)
{
    auto humaneLoc = context->sourceManager->getHumaneLoc(loc, SourceLocType::Actual);
    return humaneLoc.line == context->line && context->col >= humaneLoc.column &&
           context->col <= humaneLoc.column + length &&
           humaneLoc.pathInfo.foundPath.getUnownedSlice().endsWithCaseInsensitive(
               context->sourceFileName);
}
bool _isLocInRange(ASTLookupContext* context, SourceLoc start, SourceLoc end)
{
    auto startLoc = context->sourceManager->getHumaneLoc(start, SourceLocType::Actual);
    auto endLoc = context->sourceManager->getHumaneLoc(end, SourceLocType::Actual);
    
    Loc s{startLoc.line, startLoc.column};
    Loc e{endLoc.line, endLoc.column};
    Loc c{context->line, context->col};
    return s <= c && c <= e;
}

bool _findAstNodeImpl(ASTLookupContext& context, SyntaxNode* node);

struct ASTLookupExprVisitor: public ExprVisitor<ASTLookupExprVisitor, bool>
{
public:
    ASTLookupContext* context;

    ASTLookupExprVisitor(ASTLookupContext* ctx)
        : context(ctx)
    {}
    bool dispatchIfNotNull(Expr* expr)
    {
        if (!expr)
            return false;
        return dispatch(expr);
    }
    bool visitExpr(Expr*) { return false; }
    bool visitBoolLiteralExpr(BoolLiteralExpr*) { return false; }
    bool visitNullPtrLiteralExpr(NullPtrLiteralExpr*) { return false; }
    bool visitIntegerLiteralExpr(IntegerLiteralExpr*) { return false; }
    bool visitFloatingPointLiteralExpr(FloatingPointLiteralExpr*) { return false; }
    bool visitStringLiteralExpr(StringLiteralExpr*) { return false; }
    bool visitIncompleteExpr(IncompleteExpr*) { return false; }
    bool visitIndexExpr(IndexExpr* subscriptExpr)
    {
        if (dispatchIfNotNull(subscriptExpr->indexExpression))
            return true;
        return dispatchIfNotNull(subscriptExpr->baseExpression);
    }

    bool visitParenExpr(ParenExpr* expr)
    {
        return dispatchIfNotNull(expr->base);
    }

    bool visitAssignExpr(AssignExpr* expr)
    {
        if (dispatchIfNotNull(expr->left))
            return true;
        return dispatchIfNotNull(expr->right);
    }

    bool visitGenericAppExpr(GenericAppExpr* genericAppExpr)
    {
        if (dispatchIfNotNull(genericAppExpr->functionExpr))
            return true;
        for (auto arg : genericAppExpr->arguments)
            if (dispatchIfNotNull(arg))
                return true;
        return false;
    }

    bool visitSharedTypeExpr(SharedTypeExpr* expr) { return dispatchIfNotNull(expr->base.exp); }

    bool visitTaggedUnionTypeExpr(TaggedUnionTypeExpr*)
    {
        return false;
    }

    bool visitInvokeExpr(InvokeExpr* expr)
    {
        PushNode pushNodeRAII(context, expr);
        if (dispatchIfNotNull(expr->functionExpr))
            return true;
        for (auto arg : expr->arguments)
            if (dispatchIfNotNull(arg))
                return true;
        if (context->findType == ASTLookupType::Invoke && expr->argumentDelimeterLocs.getCount())
        {
            String fileName;
            Loc start = context->getLoc(expr->argumentDelimeterLocs.getFirst(), &fileName);
            Loc end = context->getLoc(expr->argumentDelimeterLocs.getLast(), nullptr);
            if (fileName.getUnownedSlice().endsWithCaseInsensitive(context->sourceFileName) &&
                start < context->cursorLoc && context->cursorLoc <= end)
            {
                ASTLookupResult result;
                result.path = context->nodePath;
                result.path.add(expr);
                context->results.add(result);
                return true;
            }
        }
        return false;
    }

    bool visitVarExpr(VarExpr* expr)
    {
        if (expr->name && expr->declRef.getDecl() &&
            _isLocInRange(context, expr->loc, _getDeclNameLength(expr->name)))
        {
            if (expr->declRef.getDecl()->hasModifier<ImplicitConversionModifier>())
                return false;
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        return dispatchIfNotNull(expr->originalExpr);
    }

    bool visitTypeCastExpr(TypeCastExpr* expr)
    {
        if (dispatchIfNotNull(expr->functionExpr))
            return true;
        for (auto arg : expr->arguments)
            if (dispatchIfNotNull(arg))
                return true;
        return false;
    }

    bool visitDerefExpr(DerefExpr* expr) { return dispatchIfNotNull(expr->base); }
    bool visitMatrixSwizzleExpr(MatrixSwizzleExpr* expr)
    {
        if (_isLocInRange(context, expr->memberOpLoc, 0))
        {
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        return dispatchIfNotNull(expr->base);
    }
    bool visitSwizzleExpr(SwizzleExpr* expr)
    {
        if (_isLocInRange(context, expr->memberOpLoc, 0))
        {
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        return dispatchIfNotNull(expr->base);
    }
    bool visitOverloadedExpr(OverloadedExpr* expr)
    {
        if (dispatchIfNotNull(expr->base))
            return true;
        if (expr->lookupResult2.getName() &&
            _isLocInRange(
                context,
                expr->loc,
                _getDeclNameLength(expr->lookupResult2.getName())))
        {
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        return false;
    }
    bool visitOverloadedExpr2(OverloadedExpr2* expr)
    {
        if (dispatchIfNotNull(expr->base))
            return true;
        bool result = false;
        for (auto candidate : expr->candidiateExprs)
        {
            result |= dispatchIfNotNull(candidate);
        }
        return result;
    }
    bool visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
    {
        if (dispatchIfNotNull(expr->base.exp))
            return true;
        for (auto arg : expr->arguments)
        {
            if (dispatchIfNotNull(arg))
                return true;
        }
        return false;
    }
    bool visitCastToSuperTypeExpr(CastToSuperTypeExpr* expr)
    {
        return dispatchIfNotNull(expr->valueArg);
    }
    bool visitModifierCastExpr(ModifierCastExpr* expr) { return dispatchIfNotNull(expr->valueArg); }
    bool visitLetExpr(LetExpr* expr)
    {
        if (dispatchIfNotNull(expr->body))
            return true;
        return _findAstNodeImpl(*context, expr->decl);
    }
    bool visitExtractExistentialValueExpr(ExtractExistentialValueExpr* expr)
    {
        if (expr->declRef.getDecl() && expr->declRef.getName() &&
            _isLocInRange(
                context, expr->loc, _getDeclNameLength(expr->declRef.getName())))
        {
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        return false;
    }

    bool visitDeclRefExpr(DeclRefExpr* expr)
    {
        if (expr->declRef.getDecl() && expr->declRef.getDecl()->getName() &&
            _isLocInRange(
                context,
                expr->loc,
                _getDeclNameLength(expr->declRef.getDecl()->getName())))
        {
            if (expr->declRef.getDecl()->hasModifier<ImplicitConversionModifier>())
                return false;
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        return false;
    }

    bool visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        if (_isLocInRange(context, expr->memberOperatorLoc, 0))
        {
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        if (visitDeclRefExpr(expr))
            return true;
        return dispatchIfNotNull(expr->baseExpression);
    }

    bool visitMemberExpr(MemberExpr* expr)
    {
        if (_isLocInRange(context, expr->memberOperatorLoc, 0))
        {
            ASTLookupResult result;
            result.path = context->nodePath;
            result.path.add(expr);
            context->results.add(result);
            return true;
        }
        if (visitDeclRefExpr(expr)) return true;
        return dispatchIfNotNull(expr->baseExpression);
    }

    bool visitInitializerListExpr(InitializerListExpr* expr)
    {
        for (auto arg : expr->args)
        {
            if (dispatchIfNotNull(arg))
                return true;
        }
        return false;
    }

    bool visitThisExpr(ThisExpr*) { return false; }
    bool visitThisTypeExpr(ThisTypeExpr*) { return false; }
    bool visitAndTypeExpr(AndTypeExpr* expr)
    {
        if (dispatchIfNotNull(expr->left.exp))
            return true;
        return dispatchIfNotNull(expr->right.exp);
    }
    bool visitModifiedTypeExpr(ModifiedTypeExpr* expr) { return dispatchIfNotNull(expr->base.exp); }
    bool visitTryExpr(TryExpr* expr) { return dispatchIfNotNull(expr->base); }

};

struct ASTLookupStmtVisitor : public StmtVisitor<ASTLookupStmtVisitor, bool>
{
    ASTLookupContext* context;

    ASTLookupStmtVisitor(ASTLookupContext* ctx)
        : context(ctx)
    {}

    bool dispatchIfNotNull(Stmt* stmt)
    {
        if (!stmt)
            return false;
        return dispatch(stmt);
    }

    bool checkExpr(Expr* expr)
    {
        if (!expr)
            return false;
        ASTLookupExprVisitor visitor(context);
        return visitor.dispatch(expr);
    }

    bool visitDeclStmt(DeclStmt* stmt) { return _findAstNodeImpl(*context, stmt->decl); }

    bool visitBlockStmt(BlockStmt* stmt)
    {
        if (!_isLocInRange(context, stmt->loc, stmt->closingSourceLoc))
            return false;
        return dispatchIfNotNull(stmt->body);
    }

    bool visitSeqStmt(SeqStmt* seqStmt)
    {
        for (auto stmt : seqStmt->stmts)
            if (dispatchIfNotNull(stmt))
                return true;
        return false;
    }

    bool visitBreakStmt(BreakStmt*) { return false; }

    bool visitContinueStmt(ContinueStmt*) { return false; }

    bool visitDoWhileStmt(DoWhileStmt* stmt)
    {
        if (checkExpr(stmt->predicate))
            return true;
        return dispatchIfNotNull(stmt->statement);
    }

    bool visitForStmt(ForStmt* stmt)
    {
        if (dispatchIfNotNull(stmt->initialStatement))
            return true;
        if (checkExpr(stmt->predicateExpression))
            return true;
        if (checkExpr(stmt->sideEffectExpression))
            return true;
        return dispatchIfNotNull(stmt->statement);
    }

    bool visitCompileTimeForStmt(CompileTimeForStmt*)
    {
        return false;
    }

    bool visitSwitchStmt(SwitchStmt* stmt)
    {
        if (checkExpr(stmt->condition))
            return true;
        return dispatchIfNotNull(stmt->body);
    }

    bool visitCaseStmt(CaseStmt* stmt) { return checkExpr(stmt->expr); }

    bool visitDefaultStmt(DefaultStmt*) { return false; }

    bool visitIfStmt(IfStmt* stmt)
    {
        if (checkExpr(stmt->predicate))
            return true;
        if (dispatchIfNotNull(stmt->positiveStatement))
            return true;
        return dispatchIfNotNull(stmt->negativeStatement);
    }

    bool visitUnparsedStmt(UnparsedStmt*) { return false; }

    bool visitEmptyStmt(EmptyStmt*) { return false; }

    bool visitDiscardStmt(DiscardStmt*) { return false; }

    bool visitReturnStmt(ReturnStmt* stmt) { return checkExpr(stmt->expression); }

    bool visitWhileStmt(WhileStmt* stmt)
    {
        if (checkExpr(stmt->predicate))
            return true;
        return dispatchIfNotNull(stmt->statement);
    }

    bool visitGpuForeachStmt(GpuForeachStmt*) { return false; }

    bool visitExpressionStmt(ExpressionStmt* stmt)
    {
        return checkExpr(stmt->expression);
    }
};

bool _findAstNodeImpl(ASTLookupContext& context, SyntaxNode* node)
{
    if (!node)
        return false;
    PushNode pushNodeRAII(&context, node);
    if (auto decl = as<Decl>(node))
    {
        if (decl->getName())
        {
            if (_isLocInRange(
                    &context,
                    decl->nameAndLoc.loc,
                    _getDeclNameLength(decl->getName())))
            {
                ASTLookupResult result;
                result.path = context.nodePath;
                context.results.add(_Move(result));
                return true;
            }
        }
        if (auto funcDecl = as<FunctionDeclBase>(node))
        {
            ASTLookupStmtVisitor visitor(&context);
            if (visitor.dispatchIfNotNull(funcDecl->body))
                return true;
            ASTLookupExprVisitor exprVisitor(&context);
            if (exprVisitor.dispatchIfNotNull(funcDecl->returnType.exp))
                return true;
        }
        else if (auto propertyDecl = as<PropertyDecl>(node))
        {
            ASTLookupExprVisitor exprVisitor(&context);
            if (exprVisitor.dispatchIfNotNull(propertyDecl->type.exp))
                return true;
        }
        else if (auto varDecl = as<VarDeclBase>(node))
        {
            ASTLookupExprVisitor visitor(&context);
            if (visitor.dispatchIfNotNull(varDecl->type.exp))
                return true;
            if (visitor.dispatchIfNotNull(varDecl->initExpr))
                return true;
        }
        else if (auto genericDecl = as<GenericDecl>(node))
        {
            if (_findAstNodeImpl(context, genericDecl->inner))
                return true;
        }
        else if (auto typeConstraint = as<TypeConstraintDecl>(node))
        {
            ASTLookupExprVisitor visitor(&context);
            if (visitor.dispatchIfNotNull(typeConstraint->getSup().exp))
                return true;
        }
        else if (auto typedefDecl = as<TypeDefDecl>(node))
        {
            ASTLookupExprVisitor visitor(&context);
            if (visitor.dispatchIfNotNull(typedefDecl->type.exp))
                return true;
        }
        if (auto container = as<ContainerDecl>(node))
        {
            bool shouldInspectChildren = true;
            if (auto genericDecl = as<GenericDecl>(node))
            {}
            else if (container->closingSourceLoc.getRaw() >= container->loc.getRaw())
            {
                if (!_isLocInRange(&context, container->loc, container->closingSourceLoc))
                {
                    shouldInspectChildren = false;
                }
            }
            if (shouldInspectChildren)
            {
                for (auto member : container->members)
                {
                    if (_findAstNodeImpl(context, member))
                        return true;
                }
            }
        }
    }
    return false;
}

List<ASTLookupResult> findASTNodesAt(
    SourceManager* sourceManager, ModuleDecl* moduleDecl, ASTLookupType findType, UnownedStringSlice fileName, Int line, Int col)
{
    ASTLookupContext context;
    context.sourceManager = sourceManager;
    context.line = line;
    context.col = col;
    context.cursorLoc = Loc{line, col};
    context.findType = findType;
    context.sourceFileName = fileName;
    _findAstNodeImpl(context, moduleDecl);
    return context.results;
}

} // namespace Slang
