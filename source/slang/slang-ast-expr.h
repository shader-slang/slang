// slang-ast-expr.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

using SpvWord = uint32_t;

// Syntax class definitions for expressions.
// 
    // A placeholder for where an Expr is expected but is missing from source.
class IncompleteExpr : public Expr
{
    SLANG_AST_CLASS(IncompleteExpr)
};
    // Base class for expressions that will reference declarations
class DeclRefExpr: public Expr
{
    SLANG_ABSTRACT_AST_CLASS(DeclRefExpr)

    
    // The declaration of the symbol being referenced

    DeclRef<Decl> declRef;
    // The name of the symbol being referenced
    Name* name = nullptr;

    // The original expr before DeclRef resolution.
    Expr* originalExpr = nullptr;

    SLANG_UNREFLECTED
    // The scope in which to perform lookup
    Scope* scope = nullptr;
};

class VarExpr : public DeclRefExpr
{
    SLANG_AST_CLASS(VarExpr)
};

// An expression that references an overloaded set of declarations
// having the same name.
class OverloadedExpr : public Expr
{
    SLANG_AST_CLASS(OverloadedExpr)

    // The name that was looked up and found to be overloaded
    Name* name = nullptr;

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    Expr* base = nullptr;

    // The lookup result that was ambiguous
    LookupResult lookupResult2;
};

// An expression that references an overloaded set of declarations
// having the same name.
class OverloadedExpr2: public Expr
{
    SLANG_AST_CLASS(OverloadedExpr2)

    // Optional: the base expression is this overloaded result
    // arose from a member-reference expression.
    Expr* base = nullptr;

    // The lookup result that was ambiguous
    List<Expr*> candidiateExprs;
};

class LiteralExpr : public Expr
{
    SLANG_ABSTRACT_AST_CLASS(LiteralExpr)
    // The token that was used to express the literal. This can be
    // used to get the raw text of the literal, including any suffix.
    Token token;
};

class IntegerLiteralExpr : public LiteralExpr
{
    SLANG_AST_CLASS(IntegerLiteralExpr)

    IntegerLiteralValue value;
};

class FloatingPointLiteralExpr: public LiteralExpr
{
    SLANG_AST_CLASS(FloatingPointLiteralExpr)
    FloatingPointLiteralValue value;
};

class BoolLiteralExpr : public LiteralExpr
{
    SLANG_AST_CLASS(BoolLiteralExpr)
    bool value;
};

class NullPtrLiteralExpr : public LiteralExpr
{
    SLANG_AST_CLASS(NullPtrLiteralExpr)
};

class NoneLiteralExpr : public LiteralExpr
{
    SLANG_AST_CLASS(NoneLiteralExpr)
};

class StringLiteralExpr : public LiteralExpr
{
    SLANG_AST_CLASS(StringLiteralExpr)

    // TODO: consider storing the "segments" of the string
    // literal, in the case where multiple literals were
    //lined up at the lexer level, e.g.:
    //
    //      "first" "second" "third"
    //
    String value;
};

// An initializer list, e.g. `{ 1, 2, 3 }`
class InitializerListExpr : public Expr
{
    SLANG_AST_CLASS(InitializerListExpr)
    List<Expr*> args;
};

class GetArrayLengthExpr : public Expr
{
    SLANG_AST_CLASS(GetArrayLengthExpr)
    Expr* arrayExpr = nullptr;
};

// A base class for expressions with arguments
class ExprWithArgsBase : public Expr
{
    SLANG_ABSTRACT_AST_CLASS(ExprWithArgsBase)

    List<Expr*> arguments;
};

// An aggregate type constructor
class AggTypeCtorExpr : public ExprWithArgsBase
{
    SLANG_AST_CLASS(AggTypeCtorExpr)

    TypeExp base;
};


// A base expression being applied to arguments: covers
// both ordinary `()` function calls and `<>` generic application
class AppExprBase : public ExprWithArgsBase
{
    SLANG_ABSTRACT_AST_CLASS(AppExprBase)

    Expr* functionExpr = nullptr;

    // The original function expr before overload resolution.
    Expr* originalFunctionExpr = nullptr;

    // The source location of `(`, `)`, and `,` that marks the start/end of the application op and
    // each argument expr. This info is used by language server.
    List<SourceLoc> argumentDelimeterLocs;
};

class InvokeExpr: public AppExprBase
{
    SLANG_AST_CLASS(InvokeExpr)
};

enum class TryClauseType
{
    None,
    Standard, // Normal `try` clause
    Optional, // (Not implemented) `try?` clause that returns an optional value.
    Assert, // (Not implemented) `try!` clause that should always succeed and triggers runtime error if failed.
};

char const* getTryClauseTypeName(TryClauseType value);

class TryExpr : public Expr
{
    SLANG_AST_CLASS(TryExpr)

    Expr* base;

    TryClauseType tryClauseType = TryClauseType::Standard;

    // The scope of this expr.
    Scope* scope = nullptr;
};

class NewExpr : public InvokeExpr
{
    SLANG_AST_CLASS(NewExpr)
};

class OperatorExpr: public InvokeExpr
{
    SLANG_AST_CLASS(OperatorExpr)
};

class InfixExpr: public OperatorExpr
{
    SLANG_AST_CLASS(InfixExpr)
};
class PrefixExpr: public OperatorExpr
{
    SLANG_AST_CLASS(PrefixExpr)
};
class PostfixExpr: public OperatorExpr
{
    SLANG_AST_CLASS(PostfixExpr)
};

class IndexExpr: public Expr
{
    SLANG_AST_CLASS(IndexExpr)
    Expr* baseExpression;
    List<Expr*> indexExprs;

    // The source location of `(`, `)`, and `,` that marks the start/end of the application op and
    // each argument expr. This info is used by language server.
    List<SourceLoc> argumentDelimeterLocs;
};

class MemberExpr: public DeclRefExpr
{
    SLANG_AST_CLASS(MemberExpr)
    Expr* baseExpression = nullptr;
    SourceLoc memberOperatorLoc;
};

// Member looked up on a type, rather than a value
class StaticMemberExpr: public DeclRefExpr
{
    SLANG_AST_CLASS(StaticMemberExpr)
    Expr* baseExpression = nullptr;
    SourceLoc memberOperatorLoc;
};

struct MatrixCoord
{
    bool operator==(const MatrixCoord& rhs) const { return row == rhs.row && col == rhs.col; };
    bool operator!=(const MatrixCoord& rhs) const { return !(*this == rhs); };
    // Rows and columns are zero indexed 
    int row;
    int col;
};

class MatrixSwizzleExpr : public Expr
{
    SLANG_AST_CLASS(MatrixSwizzleExpr)
    Expr* base = nullptr;
    int elementCount;
    MatrixCoord elementCoords[4];
    SourceLoc memberOpLoc;
};

class SwizzleExpr: public Expr
{
    SLANG_AST_CLASS(SwizzleExpr)
    Expr* base = nullptr;
    int elementCount;
    int elementIndices[4];
    SourceLoc memberOpLoc;
};

// An operation to convert an l-value to a reference type.
class MakeRefExpr : public Expr
{
    SLANG_AST_CLASS(MakeRefExpr)
    Expr* base = nullptr;
};

// A dereference of a pointer or pointer-like type
class DerefExpr: public Expr
{
    SLANG_AST_CLASS(DerefExpr)
    Expr* base = nullptr;
};

// Any operation that performs type-casting
class TypeCastExpr: public InvokeExpr
{
    SLANG_AST_CLASS(TypeCastExpr)
//    TypeExp TargetType;
//    Expr* Expression = nullptr;
};

// An explicit type-cast that appear in the user's code with `(type) expr` syntax
class ExplicitCastExpr: public TypeCastExpr
{
    SLANG_AST_CLASS(ExplicitCastExpr)
};

// An implicit type-cast inserted during semantic checking
class ImplicitCastExpr : public TypeCastExpr
{
    SLANG_AST_CLASS(ImplicitCastExpr)
};

class LValueImplicitCastExpr : public TypeCastExpr
{
    SLANG_AST_CLASS(LValueImplicitCastExpr)

    explicit LValueImplicitCastExpr(const TypeCastExpr& rhs) :Super(rhs) {}
};

// To work around situations like int += uint
// where we want to allow an LValue to work with an implicit cast.
// The argument being cast *must* be an LValue.
class OutImplicitCastExpr : public LValueImplicitCastExpr
{
    SLANG_AST_CLASS(OutImplicitCastExpr)

        /// Allow explict construction from any TypeCastExpr
    explicit OutImplicitCastExpr(const TypeCastExpr& rhs) :Super(rhs) {}
};

class InOutImplicitCastExpr : public LValueImplicitCastExpr
{
    SLANG_AST_CLASS(InOutImplicitCastExpr)

    /// Allow explict construction from any TypeCastExpr
    explicit InOutImplicitCastExpr(const TypeCastExpr& rhs) :Super(rhs) {}
};

    /// A cast of a value to a super-type of its type.
    ///
    /// The type being cast to is stored as this expression's `type`.
    ///
class CastToSuperTypeExpr: public Expr
{
    SLANG_AST_CLASS(CastToSuperTypeExpr)

    /// The value being cast to a super type
    ///
    /// The type being cast from is `valueArg->type`.
    ///
    Expr* valueArg = nullptr;

    /// A witness showing that `valueArg`'s type is a sub-type of this expression's `type`   
    Val* witnessArg = nullptr;
};

    /// A `value is Type` expression that evaluates to `true` if type of `value` is a sub-type of
    /// `Type`.
class IsTypeExpr : public Expr
{
    SLANG_AST_CLASS(IsTypeExpr)

    Expr* value = nullptr;
    TypeExp typeExpr;

    // A witness showing that `typeExpr.type` is a subtype of `typeof(value)`.
    Val* witnessArg = nullptr;

    // non-null if evaluates to a constant.
    BoolLiteralExpr* constantVal = nullptr;
};

    /// A `value as Type` expression that casts `value` to `Type` within type hierarchy.
    /// The result is undefined if `value` is not `Type`.
class AsTypeExpr : public Expr
{
    SLANG_AST_CLASS(AsTypeExpr)

    Expr* value = nullptr;
    Expr* typeExpr = nullptr;

    // A witness showing that `typeExpr` is a subtype of `typeof(value)`.
    Val* witnessArg = nullptr;

};

class SizeOfLikeExpr : public Expr
{
    SLANG_AST_CLASS(SizeOfLikeExpr);

    // Set during the parse, could be an expression, a variable or a type
    Expr* value = nullptr;

    // The type the size/alignment needs to operate on. Set during traversal of SemanticsExprVisitor
    Type* sizedType = nullptr;
};

class SizeOfExpr : public SizeOfLikeExpr
{
    SLANG_AST_CLASS(SizeOfExpr);
};

class AlignOfExpr : public SizeOfLikeExpr
{
    SLANG_AST_CLASS(AlignOfExpr);
};

class MakeOptionalExpr : public Expr
{
    SLANG_AST_CLASS(MakeOptionalExpr)

        // If `value` is null, this constructs an `Optional<T>` that doesn't have a value.
    Expr* value = nullptr;
    Expr* typeExpr = nullptr;
};

    /// A cast of a value to the same type, with different modifiers.
    ///
    /// The type being cast to is stored as this expression's `type`.
    ///
class ModifierCastExpr : public Expr
{
    SLANG_AST_CLASS(ModifierCastExpr)

    /// The value being cast.
    ///
    /// The type being cast from is `valueArg->type`.
    ///
    Expr* valueArg = nullptr;
};

class SelectExpr: public OperatorExpr
{
    SLANG_AST_CLASS(SelectExpr)
};

class GenericAppExpr: public AppExprBase
{
    SLANG_AST_CLASS(GenericAppExpr)
};

// An expression representing re-use of the syntax for a type in more
// than once conceptually-distinct declaration
class SharedTypeExpr: public Expr
{
    SLANG_AST_CLASS(SharedTypeExpr)
    // The underlying type expression that we want to share
    TypeExp base;
};

class AssignExpr: public Expr
{
    SLANG_AST_CLASS(AssignExpr)
    Expr* left = nullptr;
    Expr* right = nullptr;
};

// Just an expression inside parentheses `(exp)`
//
// We keep this around explicitly to be sure we don't lose any structure
// when we do rewriter stuff.
class ParenExpr: public Expr
{
    SLANG_AST_CLASS(ParenExpr)
    Expr* base = nullptr;
};

// An object-oriented `this` expression, used to
// refer to the current instance of an enclosing type.
class ThisExpr: public Expr
{
    SLANG_AST_CLASS(ThisExpr)

    SLANG_UNREFLECTED
    Scope* scope = nullptr;
};

// Represent a reference to the virtual __return_val object holding the return value of
// functions whose result type is non-copyable.
class ReturnValExpr : public Expr
{
    SLANG_AST_CLASS(ReturnValExpr)

    SLANG_UNREFLECTED
    Scope* scope = nullptr;
};

// An expression that binds a temporary variable in a local expression context
class LetExpr: public Expr
{
    SLANG_AST_CLASS(LetExpr)
    VarDecl* decl = nullptr;
    Expr* body = nullptr;
};

class ExtractExistentialValueExpr: public Expr
{
    SLANG_AST_CLASS(ExtractExistentialValueExpr)
    DeclRef<VarDeclBase> declRef;
    Expr* originalExpr;
};

class OpenRefExpr : public Expr
{
    SLANG_AST_CLASS(OpenRefExpr)

    Expr* innerExpr = nullptr;
};

    /// Base class for higher-order function application
    /// Eg: foo(fn) where fn is a function expression.
    ///
class HigherOrderInvokeExpr : public Expr
{
    SLANG_ABSTRACT_AST_CLASS(HigherOrderInvokeExpr)
    Expr* baseFunction;
    List<Name*> newParameterNames;
};

class PrimalSubstituteExpr : public HigherOrderInvokeExpr
{
    SLANG_AST_CLASS(PrimalSubstituteExpr)
};

class DifferentiateExpr : public HigherOrderInvokeExpr
{
    SLANG_ABSTRACT_AST_CLASS(DifferentiateExpr)
};
    /// An expression of the form `__fwd_diff(fn)` to access the 
    /// forward-mode derivative version of the function `fn`
    ///
class ForwardDifferentiateExpr: public DifferentiateExpr
{
    SLANG_AST_CLASS(ForwardDifferentiateExpr)
};

    /// An expression of the form `__bwd_diff(fn)` to access the 
    /// forward-mode derivative version of the function `fn`
    ///
class BackwardDifferentiateExpr: public DifferentiateExpr
{
    SLANG_AST_CLASS(BackwardDifferentiateExpr)
};

    /// An expression of the form `__dispatch_kernel(fn, threadGroupSize, dispatchSize)` to
    /// dispatch a compute kernel from host.
    ///
class DispatchKernelExpr : public HigherOrderInvokeExpr
{
    SLANG_AST_CLASS(DispatchKernelExpr)
    Expr* threadGroupSize;
    Expr* dispatchSize;
};

    /// An express to mark its inner expression as an intended non-differential call.
class TreatAsDifferentiableExpr : public Expr
{
    SLANG_AST_CLASS(TreatAsDifferentiableExpr)

    Expr* innerExpr;
    Scope* scope;
    
    enum Flavor 
    {
        /// Represents a no_diff wrapper over
        /// a non-differentiable method.
        /// i.e. no_diff(fn(...))
        /// 
        NoDiff,

        /// Represents a call to a method that
        /// is either marked differentiable, or has
        /// a user-defined derivative in scope.
        /// 
        Differentiable
    };

    Flavor flavor;
};

    /// A type expression of the form `This`
    ///
    /// Refers to the type of `this` in the current context.
    ///
class ThisTypeExpr: public Expr
{
    SLANG_AST_CLASS(ThisTypeExpr)

    SLANG_UNREFLECTED
    Scope* scope = nullptr;
};

    /// A type expression of the form `Left & Right`.
class AndTypeExpr : public Expr
{
    SLANG_AST_CLASS(AndTypeExpr);

    TypeExp left;
    TypeExp right;
};

    /// A type exprssion that applies one or more modifiers to another type
class ModifiedTypeExpr : public Expr
{
    SLANG_AST_CLASS(ModifiedTypeExpr);

    Modifiers modifiers;
    TypeExp base;
};

    /// A type expression that rrepresents a pointer type, e.g. T*
class PointerTypeExpr : public Expr
{
    SLANG_AST_CLASS(PointerTypeExpr);

    TypeExp base;
};

    /// A type expression that represents a function type, e.g. (bool, int) -> float
class FuncTypeExpr : public Expr
{
    SLANG_AST_CLASS(FuncTypeExpr);

    List<TypeExp> parameters;
    TypeExp result;
};

class TupleTypeExpr : public Expr
{
    SLANG_AST_CLASS(TupleTypeExpr);

    List<TypeExp> members;
};

    /// An expression that applies a generic to arguments for some,
    /// but not all, of its explicit parameters.
    ///
class PartiallyAppliedGenericExpr : public Expr
{
    SLANG_AST_CLASS(PartiallyAppliedGenericExpr);

public:
    Expr* originalExpr = nullptr;

        /// The generic being applied
    DeclRef<GenericDecl> baseGenericDeclRef;

        /// A substitution that includes the generic arguments known so far
    List<Val*> knownGenericArgs;
};

class SPIRVAsmOperand
{
    SLANG_VALUE_CLASS(SPIRVAsmOperand);

public:
    enum Flavor
    {
        Literal, // No prefix
        Id, // Prefixed with %
        ResultMarker, // "result" (without quotes)
        NamedValue, // Any other identifier
        SlangValue,
        SlangValueAddr,
        SlangType,
        SampledType, // __sampledType(T), this becomes a 4 vector of the component type of T
        ImageType, // __imageType(texture), returns the equivalaent OpTypeImage of a given texture typed value.
        SampledImageType, // __sampledImageType(texture), returns the equivalent OpTypeSampledImage of a given texture typed value.
        TruncateMarker, // __truncate, an invented instruction which coerces to the result type by truncating the element count
        EntryPoint, // __entryPoint, a placeholder for the id of a referencing entryPoint.
        BuiltinVar,
        GLSL450Set,
        NonSemanticDebugPrintfExtSet,
    };

    // The flavour and token describes how this was parsed
    Flavor flavor;
    // The single token this came from
    Token token;

    // If this was a SlangValue or SlangValueAddr or SlangType, then we also
    // store the expression, which should be a single VarExpr because we only
    // parse single idents at the moment
    Expr* expr = nullptr;

    // If this is part of a bitwise or expression, this will point to the
    // remaining operands values in such an expression must be of flavour
    // Literal or NamedValue
    List<SPIRVAsmOperand> bitwiseOrWith = List<SPIRVAsmOperand>();

    // If this is a named value then we calculate the value here during
    // checking. If this is an opcode, then the parser will populate this too
    // (or set it to 0xffffffff);
    SpvWord knownValue = 0xffffffff;
    // Although this might be a constant in the source we should actually pass
    // it as an id created with OpConstant
    bool wrapInId = false;

    // Once we've checked things, the SlangType and BuiltinVar flavour operands
    // will have this type populated.
    TypeExp type = TypeExp();
};

class SPIRVAsmInst
{
    SLANG_VALUE_CLASS(SPIRVAsmInst);

public:
    SPIRVAsmOperand opcode;
    List<SPIRVAsmOperand> operands;
};

class SPIRVAsmExpr : public Expr
{
    SLANG_AST_CLASS(SPIRVAsmExpr);

public:
    List<SPIRVAsmInst> insts;
};

} // namespace Slang
