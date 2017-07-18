// lower.cpp
#include "lower.h"

#include "type-layout.h"
#include "visitor.h"

namespace Slang
{

struct CloneVisitor
    : ModifierVisitor<CloneVisitor, RefPtr<Modifier>>
{
#define ABSTRACT_SYNTAX_CLASS(NAME, BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE, ...)                   \
    RefPtr<NAME> visit ## NAME(NAME* obj) { return new NAME(*obj); }

#include "object-meta-begin.h"
#include "modifier-defs.h"
#include "object-meta-end.h"

};

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

#if 0
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
#endif

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
    RefPtr<ExpressionSyntaxNode> visit##NAME(NAME* obj) {     \
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
class TupleVarDecl : public VarDeclBase
{
public:
    virtual void accept(IDeclVisitor *, void *) override
    {
        throw "unexpected";
    }

    TupleTypeModifier*          tupleType;
    RefPtr<VarDeclBase>         primaryDecl;
    List<RefPtr<VarDeclBase>>   tupleDecls;
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

    struct Element
    {
        DeclRef<VarDeclBase>            tupleFieldDeclRef;
        RefPtr<ExpressionSyntaxNode>    expr;
    };

    // Optional reference to the "primary" value of the tuple,
    // in the case of a tuple type with "orinary" fields
    RefPtr<ExpressionSyntaxNode>    primaryExpr;

    // Additional fields to store values for any non-ordinary fields
    // (or fields that aren't exclusively orginary)
    List<Element>                   tupleElements;
};

// Pseudo-syntax used during lowering
class VaryingTupleVarDecl : public VarDeclBase
{
public:
    virtual void accept(IDeclVisitor *, void *) override
    {
        throw "unexpected";
    }
};

// Pseudo-syntax used during lowering:
// represents an ordered list of expressions as a single unit
class VaryingTupleExpr : public ExpressionSyntaxNode
{
public:
    virtual void accept(IExprVisitor *, void *) override
    {
        throw "unexpected";
    }

    struct Element
    {
        DeclRef<VarDeclBase>            originalFieldDeclRef;
        RefPtr<ExpressionSyntaxNode>    expr;
    };

    List<Element>   elements;
};

struct SharedLoweringContext
{
    CompileRequest*     compileRequest;
    EntryPointRequest*  entryPointRequest;

    ProgramLayout*      programLayout;
    EntryPointLayout*   entryPointLayout;

    // The target we are going to generate code for.
    // 
    // We may need to specialize how constructs get lowered based
    // on the capabilities of the target language.
    CodeGenTarget   target;

    // A set of words reserved by the target
    Dictionary<String, String> reservedWords;


    RefPtr<ProgramSyntaxNode>   loweredProgram;

    Dictionary<Decl*, RefPtr<Decl>> loweredDecls;
    Dictionary<Decl*, Decl*> mapLoweredDeclToOriginal;

    // Work to be done at the very start and end of the entry point
    RefPtr<StatementSyntaxNode> entryPointInitializeStmt;
    RefPtr<StatementSyntaxNode> entryPointFinalizeStmt;

    // Counter used for generating unique temporary names
    int nameCounter = 0;

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
    , ValVisitor<LoweringVisitor, RefPtr<Val>, RefPtr<ExpressionType>>
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

    bool isReservedWord(String const& name)
    {
        return shared->reservedWords.TryGetValue(name) != nullptr;
    }

    void registerReservedWord(
        String const&   name)
    {
        shared->reservedWords.Add(name, name);
    }

    void registerReservedWords()
    {
#define WORD(NAME) registerReservedWord(#NAME)

        switch (shared->target)
        {
        case CodeGenTarget::GLSL:
            WORD(attribute);
            WORD(const);
            WORD(uniform);
            WORD(varying);
            WORD(buffer);

            WORD(shared);
            WORD(coherent);
            WORD(volatile);
            WORD(restrict);
            WORD(readonly);
            WORD(writeonly);
            WORD(atomic_unit);
            WORD(layout);
            WORD(centroid);
            WORD(flat);
            WORD(smooth);
            WORD(noperspective);
            WORD(patch);
            WORD(sample);
            WORD(break);
            WORD(continue);
            WORD(do);
            WORD(for);
            WORD(while);
            WORD(switch);
            WORD(case);
            WORD(default);
            WORD(if);
            WORD(else);
            WORD(subroutine);
            WORD(in);
            WORD(out);
            WORD(inout);
            WORD(float);
            WORD(double);
            WORD(int);
            WORD(void);
            WORD(bool);
            WORD(true);
            WORD(false);
            WORD(invariant);
            WORD(precise);
            WORD(discard);
            WORD(return);

            WORD(lowp);
            WORD(mediump);
            WORD(highp);
            WORD(precision);
            WORD(struct);
            WORD(uint);

            WORD(common);
            WORD(partition);
            WORD(active);
            WORD(asm);
            WORD(class);
            WORD(union);
            WORD(enum);
            WORD(typedef);
            WORD(template);
            WORD(this);
            WORD(resource);

            WORD(goto);
            WORD(inline);
            WORD(noinline);
            WORD(public);
            WORD(static);
            WORD(extern);
            WORD(external);
            WORD(interface);
            WORD(long);
            WORD(short);
            WORD(half);
            WORD(fixed);
            WORD(unsigned);
            WORD(superp);
            WORD(input);
            WORD(output);
            WORD(filter);
            WORD(sizeof);
            WORD(cast);
            WORD(namespace);
            WORD(using);

#define CASE(NAME) \
        WORD(NAME ## 2); WORD(NAME ## 3); WORD(NAME ## 4)

            CASE(mat);
            CASE(dmat);
            CASE(mat2x);
            CASE(mat3x);
            CASE(mat4x);
            CASE(dmat2x);
            CASE(dmat3x);
            CASE(dmat4x);
            CASE(vec);
            CASE(ivec);
            CASE(bvec);
            CASE(dvec);
            CASE(uvec);
            CASE(hvec);
            CASE(fvec);

#undef CASE

#define CASE(NAME)          \
        WORD(NAME ## 1D);       \
        WORD(NAME ## 2D);       \
        WORD(NAME ## 3D);       \
        WORD(NAME ## Cube);     \
        WORD(NAME ## 1DArray);  \
        WORD(NAME ## 2DArray);  \
        WORD(NAME ## 3DArray);  \
        WORD(NAME ## CubeArray);\
        WORD(NAME ## 2DMS);     \
        WORD(NAME ## 2DMSArray) \
        /* end */

#define CASE2(NAME)     \
        CASE(NAME);         \
        CASE(i ## NAME);    \
        CASE(u ## NAME)     \
        /* end */

            CASE2(sampler);
            CASE2(image);
            CASE2(texture);

#undef CASE2
#undef CASE
            break;

            default:
                break;
        }
    }


    //
    // Values
    //

    RefPtr<Val> lowerVal(Val* val)
    {
        if (!val) return nullptr;
        return ValVisitor::dispatch(val);
    }

    RefPtr<Val> visitGenericParamIntVal(GenericParamIntVal* val)
    {
        return new GenericParamIntVal(translateDeclRef(DeclRef<Decl>(val->declRef)).As<VarDeclBase>());
    }

    RefPtr<Val> visitConstantIntVal(ConstantIntVal* val)
    {
        return val;
    }

    //
    // Types
    //

    RefPtr<ExpressionType> lowerType(
        ExpressionType* type)
    {
        if (!type) return nullptr;
        return TypeVisitor::dispatch(type);
    }

    TypeExp lowerType(
        TypeExp const& typeExp)
    {
        TypeExp result;
        result.type = lowerType(typeExp.type);
        result.exp = lowerExpr(typeExp.exp);
        return result;
    }

    RefPtr<ExpressionType> visitErrorType(ErrorType* type)
    {
        return type;
    }

    RefPtr<ExpressionType> visitOverloadGroupType(OverloadGroupType* type)
    {
        return type;
    }

    RefPtr<ExpressionType> visitInitializerListType(InitializerListType* type)
    {
        return type;
    }

    RefPtr<ExpressionType> visitGenericDeclRefType(GenericDeclRefType* type)
    {
        return new GenericDeclRefType(translateDeclRef(DeclRef<Decl>(type->declRef)).As<GenericDecl>());
    }

    RefPtr<ExpressionType> visitFuncType(FuncType* type)
    {
        RefPtr<FuncType> loweredType = new FuncType();
        loweredType->declRef = translateDeclRef(DeclRef<Decl>(type->declRef)).As<CallableDecl>();
        return loweredType;
    }

    RefPtr<ExpressionType> visitDeclRefType(DeclRefType* type)
    {
        auto loweredDeclRef = translateDeclRef(type->declRef);
        return DeclRefType::Create(loweredDeclRef);
    }

    RefPtr<ExpressionType> visitNamedExpressionType(NamedExpressionType* type)
    {
        if (shared->target == CodeGenTarget::GLSL)
        {
            // GLSL does not support `typedef`, so we will lower it out of existence here
            return lowerType(GetType(type->declRef));
        }

        return new NamedExpressionType(translateDeclRef(DeclRef<Decl>(type->declRef)).As<TypeDefDecl>());
    }

    RefPtr<ExpressionType> visitTypeType(TypeType* type)
    {
        return new TypeType(lowerType(type->type));
    }

    RefPtr<ExpressionType> visitArrayExpressionType(ArrayExpressionType* type)
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

    RefPtr<ExpressionSyntaxNode> lowerExprOrTuple(
        ExpressionSyntaxNode* expr)
    {
        if (!expr) return nullptr;
        return ExprVisitor::dispatch(expr);
    }

    RefPtr<ExpressionSyntaxNode> lowerExpr(
        ExpressionSyntaxNode* expr)
    {
        if (!expr) return nullptr;

        auto result = lowerExprOrTuple(expr);
        return maybeReifyTuple(result);
    }

    // catch-all
    RefPtr<ExpressionSyntaxNode> visitExpressionSyntaxNode(
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

    RefPtr<ExpressionSyntaxNode> createUncheckedVarRef(
        char const* name)
    {
        RefPtr<VarExpressionSyntaxNode> result = new VarExpressionSyntaxNode();
        result->name = name;
        return result;
    }

    RefPtr<ExpressionSyntaxNode> createVarRef(
        CodePosition const& loc,
        VarDeclBase*        decl)
    {
        if (auto tupleDecl = dynamic_cast<TupleVarDecl*>(decl))
        {
            return createTupleRef(loc, tupleDecl);
        }
        else if (auto varyingTupleDecl = dynamic_cast<VaryingTupleVarDecl*>(decl))
        {
            return createVaryingTupleRef(loc, varyingTupleDecl);
        }
        else
        {
            RefPtr<VarExpressionSyntaxNode> result = new VarExpressionSyntaxNode();
            result->Position = loc;
            result->Type.type = decl->Type.type;
            result->declRef = makeDeclRef(decl);
            result->name = decl->getName();
            return result;
        }
    }

    RefPtr<ExpressionSyntaxNode> createTupleRef(
        CodePosition const&     loc,
        TupleVarDecl*           decl)
    {
        RefPtr<TupleExpr> result = new TupleExpr();
        result->Position = loc;
        result->Type.type = decl->Type.type;

        if (auto primaryDecl = decl->primaryDecl)
        {
            result->primaryExpr = createVarRef(loc, primaryDecl);
        }

        for (auto dd : decl->tupleDecls)
        {
            auto tupleVarMod = dd->FindModifier<TupleVarModifier>();
            assert(tupleVarMod);
            auto tupleFieldMod = tupleVarMod->tupleField;
            assert(tupleFieldMod);
            assert(tupleFieldMod->decl);

            TupleExpr::Element elem;
            elem.tupleFieldDeclRef = makeDeclRef(tupleFieldMod->decl);
            elem.expr = createVarRef(loc, dd);
            result->tupleElements.Add(elem);
        }

        return result;
    }

    RefPtr<ExpressionSyntaxNode> createVaryingTupleRef(
        CodePosition const&     loc,
        VaryingTupleVarDecl*    decl)
    {
        return decl->Expr;
    }

    RefPtr<ExpressionSyntaxNode> visitVarExpressionSyntaxNode(
        VarExpressionSyntaxNode* expr)
    {
        doSampleRateInputCheck(expr->name);

        // If the expression didn't get resolved, we can leave it as-is
        if (!expr->declRef)
            return expr;

        auto loweredDeclRef = translateDeclRef(expr->declRef);
        auto loweredDecl = loweredDeclRef.getDecl();

        if (auto tupleVarDecl = dynamic_cast<TupleVarDecl*>(loweredDecl))
        {
            // If we are referencing a declaration that got tuple-ified,
            // then we need to produce a tuple expression as well.

            return createTupleRef(expr->Position, tupleVarDecl);
        }
        else if (auto varyingTupleVarDecl = dynamic_cast<VaryingTupleVarDecl*>(loweredDecl))
        {
            return createVaryingTupleRef(expr->Position, varyingTupleVarDecl);
        }

        RefPtr<VarExpressionSyntaxNode> loweredExpr = new VarExpressionSyntaxNode();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->declRef = loweredDeclRef;
        loweredExpr->name = expr->name;
        return loweredExpr;
    }

    String generateName()
    {
        int id = shared->nameCounter++;

        String result;
        result.append("SLANG_tmp_");
        result.append(id);
        return result;
    }

    // The idea of this function is to take an expression that we plan to
    // use/evaluate more than once, and if needed replace it with a
    // reference to a temporary (initialized with the expr) so that it
    // can safely be re-evaluated.
    RefPtr<ExpressionSyntaxNode> maybeMoveTemp(
        RefPtr<ExpressionSyntaxNode> expr)
    {
        // TODO: actually implement this properly!

        // Certain expressions are already in a form we can directly re-use,
        // so  there is no reason to move them.
        if (expr.As<VarExpressionSyntaxNode>())
            return expr;
        if (expr.As<ConstantExpressionSyntaxNode>())
            return expr;

        if (auto varyingTupleExpr = expr.As<VaryingTupleExpr>())
        {
            RefPtr<VaryingTupleExpr> resultExpr = new VaryingTupleExpr();
            resultExpr->Position = expr->Position;
            resultExpr->Type = expr->Type;
            for (auto ee : varyingTupleExpr->elements)
            {
                VaryingTupleExpr::Element elem;
                elem.originalFieldDeclRef = ee.originalFieldDeclRef;
                elem.expr = maybeMoveTemp(ee.expr);

                resultExpr->elements.Add(elem);
            }

            return resultExpr;
        }

        // TODO: handle the tuple cases here...

        // In the general case, though, we need to introduce a temporary
        RefPtr<Variable> varDecl = new Variable();
        varDecl->Name.Content = generateName();
        varDecl->Type.type = expr->Type.type;
        varDecl->Expr = expr;

        addDecl(varDecl);

        return createVarRef(expr->Position, varDecl);
    }

    // Similar to the above, this ensures that an l-value expression
    // is safe to re-evaluate, by recursively moving things off
    // to temporaries where needed.
    RefPtr<ExpressionSyntaxNode> ensureSimpleLValue(
        RefPtr<ExpressionSyntaxNode> expr)
    {
        // TODO: actually implement this properly!

        return expr;
    }

    RefPtr<ExpressionSyntaxNode> createAssignExpr(
        RefPtr<ExpressionSyntaxNode>    leftExpr,
        RefPtr<ExpressionSyntaxNode>    rightExpr)
    {
        auto leftTuple = leftExpr.As<TupleExpr>();
        auto rightTuple = rightExpr.As<TupleExpr>();
        if (leftTuple && rightTuple)
        {
            RefPtr<TupleExpr> resultTuple = new TupleExpr();
            resultTuple->Type = leftExpr->Type;

            if (leftTuple->primaryExpr)
            {
                assert(rightTuple->primaryExpr);

                resultTuple->primaryExpr = createAssignExpr(
                    leftTuple->primaryExpr,
                    rightTuple->primaryExpr);
            }

            auto elementCount = leftTuple->tupleElements.Count();
            assert(elementCount == rightTuple->tupleElements.Count());
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto leftElement = leftTuple->tupleElements[ee];
                auto rightElement = rightTuple->tupleElements[ee];

                TupleExpr::Element resultElement;

                resultElement.tupleFieldDeclRef = leftElement.tupleFieldDeclRef;
                resultElement.expr = createAssignExpr(
                    leftElement.expr,
                    rightElement.expr);

                resultTuple->tupleElements.Add(resultElement);
            }

            return resultTuple;
        }
        else
        {
            assert(!leftTuple && !rightTuple);
        }

        auto leftVaryingTuple = leftExpr.As<VaryingTupleExpr>();
        auto rightVaryingTuple = rightExpr.As<VaryingTupleExpr>();
        if (leftVaryingTuple && rightVaryingTuple)
        {
            RefPtr<VaryingTupleExpr> resultTuple = new VaryingTupleExpr();
            resultTuple->Type.type = lowerType(leftExpr->Type.type);
            resultTuple->Position = leftExpr->Position;

            assert(resultTuple->Type.type);

            UInt elementCount = leftVaryingTuple->elements.Count();
            assert(elementCount == rightVaryingTuple->elements.Count());

            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto leftElem = leftVaryingTuple->elements[ee];
                auto rightElem = rightVaryingTuple->elements[ee];

                VaryingTupleExpr::Element elem;
                elem.originalFieldDeclRef = leftElem.originalFieldDeclRef;
                elem.expr = createAssignExpr(
                    leftElem.expr,
                    rightElem.expr);
            }
        }
        else if (leftVaryingTuple)
        {
            // Assigning from ordinary expression on RHS to tuple.
            // This will naturally yield a tuple expression.
            //
            // TODO: need to be careful about side-effects, or
            // about dropping sub-expressions after the assignment.
            // For now this will really only work directly in
            // a statement context.

            UInt elementCount = leftVaryingTuple->elements.Count();

            // Move everything into temps if we can

            rightExpr = maybeMoveTemp(rightExpr);
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto& leftElem = leftVaryingTuple->elements[ee];
                leftElem.expr = ensureSimpleLValue(leftElem.expr);
            }

            // We need to combine the sub-expressions into a giant sequence expression.
            //
            // We will procede through thigns from last to first, to build a bunch
            // of "operator comma" expressions bottom-up.
            RefPtr<ExpressionSyntaxNode> resultExpr = leftExpr;

            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto leftElem = leftVaryingTuple->elements[elementCount - ee - 1];

                RefPtr<MemberExpressionSyntaxNode> rightElemExpr = new MemberExpressionSyntaxNode();
                rightElemExpr->Position = rightExpr->Position;
                rightElemExpr->Type = leftElem.expr->Type;
                rightElemExpr->declRef = leftElem.originalFieldDeclRef;
                rightElemExpr->name = leftElem.originalFieldDeclRef.GetName();
                rightElemExpr->BaseExpression = rightExpr;

                auto subExpr = createAssignExpr(
                    leftElem.expr,
                    rightElemExpr);

                RefPtr<InfixExpr> seqExpr = new InfixExpr();
                seqExpr->FunctionExpr = createUncheckedVarRef(",");
                seqExpr->Arguments.Add(subExpr);
                seqExpr->Arguments.Add(resultExpr);

                resultExpr = seqExpr;
            }

            return resultExpr;
        }
        else if (rightVaryingTuple)
        {
            // Pretty much the same as the above case, and we should
            // probably try to share code eventually.

            UInt elementCount = rightVaryingTuple->elements.Count();

            // Move everything into temps if we can

            leftExpr = ensureSimpleLValue(leftExpr);
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto& rightElem = rightVaryingTuple->elements[ee];
                rightElem.expr = maybeMoveTemp(rightElem.expr);
            }

            // We need to combine the sub-expressions into a giant sequence expression.
            //
            // We will procede through thigns from last to first, to build a bunch
            // of "operator comma" expressions bottom-up.
            RefPtr<ExpressionSyntaxNode> resultExpr = leftExpr;

            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto rightElem = rightVaryingTuple->elements[elementCount - ee - 1];

                RefPtr<MemberExpressionSyntaxNode> leftElemExpr = new MemberExpressionSyntaxNode();
                leftElemExpr->Position = leftExpr->Position;
                leftElemExpr->Type = rightElem.expr->Type;
                leftElemExpr->declRef = rightElem.originalFieldDeclRef;
                leftElemExpr->name = rightElem.originalFieldDeclRef.GetName();
                leftElemExpr->BaseExpression = leftExpr;

                auto subExpr = createAssignExpr(
                    leftElemExpr,
                    rightElem.expr);

                RefPtr<InfixExpr> seqExpr = new InfixExpr();
                seqExpr->FunctionExpr = createUncheckedVarRef(",");
                seqExpr->Arguments.Add(subExpr);
                seqExpr->Arguments.Add(resultExpr);

                resultExpr = seqExpr;
            }

            return resultExpr;
        }

        // Default case: no tuples of any kind...


        RefPtr<AssignExpr> loweredExpr = new AssignExpr();
        loweredExpr->Type = leftExpr->Type;
        loweredExpr->left = leftExpr;
        loweredExpr->right = rightExpr;
        return loweredExpr;
    }

    RefPtr<ExpressionSyntaxNode> visitAssignExpr(
        AssignExpr* expr)
    {
        auto leftExpr = lowerExprOrTuple(expr->left);
        auto rightExpr = lowerExprOrTuple(expr->right);

        auto loweredExpr = createAssignExpr(leftExpr, rightExpr);
        lowerExprCommon(loweredExpr, expr);
        return loweredExpr;
    }

    RefPtr<ExpressionType> getSubscripResultType(
        RefPtr<ExpressionType>  type)
    {
        if (auto arrayType = type->As<ArrayExpressionType>())
        {
            return arrayType->BaseType;
        }
        return nullptr;
    }

    RefPtr<ExpressionSyntaxNode> createSubscriptExpr(
        RefPtr<ExpressionSyntaxNode>    baseExpr,
        RefPtr<ExpressionSyntaxNode>    indexExpr)
    {
        // TODO: This logic ends up duplicating the `indexExpr`
        // that was given, without worrying about any side
        // effects it might contain. That needs to be fixed.

        if (auto baseTuple = baseExpr.As<TupleExpr>())
        {
            indexExpr = maybeMoveTemp(indexExpr);

            auto loweredExpr = new TupleExpr();
            loweredExpr->Type.type = getSubscripResultType(baseExpr->Type.type);

            if (auto basePrimary = baseTuple->primaryExpr)
            {
                loweredExpr->primaryExpr = createSubscriptExpr(
                    basePrimary,
                    indexExpr);
            }
            for (auto elem : baseTuple->tupleElements)
            {
                TupleExpr::Element loweredElem;
                loweredElem.tupleFieldDeclRef = elem.tupleFieldDeclRef;
                loweredElem.expr = createSubscriptExpr(
                    elem.expr,
                    indexExpr);

                loweredExpr->tupleElements.Add(loweredElem);
            }

            return loweredExpr;
        }
        else if (auto baseVaryingTuple = baseExpr.As<VaryingTupleExpr>())
        {
            indexExpr = maybeMoveTemp(indexExpr);

            auto loweredExpr = new VaryingTupleExpr();
            loweredExpr->Type.type = getSubscripResultType(baseExpr->Type.type);

            assert(loweredExpr->Type.type);

            for (auto elem : baseVaryingTuple->elements)
            {
                VaryingTupleExpr::Element loweredElem;
                loweredElem.originalFieldDeclRef = elem.originalFieldDeclRef;
                loweredElem.expr = createSubscriptExpr(
                    elem.expr,
                    indexExpr);
            }

            return loweredExpr;
        }
        else
        {
            // Default case: just reconstrut a subscript expr
            auto loweredExpr = new IndexExpressionSyntaxNode();

            loweredExpr->Type.type = getSubscripResultType(baseExpr->Type.type);

            loweredExpr->BaseExpression = baseExpr;
            loweredExpr->IndexExpression = indexExpr;
            return loweredExpr;
        }
    }

    RefPtr<ExpressionSyntaxNode> visitIndexExpressionSyntaxNode(
        IndexExpressionSyntaxNode* subscriptExpr)
    {
        auto baseExpr = lowerExprOrTuple(subscriptExpr->BaseExpression);
        auto indexExpr = lowerExpr(subscriptExpr->IndexExpression);

        // An attempt to subscript a tuple must be turned into a
        // tuple of subscript expressions.
        if (auto baseTuple = baseExpr.As<TupleExpr>())
        {
            return createSubscriptExpr(baseExpr, indexExpr);
        }
        else if (auto baseVaryingTuple = baseExpr.As<VaryingTupleExpr>())
        {
            return createSubscriptExpr(baseExpr, indexExpr);
        }
        else
        {
            // Default case: just reconstrut a subscript expr
            RefPtr<IndexExpressionSyntaxNode> loweredExpr = new IndexExpressionSyntaxNode();
            lowerExprCommon(loweredExpr, subscriptExpr);
            loweredExpr->BaseExpression = baseExpr;
            loweredExpr->IndexExpression = indexExpr;
            return loweredExpr;
        }
    }

    RefPtr<ExpressionSyntaxNode> maybeReifyTuple(
        RefPtr<ExpressionSyntaxNode> expr)
    {
        if (auto tupleExpr = expr.As<TupleExpr>())
        {
            // TODO: need to diagnose
            return expr;
        }
        else if (auto varyingTupleExpr = expr.As<VaryingTupleExpr>())
        {
            // Need to pass an ordinary (non-tuple) expression of
            // the corresponding type here.

            // TODO(tfoley): This won't work at all for an `out` or `inout`
            // function argument, so we'll need to figure out a plan
            // to handle that case...

            RefPtr<AggTypeCtorExpr> resultExpr = new AggTypeCtorExpr();
            resultExpr->Type = varyingTupleExpr->Type;
            resultExpr->base.type = varyingTupleExpr->Type.type;
            assert(resultExpr->Type.type);

            for (auto elem : varyingTupleExpr->elements)
            {
                addArgs(resultExpr, elem.expr);
            }

            return resultExpr;
        }

        // Default case: nothing special to this expression
        return expr;
    }

    void addArgs(
        ExprWithArgsBase*               callExpr,
        RefPtr<ExpressionSyntaxNode>    argExpr)
    {
        if (auto argTuple = argExpr.As<TupleExpr>())
        {
            if (argTuple->primaryExpr)
            {
                addArgs(callExpr, argTuple->primaryExpr);
            }
            for (auto elem : argTuple->tupleElements)
            {
                addArgs(callExpr, elem.expr);
            }
        }
        else if (auto varyingArgTuple = argExpr.As<VaryingTupleExpr>())
        {
            // Need to pass an ordinary (non-tuple) expression of
            // the corresponding type here.

            callExpr->Arguments.Add(maybeReifyTuple(argExpr));
        }
        else
        {
            callExpr->Arguments.Add(argExpr);
        }
    }

    RefPtr<ExpressionSyntaxNode> lowerCallExpr(
        RefPtr<InvokeExpressionSyntaxNode>  loweredExpr,
        InvokeExpressionSyntaxNode*         expr)
    {
        lowerExprCommon(loweredExpr, expr);

        loweredExpr->FunctionExpr = lowerExpr(expr->FunctionExpr);

        for (auto arg : expr->Arguments)
        {
            auto loweredArg = lowerExprOrTuple(arg);
            addArgs(loweredExpr, loweredArg);
        }

        return loweredExpr;
    }

    RefPtr<ExpressionSyntaxNode> visitInvokeExpressionSyntaxNode(
        InvokeExpressionSyntaxNode* expr)
    {
        return lowerCallExpr(new InvokeExpressionSyntaxNode(), expr);
    }

    RefPtr<ExpressionSyntaxNode> visitInfixExpr(
        InfixExpr* expr)
    {
        return lowerCallExpr(new InfixExpr(), expr);
    }

    RefPtr<ExpressionSyntaxNode> visitPrefixExpr(
        PrefixExpr* expr)
    {
        return lowerCallExpr(new PrefixExpr(), expr);
    }

    RefPtr<ExpressionSyntaxNode> visitSelectExpressionSyntaxNode(
        SelectExpressionSyntaxNode* expr)
    {
        // TODO: A tuple needs to be special-cased here

        return lowerCallExpr(new SelectExpressionSyntaxNode(), expr);
    }

    RefPtr<ExpressionSyntaxNode> visitPostfixExpr(
        PostfixExpr* expr)
    {
        return lowerCallExpr(new PostfixExpr(), expr);
    }

    RefPtr<ExpressionSyntaxNode> visitDerefExpr(
        DerefExpr*  expr)
    {
        auto loweredBase = lowerExprOrTuple(expr->base);

        if (auto baseTuple = loweredBase.As<TupleExpr>())
        {
            if (auto primaryExpr = baseTuple->primaryExpr)
            {
                RefPtr<DerefExpr> loweredPrimary = new DerefExpr();
                lowerExprCommon(loweredPrimary, expr);
                loweredPrimary->base = baseTuple->primaryExpr;
                baseTuple->primaryExpr = loweredPrimary;
                return baseTuple;
            }
        }

        RefPtr<DerefExpr> loweredExpr = new DerefExpr();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->base = loweredBase;
        return loweredExpr;
    }

    RefPtr<ExpressionSyntaxNode> visitMemberExpressionSyntaxNode(
        MemberExpressionSyntaxNode* expr)
    {
        auto loweredBase = lowerExprOrTuple(expr->BaseExpression);

        auto loweredDeclRef = translateDeclRef(expr->declRef);


        // Are we extracting an element from a tuple?
        if (auto baseTuple = loweredBase.As<TupleExpr>())
        {
            auto tupleFieldMod = loweredDeclRef.getDecl()->FindModifier<TupleFieldModifier>();
            if (tupleFieldMod)
            {
                // This field has a tuple part to it, so we need to search for it

                RefPtr<ExpressionSyntaxNode> tupleFieldExpr;
                for (auto elem : baseTuple->tupleElements)
                {
                    if (loweredDeclRef.getDecl() == elem.tupleFieldDeclRef.getDecl())
                    {
                        tupleFieldExpr = elem.expr;
                        break;
                    }
                }

                if (!tupleFieldMod->hasAnyNonTupleFields)
                    return tupleFieldExpr;

                auto tupleFieldTupleExpr = tupleFieldExpr.As<TupleExpr>();
                assert(tupleFieldTupleExpr);
                assert(!tupleFieldTupleExpr->primaryExpr);


                RefPtr<MemberExpressionSyntaxNode> loweredPrimaryExpr = new MemberExpressionSyntaxNode();
                lowerExprCommon(loweredPrimaryExpr, expr);
                loweredPrimaryExpr->BaseExpression = baseTuple->primaryExpr;
                loweredPrimaryExpr->declRef = loweredDeclRef;
                loweredPrimaryExpr->name = expr->name;

                tupleFieldTupleExpr->primaryExpr = loweredPrimaryExpr;
                return tupleFieldTupleExpr;
            }

            // If the field was a non-tupe field, then we can
            // simply fall through to the ordinary case below.
            loweredBase = baseTuple->primaryExpr;
        }
        else if (auto baseVaryingTuple = loweredBase.As<VaryingTupleExpr>())
        {
            // Search for the element corresponding to this field
            for(auto elem : baseVaryingTuple->elements)
            {
                if (expr->declRef.getDecl() == elem.originalFieldDeclRef.getDecl())
                {
                    // We found the field!
                    return elem.expr;
                }
            }

            assert(!"unexpected");
        }

        // Default handling:
        assert(!dynamic_cast<TupleVarDecl*>(loweredDeclRef.getDecl()));

        RefPtr<MemberExpressionSyntaxNode> loweredExpr = new MemberExpressionSyntaxNode();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->BaseExpression = loweredBase;
        loweredExpr->declRef = loweredDeclRef;
        loweredExpr->name = expr->name;

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
        if (!stmt)
            return nullptr;

        LoweringVisitor subVisitor = *this;
        subVisitor.stmtBeingBuilt = nullptr;

        subVisitor.lowerStmtImpl(stmt);

        if (!subVisitor.stmtBeingBuilt)
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
        if (!originalStmt) return nullptr;

        for (auto state = &stmtLoweringState; state; state = state->parent)
        {
            if (state->originalStmt == originalStmt)
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

    RefPtr<ScopeDecl> visitScopeDecl(ScopeDecl* decl)
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
        if (!dest)
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
        // Desugar tuples in statement position
        if (auto tupleExpr = expr.As<TupleExpr>())
        {
            if (tupleExpr->primaryExpr)
            {
                addExprStmt(tupleExpr->primaryExpr);
            }
            for (auto ee : tupleExpr->tupleElements)
            {
                addExprStmt(ee.expr);
            }
            return;
        }
        else if (auto varyingTupleExpr = expr.As<VaryingTupleExpr>())
        {
            for (auto ee : varyingTupleExpr->elements)
            {
                addExprStmt(ee.expr);
            }
            return;
        }
        else if (auto infixExpr = expr.As<InfixExpr>())
        {
            if (auto varExpr = infixExpr->FunctionExpr.As<VarExpressionSyntaxNode>())
            {
                if (varExpr->name == ",")
                {
                    // Call to "operator comma"
                    for (auto aa : infixExpr->Arguments)
                    {
                        addExprStmt(aa);
                    }
                    return;
                }
            }
        }

        RefPtr<ExpressionStatementSyntaxNode> stmt = new ExpressionStatementSyntaxNode();
        stmt->Expression = expr;
        addStmt(stmt);
    }

    void visitBlockStmt(BlockStmt* stmt)
    {
        RefPtr<BlockStmt> loweredStmt = new BlockStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->body = subVisitor.lowerStmt(stmt->body);

        addStmt(loweredStmt);
    }

    void visitSeqStmt(SeqStmt* stmt)
    {
        for (auto ss : stmt->stmts)
        {
            lowerStmtImpl(ss);
        }
    }

    void visitExpressionStatementSyntaxNode(ExpressionStatementSyntaxNode* stmt)
    {
        addExprStmt(lowerExprOrTuple(stmt->Expression));
    }

    void visitVarDeclrStatementSyntaxNode(VarDeclrStatementSyntaxNode* stmt)
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

    void visitContinueStatementSyntaxNode(ContinueStatementSyntaxNode* stmt)
    {
        RefPtr<ContinueStatementSyntaxNode> loweredStmt = new ContinueStatementSyntaxNode();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitBreakStatementSyntaxNode(BreakStatementSyntaxNode* stmt)
    {
        RefPtr<BreakStatementSyntaxNode> loweredStmt = new BreakStatementSyntaxNode();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitDefaultStmt(DefaultStmt* stmt)
    {
        RefPtr<DefaultStmt> loweredStmt = new DefaultStmt();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitDiscardStatementSyntaxNode(DiscardStatementSyntaxNode* stmt)
    {
        RefPtr<DiscardStatementSyntaxNode> loweredStmt = new DiscardStatementSyntaxNode();
        lowerStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitEmptyStatementSyntaxNode(EmptyStatementSyntaxNode* stmt)
    {
        RefPtr<EmptyStatementSyntaxNode> loweredStmt = new EmptyStatementSyntaxNode();
        lowerStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitUnparsedStmt(UnparsedStmt* stmt)
    {
        RefPtr<UnparsedStmt> loweredStmt = new UnparsedStmt();
        lowerStmtFields(loweredStmt, stmt);

        for (auto token : stmt->tokens)
        {
            if (token.Type == TokenType::Identifier)
                doSampleRateInputCheck(token.Content);
        }

        loweredStmt->tokens = stmt->tokens;

        addStmt(loweredStmt);
    }

    void visitCaseStmt(CaseStmt* stmt)
    {
        RefPtr<CaseStmt> loweredStmt = new CaseStmt();
        lowerChildStmtFields(loweredStmt, stmt);

        loweredStmt->expr = lowerExpr(stmt->expr);

        addStmt(loweredStmt);
    }

    void visitIfStatementSyntaxNode(IfStatementSyntaxNode* stmt)
    {
        RefPtr<IfStatementSyntaxNode> loweredStmt = new IfStatementSyntaxNode();
        lowerStmtFields(loweredStmt, stmt);

        loweredStmt->Predicate = lowerExpr(stmt->Predicate);
        loweredStmt->PositiveStatement = lowerStmt(stmt->PositiveStatement);
        loweredStmt->NegativeStatement = lowerStmt(stmt->NegativeStatement);

        addStmt(loweredStmt);
    }

    void visitSwitchStmt(SwitchStmt* stmt)
    {
        RefPtr<SwitchStmt> loweredStmt = new SwitchStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->condition = subVisitor.lowerExpr(stmt->condition);
        loweredStmt->body = subVisitor.lowerStmt(stmt->body);

        addStmt(loweredStmt);
    }

    void lowerForStmtCommon(
        RefPtr<ForStatementSyntaxNode>  loweredStmt,
        ForStatementSyntaxNode*         stmt)
    {
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->InitialStatement = subVisitor.lowerStmt(stmt->InitialStatement);
        loweredStmt->SideEffectExpression = subVisitor.lowerExpr(stmt->SideEffectExpression);
        loweredStmt->PredicateExpression = subVisitor.lowerExpr(stmt->PredicateExpression);
        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visitForStatementSyntaxNode(ForStatementSyntaxNode* stmt)
    {
        lowerForStmtCommon(new ForStatementSyntaxNode(), stmt);
    }

    void visitUnscopedForStmt(UnscopedForStmt* stmt)
    {
        lowerForStmtCommon(new UnscopedForStmt(), stmt);
    }

    void visitWhileStatementSyntaxNode(WhileStatementSyntaxNode* stmt)
    {
        RefPtr<WhileStatementSyntaxNode> loweredStmt = new WhileStatementSyntaxNode();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Predicate = subVisitor.lowerExpr(stmt->Predicate);
        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visitDoWhileStatementSyntaxNode(DoWhileStatementSyntaxNode* stmt)
    {
        RefPtr<DoWhileStatementSyntaxNode> loweredStmt = new DoWhileStatementSyntaxNode();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);
        loweredStmt->Predicate = subVisitor.lowerExpr(stmt->Predicate);

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
        auto assignExpr = createAssignExpr(destExpr, srcExpr);
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

    void visitReturnStatementSyntaxNode(ReturnStatementSyntaxNode* stmt)
    {
        auto loweredStmt = new ReturnStatementSyntaxNode();
        lowerStmtCommon(loweredStmt, stmt);

        if (stmt->Expression)
        {
            if (resultVariable)
            {
                // Do it as an assignment
                assign(resultVariable, lowerExprOrTuple(stmt->Expression));
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
        for (auto pp = decl; pp; pp = pp->ParentDecl)
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
        if (!decl)
            return;

        if (isBuildingStmt)
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

    // If the name of the declarations collides with a reserved word
    // for the code generation target, then rename it to avoid the conflict
    //
    // Note that this does *not* implement any kind of comprehensive renaming
    // to, e.g., avoid conflicts between user-defined and library functions.
    void ensureDeclHasAValidName(Decl* decl)
    {
        // By default, we would like to emit a name in the generated
        // code exactly as it appeared in the original program.
        // When that isn't possible, we'd like to emit a name as
        // close to the original as possible (to ensure that existing
        // debugging tools still work reasonably well).
        //
        // One reason why a name might not be allowed as-is is that
        // it could collide with a reserved word in the target language.
        // Another reason is that it might not follow a naming convention
        // imposed by the target (e.g., in GLSL names starting with
        // `gl_` or containing `__` are reserved).
        //
        // Given a name that should not be allowed, we want to
        // change it to a name that *is* allowed. e.g., by adding
        // `_` to the end of a reserved word.
        //
        // The next problem this creates is that the modified name
        // could not collide with an existing use of the same
        // (valid) name.
        //
        // For now we are going to solve this problem in a simple
        // and ad hoc fashion, but longer term we'll want to do
        // something sytematic.

        if (isReservedWord(decl->getName()))
        {
            decl->Name.Content.append("_");
        }
    }

    RefPtr<VarLayout> tryToFindLayout(
        Decl* decl)
    {
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
                        return fieldLayout;
                    }
                }

                // TODO: are there other cases to handle here?
            }
        }
        return nullptr;
    }

    void lowerDeclCommon(
        Decl* loweredDecl,
        Decl* decl)
    {
        registerLoweredDecl(loweredDecl, decl);

        loweredDecl->Position = decl->Position;
        loweredDecl->Name = decl->getNameToken();

        // Deal with renaming - we shouldn't allow decls with names that are reserved words
        ensureDeclHasAValidName(loweredDecl);

        // Lower modifiers as needed

        // HACK: just doing a shallow copy of modifiers, which will
        // suffice for most of them, but we need to do something
        // better soon.
        loweredDecl->modifiers = decl->modifiers;

        // deal with layout stuff

        if (auto fieldLayout = tryToFindLayout(decl))
        {
            attachLayout(loweredDecl, fieldLayout);
        }
    }

    // Catch-all

    RefPtr<Decl> visitModifierDecl(ModifierDecl*)
    {
        // should not occur in user code
        SLANG_UNEXPECTED("modifiers shouldn't occur in user code");
    }

    RefPtr<Decl> visitGenericValueParamDecl(GenericValueParamDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visitGenericTypeParamDecl(GenericTypeParamDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visitGenericTypeConstraintDecl(GenericTypeConstraintDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visitGenericDecl(GenericDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    RefPtr<Decl> visitProgramSyntaxNode(ProgramSyntaxNode*)
    {
        SLANG_UNEXPECTED("module decls should be lowered explicitly");
    }

    RefPtr<Decl> visitSubscriptDecl(SubscriptDecl*)
    {
        // We don't expect to find direct references to a subscript
        // declaration, but rather to the underlying accessors
        return nullptr;
    }

    RefPtr<Decl> visitInheritanceDecl(InheritanceDecl*)
    {
        // We should deal with these explicitly, as part of lowering
        // the type that contains them.
        return nullptr;
    }

    RefPtr<Decl> visitExtensionDecl(ExtensionDecl*)
    {
        // Extensions won't exist in the lowered code: their members
        // will turn into ordinary functions that get called explicitly
        return nullptr;
    }

    RefPtr<Decl> visitTypeDefDecl(TypeDefDecl* decl)
    {
        if (shared->target == CodeGenTarget::GLSL)
        {
            // GLSL does not support `typedef`, so we will lower it out of existence here
            return nullptr;
        }

        RefPtr<TypeDefDecl> loweredDecl = new TypeDefDecl();
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->Type = lowerType(decl->Type);

        addMember(shared->loweredProgram, loweredDecl);
        return loweredDecl;
    }

    RefPtr<ImportDecl> visitImportDecl(ImportDecl*)
    {
        // We could unconditionally output the declarations in the
        // imported code, but this could cause problems if any
        // of those declarations used capabilities not allowed
        // by the target pipeline stage (e.g., `discard` is
        // an error in a GLSL vertex shader file, even if
        // it is in a function that never gets called).
        //
        // As a result, we just ignore the `import` step,
        // and allow declarations to be pulled in by
        // their use sites.
        //
        // If this proves to be a problem, we will need
        // a pass that resolves which declarations in imported
        // modules are "valid" for the chosen target stage.

        // Don't actually include a representation of
        // the import declaration in the output
        return nullptr;
    }

    RefPtr<EmptyDecl> visitEmptyDecl(EmptyDecl* decl)
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

    TupleTypeModifier* isTupleType(ExpressionType* type)
    {
        if (auto declRefType = type->As<DeclRefType>())
        {
            if (auto tupleTypeMod = declRefType->declRef.getDecl()->FindModifier<TupleTypeModifier>())
            {
                return tupleTypeMod;
            }
        }

        return nullptr;
    }

    ExpressionType* unwrapArray(ExpressionType* inType)
    {
        auto type = inType;
        while (auto arrayType = type->As<ArrayExpressionType>())
        {
            type = arrayType->BaseType;
        }
        return type;
    }

    TupleTypeModifier* isTupleTypeOrArrayOfTupleType(ExpressionType* type)
    {
        return isTupleType(unwrapArray(type));
    }

    bool isResourceType(ExpressionType* type)
    {
        while (auto arrayType = type->As<ArrayExpressionType>())
        {
            type = arrayType->BaseType;
        }

        if (auto textureTypeBase = type->As<TextureTypeBase>())
        {
            return true;
        }
        else if (auto samplerType = type->As<SamplerStateType>())
        {
            return true;
        }

        // TODO: need more comprehensive coverage here

        return false;
    }

    RefPtr<Decl> visitAggTypeDecl(AggTypeDecl* decl)
    {
        // We want to lower any aggregate type declaration
        // to just a `struct` type that contains its fields.
        //
        // Any non-field members (e.g., methods) will be
        // lowered separately.

        RefPtr<StructSyntaxNode> loweredDecl = new StructSyntaxNode();
        lowerDeclCommon(loweredDecl, decl);

        // We need to be ready to turn this type into a "tuple" type,
        // if it has any members that can't normally be kept in a `struct`
        //
        // We don't want to do this unconditionally, though, because
        // then we'll end up changing the meaning of user code in
        // languages like HLSL that support such nesting.

        bool shouldDesugarTupleTypes = false;
        if (getTarget() == CodeGenTarget::GLSL)
        {
            // Always desugar this stuff for GLSL, since it doesn't
            // support nesting of resources in structs.
            //
            // TODO: Need a way to make this more fine-grained to
            // handle cases where a nested member might be allowed
            // due to, e.g., bindless textures.
            shouldDesugarTupleTypes = true;
        }

        bool isResultATupleType = false;
        bool hasAnyNonTupleFields = false;

        for (auto field : decl->getMembersOfType<VarDeclBase>())
        {
            // We lower the field, which will involve lowering the field type
            auto loweredField = translateDeclRef(field).As<VarDeclBase>();

            // Add the field to the result declaration
            addMember(loweredDecl, loweredField);

            // Don't consider any of the following desugaring logic,
            // if we aren't supposed to be desugaring this type
            if (!shouldDesugarTupleTypes)
            {
                hasAnyNonTupleFields = true;
                continue;
            }


            // If the field is of a type that requires special handling,
            // we need to make a note of it.
            auto loweredFieldType = loweredField->Type.type;
            bool isTupleField = false;
            bool fieldHasAnyNonTupleFields = false;
            bool fieldHasTupleType = false;
            if (auto fieldTupleTypeMod = isTupleTypeOrArrayOfTupleType(loweredFieldType))
            {
                isTupleField = true;
                fieldHasTupleType = true;
                if (fieldTupleTypeMod->hasAnyNonTupleFields)
                {
                    fieldHasAnyNonTupleFields = true;
                    hasAnyNonTupleFields = true;
                }
            }
            else if (isResourceType(loweredFieldType))
            {
                isTupleField = true;
            }
            else
            {
                hasAnyNonTupleFields = true;
            }

            if (isTupleField)
            {
                isResultATupleType = true;

                RefPtr<TupleFieldModifier> tupleFieldMod = new TupleFieldModifier();
                tupleFieldMod->decl = loweredField;
                tupleFieldMod->hasAnyNonTupleFields = fieldHasAnyNonTupleFields;
                tupleFieldMod->isNestedTuple = fieldHasTupleType;

                addModifier(loweredField, tupleFieldMod);
            }
        }

        // An empty `struct` must be treated as a tuple type,
        // in order to ensure that we don't mess up layout logic
        //
        // (Also, GLSL doesn't allow empty structs IIRC)
        //
        // Note: in this one case we are desugaring things even
        // when targetting HLSL, just to keep things manageable.
        if (!hasAnyNonTupleFields)
        {
            isResultATupleType = true;
        }

        if (isResultATupleType)
        {
            RefPtr<TupleTypeModifier> tupleTypeMod = new TupleTypeModifier();
            tupleTypeMod->decl = loweredDecl;
            tupleTypeMod->hasAnyNonTupleFields = hasAnyNonTupleFields;
            addModifier(loweredDecl, tupleTypeMod);
        }

        if (isResultATupleType && !hasAnyNonTupleFields)
        {
            // We don't want any pure-tuple types showing up in
            // the output program, so we skip that here.
        }
        else
        {
            addMember(
                shared->loweredProgram,
                loweredDecl);
        }


        return loweredDecl;
    }

    RefPtr<VarDeclBase> lowerSimpleVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl,
        TypeExp const&      loweredType)
    {
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->Type = loweredType;
        loweredDecl->Expr = lowerExpr(decl->Expr);

        return loweredDecl;
    }
    RefPtr<VarDeclBase> lowerSimpleVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl)
    {
        auto loweredType = lowerType(decl->Type);
        return lowerSimpleVarDeclCommon(loweredDecl, decl, loweredType);
    }

    struct TupleTypeSecondaryVarArraySpec
    {
        TupleTypeSecondaryVarArraySpec* next;
        RefPtr<IntVal>                  elementCount;
    };

    struct TupleSecondaryVarInfo
    {
        // Parent tuple decl to add the secondary decl into
        RefPtr<TupleVarDecl>        tupleDecl;

        // Syntax class for declarations to create
        SyntaxClass<VarDeclBase>    varDeclClass;

        // Name "stem" to use for any actual variables we create
        String                      name;

        // The parent tuple type (or array thereof) we are scalarizing
        RefPtr<ExpressionType>  tupleType;

        // The actual declaration of the tuple type (which will give us the fields)
        DeclRef<AggTypeDecl>    tupleTypeDecl;

        // An initializer expression to use for the tuple members
        RefPtr<ExpressionSyntaxNode>    initExpr;

        // The original layout given to the top-level variable
        RefPtr<VarLayout>               primaryVarLayout;

        // The computed layout of the tuple type itself
        RefPtr<StructTypeLayout>            tupleTypeLayout;

        TupleTypeSecondaryVarArraySpec* arraySpecs = nullptr;
    };

    void createTupleTypeSecondaryVarDecls(
        TupleSecondaryVarInfo const& info)
    {
        if (auto arrayType = info.tupleType->As<ArrayExpressionType>())
        {
            TupleTypeSecondaryVarArraySpec arraySpec;
            arraySpec.next = info.arraySpecs;
            arraySpec.elementCount = arrayType->ArrayLength;

            TupleSecondaryVarInfo subInfo = info;
            subInfo.tupleType = arrayType->BaseType;
            subInfo.arraySpecs = &arraySpec;
            createTupleTypeSecondaryVarDecls(subInfo);
            return;
        }

        // Next, we need to go through the declarations in the aggregate
        // type, and deal with all of those that should be tuple-ified.
        for (auto dd : getMembersOfType<VarDeclBase>(info.tupleTypeDecl))
        {
            if (dd.getDecl()->HasModifier<HLSLStaticModifier>())
                continue;

            auto tupleFieldMod = dd.getDecl()->FindModifier<TupleFieldModifier>();
            if (!tupleFieldMod)
                continue;

            // TODO: need to extract the initializer for this field
            assert(!info.initExpr);
            RefPtr<ExpressionSyntaxNode> fieldInitExpr;

            String fieldName = info.name + "_" + dd.GetName();

            auto fieldType = GetType(dd);

            Decl* originalFieldDecl;
            shared->mapLoweredDeclToOriginal.TryGetValue(dd, originalFieldDecl);
            assert(originalFieldDecl);

            RefPtr<VarLayout> fieldLayout;
            if(info.tupleTypeLayout)
            {
                info.tupleTypeLayout->mapVarToLayout.TryGetValue(originalFieldDecl, fieldLayout);
            }
            if (fieldLayout && info.primaryVarLayout)
            {
                // The layout for a field may need to be adjusted
                // based on a base offset stored in the primary
                // variable.
                //
                // For example, if the primary variable was recoreded
                // to start at descriptor-table slot N, then the
                // field layout might say it uses slot k, but that
                // needs to be understood relative to the parent,
                // so we want slot N + k... and actuall N + k + 1,
                // in the case where the parent itself took up
                // space of that type...

                bool needsOffset = false;
                for (auto rr : fieldLayout->resourceInfos)
                {
                    if (auto parentInfo = info.primaryVarLayout->FindResourceInfo(rr.kind))
                    {
                        if (parentInfo->index != 0 || parentInfo->space != 0)
                        {
                            needsOffset = true;
                            break;
                        }
                    }
                }
                if (needsOffset)
                {
                    RefPtr<VarLayout> newFieldLayout = new VarLayout();
                    newFieldLayout->typeLayout = fieldLayout->typeLayout;
                    newFieldLayout->flags = fieldLayout->flags;
                    newFieldLayout->varDecl = fieldLayout->varDecl;
                    newFieldLayout->systemValueSemantic = fieldLayout->systemValueSemantic;
                    newFieldLayout->systemValueSemanticIndex = fieldLayout->systemValueSemanticIndex;

                    for (auto resInfo : fieldLayout->resourceInfos)
                    {
                        auto newResInfo = newFieldLayout->findOrAddResourceInfo(resInfo.kind);
                        newResInfo->index = resInfo.index;
                        newResInfo->space = resInfo.space;
                        if (auto parentInfo = info.primaryVarLayout->FindResourceInfo(resInfo.kind))
                        {
                            newResInfo->index += parentInfo->index;
                            newResInfo->space += parentInfo->space;
                        }
                    }

                    fieldLayout = newFieldLayout;
                }

            }

            RefPtr<VarDeclBase> fieldVarOrTupleDecl;
            if (auto fieldTupleTypeMod = isTupleTypeOrArrayOfTupleType(fieldType))
            {
                // If the field is itself a tuple, then recurse
                RefPtr<TupleVarDecl> fieldTupleDecl = new TupleVarDecl();

                TupleSecondaryVarInfo fieldInfo;
                fieldInfo.tupleDecl = fieldTupleDecl;
                fieldInfo.varDeclClass = info.varDeclClass;
                fieldInfo.name = fieldName;
                fieldInfo.tupleType = fieldType;
                fieldInfo.tupleTypeDecl = makeDeclRef(fieldTupleTypeMod->decl);
                fieldInfo.initExpr = fieldInitExpr;
                fieldInfo.primaryVarLayout = fieldLayout;
                fieldInfo.tupleTypeLayout = getBodyStructTypeLayout(fieldLayout ? fieldLayout->typeLayout : nullptr);
                fieldInfo.arraySpecs = info.arraySpecs;

                fieldTupleDecl->tupleType = fieldTupleTypeMod;
                createTupleTypeSecondaryVarDecls(fieldInfo);

                 fieldVarOrTupleDecl = fieldTupleDecl;
            }
            else
            {
                // Otherwise the field has a simple type, and we just need to declare the variable here

                RefPtr<ExpressionType> fieldVarType = fieldType;
                for (auto aa = info.arraySpecs; aa; aa = aa->next)
                {
                    RefPtr<ArrayExpressionType> arrayType = new ArrayExpressionType();
                    arrayType->BaseType = fieldVarType;
                    arrayType->ArrayLength = aa->elementCount;

                    fieldVarType = arrayType;
                }

                RefPtr<VarDeclBase> fieldVarDecl = info.varDeclClass.createInstance();
                fieldVarDecl->Name.Content = fieldName;
                fieldVarDecl->Type.type = fieldVarType;

                addDecl(fieldVarDecl);

                if (fieldLayout)
                {
                    RefPtr<ComputedLayoutModifier> layoutMod = new ComputedLayoutModifier();
                    layoutMod->layout = fieldLayout;
                    addModifier(fieldVarDecl, layoutMod);
                }

                fieldVarOrTupleDecl = fieldVarDecl;
            }

            RefPtr<TupleVarModifier> fieldTupleVarMod = new TupleVarModifier();
            fieldTupleVarMod->tupleField = tupleFieldMod;
            addModifier(fieldVarOrTupleDecl, fieldTupleVarMod);

            info.tupleDecl->tupleDecls.Add(fieldVarOrTupleDecl);
        }
    }

    RefPtr<VarDeclBase> createTupleTypeVarDecls(
        SyntaxClass<VarDeclBase>        varDeclClass,
        RefPtr<VarDeclBase>             originalVarDecl,
        String const&                   name,
        RefPtr<ExpressionType>          tupleType,
        DeclRef<AggTypeDecl>            tupleTypeDecl,
        TupleTypeModifier*              tupleTypeMod,
        RefPtr<ExpressionSyntaxNode>    initExpr,
        RefPtr<VarLayout>               primaryVarLayout,
        RefPtr<StructTypeLayout>        tupleTypeLayout)
    {
        // Not handling initializers just yet...
        assert(!initExpr);

        // We'll need a placeholder declaration to wrap the whole thing up:
        RefPtr<TupleVarDecl> tupleDecl = new TupleVarDecl();
        tupleDecl->Name.Content = name;

        // First, if the tuple type had any "ordinary" data,
        // then we go ahead and create a declaration for that stuff
        if (tupleTypeMod->hasAnyNonTupleFields)
        {
            RefPtr<VarDeclBase> primaryVarDecl = varDeclClass.createInstance();
            primaryVarDecl->Name.Content = name;
            primaryVarDecl->Type.type = tupleType;

            primaryVarDecl->modifiers = originalVarDecl->modifiers;

            tupleDecl->primaryDecl = primaryVarDecl;

            if (primaryVarLayout)
            {
                RefPtr<ComputedLayoutModifier> layoutMod = new ComputedLayoutModifier();
                layoutMod->layout = primaryVarLayout;
                addModifier(primaryVarDecl, layoutMod);
            }

            addDecl(primaryVarDecl);
        }

        TupleSecondaryVarInfo info;
        info.tupleDecl = tupleDecl;
        info.varDeclClass = varDeclClass;
        info.name = name;
        info.tupleType = tupleType;
        info.tupleTypeDecl = tupleTypeDecl;
        info.initExpr = initExpr;
        info.primaryVarLayout = primaryVarLayout;
        info.tupleTypeLayout = tupleTypeLayout;

        createTupleTypeSecondaryVarDecls(info);

        return tupleDecl;
    }

    RefPtr<StructTypeLayout> getBodyStructTypeLayout(RefPtr<TypeLayout> typeLayout)
    {
        if (!typeLayout)
            return nullptr;

        while (auto parameterBlockTypeLayout = typeLayout.As<ParameterBlockTypeLayout>())
        {
            typeLayout = parameterBlockTypeLayout->elementTypeLayout;
        }

        while (auto arrayTypeLayout = typeLayout.As<ArrayTypeLayout>())
        {
            typeLayout = arrayTypeLayout->elementTypeLayout;
        }

        if (auto structTypeLayout = typeLayout.As<StructTypeLayout>())
        {
            return structTypeLayout;
        }

        return nullptr;
    }

    RefPtr<VarDeclBase> createTupleTypeVarDecls(
        SyntaxClass<VarDeclBase>        varDeclClass,
        RefPtr<VarDeclBase>             originalVarDecl,
        String const&                   name,
        RefPtr<ExpressionType>          tupleType,
        TupleTypeModifier*              tupleTypeMod,
        RefPtr<ExpressionSyntaxNode>    initExpr,
        RefPtr<VarLayout>               primaryVarLayout)
    {
        RefPtr<StructTypeLayout> tupleTypeLayout;
        if (primaryVarLayout)
        {
            auto primaryTypeLayout = primaryVarLayout->typeLayout;
            tupleTypeLayout = getBodyStructTypeLayout(primaryTypeLayout);
        }

        return createTupleTypeVarDecls(
            varDeclClass,
            originalVarDecl,
            name,
            tupleType,
            makeDeclRef(tupleTypeMod->decl),
            tupleTypeMod,
            initExpr,
            primaryVarLayout,
            tupleTypeLayout);
    }

    RefPtr<VarDeclBase> lowerVarDeclCommonInner(
        VarDeclBase*                decl,
        SyntaxClass<VarDeclBase>    loweredDeclClass)
    {
        auto loweredType = lowerType(decl->Type);

        if (auto tupleTypeMod = isTupleTypeOrArrayOfTupleType(loweredType))
        {
            auto varLayout = tryToFindLayout(decl).As<VarLayout>();

            // The type for the variable is a "tuple type"
            // so we need to go ahead and create multiple variables
            // to represent it.

            // If the variable had an initializer, we expect it
            // to resolve to a tuple *value*
            auto loweredInit = lowerExpr(decl->Expr);

            // TODO: need to extract layout here and propagate it down!

            auto tupleDecl = createTupleTypeVarDecls(
                loweredDeclClass,
                decl,
                decl->getName(),
                loweredType.type,
                tupleTypeMod,
                loweredInit,
                varLayout);

            shared->loweredDecls.Add(decl, tupleDecl);
            return tupleDecl;
        }
        if (auto bufferType = loweredType->As<UniformParameterBlockType>())
        {
            auto varLayout = tryToFindLayout(decl).As<VarLayout>();

            auto elementType = bufferType->elementType;
            if (auto elementTupleTypeMod = isTupleTypeOrArrayOfTupleType(elementType))
            {
                auto tupleDecl = createTupleTypeVarDecls(
                    loweredDeclClass,
                    decl,
                    decl->getName(),
                    loweredType.type,
                    elementTupleTypeMod,
                    nullptr,
                    varLayout);

                shared->loweredDecls.Add(decl, tupleDecl);
                return tupleDecl;
            }
        }

        RefPtr<VarDeclBase> loweredDecl = loweredDeclClass.createInstance();
        addDecl(loweredDecl);
        return lowerSimpleVarDeclCommon(loweredDecl, decl, loweredType);
    }

    RefPtr<VarDeclBase> lowerVarDeclCommon(
        VarDeclBase*                decl,
        SyntaxClass<VarDeclBase>    loweredDeclClass)
    {
        // We need to add things to an appropriate scope, based on what
        // we are referencing.
        //
        // If this is a global variable (program scope), then add it
        // to the global scope.
        RefPtr<ContainerDecl> pp = decl->ParentDecl;
        if (auto parentModuleDecl = pp.As<ProgramSyntaxNode>())
        {
            LoweringVisitor subVisitor = *this;
            subVisitor.parentDecl = translateDeclRef(parentModuleDecl);
            subVisitor.isBuildingStmt = false;

            return subVisitor.lowerVarDeclCommonInner(decl, loweredDeclClass);
        }
        // TODO: handle `static` function-scope variables
        else
        {
            // The default behavior is to lower into whatever
            // scope was already in places
            return lowerVarDeclCommonInner(decl, loweredDeclClass);
        }
    }

    SourceLanguage getSourceLanguage(ProgramSyntaxNode* moduleDecl)
    {
        for (auto translationUnit : shared->compileRequest->translationUnits)
        {
            if (moduleDecl == translationUnit->SyntaxNode)
                return translationUnit->sourceLanguage;
        }

        for (auto loadedModuleDecl : shared->compileRequest->loadedModulesList)
        {
            if (moduleDecl == loadedModuleDecl)
                return SourceLanguage::Slang;
        }

        return SourceLanguage::Unknown;
    }

    void setSampleRateFlag()
    {
        shared->entryPointLayout->flags |= EntryPointLayout::Flag::usesAnySampleRateInput;
    }

    void doSampleRateInputCheck(VarDeclBase* decl)
    {
        if (decl->HasModifier<HLSLSampleModifier>())
        {
            setSampleRateFlag();
        }
    }

    void doSampleRateInputCheck(String const& name)
    {
        if (name == "gl_SampleIndex")
        {
            setSampleRateFlag();
        }
    }

    bool isImportedStructType(RefPtr<ExpressionType> type)
    {
        if (type->As<BasicExpressionType>()) return false;
        else if (type->As<VectorExpressionType>()) return false;
        else if (type->As<MatrixExpressionType>()) return false;
        else if (type->As<ResourceType>()) return false;
        else if (type->As<BuiltinGenericType>()) return false;
        else if (auto declRefType = type->As<DeclRefType>())
        {
            if (auto aggTypeDeclRef = declRefType->declRef.As<AggTypeDecl>())
            {
                Decl* pp = aggTypeDeclRef.getDecl();
                while (pp->ParentDecl)
                    pp = pp->ParentDecl;

                // Did the declaration come from this translation unit?
                if (pp == shared->entryPointRequest->getTranslationUnit()->SyntaxNode.Ptr())
                    return false;

                return true;
            }
        }

        return false;
    }

    RefPtr<VarDeclBase> visitVariable(
        Variable* decl)
    {
        // Global variable? Check if it is a sample-rate input.
        if (dynamic_cast<ProgramSyntaxNode*>(decl->ParentDecl))
        {
            if (decl->HasModifier<InModifier>())
            {
                doSampleRateInputCheck(decl);
            }

            auto varLayout = tryToFindLayout(decl);
            if (varLayout)
            {
                auto inRes = varLayout->FindResourceInfo(LayoutResourceKind::VertexInput);
                auto outRes = varLayout->FindResourceInfo(LayoutResourceKind::FragmentOutput);

                if( (inRes || outRes) && isImportedStructType(decl->Type.type))
                {
                    // We are seemingly looking at a GLSL global-scope varying
                    // of an aggregate type which was imported from library
                    // code. We should destructure that into individual
                    // declarations.

                    // We can't easily support `in out` declarations with this approach
                    assert(!(inRes && outRes));

                    RefPtr<ExpressionSyntaxNode> loweredExpr;
                    if (inRes)
                    {
                        loweredExpr = lowerShaderParameterToGLSLGLobals(
                            decl,
                            varLayout,
                            VaryingParameterDirection::Input);
                    }

                    if (outRes)
                    {
                        loweredExpr = lowerShaderParameterToGLSLGLobals(
                            decl,
                            varLayout,
                            VaryingParameterDirection::Output);
                    }

                    assert(loweredExpr);
                    auto loweredDecl = createVaryingTupleVarDecl(
                        decl,
                        loweredExpr);

                    registerLoweredDecl(loweredDecl, decl);
                    return loweredDecl;
                }
            }
        }

        auto loweredDecl = lowerVarDeclCommon(decl, getClass<Variable>());
        if(!loweredDecl)
            return nullptr;

        return loweredDecl;
    }

    RefPtr<VarDeclBase> visitStructField(
        StructField* decl)
    {
        return lowerSimpleVarDeclCommon(new StructField(), decl);
    }

    RefPtr<VarDeclBase> visitParameterSyntaxNode(
        ParameterSyntaxNode* decl)
    {
        auto loweredDecl = lowerVarDeclCommon(decl, getClass<ParameterSyntaxNode>());
        return loweredDecl;
    }

    RefPtr<DeclBase> transformSyntaxField(DeclBase* decl)
    {
        return lowerDeclBase(decl);
    }


    RefPtr<Decl> visitDeclGroup(
        DeclGroup* group)
    {
        for (auto decl : group->decls)
        {
            lowerDecl(decl);
        }
        return nullptr;
    }

    RefPtr<FunctionSyntaxNode> visitFunctionDeclBase(
        FunctionDeclBase*   decl)
    {
        // TODO: need to generate a name

        RefPtr<FunctionSyntaxNode> loweredDecl = new FunctionSyntaxNode();
        lowerDeclCommon(loweredDecl, decl);

        // TODO: push scope for parent decl here...
        LoweringVisitor subVisitor = *this;
        subVisitor.parentDecl = loweredDecl;

        // If we are a being called recurisvely, then we need to
        // be careful not to let the context get polluted
        subVisitor.resultVariable = nullptr;
        subVisitor.stmtBeingBuilt = nullptr;
        subVisitor.isBuildingStmt = false;

        for (auto paramDecl : decl->GetParameters())
        {
            subVisitor.translateDeclRef(paramDecl);
        }

        auto loweredReturnType = subVisitor.lowerType(decl->ReturnType);

        loweredDecl->ReturnType = loweredReturnType;

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

    struct VaryingParameterVarChain
    {
        VaryingParameterVarChain*   next = nullptr;
        VarDeclBase*                varDecl;
    };

    template<typename T>
    T* findModifier(VaryingParameterVarChain* chain)
    {
        for (auto c = chain; c; c = c->next)
        {
            auto v = c->varDecl;
            if (auto mod = v->FindModifier<T>())
                return mod;
        }
        return nullptr;
    }

    RefPtr<Modifier> cloneModifier(Modifier* modifier)
    {
        if (!modifier) return nullptr;

        // For now we just do a shallow copy of the modifier

        CloneVisitor visitor;
        return visitor.dispatch(modifier);
    }

    struct VaryingParameterInfo
    {
        String                      name;
        VaryingParameterDirection   direction;
        VaryingParameterArraySpec*  arraySpecs = nullptr;
        VaryingParameterVarChain*   varChain = nullptr;
    };

    RefPtr<ExpressionSyntaxNode> createGLSLBuiltinRef(
        char const* name)
    {
        RefPtr<VarExpressionSyntaxNode> globalVarRef = new VarExpressionSyntaxNode();
        globalVarRef->name = name;
        return globalVarRef;
    }

    bool isIntegralType(
        ExpressionType* type)
    {
        if (auto baseType = type->As<BasicExpressionType>())
        {
            switch (baseType->BaseType)
            {
            default:
                return false;

            case BaseType::Int:
            case BaseType::UInt:
            case BaseType::UInt64:
                return true;
            }
        }
        else if (auto vecType = type->As<VectorExpressionType>())
        {
            return isIntegralType(vecType->elementType);
        }
        else if (auto matType = type->As<MatrixExpressionType>())
        {
            return isIntegralType(matType->getElementType());
        }

        return false;
    }

    void requireGLSLVersion(ProfileVersion version)
    {
        if (shared->target != CodeGenTarget::GLSL)
            return;

        auto entryPoint = shared->entryPointRequest;
        auto profile = entryPoint->profile;
        auto currentVersion = profile.GetVersion();
        if (profile.getFamily() == ProfileFamily::GLSL)
        {
            // Check if this profile is newer
            if ((UInt)version > (UInt)profile.GetVersion())
            {
                profile.setVersion(version);
                entryPoint->profile = profile;
            }
        }
        else
        {
            // Non-GLSL target? Set it to a GLSL one.
            profile.setVersion(version);
            entryPoint->profile = profile;
        }
    }

    RefPtr<ExpressionSyntaxNode> lowerSimpleShaderParameterToGLSLGlobal(
        VaryingParameterInfo const&     info,
        RefPtr<ExpressionType>          varType,
        RefPtr<VarLayout>               varLayout)
    {
        RefPtr<ExpressionType> type = varType;

        for (auto aa = info.arraySpecs; aa; aa = aa->next)
        {
            RefPtr<ArrayExpressionType> arrayType = new ArrayExpressionType();
            arrayType->BaseType = type;
            arrayType->ArrayLength = aa->elementCount;

            type = arrayType;
        }

        // We need to create a reference to the global-scope declaration
        // of the proper GLSL input/output variable. This might
        // be a user-defined input/output, or a system-defined `gl_` one.
        RefPtr<ExpressionSyntaxNode> globalVarExpr;

        // Handle system-value inputs/outputs
        assert(varLayout);
        auto systemValueSemantic = varLayout->systemValueSemantic;
        if (systemValueSemantic.Length() != 0)
        {
            auto ns = systemValueSemantic.ToLower();

            if (ns == "sv_target")
            {
                // Note: we do *not* need to generate some kind of `gl_`
                // builtin for fragment-shader outputs: they are just
                // ordinary `out` variables, with ordinary `location`s,
                // as far as GLSL is concerned.
            }
            else if (ns == "sv_position")
            {
                if (info.direction == VaryingParameterDirection::Input)
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_FragCoord");
                }
                else
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_Position");
                }
            }
            else if (ns == "sv_clipdistance")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_ClipDistance");
            }
            else if (ns == "sv_culldistance")
            {
                // TODO: ARB_cull_distance
                globalVarExpr = createGLSLBuiltinRef("gl_CullDistance");
            }
            else if (ns == "sv_coverage")
            {
                if (info.direction == VaryingParameterDirection::Input)
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_SampleMaskIn");
                }
                else
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_SampleMask");
                }
            }
            else if (ns == "sv_depth")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_FragDepth");
            }
            else if (ns == "sv_depthgreaterequal")
            {
                // TODO: layout(depth_greater) out float gl_FragDepth;
                globalVarExpr = createGLSLBuiltinRef("gl_FragDepth");
            }
            else if (ns == "sv_depthlessequal")
            {
                // TODO: layout(depth_less) out float gl_FragDepth;
                globalVarExpr = createGLSLBuiltinRef("gl_FragDepth");
            }
            else if (ns == "sv_dispatchthreadid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_GlobalInvocationID");
            }
            else if (ns == "sv_domainlocation")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_TessCoord");
            }
            else if (ns == "sv_groupid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_WorkGroupID");
            }
            else if (ns == "sv_groupindex")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_LocationInvocationIndex");
            }
            else if (ns == "sv_groupthreadid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_LocalInvocationID");
            }
            else if (ns == "sv_gsinstanceid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InvocationID");
            }
            else if (ns == "sv_insidetessfactor")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_TessLevelInner");
            }
            else if (ns == "sv_instanceid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InstanceIndex");
            }
            else if (ns == "sv_isfrontface")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_FrontFacing");
            }
            else if (ns == "sv_outputcontrolpointid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InvocationID");
            }
            else if (ns == "sv_outputcontrolpointid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InvocationID");
            }
            else if (ns == "sv_primitiveid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_PrimitiveID");
            }
            else if (ns == "sv_rendertargetarrayindex")
            {
                if (info.direction == VaryingParameterDirection::Input)
                {
                    requireGLSLVersion(ProfileVersion::GLSL_430);
                }
                globalVarExpr = createGLSLBuiltinRef("gl_Layer");
            }
            else if (ns == "sv_sampleindex")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_SampleID");
            }
            else if (ns == "sv_stencilref")
            {
                // TODO: ARB_shader_stencil_export
                globalVarExpr = createGLSLBuiltinRef("gl_SampleID");
            }
            else if (ns == "sv_tessfactor")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_TessLevelOuter");
            }
            else if (ns == "sv_vertexid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_VertexIndex");
            }
            else if (ns == "sv_viewportarrayindex")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_ViewportIndex");
            }
            else
            {
                assert(!"unhandled");
            }
        }

        // If we didn't match some kind of builtin input/output,
        // then declare a user input/output variable instead
        if (!globalVarExpr)
        {
            RefPtr<Variable> globalVarDecl = new Variable();
            globalVarDecl->Name.Content = info.name;
            globalVarDecl->Type.type = type;

            ensureDeclHasAValidName(globalVarDecl);

            addMember(shared->loweredProgram, globalVarDecl);

            // Add the layout information
            RefPtr<ComputedLayoutModifier> modifier = new ComputedLayoutModifier();
            modifier->layout = varLayout;
            addModifier(globalVarDecl, modifier);

            // Add appropriate in/out modifier
            switch (info.direction)
            {
            case VaryingParameterDirection::Input:
                addModifier(globalVarDecl, new InModifier());
                break;

            case VaryingParameterDirection::Output:
                addModifier(globalVarDecl, new OutModifier());
                break;
            }

            // We want to copy certain modifiers from the declaration as given,
            // over to the newly created global variable. The most important
            // of these is any interpolation-mode modifier.
            //
            // Note that a shader parameter could have been nested inside
            // a `struct` type, so we will look for interpolation modifiers
            // starting on the "deepest" field, and working out way out.

            // Look for interpolation mode modifier
            if (auto interpolationModeModifier = findModifier<InterpolationModeModifier>(info.varChain))
            {
                addModifier(globalVarDecl, cloneModifier(interpolationModeModifier));
            }
            // Otherwise, check if we need to add one:
            else if (isIntegralType(varType))
            {
                if (info.direction == VaryingParameterDirection::Input
                    && shared->entryPointRequest->profile.GetStage() == Stage::Vertex)
                {
                    // Don't add extra qualification to VS inputs
                }
                else
                {
                    auto mod = new HLSLNoInterpolationModifier();
                    addModifier(globalVarDecl, mod);
                }
            }


            RefPtr<VarExpressionSyntaxNode> globalVarRef = new VarExpressionSyntaxNode();
            globalVarRef->Position = globalVarDecl->Position;
            globalVarRef->declRef = makeDeclRef(globalVarDecl.Ptr());
            globalVarRef->name = globalVarDecl->getName();

            globalVarExpr = globalVarRef;
        }

        return globalVarExpr;
    }

    RefPtr<ExpressionSyntaxNode> lowerShaderParameterToGLSLGLobalsRec(
        VaryingParameterInfo const&     info,
        RefPtr<ExpressionType>          varType,
        RefPtr<VarLayout>               varLayout)
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

            // Note that we use the original `varLayout` that was passed in,
            // since that is the layout that will ultimately need to be
            // used on the array elements.
            //
            // TODO: That won't actually work if we ever had an array of
            // heterogeneous stuff...
            return lowerShaderParameterToGLSLGLobalsRec(
                arrayInfo,
                arrayType->BaseType,
                varLayout);
        }
        else if (auto declRefType = varType->As<DeclRefType>())
        {
            auto declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
            {
                // The shader parameter had a structured type, so we need
                // to destructure it into its constituent fields

                RefPtr<VaryingTupleExpr> tupleExpr = new VaryingTupleExpr();
                tupleExpr->Type.type = varType;

                assert(tupleExpr->Type.type);

                for (auto fieldDeclRef : getMembersOfType<VarDeclBase>(aggTypeDeclRef))
                {
                    // Don't emit storage for `static` fields here, of course
                    if (fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    VaryingParameterVarChain fieldVarChain;
                    fieldVarChain.next = info.varChain;
                    fieldVarChain.varDecl = fieldDeclRef.getDecl();

                    VaryingParameterInfo fieldInfo = info;
                    fieldInfo.name = info.name + "_" + fieldDeclRef.GetName();
                    fieldInfo.varChain = &fieldVarChain;

                    // Need to find the layout for the given field...
                    Decl* originalFieldDecl = nullptr;
                    shared->mapLoweredDeclToOriginal.TryGetValue(fieldDeclRef.getDecl(), originalFieldDecl);
                    assert(originalFieldDecl);

                    auto structTypeLayout = varLayout->typeLayout.As<StructTypeLayout>();
                    assert(structTypeLayout);

                    RefPtr<VarLayout> fieldLayout;
                    structTypeLayout->mapVarToLayout.TryGetValue(originalFieldDecl, fieldLayout);
                    assert(fieldLayout);

                    auto loweredFieldExpr = lowerShaderParameterToGLSLGLobalsRec(
                        fieldInfo,
                        GetType(fieldDeclRef),
                        fieldLayout);

                    VaryingTupleExpr::Element elem;
                    elem.originalFieldDeclRef = makeDeclRef(originalFieldDecl).As<VarDeclBase>();
                    elem.expr = loweredFieldExpr;

                    tupleExpr->elements.Add(elem);
                }

                // Okay, we are done with this parameter
                return tupleExpr;
            }
        }

        // Default case: just try to emit things as-is
        return lowerSimpleShaderParameterToGLSLGlobal(info, varType, varLayout);
    }

    RefPtr<ExpressionSyntaxNode> lowerShaderParameterToGLSLGLobals(
        RefPtr<VarDeclBase>         originalVarDecl,
        RefPtr<VarLayout>           paramLayout,
        VaryingParameterDirection   direction)
    {
        auto name = originalVarDecl->getName();
        auto declRef = makeDeclRef(originalVarDecl.Ptr());

        VaryingParameterVarChain varChain;
        varChain.next = nullptr;
        varChain.varDecl = originalVarDecl;

        VaryingParameterInfo info;
        info.name = name;
        info.direction = direction;
        info.varChain = &varChain;

        // Ensure that we don't get name collisions on `inout` variables
        switch (direction)
        {
        case VaryingParameterDirection::Input:
            info.name = "SLANG_in_" + name;
            break;

        case VaryingParameterDirection::Output:
            info.name = "SLANG_out_" + name;
            break;
        }

        auto loweredType = lowerType(originalVarDecl->Type);

        auto loweredExpr = lowerShaderParameterToGLSLGLobalsRec(
            info,
            loweredType.type,
            paramLayout);

#if 0
        RefPtr<VaryingTupleVarDecl> loweredDecl = createVaryingTupleVarDecl(
            originalVarDecl,
            loweredType,
            loweredExpr);

        registerLoweredDecl(loweredDecl, originalVarDecl);
        addDecl(loweredDecl);
#endif

        return loweredExpr;
    }

    RefPtr<VaryingTupleVarDecl> createVaryingTupleVarDecl(
        RefPtr<VarDeclBase>             originalVarDecl,
        TypeExp const&                  loweredType,
        RefPtr<ExpressionSyntaxNode>    loweredExpr)
    {
        RefPtr<VaryingTupleVarDecl> loweredDecl = new VaryingTupleVarDecl();
        loweredDecl->Name = originalVarDecl->Name;
        loweredDecl->Type = loweredType;
        loweredDecl->Expr = loweredExpr;

        return loweredDecl;
    }

    RefPtr<VaryingTupleVarDecl> createVaryingTupleVarDecl(
        RefPtr<VarDeclBase>             originalVarDecl,
        RefPtr<ExpressionSyntaxNode>    loweredExpr)
    {
        auto loweredType = lowerType(originalVarDecl->Type);
        return createVaryingTupleVarDecl(originalVarDecl, loweredType, loweredExpr);
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
        auto loweredEntryPointFunc = visitFunctionDeclBase(entryPointDecl);

        // Now we will generate a `void main() { ... }` function to call the lowered code.
        RefPtr<FunctionSyntaxNode> mainDecl = new FunctionSyntaxNode();
        mainDecl->ReturnType.type = ExpressionType::GetVoid();
        mainDecl->Name.Content = "main";

        // If the user's entry point was called `main` then rename it here
        if (loweredEntryPointFunc->getName() == "main")
            loweredEntryPointFunc->Name.Content = "main_";

        RefPtr<BlockStmt> bodyStmt = new BlockStmt();
        bodyStmt->scopeDecl = new ScopeDecl();

        // We will want to generate declarations into the body of our new `main()`
        LoweringVisitor subVisitor = *this;
        subVisitor.isBuildingStmt = true;
        subVisitor.stmtBeingBuilt = nullptr;
        subVisitor.parentDecl = bodyStmt->scopeDecl;

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

            ensureDeclHasAValidName(localVarDecl);

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
                auto loweredExpr = subVisitor.lowerShaderParameterToGLSLGLobals(
                    paramPair.original,
                    paramPair.layout,
                    VaryingParameterDirection::Input);

                subVisitor.assign(paramPair.lowered, loweredExpr);
            }
        }

        // Generate a local variable for the result, if any
        RefPtr<Variable> resultVarDecl;
        if (!loweredEntryPointFunc->ReturnType->Equals(ExpressionType::GetVoid()))
        {
            resultVarDecl = new Variable();
            resultVarDecl->Position = loweredEntryPointFunc->Position;
            resultVarDecl->Name.Content = "main_result";
            resultVarDecl->Type = TypeExp(loweredEntryPointFunc->ReturnType);

            ensureDeclHasAValidName(resultVarDecl);

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
                auto loweredExpr = subVisitor.lowerShaderParameterToGLSLGLobals(
                    paramPair.original,
                    paramPair.layout,
                    VaryingParameterDirection::Output);

                subVisitor.assign(loweredExpr, paramPair.lowered);
            }
        }
        if (resultVarDecl)
        {
            VaryingParameterInfo info;
            info.name = "SLANG_out_" + resultVarDecl->getName();
            info.direction = VaryingParameterDirection::Output;
            info.varChain = nullptr;

            auto loweredExpr = lowerShaderParameterToGLSLGLobalsRec(
                info,
                resultVarDecl->Type.type,
                entryPointLayout->resultLayout);

            subVisitor.assign(loweredExpr, resultVarDecl);
        }

        bodyStmt->body = subVisitor.stmtBeingBuilt;

        mainDecl->Body = bodyStmt;


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
            return visitFunctionDeclBase(entryPointDecl);

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
    sharedContext.compileRequest = entryPoint->compileRequest;
    sharedContext.entryPointRequest = entryPoint;
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

    // TODO: this should only need to take the shared context
    visitor.registerReservedWords();

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

    sharedContext.entryPointLayout = visitor.findEntryPointLayout(entryPoint);

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
