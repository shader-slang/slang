// slang-visitor.h
#ifndef SLANG_VISITOR_H_INCLUDED
#define SLANG_VISITOR_H_INCLUDED

// This file defines the basic "Visitor" pattern for doing dispatch
// over the various categories of syntax node.

#include "slang-ast-dispatch.h"
#include "slang-ast-forward-declarations.h"
#include "slang-syntax.h"

namespace Slang
{

// Dispatch

#if 0 // FIDDLE TEMPLATE:
%function SLANG_VISITOR_DISPATCH_RESULT_IMPL(baseType)
%  for _,T in ipairs(baseType.subclasses) do
%    if not T.isAbstract then
    Result _dispatchImpl($T* obj)
    {
        return ((Derived*)this)->visit$T(obj);
    }
%    end
%  end
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END

// Visitor with and without result

#if 0 // FIDDLE TEMPLATE:
%function SLANG_VISITOR_VISIT_RESULT_IMPL(baseType)
%  for _,T in ipairs(baseType.subclasses) do
    Result visit$T($T* obj)
    {
        return ((Derived*)this)->visit$(T.directSuperClass)(obj);
    }
%  end
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 1
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END

// Args

#if 0 // FIDDLE TEMPLATE:
%function SLANG_VISITOR_DISPATCH_ARG_IMPL(baseType)
%  for _, T in ipairs(baseType.subclasses) do
%    if not T.isAbstract then
virtual void _dispatchImpl($T* obj, Arg const& arg)
{
    ((Derived*)this)->visit$T(obj, arg);
}
%    end
%  end
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 2
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END

#if 0 // FIDDLE TEMPLATE:
%function SLANG_VISITOR_VISIT_ARG_IMPL(baseType)
% for _, T in ipairs(baseType.subclasses) do
void visit$T($T* obj, Arg const& arg)
{
    ((Derived*)this)->visit$(T.directSuperClass)(obj, arg);
}
%  end
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 3
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END

//
// type Visitors
//

// Suppress VS2017 Unreachable code warning
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

template<typename Derived, typename Result = void>
struct TypeVisitor
{
    Result dispatch(Type* type)
    {
        return ASTNodeDispatcher<Type, Result>::dispatch(
            type,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

    Result dispatchType(Type* type)
    {
        return ASTNodeDispatcher<Type, Result>::dispatch(
            type,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

#if 0 // FIDDLE TEMPLATE:
        % SLANG_VISITOR_DISPATCH_RESULT_IMPL(Slang.Type)
        % SLANG_VISITOR_VISIT_RESULT_IMPL(Slang.Type)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 4
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

template<typename Derived, typename Arg>
struct TypeVisitorWithArg
{
    void dispatch(Type* type, Arg const& arg)
    {
        ASTNodeDispatcher<Type, void>::dispatch(type, [&](auto obj) { _dispatchImpl(obj, arg); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_ARG_IMPL(Slang.Type)
    % SLANG_VISITOR_VISIT_ARG_IMPL(Slang.Type)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 5
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

//
// Expression Visitors
//

template<typename Derived, typename Result = void>
struct ExprVisitor
{
    Result dispatch(Expr* expr)
    {
        return ASTNodeDispatcher<Expr, Result>::dispatch(
            expr,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_RESULT_IMPL(Slang.Expr)
    % SLANG_VISITOR_VISIT_RESULT_IMPL(Slang.Expr)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 6
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

template<typename Derived, typename Arg>
struct ExprVisitorWithArg
{
    void dispatch(Expr* expr, Arg const& arg)
    {
        ASTNodeDispatcher<Expr, void>::dispatch(expr, [&](auto obj) { _dispatchImpl(obj, arg); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_ARG_IMPL(Slang.Expr)
    % SLANG_VISITOR_VISIT_ARG_IMPL(Slang.Expr)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 7
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

/// A CRTP expression visitor that recursively walks and **mutates** child expression
/// pointers. Derived classes override specific `visitXXX` methods (returning the
/// replacement `Expr*`) and rely on the base for plain recursive traversal of all
/// other node types.
///
/// The default implementation for each expression type recurses into its child
/// expressions, reassigning each child pointer with the dispatch result, and
/// returns the (possibly-modified) expression. Leaf expressions simply return
/// themselves unchanged.
///
/// Inherits dispatch machinery from `ExprVisitor<Derived, Expr*>`.
/// Modeled after `ASTIteratorExprVisitor` in `slang-ast-iterator.h`.
template<typename Derived>
struct ModifyingExprVisitor : ExprVisitor<Derived, Expr*>
{
    Expr* dispatchIfNotNull(Expr* expr) { return expr ? this->dispatch(expr) : nullptr; }

    // --- Leaf / default ---
    Expr* visitExpr(Expr* e) { return e; }

    // --- Member / field access ---
    Expr* visitMemberExpr(MemberExpr* e)
    {
        e->baseExpression = dispatchIfNotNull(e->baseExpression);
        return e;
    }
    Expr* visitStaticMemberExpr(StaticMemberExpr* e)
    {
        e->baseExpression = dispatchIfNotNull(e->baseExpression);
        return e;
    }

    // --- Subscript ---
    Expr* visitIndexExpr(IndexExpr* e)
    {
        e->baseExpression = dispatchIfNotNull(e->baseExpression);
        for (auto& arg : e->indexExprs)
            arg = dispatchIfNotNull(arg);
        return e;
    }

    // --- Call / generic application ---
    // AppExprBase is the common base for InvokeExpr, GenericAppExpr, TypeCastExpr,
    // OperatorExpr, etc. All share functionExpr + arguments.
    Expr* visitAppExprBase(AppExprBase* e)
    {
        e->functionExpr = dispatchIfNotNull(e->functionExpr);
        for (auto& arg : e->arguments)
            arg = dispatchIfNotNull(arg);
        return e;
    }

    // --- Unary-like ---
    Expr* visitDerefExpr(DerefExpr* e)
    {
        e->base = dispatchIfNotNull(e->base);
        return e;
    }
    Expr* visitSwizzleExpr(SwizzleExpr* e)
    {
        e->base = dispatchIfNotNull(e->base);
        return e;
    }
    Expr* visitMatrixSwizzleExpr(MatrixSwizzleExpr* e)
    {
        e->base = dispatchIfNotNull(e->base);
        return e;
    }
    Expr* visitOpenRefExpr(OpenRefExpr* e)
    {
        e->innerExpr = dispatchIfNotNull(e->innerExpr);
        return e;
    }
    Expr* visitParenExpr(ParenExpr* e)
    {
        e->base = dispatchIfNotNull(e->base);
        return e;
    }
    Expr* visitTryExpr(TryExpr* e)
    {
        e->base = dispatchIfNotNull(e->base);
        return e;
    }
    Expr* visitHigherOrderInvokeExpr(HigherOrderInvokeExpr* e)
    {
        e->baseFunction = dispatchIfNotNull(e->baseFunction);
        return e;
    }
    Expr* visitTreatAsDifferentiableExpr(TreatAsDifferentiableExpr* e)
    {
        e->innerExpr = dispatchIfNotNull(e->innerExpr);
        return e;
    }

    // --- Binary ---
    Expr* visitAssignExpr(AssignExpr* e)
    {
        e->left = dispatchIfNotNull(e->left);
        e->right = dispatchIfNotNull(e->right);
        return e;
    }

    // --- Let bindings ---
    Expr* visitLetExpr(LetExpr* e)
    {
        if (e->decl && e->decl->initExpr)
            e->decl->initExpr = dispatchIfNotNull(e->decl->initExpr);
        e->body = dispatchIfNotNull(e->body);
        return e;
    }

    // --- Cast-like ---
    Expr* visitCastToSuperTypeExpr(CastToSuperTypeExpr* e)
    {
        e->valueArg = dispatchIfNotNull(e->valueArg);
        return e;
    }
    Expr* visitModifierCastExpr(ModifierCastExpr* e)
    {
        e->valueArg = dispatchIfNotNull(e->valueArg);
        return e;
    }

    // --- Overloaded exprs ---
    Expr* visitOverloadedExpr(OverloadedExpr* e)
    {
        e->base = dispatchIfNotNull(e->base);
        return e;
    }
    Expr* visitOverloadedExpr2(OverloadedExpr2* e)
    {
        e->base = dispatchIfNotNull(e->base);
        for (auto& candidate : e->candidateExprs)
            candidate = dispatchIfNotNull(candidate);
        return e;
    }

    // --- Tuple / initializer list ---
    Expr* visitTupleExpr(TupleExpr* e)
    {
        for (auto& elem : e->elements)
            elem = dispatchIfNotNull(elem);
        return e;
    }
    Expr* visitInitializerListExpr(InitializerListExpr* e)
    {
        for (auto& arg : e->args)
            arg = dispatchIfNotNull(arg);
        return e;
    }

    // --- Type check / cast exprs ---
    Expr* visitIsTypeExpr(IsTypeExpr* e)
    {
        e->value = dispatchIfNotNull(e->value);
        return e;
    }
    Expr* visitAsTypeExpr(AsTypeExpr* e)
    {
        e->value = dispatchIfNotNull(e->value);
        e->typeExpr = dispatchIfNotNull(e->typeExpr);
        return e;
    }
    Expr* visitMakeOptionalExpr(MakeOptionalExpr* e)
    {
        e->value = dispatchIfNotNull(e->value);
        e->typeExpr = dispatchIfNotNull(e->typeExpr);
        return e;
    }

    // --- Existential ---
    Expr* visitExtractExistentialValueExpr(ExtractExistentialValueExpr* e) { return e; }

    // --- Pack / expand ---
    Expr* visitPackExpr(PackExpr* e)
    {
        for (auto& arg : e->args)
            arg = dispatchIfNotNull(arg);
        return e;
    }
    Expr* visitExpandExpr(ExpandExpr* e)
    {
        e->baseExpr = dispatchIfNotNull(e->baseExpr);
        return e;
    }
    Expr* visitEachExpr(EachExpr* e)
    {
        e->baseExpr = dispatchIfNotNull(e->baseExpr);
        return e;
    }

    // --- Misc ---
    Expr* visitPartiallyAppliedGenericExpr(PartiallyAppliedGenericExpr* e)
    {
        e->originalExpr = dispatchIfNotNull(e->originalExpr);
        return e;
    }
    Expr* visitSPIRVAsmExpr(SPIRVAsmExpr* e) { return e; }
    Expr* visitAggTypeCtorExpr(AggTypeCtorExpr* e)
    {
        for (auto& arg : e->arguments)
            arg = dispatchIfNotNull(arg);
        return e;
    }
    Expr* visitGetArrayLengthExpr(GetArrayLengthExpr* e)
    {
        e->arrayExpr = dispatchIfNotNull(e->arrayExpr);
        return e;
    }
    Expr* visitAddressOfExpr(AddressOfExpr* e)
    {
        e->arg = dispatchIfNotNull(e->arg);
        return e;
    }
};

//
// Statement Visitors
//

template<typename Derived, typename Result = void>
struct StmtVisitor
{
    Result dispatch(Stmt* stmt)
    {
        return ASTNodeDispatcher<Stmt, Result>::dispatch(
            stmt,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_RESULT_IMPL(Slang.Stmt)
    % SLANG_VISITOR_VISIT_RESULT_IMPL(Slang.Stmt)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 8
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

//
// Declaration Visitors
//

template<typename Derived, typename Result = void>
struct DeclVisitor
{
    Result dispatch(DeclBase* decl)
    {
        return ASTNodeDispatcher<DeclBase, Result>::dispatch(
            decl,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_RESULT_IMPL(Slang.DeclBase)
    % SLANG_VISITOR_VISIT_RESULT_IMPL(Slang.DeclBase)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 9
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

template<typename Derived, typename Arg>
struct DeclVisitorWithArg
{
    void dispatch(DeclBase* decl, Arg const& arg)
    {
        ASTNodeDispatcher<Expr, void>::dispatch(decl, [&](auto obj) { _dispatchImpl(obj, arg); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_ARG_IMPL(Slang.DeclBase)
    % SLANG_VISITOR_VISIT_ARG_IMPL(Slang.DeclBase)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 10
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};


//
// Modifier Visitors
//

template<typename Derived, typename Result = void>
struct ModifierVisitor
{
    Result dispatch(Modifier* modifier)
    {
        return ASTNodeDispatcher<Modifier, Result>::dispatch(
            modifier,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_RESULT_IMPL(Slang.Modifier)
    % SLANG_VISITOR_VISIT_RESULT_IMPL(Slang.Modifier)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 11
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

//
// Val Visitors
//

template<typename Derived, typename Result = void, typename TypeResult = void>
struct ValVisitor : TypeVisitor<Derived, TypeResult>
{
    Result dispatch(Val* val)
    {
        return ASTNodeDispatcher<Val, Result>::dispatch(
            val,
            [&](auto obj) { return _dispatchImpl(obj); });
    }

#if 0 // FIDDLE TEMPLATE:
    % SLANG_VISITOR_DISPATCH_RESULT_IMPL(Slang.Val)
    % SLANG_VISITOR_VISIT_RESULT_IMPL(Slang.Val)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 12
#include "slang-visitor.h.fiddle"
#endif // FIDDLE END
};

// Re-activate VS2017 warning settings
#ifdef _MSC_VER
#pragma warning(pop)
#endif
} // namespace Slang

#endif
