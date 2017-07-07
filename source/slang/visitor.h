// visitor.h
#ifndef SLANG_VISITOR_H_INCLUDED
#define SLANG_VISITOR_H_INCLUDED

// This file defines the basic "Visitor" pattern for doing dispatch
// over the various categories of syntax node.

#include "syntax.h"

namespace Slang {

//
// Type Visitors
//

struct ITypeVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "object-meta-begin.h"
#include "type-defs.h"
#include "object-meta-end.h"
};

template<typename Derived, typename Result = void, typename Base = ITypeVisitor>
struct TypeVisitor : Base
{
    Result dispatch(ExpressionType* type)
    {
        Result result;
        type->accept(this, &result);
        return result;
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "object-meta-begin.h"
#include "type-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "type-defs.h"
#include "object-meta-end.h"
};

template<typename Derived, typename Base>
struct TypeVisitor<Derived,void,Base> : Base
{
    void dispatch(ExpressionType* type)
    {
        type->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "object-meta-begin.h"
#include "type-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "type-defs.h"
#include "object-meta-end.h"
};

//
// Expression Visitors
//

struct IExprVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "object-meta-begin.h"
#include "expr-defs.h"
#include "object-meta-end.h"
};

template<typename Derived, typename Result = void>
struct ExprVisitor : IExprVisitor
{
    Result dispatch(ExpressionSyntaxNode* expr)
    {
        Result result;
        expr->accept(this, &result);
        return result;
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "object-meta-begin.h"
#include "expr-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "expr-defs.h"
#include "object-meta-end.h"
};

template<typename Derived>
struct ExprVisitor<Derived,void> : IExprVisitor
{
    void dispatch(ExpressionSyntaxNode* expr)
    {
        expr->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "object-meta-begin.h"
#include "expr-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "expr-defs.h"
#include "object-meta-end.h"
};

//
// Statement Visitors
//

struct IStmtVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "object-meta-begin.h"
#include "stmt-defs.h"
#include "object-meta-end.h"
};

template<typename Derived, typename Result = void>
struct StmtVisitor : IStmtVisitor
{
    Result dispatch(StatementSyntaxNode* stmt)
    {
        Result result;
        stmt->accept(this, &result);
        return result;
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) override \
    { *(Result*)extra = ((Derived*) this)->visit##NAME(obj); }

#include "object-meta-begin.h"
#include "stmt-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "stmt-defs.h"
#include "object-meta-end.h"
};

template<typename Derived>
struct StmtVisitor<Derived,void> : IStmtVisitor
{
    void dispatch(StatementSyntaxNode* stmt)
    {
        stmt->accept(this, 0);
    }

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void*) override \
    { ((Derived*) this)->visit##NAME(obj); }

#include "object-meta-begin.h"
#include "stmt-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "stmt-defs.h"
#include "object-meta-end.h"
};

//
// Declaration Visitors
//

struct IDeclVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "object-meta-begin.h"
#include "decl-defs.h"
#include "object-meta-end.h"
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

#include "object-meta-begin.h"
#include "decl-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "decl-defs.h"
#include "object-meta-end.h"
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

#include "object-meta-begin.h"
#include "decl-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "decl-defs.h"
#include "object-meta-end.h"
};

//
// Modifier Visitors
//

struct IModifierVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "object-meta-begin.h"
#include "modifier-defs.h"
#include "object-meta-end.h"
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

#include "object-meta-begin.h"
#include "modifier-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "modifier-defs.h"
#include "object-meta-end.h"
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

#include "object-meta-begin.h"
#include "modifier-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "modifier-defs.h"
#include "object-meta-end.h"
};

//
// Val Visitors
//

struct IValVisitor : ITypeVisitor
{
#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#include "object-meta-begin.h"
#include "val-defs.h"
#include "object-meta-end.h"
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

#include "object-meta-begin.h"
#include "val-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    Result visit##NAME(NAME* obj) \
    { return ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "val-defs.h"
#include "object-meta-end.h"
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

#include "object-meta-begin.h"
#include "val-defs.h"
#include "object-meta-end.h"

#define ABSTRACT_SYNTAX_CLASS(NAME,BASE) SYNTAX_CLASS(NAME, BASE)
#define SYNTAX_CLASS(NAME, BASE) \
    void visit##NAME(NAME* obj) \
    { ((Derived*) this)->visit##BASE(obj); }

#include "object-meta-begin.h"
#include "val-defs.h"
#include "object-meta-end.h"

};

}

#endif