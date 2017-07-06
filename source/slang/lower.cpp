// lower.cpp
#include "lower.h"

#include "visitor.h"

namespace Slang
{

//

template<typename V>
struct StructuralTransformVisitorBase
{
    V* visitor;

    RefPtr<StatementSyntaxNode> transformDeclField(StatementSyntaxNode* stmt)
    {
        return visitor->translateStmtRef(stmt);
    }

    RefPtr<Decl> transformDeclField(Decl* decl)
    {
        return visitor->translateDeclRef(decl);
    }

    template<typename T>
    DeclRef<T> transformDeclField(DeclRef<T> const& decl)
    {
        return visitor->translateDeclRef(decl).As<T>();
    }

    TypeExp transformSyntaxField(TypeExp const& typeExp)
    {
        TypeExp result;
        result.type = visitor->transformSyntaxField(typeExp.type);
        return result;
    }

    QualType transformSyntaxField(QualType const& qualType)
    {
        QualType result = qualType;
        result.type = visitor->transformSyntaxField(qualType.type);
        return result;
    }

    RefPtr<ExpressionSyntaxNode> transformSyntaxField(ExpressionSyntaxNode* expr)
    {
        return visitor->transformSyntaxField(expr);
    }

    RefPtr<StatementSyntaxNode> transformSyntaxField(StatementSyntaxNode* stmt)
    {
        return visitor->transformSyntaxField(stmt);
    }

    RefPtr<DeclBase> transformSyntaxField(DeclBase* decl)
    {
        return visitor->transformSyntaxField(decl);
    }

    RefPtr<ScopeDecl> transformSyntaxField(ScopeDecl* decl)
    {
        if(!decl) return nullptr;
        return visitor->transformSyntaxField(decl).As<ScopeDecl>();
    }

    template<typename T>
    List<T> transformSyntaxField(List<T> const& list)
    {
        List<T> result;
        for (auto item : list)
        {
            result.Add(transformSyntaxField(item));
        }
        return result;
    }
};

template<typename V>
struct StructuralTransformStmtVisitor
    : StructuralTransformVisitorBase<V>
    , StmtVisitor<StructuralTransformStmtVisitor<V>, RefPtr<StatementSyntaxNode>>
{
    void transformFields(StatementSyntaxNode*, StatementSyntaxNode*)
    {
    }

#define SYNTAX_CLASS(NAME, BASE, ...)                   \
    RefPtr<StatementSyntaxNode> visit(NAME* obj) {      \
        RefPtr<NAME> result = new NAME(*obj);           \
        transformFields(result, obj);                   \
        return result;                                  \
    }                                                   \
    void transformFields(NAME* result, NAME* obj) {     \
        transformFields((BASE*) result, (BASE*) obj);   \

#define SYNTAX_FIELD(TYPE, NAME)    result->NAME = this->transformSyntaxField(obj->NAME);
#define DECL_FIELD(TYPE, NAME)      result->NAME = this->transformDeclField(obj->NAME);

#define FIELD(TYPE, NAME) /* empty */

#define END_SYNTAX_CLASS()  \
    }

#include "object-meta-begin.h"
#include "stmt-defs.h"
#include "object-meta-end.h"

};

template<typename V>
RefPtr<StatementSyntaxNode> structuralTransform(
    StatementSyntaxNode*    stmt,
    V*                      visitor)
{
    StructuralTransformStmtVisitor<V> transformer;
    transformer.visitor = visitor;
    return transformer.dispatch(stmt);
}

template<typename V>
struct StructuralTransformExprVisitor
    : StructuralTransformVisitorBase<V>
    , ExprVisitor<StructuralTransformExprVisitor<V>, RefPtr<ExpressionSyntaxNode>>
{
    void transformFields(ExpressionSyntaxNode* result, ExpressionSyntaxNode* obj)
    {
        result->Type = transformSyntaxField(obj->Type);
    }


#define SYNTAX_CLASS(NAME, BASE, ...)                   \
    RefPtr<ExpressionSyntaxNode> visit(NAME* obj) {     \
        RefPtr<NAME> result = new NAME(*obj);           \
        transformFields(result, obj);                   \
        return result;                                  \
    }                                                   \
    void transformFields(NAME* result, NAME* obj) {     \
        transformFields((BASE*) result, (BASE*) obj);   \

#define SYNTAX_FIELD(TYPE, NAME)    result->NAME = transformSyntaxField(obj->NAME);
#define DECL_FIELD(TYPE, NAME)      result->NAME = transformDeclField(obj->NAME);

#define FIELD(TYPE, NAME) /* empty */

#define END_SYNTAX_CLASS()  \
    }

#include "object-meta-begin.h"
#include "expr-defs.h"
#include "object-meta-end.h"
};


template<typename V>
RefPtr<ExpressionSyntaxNode> structuralTransform(
    ExpressionSyntaxNode*   expr,
    V*                      visitor)
{
    StructuralTransformExprVisitor<V> transformer;
    transformer.visitor = visitor;
    return transformer.dispatch(expr);
}

//

// Pseudo-syntax used during lowering
class TupleDecl : public VarDeclBase
{
public:
    virtual void accept(IDeclVisitor *, void *) override
    {
        throw "unexpected";
    }

    List<RefPtr<VarDeclBase>> decls;
};

// Pseudo-syntax used during lowering:
// represents an ordered list of expressions as a single unit
class TupleExpr : public ExpressionSyntaxNode
{
public:
    virtual void accept(IExprVisitor *, void *) override
    {
        throw "unexpected";
    }

    List<RefPtr<ExpressionSyntaxNode>> exprs;
};

struct SharedLoweringContext
{
    ProgramLayout*  programLayout;
    CodeGenTarget   target;

    RefPtr<ProgramSyntaxNode>   loweredProgram;

    Dictionary<Decl*, RefPtr<Decl>> loweredDecls;
    Dictionary<Decl*, Decl*> mapLoweredDeclToOriginal;

    bool isRewrite;
};

static void attachLayout(
    ModifiableSyntaxNode*   syntax,
    Layout*                 layout)
{
    RefPtr<ComputedLayoutModifier> modifier = new ComputedLayoutModifier();
    modifier->layout = layout;

    addModifier(syntax, modifier);
}

struct LoweringVisitor
    : ExprVisitor<LoweringVisitor, RefPtr<ExpressionSyntaxNode>>
    , StmtVisitor<LoweringVisitor, void>
    , DeclVisitor<LoweringVisitor, RefPtr<Decl>>
    , TypeVisitor<LoweringVisitor, RefPtr<ExpressionType>>
    , ValVisitor<LoweringVisitor, RefPtr<Val>>
{
    //
    SharedLoweringContext*      shared;
    RefPtr<Substitutions>       substitutions;

    bool                        isBuildingStmt = false;
    RefPtr<StatementSyntaxNode> stmtBeingBuilt;

    // If we *aren't* building a statement, then this
    // is the container we should be adding declarations to
    RefPtr<ContainerDecl>       parentDecl;

    // If we are in a context where a `return` should be turned
    // into assignment to a variable (followed by a `return`),
    // then this will point to that variable.
    RefPtr<Variable>            resultVariable;

    CodeGenTarget getTarget() { return shared->target; }

    //
    // Values
    //

    RefPtr<Val> lowerVal(Val* val)
    {
        if (!val) return nullptr;
        return ValVisitor::dispatch(val);
    }

    RefPtr<Val> visit(GenericParamIntVal* val)
    {
        return new GenericParamIntVal(translateDeclRef(DeclRef<Decl>(val->declRef)).As<VarDeclBase>());
    }

    RefPtr<Val> visit(ConstantIntVal* val)
    {
        return val;
    }

    //
    // Types
    //

    RefPtr<ExpressionType> lowerType(
        ExpressionType* type)
    {
        if(!type) return nullptr;
        return TypeVisitor::dispatch(type);
    }

    TypeExp lowerType(
        TypeExp const& typeExp)
    {
        TypeExp result;
        result.type = lowerType(typeExp.type);
        return result;
    }

    RefPtr<ExpressionType> visit(ErrorType* type)
    {
        return type;
    }

    RefPtr<ExpressionType> visit(OverloadGroupType* type)
    {
        return type;
    }

    RefPtr<ExpressionType> visit(InitializerListType* type)
    {
        return type;
    }

    RefPtr<ExpressionType> visit(GenericDeclRefType* type)
    {
        return new GenericDeclRefType(translateDeclRef(DeclRef<Decl>(type->declRef)).As<GenericDecl>());
    }

    RefPtr<ExpressionType> visit(FuncType* type)
    {
        RefPtr<FuncType> loweredType = new FuncType();
        loweredType->declRef = translateDeclRef(DeclRef<Decl>(type->declRef)).As<CallableDecl>();
        return loweredType;
    }

    RefPtr<ExpressionType> visit(DeclRefType* type)
    {
        auto loweredDeclRef = translateDeclRef(type->declRef);
        return DeclRefType::Create(loweredDeclRef);
    }

    RefPtr<ExpressionType> visit(NamedExpressionType* type)
    {
        if (shared->target == CodeGenTarget::GLSL)
        {
            // GLSL does not support `typedef`, so we will lower it out of existence here
            return lowerType(GetType(type->declRef));
        }

        return new NamedExpressionType(translateDeclRef(DeclRef<Decl>(type->declRef)).As<TypeDefDecl>());
    }

    RefPtr<ExpressionType> visit(TypeType* type)
    {
        return new TypeType(lowerType(type->type));
    }

    RefPtr<ExpressionType> visit(ArrayExpressionType* type)
    {
        RefPtr<ArrayExpressionType> loweredType = new ArrayExpressionType();
        loweredType->BaseType = lowerType(type->BaseType);
        loweredType->ArrayLength = lowerVal(type->ArrayLength).As<IntVal>();
        return loweredType;
    }

    RefPtr<ExpressionType> transformSyntaxField(ExpressionType* type)
    {
        return lowerType(type);
    }

    //
    // Expressions
    //

    RefPtr<ExpressionSyntaxNode> lowerExpr(
        ExpressionSyntaxNode* expr)
    {
        if (!expr) return nullptr;
        return ExprVisitor::dispatch(expr);
    }

    // catch-all
    RefPtr<ExpressionSyntaxNode> visit(
        ExpressionSyntaxNode* expr)
    {
        return structuralTransform(expr, this);
    }

    RefPtr<ExpressionSyntaxNode> transformSyntaxField(ExpressionSyntaxNode* expr)
    {
        return lowerExpr(expr);
    }

    void lowerExprCommon(
        RefPtr<ExpressionSyntaxNode>    loweredExpr,
        RefPtr<ExpressionSyntaxNode>    expr)
    {
        loweredExpr->Position = expr->Position;
        loweredExpr->Type.type = lowerType(expr->Type.type);
    }

    RefPtr<ExpressionSyntaxNode> createVarRef(
        CodePosition const& loc,
        VarDeclBase*        decl)
    {
        if (auto tupleDecl = dynamic_cast<TupleDecl*>(decl))
        {
            return createTupleRef(loc, tupleDecl);
        }
        else
        {
            RefPtr<VarExpressionSyntaxNode> result = new VarExpressionSyntaxNode();
            result->Position = loc;
            result->Type.type = decl->Type.type;
            result->declRef = makeDeclRef(decl);
            return result;
        }
    }

    RefPtr<ExpressionSyntaxNode> createTupleRef(
        CodePosition const& loc,
        TupleDecl*          decl)
    {
        RefPtr<TupleExpr> result = new TupleExpr();
        result->Position = loc;
        result->Type.type = decl->Type.type;

        for (auto dd : decl->decls)
        {
            auto expr = createVarRef(loc, dd);
            result->exprs.Add(expr);
        }

        return result;
    }

    RefPtr<ExpressionSyntaxNode> visit(
        VarExpressionSyntaxNode* expr)
    {
        // If the expression didn't get resolved, we can leave it as-is
        if (!expr->declRef)
            return expr;

        auto loweredDeclRef = translateDeclRef(expr->declRef);
        auto loweredDecl = loweredDeclRef.getDecl();

        if (auto tupleDecl = dynamic_cast<TupleDecl*>(loweredDecl))
        {
            // If we are referencing a declaration that got tuple-ified,
            // then we need to produce a tuple expression as well.

            return createTupleRef(expr->Position, tupleDecl);
        }

        RefPtr<VarExpressionSyntaxNode> loweredExpr = new VarExpressionSyntaxNode();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->declRef = loweredDeclRef;
        return loweredExpr;
    }

    RefPtr<ExpressionSyntaxNode> visit(
        MemberExpressionSyntaxNode* expr)
    {
        auto loweredBase = lowerExpr(expr->BaseExpression);

        // Are we extracting an element from a tuple?
        if (auto baseTuple = loweredBase.As<TupleExpr>())
        {
            // We need to find the correct member expression,
            // based on the actual tuple type.

            throw "unimplemented";
        }

        // Default handling:
        auto loweredDeclRef = translateDeclRef(expr->declRef);
        assert(!dynamic_cast<TupleDecl*>(loweredDeclRef.getDecl()));

        RefPtr<MemberExpressionSyntaxNode> loweredExpr = new MemberExpressionSyntaxNode();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->BaseExpression = loweredBase;
        loweredExpr->declRef = loweredDeclRef;

        return loweredExpr;
    }

    //
    // Statements
    //

    // Lowering one statement to another.
    // The source statement might desugar into multiple statements,
    // (or event to none), and in such a case this function wraps
    // the result up as a `SeqStmt` or `EmptyStmt` as appropriate.
    //
    RefPtr<StatementSyntaxNode> lowerStmt(
        StatementSyntaxNode* stmt)
    {
        if(!stmt)
            return nullptr;

        LoweringVisitor subVisitor = *this;
        subVisitor.stmtBeingBuilt = nullptr;

        subVisitor.lowerStmtImpl(stmt);

        if( !subVisitor.stmtBeingBuilt )
        {
            return new EmptyStatementSyntaxNode();
        }
        else
        {
            return subVisitor.stmtBeingBuilt;
        }
    }


    // Structure to track "outer" statements during lowering
    struct StmtLoweringState
    {
        // The next "outer" statement entry
        StmtLoweringState*      parent = nullptr;

        // The outer statement (both lowered and original)
        StatementSyntaxNode*    loweredStmt = nullptr;
        StatementSyntaxNode*    originalStmt = nullptr;
    };
    StmtLoweringState stmtLoweringState;

    // Translate a reference from one statement to an outer statement
    StatementSyntaxNode* translateStmtRef(
        StatementSyntaxNode* originalStmt)
    {
        if(!originalStmt) return nullptr;

        for( auto state = &stmtLoweringState; state; state = state->parent )
        {
            if(state->originalStmt == originalStmt)
                return state->loweredStmt;
        }

        assert(!"unexepcted");

        return nullptr;
    }

    // Expand a statement to be lowered into one or more statements
    void lowerStmtImpl(
        StatementSyntaxNode* stmt)
    {
        StmtVisitor::dispatch(stmt);
    }

    RefPtr<ScopeDecl> visit(ScopeDecl* decl)
    {
        RefPtr<ScopeDecl> loweredDecl = new ScopeDecl();
        lowerDeclCommon(loweredDecl, decl);
        return loweredDecl;
    }

    LoweringVisitor pushScope(
        RefPtr<ScopeStmt>   loweredStmt,
        RefPtr<ScopeStmt>   originalStmt)
    {
        loweredStmt->scopeDecl = translateDeclRef(originalStmt->scopeDecl).As<ScopeDecl>();

        LoweringVisitor subVisitor = *this;
        subVisitor.isBuildingStmt = true;
        subVisitor.stmtBeingBuilt = nullptr;
        subVisitor.parentDecl = loweredStmt->scopeDecl;
        subVisitor.stmtLoweringState.parent = &stmtLoweringState;
        subVisitor.stmtLoweringState.originalStmt = originalStmt;
        subVisitor.stmtLoweringState.loweredStmt = loweredStmt;
        return subVisitor;
    }

    void addStmtImpl(
        RefPtr<StatementSyntaxNode>&    dest,
        StatementSyntaxNode*            stmt)
    {
        // add a statement to the code we are building...
        if( !dest )
        {
            dest = stmt;
            return;
        }

        if (auto blockStmt = dest.As<BlockStmt>())
        {
            addStmtImpl(blockStmt->body, stmt);
            return;
        }

        if (auto seqStmt = dest.As<SeqStmt>())
        {
            seqStmt->stmts.Add(stmt);
        }
        else
        {
            RefPtr<SeqStmt> newSeqStmt = new SeqStmt();

            newSeqStmt->stmts.Add(dest);
            newSeqStmt->stmts.Add(stmt);

            dest = newSeqStmt;
        }

    }

    void addStmt(
        StatementSyntaxNode* stmt)
    {
        addStmtImpl(stmtBeingBuilt, stmt);
    }

    void addExprStmt(
        RefPtr<ExpressionSyntaxNode>    expr)
    {
        // TODO: handle cases where the `expr` cannot be directly
        // represented as a single statement

        RefPtr<ExpressionStatementSyntaxNode> stmt = new ExpressionStatementSyntaxNode();
        stmt->Expression = expr;
        addStmt(stmt);
    }

    void visit(BlockStmt* stmt)
    {
        RefPtr<BlockStmt> loweredStmt = new BlockStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->body = subVisitor.lowerStmt(stmt->body);

        addStmt(loweredStmt);
    }

    void visit(SeqStmt* stmt)
    {
        for( auto ss : stmt->stmts )
        {
            lowerStmtImpl(ss);
        }
    }

    void visit(ExpressionStatementSyntaxNode* stmt)
    {
        addExprStmt(lowerExpr(stmt->Expression));
    }

    void visit(VarDeclrStatementSyntaxNode* stmt)
    {
        DeclVisitor::dispatch(stmt->decl);
    }

    void lowerStmtFields(
        StatementSyntaxNode* loweredStmt,
        StatementSyntaxNode* originalStmt)
    {
        loweredStmt->Position = originalStmt->Position;
        loweredStmt->modifiers = originalStmt->modifiers;
    }

    void lowerScopeStmtFields(
        ScopeStmt* loweredStmt,
        ScopeStmt* originalStmt)
    {
        lowerStmtFields(loweredStmt, originalStmt);
        loweredStmt->scopeDecl = translateDeclRef(originalStmt->scopeDecl).As<ScopeDecl>();
    }

    // Child statements reference their parent statement,
    // so we need to translate that cross-reference
    void lowerChildStmtFields(
        ChildStmt* loweredStmt,
        ChildStmt* originalStmt)
    {
        lowerStmtFields(loweredStmt, originalStmt);

        loweredStmt->parentStmt = translateStmtRef(originalStmt->parentStmt);
    }

    void visit(ContinueStatementSyntaxNode* stmt)
    {
        RefPtr<ContinueStatementSyntaxNode> loweredStmt = new ContinueStatementSyntaxNode();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visit(BreakStatementSyntaxNode* stmt)
    {
        RefPtr<BreakStatementSyntaxNode> loweredStmt = new BreakStatementSyntaxNode();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visit(DefaultStmt* stmt)
    {
        RefPtr<DefaultStmt> loweredStmt = new DefaultStmt();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visit(DiscardStatementSyntaxNode* stmt)
    {
        RefPtr<DiscardStatementSyntaxNode> loweredStmt = new DiscardStatementSyntaxNode();
        lowerStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visit(EmptyStatementSyntaxNode* stmt)
    {
        RefPtr<EmptyStatementSyntaxNode> loweredStmt = new EmptyStatementSyntaxNode();
        lowerStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visit(UnparsedStmt* stmt)
    {
        RefPtr<UnparsedStmt> loweredStmt = new UnparsedStmt();
        lowerStmtFields(loweredStmt, stmt);

        loweredStmt->tokens = stmt->tokens;

        addStmt(loweredStmt);
    }

    void visit(CaseStmt* stmt)
    {
        RefPtr<CaseStmt> loweredStmt = new CaseStmt();
        lowerChildStmtFields(loweredStmt, stmt);

        loweredStmt->expr = lowerExpr(stmt->expr);

        addStmt(loweredStmt);
    }

    void visit(IfStatementSyntaxNode* stmt)
    {
        RefPtr<IfStatementSyntaxNode> loweredStmt = new IfStatementSyntaxNode();
        lowerStmtFields(loweredStmt, stmt);

        loweredStmt->Predicate          = lowerExpr(stmt->Predicate);
        loweredStmt->PositiveStatement  = lowerStmt(stmt->PositiveStatement);
        loweredStmt->NegativeStatement  = lowerStmt(stmt->NegativeStatement);

        addStmt(loweredStmt);
    }

    void visit(SwitchStmt* stmt)
    {
        RefPtr<SwitchStmt> loweredStmt = new SwitchStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->condition  = subVisitor.lowerExpr(stmt->condition);
        loweredStmt->body       = subVisitor.lowerStmt(stmt->body);

        addStmt(loweredStmt);
    }


    void visit(ForStatementSyntaxNode* stmt)
    {
        RefPtr<ForStatementSyntaxNode> loweredStmt = new ForStatementSyntaxNode();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->InitialStatement       = subVisitor.lowerStmt(stmt->InitialStatement);
        loweredStmt->SideEffectExpression   = subVisitor.lowerExpr(stmt->SideEffectExpression);
        loweredStmt->PredicateExpression    = subVisitor.lowerExpr(stmt->PredicateExpression);
        loweredStmt->Statement              = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visit(WhileStatementSyntaxNode* stmt)
    {
        RefPtr<WhileStatementSyntaxNode> loweredStmt = new WhileStatementSyntaxNode();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Predicate  = subVisitor.lowerExpr(stmt->Predicate);
        loweredStmt->Statement  = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visit(DoWhileStatementSyntaxNode* stmt)
    {
        RefPtr<DoWhileStatementSyntaxNode> loweredStmt = new DoWhileStatementSyntaxNode();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Statement  = subVisitor.lowerStmt(stmt->Statement);
        loweredStmt->Predicate  = subVisitor.lowerExpr(stmt->Predicate);

        addStmt(loweredStmt);
    }

    RefPtr<StatementSyntaxNode> transformSyntaxField(StatementSyntaxNode* stmt)
    {
        return lowerStmt(stmt);
    }

    void lowerStmtCommon(StatementSyntaxNode* loweredStmt, StatementSyntaxNode* stmt)
    {
        loweredStmt->modifiers = stmt->modifiers;
    }

    void assign(
        RefPtr<ExpressionSyntaxNode> destExpr,
        RefPtr<ExpressionSyntaxNode> srcExpr)
    {
        RefPtr<AssignExpr> assignExpr = new AssignExpr();
        assignExpr->Position = destExpr->Position;
        assignExpr->left = destExpr;
        assignExpr->right = srcExpr;

        addExprStmt(assignExpr);
    }

    void assign(VarDeclBase* varDecl, RefPtr<ExpressionSyntaxNode> expr)
    {
        assign(createVarRef(expr->Position, varDecl), expr);
    }

    void assign(RefPtr<ExpressionSyntaxNode> expr, VarDeclBase* varDecl)
    {
        assign(expr, createVarRef(expr->Position, varDecl));
    }

    void visit(ReturnStatementSyntaxNode* stmt)
    {
        auto loweredStmt = new ReturnStatementSyntaxNode();
        lowerStmtCommon(loweredStmt, stmt);

        if (stmt->Expression)
        {
            if (resultVariable)
            {
                // Do it as an assignment
                assign(resultVariable, lowerExpr(stmt->Expression));
            }
            else
            {
                // Simple case
                loweredStmt->Expression = lowerExpr(stmt->Expression);
            }
        }

        addStmt(loweredStmt);
    }

    //
    // Declarations
    //

    RefPtr<Val> translateVal(Val* val)
    {
        if (auto type = dynamic_cast<ExpressionType*>(val))
            return lowerType(type);

        if (auto litVal = dynamic_cast<ConstantIntVal*>(val))
            return val;

        throw 99;
    }

    RefPtr<Substitutions> translateSubstitutions(
        Substitutions*  inSubstitutions)
    {
        if (!inSubstitutions) return nullptr;

        RefPtr<Substitutions> result = new Substitutions();
        result->genericDecl = translateDeclRef(inSubstitutions->genericDecl).As<GenericDecl>();
        for (auto arg : inSubstitutions->args)
        {
            result->args.Add(translateVal(arg));
        }
        return result;
    }

    static Decl* getModifiedDecl(Decl* decl)
    {
        if (!decl) return nullptr;
        if (auto genericDecl = dynamic_cast<GenericDecl*>(decl->ParentDecl))
            return genericDecl;
        return decl;
    }

    DeclRef<Decl> translateDeclRef(
        DeclRef<Decl> const& decl)
    {
        DeclRef<Decl> result;
        result.decl = translateDeclRef(decl.decl);
        result.substitutions = translateSubstitutions(decl.substitutions);
        return result;
    }

   RefPtr<Decl> translateDeclRef(
        Decl* decl)
    {
       if (!decl) return nullptr;

        // We don't want to translate references to built-in declarations,
        // since they won't be subtituted anyway.
        if (getModifiedDecl(decl)->HasModifier<FromStdLibModifier>())
            return decl;

        // If any parent of the declaration was in the stdlib, then
        // we need to skip it.
        for(auto pp = decl; pp; pp = pp->ParentDecl)
        {
            if (pp->HasModifier<FromStdLibModifier>())
                return decl;
        }

        if (getModifiedDecl(decl)->HasModifier<BuiltinModifier>())
            return decl;

        RefPtr<Decl> loweredDecl;
        if (shared->loweredDecls.TryGetValue(decl, loweredDecl))
            return loweredDecl;

        // Time to force it
        return lowerDecl(decl);
    }

   RefPtr<ContainerDecl> translateDeclRef(
       ContainerDecl* decl)
   {
       return translateDeclRef((Decl*)decl).As<ContainerDecl>();
   }

    RefPtr<DeclBase> lowerDeclBase(
        DeclBase* declBase)
    {
        if (Decl* decl = dynamic_cast<Decl*>(declBase))
        {
            return lowerDecl(decl);
        }
        else
        {
            return DeclVisitor::dispatch(declBase);
        }
    }

    RefPtr<Decl> lowerDecl(
        Decl* decl)
    {
        RefPtr<Decl> loweredDecl = DeclVisitor::dispatch(decl).As<Decl>();
        return loweredDecl;
    }

    static void addMember(
        RefPtr<ContainerDecl>   containerDecl,
        RefPtr<Decl>            memberDecl)
    {
        containerDecl->Members.Add(memberDecl);
        memberDecl->ParentDecl = containerDecl.Ptr();
    }

    void addDecl(
        Decl* decl)
    {
        if(isBuildingStmt)
        {
            RefPtr<VarDeclrStatementSyntaxNode> declStmt = new VarDeclrStatementSyntaxNode();
            declStmt->Position = decl->Position;
            declStmt->decl = decl;
            addStmt(declStmt);
        }


        // We will add the declaration to the current container declaration being
        // translated, which the user will maintain via pua/pop.
        //

        assert(parentDecl);
        addMember(parentDecl, decl);
    }

    void registerLoweredDecl(Decl* loweredDecl, Decl* decl)
    {
        shared->loweredDecls.Add(decl, loweredDecl);

        shared->mapLoweredDeclToOriginal.Add(loweredDecl, decl);
    }

    void lowerDeclCommon(
        Decl* loweredDecl,
        Decl* decl)
    {
        registerLoweredDecl(loweredDecl, decl);

        loweredDecl->Position = decl->Position;
        loweredDecl->Name = decl->getNameToken();

        // Lower modifiers as needed

        // HACK: just doing a shallow copy of modifiers, which will
        // suffice for most of them, but we need to do something
        // better soon.
        loweredDecl->modifiers = decl->modifiers;

        // deal with layout stuff

        auto loweredParent = translateDeclRef(decl->ParentDecl);
        if (loweredParent)
        {
            auto layoutMod = loweredParent->FindModifier<ComputedLayoutModifier>();
            if (layoutMod)
            {
                auto parentLayout = layoutMod->layout;
                if (auto structLayout = parentLayout.As<StructTypeLayout>())
                {
                    RefPtr<VarLayout> fieldLayout;
                    if (structLayout->mapVarToLayout.TryGetValue(decl, fieldLayout))
                    {
                        attachLayout(loweredDecl, fieldLayout);
                    }
                }

                // TODO: are there other cases to handle here?
            }
        }
    }

    // Catch-all

    RefPtr<Decl> visit(ModifierDecl*)
    {
        // should not occur in user code
        SLANG_UNEXPECTED("modifiers shouldn't occur in user code");
    }

    RefPtr<Decl> visit(GenericValueParamDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visit(GenericTypeParamDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visit(GenericTypeConstraintDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visit(GenericDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visit(ProgramSyntaxNode*)
    {
        SLANG_UNEXPECTED("module decls should be lowered explicitly");
    }

    RefPtr<Decl> visit(SubscriptDecl*)
    {
        // We don't expect to find direct references to a subscript
        // declaration, but rather to the underlying accessors
        return nullptr;
    }

    RefPtr<Decl> visit(InheritanceDecl*)
    {
        // We should deal with these explicitly, as part of lowering
        // the type that contains them.
        return nullptr;
    }

    RefPtr<Decl> visit(ExtensionDecl*)
    {
        // Extensions won't exist in the lowered code: their members
        // will turn into ordinary functions that get called explicitly
        return nullptr;
    }

    RefPtr<Decl> visit(TypeDefDecl* decl)
    {
        RefPtr<TypeDefDecl> loweredDecl = new TypeDefDecl();
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->Type = lowerType(decl->Type);

        addMember(shared->loweredProgram, loweredDecl);
        return loweredDecl;
    }

    RefPtr<ImportDecl> visit(ImportDecl* decl)
    {
        // No need to translate things here if we are
        // in "full" mode, because we will selectively
        // translate the imported declarations at their
        // use sites(s).
        if (!shared->isRewrite)
            return nullptr;

        for (auto dd : decl->importedModuleDecl->Members)
        {
            translateDeclRef(dd);
        }

        // Don't actually include a representation of
        // the import declaration in the output
        return nullptr;
    }

    RefPtr<EmptyDecl> visit(EmptyDecl* decl)
    {
        // Empty declarations are really only useful in GLSL,
        // where they are used to hold metadata that doesn't
        // attach to any particular shader parameter.
        //
        // TODO: Only lower empty declarations if we are
        // rewriting a GLSL file, and otherwise ignore them.
        //
        RefPtr<EmptyDecl> loweredDecl = new EmptyDecl();
        lowerDeclCommon(loweredDecl, decl);

        addDecl(loweredDecl);

        return loweredDecl;
    }

    RefPtr<Decl> visit(AggTypeDecl* decl)
    {
        // We want to lower any aggregate type declaration
        // to just a `struct` type that contains its fields.
        //
        // Any non-field members (e.g., methods) will be
        // lowered separately.

        // TODO: also need to figure out how to handle fields
        // with types that should not be allowed in a `struct`
        // for the chosen target.
        // (also: what to do if there are no fields left
        // after removing invalid ones?)

        RefPtr<StructSyntaxNode> loweredDecl = new StructSyntaxNode();
        lowerDeclCommon(loweredDecl, decl);

        for (auto field : decl->getMembersOfType<VarDeclBase>())
        {
            // TODO: anything more to do than this?
            addMember(loweredDecl, translateDeclRef(field));
        }

        addMember(
            shared->loweredProgram,
            loweredDecl);

        return loweredDecl;
    }

    RefPtr<VarDeclBase> lowerVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl)
    {
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->Type = lowerType(decl->Type);
        loweredDecl->Expr = lowerExpr(decl->Expr);

        return loweredDecl;
    }

    RefPtr<VarDeclBase> visit(
        Variable* decl)
    {
        auto loweredDecl = lowerVarDeclCommon(new Variable(), decl);

        // We need to add things to an appropriate scope, based on what
        // we are referencing.
        //
        // If this is a global variable (program scope), then add it
        // to the global scope.
        RefPtr<ContainerDecl> pp = decl->ParentDecl;
        if (auto parentModuleDecl = pp.As<ProgramSyntaxNode>())
        {
            addMember(
                translateDeclRef(parentModuleDecl),
                loweredDecl);
        }
        // TODO: handle `static` function-scope variables
        else
        {
            // A local variable declaration will get added to the
            // statement scope we are currently processing.
            addDecl(loweredDecl);
        }

        return loweredDecl;
    }

    RefPtr<VarDeclBase> visit(
        StructField* decl)
    {
        return lowerVarDeclCommon(new StructField(), decl);
    }

    RefPtr<VarDeclBase> visit(
        ParameterSyntaxNode* decl)
    {
        return lowerVarDeclCommon(new ParameterSyntaxNode(), decl);
    }

    RefPtr<DeclBase> transformSyntaxField(DeclBase* decl)
    {
        return lowerDeclBase(decl);
    }


    RefPtr<Decl> visit(
        DeclGroup* group)
    {
        for (auto decl : group->decls)
        {
            lowerDecl(decl);
        }
        return nullptr;
    }

    RefPtr<FunctionSyntaxNode> visit(
        FunctionDeclBase*   decl)
    {
        // TODO: need to generate a name

        RefPtr<FunctionSyntaxNode> loweredDecl = new FunctionSyntaxNode();
        lowerDeclCommon(loweredDecl, decl);

        // TODO: push scope for parent decl here...

        // TODO: need to copy over relevant modifiers

        for (auto paramDecl : decl->GetParameters())
        {
            addMember(loweredDecl, translateDeclRef(paramDecl));
        }

        auto loweredReturnType = lowerType(decl->ReturnType);

        loweredDecl->ReturnType = loweredReturnType;

        // If we are a being called recurisvely, then we need to
        // be careful not to let the context get polluted
        LoweringVisitor subVisitor = *this;
        subVisitor.resultVariable = nullptr;
        subVisitor.stmtBeingBuilt = nullptr;

        loweredDecl->Body = subVisitor.lowerStmt(decl->Body);

        // A lowered function always becomes a global-scope function,
        // even if it had been a member function when declared.
        addMember(shared->loweredProgram, loweredDecl);

        return loweredDecl;
    }

    //
    // Entry Points
    //

    EntryPointLayout* findEntryPointLayout(
        EntryPointRequest*  entryPointRequest)
    {
        for( auto entryPointLayout : shared->programLayout->entryPoints )
        {
            if(entryPointLayout->entryPoint->getName() != entryPointRequest->name)
                continue;

            if(entryPointLayout->profile != entryPointRequest->profile)
                continue;

            // TODO: can't easily filter on translation unit here...
            // Ideally the `EntryPointRequest` should get filled in with a pointer
            // the specific function declaration that represents the entry point.

            return entryPointLayout.Ptr();
        }

        return nullptr;
    }

    enum class VaryingParameterDirection
    {
        Input,
        Output,
    };

    struct VaryingParameterArraySpec
    {
        VaryingParameterArraySpec*  next = nullptr;
        IntVal*                     elementCount;
    };

    struct VaryingParameterInfo
    {
        String                      name;
        VaryingParameterDirection   direction;
        VaryingParameterArraySpec*  arraySpecs = nullptr;
    };


    void lowerSimpleShaderParameterToGLSLGlobal(
        VaryingParameterInfo const&     info,
        RefPtr<ExpressionType>          varType,
        RefPtr<VarLayout>               varLayout,
        RefPtr<ExpressionSyntaxNode>    varExpr)
    {
        RefPtr<ExpressionType> type = varType;

        for (auto aa = info.arraySpecs; aa; aa = aa->next)
        {
            RefPtr<ArrayExpressionType> arrayType = new ArrayExpressionType();
            arrayType->BaseType = type;
            arrayType->ArrayLength = aa->elementCount;

            type = arrayType;
        }

        // TODO: if we are declaring an SOA-ized array,
        // this is where those array dimensions would need
        // to be tacked on.

        RefPtr<Variable> globalVarDecl = new Variable();
        globalVarDecl->Name.Content = info.name;
        globalVarDecl->Type.type = type;

        addMember(shared->loweredProgram, globalVarDecl);

        // Add the layout information
        RefPtr<ComputedLayoutModifier> modifier = new ComputedLayoutModifier();
        modifier->layout = varLayout;
        addModifier(globalVarDecl, modifier);

        // Need to generate an assignment in the right direction.
        //
        // TODO: for now I am just dealing with input:

        switch (info.direction)
        {
        case VaryingParameterDirection::Input:
            addModifier(globalVarDecl, new InModifier());
            assign(varExpr, globalVarDecl);
            break;

        case VaryingParameterDirection::Output:
            addModifier(globalVarDecl, new OutModifier());

            assign(globalVarDecl, varExpr);
            break;
        }
    }

    void lowerShaderParameterToGLSLGLobalsRec(
        VaryingParameterInfo const&     info,
        RefPtr<ExpressionType>          varType,
        RefPtr<VarLayout>               varLayout,
        RefPtr<ExpressionSyntaxNode>    varExpr)
    {
        assert(varLayout);

        if (auto basicType = varType->As<BasicExpressionType>())
        {
            // handled below
        }
        else if (auto vectorType = varType->As<VectorExpressionType>())
        {
            // handled below
        }
        else if (auto matrixType = varType->As<MatrixExpressionType>())
        {
            // handled below
        }
        else if (auto arrayType = varType->As<ArrayExpressionType>())
        {
            // We will accumulate information on the array
            // types that were encoutnered on our walk down
            // to the leaves, and then apply these array dimensions
            // to any leaf parameters.

            VaryingParameterArraySpec arraySpec;
            arraySpec.next = info.arraySpecs;
            arraySpec.elementCount = arrayType->ArrayLength;

            VaryingParameterInfo arrayInfo = info;
            arrayInfo.arraySpecs = &arraySpec;

            RefPtr<IndexExpressionSyntaxNode> subscriptExpr = new IndexExpressionSyntaxNode();
            subscriptExpr->Position = varExpr->Position;
            subscriptExpr->BaseExpression = varExpr;

            // Note that we use the original `varLayout` that was passed in,
            // since that is the layout that will ultimately need to be
            // used on the array elements.
            //
            // TODO: That won't actually work if we ever had an array of
            // heterogeneous stuff...
            lowerShaderParameterToGLSLGLobalsRec(
                arrayInfo,
                arrayType->BaseType,
                varLayout,
                subscriptExpr);

            // TODO: we need to construct syntax for a loop to initialize
            // the array here...
            throw "unimplemented";
        }
        else if (auto declRefType = varType->As<DeclRefType>())
        {
            auto declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
            {
                // The shader parameter had a structured type, so we need
                // to destructure it into its constituent fields

                for (auto fieldDeclRef : getMembersOfType<VarDeclBase>(aggTypeDeclRef))
                {
                    // Don't emit storage for `static` fields here, of course
                    if (fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    RefPtr<MemberExpressionSyntaxNode> fieldExpr = new MemberExpressionSyntaxNode();
                    fieldExpr->Position = varExpr->Position;
                    fieldExpr->Type.type = GetType(fieldDeclRef);
                    fieldExpr->declRef = fieldDeclRef;
                    fieldExpr->BaseExpression = varExpr;

                    VaryingParameterInfo fieldInfo = info;
                    fieldInfo.name = info.name + "_" + fieldDeclRef.GetName();

                    // Need to find the layout for the given field...
                    Decl* originalFieldDecl = nullptr;
                    shared->mapLoweredDeclToOriginal.TryGetValue(fieldDeclRef.getDecl(), originalFieldDecl);
                    assert(originalFieldDecl);

                    auto structTypeLayout = varLayout->typeLayout.As<StructTypeLayout>();
                    assert(structTypeLayout);

                    RefPtr<VarLayout> fieldLayout;
                    structTypeLayout->mapVarToLayout.TryGetValue(originalFieldDecl, fieldLayout);
                    assert(fieldLayout);

                    lowerShaderParameterToGLSLGLobalsRec(
                        fieldInfo,
                        GetType(fieldDeclRef),
                        fieldLayout,
                        fieldExpr);
                }

                // Okay, we are done with this parameter
                return;
            }
        }

        // Default case: just try to emit things as-is
        lowerSimpleShaderParameterToGLSLGlobal(info, varType, varLayout, varExpr);
    }

    void lowerShaderParameterToGLSLGLobals(
        RefPtr<Variable>            localVarDecl,
        RefPtr<VarLayout>           paramLayout,
        VaryingParameterDirection   direction)
    {
        auto name = localVarDecl->getName();
        auto declRef = makeDeclRef(localVarDecl.Ptr());

        RefPtr<VarExpressionSyntaxNode> expr = new VarExpressionSyntaxNode();
        expr->name = name;
        expr->declRef = declRef;
        expr->Type.type = GetType(declRef);

        VaryingParameterInfo info;
        info.name = name;
        info.direction = direction;

        lowerShaderParameterToGLSLGLobalsRec(
            info,
            localVarDecl->getType(),
            paramLayout,
            expr);
    }

    struct EntryPointParamPair
    {
        RefPtr<ParameterSyntaxNode> original;
        RefPtr<VarLayout>           layout;
        RefPtr<Variable>            lowered;
    };

    RefPtr<FunctionSyntaxNode> lowerEntryPointToGLSL(
        FunctionSyntaxNode*         entryPointDecl,
        RefPtr<EntryPointLayout>    entryPointLayout)
    {
        // First, loer the entry-point function as an ordinary function:
        auto loweredEntryPointFunc = visit(entryPointDecl);

        // Now we will generate a `void main() { ... }` function to call the lowered code.
        RefPtr<FunctionSyntaxNode> mainDecl = new FunctionSyntaxNode();
        mainDecl->ReturnType.type = ExpressionType::GetVoid();
        mainDecl->Name.Content = "main";

        // If the user's entry point was called `main` then rename it here
        if (loweredEntryPointFunc->getName() == "main")
            loweredEntryPointFunc->Name.Content = "main_";

        // We will want to generate declarations into the body of our new `main()`
        LoweringVisitor subVisitor = *this;
        subVisitor.isBuildingStmt = true;
        subVisitor.stmtBeingBuilt = nullptr;

        // The parameters of the entry-point function will be translated to
        // both a local variable (for passing to/from the entry point func),
        // and to global variables (used for parameter passing)

        List<EntryPointParamPair> params;

        // First generate declarations for the locals
        for (auto paramDecl : entryPointDecl->GetParameters())
        {
            RefPtr<VarLayout> paramLayout;
            entryPointLayout->mapVarToLayout.TryGetValue(paramDecl.Ptr(), paramLayout);
            assert(paramLayout);

            RefPtr<Variable> localVarDecl = new Variable();
            localVarDecl->Position = paramDecl->Position;
            localVarDecl->Name.Content = paramDecl->getName();
            localVarDecl->Type = lowerType(paramDecl->Type);

            subVisitor.addDecl(localVarDecl);

            EntryPointParamPair paramPair;
            paramPair.original = paramDecl;
            paramPair.layout = paramLayout;
            paramPair.lowered = localVarDecl;

            params.Add(paramPair);
        }

        // Next generate globals for the inputs, and initialize them
        for (auto paramPair : params)
        {
            auto paramDecl = paramPair.original;
            if (paramDecl->HasModifier<InModifier>()
                || paramDecl->HasModifier<InOutModifier>()
                || !paramDecl->HasModifier<OutModifier>())
            {
                subVisitor.lowerShaderParameterToGLSLGLobals(
                    paramPair.lowered,
                    paramPair.layout,
                    VaryingParameterDirection::Input);
            }
        }

        // Generate a local variable for the result, if any
        RefPtr<Variable> resultVarDecl;
        if (!loweredEntryPointFunc->ReturnType->Equals(ExpressionType::GetVoid()))
        {
            resultVarDecl = new Variable();
            resultVarDecl->Position = loweredEntryPointFunc->Position;
            resultVarDecl->Name.Content = "_main_result";
            resultVarDecl->Type = TypeExp(loweredEntryPointFunc->ReturnType);

            subVisitor.addDecl(resultVarDecl);
        }

        // Now generate a call to the entry-point function, using the local variables
        auto entryPointDeclRef = makeDeclRef(loweredEntryPointFunc.Ptr());

        RefPtr<FuncType> entryPointType = new FuncType();
        entryPointType->declRef = entryPointDeclRef;

        RefPtr<VarExpressionSyntaxNode> entryPointRef = new VarExpressionSyntaxNode();
        entryPointRef->name = loweredEntryPointFunc->getName();
        entryPointRef->declRef = entryPointDeclRef;
        entryPointRef->Type = QualType(entryPointType);

        RefPtr<InvokeExpressionSyntaxNode> callExpr = new InvokeExpressionSyntaxNode();
        callExpr->FunctionExpr = entryPointRef;
        callExpr->Type = QualType(loweredEntryPointFunc->ReturnType);

        //
        for (auto paramPair : params)
        {
            auto localVarDecl = paramPair.lowered;

            RefPtr<VarExpressionSyntaxNode> varRef = new VarExpressionSyntaxNode();
            varRef->name = localVarDecl->getName();
            varRef->declRef = makeDeclRef(localVarDecl.Ptr());
            varRef->Type = QualType(localVarDecl->getType());

            callExpr->Arguments.Add(varRef);
        }

        if (resultVarDecl)
        {
            // Non-`void` return type, so we need to store it
            subVisitor.assign(resultVarDecl, callExpr);
        }
        else
        {
            // `void` return type: just call it
            subVisitor.addExprStmt(callExpr);
        }


        // Finally, generate logic to copy the outputs to global parameters
        for (auto paramPair : params)
        {
            auto paramDecl = paramPair.original;
            if (paramDecl->HasModifier<OutModifier>()
                || paramDecl->HasModifier<InOutModifier>())
            {
                subVisitor.lowerShaderParameterToGLSLGLobals(
                    paramPair.lowered,
                    paramPair.layout,
                    VaryingParameterDirection::Output);
            }
        }
        if (resultVarDecl)
        {
            subVisitor.lowerShaderParameterToGLSLGLobals(
                resultVarDecl,
                entryPointLayout->resultLayout,
                VaryingParameterDirection::Output);
        }

        mainDecl->Body = subVisitor.stmtBeingBuilt;


        // Once we are done building the body, we append our new declaration to the program.
        addMember(shared->loweredProgram, mainDecl);
        return mainDecl;

#if 0
        RefPtr<FunctionSyntaxNode> loweredDecl = new FunctionSyntaxNode();
        lowerDeclCommon(loweredDecl, entryPointDecl);

        // We create a sub-context appropriate for lowering the function body

        LoweringVisitor subVisitor = *this;
        subVisitor.isBuildingStmt = true;
        subVisitor.stmtBeingBuilt = nullptr;

        // The parameters of the entry-point function must be translated
        // to global-scope declarations
        for (auto paramDecl : entryPointDecl->GetParameters())
        {
            subVisitor.lowerShaderParameterToGLSLGLobals(paramDecl);
        }

        // The output of the function must also be translated into a
        // global-scope declaration.
        auto loweredReturnType = lowerType(entryPointDecl->ReturnType);
        RefPtr<Variable> resultGlobal;
        if (!loweredReturnType->Equals(ExpressionType::GetVoid()))
        {
            resultGlobal = new Variable();
            // TODO: need a scheme for generating unique names
            resultGlobal->Name.Content = "_main_result";
            resultGlobal->Type = loweredReturnType;

            addMember(shared->loweredProgram, resultGlobal);
        }

        loweredDecl->Name.Content = "main";
        loweredDecl->ReturnType.type = ExpressionType::GetVoid();

        // We will emit the body statement in a context where
        // a `return` statmenet will generate writes to the
        // result global that we declared.
        subVisitor.resultVariable = resultGlobal;

        auto loweredBody = subVisitor.lowerStmt(entryPointDecl->Body);
        subVisitor.addStmt(loweredBody);

        loweredDecl->Body = subVisitor.stmtBeingBuilt;

        // TODO: need to append writes for `out` parameters here...

        addMember(shared->loweredProgram, loweredDecl);
        return loweredDecl;
#endif
    }

    RefPtr<FunctionSyntaxNode> lowerEntryPoint(
        FunctionSyntaxNode*         entryPointDecl,
        RefPtr<EntryPointLayout>    entryPointLayout)
    {
        switch( getTarget() )
        {
        // Default case: lower an entry point just like any other function
        default:
            return visit(entryPointDecl);

        // For Slang->GLSL translation, we need to lower things from HLSL-style
        // declarations over to GLSL conventions
        case CodeGenTarget::GLSL:
            return lowerEntryPointToGLSL(entryPointDecl, entryPointLayout);
        }
    }

    RefPtr<FunctionSyntaxNode> lowerEntryPoint(
        EntryPointRequest*  entryPointRequest)
    {
        auto entryPointLayout = findEntryPointLayout(entryPointRequest);
        auto entryPointDecl = entryPointLayout->entryPoint;

        return lowerEntryPoint(
            entryPointDecl,
            entryPointLayout);
    }


};

static RefPtr<StructTypeLayout> getGlobalStructLayout(
    ProgramLayout*      programLayout)
{
    // Layout information for the global scope is either an ordinary
    // `struct` in the common case, or a constant buffer in the case
    // where there were global-scope uniforms.
    auto globalScopeLayout = programLayout->globalScopeLayout;
    StructTypeLayout* globalStructLayout = globalScopeLayout.As<StructTypeLayout>();
    if(globalStructLayout)
    { }
    else if(auto globalConstantBufferLayout = globalScopeLayout.As<ParameterBlockTypeLayout>())
    {
        // TODO: the `cbuffer` case really needs to be emitted very
        // carefully, but that is beyond the scope of what a simple rewriter
        // can easily do (without semantic analysis, etc.).
        //
        // The crux of the problem is that we need to collect all the
        // global-scope uniforms (but not declarations that don't involve
        // uniform storage...) and put them in a single `cbuffer` declaration,
        // so that we can give it an explicit location. The fields in that
        // declaration might use various type declarations, so we'd really
        // need to emit all the type declarations first, and that involves
        // some large scale reorderings.
        //
        // For now we will punt and just emit the declarations normally,
        // and hope that the global-scope block (`$Globals`) gets auto-assigned
        // the same location that we manually asigned it.

        auto elementTypeLayout = globalConstantBufferLayout->elementTypeLayout;
        auto elementTypeStructLayout = elementTypeLayout.As<StructTypeLayout>();

        // We expect all constant buffers to contain `struct` types for now
        assert(elementTypeStructLayout);

        globalStructLayout = elementTypeStructLayout.Ptr();
    }
    else
    {
        assert(!"unexpected");
    }
    return globalStructLayout;
}


// Determine if the user is just trying to "rewrite" their input file
// into an output file. This will affect the way we approach code
// generation, because we want to leave their code "as is" whenever
// possible.
bool isRewriteRequest(
    SourceLanguage  sourceLanguage,
    CodeGenTarget   target)
{
    // TODO: we might only consider things to be a rewrite request
    // in the specific case where checking is turned off...

    switch( target )
    {
    default:
        return false;

    case CodeGenTarget::HLSL:
        return sourceLanguage == SourceLanguage::HLSL;

    case CodeGenTarget::GLSL:
        return sourceLanguage == SourceLanguage::GLSL;
    }
}



LoweredEntryPoint lowerEntryPoint(
    EntryPointRequest*  entryPoint,
    ProgramLayout*      programLayout,
    CodeGenTarget       target)
{
    SharedLoweringContext sharedContext;
    sharedContext.programLayout = programLayout;
    sharedContext.target = target;

    auto translationUnit = entryPoint->getTranslationUnit();

    // Create a single module/program to hold all the lowered code
    // (with the exception of instrinsic/stdlib declarations, which
    // will be remain where they are)
    RefPtr<ProgramSyntaxNode> loweredProgram = new ProgramSyntaxNode();
    sharedContext.loweredProgram = loweredProgram;

    LoweringVisitor visitor;
    visitor.shared = &sharedContext;
    visitor.parentDecl = loweredProgram;

    // We need to register the lowered program as the lowered version
    // of the existing translation unit declaration.
    
    visitor.registerLoweredDecl(
        loweredProgram,
        translationUnit->SyntaxNode);

    // We also need to register the lowered program as the lowered version
    // of any imported modules (since we will be collecting everything into
    // a single module for code generation).
    for (auto rr : entryPoint->compileRequest->loadedModulesList)
    {
        sharedContext.loweredDecls.Add(
            rr,
            loweredProgram);
    }

    // We also want to remember the layout information for
    // that declaration, so that we can apply it during emission
    attachLayout(loweredProgram,
        getGlobalStructLayout(programLayout));


    bool isRewrite = isRewriteRequest(translationUnit->sourceLanguage, target);
    sharedContext.isRewrite = isRewrite;

    LoweredEntryPoint result;
    if (isRewrite)
    {
        for (auto dd : translationUnit->SyntaxNode->Members)
        {
            visitor.translateDeclRef(dd);
        }
    }
    else
    {
        auto loweredEntryPoint = visitor.lowerEntryPoint(entryPoint);
        result.entryPoint = loweredEntryPoint;
    }

    result.program = sharedContext.loweredProgram;

    return result;
}
}
