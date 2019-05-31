// slang-visitor.h
#ifndef SLANG_VISITOR_H_INCLUDED
#define SLANG_VISITOR_H_INCLUDED

// This file defines the basic "Visitor" pattern for doing dispatch
// over the various categories of syntax node.

#include "slang-syntax.h"

namespace Slang {

//
// type Visitors
//

struct ITypeVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"
};

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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"
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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived, typename Arg, typename Base = ITypeVisitor>
struct TypeVisitorWithArg : Base
{
    void dispatch(Type* type, Arg const& arg)
    {
        type->accept(this, (void*)&arg);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* arg) override \
    { ((Derived*) this)->visit##NAME(obj, *(Arg*)arg); }

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj, Arg const& arg) \
    { ((Derived*) this)->visit##BASE(obj, arg); }

#include "slang-object-meta-begin.h"
#include "slang-type-defs.h"
#include "slang-object-meta-end.h"
};

//
// Expression Visitors
//

struct IExprVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"
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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived>
struct ExprVisitor<Derived,void> : IExprVisitor
{
    void dispatch(Expr* expr)
    {
        expr->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived, typename Arg>
struct ExprVisitorWithArg : IExprVisitor
{
    void dispatch(Expr* obj, Arg const& arg)
    {
        obj->accept(this, (void*)&arg);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* arg) override \
    { ((Derived*) this)->visit##NAME(obj, *(Arg*)arg); }

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj, Arg const& arg) \
    { ((Derived*) this)->visit##BASE(obj, arg); }

#include "slang-object-meta-begin.h"
#include "slang-expr-defs.h"
#include "slang-object-meta-end.h"
};

//
// Statement Visitors
//

struct IStmtVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "slang-object-meta-begin.h"
#include "slang-stmt-defs.h"
#include "slang-object-meta-end.h"
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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-stmt-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-stmt-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived>
struct StmtVisitor<Derived,void> : IStmtVisitor
{
    void dispatch(Stmt* stmt)
    {
        stmt->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-stmt-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-stmt-defs.h"
#include "slang-object-meta-end.h"
};

//
// Declaration Visitors
//

struct IDeclVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"
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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived>
struct DeclVisitor<Derived,void> : IDeclVisitor
{
    void dispatch(DeclBase* decl)
    {
        decl->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived, typename Arg>
struct DeclVisitorWithArg : IDeclVisitor
{
    void dispatch(DeclBase* obj, Arg const& arg)
    {
        obj->accept(this, (void*)&arg);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* arg) override \
    { ((Derived*) this)->visit##NAME(obj, *(Arg*)arg); }

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj, Arg const& arg) \
    { ((Derived*) this)->visit##BASE(obj, arg); }

#include "slang-object-meta-begin.h"
#include "slang-decl-defs.h"
#include "slang-object-meta-end.h"
};


//
// Modifier Visitors
//

struct IModifierVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "slang-object-meta-begin.h"
#include "slang-modifier-defs.h"
#include "slang-object-meta-end.h"
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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-modifier-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-modifier-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived>
struct ModifierVisitor<Derived, void> : IModifierVisitor
{
    void dispatch(Modifier* modifier)
    {
        modifier->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-modifier-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-modifier-defs.h"
#include "slang-object-meta-end.h"
};

//
// Val Visitors
//

struct IValVisitor : ITypeVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "slang-object-meta-begin.h"
#include "slang-val-defs.h"
#include "slang-object-meta-end.h"
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

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-val-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-val-defs.h"
#include "slang-object-meta-end.h"
};

template<typename Derived>
struct ValVisitor<Derived, void, void> : TypeVisitor<Derived, void, IValVisitor>
{
    void dispatch(Val* val)
    {
        val->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "slang-object-meta-begin.h"
#include "slang-val-defs.h"
#include "slang-object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "slang-object-meta-begin.h"
#include "slang-val-defs.h"
#include "slang-object-meta-end.h"

};

}

#endif
