// ast-legalize.cpp
#include "ast-legalize.h"

#include "emit.h"
#include "ir-insts.h"
#include "legalize-types.h"
#include "mangle.h"
#include "type-layout.h"
#include "visitor.h"

// DEBUGGING
#if 0
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#endif
#endif



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

// Forward-declare types used by `LegalExpr`
class ImplicitDerefPseudoExpr;
class TuplePseudoExpr;
class PairPseudoExpr;

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
        DeclRef<Decl> declRef = visitor->translateDeclRef(decl);
        return declRef.As<T>();
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
        RefPtr<Decl> transformed = visitor->transformSyntaxField(decl);
        return transformed.As<ScopeDecl>();
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
struct StructuralTransformExprVisitor
    : StructuralTransformVisitorBase<V>
    , ExprVisitor<StructuralTransformExprVisitor<V>, RefPtr<Expr>>
{
    void transformFields(Expr* result, Expr* obj)
    {
        result->type = this->transformSyntaxField(obj->type);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME, BASE, ...)              \
    void transformFields(NAME* result, NAME* obj) {         \
        this->transformFields((BASE*) result, (BASE*) obj); \
    /* end */


#define SYNTAX_CLASS(NAME, BASE, ...)                   \
    RefPtr<Expr> visit##NAME(NAME* obj) {               \
        RefPtr<NAME> result = new NAME(*obj);           \
        transformFields(result, obj);                   \
        return result;                                  \
    }                                                   \
    ABSTRACT_SYNTAX_CLASS(NAME, BASE)                   \
    /* end */

#define SYNTAX_FIELD(TYPE, NAME)    result->NAME = this->transformSyntaxField(obj->NAME);
#define DECL_FIELD(TYPE, NAME)      result->NAME = this->transformDeclField(obj->NAME);

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



// The result of legalizing an exrpession will usually be just a single
// expression, but it might also be a "tuple" expression that encodes
// multiple expressions.
struct LegalExpr
{
    typedef LegalType::Flavor Flavor;

    LegalExpr()
        : flavor(Flavor::none)
    {}

    LegalExpr(Expr* expr)
        : value(expr)
        , flavor(Flavor::simple)
    {}

    LegalExpr(TuplePseudoExpr* expr)
        : value((RefObject*) expr)
        , flavor(Flavor::tuple)
    {}

    LegalExpr(PairPseudoExpr* expr)
        : value((RefObject*) expr)
        , flavor(Flavor::pair)
    {}

    LegalExpr(ImplicitDerefPseudoExpr* expr)
        : value((RefObject*) expr)
        , flavor(Flavor::implicitDeref)
    {}

    Flavor getFlavor() const { return flavor; }

    Expr* getSimple() const
    {
        switch (getFlavor())
        {
        case Flavor::none:
            return nullptr;

        case Flavor::simple:
            return (Expr*)value.Ptr();

        default:
            assert(getFlavor() == Flavor::simple);
            return nullptr;
        }
    }

    TuplePseudoExpr* getTuple() const
    {
        assert(getFlavor() == Flavor::tuple);
        return (TuplePseudoExpr*)value.Ptr();
    }

    PairPseudoExpr* getPair() const
    {
        assert(getFlavor() == Flavor::pair);
        return (PairPseudoExpr*)value.Ptr();
    }

    ImplicitDerefPseudoExpr* getImplicitDeref() const
    {
        assert(getFlavor() == Flavor::implicitDeref);
        return (ImplicitDerefPseudoExpr*)value.Ptr();
    }

    // Allow use in boolean contexts
    operator void*()
    {
        return value.Ptr();
    }

private:
    RefPtr<RefObject>   value;
    Flavor              flavor;
};

struct LegalTypeExpr
{
    LegalType       type;
    RefPtr<Expr>    expr;

    LegalTypeExpr()
    {}

    LegalTypeExpr(LegalType const& type)
        : type(type)
    {
    }

    LegalTypeExpr(TypeExp const& typeExpr)
    {
        type = LegalType::simple(typeExpr.type);
        expr = typeExpr.exp;
    }

    TypeExp getSimple() const
    {
        TypeExp result;
        result.type = type.getSimple();
        result.exp = expr;
        return result;
    }
};

class PseudoExpr : public RefObject
{
public:
    SourceLoc   loc;
};

class ImplicitDerefPseudoExpr : public PseudoExpr
{
public:
    LegalExpr valueExpr;
};

class TuplePseudoExpr : public PseudoExpr
{
public:
    struct Element
    {
        LegalExpr               expr;
        DeclRef<VarDeclBase>    fieldDeclRef;
    };

    List<Element>               elements;
};

class PairPseudoExpr : public PseudoExpr
{
public:
    LegalExpr ordinary;
    LegalExpr special;

    RefPtr<PairInfo>  pairInfo;
};

static SourceLoc getPosition(LegalExpr const& expr)
{
    switch (expr.getFlavor())
    {
    case LegalExpr::Flavor::none:           return SourceLoc();
    case LegalExpr::Flavor::simple:         return expr.getSimple()         ->loc;
    case LegalExpr::Flavor::tuple:          return expr.getTuple()          ->loc;
    case LegalExpr::Flavor::pair:           return expr.getPair()           ->loc;
    case LegalExpr::Flavor::implicitDeref:  return expr.getImplicitDeref()  ->loc;

    default:
        SLANG_UNREACHABLE("all cases handled");
        UNREACHABLE_RETURN(SourceLoc());
    }
}


struct SharedLoweringContext
{
    CompileRequest*     compileRequest;
    EntryPointRequest*  entryPointRequest;

    // The "main" module that is being translated (as opposed
    // to any of the modules that might have been imported).
    ModuleDecl* mainModuleDecl;

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

    Dictionary<Decl*, RefPtr<Decl>> mapOriginalDeclToLowered;
    Dictionary<Decl*, LegalExpr>    mapOriginalDeclToExpr;
    Dictionary<RefObject*, Decl*> mapLoweredDeclToOriginal;

    // Work to be done at the very start and end of the entry point
    RefPtr<Stmt> entryPointInitializeStmt;
    RefPtr<Stmt> entryPointFinalizeStmt;

    // Counter used for generating unique temporary names
    int nameCounter = 0;

    bool isRewrite = false;
    bool requiresCopyGLPositionToPositionPerView = false;

    // State for lowering imported declarations to IR as needed
    IRSpecializationState* irSpecializationState = nullptr;

    // The actual result we want to return
    LoweredEntryPoint result;

    /// State to use when legalizing types.
    TypeLegalizationContext* typeLegalizationContext;
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
    : ExprVisitor<LoweringVisitor, LegalExpr>
    , StmtVisitor<LoweringVisitor, void>
    , DeclVisitor<LoweringVisitor, RefPtr<Decl>>
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

    TypeLegalizationContext* getTypeLegalizationContext()
    {
        return shared->typeLegalizationContext;
    }

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

    RefPtr<Witness> visitWitness(Witness* witness)
    {
        return witness;
    }

    //
    // Types
    //

    RefPtr<Type> lowerTypeEx(
        Type* type)
    {
        if (!type) return nullptr;
        RefPtr<Type> loweredType = dispatchType(type);
        return loweredType;
    }

    LegalType lowerLegalType(
        LegalType legalType)
    {
        switch(legalType.flavor)
        {
        case LegalType::Flavor::none:
            return LegalType();

        case LegalType::Flavor::simple:
            return LegalType::simple(
                lowerTypeEx(legalType.getSimple()));

        case LegalType::Flavor::tuple:
            {
                auto inputTuple = legalType.getTuple();
                RefPtr<TuplePseudoType>  resultTuple = new TuplePseudoType();
                for(auto ee : inputTuple->elements)
                {
                    TuplePseudoType::Element element;
                    element.fieldDeclRef = ee.fieldDeclRef;
                    element.type = lowerLegalType(ee.type);
                    resultTuple->elements.Add(element);
                }
                return LegalType::tuple(resultTuple);
            }
            break;

        case LegalType::Flavor::pair:
            {
                auto inputPair = legalType.getPair();
                RefPtr<PairPseudoType> resultPair = new PairPseudoType();
                return LegalType::pair(
                    lowerLegalType(inputPair->ordinaryType),
                    lowerLegalType(inputPair->specialType),
                    inputPair->pairInfo);
            }
            break;

        case LegalType::Flavor::implicitDeref:
            {
                return LegalType::implicitDeref(
                    lowerLegalType(legalType.getImplicitDeref()->valueType));
            }
            break;

        default:
            SLANG_UNEXPECTED("uhandled type flavor");
            UNREACHABLE_RETURN(LegalType());
        }
    }

    LegalType lowerAndLegalizeType(
        Type* type)
    {
        if (!type) return LegalType();

        // We will first attempt to legalize the type, so that any parts of
        // it that won't be allowed on the target get excised. Once we are
        // done with that, we will do the "lowering" process of copying
        // any needed bits of AST over to the new module.
        LegalType legalType = legalizeType(
            getTypeLegalizationContext(),
            type);

        LegalType loweredType = lowerLegalType(legalType);

        return loweredType;
    }

    TypeExp lowerTypeExprEx(
        TypeExp const& typeExp)
    {
        TypeExp result;
        result.type = lowerTypeEx(typeExp.type);
        result.exp = legalizeSimpleExpr(typeExp.exp);
        return result;
    }

    LegalTypeExpr lowerAndLegalizeTypeExpr(
        TypeExp const& typeExp)
    {
        LegalTypeExpr result;
        result.type = lowerAndLegalizeType(typeExp.type);
        result.expr = legalizeSimpleExpr(typeExp.exp);
        return result;
    }

    RefPtr<Type> lowerAndLegalizeSimpleType(
        Type* type)
    {
        return lowerAndLegalizeType(type).getSimple();
    }

    TypeExp lowerAndlegalizeSimpleTypeExpr(
        TypeExp const& typeExp)
    {
        return lowerAndLegalizeTypeExpr(typeExp).getSimple();
    }

    RefPtr<Type> visitIRBasicBlockType(IRBasicBlockType* type)
    {
        return type;
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
        RefPtr<FuncType> loweredType = new FuncType();
        loweredType->setSession(getSession());
        loweredType->resultType = lowerTypeEx(type->resultType);
        for (auto paramType : type->paramTypes)
        {
            auto loweredParamType = lowerTypeEx(paramType);
            loweredType->paramTypes.Add(loweredParamType);
        }
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
            return lowerTypeEx(GetType(type->declRef));
        }

        return getNamedType(
            type->getSession(),
            translateDeclRef(DeclRef<Decl>(type->declRef)).As<TypeDefDecl>());
    }

    RefPtr<Type> visitTypeType(TypeType* type)
    {
        return getTypeType(lowerTypeEx(type->type));
    }

    RefPtr<Type> visitArrayExpressionType(ArrayExpressionType* type)
    {
        RefPtr<ArrayExpressionType> loweredType = Slang::getArrayType(
            lowerTypeEx(type->baseType),
            lowerVal(type->ArrayLength).As<IntVal>());
        return loweredType;
    }

    RefPtr<Type> visitGroupSharedType(GroupSharedType* type)
    {
        return getSession()->getGroupSharedType(
            lowerTypeEx(type->valueType));
    }

    RefPtr<Type> transformSyntaxField(Type* type)
    {
        return lowerAndLegalizeSimpleType(type);
    }

    RefPtr<Val> visitIRProxyVal(IRProxyVal* val)
    {
        return val;
    }

    //
    // Expressions
    //

    LegalExpr legalizeExpr(
        Expr* expr)
    {
        if (!expr) return LegalExpr();
        return ExprVisitor::dispatch(expr);
    }

    RefPtr<Expr> legalizeSimpleExpr(
        Expr* expr)
    {
        if (!expr) return nullptr;

        auto type = lowerAndLegalizeType(expr->type.type);
        auto result = legalizeExpr(expr);
        return maybeReifyTuple(result, type).getSimple();
    }

    // catch-all
    LegalExpr visitExpr(
        Expr* expr)
    {
        return LegalExpr(structuralTransform(expr, this));
    }

    RefPtr<Expr> transformSyntaxField(Expr* expr)
    {
        return legalizeSimpleExpr(expr);
    }

    void lowerExprCommon(
        Expr*    loweredExpr,
        Expr*    expr)
    {
        loweredExpr->loc = expr->loc;
        loweredExpr->type.type = lowerTypeEx(expr->type.type);
    }

    void lowerExprCommon(
        LegalExpr const&    legalExpr,
        Expr*               expr)
    {
        if (legalExpr.getFlavor() == LegalExpr::Flavor::simple)
        {
            lowerExprCommon(legalExpr.getSimple(), expr);
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
        String const& name)
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

    LegalExpr createVarRef(
        SourceLoc const&    loc,
        VarDeclBase*        decl)
    {
        return LegalExpr(createSimpleVarRef(loc, decl));
    }

    RefPtr<Expr> createSimpleVarExpr(
        VarExpr*                expr,
        DeclRef<Decl> const&    declRef)
    {
        RefPtr<VarExpr> loweredExpr = new VarExpr();
        if (expr)
        {
            lowerExprCommon(loweredExpr, expr);
        }
        loweredExpr->declRef = declRef;
        loweredExpr->name = expr->name;
        return loweredExpr;
    }

    LegalExpr visitVarExpr(
        VarExpr* expr)
    {
        // If the expression didn't get resolved, we can leave it as-is
        if (!expr->declRef)
            return expr;

        // Ensure that lowering has been applied to the declaration
        auto loweredDeclRef = translateDeclRef(expr->declRef);

        // Is there a value already registered for use when looking
        // up this variable?
        LegalExpr legalExpr;
        if (this->shared->mapOriginalDeclToExpr.TryGetValue(expr->declRef.getDecl(), legalExpr))
            return legalExpr;

        return LegalExpr(createSimpleVarExpr(
            expr,
            loweredDeclRef));
    }

    LegalExpr visitOverloadedExpr(
        OverloadedExpr* expr)
    {
        // The presence of an overloaded expression in the output
        // means that some amount of semantic checking failed.
        // Thus we don't need to worry about semantically transforming
        // the expression itself, but we *do* want to ensure that any
        // of the declarations that the user might have been referring
        // to get lowered so they will appear in the output.
        for (auto item : expr->lookupResult2.items)
        {
            translateDeclRef(item.declRef);
        }

        return expr;
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

    LegalExpr maybeMoveTemp(
        LegalExpr expr)
    {
        switch (expr.getFlavor())
        {
        case LegalExpr::Flavor::none:
            return LegalExpr();

        case LegalExpr::Flavor::simple:
            return LegalExpr(maybeMoveTemp(expr.getSimple()));

        case LegalExpr::Flavor::tuple:
            {
                auto tupleExpr = expr.getTuple();
                RefPtr<TuplePseudoExpr> resultExpr = new TuplePseudoExpr();
                resultExpr->loc = tupleExpr->loc;

                for (auto ee : tupleExpr->elements)
                {
                    TuplePseudoExpr::Element element;
                    element.expr = maybeMoveTemp(ee.expr);
                    element.fieldDeclRef = ee.fieldDeclRef;
                    resultExpr->elements.Add(element);
                }

                return LegalExpr(resultExpr);
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto pairExpr = expr.getPair();
                RefPtr<PairPseudoExpr> resultExpr = new PairPseudoExpr();
                resultExpr->loc = pairExpr->loc;
                resultExpr->pairInfo = pairExpr->pairInfo;

                resultExpr->ordinary = maybeMoveTemp(pairExpr->ordinary);
                resultExpr->special = maybeMoveTemp(pairExpr->special);

                return LegalExpr(resultExpr);
            }
            break;

        case LegalExpr::Flavor::implicitDeref:
            {
                auto implicitDerefExpr = expr.getImplicitDeref();
                RefPtr<ImplicitDerefPseudoExpr> resultExpr = new ImplicitDerefPseudoExpr();
                resultExpr->loc = implicitDerefExpr->loc;

                resultExpr->valueExpr = maybeMoveTemp(implicitDerefExpr->valueExpr);

                return LegalExpr(resultExpr);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled case");
            UNREACHABLE_RETURN(LegalExpr());
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

    LegalExpr ensureSimpleLValue(
        LegalExpr expr)
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
                if (auto leftElemVecType = leftArrayType->baseType->As<VectorExpressionType>())
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
                            leftArrayType->baseType,
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
                if (auto leftElemVecType = leftArrayType->baseType->As<VectorExpressionType>())
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
                        arrayElemExpr->type.type = leftArrayType->baseType;
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

    LegalExpr createAssignExpr(
        LegalExpr   leftExpr,
        LegalExpr   rightExpr,
        AssignMode    mode = AssignMode::Default)
    {
        switch (leftExpr.getFlavor())
        {
        case LegalExpr::Flavor::none:
            return LegalExpr();

        case LegalExpr::Flavor::simple:
            switch (rightExpr.getFlavor())
            {
            case LegalExpr::Flavor::simple:
                return LegalExpr(createSimpleAssignExpr(
                    leftExpr.getSimple(),
                    rightExpr.getSimple(),
                    mode));

            case LegalExpr::Flavor::tuple:
                {
                    auto rightTuple = rightExpr.getTuple();
                    RefPtr<TuplePseudoExpr> resultTuple = new TuplePseudoExpr();
                    for (auto ee : rightTuple->elements)
                    {
                        TuplePseudoExpr::Element element;
                        element.fieldDeclRef = ee.fieldDeclRef;
                        element.expr = createAssignExpr(
                            extractField(leftExpr, ee.fieldDeclRef),
                            ee.expr,
                            mode);

                        resultTuple->elements.Add(element);
                    }
                    return LegalExpr(resultTuple);
                }
                break;

            default:
                SLANG_UNEXPECTED("unimplemented");
                UNREACHABLE_RETURN(LegalExpr());
            }
            break;

        case LegalExpr::Flavor::tuple:
            {
                rightExpr = maybeMoveTemp(rightExpr);

                auto leftTuple = leftExpr.getTuple();
                RefPtr<TuplePseudoExpr> resultTuple = new TuplePseudoExpr();
                resultTuple->loc = leftTuple->loc;
                for (auto ee : leftTuple->elements)
                {
                    TuplePseudoExpr::Element element;
                    element.fieldDeclRef = ee.fieldDeclRef;
                    element.expr = createAssignExpr(
                        ee.expr,
                        extractField(rightExpr, ee.fieldDeclRef),
                        mode);

                    resultTuple->elements.Add(element);
                }
                return LegalExpr(resultTuple);
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto leftPair = leftExpr.getPair();
                switch( rightExpr.getFlavor() )
                {
                case LegalExpr::Flavor::pair:
                    {
                        auto rightPair = rightExpr.getPair();
                        RefPtr<PairPseudoExpr> resultPair = new PairPseudoExpr();
                        resultPair->loc = leftPair->loc;
                        resultPair->pairInfo = leftPair->pairInfo;

                        resultPair->ordinary = createAssignExpr(
                            leftPair->ordinary,
                            rightPair->ordinary,
                            mode);
                        resultPair->special = createAssignExpr(
                            leftPair->special,
                            rightPair->special,
                            mode);

                        return LegalExpr(resultPair);
                    }
                    break;

                default:
                    SLANG_UNEXPECTED("unimplemented");
                    UNREACHABLE_RETURN(LegalExpr());
                }
            }
            break;

        case LegalExpr::Flavor::implicitDeref:
            {
                auto leftImplicitDeref = leftExpr.getImplicitDeref();
                switch(rightExpr.getFlavor())
                {
                case LegalExpr::Flavor::implicitDeref:
                    {
                        auto rightImplicitDeref = rightExpr.getImplicitDeref();
                        RefPtr<ImplicitDerefPseudoExpr> resultImplicitDeref = new ImplicitDerefPseudoExpr();
                        resultImplicitDeref->loc = leftImplicitDeref->loc;
                        resultImplicitDeref->valueExpr = createAssignExpr(
                            leftImplicitDeref->valueExpr,
                            rightImplicitDeref->valueExpr,
                            mode);

                        return LegalExpr(resultImplicitDeref);
                    }

                default:
                    SLANG_UNEXPECTED("unimplemented");
                    UNREACHABLE_RETURN(LegalExpr());
                }
            }

        default:
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(LegalExpr());
        }
    }

    LegalExpr visitAssignExpr(
        AssignExpr* expr)
    {
        auto leftExpr = legalizeExpr(expr->left);
        auto rightExpr = legalizeExpr(expr->right);

        auto loweredExpr = createAssignExpr(leftExpr, rightExpr);
        lowerExprCommon(loweredExpr, expr);
        return loweredExpr;
    }

    RefPtr<Type> getSubscripResultType(
        RefPtr<Type>  type)
    {
        if (auto arrayType = type->As<ArrayExpressionType>())
        {
            return arrayType->baseType;
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

    LegalExpr createSubscriptExpr(
        LegalExpr       baseExpr,
        RefPtr<Expr>    indexExpr)
    {
        switch (baseExpr.getFlavor())
        {
        case LegalExpr::Flavor::none:
            return LegalExpr();

        case LegalExpr::Flavor::simple:
            return LegalExpr(createSimpleSubscriptExpr(
                baseExpr.getSimple(),
                indexExpr));

        case LegalExpr::Flavor::tuple:
            {
                indexExpr = maybeMoveTemp(indexExpr);

                auto baseTuple = baseExpr.getTuple();

                auto resultTuple = new TuplePseudoExpr();
                resultTuple->loc = baseTuple->loc;

                for (auto ee : baseTuple->elements)
                {
                    TuplePseudoExpr::Element element;
                    element.fieldDeclRef = ee.fieldDeclRef;
                    element.expr = createSubscriptExpr(
                        ee.expr,
                        indexExpr);

                    resultTuple->elements.Add(element);
                }

                return LegalExpr(resultTuple);
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                indexExpr = maybeMoveTemp(indexExpr);

                auto basePair = baseExpr.getPair();

                RefPtr<PairPseudoExpr> resultPair = new PairPseudoExpr();
                resultPair->pairInfo = basePair->pairInfo;
                resultPair->loc = basePair->loc;

                resultPair->ordinary = createSubscriptExpr(basePair->ordinary, indexExpr);
                resultPair->special = createSubscriptExpr(basePair->special, indexExpr);

                return LegalExpr(resultPair);
            }

        case LegalExpr::Flavor::implicitDeref:
            {
                auto baseImplicitDeref = baseExpr.getImplicitDeref();

                RefPtr<ImplicitDerefPseudoExpr> resultImplicitDeref = new ImplicitDerefPseudoExpr();
                resultImplicitDeref->loc = baseImplicitDeref->loc;

                resultImplicitDeref->valueExpr = createSubscriptExpr(baseImplicitDeref->valueExpr, indexExpr);

                return LegalExpr(resultImplicitDeref);
            }

        default:
            SLANG_UNEXPECTED("unhandled case");
            UNREACHABLE_RETURN(LegalExpr());
        }

#if 0
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
            return LegalExpr(createSimpleSubscriptExpr(
                baseExpr.getExpr(),
                indexExpr));
        }
#endif
    }

    LegalExpr visitIndexExpr(
        IndexExpr* subscriptExpr)
    {
        auto baseExpr = legalizeExpr(subscriptExpr->BaseExpression);
        auto indexExpr = legalizeSimpleExpr(subscriptExpr->IndexExpression);

        if(baseExpr.getFlavor() == LegalExpr::Flavor::simple)
        {
            // Default case: just reconstrut a subscript expr
            RefPtr<IndexExpr> loweredExpr = new IndexExpr();
            lowerExprCommon(loweredExpr, subscriptExpr);
            loweredExpr->BaseExpression = baseExpr.getSimple();
            loweredExpr->IndexExpression = indexExpr;
            return LegalExpr(loweredExpr);
        }

        return createSubscriptExpr(baseExpr, indexExpr);
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
                    varType = arrayType->baseType;
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

    // Take a legalized expression that might be represented as a tuple,
    // and turn it back into a single ordinary expression of the given type.
    //
    // This is used in the case where we tuple-ified a value that has
    // a legal type, but just isn't legal to use in a particular context.
    static RefPtr<Expr> reifyTuple(
        LegalExpr       legalExpr,
        RefPtr<Type>    type)
    {
        if (legalExpr.getFlavor() == LegalExpr::Flavor::simple)
            return legalExpr.getSimple();

        if (auto declRefType = type->As<DeclRefType>())
        {
            auto declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
            {
                // We want a single value of an aggregate type, which
                // means we need to extract each of its fields from
                // the expression.

                switch (legalExpr.getFlavor())
                {
                case LegalExpr::Flavor::tuple:
                    {
                        auto tupleExpr = legalExpr.getTuple();

                        RefPtr<AggTypeCtorExpr> resultExpr = new AggTypeCtorExpr();
                        resultExpr->type.type = type;
                        resultExpr->base.type = type;
                        SLANG_RELEASE_ASSERT(resultExpr->type.type);

                        UInt fieldCounter = 0;
                        for (auto fieldDeclRef : getMembersOfType<StructField>(aggTypeDeclRef))
                        {
                            if (fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                                continue;

                            UInt fieldIndex = fieldCounter++;

                            resultExpr->Arguments.Add(reifyTuple(
                                tupleExpr->elements[fieldIndex].expr,
                                GetType(fieldDeclRef)));
                        }

                        return resultExpr;
                    }
                    break;
                }

            }
        }
        // TODO: need to handle array types here...

        SLANG_UNEXPECTED("unhandled case");
        UNREACHABLE_RETURN(legalExpr.getSimple());
    }

    static LegalExpr maybeReifyTuple(
        LegalExpr       legalExpr,
        LegalType       expectedLegalType)
    {
        if (expectedLegalType.flavor != LegalType::Flavor::simple)
            return legalExpr;

        RefPtr<Type> expectedType = expectedLegalType.getSimple();
        if(auto errorType = expectedType->As<ErrorType>())
        {
            return legalExpr;
        }

        if (legalExpr.getFlavor() == LegalExpr::Flavor::simple)
            return legalExpr;

        return LegalExpr(reifyTuple(legalExpr, expectedLegalType.getSimple()));
    }

    // This function exists to work around cases where `addArgs` gets called
    // and the structure of the type expected in context (the legalized parameter
    // type) differs from the structure of the actual argument.
    //
    // This function ignores type information and just adds things based on
    // what is present in the actual expression.
    void addArgsWorkaround(
        ExprWithArgsBase*   callExpr,
        LegalExpr           argExpr)
    {

        switch (argExpr.getFlavor())
        {
        case LegalExpr::Flavor::none:
            break;

        case LegalExpr::Flavor::simple:
            addArg(callExpr, argExpr.getSimple());
            break;

        case LegalExpr::Flavor::tuple:
            {
                auto aa = argExpr.getTuple();
                auto elementCount = aa->elements.Count();
                for (UInt ee = 0; ee < elementCount; ++ee)
                {
                    addArgsWorkaround(callExpr, aa->elements[ee].expr);
                }
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto aa = argExpr.getPair();
                addArgsWorkaround(callExpr, aa->ordinary);
                addArgsWorkaround(callExpr, aa->special);
            }
            break;

        case LegalExpr::Flavor::implicitDeref:
            {
                auto aa = argExpr.getImplicitDeref();
                addArgsWorkaround(callExpr, aa->valueExpr);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled case");
            break;
        }
    }

    void addArgs(
        ExprWithArgsBase*   callExpr,
        LegalType           argType,
        LegalExpr           argExpr)
    {
        argExpr = maybeReifyTuple(argExpr, argType);

        if (argExpr.getFlavor() != argType.flavor)
        {
            // A mismatch may also arise if we are in the `-no-checking` mode,
            // so that we are making a call that didn't type-check.
            addArgsWorkaround(callExpr, argExpr);
            return;
        }

        switch (argExpr.getFlavor())
        {
        case LegalExpr::Flavor::none:
            break;

        case LegalExpr::Flavor::simple:
            addArg(callExpr, argExpr.getSimple());
            break;

        case LegalExpr::Flavor::tuple:
            {
                auto aa = argExpr.getTuple();
                auto at = argType.getTuple();
                auto elementCount = aa->elements.Count();
                for (UInt ee = 0; ee < elementCount; ++ee)
                {
                    addArgs(callExpr, at->elements[ee].type, aa->elements[ee].expr);
                }
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto aa = argExpr.getPair();
                auto at = argType.getPair();
                addArgs(callExpr, at->ordinaryType, aa->ordinary);
                addArgs(callExpr, at->specialType, aa->special);
            }
            break;

        case LegalExpr::Flavor::implicitDeref:
            {
                auto aa = argExpr.getImplicitDeref();
                auto at = argType.getImplicitDeref();
                addArgs(callExpr, at->valueType, aa->valueExpr);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled case");
            break;
        }
    }

    RefPtr<Expr> lowerCallExpr(
        RefPtr<InvokeExpr>  loweredExpr,
        InvokeExpr*         expr)
    {
        lowerExprCommon(loweredExpr, expr);

        loweredExpr->FunctionExpr = legalizeSimpleExpr(expr->FunctionExpr);

        for (auto arg : expr->Arguments)
        {
            auto argType = lowerAndLegalizeType(arg->type.type);
            auto loweredArg = legalizeExpr(arg);
            addArgs(loweredExpr, argType, loweredArg);
        }

        return loweredExpr;
    }

    LegalExpr visitInvokeExpr(
        InvokeExpr* expr)
    {
        // Create a clone with the same class
        InvokeExpr* loweredExpr = (InvokeExpr*) expr->getClass().createInstance();
        return LegalExpr(lowerCallExpr(loweredExpr, expr));
    }

    LegalExpr visitHiddenImplicitCastExpr(
        HiddenImplicitCastExpr* expr)
    {
        LegalExpr legalArg = legalizeExpr(expr->Arguments[0]);
        if(legalArg.getFlavor() == LegalExpr::Flavor::simple)
        {
            InvokeExpr* loweredExpr = (InvokeExpr*) expr->getClass().createInstance();
            lowerExprCommon(loweredExpr, expr);
            loweredExpr->FunctionExpr = legalizeSimpleExpr(expr->FunctionExpr);
            addArg(loweredExpr, legalArg.getSimple());
            return LegalExpr(loweredExpr);
        }
        else
        {
            // If we hit this case, then there seems to have been a type-checking
            // error around a type that needed to be desugared. We want to use
            // the original expression rather than hide it behind a cast, because
            // it might need to be unpacked into multiple arguments for a call, etc.
            //
            return legalArg;
        }
    }

    LegalExpr visitSelectExpr(
        SelectExpr* expr)
    {
        // TODO: A tuple needs to be special-cased here

        return LegalExpr(lowerCallExpr(new SelectExpr(), expr));
    }

    LegalExpr visitDerefExpr(
        DerefExpr*  expr)
    {
        auto legalBase = legalizeExpr(expr->base);
        if (legalBase.getFlavor() == LegalExpr::Flavor::simple)
        {
            // Default case is just to lower a dereference opertion
            // into another dereference.
            RefPtr<DerefExpr> loweredExpr = new DerefExpr();
            lowerExprCommon(loweredExpr, expr);
            loweredExpr->base = legalBase.getSimple();
            return LegalExpr(loweredExpr);
        }

        return deref(legalBase);

#if 0
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
            else
            {
                // No primary expression? Then there is nothing
                // to dereference.
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
#endif
    }

    DiagnosticSink* getSink()
    {
        return &shared->compileRequest->mSink;
    }

    LegalExpr visitStaticMemberExpr(
        StaticMemberExpr* expr)
    {
        auto loweredBase = legalizeExpr(expr->BaseExpression);
        auto loweredDeclRef = translateDeclRef(expr->declRef);

        // TODO: we should probably support type-type members here.

        RefPtr<StaticMemberExpr> loweredExpr = new StaticMemberExpr();
        lowerExprCommon(loweredExpr, expr);
        loweredExpr->BaseExpression = loweredBase.getSimple();
        loweredExpr->declRef = loweredDeclRef.As<Decl>();
        loweredExpr->name = expr->name;

        return LegalExpr(loweredExpr);
    }

    LegalExpr visitMemberExpr(
        MemberExpr* expr)
    {
        assert(expr->BaseExpression);
        auto legalBase = legalizeExpr(expr->BaseExpression);
        assert(legalBase);

        if (legalBase.getFlavor() == LegalExpr::Flavor::simple)
        {
            // Default handling:
            RefPtr<MemberExpr> loweredExpr = new MemberExpr();
            lowerExprCommon(loweredExpr, expr);
            loweredExpr->BaseExpression = legalBase.getSimple();
            loweredExpr->declRef = translateDeclRef(expr->declRef);
            loweredExpr->name = expr->name;
            assert(loweredExpr->BaseExpression);
            return LegalExpr(loweredExpr);
        }

        return extractField(legalBase, expr->declRef.As<VarDeclBase>());
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

    RefPtr<Decl> visitScopeDecl(ScopeDecl* decl)
    {
        RefPtr<ScopeDecl> loweredDecl = new ScopeDecl();
        lowerDeclCommon(loweredDecl, decl);
        return loweredDecl;
    }

    LoweringVisitor pushScope(
        RefPtr<ScopeStmt>   loweredStmt,
        RefPtr<ScopeStmt>   originalStmt)
    {
        loweredStmt->scopeDecl = translateDeclRef(originalStmt->scopeDecl)->As<ScopeDecl>();

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
        LegalExpr     expr)
    {
        // Desugar tuples in statement position
        switch (expr.getFlavor())
        {
        case LegalExpr::Flavor::none:
            break;

        case LegalExpr::Flavor::simple:
            addSimpleExprStmt(expr.getSimple());
            break;

        case LegalExpr::Flavor::tuple:
            {
                auto tupleExpr = expr.getTuple();
                for (auto ee : tupleExpr->elements)
                {
                    addExprStmt(ee.expr);
                }
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto pairExpr = expr.getPair();
                addExprStmt(pairExpr->ordinary);
                addExprStmt(pairExpr->special);
            }
            break;

        case LegalExpr::Flavor::implicitDeref:
            {
                auto implicitDerefExpr = expr.getImplicitDeref();
                addExprStmt(implicitDerefExpr->valueExpr);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled case");
            break;
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
        addExprStmt(legalizeExpr(stmt->Expression));
    }

    void visitDeclStmt(DeclStmt* stmt)
    {
        DeclVisitor::dispatch(stmt->decl);
    }

    Modifiers shallowCloneModifiers(Modifiers const& oldModifiers)
    {
        RefPtr<SharedModifiers> sharedModifiers = new SharedModifiers();
        sharedModifiers->next = oldModifiers.first;

        Modifiers newModifiers;
        newModifiers.first = sharedModifiers;
        return newModifiers;
    }

    void lowerStmtFields(
        Stmt* loweredStmt,
        Stmt* originalStmt)
    {
        loweredStmt->loc = originalStmt->loc;
        loweredStmt->modifiers = shallowCloneModifiers(originalStmt->modifiers);
    }

    void lowerScopeStmtFields(
        ScopeStmt* loweredStmt,
        ScopeStmt* originalStmt)
    {
        lowerStmtFields(loweredStmt, originalStmt);
        loweredStmt->scopeDecl = translateDeclRef(originalStmt->scopeDecl)->As<ScopeDecl>();
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

        loweredStmt->tokens = stmt->tokens;

        addStmt(loweredStmt);
    }

    void visitCaseStmt(CaseStmt* stmt)
    {
        RefPtr<CaseStmt> loweredStmt = new CaseStmt();
        lowerChildStmtFields(loweredStmt, stmt);

        loweredStmt->expr = legalizeSimpleExpr(stmt->expr);

        addStmt(loweredStmt);
    }

    void visitIfStmt(IfStmt* stmt)
    {
        RefPtr<IfStmt> loweredStmt = new IfStmt();
        lowerStmtFields(loweredStmt, stmt);

        loweredStmt->Predicate = legalizeSimpleExpr(stmt->Predicate);
        loweredStmt->PositiveStatement = lowerStmt(stmt->PositiveStatement);
        loweredStmt->NegativeStatement = lowerStmt(stmt->NegativeStatement);

        addStmt(loweredStmt);
    }

    void visitSwitchStmt(SwitchStmt* stmt)
    {
        RefPtr<SwitchStmt> loweredStmt = new SwitchStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->condition = subVisitor.legalizeSimpleExpr(stmt->condition);
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
        loweredStmt->SideEffectExpression = subVisitor.legalizeSimpleExpr(stmt->SideEffectExpression);
        loweredStmt->PredicateExpression = subVisitor.legalizeSimpleExpr(stmt->PredicateExpression);
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
        shared->mapOriginalDeclToLowered[varDecl] = nullptr;

        auto varType = lowerTypeExprEx(varDecl->type);

        for (IntegerLiteralValue ii = rangeBeginVal; ii < rangeEndVal; ++ii)
        {
            RefPtr<ConstantExpr> constExpr = new ConstantExpr();
            constExpr->type.type = varType.type;
            constExpr->ConstType = ConstantExpr::ConstantType::Int;
            constExpr->integerValue = ii;

            shared->mapOriginalDeclToExpr[varDecl] = LegalExpr(constExpr);

            lowerStmtImpl(stmt->body);
        }
    }

    void visitWhileStmt(WhileStmt* stmt)
    {
        RefPtr<WhileStmt> loweredStmt = new WhileStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Predicate = subVisitor.legalizeSimpleExpr(stmt->Predicate);
        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);

        addStmt(loweredStmt);
    }

    void visitDoWhileStmt(DoWhileStmt* stmt)
    {
        RefPtr<DoWhileStmt> loweredStmt = new DoWhileStmt();
        lowerScopeStmtFields(loweredStmt, stmt);

        LoweringVisitor subVisitor = pushScope(loweredStmt, stmt);

        loweredStmt->Statement = subVisitor.lowerStmt(stmt->Statement);
        loweredStmt->Predicate = subVisitor.legalizeSimpleExpr(stmt->Predicate);

        addStmt(loweredStmt);
    }

    RefPtr<Stmt> transformSyntaxField(Stmt* stmt)
    {
        return lowerStmt(stmt);
    }

    void lowerStmtCommon(Stmt* loweredStmt, Stmt* stmt)
    {
        loweredStmt->modifiers = shallowCloneModifiers(stmt->modifiers);
    }

    void assign(
        LegalExpr destExpr,
        LegalExpr srcExpr,
        AssignMode  mode = AssignMode::Default)
    {
        auto assignExpr = createAssignExpr(destExpr, srcExpr, mode);
        addExprStmt(assignExpr);
    }

    void assign(VarDeclBase* varDecl, LegalExpr expr)
    {
        assign(LegalExpr(createVarRef(getPosition(expr), varDecl)), expr);
    }

    void assign(LegalExpr expr, VarDeclBase* varDecl)
    {
        assign(expr, LegalExpr(createVarRef(getPosition(expr), varDecl)));
    }

    RefPtr<Expr> createTypeExpr(
        RefPtr<Type>    type)
    {
        auto typeType = new TypeType();
        typeType->setSession(getSession());
        typeType->type = type;

        auto result = new SharedTypeExpr();
        result->base.type = type;
        result->type.type = typeType;

        return result;
    }

    RefPtr<Expr> createCastExpr(
        RefPtr<Type>    type,
        RefPtr<Expr>    expr)
    {
        RefPtr<ExplicitCastExpr> castExpr = new ExplicitCastExpr();
        castExpr->loc = expr->loc;
        castExpr->type.type = type;

        castExpr->FunctionExpr = createTypeExpr(type);
        castExpr->Arguments.Add(expr);
        return castExpr;
    }

    // Like `assign`, but with some extra logic to handle cases
    // where the types don't actually line up, because of
    // differences in how something is declared in HLSL vs. GLSL
    void assignWithFixups(
        LegalExpr destExpr,
        LegalExpr srcExpr)
    {
        assign(destExpr, srcExpr, AssignMode::WithFixups);
    }

    void assignWithFixups(VarDeclBase* varDecl, LegalExpr expr)
    {
        assignWithFixups(LegalExpr(createVarRef(getPosition(expr), varDecl)), expr);
    }

    void assignWithFixups(LegalExpr expr, VarDeclBase* varDecl)
    {
        assignWithFixups(expr, LegalExpr(createVarRef(getPosition(expr), varDecl)));
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
                assign(resultVariable, legalizeExpr(stmt->Expression));
            }
            else
            {
                // Simple case
                loweredStmt->Expression = legalizeSimpleExpr(stmt->Expression);
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
            return lowerTypeEx(type);

        if (auto litVal = dynamic_cast<ConstantIntVal*>(val))
            return val;

        // We do not use subtype witness for ast lowering, return it unchanged.
        if (auto subtypeWitnessVal = dynamic_cast<SubtypeWitness*>(val))
            return val;
        SLANG_UNEXPECTED("unhandled value kind");
    }

    SubstitutionSet translateSubstitutions(
        SubstitutionSet  inSubstitutions)
    {
        if (!inSubstitutions) return SubstitutionSet();
        SubstitutionSet rs;
        if (auto genSubst = inSubstitutions.genericSubstitutions)
        {
            RefPtr<GenericSubstitution> result = new GenericSubstitution();
            result->genericDecl = translateDeclRef(genSubst->genericDecl)->As<GenericDecl>();
            for (auto arg : genSubst->args)
            {
                result->args.Add(translateVal(arg));
            }
            rs.genericSubstitutions = result;
        }
        if (auto thisSubst = inSubstitutions.thisTypeSubstitution)
        {
            RefPtr<ThisTypeSubstitution> result = new ThisTypeSubstitution();
            if (result->sourceType)
                result->sourceType = translateVal(result->sourceType);
            rs.thisTypeSubstitution = result;
        }
        return rs;
    }

    static Decl* getModifiedDecl(Decl* decl)
    {
        if (!decl) return nullptr;
        if (auto genericDecl = dynamic_cast<GenericDecl*>(decl->ParentDecl))
            return genericDecl;
        return decl;
    }

    DeclRef<Decl> translateDeclRef(
        DeclRef<Decl> const& declRef)
    {
        DeclRef<Decl> result;
        result.decl = translateDeclRefImpl(declRef);
        result.substitutions = translateSubstitutions(declRef.substitutions);
        return result;
    }

    RefPtr<Decl> translateDeclRef(
        Decl*   decl)
    {
        return translateDeclRefImpl(DeclRef<Decl>(decl, SubstitutionSet()));
    }

    LegalExpr translateSimpleLegalValToLegalExpr(IRValue* irVal)
    {
        switch (irVal->op)
        {
        case kIROp_global_var:
            {
                IRGlobalVar* globalVar = (IRGlobalVar*)irVal;
                String mangledName = globalVar->mangledName;
                SLANG_ASSERT(mangledName.Length() != 0);

                RefPtr<Expr> varRef = createUncheckedVarRef(mangledName);
                varRef->type.type = globalVar->getType()->getValueType();

                return LegalExpr(varRef);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled opcode");
            UNREACHABLE_RETURN(LegalExpr());
        }
    }

    LegalExpr translateLegalValToLegalExpr(LegalVal legalVal)
    {
        switch (legalVal.flavor)
        {
        case LegalVal::Flavor::none:
            return LegalExpr();

        case LegalVal::Flavor::simple:
            return translateSimpleLegalValToLegalExpr(legalVal.getSimple());
            break;

        case LegalVal::Flavor::pair:
            {
                auto pairVal = legalVal.getPair();
                RefPtr<PairPseudoExpr> pairExpr = new PairPseudoExpr();
                pairExpr->pairInfo = pairVal->pairInfo;
                pairExpr->ordinary = translateLegalValToLegalExpr(pairVal->ordinaryVal);
                pairExpr->special = translateLegalValToLegalExpr(pairVal->specialVal);
                return LegalExpr(pairExpr);
            }
            break;

        case LegalVal::Flavor::tuple:
            {
                auto tupleVal = legalVal.getTuple();
                RefPtr<TuplePseudoExpr> tupleExpr = new TuplePseudoExpr();
                for (auto ee : tupleVal->elements)
                {
                    TuplePseudoExpr::Element element;
                    element.fieldDeclRef = ee.fieldDeclRef;
                    element.expr = translateLegalValToLegalExpr(ee.val);
                    tupleExpr->elements.Add(element);
                }
                return LegalExpr(tupleExpr);
            }
            break;

        case LegalVal::Flavor::implicitDeref:
            {
                auto implicitDerefVal = legalVal.getImplicitDeref();
                RefPtr<ImplicitDerefPseudoExpr> implicitDerefExpr = new ImplicitDerefPseudoExpr();
                implicitDerefExpr->valueExpr = translateLegalValToLegalExpr(implicitDerefVal);
                return LegalExpr(implicitDerefExpr);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled flavor");
            UNREACHABLE_RETURN(LegalExpr());
        }
    }

    void maybeLegalizeIRGlobal(
        DeclRef<Decl>   declRef)
    {
        // We've been given a decl-ref to a value that was translated via IR,
        // and we need to determine if it needs custom handling for legalization,
        // because it was an IR global that got split.

        // TODO: this code is using decls in places it should use decl-refs,
        // and that likely needs to get cleaned up...
        auto decl = declRef.getDecl();

        // If we already have an expression registered, then don't bother.
        if (shared->mapOriginalDeclToExpr.ContainsKey(decl))
            return;

        String mangledName = getMangledName(declRef);
        if (mangledName.Length() == 0)
            return;

        LegalVal legalVal;
        if (!shared->typeLegalizationContext->mapMangledNameToLegalIRValue.TryGetValue(mangledName, legalVal))
            return;

        LegalExpr legalExpr = translateLegalValToLegalExpr(legalVal);
        shared->mapOriginalDeclToExpr.Add(decl, legalExpr);
    }

    RefPtr<Decl> translateDeclRefImpl(
        DeclRef<Decl>   declRef)
    {
        Decl* decl = declRef.getDecl();
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

        // If we are using the IR, and the declaration comes from
        // an imported module (rather than the "rewrite-mode" module
        // being translated), then we need to ensure that it gets lowered
        // to IR instead.
        if (shared->compileRequest->compileFlags & SLANG_COMPILE_FLAG_USE_IR)
        {
            auto parentModule = findModuleForDecl(decl);
            if (parentModule && (parentModule != shared->mainModuleDecl))
            {
                // This declaration should already have been lowered to
                // the IR during the "walk" pass that happened earlier,
                // and so we won't do it again here.
                //
                // Instead, we need to check if the particular
                // declaration is one that needs to be swapped for
                // a legalized expression (e.g., because it was an IR
                // global that got split)
                //
                maybeLegalizeIRGlobal(declRef);

                // Remember that this declaration is handled via IR,
                // rather than being present in the legalized AST.
                shared->result.irDecls.Add(declRef.getDecl());

                // This method can't actually return a `LegalExpr`,
                // so for now we just assume that the original
                // declaration is the right stand-in for the IR
                // value we want.

                return decl;
            }
        }

        if (getModifiedDecl(decl)->HasModifier<LegalizedModifier>())
        {
            // We are trying to translate a reference to a declaration
            // that was created by the type legalization process. The
            // target declaration should already be placed inside of
            // the output module.
            return decl;
        }

        RefPtr<Decl> loweredDecl;
        if (shared->mapOriginalDeclToLowered.TryGetValue(decl, loweredDecl))
            return loweredDecl;

        // Time to force it
        return lowerDecl(decl);
    }

    DeclRef<ContainerDecl> translateDeclRef(
        DeclRef<ContainerDecl>  declRef)
    {
        return translateDeclRef(declRef).As<ContainerDecl>();
    }

    RefPtr<Decl> lowerDeclBase(
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
        RefPtr<Decl> loweredDecl = DeclVisitor::dispatch(decl);
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

    void registerLoweredDecl(RefPtr<Decl> loweredDecl, Decl* decl)
    {
        shared->mapOriginalDeclToLowered.Add(decl, loweredDecl);
        shared->mapLoweredDeclToOriginal.Add(loweredDecl.Ptr(), decl);
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
        RefPtr<Decl> loweredParent;
        if (auto genericParentDecl = decl->ParentDecl->As<GenericDecl>())
            loweredParent = translateDeclRef(genericParentDecl->ParentDecl);
        else
            loweredParent = translateDeclRef(decl->ParentDecl);
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
        loweredDecl->modifiers = shallowCloneModifiers(decl->modifiers);

        // deal with layout stuff

        if (auto fieldLayout = tryToFindLayout(decl))
        {
            attachLayout(loweredDecl, fieldLayout);
        }
    }

    // Catch-all

    RefPtr<Decl> visitSyntaxDecl(SyntaxDecl*)
    {
        return nullptr;
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

    RefPtr<Decl> visitModuleDecl(ModuleDecl*)
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

    RefPtr<Decl> visitAssocTypeDecl(AssocTypeDecl * /*assocType*/)
    {
        // not supported
        SLANG_UNREACHABLE("visitAssocTypeDecl in LowerVisitor");
        UNREACHABLE_RETURN(nullptr);
    }

    RefPtr<Decl> visitGlobalGenericParamDecl(GlobalGenericParamDecl * /*decl*/)
    {
        // not supported
        SLANG_UNREACHABLE("visitGlobalGenericParamDecl in LowerVisitor");
        UNREACHABLE_RETURN(nullptr);
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

        // TODO: Need to handle the case where we `typedef` an aggregate
        // type that needs to be legalized; in that case we should desugar
        // the `typedef` out of existence.
        loweredDecl->type = lowerTypeExprEx(decl->type);

        addMember(shared->loweredProgram, loweredDecl);
        return loweredDecl;
    }

    RefPtr<Decl> visitImportDecl(ImportDecl*)
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

    RefPtr<Decl> visitEmptyDecl(EmptyDecl* decl)
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

    Type* unwrapArray(Type* inType)
    {
        auto type = inType;
        while (auto arrayType = type->As<ArrayExpressionType>())
        {
            type = arrayType->baseType;
        }
        return type;
    }

    RefPtr<Decl> visitAggTypeDecl(AggTypeDecl* decl)
    {
        // An aggregate type declaration might get "legalized away"
        // and result in a new type declaration created by the
        // type legalization logic. If that happens, we don't want
        // the original type declaration to appear in the output.
        //
        // If the result *doesn't* get legalized away, though, we
        // need to try to reproduce this declaration as it originally
        // appeared.

        // We start by creating a type to reference this declaration,
        // and then we will try to legalize that.
        //
        // Note: This logic shouldn't need to defend against generic
        // types, since it won't get applied to Slang code that might
        // include generics (just HLSL/GLSL code).
        RefPtr<DeclRefType> declRefType = DeclRefType::Create(
            getSession(),
            makeDeclRef(decl));
        DeclRef<Decl> declRef = declRefType->declRef;

        LegalType legalType = legalizeType(getTypeLegalizationContext(), declRefType);
        if(legalType.flavor != LegalType::Flavor::simple)
        {
            // Something happened to this type during legalization, so
            // we don't want to let its declaration appear in the output.
            //
            // However, we need to ensure that when declaration references
            // that might reference this declaration get constructed (e.g.,
            // this might be the `T` in a `ConstantBuffer<T>`, we have something
            // to stick in there.
            //
            // For now we'll use the original declaration and hope for the best.
            return decl;
        }

        // if we get this far, then we want to produce an "equivalent"
        // aggregate type declaration to what the user wrote.

        RefPtr<StructDecl> loweredDecl = new StructDecl();
        lowerDeclCommon(loweredDecl, decl);

        for (auto field : decl->getMembersOfType<VarDeclBase>())
        {
            // We lower the field, which will involve lowering the field type
            auto loweredField = translateDeclRef(field)->As<VarDeclBase>();

            // Add the field to the result declaration
            addMember(loweredDecl, loweredField);
        }

        // TODO: we should really be copying over *all* the members,
        // in the case where this is a user-authored type.

        addMember(
            shared->loweredProgram,
            loweredDecl);

        return loweredDecl;
    }

    RefPtr<VarDeclBase> lowerSimpleVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl,
        TypeExp const&      loweredType)
    {
        lowerDeclCommon(loweredDecl, decl);

        loweredDecl->type = loweredType;
        loweredDecl->initExpr = legalizeSimpleExpr(decl->initExpr);

        return loweredDecl;
    }

    RefPtr<VarDeclBase> lowerSimpleVarDeclCommon(
        RefPtr<VarDeclBase> loweredDecl,
        VarDeclBase*        decl)
    {
        auto loweredType = lowerTypeExprEx(decl->type);
        return lowerSimpleVarDeclCommon(loweredDecl, decl, loweredType);
    }

    RefPtr<StructTypeLayout> getBodyStructTypeLayout(RefPtr<TypeLayout> typeLayout)
    {
        if (!typeLayout)
            return nullptr;

        while (auto parameterGroupTypeLayout = typeLayout.As<ParameterGroupTypeLayout>())
        {
            typeLayout = parameterGroupTypeLayout->offsetElementTypeLayout;
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

    LegalExpr deref(
        LegalExpr   base)
    {
        switch (base.getFlavor())
        {
        case LegalExpr::Flavor::none:
            return LegalExpr();

        case LegalExpr::Flavor::simple:
            {
                auto simpleBase = base.getSimple();

                RefPtr<DerefExpr> resultExpr = new DerefExpr();
                // TODO: need to fill in a type here?
                resultExpr->base = simpleBase;
                return LegalExpr(resultExpr);
            }
            break;

        case LegalExpr::Flavor::tuple:
            {
                auto tupleExpr = base.getTuple();
                RefPtr<TuplePseudoExpr> resultExpr = new TuplePseudoExpr();

                for (auto ee : tupleExpr->elements)
                {
                    TuplePseudoExpr::Element element;
                    element.fieldDeclRef = ee.fieldDeclRef;
                    element.expr = deref(ee.expr);

                    resultExpr->elements.Add(element);
                }

                return LegalExpr(resultExpr);
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto basePair = base.getPair();
                RefPtr<PairPseudoExpr> resultPair = new PairPseudoExpr();
                resultPair->pairInfo = basePair->pairInfo;

                resultPair->ordinary = deref(basePair->ordinary);
                resultPair->special = deref(basePair->special);

                return LegalExpr(resultPair);
            }

        case LegalExpr::Flavor::implicitDeref:
            {
                auto implicitDerefExpr = base.getImplicitDeref();
                return implicitDerefExpr->valueExpr;
            }
            break;

        default:
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(LegalExpr());
            break;
        }
    }

    LegalExpr extractField(
        LegalExpr               base,
        DeclRef<VarDeclBase>    fieldDeclRef)
    {
        switch (base.getFlavor())
        {
        case LegalExpr::Flavor::none:
            return LegalExpr();

        case LegalExpr::Flavor::simple:
            {
                auto simpleBase = base.getSimple();

                RefPtr<MemberExpr> resultExpr = new MemberExpr();
                resultExpr->BaseExpression = simpleBase;
                resultExpr->type.type = GetType(fieldDeclRef);
                resultExpr->declRef = translateDeclRef(fieldDeclRef.As<Decl>());
                resultExpr->name = fieldDeclRef.GetName();
                return LegalExpr(resultExpr);
            }
            break;

        case LegalExpr::Flavor::tuple:
            {
                auto baseTuple = base.getTuple();
                for (auto ee : baseTuple->elements)
                {
                    if (ee.fieldDeclRef.Equals(fieldDeclRef))
                    {
                        return ee.expr;
                    }
                }

                SLANG_UNEXPECTED("failed to find tuple element");
            }
            break;

        case LegalExpr::Flavor::pair:
            {
                auto basePair = base.getPair();

                // Need to determine if this field is on the
                // ordinary side, the special side, or both.

                auto pairInfo = basePair->pairInfo;
                auto pairElement = pairInfo->findElement(fieldDeclRef);
                if (!pairElement)
                {
                    SLANG_UNEXPECTED("failed to find tuple element");
                    UNREACHABLE_RETURN(LegalExpr());
                }

                if ((pairElement->flags & PairInfo::kFlag_hasOrdinaryAndSpecial) == PairInfo::kFlag_hasOrdinaryAndSpecial)
                {
                    // we have both flags
                    LegalExpr ordinaryResult = extractField(basePair->ordinary,
                        pairElement->ordinaryFieldDeclRef.As<VarDeclBase>());
                    LegalExpr specialResult = extractField(basePair->special, fieldDeclRef);

                    RefPtr<PairPseudoExpr> resultPair = new PairPseudoExpr();
                    resultPair->ordinary = ordinaryResult;
                    resultPair->special = specialResult;
                    resultPair->pairInfo = pairElement->type.getPair()->pairInfo;
                    return LegalExpr(resultPair);
                }
                else if(pairElement->flags & PairInfo::kFlag_hasOrdinary)
                {
                    return extractField(basePair->ordinary,
                        pairElement->ordinaryFieldDeclRef.As<VarDeclBase>());
                }
                else
                {
                    SLANG_ASSERT(pairElement->flags & PairInfo::kFlag_hasSpecial);
                    return extractField(basePair->special, fieldDeclRef);
                }
            }
            break;

        case LegalExpr::Flavor::implicitDeref:
            {
                auto baseImplicitDeref = base.getImplicitDeref();

                RefPtr<ImplicitDerefPseudoExpr> resultImplicitDeref = new ImplicitDerefPseudoExpr();
                resultImplicitDeref->valueExpr = extractField(
                    baseImplicitDeref->valueExpr,
                    fieldDeclRef);
                return LegalExpr(resultImplicitDeref);
            }

        default:
            SLANG_UNEXPECTED("unimplemented");
            UNREACHABLE_RETURN(LegalExpr());
            break;
        }
    }

    void attachLayoutModifier(
        VarDeclBase*    decl,
        VarLayout*      layout)
    {
        if (!layout)
            return;

        RefPtr<ComputedLayoutModifier> mod = new ComputedLayoutModifier();
        mod->layout = layout;
        addModifier(decl, mod);
    }

    RefPtr<VarDeclBase> declareSimpleVar(
        VarDeclBase*                decl,
        SourceLoc const&            loc,
        Name*                       name,
        SyntaxClass<VarDeclBase>    loweredDeclClass,
        VarLayout*                  varLayout,
        RefPtr<Expr>                initExpr,
        TypeExp const&              typeExpr)
    {
        RefPtr<VarDeclBase> loweredDecl = loweredDeclClass.createInstance();
        if (decl)
        {
            lowerDeclCommon(loweredDecl, decl);
        }
        loweredDecl->nameAndLoc.name = name;
        loweredDecl->nameAndLoc.loc = loc;

        loweredDecl->type = typeExpr;
        loweredDecl->initExpr = initExpr;

        attachLayoutModifier(loweredDecl, varLayout);

        addDecl(loweredDecl);
        return loweredDecl;
    }

    LegalExpr declareSimpleVar(
        VarDeclBase*                originalDecl,
        LegalVarChain*              varChain,
        SourceLoc const&            loc,
        String const&               name,
        SyntaxClass<VarDeclBase>    loweredDeclClass,
        TypeLayout*                 typeLayout,
        LegalExpr                   legalInit,
        LegalTypeExpr const&        legalTypeExpr)
    {
        RefPtr<VarLayout> varLayout = createVarLayout(varChain, typeLayout);

        RefPtr<VarDeclBase> varDecl = declareSimpleVar(
            originalDecl,
            loc,
            getName(name),
            loweredDeclClass,
            varLayout,
            legalInit.getSimple(),
            legalTypeExpr.getSimple());

        return createVarRef(loc, varDecl);
    }

    LegalExpr declareVars(
        VarDeclBase*                originalDecl,
        LegalVarChain*              varChain,
        SourceLoc const&            loc,
        String const&               name,
        SyntaxClass<VarDeclBase>    loweredDeclClass,
        TypeLayout*                 typeLayout,
        LegalExpr                   legalInit,
        LegalTypeExpr const&        legalTypeExpr)
    {
        auto& legalType = legalTypeExpr.type;
        switch (legalType.flavor)
        {
        case LegalType::Flavor::simple:
            {
                return declareSimpleVar(
                    originalDecl,
                    varChain,
                    loc,
                    name,
                    loweredDeclClass,
                    typeLayout,
                    legalInit,
                    legalTypeExpr);
            }
            break;

        case LegalType::Flavor::implicitDeref:
            {
                auto implicitDerefType = legalType.getImplicitDeref();

                auto valueType = implicitDerefType->valueType;

                // Don't apply dereferencing to the type layout, because
                // other steps will also implicitly remove wrappers (like
                // parameter groups) and this could mess up the final
                // type layout for a variable.
                //
                // Instead, any other "unwrapping" that needs to occur
                // when declaring variables should be handled in the
                // case for the specific type (e.g., when extracting
                // fields for a tuple, we should auto-dereference).
                auto valueTypeLayout = typeLayout;
                auto valueInit = deref(legalInit);

                LegalExpr valueExpr = declareVars(
                    originalDecl,
                    varChain,
                    loc,
                    name,
                    loweredDeclClass,
                    valueTypeLayout,
                    valueInit,
                    valueType);

                RefPtr<ImplicitDerefPseudoExpr> implicitDerefExpr = new ImplicitDerefPseudoExpr();
                implicitDerefExpr->valueExpr = valueExpr;
                return LegalExpr(implicitDerefExpr);
            }
            break;

        case LegalType::Flavor::tuple:
            {
                auto tupleType = legalType.getTuple();

                RefPtr<TuplePseudoExpr> tupleExpr = new TuplePseudoExpr();

                for (auto ff : tupleType->elements)
                {
                    RefPtr<VarLayout> fieldLayout = getFieldLayout(
                        typeLayout,
                        ff.fieldDeclRef);
                    RefPtr<TypeLayout> fieldTypeLayout = fieldLayout ? fieldLayout->typeLayout : nullptr;
                    SLANG_ASSERT(fieldLayout || !typeLayout);
                    LegalExpr fieldInit = extractField(legalInit, ff.fieldDeclRef);

                    String fieldName = name + "_" + getText(ff.fieldDeclRef.GetName());

                    LegalVarChain fieldVarChain;
                    fieldVarChain.next = varChain;
                    fieldVarChain.varLayout = fieldLayout;

                    LegalExpr fieldExpr = declareVars(
                        nullptr,
                        &fieldVarChain,
                        loc,
                        fieldName,
                        loweredDeclClass,
                        fieldTypeLayout,
                        fieldInit,
                        ff.type);

                    TuplePseudoExpr::Element element;
                    element.expr = fieldExpr;
                    element.fieldDeclRef = ff.fieldDeclRef;

                    tupleExpr->elements.Add(element);
                }

                return LegalExpr(tupleExpr);
            }
            break;

        case LegalType::Flavor::pair:
            {
                auto pairType = legalType.getPair();
                RefPtr<PairPseudoExpr> pairExpr = new PairPseudoExpr();
                pairExpr->pairInfo = pairType->pairInfo;
                pairExpr->loc = loc;

                pairExpr->ordinary = declareVars(
                    originalDecl,
                    varChain,
                    loc,
                    name,
                    loweredDeclClass,
                    typeLayout,
                    legalInit,
                    pairType->ordinaryType);

                pairExpr->special = declareVars(
                    originalDecl,
                    varChain,
                    loc,
                    name,
                    loweredDeclClass,
                    typeLayout,
                    legalInit,
                    pairType->specialType);

                return LegalExpr(pairExpr);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled legalized type flavor");
            UNREACHABLE_RETURN(LegalExpr());
            break;
        }
    }

    void lowerVarDeclCommonInner(
        VarDeclBase*                decl,
        SyntaxClass<VarDeclBase>    loweredDeclClass)
    {
        auto legalTypeExpr = lowerAndLegalizeTypeExpr(decl->type);

        auto varLayout = tryToFindLayout(decl).As<VarLayout>();

        // Note: we lower the initialization expression, if any,
        // *before* we add the declaration to the current context (e.g., a statement being
        // built), so that any operations inside the initialization expression that
        // might need to inject statements/temporaries/whatever happen *before*
        // the declaration of this variable.
        auto legalInit = legalizeExpr(decl->initExpr);

        if (legalTypeExpr.type.flavor == LegalType::Flavor::simple)
        {
            declareSimpleVar(
                decl,
                decl->nameAndLoc.loc,
                decl->getName(),
                loweredDeclClass,
                varLayout,
                legalInit.getSimple(),
                legalTypeExpr.getSimple());
        }
        else
        {
            LegalVarChain varChain;
            varChain.next = nullptr;
            varChain.varLayout = varLayout;

            LegalExpr legalExpr = declareVars(
                decl,
                &varChain,
                decl->nameAndLoc.loc,
                getText(decl->getName()),
                loweredDeclClass,
                varLayout ? varLayout->typeLayout : nullptr,
                legalInit,
                legalTypeExpr);

            shared->mapOriginalDeclToExpr.Add(decl, legalExpr);
            shared->mapOriginalDeclToLowered.AddIfNotExists(decl, nullptr);
        }
    }

    void lowerVarDeclCommon(
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
            subVisitor.parentDecl = translateDeclRef(parentModuleDecl)->As<ContainerDecl>();
            subVisitor.isBuildingStmt = false;

            subVisitor.lowerVarDeclCommonInner(decl, loweredDeclClass);
        }
        // TODO: handle `static` function-scope variables
        else
        {
            // The default behavior is to lower into whatever
            // scope was already in places
            lowerVarDeclCommonInner(decl, loweredDeclClass);
        }
    }

    SourceLanguage getSourceLanguage(ModuleDecl* moduleDecl)
    {
        for (auto translationUnit : shared->compileRequest->translationUnits)
        {
            if (moduleDecl == translationUnit->SyntaxNode)
                return translationUnit->sourceLanguage;
        }

        for (auto loadedModule : shared->compileRequest->loadedModulesList)
        {
            if (moduleDecl == loadedModule->moduleDecl)
                return SourceLanguage::Slang;
        }

        return SourceLanguage::Unknown;
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

    RefPtr<Decl> visitVariable(
        Variable* decl)
    {
        if (dynamic_cast<ModuleDecl*>(decl->ParentDecl))
        {
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

                    LegalExpr loweredExpr;
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

                    shared->mapOriginalDeclToExpr.Add(decl, loweredExpr);
                    shared->mapOriginalDeclToLowered.Add(decl, nullptr);
                    return nullptr;
                }
            }
        }

        lowerVarDeclCommon(decl, getClass<Variable>());

        return nullptr;
    }

    RefPtr<Decl> visitStructField(
        StructField* decl)
    {
        return lowerSimpleVarDeclCommon(new StructField(), decl);
    }

    RefPtr<Decl> visitParamDecl(
        ParamDecl* decl)
    {
        lowerVarDeclCommon(decl, getClass<ParamDecl>());

        return nullptr;
    }

    RefPtr<Decl> transformSyntaxField(DeclBase* decl)
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

    RefPtr<Decl> visitFunctionDeclBase(
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

        auto loweredReturnType = subVisitor.lowerAndlegalizeSimpleTypeExpr(decl->ReturnType);

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
            switch (baseType->baseType)
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
               
        auto substs = new GenericSubstitution();
        substs->genericDecl = vectorGenericDecl.Ptr();
        substs->args.Add(elementType);
        substs->args.Add(elementCount);

        auto declRef = DeclRef<Decl>(vectorTypeDecl.Ptr(), substs);

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

    LegalExpr lowerSimpleShaderParameterToGLSLGlobal(
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

        return LegalExpr(globalVarExpr);
    }

    LegalExpr lowerShaderParameterToGLSLGLobalsRec(
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
                arrayType->baseType,
                varLayout);
        }
        else if (auto declRefType = varType->As<DeclRefType>())
        {
            auto declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
            {
                // The shader parameter had a structured type, so we need
                // to destructure it into its constituent fields

                RefPtr<TuplePseudoExpr> tupleExpr = new TuplePseudoExpr();

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

                    TuplePseudoExpr::Element elem;
                    elem.fieldDeclRef = makeDeclRef(originalFieldDecl).As<VarDeclBase>();
                    elem.expr = loweredFieldExpr;

                    tupleExpr->elements.Add(elem);
                }

                // Okay, we are done with this parameter
                return LegalExpr(tupleExpr);
            }
        }

        // Default case: just try to emit things as-is
        return lowerSimpleShaderParameterToGLSLGlobal(info, varType, varLayout);
    }

    LegalExpr lowerShaderParameterToGLSLGLobals(
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

        auto loweredType = lowerAndLegalizeTypeExpr(originalVarDecl->type);

        auto loweredExpr = lowerShaderParameterToGLSLGLobalsRec(
            info,
            loweredType.type.getSimple(), // TODO: handle non-simple?
            paramLayout);

        return loweredExpr;
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
        auto loweredEntryPointFunc = visitFunctionDeclBase(entryPointDecl)->As<FunctionDeclBase>();

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
            localVarDecl->type = lowerAndlegalizeSimpleTypeExpr(paramDecl->type);

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
            subVisitor.assign(resultVarDecl, LegalExpr(callExpr));
        }
        else
        {
            // `void` return type: just call it
            subVisitor.addExprStmt(LegalExpr(callExpr));
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
                LegalExpr(createSimpleVarExpr("gl_PositionPerViewNV[0]")),
                LegalExpr(createSimpleVarExpr("gl_Position")));
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
            return visitFunctionDeclBase(entryPointDecl)->As<FuncDecl>();

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

StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout);

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
    EntryPointRequest*          entryPoint,
    ProgramLayout*              programLayout,
    CodeGenTarget               target,
    ExtensionUsageTracker*      extensionUsageTracker,
    IRSpecializationState*      irSpecializationState,
    TypeLegalizationContext*    typeLegalizationContext,
    List<Decl*>                 astDecls)
{
    SharedLoweringContext sharedContext;
    sharedContext.compileRequest = entryPoint->compileRequest;
    sharedContext.entryPointRequest = entryPoint;
    sharedContext.programLayout = programLayout;
    sharedContext.target = target;
    sharedContext.extensionUsageTracker = extensionUsageTracker;
    sharedContext.irSpecializationState = irSpecializationState;
    sharedContext.typeLegalizationContext = typeLegalizationContext;

    auto translationUnit = entryPoint->getTranslationUnit();
    sharedContext.mainModuleDecl = translationUnit->SyntaxNode;

    // Create a single module/program to hold all the lowered code
    // (with the exception of instrinsic/stdlib declarations, which
    // will be remain where they are)
    RefPtr<ModuleDecl> loweredProgram = new ModuleDecl();
    sharedContext.loweredProgram = loweredProgram;

    typeLegalizationContext->mainModuleDecl = sharedContext.mainModuleDecl;
    typeLegalizationContext->outputModuleDecl = loweredProgram;


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
        sharedContext.mapOriginalDeclToLowered.Add(
            rr->moduleDecl,
            loweredProgram);
    }

    // We also want to remember the layout information for
    // that declaration, so that we can apply it during emission
    attachLayout(loweredProgram,
        getGlobalStructLayout(programLayout));


    bool isRewrite = isRewriteRequest(translationUnit->sourceLanguage, target);
    sharedContext.isRewrite = isRewrite;

    sharedContext.entryPointLayout = visitor.findEntryPointLayout(entryPoint);

    if (isRewrite)
    {
        for (auto dd : astDecls)
        {
            // Skip non-global decls
            if (!dd->ParentDecl)
                continue;
            if (!dynamic_cast<ModuleDecl*>(dd->ParentDecl))
                continue;
            visitor.translateDeclRef(dd);
        }
    }
    else
    {
        // Emit everything we need other than the entry point first
        for (auto dd : astDecls)
        {
            // Skip non-global decls
            if (!dd->ParentDecl)
                continue;
            if (!dynamic_cast<ModuleDecl*>(dd->ParentDecl))
                continue;

            // Don't emit the entry point in this pass...
            if(dd == entryPoint->decl)
                continue;

            visitor.translateDeclRef(dd);
        }

        // Now emit the entry point, after all its dependencies have
        // been emitted.
        auto loweredEntryPoint = visitor.lowerEntryPoint(entryPoint);
        sharedContext.result.entryPoint = loweredEntryPoint;
    }

    sharedContext.result.program = sharedContext.loweredProgram;

    return sharedContext.result;
}

struct FindIRDeclUsedByASTVisitor
    : ExprVisitor<FindIRDeclUsedByASTVisitor, void>
    , StmtVisitor<FindIRDeclUsedByASTVisitor, void>
    , DeclVisitor<FindIRDeclUsedByASTVisitor, void>
    , ValVisitor<FindIRDeclUsedByASTVisitor, void, void>

{
    CompileRequest*         compileRequest;
    IRSpecializationState*  irSpecializationState;
    ModuleDecl*             mainModuleDecl;

    // Declarations to be processed by the AST lowering pass
    List<Decl*>*            astDecls;

    HashSet<DeclBase*>      seenDecls;
    HashSet<DeclBase*>      addedDecls;

    void walkType(Type* type)
    {
        if(!type) return;

        TypeVisitor::dispatch(type);
    }

    void walkVal(Val* val)
    {
        if(!val) return;

        ValVisitor::dispatch(val);
    }

    void walkExpr(Expr* expr)
    {
        if(!expr) return;

        ExprVisitor::dispatch(expr);
    }

    void walkStmt(Stmt* stmt)
    {
        if(!stmt) return;

        StmtVisitor::dispatch(stmt);
    }

    void walkSubst(Substitutions* subst)
    {
        if( auto genericSubst = dynamic_cast<GenericSubstitution*>(subst) )
        {
            for( auto arg : genericSubst->args )
            {
                walkVal(arg);
            }
        }
        // TODO: handle other cases here
    }

    void walkDeclRef(DeclRef<Decl> const& declRef)
    {
        Decl* decl = declRef.getDecl();
        if (!decl) return;

        // If this is a specialized declaration reference, then any
        // of the arguments also need to be walked.
        for(auto subst = declRef.substitutions.genericSubstitutions; subst; subst = subst->outer)
        {
            walkSubst(subst);
        }
        for (auto subst = declRef.substitutions.globalGenParamSubstitutions; subst; subst = subst->outer)
        {
            walkSubst(subst);
        }
        if (declRef.substitutions.thisTypeSubstitution)
            walkSubst(declRef.substitutions.thisTypeSubstitution);
        // If any parent of the declaration was in the stdlib, or
        // is registered as a builtin, then skip it.
        for (auto pp = decl; pp; pp = pp->ParentDecl)
        {
            if (pp->HasModifier<FromStdLibModifier>())
                return;

            if (pp->HasModifier<BuiltinModifier>())
                return;
        }

        // If we are using the IR, and the declaration comes from
        // an imported module (rather than the "rewrite-mode" module
        // being translated), then we need to ensure that it gets lowered
        // to IR instead.
        if (compileRequest->compileFlags & SLANG_COMPILE_FLAG_USE_IR)
        {
            auto parentModule = findModuleForDecl(decl);
            if (parentModule && (parentModule != mainModuleDecl))
            {
                // Ensure that the IR code for the given declaration
                // gets included in the output IR module, and *also*
                // that we generate a suitable specialization of it
                // if there are any substitutions in effect.

                getSpecializedGlobalValueForDeclRef(
                    irSpecializationState,
                    declRef);

                // TODO: we probably need to track this value...

                return;
            }
        }

        // If none of the above triggered, then we seemingly have
        // a declaration from the current module, and we should
        // add it to our work list so we can walk it too.
        addDecl(decl);
    }

    // Vals

    void visitIRProxyVal(IRProxyVal*)
    {}

    void visitConstantIntVal(ConstantIntVal*)
    {}

    void visitGenericParamIntVal(GenericParamIntVal* val)
    {
        walkDeclRef(val->declRef);
    }

    void visitWitness(Witness*)
    {}

    // Types

    void visitOverloadGroupType(OverloadGroupType*)
    {}

    void visitInitializerListType(InitializerListType*)
    {}

    void visitErrorType(ErrorType*)
    {}

    void visitIRBasicBlockType(IRBasicBlockType*)
    {}

    void visitDeclRefType(DeclRefType* type)
    {
        walkDeclRef(type->declRef);
    }

    void visitGenericDeclRefType(GenericDeclRefType* type)
    {
        walkDeclRef(type->declRef);
    }

    void visitNamedExpressionType(NamedExpressionType* type)
    {
        walkDeclRef(type->declRef);
    }

    void visitFuncType(FuncType* type)
    {
        for( auto p : type->paramTypes )
        {
            walkType(p);
        }
        walkType(type->resultType);
    }

    void visitTypeType(TypeType* type)
    {
        walkType(type->type);
    }

    void visitGroupSharedType(GroupSharedType* type)
    {
        walkType(type->valueType);
    }

    void visitArrayExpressionType(ArrayExpressionType* type)
    {
        walkType(type->baseType);
        walkVal(type->ArrayLength);
    }

    // Exprs

    void visitVarExpr(VarExpr* expr)
    {
        walkDeclRef(expr->declRef);
    }

    void visitMemberExpr(MemberExpr* expr)
    {
        walkExpr(expr->BaseExpression);
        walkDeclRef(expr->declRef);
    }

    void visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        walkExpr(expr->BaseExpression);
        walkDeclRef(expr->declRef);
    }

    void visitOverloadedExpr(OverloadedExpr* expr)
    {
        walkExpr(expr->base);

        // TODO: need to walk the lookup result too
    }

    void visitOverloadedExpr2(OverloadedExpr2* expr)
    {
        walkExpr(expr->base);
        for (auto & candidate : expr->candidiateExprs)
            walkExpr(candidate);
    }


    void visitConstantExpr(ConstantExpr*)
    {}

    void visitInitializerListExpr(InitializerListExpr* expr)
    {
        for(auto a : expr->args)
            walkExpr(a);
    }

    void visitAppExprBase(AppExprBase* expr)
    {
        walkExpr(expr->FunctionExpr);
        for(auto a : expr->Arguments)
            walkExpr(a);
    }

    void visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
    {
        walkType(expr->base);
        for(auto a : expr->Arguments)
            walkExpr(a);
    }

    void visitSharedTypeExpr(SharedTypeExpr* expr)
    {
        walkType(expr->base);
    }

    void visitAssignExpr(AssignExpr* expr)
    {
        walkExpr(expr->left);
        walkExpr(expr->right);
    }

    void visitIndexExpr(IndexExpr* expr)
    {
        walkExpr(expr->BaseExpression);
        walkExpr(expr->IndexExpression);
    }

    void visitSwizzleExpr(SwizzleExpr* expr)
    {
        walkExpr(expr->base);
    }

    void visitDerefExpr(DerefExpr* expr)
    {
        walkExpr(expr->base);
    }

    void visitParenExpr(ParenExpr* expr)
    {
        walkExpr(expr->base);
    }

    void visitThisExpr(ThisExpr*)
    {}

    // Stmts

    void visitSeqStmt(SeqStmt* stmt)
    {
        for( auto s : stmt->stmts )
        {
            walkStmt(s);
        }
    }

    void visitBlockStmt(BlockStmt* stmt)
    {
        walkStmt(stmt->body);
    }

    void visitUnparsedStmt(UnparsedStmt*)
    {}

    void visitEmptyStmt(EmptyStmt*)
    {}

    void visitDiscardStmt(DiscardStmt*)
    {}

    void visitDeclStmt(DeclStmt* stmt)
    {
        addDecl(stmt->decl);
    }

    void visitIfStmt(IfStmt* stmt)
    {
        walkExpr(stmt->Predicate);
        walkStmt(stmt->PositiveStatement);
        walkStmt(stmt->NegativeStatement);
    }

    void visitSwitchStmt(SwitchStmt* stmt)
    {
        walkExpr(stmt->condition);
        walkStmt(stmt->body);
    }

    void visitCaseStmt(CaseStmt* stmt)
    {
        walkExpr(stmt->expr);
    }

    void visitDefaultStmt(DefaultStmt*)
    {}

    void visitForStmt(ForStmt* stmt)
    {
        walkStmt(stmt->InitialStatement);
        walkExpr(stmt->SideEffectExpression);
        walkExpr(stmt->PredicateExpression);
        walkStmt(stmt->Statement);
    }

    void visitWhileStmt(WhileStmt* stmt)
    {
        walkExpr(stmt->Predicate);
        walkStmt(stmt->Statement);
    }

    void visitDoWhileStmt(DoWhileStmt* stmt)
    {
        walkExpr(stmt->Predicate);
        walkStmt(stmt->Statement);
    }

    void visitCompileTimeForStmt(CompileTimeForStmt* stmt)
    {
        addDecl(stmt->varDecl);
        walkExpr(stmt->rangeBeginExpr);
        walkExpr(stmt->rangeEndExpr);
        walkStmt(stmt->body);
    }

    void visitReturnStmt(ReturnStmt* stmt)
    {
        walkExpr(stmt->Expression);
    }

    void visitExpressionStmt(ExpressionStmt* stmt)
    {
        walkExpr(stmt->Expression);
    }

    void visitJumpStmt(JumpStmt*)
    {}

    // Decls

    void visitDeclGroup(DeclGroup* declGroup)
    {
        for( auto dd : declGroup->decls )
        {
            addDecl(dd);
        }
    }

    void visitContainerDeclCommon(ContainerDecl* decl)
    {
        for( auto mm : decl->Members )
        {
            addDecl(mm);
        }
    }

    void visitContainerDecl(ContainerDecl* decl)
    {
        visitContainerDeclCommon(decl);
    }

    void visitVarDeclBase(VarDeclBase* decl)
    {
        walkType(decl->type);
        walkExpr(decl->initExpr);
    }

    void visitAggTypeDeclBase(AggTypeDeclBase* decl)
    {
        visitContainerDeclCommon(decl);
    }

    void visitInheritanceDecl(InheritanceDecl* decl)
    {
        walkType(decl->base);
    }

    void visitTypeDefDecl(TypeDefDecl* decl)
    {
        walkType(decl->type);
    }

    void visitCallableDeclCommon(CallableDecl* decl)
    {
        visitContainerDeclCommon(decl);
        walkType(decl->ReturnType);
    }

    void visitCallableDecl(CallableDecl* decl)
    {
        visitCallableDeclCommon(decl);
    }

    void visitFunctionDeclBase(FunctionDeclBase* decl)
    {
        visitCallableDeclCommon(decl);
        walkStmt(decl->Body);
    }

    void visitImportDecl(ImportDecl*)
    {}

    void visitGenericTypeParamDecl(GenericTypeParamDecl*)
    {}

    void visitGenericTypeConstraintDecl(GenericTypeConstraintDecl*)
    {}

    void visitEmptyDecl(EmptyDecl*)
    {}

    void visitSyntaxDecl(SyntaxDecl*)
    {}

    //

    void addDecl(DeclBase* decl)
    {
        // Has this decl already been added
        // to the output list?
        if(addedDecls.Contains(decl))
            return;

        // Are we in the middel of processing this
        // decl?
        //
        // TODO: this implies a cycle, and we need to
        // break it!
        if (seenDecls.Contains(decl))
            return;

        seenDecls.Add(decl);

        // Recurse on the given decl
        DeclVisitor::dispatch(decl);

        // Add it to the output list, if needed
        if (auto dd = dynamic_cast<Decl*>(decl))
        {
            (*astDecls).Add(dd);
        }

        // Mark it as completely processed
        addedDecls.Add(decl);
    }

    void flush()
    {
    }
};


void findDeclsUsedByASTEntryPoint(
    EntryPointRequest*          entryPoint,
    CodeGenTarget               target,
    IRSpecializationState*      irSpecializationState,
    List<Decl*>&                outASTDecls)
{
    auto translationUnit = entryPoint->getTranslationUnit();
    auto mainModuleDecl = translationUnit->SyntaxNode;

    FindIRDeclUsedByASTVisitor visitor;
    visitor.compileRequest = entryPoint->compileRequest;
    visitor.irSpecializationState = irSpecializationState;
    visitor.mainModuleDecl = mainModuleDecl;
    visitor.astDecls = &outASTDecls;

    bool isRewrite = isRewriteRequest(translationUnit->sourceLanguage, target);

    if (isRewrite)
    {
        visitor.addDecl(mainModuleDecl);
    }
    else
    {
        visitor.addDecl(entryPoint->decl);
    }

    visitor.flush();
}



}
