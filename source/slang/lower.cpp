// lower.cpp
#include "lower.h"

#include "emit.h"
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

    RefPtr<Stmt> transformDeclField(Stmt* stmt)
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

    RefPtr<Expr> transformSyntaxField(Expr* expr)
    {
        return visitor->transformSyntaxField(expr);
    }

    RefPtr<Stmt> transformSyntaxField(Stmt* stmt)
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
RefPtr<Stmt> structuralTransform(
    Stmt*    stmt,
    V*                      visitor)
{
    StructuralTransformStmtVisitor<V> transformer;
    transformer.visitor = visitor;
    return transformer.dispatch(stmt);
}

template<typename V>
struct StructuralTransformExprVisitor
    : StructuralTransformVisitorBase<V>
    , ExprVisitor<StructuralTransformExprVisitor<V>, RefPtr<Expr>>
{
    void transformFields(Expr* result, Expr* obj)
    {
        result->type = transformSyntaxField(obj->type);
    }


#define SYNTAX_CLASS(NAME, BASE, ...)                   \
    RefPtr<Expr> visit##NAME(NAME* obj) {     \
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
RefPtr<Expr> structuralTransform(
    Expr*   expr,
    V*                      visitor)
{
    StructuralTransformExprVisitor<V> transformer;
    transformer.visitor = visitor;
    return transformer.dispatch(expr);
}

//

class TupleExpr;
class TupleVarDecl;
class VaryingTupleExpr;
class VaryingTupleVarDecl;


// The result of lowering a declaration will usually be a declaration,
// but it might also be a "tuple" declaration, in cases where we needed
// to sclarize (or partially scalarize) things to guarantee validity.
struct LoweredDecl
{
    enum class Flavor
    {
        Decl,           // A single declaration (the default case)
        Tuple,          // A `TupleVarDecl` representing multiple decls
        VaryingTuple,   // A `VaryingTupleVarDecl` representing multiple decls
    };

    LoweredDecl()
        : flavor(Flavor::Decl)
    {}

    LoweredDecl(Decl* decl)
        : value(decl)
        , flavor(Flavor::Decl)
    {}

    LoweredDecl(TupleVarDecl* decl)
        : value((RefObject*) decl)
        , flavor(Flavor::Tuple)
    {}

    LoweredDecl(VaryingTupleVarDecl* decl)
        : value((RefObject*) decl)
        , flavor(Flavor::VaryingTuple)
    {}

    Flavor getFlavor() const { return flavor; }
    RefObject* getValue() const { return value; }

    Decl* getDecl() const
    {
        SLANG_ASSERT(getFlavor() == Flavor::Decl);
        return (Decl*) value.Ptr();
    }

    TupleVarDecl* getTupleDecl() const
    {
        SLANG_ASSERT(getFlavor() == Flavor::Tuple);
        return (TupleVarDecl*) value.Ptr();
    }

    VaryingTupleVarDecl* getVaryingTupleDecl() const
    {
        SLANG_ASSERT(getFlavor() == Flavor::VaryingTuple);
        return (VaryingTupleVarDecl*) value.Ptr();
    }

    Decl* asDecl() const
    {
        return (getFlavor() == Flavor::Decl) ? getDecl() : nullptr;
    }

    TupleVarDecl* asTupleDecl() const
    {
        return (getFlavor() == Flavor::Tuple) ? getTupleDecl() : nullptr;
    }

    VaryingTupleVarDecl* asVaryingTupleDecl() const
    {
        return (getFlavor() == Flavor::VaryingTuple) ? getVaryingTupleDecl() : nullptr;
    }

private:
    RefPtr<RefObject>   value;
    Flavor              flavor;
};

struct LoweredDeclRef
{
public:
    LoweredDecl             decl;
    RefPtr<Substitutions>   substitutions;

    LoweredDecl getDecl() { return decl; }

    template<typename T>
    DeclRef<T> As()
    {
        return DeclRef<Decl>(decl.getDecl(), substitutions).As<T>();
    }
};


// The result of lowering an exrpession will usually be just a single
// expression, but it might also be a "tuple" expression that encodes
// multiple expressions.
struct LoweredExpr
{
    enum class Flavor
    {
        Expr,
        Tuple,
        VaryingTuple,
    };

    LoweredExpr()
        : flavor(Flavor::Expr)
    {}

    LoweredExpr(Expr* expr)
        : value(expr)
        , flavor(Flavor::Expr)
    {}

    LoweredExpr(TupleExpr* expr)
        : value((RefObject*) expr)
        , flavor(Flavor::Tuple)
    {}

    LoweredExpr(VaryingTupleExpr* expr)
        : value((RefObject*) expr)
        , flavor(Flavor::VaryingTuple)
    {}

    Flavor getFlavor() const { return flavor; }

    Expr* getExpr() const
    {
        assert(getFlavor() == Flavor::Expr);
        return (Expr*)value.Ptr();
    }

    TupleExpr* getTupleExpr() const
    {
        assert(getFlavor() == Flavor::Tuple);
        return (TupleExpr*)value.Ptr();
    }


    VaryingTupleExpr* getVaryingTupleExpr() const
    {
        assert(getFlavor() == Flavor::VaryingTuple);
        return (VaryingTupleExpr*)value.Ptr();
    }

    Expr* asExpr() const
    {
        return (getFlavor() == Flavor::Expr) ? getExpr() : nullptr;
    }

    TupleExpr* asTuple() const
    {
        return (getFlavor() == Flavor::Tuple) ? getTupleExpr() : nullptr;
    }

    VaryingTupleExpr* asVaryingTuple() const
    {
        return (getFlavor() == Flavor::VaryingTuple) ? getVaryingTupleExpr() : nullptr;
    }

private:
    RefPtr<RefObject>   value;
    Flavor              flavor;
};

// Pseudo-syntax used during lowering
class PseudoVarDecl : public RefObject
{
public:
    NameLoc nameAndLoc;
    SourceLoc loc;
    TypeExp type;
};

class TupleVarDecl : public PseudoVarDecl
{
public:
    struct Element
    {
        RefPtr<TupleVarModifier>    tupleVarMod;
        LoweredDecl                 decl;
    };

    TupleTypeModifier*      tupleType;
    RefPtr<VarDeclBase>     primaryDecl;
    List<Element>           tupleDecls;
};

class PseudoExpr : public RefObject
{
public:
    SourceLoc loc;
    QualType type;
};

// Pseudo-syntax used during lowering:
// represents an ordered list of expressions as a single unit
class TupleExpr : public PseudoExpr
{
public:
    struct Element
    {
        DeclRef<VarDeclBase>    tupleFieldDeclRef;
        LoweredExpr             expr;
    };

    // Optional reference to the "primary" value of the tuple,
    // in the case of a tuple type with "orinary" fields
    RefPtr<Expr>    primaryExpr;

    // Additional fields to store values for any non-ordinary fields
    // (or fields that aren't exclusively orginary)
    List<Element>                   tupleElements;
};

// Pseudo-syntax used during lowering
class VaryingTupleVarDecl : public PseudoVarDecl
{
public:
    LoweredExpr     expr;
};

// Pseudo-syntax used during lowering:
// represents an ordered list of expressions as a single unit
class VaryingTupleExpr : public PseudoExpr
{
public:
    struct Element
    {
        DeclRef<VarDeclBase>    originalFieldDeclRef;
        LoweredExpr             expr;
    };

    List<Element>   elements;
};

static SourceLoc getPosition(LoweredExpr const& expr)
{
    switch (expr.getFlavor())
    {
    case LoweredExpr::Flavor::Expr:         return expr.getExpr()            ->loc;
    case LoweredExpr::Flavor::Tuple:        return expr.getTupleExpr()       ->loc;
    case LoweredExpr::Flavor::VaryingTuple: return expr.getVaryingTupleExpr()->loc;
    default:
        SLANG_UNREACHABLE("all cases handled");
        return SourceLoc();
    }
}


struct SharedLoweringContext
{
    CompileRequest*     compileRequest;
    EntryPointRequest*  entryPointRequest;

    ExtensionUsageTracker* extensionUsageTracker;

    ProgramLayout*      programLayout;
    EntryPointLayout*   entryPointLayout;

    // The target we are going to generate code for.
    // 
    // We may need to specialize how constructs get lowered based
    // on the capabilities of the target language.
    CodeGenTarget   target;

    // A set of words reserved by the target
    HashSet<Name*> reservedWords;


    RefPtr<ModuleDecl>   loweredProgram;

    Dictionary<Decl*, LoweredDecl> loweredDecls;
    Dictionary<RefObject*, Decl*> mapLoweredDeclToOriginal;

    // Work to be done at the very start and end of the entry point
    RefPtr<Stmt> entryPointInitializeStmt;
    RefPtr<Stmt> entryPointFinalizeStmt;

    // Counter used for generating unique temporary names
    int nameCounter = 0;

    bool isRewrite = false;
    bool requiresCopyGLPositionToPositionPerView = false;
};

static void attachLayout(
    ModifiableSyntaxNode*   syntax,
    Layout*                 layout)
{
    RefPtr<ComputedLayoutModifier> modifier = new ComputedLayoutModifier();
    modifier->layout = layout;

    addModifier(syntax, modifier);
}

void requireGLSLVersion(
    EntryPointRequest*  entryPoint,
    ProfileVersion      version);

struct LoweringVisitor
    : ExprVisitor<LoweringVisitor, LoweredExpr>
    , StmtVisitor<LoweringVisitor, void>
    , DeclVisitor<LoweringVisitor, LoweredDecl>
    , ValVisitor<LoweringVisitor, RefPtr<Val>, RefPtr<Type>>
{
    //
    SharedLoweringContext*      shared;
    RefPtr<Substitutions>       substitutions;

    bool                        isBuildingStmt = false;
    RefPtr<Stmt> stmtBeingBuilt;

    // If we *aren't* building a statement, then this
    // is the container we should be adding declarations to
    RefPtr<ContainerDecl>       parentDecl;

    // If we are in a context where a `return` should be turned
    // into assignment to a variable (followed by a `return`),
    // then this will point to that variable.
    RefPtr<Variable>            resultVariable;

    Session* getSession()
    {
        return shared->compileRequest->mSession;
    }

    CodeGenTarget getTarget() { return shared->target; }

    bool isReservedWord(Name* name)
    {
        return shared->reservedWords.Contains(name);
    }

    void registerReservedWord(
        String const&   text)
    {
        Name* name = shared->compileRequest->getNamePool()->getName(text);
        shared->reservedWords.Add(name);
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

    RefPtr<Type> lowerType(
        Type* type)
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

    RefPtr<Type> visitErrorType(ErrorType* type)
    {
        return type;
    }

    RefPtr<Type> visitOverloadGroupType(OverloadGroupType* type)
    {
        return type;
    }

    RefPtr<Type> visitInitializerListType(InitializerListType* type)
    {
        return type;
    }

    RefPtr<Type> visitGenericDeclRefType(GenericDeclRefType* type)
    {
        return getGenericDeclRefType(
            type->getSession(),
            translateDeclRef(DeclRef<Decl>(type->declRef)).As<GenericDecl>());
    }

    RefPtr<Type> visitFuncType(FuncType* type)
    {
        RefPtr<FuncType> loweredType = getFuncType(
            getSession(),
            translateDeclRef(DeclRef<Decl>(type->declRef)).As<CallableDecl>());
        return loweredType;
    }

    RefPtr<Type> visitDeclRefType(DeclRefType* type)
    {
        auto loweredDeclRef = translateDeclRef(type->declRef);
        return DeclRefType::Create(
            type->getSession(),
            loweredDeclRef.As<Decl>());
    }

    RefPtr<Type> visitNamedExpressionType(NamedExpressionType* type)
    {
        if (shared->target == CodeGenTarget::GLSL)
        {
            // GLSL does not support `typedef`, so we will lower it out of existence here
            return lowerType(GetType(type->declRef));
        }

        return getNamedType(
            type->getSession(),
            translateDeclRef(DeclRef<Decl>(type->declRef)).As<TypeDefDecl>());
    }

    RefPtr<Type> visitTypeType(TypeType* type)
    {
        return getTypeType(lowerType(type->type));
    }

    RefPtr<Type> visitArrayExpressionType(ArrayExpressionType* type)
    {
        RefPtr<ArrayExpressionType> loweredType = Slang::getArrayType(
            lowerType(type->BaseType),
            lowerVal(type->ArrayLength).As<IntVal>());
        return loweredType;
    }

    RefPtr<Type> transformSyntaxField(Type* type)
    {
        return lowerType(type);
    }

    //
    // Expressions
    //

    LoweredExpr lowerExprOrTuple(
        Expr* expr)
    {
        if (!expr) return LoweredExpr();
        return ExprVisitor::dispatch(expr);
    }

    RefPtr<Expr> lowerExpr(
        Expr* expr)
    {
        if (!expr) return nullptr;

        auto result = lowerExprOrTuple(expr);
        return maybeReifyTuple(result);
    }

    // catch-all
    LoweredExpr visitExpr(
        Expr* expr)
    {
        return LoweredExpr(structuralTransform(expr, this));
    }

    RefPtr<Expr> transformSyntaxField(Expr* expr)
    {
        return lowerExpr(expr);
    }

    void lowerExprCommon(
        Expr*    loweredExpr,
        Expr*    expr)
    {
        loweredExpr->loc = expr->loc;
        loweredExpr->type.type = lowerType(expr->type.type);
    }

    void lowerExprCommon(
        LoweredExpr const&      loweredExpr,
        Expr*   expr)
    {
        if (auto simpleExpr = loweredExpr.asExpr())
        {
            lowerExprCommon(simpleExpr, expr);
        }
    }

    RefPtr<Expr> createUncheckedVarRef(
        Name* name)
    {
        RefPtr<VarExpr> result = new VarExpr();
        result->name = name;
        return result;
    }


    RefPtr<Expr> createUncheckedVarRef(
        char const* name)
    {
        return createUncheckedVarRef(
            shared->compileRequest->getNamePool()->getName(name));
    }

    RefPtr<Expr> createSimpleVarRef(
        SourceLoc const& loc,
        VarDeclBase*        decl)
    {
        RefPtr<VarExpr> result = new VarExpr();
        result->loc = loc;
        result->type.type = decl->type.type;
        result->declRef = makeDeclRef(decl);
        result->name = decl->getName();
        return result;
    }

    LoweredExpr createVarRef(
        SourceLoc const& loc,
        LoweredDecl const&  decl)
    {
        switch (decl.getFlavor())
        {
        case LoweredDecl::Flavor::Decl:
            return LoweredExpr(createSimpleVarRef(loc, decl.getDecl()->As<VarDeclBase>()));

        case LoweredDecl::Flavor::Tuple:
            return createTupleRef(loc, decl.getTupleDecl());

        case LoweredDecl::Flavor::VaryingTuple:
            return createVaryingTupleRef(loc, decl.getVaryingTupleDecl());

        default:
            SLANG_UNREACHABLE("all cases handled");
            return LoweredExpr();
        }
    }


    LoweredExpr createTupleRef(
        SourceLoc const&     loc,
        TupleVarDecl*           decl)
    {
        RefPtr<TupleExpr> result = new TupleExpr();
        result->loc = loc;
        result->type.type = decl->type.type;

        if (auto primaryDecl = decl->primaryDecl)
        {
            result->primaryExpr = createSimpleVarRef(loc, primaryDecl);
        }

        for (auto declElem : decl->tupleDecls)
        {
            auto tupleVarMod = declElem.tupleVarMod;
            SLANG_RELEASE_ASSERT(tupleVarMod);
            auto tupleFieldMod = tupleVarMod->tupleField;
            SLANG_RELEASE_ASSERT(tupleFieldMod);
            SLANG_RELEASE_ASSERT(tupleFieldMod->decl);

            TupleExpr::Element elem;
            elem.tupleFieldDeclRef = makeDeclRef(tupleFieldMod->decl);
            elem.expr = createVarRef(loc, declElem.decl);
            result->tupleElements.Add(elem);
        }

        return LoweredExpr(result);
    }

    LoweredExpr createVaryingTupleRef(
        SourceLoc const&     /*loc*/,
        VaryingTupleVarDecl*    decl)
    {
        return decl->expr;
    }

    LoweredExpr visitVarExpr(
        VarExpr* expr)
    {
        doSampleRateInputCheck(expr->name);

        // If the expression didn't get resolved, we can leave it as-is
        if (!expr->declRef)
            return expr;

        auto loweredDeclRef = translateDeclRef(expr->declRef);
        auto loweredDecl = loweredDeclRef.getDecl();

        if (auto tupleVarDecl = loweredDecl.asTupleDecl())
        {
            // If we are referencing a declaration that got tuple-ified,
            // then we need to produce a tuple expression as well.

            return createTupleRef(expr->loc, tupleVarDecl);
        }
        else if (auto varyingTupleVarDecl = loweredDecl.asVaryingTupleDecl())
        {
            return createVaryingTupleRef(expr->loc, varyingTupleVarDecl);
        }

        RefPtr<VarExpr> loweredExpr = new VarExpr();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->declRef = loweredDeclRef.As<Decl>();
        loweredExpr->name = expr->name;
        return LoweredExpr(loweredExpr);
    }

    Name* getName(String const& text)
    {
        return shared->compileRequest->getNamePool()->getName(text);
    }

    Name* generateName()
    {
        int id = shared->nameCounter++;

        String result;
        result.append("SLANG_tmp_");
        result.append(id);
        return getName(result);
    }

    RefPtr<Expr> moveTemp(RefPtr<Expr> expr)
    {
        RefPtr<Variable> varDecl = new Variable();
        varDecl->nameAndLoc.name = generateName();
        varDecl->type.type = expr->type.type;
        varDecl->initExpr = expr;

        addDecl(varDecl);

        return createSimpleVarRef(expr->loc, varDecl);
    }

    // The idea of this function is to take an expression that we plan to
    // use/evaluate more than once, and if needed replace it with a
    // reference to a temporary (initialized with the expr) so that it
    // can safely be re-evaluated.
    RefPtr<Expr> maybeMoveTemp(
        Expr* expr)
    {
        // TODO: actually implement this properly!

        // Certain expressions are already in a form we can directly re-use,
        // so  there is no reason to move them.
        if (dynamic_cast<VarExpr*>(expr))
            return expr;
        if (dynamic_cast<ConstantExpr*>(expr))
            return expr;

        // In the general case, though, we need to introduce a temporary
        return moveTemp(expr);
    }

    LoweredExpr maybeMoveTemp(
        LoweredExpr expr)
    {
        if (auto tupleExpr = expr.asTuple())
        {
            RefPtr<TupleExpr> resultExpr = new TupleExpr();
            resultExpr->loc = tupleExpr->loc;
            resultExpr->type = tupleExpr->type;
            if (tupleExpr->primaryExpr)
            {
                resultExpr->primaryExpr = maybeMoveTemp(tupleExpr->primaryExpr);
            }
            for (auto ee : tupleExpr->tupleElements)
            {
                TupleExpr::Element elem;
                elem.tupleFieldDeclRef = ee.tupleFieldDeclRef;
                elem.expr = maybeMoveTemp(ee.expr);

                resultExpr->tupleElements.Add(elem);
            }

            return LoweredExpr(resultExpr);
        }
        else if (auto varyingTupleExpr = expr.asVaryingTuple())
        {
            RefPtr<VaryingTupleExpr> resultExpr = new VaryingTupleExpr();
            resultExpr->loc = varyingTupleExpr->loc;
            resultExpr->type = varyingTupleExpr->type;
            for (auto ee : varyingTupleExpr->elements)
            {
                VaryingTupleExpr::Element elem;
                elem.originalFieldDeclRef = ee.originalFieldDeclRef;
                elem.expr = maybeMoveTemp(ee.expr);

                resultExpr->elements.Add(elem);
            }

            return LoweredExpr(resultExpr);
        }
        else
        {
            return LoweredExpr(maybeMoveTemp(expr.getExpr()));
        }
    }

    // Similar to the above, this ensures that an l-value expression
    // is safe to re-evaluate, by recursively moving things off
    // to temporaries where needed.
    RefPtr<Expr> ensureSimpleLValue(
        Expr* expr)
    {
        // TODO: actually implement this properly!

        return expr;
    }

    LoweredExpr ensureSimpleLValue(
        LoweredExpr expr)
    {
        // TODO: actually implement this properly!

        return expr;
    }

    // When constructing assignment syntax, we can either
    // just leave things alone, or create code that will
    // try to coerce types to "fix up" differences in
    // the apparent type of things.
    enum class AssignMode
    {
        Default,
        WithFixups,
    };

    RefPtr<Expr> createSimpleAssignExpr(
        RefPtr<Expr>    leftExpr,
        RefPtr<Expr>    rightExpr)
    {
        RefPtr<AssignExpr> loweredExpr = new AssignExpr();
        loweredExpr->type = leftExpr->type;
        loweredExpr->left = leftExpr;
        loweredExpr->right = rightExpr;
        return loweredExpr;
    }

    RefPtr<Expr> convertExprForAssignmentWithFixups(
        RefPtr<Type>          leftType,
        RefPtr<Expr>    rightExpr)
    {
        auto rightType = rightExpr->type.type;
        if (auto leftArrayType = leftType->As<ArrayExpressionType>())
        {
            // LHS type was an array

            if (auto rightVecType = rightType->As<VectorExpressionType>())
            {
                // RHS type was a vector
                if (auto leftElemVecType = leftArrayType->BaseType->As<VectorExpressionType>())
                {
                    // LHS element type was also a vector, so this is a "scalar splat
                    // to array" case.
                }
                else
                {
                    // LHS is an array of non-vectors, while RHS is a vector,
                    // so in this case we want to splat out the vector elements
                    // to create an array and use that.
                    rightExpr = maybeMoveTemp(rightExpr);

                    RefPtr<AggTypeCtorExpr> ctorExpr = new AggTypeCtorExpr();
                    ctorExpr->loc = rightExpr->loc;
                    ctorExpr->type.type = leftType;
                    ctorExpr->base.type = leftType;

                    int elementCount = (int) GetIntVal(rightVecType->elementCount);
                    for (int ee = 0; ee < elementCount; ++ee)
                    {
                        RefPtr<SwizzleExpr> swizzleExpr = new SwizzleExpr();
                        swizzleExpr->loc = rightExpr->loc;
                        swizzleExpr->type.type = rightVecType->elementType;
                        swizzleExpr->base = rightExpr;
                        swizzleExpr->elementCount = 1;
                        swizzleExpr->elementIndices[0] = ee;

                        auto convertedArgExpr = convertExprForAssignmentWithFixups(
                            leftArrayType->BaseType,
                            swizzleExpr);

                        ctorExpr->Arguments.Add(convertedArgExpr);
                    }

                    return ctorExpr;
                }
            }
        }

        // Default case: if the types didn't match, try to insert
        // an explicit cast to deal with the issue.
        return createCastExpr(leftType, rightExpr);

    }

    RefPtr<Expr> createConstIntExpr(IntegerLiteralValue value)
    {
        RefPtr<ConstantExpr> expr = new ConstantExpr();
        expr->type.type = getIntType();
        expr->ConstType = ConstantExpr::ConstantType::Int;
        expr->integerValue = value;
        return expr;
    }

    struct SeqExprBuilder
    {
        RefPtr<Expr> expr;
        RefPtr<Expr>* link = nullptr;
    };

    RefPtr<Expr> createSimpleVarExpr(Name* name)
    {
        RefPtr<VarExpr> varExpr = new VarExpr();
        varExpr->name = name;
        return varExpr;
    }

    RefPtr<Expr> createSimpleVarExpr(char const* name)
    {
        return createSimpleVarExpr(getName(name));
    }

    RefPtr<InvokeExpr> createSeqExpr(
        RefPtr<Expr>    left,
        RefPtr<Expr>    right)
    {
        RefPtr<InfixExpr> seqExpr = new InfixExpr();
        seqExpr->loc = left->loc;
        seqExpr->type = right->type;
        seqExpr->FunctionExpr = createSimpleVarExpr(",");
        seqExpr->Arguments.Add(left);
        seqExpr->Arguments.Add(right);
        return seqExpr;
    }

    void addExpr(SeqExprBuilder* builder, RefPtr<Expr> expr)
    {
        // No expression to add? Do nothing.
        if (!expr) return;

        if (!builder->expr)
        {
            // No expression so far?
            // Set up a single-expression result.

            builder->expr = expr;
            builder->link = &builder->expr;
            return;
        }

        // There is an existing expression, so we need to append
        // to the sequence of expressions. The invariant is
        // that `link` points to the last expression in the
        // sequence.

        // We will extract the old last element, and construct
        // a new sequence expression ("operator comma") that
        // concatenates with with our new last expression.
        auto oldLastExpr = *builder->link;
        auto seqExpr = createSeqExpr(oldLastExpr, expr);

        // Now we need to overwrite the old last expression,
        // wherever it occured in the AST (which we handily
        // stored in `link`) and set our `link` to track
        // the new last expression (which will be the second
        // argument to our sequence expression).
        *builder->link = seqExpr;
        builder->link = &seqExpr->Arguments[1];
    }

    RefPtr<Expr> createSimpleAssignExprWithFixups(
        RefPtr<Expr>    leftExpr,
        RefPtr<Expr>    rightExpr)
    {
        auto leftType = leftExpr->type.type;
        auto rightType = rightExpr->type.type;

        // If types are unknown, or match, then just do
        // things the ordinary way.
        if (!leftType
            || !rightType
            || leftType->As<ErrorType>()
            || rightType->As<ErrorType>()
            || leftType->Equals(rightType))
        {
            return createSimpleAssignExpr(leftExpr, rightExpr);
        }

        // Otherwise, start to look at the types involved,
        // and see if we can do something.

        if (auto leftArrayType = leftType->As<ArrayExpressionType>())
        {
            // LHS type was an array

            if (auto rightVecType = rightType->As<VectorExpressionType>())
            {
                // RHS type was a vector
                if (auto leftElemVecType = leftArrayType->BaseType->As<VectorExpressionType>())
                {
                    // LHS element type was also a vector, so this is a "scalar splat
                    // to array" case.
                }
                else
                {
                    // LHS is an array of non-vectors, while RHS is a vector,
                    // so in this case we want to splat out the vector elements
                    // to create an array and use that.
                    leftExpr = maybeMoveTemp(leftExpr);
                    rightExpr = maybeMoveTemp(rightExpr);

                    SeqExprBuilder builder;

                    int elementCount = (int) GetIntVal(rightVecType->elementCount);
                    for (int ee = 0; ee < elementCount; ++ee)
                    {
                        // LHS array element
                        RefPtr<IndexExpr> arrayElemExpr = new IndexExpr();
                        arrayElemExpr->loc = leftExpr->loc;
                        arrayElemExpr->type.type = leftArrayType->BaseType;
                        arrayElemExpr->BaseExpression = leftExpr;
                        arrayElemExpr->IndexExpression = createConstIntExpr(ee);

                        // RHS swizzle
                        RefPtr<SwizzleExpr> swizzleExpr = new SwizzleExpr();
                        swizzleExpr->loc = rightExpr->loc;
                        swizzleExpr->type.type = rightVecType->elementType;
                        swizzleExpr->base = rightExpr;
                        swizzleExpr->elementCount = 1;
                        swizzleExpr->elementIndices[0] = ee;

                        auto elemAssignExpr = createSimpleAssignExprWithFixups(
                            arrayElemExpr,
                            swizzleExpr);

                        addExpr(&builder, elemAssignExpr);
                    }

                    return builder.expr;
                }
            }
        }




        // TODO: are there any cases we can't solve with a cast?

        // Try to convert the right-hand-side expression to have the type
        // we expect on the left-hand side
        auto convertedRightExpr = convertExprForAssignmentWithFixups(leftType, rightExpr);
        return createSimpleAssignExpr(leftExpr, convertedRightExpr);
    }

    RefPtr<Expr> createSimpleAssignExpr(
        Expr*   leftExpr,
        Expr*   rightExpr,
        AssignMode              mode)
    {
        switch (mode)
        {
        default:
            return createSimpleAssignExpr(leftExpr, rightExpr);

        case AssignMode::WithFixups:
            return createSimpleAssignExprWithFixups(leftExpr, rightExpr);
        }
    }

    LoweredExpr createAssignExpr(
        LoweredExpr   leftExpr,
        LoweredExpr   rightExpr,
        AssignMode    mode = AssignMode::Default)
    {
        auto leftTuple = leftExpr.asTuple();
        auto rightTuple = rightExpr.asTuple();
        if (leftTuple && rightTuple)
        {
            RefPtr<TupleExpr> resultTuple = new TupleExpr();
            resultTuple->type = leftTuple->type;

            if (leftTuple->primaryExpr)
            {
                SLANG_RELEASE_ASSERT(rightTuple->primaryExpr);

                resultTuple->primaryExpr = createSimpleAssignExpr(
                    leftTuple->primaryExpr,
                    rightTuple->primaryExpr,
                    mode);
            }

            auto elementCount = leftTuple->tupleElements.Count();
            SLANG_RELEASE_ASSERT(elementCount == rightTuple->tupleElements.Count());
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto leftElement = leftTuple->tupleElements[ee];
                auto rightElement = rightTuple->tupleElements[ee];

                TupleExpr::Element resultElement;

                resultElement.tupleFieldDeclRef = leftElement.tupleFieldDeclRef;
                resultElement.expr = createAssignExpr(
                    leftElement.expr,
                    rightElement.expr,
                    mode);

                resultTuple->tupleElements.Add(resultElement);
            }

            return LoweredExpr(resultTuple);
        }
        else
        {
            SLANG_RELEASE_ASSERT(!leftTuple && !rightTuple);
        }

        auto leftVaryingTuple = leftExpr.asVaryingTuple();
        auto rightVaryingTuple = rightExpr.asVaryingTuple();

        RefPtr<Expr> leftSimpleExpr = leftExpr.asExpr();
        RefPtr<Expr> rightSimpleExpr = rightExpr.asExpr();

        if (leftVaryingTuple && rightVaryingTuple)
        {
            RefPtr<VaryingTupleExpr> resultTuple = new VaryingTupleExpr();
            resultTuple->type.type = leftVaryingTuple->type.type;
            resultTuple->loc = leftVaryingTuple->loc;

            SLANG_RELEASE_ASSERT(resultTuple->type.type);

            UInt elementCount = leftVaryingTuple->elements.Count();
            SLANG_RELEASE_ASSERT(elementCount == rightVaryingTuple->elements.Count());

            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto leftElem = leftVaryingTuple->elements[ee];
                auto rightElem = rightVaryingTuple->elements[ee];

                VaryingTupleExpr::Element elem;
                elem.originalFieldDeclRef = leftElem.originalFieldDeclRef;
                elem.expr = createAssignExpr(
                    leftElem.expr,
                    rightElem.expr,
                    mode);
            }

            return LoweredExpr(resultTuple);
        }
        else if (leftVaryingTuple && rightSimpleExpr)
        {
            // Assigning from ordinary expression on RHS to tuple.
            // This will naturally yield a tuple expression.

            RefPtr<VaryingTupleExpr> resultTuple = new VaryingTupleExpr();
            resultTuple->type.type = leftVaryingTuple->type.type;
            resultTuple->loc = leftVaryingTuple->loc;

            SLANG_RELEASE_ASSERT(resultTuple->type.type);

            UInt elementCount = leftVaryingTuple->elements.Count();

            // Move everything into temps if we can
            rightSimpleExpr = maybeMoveTemp(rightSimpleExpr);
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto& leftElem = leftVaryingTuple->elements[ee];
                leftElem.expr = ensureSimpleLValue(leftElem.expr);
            }

            //

            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto leftElem = leftVaryingTuple->elements[ee];


                RefPtr<MemberExpr> rightElemExpr = new MemberExpr();
                rightElemExpr->loc = rightSimpleExpr->loc;
                rightElemExpr->type.type = GetType(leftElem.originalFieldDeclRef);
                rightElemExpr->declRef = leftElem.originalFieldDeclRef;
                rightElemExpr->name = leftElem.originalFieldDeclRef.GetName();
                rightElemExpr->BaseExpression = rightSimpleExpr;

                VaryingTupleExpr::Element elem;
                elem.originalFieldDeclRef = leftElem.originalFieldDeclRef;
                elem.expr = createAssignExpr(
                    leftElem.expr,
                    LoweredExpr(rightElemExpr),
                    mode);

                resultTuple->elements.Add(elem);
            }

            return LoweredExpr(resultTuple);
        }
        else if (leftSimpleExpr && rightVaryingTuple)
        {
            // Pretty much the same as the above case, and we should
            // probably try to share code eventually.


            RefPtr<VaryingTupleExpr> resultTuple = new VaryingTupleExpr();
            resultTuple->type.type = leftSimpleExpr->type.type;
            resultTuple->loc = leftSimpleExpr->loc;

            SLANG_RELEASE_ASSERT(resultTuple->type.type);

            UInt elementCount = rightVaryingTuple->elements.Count();

            // Move everything into temps if we can
            leftSimpleExpr = ensureSimpleLValue(leftSimpleExpr);
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto& rightElem = rightVaryingTuple->elements[ee];
                rightElem.expr = maybeMoveTemp(rightElem.expr);
            }


            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                auto rightElem = rightVaryingTuple->elements[ee];


                RefPtr<MemberExpr> leftElemExpr = new MemberExpr();
                leftElemExpr->loc = leftSimpleExpr->loc;
                leftElemExpr->type.type = GetType(rightElem.originalFieldDeclRef);
                leftElemExpr->declRef = rightElem.originalFieldDeclRef;
                leftElemExpr->name = rightElem.originalFieldDeclRef.GetName();
                leftElemExpr->BaseExpression = leftSimpleExpr;

                VaryingTupleExpr::Element elem;
                elem.originalFieldDeclRef = rightElem.originalFieldDeclRef;
                elem.expr = createAssignExpr(
                    LoweredExpr(leftElemExpr),
                    rightElem.expr,
                    mode);

                resultTuple->elements.Add(elem);
            }

            return LoweredExpr(resultTuple);
        }
        else if (leftSimpleExpr && rightSimpleExpr)
        {
            // Default case: no tuples of any kind...

            return LoweredExpr(createSimpleAssignExpr(leftSimpleExpr, rightSimpleExpr, mode));
        }
        else
        {
            // Some case wasn't handled: diagnose!
            SLANG_UNEXPECTED("bad combination of tuple types");
        }
    }

    LoweredExpr visitAssignExpr(
        AssignExpr* expr)
    {
        auto leftExpr = lowerExprOrTuple(expr->left);
        auto rightExpr = lowerExprOrTuple(expr->right);

        auto loweredExpr = createAssignExpr(leftExpr, rightExpr);
        lowerExprCommon(loweredExpr, expr);
        return loweredExpr;
    }

    RefPtr<Type> getSubscripResultType(
        RefPtr<Type>  type)
    {
        if (auto arrayType = type->As<ArrayExpressionType>())
        {
            return arrayType->BaseType;
        }
        return nullptr;
    }

    RefPtr<Expr> createSimpleSubscriptExpr(
        RefPtr<Expr>    baseExpr,
        RefPtr<Expr>    indexExpr)
    {
        // Default case: just reconstrut a subscript expr
        auto loweredExpr = new IndexExpr();

        loweredExpr->type.type = getSubscripResultType(baseExpr->type.type);

        loweredExpr->BaseExpression = baseExpr;
        loweredExpr->IndexExpression = indexExpr;
        return loweredExpr;
    }

    LoweredExpr createSubscriptExpr(
        LoweredExpr                     baseExpr,
        RefPtr<Expr>    indexExpr)
    {
        // TODO: This logic ends up duplicating the `indexExpr`
        // that was given, without worrying about any side
        // effects it might contain. That needs to be fixed.

        if (auto baseTuple = baseExpr.asTuple())
        {
            indexExpr = maybeMoveTemp(indexExpr);

            auto loweredExpr = new TupleExpr();
            loweredExpr->type.type = getSubscripResultType(baseTuple->type.type);

            if (auto basePrimary = baseTuple->primaryExpr)
            {
                loweredExpr->primaryExpr = createSimpleSubscriptExpr(
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
        else if (auto baseVaryingTuple = baseExpr.asVaryingTuple())
        {
            indexExpr = maybeMoveTemp(indexExpr);

            auto loweredExpr = new VaryingTupleExpr();
            loweredExpr->type.type = getSubscripResultType(baseVaryingTuple->type.type);

            SLANG_RELEASE_ASSERT(loweredExpr->type.type);

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
            return LoweredExpr(createSimpleSubscriptExpr(
                baseExpr.getExpr(),
                indexExpr));
        }
    }

    LoweredExpr visitIndexExpr(
        IndexExpr* subscriptExpr)
    {
        auto baseExpr = lowerExprOrTuple(subscriptExpr->BaseExpression);
        auto indexExpr = lowerExpr(subscriptExpr->IndexExpression);

        // An attempt to subscript a tuple must be turned into a
        // tuple of subscript expressions.
        if (auto baseTuple = baseExpr.asTuple())
        {
            return createSubscriptExpr(baseExpr, indexExpr);
        }
        else if (auto baseVaryingTuple = baseExpr.asVaryingTuple())
        {
            return createSubscriptExpr(baseExpr, indexExpr);
        }
        else
        {
            // Default case: just reconstrut a subscript expr
            RefPtr<IndexExpr> loweredExpr = new IndexExpr();
            lowerExprCommon(loweredExpr, subscriptExpr);
            loweredExpr->BaseExpression = baseExpr.getExpr();
            loweredExpr->IndexExpression = indexExpr;
            return LoweredExpr(loweredExpr);
        }
    }

    RefPtr<Expr> maybeReifyTuple(
        LoweredExpr expr)
    {
        if (auto tupleExpr = expr.asTuple())
        {
            // TODO: need to diagnose
            return tupleExpr->primaryExpr;
        }
        else if (auto varyingTupleExpr = expr.asVaryingTuple())
        {
            // Need to pass an ordinary (non-tuple) expression of
            // the corresponding type here.

            // TODO(tfoley): This won't work at all for an `out` or `inout`
            // function argument, so we'll need to figure out a plan
            // to handle that case...

            RefPtr<AggTypeCtorExpr> resultExpr = new AggTypeCtorExpr();
            resultExpr->type = varyingTupleExpr->type;
            resultExpr->base.type = varyingTupleExpr->type.type;
            SLANG_RELEASE_ASSERT(resultExpr->type.type);

            for (auto elem : varyingTupleExpr->elements)
            {
                addArgs(resultExpr, elem.expr);
            }

            return resultExpr;
        }

        // Default case: nothing special to this expression
        return expr.getExpr();
    }

    bool needGlslangBug988Workaround(
        RefPtr<Expr> inExpr)
    {
        switch (getTarget())
        {
        default:
            return false;

        case CodeGenTarget::GLSL:
            break;
        }

        // There are two conditions we care about here:
        //
        // (1) is the *type* of the expression something that needs the WAR
        // (2) does the expression reference a constant-buffer member?
        //

        // Issue (1): is the type of the expression something that needs the WAR?

        auto exprType = inExpr->type.type;
        exprType = unwrapArray(exprType);

        if (!isStructType(exprType))
            return false;


        // Issue (2): does the expression reference a constant-buffer member?

        auto expr = inExpr;
        for (;;)
        {
            if (auto memberRefExpr = expr.As<MemberExpr>())
            {
                expr = memberRefExpr->BaseExpression;
                continue;
            }

            if (auto derefExpr = expr.As<DerefExpr>())
            {
                expr = derefExpr->base;
                continue;
            }

            if (auto subscriptExpr = expr.As<IndexExpr>())
            {
                expr = subscriptExpr->BaseExpression;
                continue;
            }

            break;
        }

        if (auto varExpr = expr.As<VarExpr>())
        {
            auto declRef = varExpr->declRef;
            if (!declRef)
                return false;

            if (auto varDeclRef = declRef.As<Variable>())
            {
                auto varType = GetType(varDeclRef);

                while (auto arrayType = varType->As<ArrayExpressionType>())
                {
                    varType = arrayType->BaseType;
                }

                if (auto constantBufferType = varType->As<ConstantBufferType>())
                {
                    return true;
                }
            }
        }

        return false;
    }

    void addArg(
        ExprWithArgsBase*               callExpr,
        RefPtr<Expr>    argExpr)
    {
        // This should be the default case where we have a perfectly
        // ordinary expression, but we need to work around a glslang
        // but here, where passing a member of a `uniform` block
        // that has `struct` type directly to a function call causes
        // invalid SPIR-V to be generated.
        if (needGlslangBug988Workaround(argExpr))
        {
            argExpr = moveTemp(argExpr);
        }

        // Here's the actual default case where we just add an argment
        callExpr->Arguments.Add(argExpr);
    }

    void addArgs(
        ExprWithArgsBase*   callExpr,
        LoweredExpr         argExpr)
    {
        if (auto argTuple = argExpr.asTuple())
        {
            if (argTuple->primaryExpr)
            {
                addArg(callExpr, argTuple->primaryExpr);
            }
            for (auto elem : argTuple->tupleElements)
            {
                addArgs(callExpr, elem.expr);
            }
        }
        else if (auto varyingArgTuple = argExpr.asVaryingTuple())
        {
            // Need to pass an ordinary (non-tuple) expression of
            // the corresponding type here.

            callExpr->Arguments.Add(maybeReifyTuple(argExpr));
        }
        else
        {
            addArg(callExpr, argExpr.getExpr());
        }
    }

    RefPtr<Expr> lowerCallExpr(
        RefPtr<InvokeExpr>  loweredExpr,
        InvokeExpr*         expr)
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

    LoweredExpr visitInvokeExpr(
        InvokeExpr* expr)
    {
        return LoweredExpr(lowerCallExpr(new InvokeExpr(), expr));
    }

    LoweredExpr visitInfixExpr(
        InfixExpr* expr)
    {
        return LoweredExpr(lowerCallExpr(new InfixExpr(), expr));
    }

    LoweredExpr visitPrefixExpr(
        PrefixExpr* expr)
    {
        return LoweredExpr(lowerCallExpr(new PrefixExpr(), expr));
    }

    LoweredExpr visitSelectExpr(
        SelectExpr* expr)
    {
        // TODO: A tuple needs to be special-cased here

        return LoweredExpr(lowerCallExpr(new SelectExpr(), expr));
    }

    LoweredExpr visitPostfixExpr(
        PostfixExpr* expr)
    {
        return LoweredExpr(lowerCallExpr(new PostfixExpr(), expr));
    }

    LoweredExpr visitDerefExpr(
        DerefExpr*  expr)
    {
        auto loweredBase = lowerExprOrTuple(expr->base);

        if (auto baseTuple = loweredBase.asTuple())
        {
            // In the case of a tuple created for "resources in structs" reasons,
            // only the primary expression (if any) needs to be dereferenced.
            //
            // We cheat a bit here and re-use the same tuple we already have,
            // and just insert the deref into its primary.
            //
            // More or less we are lowering:
            //
            //    *(P, T0, T1, ...)
            //
            // into:
            //    (*P, T0, T1, ...)
            //
            if (auto primaryExpr = baseTuple->primaryExpr)
            {
                RefPtr<DerefExpr> loweredPrimary = new DerefExpr();
                lowerExprCommon(loweredPrimary, expr);
                loweredPrimary->base = baseTuple->primaryExpr;
                baseTuple->primaryExpr = loweredPrimary;
                return baseTuple;
            }
        }
        else if (auto baseVaryingTuple = loweredBase.asVaryingTuple())
        {
            // We don't expect to see this case arise for a "varying"
            // tuple, since there aren't pointer-like varyings, but
            // the desugaring seems natural: just dereference each
            // field.
            //
            // TODO: implement this.
        }

        // Default case is just to lower a dereference opertion
        // into another dereference.
        RefPtr<DerefExpr> loweredExpr = new DerefExpr();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->base = loweredBase.getExpr();
        return LoweredExpr(loweredExpr);
    }

    DiagnosticSink* getSink()
    {
        return &shared->compileRequest->mSink;
    }

    LoweredExpr visitMemberExpr(
        MemberExpr* expr)
    {
        auto loweredBase = lowerExprOrTuple(expr->BaseExpression);

        auto loweredDeclRef = translateDeclRef(expr->declRef);


        // Are we extracting an element from a tuple?
        if (auto baseTuple = loweredBase.asTuple())
        {
            auto loweredFieldDecl = loweredDeclRef.As<Decl>().getDecl();
            auto tupleFieldMod = loweredFieldDecl->FindModifier<TupleFieldModifier>();
            if (tupleFieldMod)
            {
                // This field has a tuple part to it, so we need to search for it

                LoweredExpr tupleFieldExpr;
                for (auto elem : baseTuple->tupleElements)
                {
                    if (loweredFieldDecl == elem.tupleFieldDeclRef.getDecl())
                    {
                        tupleFieldExpr = elem.expr;
                        break;
                    }
                }

                if (!tupleFieldMod->hasAnyNonTupleFields)
                    return tupleFieldExpr;

                auto tupleFieldTupleExpr = tupleFieldExpr.asTuple();
                SLANG_RELEASE_ASSERT(tupleFieldTupleExpr);
                SLANG_RELEASE_ASSERT(!tupleFieldTupleExpr->primaryExpr);


                RefPtr<MemberExpr> loweredPrimaryExpr = new MemberExpr();
                lowerExprCommon(loweredPrimaryExpr, expr);
                loweredPrimaryExpr->BaseExpression = baseTuple->primaryExpr;
                loweredPrimaryExpr->declRef = loweredDeclRef.As<Decl>();
                loweredPrimaryExpr->name = expr->name;

                tupleFieldTupleExpr->primaryExpr = loweredPrimaryExpr;
                return tupleFieldTupleExpr;
            }

            // If the field was a non-tuple field, then we can
            // simply fall through to the ordinary case below.
            loweredBase = LoweredExpr(baseTuple->primaryExpr);
        }
        else if (auto baseVaryingTuple = loweredBase.asVaryingTuple())
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

            SLANG_DIAGNOSE_UNEXPECTED(getSink(), expr, "failed to find tuple field during lowering");
        }

        // Default handling:

        RefPtr<MemberExpr> loweredExpr = new MemberExpr();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->BaseExpression = loweredBase.getExpr();
        loweredExpr->declRef = loweredDeclRef.As<Decl>();
        loweredExpr->name = expr->name;

        return LoweredExpr(loweredExpr);
    }

    //
    // Statements
    //

    // Lowering one statement to another.
    // The source statement might desugar into multiple statements,
    // (or event to none), and in such a case this function wraps
    // the result up as a `SeqStmt` or `EmptyStmt` as appropriate.
    //
    RefPtr<Stmt> lowerStmt(
        Stmt* stmt)
    {
        if (!stmt)
            return nullptr;

        LoweringVisitor subVisitor = *this;
        subVisitor.stmtBeingBuilt = nullptr;

        subVisitor.lowerStmtImpl(stmt);

        if (!subVisitor.stmtBeingBuilt)
        {
            return new EmptyStmt();
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
        Stmt*    loweredStmt = nullptr;
        Stmt*    originalStmt = nullptr;
    };
    StmtLoweringState stmtLoweringState;

    // Translate a reference from one statement to an outer statement
    Stmt* translateStmtRef(
        Stmt* originalStmt)
    {
        if (!originalStmt) return nullptr;

        for (auto state = &stmtLoweringState; state; state = state->parent)
        {
            if (state->originalStmt == originalStmt)
                return state->loweredStmt;
        }

        SLANG_DIAGNOSE_UNEXPECTED(getSink(), originalStmt, "failed to find outer statement during lowering");

        return nullptr;
    }

    // Expand a statement to be lowered into one or more statements
    void lowerStmtImpl(
        Stmt* stmt)
    {
        StmtVisitor::dispatch(stmt);
    }

    LoweredDecl visitScopeDecl(ScopeDecl* decl)
    {
        RefPtr<ScopeDecl> loweredDecl = new ScopeDecl();
        lowerDeclCommon(loweredDecl, decl);
        return LoweredDecl(loweredDecl);
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
        RefPtr<Stmt>&    dest,
        Stmt*            stmt)
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
        Stmt* stmt)
    {
        addStmtImpl(stmtBeingBuilt, stmt);
    }

    void addSimpleExprStmt(
        RefPtr<Expr>    expr)
    {
        if (auto infixExpr = expr.As<InfixExpr>())
        {
            if (auto varExpr = infixExpr->FunctionExpr.As<VarExpr>())
            {
                if (getText(varExpr->name) == ",")
                {
                    // Call to "operator comma"
                    for (auto aa : infixExpr->Arguments)
                    {
                        addSimpleExprStmt(aa);
                    }
                    return;
                }
            }
        }
        else if (auto varExpr = expr.As<VarExpr>())
        {
            // Skip an expression that is just a reference to a single variable
            return;
        }

        RefPtr<ExpressionStmt> stmt = new ExpressionStmt();
        stmt->Expression = expr;
        addStmt(stmt);
    }

    void addExprStmt(
        LoweredExpr     expr)
    {
        // Desugar tuples in statement position
        if (auto tupleExpr = expr.asTuple())
        {
            if (tupleExpr->primaryExpr)
            {
                addSimpleExprStmt(tupleExpr->primaryExpr);
            }
            for (auto ee : tupleExpr->tupleElements)
            {
                addExprStmt(ee.expr);
            }
            return;
        }
        else if (auto varyingTupleExpr = expr.asVaryingTuple())
        {
            for (auto ee : varyingTupleExpr->elements)
            {
                addExprStmt(ee.expr);
            }
            return;
        }
        else
        {
            addSimpleExprStmt(expr.getExpr());
        }
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

    void visitExpressionStmt(ExpressionStmt* stmt)
    {
        addExprStmt(lowerExprOrTuple(stmt->Expression));
    }

    void visitDeclStmt(DeclStmt* stmt)
    {
        DeclVisitor::dispatch(stmt->decl);
    }

    void lowerStmtFields(
        Stmt* loweredStmt,
        Stmt* originalStmt)
    {
        loweredStmt->loc = originalStmt->loc;
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

    void visitContinueStmt(ContinueStmt* stmt)
    {
        RefPtr<ContinueStmt> loweredStmt = new ContinueStmt();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitBreakStmt(BreakStmt* stmt)
    {
        RefPtr<BreakStmt> loweredStmt = new BreakStmt();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitDefaultStmt(DefaultStmt* stmt)
    {
        RefPtr<DefaultStmt> loweredStmt = new DefaultStmt();
        lowerChildStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitDiscardStmt(DiscardStmt* stmt)
    {
        RefPtr<DiscardStmt> loweredStmt = new DiscardStmt();
        lowerStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitEmptyStmt(EmptyStmt* stmt)
    {
        RefPtr<EmptyStmt> loweredStmt = new EmptyStmt();
        lowerStmtFields(loweredStmt, stmt);
        addStmt(loweredStmt);
    }

    void visitUnparsedStmt(UnparsedStmt* stmt)
    {
        RefPtr<UnparsedStmt> loweredStmt = new UnparsedStmt();
        lowerStmtFields(loweredStmt, stmt);

        for (auto token : stmt->tokens)
        {
            if (token.type == TokenType::Identifier)
            {
                doSampleRateInputCheck(token.getName());
            }
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

    void visitIfStmt(IfStmt* stmt)
    {
        RefPtr<IfStmt> loweredStmt = new IfStmt();
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
        RefPtr<ForStmt>  loweredStmt,
        ForStmt*         stmt)
    {
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->InitialStatement = subVisitor.lowerStmt(stmt->InitialStatement);
        loweredStmt->SideEffectExpression = subVisitor.lowerExpr(stmt->SideEffectExpression);
        loweredStmt->PredicateExpression = subVisitor.lowerExpr(stmt->PredicateExpression);
        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visitForStmt(ForStmt* stmt)
    {
        lowerForStmtCommon(new ForStmt(), stmt);
    }

    void visitUnscopedForStmt(UnscopedForStmt* stmt)
    {
        lowerForStmtCommon(new UnscopedForStmt(), stmt);
    }

    void visitCompileTimeForStmt(CompileTimeForStmt* stmt)
    {
        // We can either lower this here, so that emit logic doesn't have to deal with it,
        // or else just translate it and then let emit deal with it.
        //
        // The right answer is really to lower it here, I guess.

        auto rangeBeginVal = GetIntVal(stmt->rangeBeginVal);
        auto rangeEndVal = GetIntVal(stmt->rangeEndVal);

        if (rangeBeginVal >= rangeEndVal)
            return;

        auto varDecl = stmt->varDecl;

        auto varType = lowerType(varDecl->type);

        for (IntegerLiteralValue ii = rangeBeginVal; ii < rangeEndVal; ++ii)
        {
            RefPtr<ConstantExpr> constExpr = new ConstantExpr();
            constExpr->type.type = varType.type;
            constExpr->ConstType = ConstantExpr::ConstantType::Int;
            constExpr->integerValue = ii;

            RefPtr<VaryingTupleVarDecl> loweredVarDecl = new VaryingTupleVarDecl();
            loweredVarDecl->loc = varDecl->loc;
            loweredVarDecl->type = varType;
            loweredVarDecl->expr = LoweredExpr(constExpr);

            shared->loweredDecls[varDecl] = LoweredDecl(loweredVarDecl);

            lowerStmtImpl(stmt->body);
        }
    }

    void visitWhileStmt(WhileStmt* stmt)
    {
        RefPtr<WhileStmt> loweredStmt = new WhileStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Predicate = subVisitor.lowerExpr(stmt->Predicate);
        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visitDoWhileStmt(DoWhileStmt* stmt)
    {
        RefPtr<DoWhileStmt> loweredStmt = new DoWhileStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);
        loweredStmt->Predicate = subVisitor.lowerExpr(stmt->Predicate);

        addStmt(loweredStmt);
    }

    RefPtr<Stmt> transformSyntaxField(Stmt* stmt)
    {
        return lowerStmt(stmt);
    }

    void lowerStmtCommon(Stmt* loweredStmt, Stmt* stmt)
    {
        loweredStmt->modifiers = stmt->modifiers;
    }

    void assign(
        LoweredExpr destExpr,
        LoweredExpr srcExpr,
        AssignMode  mode = AssignMode::Default)
    {
        auto assignExpr = createAssignExpr(destExpr, srcExpr, mode);
        addExprStmt(assignExpr);
    }

    void assign(VarDeclBase* varDecl, LoweredExpr expr)
    {
        assign(LoweredExpr(createVarRef(getPosition(expr), varDecl)), expr);
    }

    void assign(LoweredExpr expr, VarDeclBase* varDecl)
    {
        assign(expr, LoweredExpr(createVarRef(getPosition(expr), varDecl)));
    }

    RefPtr<Expr> createCastExpr(
        RefPtr<Type>          type,
        RefPtr<Expr>    expr)
    {
        RefPtr<ExplicitCastExpr> castExpr = new ExplicitCastExpr();
        castExpr->loc = expr->loc;
        castExpr->type.type = type;
        castExpr->TargetType.type = type;
        castExpr->Expression = expr;
        return castExpr;
    }

    // Like `assign`, but with some extra logic to handle cases
    // where the types don't actually line up, because of
    // differences in how something is declared in HLSL vs. GLSL
    void assignWithFixups(
        LoweredExpr destExpr,
        LoweredExpr srcExpr)
    {
        assign(destExpr, srcExpr, AssignMode::WithFixups);
    }

    void assignWithFixups(VarDeclBase* varDecl, LoweredExpr expr)
    {
        assignWithFixups(LoweredExpr(createVarRef(getPosition(expr), varDecl)), expr);
    }

    void assignWithFixups(LoweredExpr expr, VarDeclBase* varDecl)
    {
        assignWithFixups(expr, LoweredExpr(createVarRef(getPosition(expr), varDecl)));
    }

    void visitReturnStmt(ReturnStmt* stmt)
    {
        auto loweredStmt = new ReturnStmt();
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
        if (auto type = dynamic_cast<Type*>(val))
            return lowerType(type);

        if (auto litVal = dynamic_cast<ConstantIntVal*>(val))
            return val;

        SLANG_UNEXPECTED("unhandled value kind");
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

    LoweredDeclRef translateDeclRef(
        DeclRef<Decl> const& decl)
    {
        LoweredDeclRef result;
        result.decl = translateDeclRef(decl.decl);
        result.substitutions = translateSubstitutions(decl.substitutions);
        return result;
    }

    LoweredDecl translateDeclRef(
        Decl* decl)
    {
        if (!decl) return LoweredDecl();

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

        LoweredDecl loweredDecl;
        if (shared->loweredDecls.TryGetValue(decl, loweredDecl))
            return loweredDecl;

        // Time to force it
        return lowerDecl(decl);
    }

    RefPtr<ContainerDecl> translateDeclRef(
        ContainerDecl* decl)
    {
        return translateDeclRef((Decl*)decl).getDecl()->As<ContainerDecl>();
    }

    LoweredDecl lowerDeclBase(
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

    LoweredDecl lowerDecl(
        Decl* decl)
    {
        LoweredDecl loweredDecl = DeclVisitor::dispatch(decl);
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
            RefPtr<DeclStmt> declStmt = new DeclStmt();
            declStmt->loc = decl->loc;
            declStmt->decl = decl;
            addStmt(declStmt);
        }


        // We will add the declaration to the current container declaration being
        // translated, which the user will maintain via pua/pop.
        //

        SLANG_RELEASE_ASSERT(parentDecl);
        addMember(parentDecl, decl);
    }

    void registerLoweredDecl(LoweredDecl loweredDecl, Decl* decl)
    {
        shared->loweredDecls.Add(decl, loweredDecl);

        shared->mapLoweredDeclToOriginal.Add(loweredDecl.getValue(), decl);
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

        auto name = decl->getName();
        if (isReservedWord(name))
        {
            auto nameText = getText(name);
            nameText.append("_");

            decl->nameAndLoc.name = getName(nameText);
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

        loweredDecl->loc = decl->loc;
        loweredDecl->nameAndLoc = decl->nameAndLoc;

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

    LoweredDecl visitSyntaxDecl(SyntaxDecl*)
    {
        return LoweredDecl();
    }

    LoweredDecl visitGenericValueParamDecl(GenericValueParamDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    LoweredDecl visitGenericTypeParamDecl(GenericTypeParamDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    LoweredDecl visitGenericTypeConstraintDecl(GenericTypeConstraintDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    LoweredDecl visitGenericDecl(GenericDecl*)
    {
        SLANG_UNEXPECTED("generics should be lowered to specialized decls");
    }

    LoweredDecl visitModuleDecl(ModuleDecl*)
    {
        SLANG_UNEXPECTED("module decls should be lowered explicitly");
    }

    LoweredDecl visitSubscriptDecl(SubscriptDecl*)
    {
        // We don't expect to find direct references to a subscript
        // declaration, but rather to the underlying accessors
        return LoweredDecl();
    }

    LoweredDecl visitInheritanceDecl(InheritanceDecl*)
    {
        // We should deal with these explicitly, as part of lowering
        // the type that contains them.
        return LoweredDecl();
    }

    LoweredDecl visitExtensionDecl(ExtensionDecl*)
    {
        // Extensions won't exist in the lowered code: their members
        // will turn into ordinary functions that get called explicitly
        return LoweredDecl();
    }

    LoweredDecl visitTypeDefDecl(TypeDefDecl* decl)
    {
        if (shared->target == CodeGenTarget::GLSL)
        {
            // GLSL does not support `typedef`, so we will lower it out of existence here
            return LoweredDecl();
        }

        RefPtr<TypeDefDecl> loweredDecl = new TypeDefDecl();
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->type = lowerType(decl->type);

        addMember(shared->loweredProgram, loweredDecl);
        return LoweredDecl(loweredDecl);
    }

    LoweredDecl visitImportDecl(ImportDecl*)
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
        return LoweredDecl();
    }

    LoweredDecl visitEmptyDecl(EmptyDecl* decl)
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

        return LoweredDecl(loweredDecl);
    }

    TupleTypeModifier* isTupleType(Type* type)
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

    Type* unwrapArray(Type* inType)
    {
        auto type = inType;
        while (auto arrayType = type->As<ArrayExpressionType>())
        {
            type = arrayType->BaseType;
        }
        return type;
    }

    TupleTypeModifier* isTupleTypeOrArrayOfTupleType(Type* type)
    {
        return isTupleType(unwrapArray(type));
    }

    bool isResourceType(Type* type)
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

    LoweredDecl visitAggTypeDecl(AggTypeDecl* decl)
    {
        // We want to lower any aggregate type declaration
        // to just a `struct` type that contains its fields.
        //
        // Any non-field members (e.g., methods) will be
        // lowered separately.

        RefPtr<StructDecl> loweredDecl = new StructDecl();
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
            auto loweredField = translateDeclRef(field).getDecl()->As<VarDeclBase>();

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
            auto loweredFieldType = loweredField->type.type;
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


        return LoweredDecl(loweredDecl);
    }

    RefPtr<VarDeclBase> lowerSimpleVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl,
        TypeExp const&      loweredType)
    {
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->type = loweredType;
        loweredDecl->initExpr = lowerExpr(decl->initExpr);

        return loweredDecl;
    }

    RefPtr<VarDeclBase> lowerSimpleVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl)
    {
        auto loweredType = lowerType(decl->type);
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

        // name "stem" to use for any actual variables we create
        String                      name;

        // The parent tuple type (or array thereof) we are scalarizing
        RefPtr<Type>  tupleType;

        // The actual declaration of the tuple type (which will give us the fields)
        DeclRef<AggTypeDecl>    tupleTypeDecl;

        // An initializer expression to use for the tuple members
        RefPtr<Expr>    initExpr;

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
            SLANG_RELEASE_ASSERT(!info.initExpr);
            RefPtr<Expr> fieldInitExpr;

            String fieldName = info.name + "_" + getText(dd.GetName());

            auto fieldType = GetType(dd);

            Decl* originalFieldDecl;
            shared->mapLoweredDeclToOriginal.TryGetValue(dd, originalFieldDecl);
            SLANG_RELEASE_ASSERT(originalFieldDecl);

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

            LoweredDecl fieldVarOrTupleDecl;
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

                fieldVarOrTupleDecl = LoweredDecl(fieldTupleDecl);
            }
            else
            {
                // Otherwise the field has a simple type, and we just need to declare the variable here

                RefPtr<Type> fieldVarType = fieldType;
                for (auto aa = info.arraySpecs; aa; aa = aa->next)
                {
                    RefPtr<ArrayExpressionType> arrayType = Slang::getArrayType(
                        fieldVarType,
                        aa->elementCount);

                    fieldVarType = arrayType;
                }

                RefPtr<VarDeclBase> fieldVarDecl = info.varDeclClass.createInstance();
                fieldVarDecl->nameAndLoc = NameLoc(getName(fieldName));
                fieldVarDecl->type.type = fieldVarType;

                addDecl(fieldVarDecl);

                if (fieldLayout)
                {
                    RefPtr<ComputedLayoutModifier> layoutMod = new ComputedLayoutModifier();
                    layoutMod->layout = fieldLayout;
                    addModifier(fieldVarDecl, layoutMod);
                }

                fieldVarOrTupleDecl = LoweredDecl(fieldVarDecl);
            }

            RefPtr<TupleVarModifier> fieldTupleVarMod = new TupleVarModifier();
            fieldTupleVarMod->tupleField = tupleFieldMod;

            TupleVarDecl::Element elem;
            elem.decl = fieldVarOrTupleDecl;
            elem.tupleVarMod = fieldTupleVarMod;

            info.tupleDecl->tupleDecls.Add(elem);
        }
    }

    LoweredDecl createTupleTypeVarDecls(
        SyntaxClass<VarDeclBase>        varDeclClass,
        RefPtr<VarDeclBase>             originalVarDecl,
        String const&                   name,
        RefPtr<Type>                    tupleType,
        DeclRef<AggTypeDecl>            tupleTypeDecl,
        TupleTypeModifier*              tupleTypeMod,
        RefPtr<Expr>                    initExpr,
        RefPtr<VarLayout>               primaryVarLayout,
        RefPtr<StructTypeLayout>        tupleTypeLayout)
    {
        // Not handling initializers just yet...
        SLANG_RELEASE_ASSERT(!initExpr);

        // We'll need a placeholder declaration to wrap the whole thing up:
        RefPtr<TupleVarDecl> tupleDecl = new TupleVarDecl();
        tupleDecl->nameAndLoc = NameLoc(getName(name));

        // First, if the tuple type had any "ordinary" data,
        // then we go ahead and create a declaration for that stuff
        if (tupleTypeMod->hasAnyNonTupleFields)
        {
            RefPtr<VarDeclBase> primaryVarDecl = varDeclClass.createInstance();
            primaryVarDecl->nameAndLoc.name = getName(name);
            primaryVarDecl->type.type = tupleType;

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

        return LoweredDecl(tupleDecl);
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

    LoweredDecl createTupleTypeVarDecls(
        SyntaxClass<VarDeclBase>        varDeclClass,
        RefPtr<VarDeclBase>             originalVarDecl,
        String const&                   name,
        RefPtr<Type>          tupleType,
        TupleTypeModifier*              tupleTypeMod,
        RefPtr<Expr>    initExpr,
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

    LoweredDecl lowerVarDeclCommonInner(
        VarDeclBase*                decl,
        SyntaxClass<VarDeclBase>    loweredDeclClass)
    {
        auto loweredType = lowerType(decl->type);

        if (auto tupleTypeMod = isTupleTypeOrArrayOfTupleType(loweredType))
        {
            auto varLayout = tryToFindLayout(decl).As<VarLayout>();

            // The type for the variable is a "tuple type"
            // so we need to go ahead and create multiple variables
            // to represent it.

            // If the variable had an initializer, we expect it
            // to resolve to a tuple *value*
            auto loweredInit = lowerExpr(decl->initExpr);

            // TODO: need to extract layout here and propagate it down!

            auto tupleDecl = createTupleTypeVarDecls(
                loweredDeclClass,
                decl,
                getText(decl->getName()),
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
                    getText(decl->getName()),
                    loweredType.type,
                    elementTupleTypeMod,
                    nullptr,
                    varLayout);

                shared->loweredDecls.Add(decl, tupleDecl);
                return tupleDecl;
            }
        }

        RefPtr<VarDeclBase> loweredDecl = loweredDeclClass.createInstance();

        // Note: we lower the declaration (including its initialization expression, if any)
        // *before* we add the declaration to the current context (e.g., a statement being
        // built), so that any operations inside the initialization expression that
        // might need to inject statements/temporaries/whatever happen *before*
        // the declaration of this variable.
        auto result = lowerSimpleVarDeclCommon(loweredDecl, decl, loweredType);
        addDecl(loweredDecl);

        return LoweredDecl(result);
    }

    LoweredDecl lowerVarDeclCommon(
        VarDeclBase*                decl,
        SyntaxClass<VarDeclBase>    loweredDeclClass)
    {
        // We need to add things to an appropriate scope, based on what
        // we are referencing.
        //
        // If this is a global variable (program scope), then add it
        // to the global scope.
        RefPtr<ContainerDecl> pp = decl->ParentDecl;
        if (auto parentModuleDecl = pp.As<ModuleDecl>())
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

    SourceLanguage getSourceLanguage(ModuleDecl* moduleDecl)
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

    void doSampleRateInputCheck(Name* name)
    {
        auto text = getText(name);
        if (text == "gl_SampleIndex")
        {
            setSampleRateFlag();
        }
    }

    AggTypeDecl* isStructType(RefPtr<Type> type)
    {
        if (type->As<BasicExpressionType>()) return nullptr;
        else if (type->As<VectorExpressionType>()) return nullptr;
        else if (type->As<MatrixExpressionType>()) return nullptr;
        else if (type->As<ResourceType>()) return nullptr;
        else if (type->As<BuiltinGenericType>()) return nullptr;
        else if (auto declRefType = type->As<DeclRefType>())
        {
            if (auto aggTypeDeclRef = declRefType->declRef.As<AggTypeDecl>())
            {
                return aggTypeDeclRef.getDecl();
            }
        }

        return nullptr;
    }

    bool isImportedStructType(RefPtr<Type> type)
    {
        // TODO: make this use `isStructType` above

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

    LoweredDecl visitVariable(
        Variable* decl)
    {
        // Global variable? Check if it is a sample-rate input.
        if (dynamic_cast<ModuleDecl*>(decl->ParentDecl))
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

                if( (inRes || outRes) && isImportedStructType(decl->type.type))
                {
                    // We are seemingly looking at a GLSL global-scope varying
                    // of an aggregate type which was imported from library
                    // code. We should destructure that into individual
                    // declarations.

                    // We can't easily support `in out` declarations with this approach
                    SLANG_RELEASE_ASSERT(!(inRes && outRes));

                    LoweredExpr loweredExpr;
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

//                    SLANG_RELEASE_ASSERT(loweredExpr);
                    auto loweredDecl = createVaryingTupleVarDecl(
                        decl,
                        loweredExpr);

                    registerLoweredDecl(LoweredDecl(loweredDecl), decl);
                    return LoweredDecl(loweredDecl);
                }
            }
        }

        auto loweredDecl = lowerVarDeclCommon(decl, getClass<Variable>());
        if(!loweredDecl.getValue())
            return LoweredDecl();

        return loweredDecl;
    }

    LoweredDecl visitStructField(
        StructField* decl)
    {
        return LoweredDecl(lowerSimpleVarDeclCommon(new StructField(), decl));
    }

    LoweredDecl visitParamDecl(
        ParamDecl* decl)
    {
        auto loweredDecl = lowerVarDeclCommon(decl, getClass<ParamDecl>());
        return loweredDecl;
    }

    LoweredDecl transformSyntaxField(DeclBase* decl)
    {
        return lowerDeclBase(decl);
    }


    LoweredDecl visitDeclGroup(
        DeclGroup* group)
    {
        for (auto decl : group->decls)
        {
            lowerDecl(decl);
        }
        return LoweredDecl();
    }

    LoweredDecl visitFunctionDeclBase(
        FunctionDeclBase*   decl)
    {
        // TODO: need to generate a name

        RefPtr<FuncDecl> loweredDecl = new FuncDecl();
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

        return LoweredDecl(loweredDecl);
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

    RefPtr<Expr> createGLSLBuiltinRef(
        char const*             name,
        RefPtr<Type>  type)
    {
        RefPtr<VarExpr> globalVarRef = new VarExpr();
        globalVarRef->name = getName(name);
        globalVarRef->type.type = type;
        return globalVarRef;
    }

    bool isIntegralType(
        Type* type)
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
        Slang::requireGLSLVersion(entryPoint, version);
    }

    RefPtr<Type> getFloatType()
    {
        return getSession()->getFloatType();
    }

    RefPtr<Type> getIntType()
    {
        return getSession()->getIntType();
    }

    RefPtr<Type> getUIntType()
    {
        return getSession()->getUIntType();
    }

    RefPtr<Type> getBoolType()
    {
        return getSession()->getBoolType();
    }

    RefPtr<VectorExpressionType> getVectorType(
        RefPtr<Type>  elementType,
        RefPtr<IntVal>          elementCount)
    {
        auto session = getSession();
        auto vectorGenericDecl = findMagicDecl(
            session,
            "Vector").As<GenericDecl>();
        auto vectorTypeDecl = vectorGenericDecl->inner;
               
        auto substitutions = new Substitutions();
        substitutions->genericDecl = vectorGenericDecl.Ptr();
        substitutions->args.Add(elementType);
        substitutions->args.Add(elementCount);

        auto declRef = DeclRef<Decl>(vectorTypeDecl.Ptr(), substitutions);

        return DeclRefType::Create(
            session,
            declRef)->As<VectorExpressionType>();
    }

    RefPtr<IntVal> getConstantIntVal(IntegerLiteralValue value)
    {
        RefPtr<ConstantIntVal> intVal = new ConstantIntVal();
        intVal->value = value;
        return intVal;
    }

    RefPtr<VectorExpressionType> getVectorType(
        RefPtr<Type>  elementType,
        int                     elementCount)
    {
        return getVectorType(elementType, getConstantIntVal(elementCount));
    }

    RefPtr<ArrayExpressionType> getUnsizedArrayType(
        RefPtr<Type>  elementType)
    {
        RefPtr<ArrayExpressionType> arrayType = Slang::getArrayType(elementType);
        return arrayType;
    }

    RefPtr<ArrayExpressionType> getArrayType(
        RefPtr<Type>  elementType,
        IntegerLiteralValue     elementCount)
    {
        return Slang::getArrayType(elementType, getConstantIntVal(elementCount));
    }

    LoweredExpr lowerSimpleShaderParameterToGLSLGlobal(
        VaryingParameterInfo const&     info,
        RefPtr<Type>          varType,
        RefPtr<VarLayout>               varLayout)
    {
        RefPtr<Type> type = varType;

        for (auto aa = info.arraySpecs; aa; aa = aa->next)
        {
            RefPtr<ArrayExpressionType> arrayType = Slang::getArrayType(
                type,
                aa->elementCount);

            type = arrayType;
        }

        assert(type);

        // We need to create a reference to the global-scope declaration
        // of the proper GLSL input/output variable. This might
        // be a user-defined input/output, or a system-defined `gl_` one.
        RefPtr<Expr> globalVarExpr;

        // Handle system-value inputs/outputs
        SLANG_RELEASE_ASSERT(varLayout);
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
                    globalVarExpr = createGLSLBuiltinRef("gl_FragCoord", getVectorType(getFloatType(), 4));
                }
                else
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_Position", getVectorType(getFloatType(), 4));
                }
            }
            else if (ns == "sv_clipdistance")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_ClipDistance", getUnsizedArrayType(getFloatType()));
            }
            else if (ns == "sv_culldistance")
            {
                requireGLSLExtension(shared->extensionUsageTracker, "ARB_cull_distance");
                globalVarExpr = createGLSLBuiltinRef("gl_CullDistance", getUnsizedArrayType(getFloatType()));
            }
            else if (ns == "sv_coverage")
            {
                if (info.direction == VaryingParameterDirection::Input)
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_SampleMaskIn", getUnsizedArrayType(getIntType()));
                }
                else
                {
                    globalVarExpr = createGLSLBuiltinRef("gl_SampleMask", getUnsizedArrayType(getIntType()));
                }
            }
            else if (ns == "sv_depth")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_FragDepth", getFloatType());
            }
            else if (ns == "sv_depthgreaterequal")
            {
                // TODO: layout(depth_greater) out float gl_FragDepth;
                globalVarExpr = createGLSLBuiltinRef("gl_FragDepth", getFloatType());
            }
            else if (ns == "sv_depthlessequal")
            {
                // TODO: layout(depth_less) out float gl_FragDepth;
                globalVarExpr = createGLSLBuiltinRef("gl_FragDepth", getFloatType());
            }
            else if (ns == "sv_dispatchthreadid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_GlobalInvocationID", getVectorType(getUIntType(), 3));
            }
            else if (ns == "sv_domainlocation")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_TessCoord", getVectorType(getFloatType(), 3));
            }
            else if (ns == "sv_groupid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_WorkGroupID", getVectorType(getUIntType(), 3));
            }
            else if (ns == "sv_groupindex")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_LocalInvocationIndex", getUIntType());
            }
            else if (ns == "sv_groupthreadid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_LocalInvocationID", getVectorType(getUIntType(), 3));
            }
            else if (ns == "sv_gsinstanceid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InvocationID", getIntType());
            }
            else if (ns == "sv_insidetessfactor")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_TessLevelInner", getArrayType(getFloatType(), 2));
            }
            else if (ns == "sv_instanceid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InstanceIndex", getIntType());
            }
            else if (ns == "sv_isfrontface")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_FrontFacing", getBoolType());
            }
            else if (ns == "sv_outputcontrolpointid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_InvocationID", getIntType());
            }
            else if (ns == "sv_primitiveid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_PrimitiveID", getIntType());
            }
            else if (ns == "sv_rendertargetarrayindex")
            {
                switch (shared->entryPointRequest->profile.GetStage())
                {
                case Stage::Geometry:
                    requireGLSLVersion(ProfileVersion::GLSL_150);
                    break;

                case Stage::Fragment:
                    requireGLSLVersion(ProfileVersion::GLSL_430);
                    break;

                default:
                    requireGLSLVersion(ProfileVersion::GLSL_450);
                    requireGLSLExtension(shared->extensionUsageTracker, "GL_ARB_shader_viewport_layer_array");
                    break;
                }

                globalVarExpr = createGLSLBuiltinRef("gl_Layer", getIntType());
            }
            else if (ns == "sv_sampleindex")
            {
                setSampleRateFlag();
                globalVarExpr = createGLSLBuiltinRef("gl_SampleID", getIntType());
            }
            else if (ns == "sv_stencilref")
            {
                requireGLSLExtension(shared->extensionUsageTracker, "ARB_shader_stencil_export");
                globalVarExpr = createGLSLBuiltinRef("gl_FragStencilRef", getIntType());
            }
            else if (ns == "sv_tessfactor")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_TessLevelOuter", getArrayType(getFloatType(), 4));
            }
            else if (ns == "sv_vertexid")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_VertexIndex", getIntType());
            }
            else if (ns == "sv_viewportarrayindex")
            {
                globalVarExpr = createGLSLBuiltinRef("gl_ViewportIndex", getIntType());
            }
            else if (ns == "nv_x_right")
            {
                requireGLSLVersion(ProfileVersion::GLSL_450);
                requireGLSLExtension(shared->extensionUsageTracker, "GL_NVX_multiview_per_view_attributes");

                // The actual output in GLSL is:
                //
                //    vec4 gl_PositionPerViewNV[];
                //
                // and is meant to support an arbitrary number of views,
                // while the HLSL case just defines a second position
                // output.
                //
                // For now we will hack this by:
                //   1. Mapping an `NV_X_Right` output to `gl_PositionPerViewNV[1]`
                //      (that is, just one element of the output array)
                //   2. Adding logic to copy the traditional `gl_Position` output
                //      over to `gl_PositionPerViewNV[0]`
                //

                globalVarExpr = createGLSLBuiltinRef("gl_PositionPerViewNV[1]",
                    getVectorType(getFloatType(), 4));

                shared->requiresCopyGLPositionToPositionPerView = true;
            }
            else if (ns == "nv_viewport_mask")
            {
                requireGLSLVersion(ProfileVersion::GLSL_450);
                requireGLSLExtension(shared->extensionUsageTracker, "GL_NVX_multiview_per_view_attributes");
                globalVarExpr = createGLSLBuiltinRef("gl_ViewportMaskPerViewNV",
                    getUnsizedArrayType(getIntType()));
            }
            else
            {
                getSink()->diagnose(info.varChain->varDecl, Diagnostics::unknownSystemValueSemantic, systemValueSemantic);
            }
        }

        // If we didn't match some kind of builtin input/output,
        // then declare a user input/output variable instead
        if (!globalVarExpr)
        {
            RefPtr<Variable> globalVarDecl = new Variable();
            globalVarDecl->nameAndLoc.name = getName(info.name);
            globalVarDecl->type.type = type;

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
                    && shared->entryPointRequest->profile.GetStage() != Stage::Fragment)
                {
                    // Don't add extra qualification to vertex shader inputs
                }
                else if (info.direction == VaryingParameterDirection::Output
                    && shared->entryPointRequest->profile.GetStage() == Stage::Fragment)
                {
                    // Don't add extra qualification to fragment shader outputs
                }
                else
                {
                    auto mod = new HLSLNoInterpolationModifier();
                    addModifier(globalVarDecl, mod);
                }
            }


            RefPtr<VarExpr> globalVarRef = new VarExpr();
            globalVarRef->loc = globalVarDecl->loc;
            globalVarRef->type.type = globalVarDecl->type.type;
            globalVarRef->declRef = makeDeclRef(globalVarDecl.Ptr());
            globalVarRef->name = globalVarDecl->getName();

            globalVarExpr = globalVarRef;
        }

        return LoweredExpr(globalVarExpr);
    }

    LoweredExpr lowerShaderParameterToGLSLGLobalsRec(
        VaryingParameterInfo const&     info,
        RefPtr<Type>          varType,
        RefPtr<VarLayout>               varLayout)
    {
        SLANG_RELEASE_ASSERT(varLayout);

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
                tupleExpr->type.type = varType;

                SLANG_RELEASE_ASSERT(tupleExpr->type.type);

                for (auto fieldDeclRef : getMembersOfType<VarDeclBase>(aggTypeDeclRef))
                {
                    // Don't emit storage for `static` fields here, of course
                    if (fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    VaryingParameterVarChain fieldVarChain;
                    fieldVarChain.next = info.varChain;
                    fieldVarChain.varDecl = fieldDeclRef.getDecl();

                    VaryingParameterInfo fieldInfo = info;
                    fieldInfo.name = info.name + "_" + getText(fieldDeclRef.GetName());
                    fieldInfo.varChain = &fieldVarChain;

                    // Need to find the layout for the given field...
                    Decl* originalFieldDecl = nullptr;
                    shared->mapLoweredDeclToOriginal.TryGetValue(fieldDeclRef.getDecl(), originalFieldDecl);
                    SLANG_RELEASE_ASSERT(originalFieldDecl);

                    auto structTypeLayout = varLayout->typeLayout.As<StructTypeLayout>();
                    SLANG_RELEASE_EXPECT(structTypeLayout, "expected a structure type layout");

                    RefPtr<VarLayout> fieldLayout;
                    structTypeLayout->mapVarToLayout.TryGetValue(originalFieldDecl, fieldLayout);
                    SLANG_RELEASE_ASSERT(fieldLayout);

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
                return LoweredExpr(tupleExpr);
            }
        }

        // Default case: just try to emit things as-is
        return lowerSimpleShaderParameterToGLSLGlobal(info, varType, varLayout);
    }

    LoweredExpr lowerShaderParameterToGLSLGLobals(
        RefPtr<VarDeclBase>         originalVarDecl,
        RefPtr<VarLayout>           paramLayout,
        VaryingParameterDirection   direction)
    {
        auto name = originalVarDecl->getName();
        auto nameText = getText(name);
        auto declRef = makeDeclRef(originalVarDecl.Ptr());

        VaryingParameterVarChain varChain;
        varChain.next = nullptr;
        varChain.varDecl = originalVarDecl;

        VaryingParameterInfo info;
        info.name = nameText;
        info.direction = direction;
        info.varChain = &varChain;

        // Ensure that we don't get name collisions on `inout` variables
        switch (direction)
        {
        case VaryingParameterDirection::Input:
            info.name = "SLANG_in_" + nameText;
            break;

        case VaryingParameterDirection::Output:
            info.name = "SLANG_out_" + nameText;
            break;
        }

        auto loweredType = lowerType(originalVarDecl->type);

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
        RefPtr<VarDeclBase> originalVarDecl,
        TypeExp const&      loweredType,
        LoweredExpr         loweredExpr)
    {
        RefPtr<VaryingTupleVarDecl> loweredDecl = new VaryingTupleVarDecl();
        loweredDecl->nameAndLoc = originalVarDecl->nameAndLoc;
        loweredDecl->type = loweredType;
        loweredDecl->expr = loweredExpr;

        return loweredDecl;
    }

    RefPtr<VaryingTupleVarDecl> createVaryingTupleVarDecl(
        RefPtr<VarDeclBase>     originalVarDecl,
        LoweredExpr             loweredExpr)
    {
        auto loweredType = lowerType(originalVarDecl->type);
        return createVaryingTupleVarDecl(originalVarDecl, loweredType, loweredExpr);
    }

    struct EntryPointParamPair
    {
        RefPtr<ParamDecl> original;
        RefPtr<VarLayout>           layout;
        RefPtr<Variable>            lowered;
    };

    RefPtr<FuncDecl> lowerEntryPointToGLSL(
        FuncDecl*         entryPointDecl,
        RefPtr<EntryPointLayout>    entryPointLayout)
    {
        // First, loer the entry-point function as an ordinary function:
        auto loweredEntryPointFunc = visitFunctionDeclBase(entryPointDecl).getDecl()->As<FunctionDeclBase>();

        auto mainName = getName("main");

        // Now we will generate a `void main() { ... }` function to call the lowered code.
        RefPtr<FuncDecl> mainDecl = new FuncDecl();
        mainDecl->ReturnType.type = getSession()->getVoidType();


        mainDecl->nameAndLoc = NameLoc(mainName);

        // If the user's entry point was called `main` then rename it here
        if (loweredEntryPointFunc->getName() == mainName)
            loweredEntryPointFunc->nameAndLoc = NameLoc(getName("main_"));

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
            SLANG_RELEASE_ASSERT(paramLayout);

            RefPtr<Variable> localVarDecl = new Variable();
            localVarDecl->loc = paramDecl->loc;
            localVarDecl->nameAndLoc = paramDecl->getNameAndLoc();
            localVarDecl->type = lowerType(paramDecl->type);

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

                subVisitor.assignWithFixups(paramPair.lowered, loweredExpr);
            }
        }

        // Generate a local variable for the result, if any
        RefPtr<Variable> resultVarDecl;
        if (!loweredEntryPointFunc->ReturnType->Equals(getSession()->getVoidType()))
        {
            resultVarDecl = new Variable();
            resultVarDecl->loc = loweredEntryPointFunc->loc;
            resultVarDecl->nameAndLoc = NameLoc(getName("main_result"));
            resultVarDecl->type = TypeExp(loweredEntryPointFunc->ReturnType);

            ensureDeclHasAValidName(resultVarDecl);

            subVisitor.addDecl(resultVarDecl);
        }

        // Now generate a call to the entry-point function, using the local variables
        auto entryPointDeclRef = makeDeclRef(loweredEntryPointFunc);

        auto entryPointType = getFuncType(
            getSession(),
            entryPointDeclRef);

        RefPtr<VarExpr> entryPointRef = new VarExpr();
        entryPointRef->name = loweredEntryPointFunc->getName();
        entryPointRef->declRef = entryPointDeclRef;
        entryPointRef->type = QualType(entryPointType);

        RefPtr<InvokeExpr> callExpr = new InvokeExpr();
        callExpr->FunctionExpr = entryPointRef;
        callExpr->type = QualType(loweredEntryPointFunc->ReturnType);

        //
        for (auto paramPair : params)
        {
            auto localVarDecl = paramPair.lowered;

            RefPtr<VarExpr> varRef = new VarExpr();
            varRef->name = localVarDecl->getName();
            varRef->declRef = makeDeclRef(localVarDecl.Ptr());
            varRef->type = QualType(localVarDecl->getType());

            callExpr->Arguments.Add(varRef);
        }

        if (resultVarDecl)
        {
            // Non-`void` return type, so we need to store it
            subVisitor.assign(resultVarDecl, LoweredExpr(callExpr));
        }
        else
        {
            // `void` return type: just call it
            subVisitor.addExprStmt(LoweredExpr(callExpr));
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

                subVisitor.assignWithFixups(loweredExpr, paramPair.lowered);
            }
        }
        if (resultVarDecl)
        {
            VaryingParameterInfo info;
            info.name = "SLANG_out_" + getText(resultVarDecl->getName());
            info.direction = VaryingParameterDirection::Output;
            info.varChain = nullptr;

            auto loweredExpr = lowerShaderParameterToGLSLGLobalsRec(
                info,
                resultVarDecl->type.type,
                entryPointLayout->resultLayout);

            subVisitor.assignWithFixups(loweredExpr, resultVarDecl);
        }
        if (shared->requiresCopyGLPositionToPositionPerView)
        {
            subVisitor.assign(
                LoweredExpr(createSimpleVarExpr("gl_PositionPerViewNV[0]")),
                LoweredExpr(createSimpleVarExpr("gl_Position")));
        }

        bodyStmt->body = subVisitor.stmtBeingBuilt;

        mainDecl->Body = bodyStmt;


        // Once we are done building the body, we append our new declaration to the program.
        addMember(shared->loweredProgram, mainDecl);
        return mainDecl;

#if 0
        RefPtr<FuncDecl> loweredDecl = new FuncDecl();
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
        if (!loweredReturnType->Equals(getSession()->getVoidType()))
        {
            resultGlobal = new Variable();
            // TODO: need a scheme for generating unique names
            resultGlobal->name.Content = "_main_result";
            resultGlobal->type = loweredReturnType;

            addMember(shared->loweredProgram, resultGlobal);
        }

        loweredDecl->name.Content = "main";
        loweredDecl->ReturnType.type = getSession()->getVoidType();

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

    RefPtr<FuncDecl> lowerEntryPoint(
        FuncDecl*         entryPointDecl,
        RefPtr<EntryPointLayout>    entryPointLayout)
    {
        switch( getTarget() )
        {
        // Default case: lower an entry point just like any other function
        default:
            return visitFunctionDeclBase(entryPointDecl).getDecl()->As<FuncDecl>();

        // For Slang->GLSL translation, we need to lower things from HLSL-style
        // declarations over to GLSL conventions
        case CodeGenTarget::GLSL:
            return lowerEntryPointToGLSL(entryPointDecl, entryPointLayout);
        }
    }

    RefPtr<FuncDecl> lowerEntryPoint(
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
        SLANG_RELEASE_ASSERT(elementTypeStructLayout);

        globalStructLayout = elementTypeStructLayout.Ptr();
    }
    else
    {
        SLANG_UNEXPECTED("unhandled type for global-scope parameter layout");
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
    EntryPointRequest*      entryPoint,
    ProgramLayout*          programLayout,
    CodeGenTarget           target,
    ExtensionUsageTracker*  extensionUsageTracker)
{
    SharedLoweringContext sharedContext;
    sharedContext.compileRequest = entryPoint->compileRequest;
    sharedContext.entryPointRequest = entryPoint;
    sharedContext.programLayout = programLayout;
    sharedContext.target = target;
    sharedContext.extensionUsageTracker = extensionUsageTracker;

    auto translationUnit = entryPoint->getTranslationUnit();

    // Create a single module/program to hold all the lowered code
    // (with the exception of instrinsic/stdlib declarations, which
    // will be remain where they are)
    RefPtr<ModuleDecl> loweredProgram = new ModuleDecl();
    sharedContext.loweredProgram = loweredProgram;

    LoweringVisitor visitor;
    visitor.shared = &sharedContext;
    visitor.parentDecl = loweredProgram;

    // TODO: this should only need to take the shared context
    visitor.registerReservedWords();

    // We need to register the lowered program as the lowered version
    // of the existing translation unit declaration.
    
    visitor.registerLoweredDecl(
        LoweredDecl(loweredProgram),
        translationUnit->SyntaxNode);

    // We also need to register the lowered program as the lowered version
    // of any imported modules (since we will be collecting everything into
    // a single module for code generation).
    for (auto rr : entryPoint->compileRequest->loadedModulesList)
    {
        sharedContext.loweredDecls.Add(
            rr,
            LoweredDecl(loweredProgram));
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
