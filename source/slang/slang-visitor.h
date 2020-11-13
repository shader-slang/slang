// slang-visitor.h
#ifndef SLANG_VISITOR_H_INCLUDED
#define SLANG_VISITOR_H_INCLUDED

// This file defines the basic "Visitor" pattern for doing dispatch
// over the various categories of syntax node.

#include "slang-syntax.h"

#include "slang-generated-ast-macro.h"

namespace Slang {

// Macros to generate from ast-generated-macro file the vistors

// Only runs 'param' macro if the marker is NONE (ie not ABSTRACT here)
#define SLANG_CLASS_ONLY_ABSTRACT_AST(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)
#define SLANG_CLASS_ONLY_AST(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) param(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)

#define SLANG_CLASS_ONLY(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) SLANG_CLASS_ONLY_##MARKER(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)

// Dispatch decl
#define SLANG_VISITOR_DISPATCH_DECL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

// Dispatch

#define SLANG_VISITOR_DISPATCH_RESULT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override  { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#define SLANG_VISITOR_DISPATCH_VOID_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    virtual void dispatch_##NAME(NAME* obj, void*) override { ((Derived*) this)->visit##NAME(obj); }

// Visitor with and without result

#define SLANG_VISITOR_RESULT_VISIT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    Result visit##NAME(NAME* obj) { return ((Derived*) this)->visit##SUPER(obj); } \

#define SLANG_VISITOR_VOID_VISIT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    void visit##NAME(NAME* obj) { ((Derived*) this)->visit##SUPER(obj); }

// Args

#define SLANG_VISITOR_DISPATCH_ARG_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    virtual void dispatch_##NAME(NAME* obj, void* arg) override { ((Derived*) this)->visit##NAME(obj, *(Arg*)arg); }

#define SLANG_VISITOR_VOID_VISIT_ARG_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    void visit##NAME(NAME* obj, Arg const& arg) { ((Derived*) this)->visit##SUPER(obj, arg); }
//
// type Visitors
//

struct ITypeVisitor
{
    SLANG_CHILDREN_ASTNode_Type(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_DECL)
};

// Suppress VS2017 Unreachable code warning
#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable:4702)
#endif

template<typename Derived, typename Result = void, typename Base = ITypeVisitor>
struct TypeVisitor : Base
{
    Result dispatch(Type* type)
    {
        Result result;
        type->accept(this, &result);
        return result;
    }

    Result dispatchType(Type* type)
    {
        Result result;
        type->accept(this, &result);
        return result;
    }

    SLANG_CHILDREN_ASTNode_Type(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_RESULT_IMPL)
    SLANG_CHILDREN_ASTNode_Type(SLANG_VISITOR_RESULT_VISIT_IMPL, _)
};

template<typename Derived, typename Base>
struct TypeVisitor<Derived,void,Base> : Base
{
    void dispatch(Type* type)
    {
        type->accept(this, 0);
    }

    void dispatchType(Type* type)
    {
        type->accept(this, 0);
    }

    SLANG_CHILDREN_ASTNode_Type(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_VOID_IMPL)
    SLANG_CHILDREN_ASTNode_Type(SLANG_VISITOR_VOID_VISIT_IMPL, _)
};

template<typename Derived, typename Arg, typename Base = ITypeVisitor>
struct TypeVisitorWithArg : Base
{
    void dispatch(Type* type, Arg const& arg)
    {
        type->accept(this, (void*)&arg);
    }

    SLANG_CHILDREN_ASTNode_Type(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_ARG_IMPL)
    SLANG_CHILDREN_ASTNode_Type(SLANG_VISITOR_VOID_VISIT_ARG_IMPL, _)
};

//
// Expression Visitors
//

struct IExprVisitor
{
    SLANG_CHILDREN_ASTNode_Expr(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_DECL)
};

template<typename Derived, typename Result = void>
struct ExprVisitor : IExprVisitor
{
    Result dispatch(Expr* expr)
    {
        Result result;
        expr->accept(this, &result);
        return result;
    }

    SLANG_CHILDREN_ASTNode_Expr(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_RESULT_IMPL)
    SLANG_CHILDREN_ASTNode_Expr(SLANG_VISITOR_RESULT_VISIT_IMPL, _)
};

template<typename Derived>
struct ExprVisitor<Derived,void> : IExprVisitor
{
    void dispatch(Expr* expr)
    {
        expr->accept(this, 0);
    }

    SLANG_CHILDREN_ASTNode_Expr(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_VOID_IMPL)
    SLANG_CHILDREN_ASTNode_Expr(SLANG_VISITOR_VOID_VISIT_IMPL, _)
};

template<typename Derived, typename Arg>
struct ExprVisitorWithArg : IExprVisitor
{
    void dispatch(Expr* obj, Arg const& arg)
    {
        obj->accept(this, (void*)&arg);
    }

    SLANG_CHILDREN_ASTNode_Expr(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_ARG_IMPL)
    SLANG_CHILDREN_ASTNode_Expr(SLANG_VISITOR_VOID_VISIT_ARG_IMPL, _)
};

//
// Statement Visitors
//

struct IStmtVisitor
{
    SLANG_CHILDREN_ASTNode_Stmt(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_DECL)
};

template<typename Derived, typename Result = void>
struct StmtVisitor : IStmtVisitor
{
    Result dispatch(Stmt* stmt)
    {
        Result result;
        stmt->accept(this, &result);
        return result;
    }

    SLANG_CHILDREN_ASTNode_Stmt(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_RESULT_IMPL)
    SLANG_CHILDREN_ASTNode_Stmt(SLANG_VISITOR_RESULT_VISIT_IMPL, _)
};

template<typename Derived>
struct StmtVisitor<Derived,void> : IStmtVisitor
{
    void dispatch(Stmt* stmt)
    {
        stmt->accept(this, 0);
    }

    SLANG_CHILDREN_ASTNode_Stmt(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_VOID_IMPL)
    SLANG_CHILDREN_ASTNode_Stmt(SLANG_VISITOR_VOID_VISIT_IMPL, _)
};

//
// Declaration Visitors
//

struct IDeclVisitor
{
    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_DECL)
};

template<typename Derived, typename Result = void>
struct DeclVisitor : IDeclVisitor
{
    Result dispatch(DeclBase* decl)
    {
        Result result;
        decl->accept(this, &result);
        return result;
    }

    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_RESULT_IMPL)
    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_VISITOR_RESULT_VISIT_IMPL, _)
};

template<typename Derived>
struct DeclVisitor<Derived,void> : IDeclVisitor
{
    void dispatch(DeclBase* decl)
    {
        decl->accept(this, 0);
    }

    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_VOID_IMPL)
    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_VISITOR_VOID_VISIT_IMPL, _)
};

template<typename Derived, typename Arg>
struct DeclVisitorWithArg : IDeclVisitor
{
    void dispatch(DeclBase* obj, Arg const& arg)
    {
        obj->accept(this, (void*)&arg);
    }

    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_ARG_IMPL)
    SLANG_CHILDREN_ASTNode_DeclBase(SLANG_VISITOR_VOID_VISIT_ARG_IMPL, _)
};


//
// Modifier Visitors
//

struct IModifierVisitor
{
    SLANG_CHILDREN_ASTNode_Modifier(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_DECL)
};

template<typename Derived, typename Result = void>
struct ModifierVisitor : IModifierVisitor
{
    Result dispatch(Modifier* modifier)
    {
        Result result;
        modifier->accept(this, &result);
        return result;
    }

    SLANG_CHILDREN_ASTNode_Modifier(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_RESULT_IMPL)
    SLANG_CHILDREN_ASTNode_Modifier(SLANG_VISITOR_RESULT_VISIT_IMPL, _)
};

template<typename Derived>
struct ModifierVisitor<Derived, void> : IModifierVisitor
{
    void dispatch(Modifier* modifier)
    {
        modifier->accept(this, 0);
    }

    SLANG_CHILDREN_ASTNode_Modifier(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_VOID_IMPL)
    SLANG_CHILDREN_ASTNode_Modifier(SLANG_VISITOR_VOID_VISIT_IMPL, _)
};

//
// Val Visitors
//

struct IValVisitor : ITypeVisitor
{
    SLANG_CHILDREN_ASTNode_Val(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_DECL)
};

template<typename Derived, typename Result = void, typename TypeResult = void>
struct ValVisitor : TypeVisitor<Derived, TypeResult, IValVisitor>
{
    Result dispatch(Val* val)
    {
        Result result;
        val->accept(this, &result);
        return result;
    }

    SLANG_CHILDREN_ASTNode_Val(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_RESULT_IMPL)
    SLANG_CHILDREN_ASTNode_Val(SLANG_VISITOR_RESULT_VISIT_IMPL, _)
};

template<typename Derived>
struct ValVisitor<Derived, void, void> : TypeVisitor<Derived, void, IValVisitor>
{
    void dispatch(Val* val)
    {
        val->accept(this, 0);
    }

    SLANG_CHILDREN_ASTNode_Val(SLANG_CLASS_ONLY, SLANG_VISITOR_DISPATCH_VOID_IMPL)
    SLANG_CHILDREN_ASTNode_Val(SLANG_VISITOR_VOID_VISIT_IMPL, _)
};

// Re-activate VS2017 warning settings
#ifdef _MSC_VER
#    pragma warning(pop)
#endif
}

#endif
