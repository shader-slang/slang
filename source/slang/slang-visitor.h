// slang-visitor.h
#ifndef SLANG_VISITOR_H_INCLUDED
#define SLANG_VISITOR_H_INCLUDED

// This file defines the basic "Visitor" pattern for doing dispatch
// over the various categories of syntax node.

#include "slang-syntax.h"

#include "slang-ast-generated-macro.h"

namespace Slang {

//
// type Visitors
//

struct ITypeVisitor
{
    SLANG_DERIVED_ASTNode_Type(SLANG_VISITOR_DISPATCH, _)
};

#define SLANG_VISITOR_DISPATCH_IMPL(NAME, SUPER, STYLE, NODE_STYLE, PARAM) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#define SLANG_VISIT_IMPL(NAME, SUPER, STYLE, NODE_STYLE, PARAM) \
    Result visit##NAME(NAME* obj) { return ((Derived*) this)->visit##SUPER(obj); }

#define SLANG_VISITOR_DISPATCH_WITH_ARG(NAME, SUPER, STYLE, NODE_STYLE, PARAM) \
    virtual void dispatch_##NAME(NAME* obj, void* arg) override { ((Derived*) this)->visit##NAME(obj, *(Arg*)arg); }

#define SLANG_VISIT_WITH_ARG(NAME, SUPER, STYLE, NODE_STYLE, PARAM) \
    void visit##NAME(NAME* obj, Arg const& arg) { ((Derived*) this)->visit##SUPER(obj, arg); }

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

    SLANG_DERIVED_ASTNode_Type(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Type(SLANG_VISIT_IMPL, _)
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

    SLANG_DERIVED_ASTNode_Type(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Type(SLANG_VISIT_IMPL, _)
};



template<typename Derived, typename Arg, typename Base = ITypeVisitor>
struct TypeVisitorWithArg : Base
{
    void dispatch(Type* type, Arg const& arg)
    {
        type->accept(this, (void*)&arg);
    }

    SLANG_DERIVED_ASTNode_Type(SLANG_VISITOR_DISPATCH_WITH_ARG, _)
    SLANG_DERIVED_ASTNode_Type(SLANG_VISIT_WITH_ARG, _)
};

//
// Expression Visitors
//

struct IExprVisitor
{
    SLANG_DERIVED_ASTNode_Expr(SLANG_VISITOR_DISPATCH, _)
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

    SLANG_DERIVED_ASTNode_Expr(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Expr(SLANG_VISIT_IMPL, _)
};

template<typename Derived>
struct ExprVisitor<Derived,void> : IExprVisitor
{
    void dispatch(Expr* expr)
    {
        expr->accept(this, 0);
    }

    SLANG_DERIVED_ASTNode_Expr(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Expr(SLANG_VISIT_IMPL, _)
};

template<typename Derived, typename Arg>
struct ExprVisitorWithArg : IExprVisitor
{
    void dispatch(Expr* obj, Arg const& arg)
    {
        obj->accept(this, (void*)&arg);
    }

    SLANG_DERIVED_ASTNode_Expr(SLANG_VISITOR_DISPATCH_WITH_ARG, _)
    SLANG_DERIVED_ASTNode_Expr(SLANG_VISIT_WITH_ARG, _)
};

//
// Statement Visitors
//

struct IStmtVisitor
{
    SLANG_DERIVED_ASTNode_Stmt(SLANG_VISITOR_DISPATCH, _)
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

    SLANG_DERIVED_ASTNode_Stmt(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Stmt(SLANG_VISIT_IMPL, _)
};

template<typename Derived>
struct StmtVisitor<Derived,void> : IStmtVisitor
{
    void dispatch(Stmt* stmt)
    {
        stmt->accept(this, 0);
    }

    SLANG_DERIVED_ASTNode_Stmt(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Stmt(SLANG_VISIT_IMPL, _)
};

//
// Declaration Visitors
//

struct IDeclVisitor
{
    SLANG_DERIVED_ASTNode_Decl(SLANG_VISITOR_DISPATCH, _)
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

    SLANG_DERIVED_ASTNode_Decl(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Decl(SLANG_VISIT_IMPL, _)
};

template<typename Derived>
struct DeclVisitor<Derived,void> : IDeclVisitor
{
    void dispatch(DeclBase* decl)
    {
        decl->accept(this, 0);
    }

    SLANG_DERIVED_ASTNode_Decl(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Decl(SLANG_VISIT_IMPL, _)
};

template<typename Derived, typename Arg>
struct DeclVisitorWithArg : IDeclVisitor
{
    void dispatch(DeclBase* obj, Arg const& arg)
    {
        obj->accept(this, (void*)&arg);
    }

    SLANG_DERIVED_ASTNode_Decl(SLANG_VISITOR_DISPATCH_WITH_ARG, _)
    SLANG_DERIVED_ASTNode_Decl(SLANG_VISIT_WITH_ARG, _)
};


//
// Modifier Visitors
//

struct IModifierVisitor
{
    SLANG_DERIVED_ASTNode_Modifier(SLANG_VISITOR_DISPATCH, _)
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

    SLANG_DERIVED_ASTNode_Modifier(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Modifier(SLANG_VISIT_IMPL, _)
};

template<typename Derived>
struct ModifierVisitor<Derived, void> : IModifierVisitor
{
    void dispatch(Modifier* modifier)
    {
        modifier->accept(this, 0);
    }

    SLANG_DERIVED_ASTNode_Modifier(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Modifier(SLANG_VISIT_IMPL, _)
};

//
// Val Visitors
//

struct IValVisitor : ITypeVisitor
{
    SLANG_DERIVED_ASTNode_Val(SLANG_VISITOR_DISPATCH, _)
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

    SLANG_DERIVED_ASTNode_Val(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Val(SLANG_VISIT_IMPL, _)
};

template<typename Derived>
struct ValVisitor<Derived, void, void> : TypeVisitor<Derived, void, IValVisitor>
{
    void dispatch(Val* val)
    {
        val->accept(this, 0);
    }

    SLANG_DERIVED_ASTNode_Val(SLANG_VISITOR_DISPATCH_IMPL, _)
    SLANG_DERIVED_ASTNode_Val(SLANG_VISIT_IMPL, _)
};

}

#endif
