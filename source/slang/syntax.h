#ifndef RASTER_RENDERER_SYNTAX_H
#define RASTER_RENDERER_SYNTAX_H

#include "../core/basic.h"
#include "Lexer.h"
#include "Profile.h"

#include "../../Slang.h"

#include <assert.h>

namespace Slang
{
    class Substitutions;
    class SyntaxVisitor;
    class FunctionSyntaxNode;
    class Layout;

    struct IExprVisitor;
    struct IDeclVisitor;
    struct IModifierVisitor;
    struct IStmtVisitor;
    struct ITypeVisitor;
    struct IValVisitor;

    // Forward-declare all syntax classes
#define SYNTAX_CLASS(NAME, BASE, ...) class NAME;
#include "object-meta-begin.h"
#include "syntax-defs.h"
#include "object-meta-end.h"

    enum class IntrinsicOp
    {
        Unknown = 0,
#define INTRINSIC(NAME) NAME,
#include "intrinsic-defs.h"
    };

    IntrinsicOp findIntrinsicOp(char const* name);

    // Helper class for iterating over a list of heap-allocated modifiers
    struct ModifierList
    {
        struct Iterator
        {
            Modifier* current;

            Modifier* operator*()
            {
                return current;
            }

            void operator++();
#if 0
            {
                current = current->next.Ptr();
            }
#endif

            bool operator!=(Iterator other)
            {
                return current != other.current;
            };

            Iterator()
                : current(nullptr)
            {}

            Iterator(Modifier* modifier)
                : current(modifier)
            {}
        };

        ModifierList()
            : modifiers(nullptr)
        {}

        ModifierList(Modifier* modifiers)
            : modifiers(modifiers)
        {}

        Iterator begin() { return Iterator(modifiers); }
        Iterator end() { return Iterator(nullptr); }

        Modifier* modifiers;
    };

    // Helper class for iterating over heap-allocated modifiers
    // of a specific type.
    template<typename T>
    struct FilteredModifierList
    {
        struct Iterator
        {
            Modifier* current;

            T* operator*()
            {
                return (T*)current;
            }

            void operator++()
            {
                current = Adjust(current->next.Ptr());
            }

            bool operator!=(Iterator other)
            {
                return current != other.current;
            };

            Iterator()
                : current(nullptr)
            {}

            Iterator(Modifier* modifier)
                : current(modifier)
            {}
        };

        FilteredModifierList()
            : modifiers(nullptr)
        {}

        FilteredModifierList(Modifier* modifiers)
            : modifiers(Adjust(modifiers))
        {}

        Iterator begin() { return Iterator(modifiers); }
        Iterator end() { return Iterator(nullptr); }

        static Modifier* Adjust(Modifier* modifier)
        {
            Modifier* m = modifier;
            for (;;)
            {
                if (!m) return m;
                if (dynamic_cast<T*>(m)) return m;
                m = m->next.Ptr();
            }
        }

        Modifier* modifiers;
    };

    // A set of modifiers attached to a syntax node
    struct Modifiers
    {
        // The first modifier in the linked list of heap-allocated modifiers
        RefPtr<Modifier> first;

        template<typename T>
        FilteredModifierList<T> getModifiersOfType() { return FilteredModifierList<T>(first.Ptr()); }

        // Find the first modifier of a given type, or return `nullptr` if none is found.
        template<typename T>
        T* findModifier()
        {
            return *getModifiersOfType<T>().begin();
        }

        template<typename T>
        bool hasModifier() { return findModifier<T>() != nullptr; }

        FilteredModifierList<Modifier>::Iterator begin() { return FilteredModifierList<Modifier>::Iterator(first.Ptr()); }
        FilteredModifierList<Modifier>::Iterator end() { return FilteredModifierList<Modifier>::Iterator(nullptr); }
    };


    enum class BaseType
    {
        // Note(tfoley): These are ordered in terms of promotion rank, so be vareful when messing with this

        Void = 0,
        Bool,
        Int,
        UInt,
        UInt64,
        Float,
        Double,
    };

    class NamedExpressionType;
    class GenericDecl;
    class ContainerDecl;

    // Try to extract a simple integer value from an `IntVal`.
    // This fill assert-fail if the object doesn't represent a literal value.
    IntegerLiteralValue GetIntVal(RefPtr<IntVal> val);

    // Represents how much checking has been applied to a declaration.
    enum class DeclCheckState : uint8_t
    {
        // The declaration has been parsed, but not checked
        Unchecked,

        // We are in the process of checking the declaration "header"
        // (those parts of the declaration needed in order to
        // reference it)
        CheckingHeader,

        // We are done checking the declaration header.
        CheckedHeader,

        // We have checked the declaration fully.
        Checked,
    };

    void addModifier(
        RefPtr<ModifiableSyntaxNode>    syntax,
        RefPtr<Modifier>                modifier);

    struct QualType
    {
        RefPtr<ExpressionType>	type;
        bool					IsLeftValue;

        QualType()
            : IsLeftValue(false)
        {}

        QualType(ExpressionType* type)
            : type(type)
            , IsLeftValue(false)
        {}

        ExpressionType* Ptr() { return type.Ptr(); }

        operator RefPtr<ExpressionType>() { return type; }
        RefPtr<ExpressionType> operator->() { return type; }
    };


    // A reference to a declaration, which may include
    // substitutions for generic parameters.
    struct DeclRefBase
    {
        typedef Decl DeclType;

        // The underlying declaration
        Decl* decl = nullptr;
        Decl* getDecl() const { return decl; }

        // Optionally, a chain of substititions to perform
        RefPtr<Substitutions> substitutions;

        DeclRefBase()
        {}

        DeclRefBase(Decl* decl, RefPtr<Substitutions> substitutions)
            : decl(decl)
            , substitutions(substitutions)
        {}

        // Apply substitutions to a type or ddeclaration
        RefPtr<ExpressionType> Substitute(RefPtr<ExpressionType> type) const;

        DeclRefBase Substitute(DeclRefBase declRef) const;

        // Apply substitutions to an expression
        RefPtr<ExpressionSyntaxNode> Substitute(RefPtr<ExpressionSyntaxNode> expr) const;

        // Apply substitutions to this declaration reference
        DeclRefBase SubstituteImpl(Substitutions* subst, int* ioDiff);

        // Check if this is an equivalent declaration reference to another
        bool Equals(DeclRefBase const& declRef) const;
        bool operator == (const DeclRefBase& other) const
        {
            return Equals(other);
        }

        // Convenience accessors for common properties of declarations
        String const& GetName() const;
        DeclRefBase GetParent() const;

        int GetHashCode() const;
    };

    template<typename T>
    struct DeclRef : DeclRefBase
    {
        typedef T DeclType;

        DeclRef()
        {}

        DeclRef(T* decl, RefPtr<Substitutions> substitutions)
            : DeclRefBase(decl, substitutions)
        {}

        template <typename U>
        DeclRef(DeclRef<U> const& other,
            typename EnableIf<IsConvertible<T*, U*>::Value, void>::type* = 0)
            : DeclRefBase(other.decl, other.substitutions)
        {
        }

        // "dynamic cast" to a more specific declaration reference type
        template<typename T>
        DeclRef<T> As() const
        {
            DeclRef<T> result;
            result.decl = dynamic_cast<T*>(decl);
            result.substitutions = substitutions;
            return result;
        }

        T* getDecl() const
        {
            return (T*)decl;
        }

        operator T*() const
        {
            return getDecl();
        }

        //
        static DeclRef<T> unsafeInit(DeclRefBase const& declRef)
        {
            return DeclRef<T>((T*) declRef.decl, declRef.substitutions);
        }

        RefPtr<ExpressionType> Substitute(RefPtr<ExpressionType> type) const
        {
            return DeclRefBase::Substitute(type);
        }
        RefPtr<ExpressionSyntaxNode> Substitute(RefPtr<ExpressionSyntaxNode> expr) const
        {
            return DeclRefBase::Substitute(expr);
        }

        // Apply substitutions to a type or ddeclaration
        template<typename U>
        DeclRef<U> Substitute(DeclRef<U> declRef) const
        {
            return DeclRef<U>::unsafeInit(DeclRefBase::Substitute(declRef));
        }

        // Apply substitutions to this declaration reference
        DeclRef<T> SubstituteImpl(Substitutions* subst, int* ioDiff)
        {
            return DeclRef<T>::unsafeInit(DeclRefBase::SubstituteImpl(subst, ioDiff));
        }

        DeclRef<ContainerDecl> GetParent() const
        {
            return DeclRef<ContainerDecl>::unsafeInit(DeclRefBase::GetParent());
        }
    };

    
    template<typename T>
    inline DeclRef<T> makeDeclRef(T* decl)
    {
        return DeclRef<T>(decl, nullptr);
    }

    template<typename T>
    struct FilteredMemberList
    {
        typedef RefPtr<Decl> Element;

        FilteredMemberList()
            : mBegin(NULL)
            , mEnd(NULL)
        {}

        explicit FilteredMemberList(
            List<Element> const& list)
            : mBegin(Adjust(list.begin(), list.end()))
            , mEnd(list.end())
        {}

        struct Iterator
        {
            Element* mCursor;
            Element* mEnd;

            bool operator!=(Iterator const& other)
            {
                return mCursor != other.mCursor;
            }

            void operator++()
            {
                mCursor = Adjust(mCursor + 1, mEnd);
            }

            RefPtr<T>& operator*()
            {
                return *(RefPtr<T>*)mCursor;
            }
        };

        Iterator begin()
        {
            Iterator iter = { mBegin, mEnd };
            return iter;
        }

        Iterator end()
        {
            Iterator iter = { mEnd, mEnd };
            return iter;
        }

        static Element* Adjust(Element* cursor, Element* end)
        {
            while (cursor != end)
            {
                if ((*cursor).As<T>())
                    return cursor;
                cursor++;
            }
            return cursor;
        }

        // TODO(tfoley): It is ugly to have these.
        // We should probably fix the call sites instead.
        RefPtr<T>& First() { return *begin(); }
        int Count()
        {
            int count = 0;
            for (auto iter : (*this))
            {
                (void)iter;
                count++;
            }
            return count;
        }

        List<RefPtr<T>> ToArray()
        {
            List<RefPtr<T>> result;
            for (auto element : (*this))
            {
                result.Add(element);
            }
            return result;
        }

        Element* mBegin;
        Element* mEnd;
    };

    struct TransparentMemberInfo
    {
        // The declaration of the transparent member
        Decl*	decl;
    };

    template<typename T>
    struct FilteredMemberRefList
    {
        List<RefPtr<Decl>> const&	decls;
        RefPtr<Substitutions>		substitutions;

        FilteredMemberRefList(
            List<RefPtr<Decl>> const&	decls,
            RefPtr<Substitutions>		substitutions)
            : decls(decls)
            , substitutions(substitutions)
        {}

        int Count() const
        {
            int count = 0;
            for (auto d : *this)
                count++;
            return count;
        }

        List<DeclRef<T>> ToArray() const
        {
            List<DeclRef<T>> result;
            for (auto d : *this)
                result.Add(d);
            return result;
        }

        struct Iterator
        {
            FilteredMemberRefList const* list;
            RefPtr<Decl>* ptr;
            RefPtr<Decl>* end;

            Iterator() : list(nullptr), ptr(nullptr) {}
            Iterator(
                FilteredMemberRefList const* list,
                RefPtr<Decl>* ptr,
                RefPtr<Decl>* end)
                : list(list)
                , ptr(ptr)
                , end(end)
            {}

            bool operator!=(Iterator other)
            {
                return ptr != other.ptr;
            }

            void operator++()
            {
                ptr = list->Adjust(ptr + 1, end);
            }

            DeclRef<T> operator*()
            {
                return DeclRef<T>((T*) ptr->Ptr(), list->substitutions);
            }
        };

        Iterator begin() const { return Iterator(this, Adjust(decls.begin(), decls.end()), decls.end()); }
        Iterator end() const { return Iterator(this, decls.end(), decls.end()); }

        RefPtr<Decl>* Adjust(RefPtr<Decl>* ptr, RefPtr<Decl>* end) const
        {
            while (ptr != end)
            {
                DeclRef<Decl> declRef(ptr->Ptr(), substitutions);
                if (declRef.As<T>())
                    return ptr;
                ptr++;
            }
            return end;
        }
    };

    //
    // Type Expressions
    //

    // A "type expression" is a term that we expect to resolve to a type during checking.
    // We store both the original syntax and the resolved type here.
    struct TypeExp
    {
        TypeExp() {}
        TypeExp(TypeExp const& other)
            : exp(other.exp)
            , type(other.type)
        {}
        explicit TypeExp(RefPtr<ExpressionSyntaxNode> exp)
            : exp(exp)
        {}
        TypeExp(RefPtr<ExpressionSyntaxNode> exp, RefPtr<ExpressionType> type)
            : exp(exp)
            , type(type)
        {}

        RefPtr<ExpressionSyntaxNode> exp;
        RefPtr<ExpressionType> type;

        bool Equals(ExpressionType* other);
#if 0
        {
            return type->Equals(other);
        }
#endif
        bool Equals(RefPtr<ExpressionType> other);
#if 0
        {
            return type->Equals(other.Ptr());
        }
#endif
        ExpressionType* Ptr() { return type.Ptr(); }
        operator ExpressionType*()
        {
            return type;
        }
        ExpressionType* operator->() { return Ptr(); }

        TypeExp Accept(SyntaxVisitor* visitor);
    };



    struct Scope : public RefObject
    {
        // The parent of this scope (where lookup should go if nothing is found locally)
        RefPtr<Scope>           parent;

        // The next sibling of this scope (a peer for lookup)
        RefPtr<Scope>           nextSibling;

        // The container to use for lookup
        //
        // Note(tfoley): This is kept as an unowned pointer
        // so that a scope can't keep parts of the AST alive,
        // but the opposite it allowed.
        ContainerDecl*          containerDecl;
    };

    // Masks to be applied when lookup up declarations
    enum class LookupMask : uint8_t
    {
        Type = 0x1,
        Function = 0x2,
        Value = 0x4,

        All = Type | Function | Value,
    };

    // Represents one item found during lookup
    struct LookupResultItem
    {
        // Sometimes lookup finds an item, but there were additional
        // "hops" taken to reach it. We need to remember these steps
        // so that if/when we consturct a full expression we generate
        // appropriate AST nodes for all the steps.
        //
        // We build up a list of these "breadcrumbs" while doing
        // lookup, and store them alongside each item found.
        class Breadcrumb : public RefObject
        {
        public:
            enum class Kind
            {
                Member, // A member was references
                Deref, // A value with pointer(-like) type was dereferenced
            };

            Kind kind;
            DeclRef<Decl> declRef;
            RefPtr<Breadcrumb> next;

            Breadcrumb(Kind kind, DeclRef<Decl> declRef, RefPtr<Breadcrumb> next)
                : kind(kind)
                , declRef(declRef)
                , next(next)
            {}
        };

        // A properly-specialized reference to the declaration that was found.
        DeclRef<Decl> declRef;

        // Any breadcrumbs needed in order to turn that declaration
        // reference into a well-formed expression.
        //
        // This is unused in the simple case where a declaration
        // is being referenced directly (rather than through
        // transparent members).
        RefPtr<Breadcrumb> breadcrumbs;

        LookupResultItem() = default;
        explicit LookupResultItem(DeclRef<Decl> declRef)
            : declRef(declRef)
        {}
        LookupResultItem(DeclRef<Decl> declRef, RefPtr<Breadcrumb> breadcrumbs)
            : declRef(declRef)
            , breadcrumbs(breadcrumbs)
        {}
    };


    // Result of looking up a name in some lexical/semantic environment.
    // Can be used to enumerate all the declarations matching that name,
    // in the case where the result is overloaded.
    struct LookupResult
    {
        // The one item that was found, in the smple case
        LookupResultItem item;

        // All of the items that were found, in the complex case.
        // Note: if there was no overloading, then this list isn't
        // used at all, to avoid allocation.
        List<LookupResultItem> items;

        // Was at least one result found?
        bool isValid() const { return item.declRef.getDecl() != nullptr; }

        bool isOverloaded() const { return items.Count() > 1; }
    };

    struct SemanticsVisitor;

    struct LookupRequest
    {
        SemanticsVisitor*   semantics   = nullptr;

        RefPtr<Scope>       scope       = nullptr;
        RefPtr<Scope>       endScope    = nullptr;

        LookupMask          mask        = LookupMask::All;
    };

    enum class Operator
    {
        Neg, Not, BitNot, PreInc, PreDec, PostInc, PostDec,
        Mul, Div, Mod,
        Add, Sub,
        Lsh, Rsh,
        Eql, Neq, Greater, Less, Geq, Leq,
        BitAnd, BitXor, BitOr,
        And,
        Or,
        Sequence,
        Select,
        Assign = 200, AddAssign, SubAssign, MulAssign, DivAssign, ModAssign,
        LshAssign, RshAssign, OrAssign, AndAssign, XorAssign,
    };

    // Generate class definition for all syntax classes
#define SYNTAX_FIELD(TYPE, NAME) TYPE NAME;
#define FIELD(TYPE, NAME) TYPE NAME;
#define FIELD_INIT(TYPE, NAME, INIT) TYPE NAME = INIT;
#define RAW(...) __VA_ARGS__
#define END_SYNTAX_CLASS() };
#define SYNTAX_CLASS(NAME, BASE, ...) class NAME : public BASE {public:
#include "object-meta-begin.h"

#include "syntax-base-defs.h"
#undef SYNTAX_CLASS

#undef ABSTRACT_SYNTAX_CLASS
#define ABSTRACT_SYNTAX_CLASS(NAME, BASE, ...)                          \
    class NAME : public BASE {                                          \
    public: /* ... */
#define SYNTAX_CLASS(NAME, BASE, ...)                                   \
    class NAME : public BASE {                                          \
    virtual void accept(NAME::Visitor* visitor, void* extra) override;  \
    public: /* ... */
#include "expr-defs.h"
#include "decl-defs.h"
#include "modifier-defs.h"
#include "stmt-defs.h"
#include "type-defs.h"
#include "val-defs.h"

#include "object-meta-end.h"

    inline RefPtr<ExpressionType> GetSub(DeclRef<GenericTypeConstraintDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->sub.Ptr());
    }

    inline RefPtr<ExpressionType> GetSup(DeclRef<GenericTypeConstraintDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->sup.Ptr());
    }

    //
#if 0

    class SyntaxVisitor : public RefObject
    {
    protected:
        DiagnosticSink * sink = nullptr;
        DiagnosticSink* getSink() { return sink; }

        SourceLanguage sourceLanguage = SourceLanguage::Unknown;
    public:
        void setSourceLanguage(SourceLanguage language)
        {
            sourceLanguage = language;
        }

        SyntaxVisitor(DiagnosticSink * sink)
            : sink(sink)
        {}
        virtual ~SyntaxVisitor()
        {
        }

        virtual RefPtr<ProgramSyntaxNode> VisitProgram(ProgramSyntaxNode* program)
        {
            for (auto & m : program->Members)
                m = m->Accept(this).As<Decl>();
            return program;
        }

        virtual void visitImportDecl(ImportDecl * decl) = 0;

        virtual RefPtr<FunctionSyntaxNode> VisitFunction(FunctionSyntaxNode* func)
        {
            func->ReturnType = func->ReturnType.Accept(this);
            for (auto & member : func->Members)
                member = member->Accept(this).As<Decl>();
            if (func->Body)
                func->Body = func->Body->Accept(this).As<BlockStatementSyntaxNode>();
            return func;
        }
        virtual RefPtr<ScopeDecl> VisitScopeDecl(ScopeDecl* decl)
        {
            // By default don't visit children, because they will always
            // be encountered in the ordinary flow of the corresponding statement.
            return decl;
        }
        virtual RefPtr<StructSyntaxNode> VisitStruct(StructSyntaxNode * s)
        {
            for (auto & f : s->Members)
                f = f->Accept(this).As<Decl>();
            return s;
        }
        virtual RefPtr<ClassSyntaxNode> VisitClass(ClassSyntaxNode * s)
        {
            for (auto & f : s->Members)
                f = f->Accept(this).As<Decl>();
            return s;
        }
        virtual RefPtr<GenericDecl> VisitGenericDecl(GenericDecl * decl)
        {
            for (auto & m : decl->Members)
                m = m->Accept(this).As<Decl>();
            decl->inner = decl->inner->Accept(this).As<Decl>();
            return decl;
        }
        virtual RefPtr<TypeDefDecl> VisitTypeDefDecl(TypeDefDecl* decl)
        {
            decl->Type = decl->Type.Accept(this);
            return decl;
        }
        virtual RefPtr<StatementSyntaxNode> VisitDiscardStatement(DiscardStatementSyntaxNode * stmt)
        {
            return stmt;
        }
        virtual RefPtr<StructField> VisitStructField(StructField * f)
        {
            f->Type = f->Type.Accept(this);
            return f;
        }
        virtual RefPtr<StatementSyntaxNode> VisitBlockStatement(BlockStatementSyntaxNode* stmt)
        {
            for (auto & s : stmt->Statements)
                s = s->Accept(this).As<StatementSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitBreakStatement(BreakStatementSyntaxNode* stmt)
        {
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitContinueStatement(ContinueStatementSyntaxNode* stmt)
        {
            return stmt;
        }

        virtual RefPtr<StatementSyntaxNode> VisitDoWhileStatement(DoWhileStatementSyntaxNode* stmt)
        {
            if (stmt->Predicate)
                stmt->Predicate = stmt->Predicate->Accept(this).As<ExpressionSyntaxNode>();
            if (stmt->Statement)
                stmt->Statement = stmt->Statement->Accept(this).As<StatementSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitEmptyStatement(EmptyStatementSyntaxNode* stmt)
        {
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitForStatement(ForStatementSyntaxNode* stmt)
        {
            if (stmt->InitialStatement)
                stmt->InitialStatement = stmt->InitialStatement->Accept(this).As<StatementSyntaxNode>();
            if (stmt->PredicateExpression)
                stmt->PredicateExpression = stmt->PredicateExpression->Accept(this).As<ExpressionSyntaxNode>();
            if (stmt->SideEffectExpression)
                stmt->SideEffectExpression = stmt->SideEffectExpression->Accept(this).As<ExpressionSyntaxNode>();
            if (stmt->Statement)
                stmt->Statement = stmt->Statement->Accept(this).As<StatementSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitIfStatement(IfStatementSyntaxNode* stmt)
        {
            if (stmt->Predicate)
                stmt->Predicate = stmt->Predicate->Accept(this).As<ExpressionSyntaxNode>();
            if (stmt->PositiveStatement)
                stmt->PositiveStatement = stmt->PositiveStatement->Accept(this).As<StatementSyntaxNode>();
            if (stmt->NegativeStatement)
                stmt->NegativeStatement = stmt->NegativeStatement->Accept(this).As<StatementSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<SwitchStmt> VisitSwitchStmt(SwitchStmt* stmt)
        {
            if (stmt->condition)
                stmt->condition = stmt->condition->Accept(this).As<ExpressionSyntaxNode>();
            if (stmt->body)
                stmt->body = stmt->body->Accept(this).As<BlockStatementSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<CaseStmt> VisitCaseStmt(CaseStmt* stmt)
        {
            if (stmt->expr)
                stmt->expr = stmt->expr->Accept(this).As<ExpressionSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<DefaultStmt> VisitDefaultStmt(DefaultStmt* stmt)
        {
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitReturnStatement(ReturnStatementSyntaxNode* stmt)
        {
            if (stmt->Expression)
                stmt->Expression = stmt->Expression->Accept(this).As<ExpressionSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitVarDeclrStatement(VarDeclrStatementSyntaxNode* stmt)
        {
            stmt->decl = stmt->decl->Accept(this).As<DeclBase>();
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitWhileStatement(WhileStatementSyntaxNode* stmt)
        {
            if (stmt->Predicate)
                stmt->Predicate = stmt->Predicate->Accept(this).As<ExpressionSyntaxNode>();
            if (stmt->Statement)
                stmt->Statement = stmt->Statement->Accept(this).As<StatementSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<StatementSyntaxNode> VisitExpressionStatement(ExpressionStatementSyntaxNode* stmt)
        {
            if (stmt->Expression)
                stmt->Expression = stmt->Expression->Accept(this).As<ExpressionSyntaxNode>();
            return stmt;
        }

        virtual RefPtr<ExpressionSyntaxNode> VisitOperatorExpression(OperatorExpressionSyntaxNode* expr)
        {
            for (auto && child : expr->Arguments)
                child->Accept(this);
            return expr;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitConstantExpression(ConstantExpressionSyntaxNode* expr)
        {
            return expr;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitIndexExpression(IndexExpressionSyntaxNode* expr)
        {
            if (expr->BaseExpression)
                expr->BaseExpression = expr->BaseExpression->Accept(this).As<ExpressionSyntaxNode>();
            if (expr->IndexExpression)
                expr->IndexExpression = expr->IndexExpression->Accept(this).As<ExpressionSyntaxNode>();
            return expr;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitMemberExpression(MemberExpressionSyntaxNode * stmt)
        {
            if (stmt->BaseExpression)
                stmt->BaseExpression = stmt->BaseExpression->Accept(this).As<ExpressionSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitSwizzleExpression(SwizzleExpr * expr)
        {
            if (expr->base)
                expr->base->Accept(this);
            return expr;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitInvokeExpression(InvokeExpressionSyntaxNode* stmt)
        {
            stmt->FunctionExpr->Accept(this);
            for (auto & arg : stmt->Arguments)
                arg = arg->Accept(this).As<ExpressionSyntaxNode>();
            return stmt;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitTypeCastExpression(TypeCastExpressionSyntaxNode * stmt)
        {
            if (stmt->Expression)
                stmt->Expression = stmt->Expression->Accept(this).As<ExpressionSyntaxNode>();
            return stmt->Expression;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitVarExpression(VarExpressionSyntaxNode* expr)
        {
            return expr;
        }

        virtual RefPtr<ParameterSyntaxNode> VisitParameter(ParameterSyntaxNode* param)
        {
            return param;
        }
        virtual RefPtr<ExpressionSyntaxNode> VisitGenericApp(GenericAppExpr* type)
        {
            return type;
        }

        virtual RefPtr<Variable> VisitDeclrVariable(Variable* dclr)
        {
            if (dclr->Expr)
                dclr->Expr = dclr->Expr->Accept(this).As<ExpressionSyntaxNode>();
            return dclr;
        }

        virtual TypeExp VisitTypeExp(TypeExp const& typeExp)
        {
            TypeExp result = typeExp;
            result.exp = typeExp.exp->Accept(this).As<ExpressionSyntaxNode>();
            if (auto typeType = result.exp->Type.type.As<TypeType>())
            {
                result.type = typeType->type;
            }
            return result;
        }

        virtual void VisitExtensionDecl(ExtensionDecl* /*decl*/)
        {}

        virtual void VisitConstructorDecl(ConstructorDecl* /*decl*/)
        {}

        virtual void visitSubscriptDecl(SubscriptDecl* decl) = 0;

        virtual void visitAccessorDecl(AccessorDecl* decl) = 0;

        virtual void visitInterfaceDecl(InterfaceDecl* /*decl*/) = 0;

        virtual void visitInheritanceDecl(InheritanceDecl* /*decl*/) = 0;

        virtual RefPtr<ExpressionSyntaxNode> VisitSharedTypeExpr(SharedTypeExpr* typeExpr)
        {
            return typeExpr;
        }

        virtual void VisitDeclGroup(DeclGroup* declGroup)
        {
            for (auto decl : declGroup->decls)
            {
                decl->Accept(this);
            }
        }

        virtual RefPtr<ExpressionSyntaxNode> visitInitializerListExpr(InitializerListExpr* expr) = 0;
    };

#endif

    // Note(tfoley): These logically belong to `ExpressionType`,
    // but order-of-declaration stuff makes that tricky
    //
    // TODO(tfoley): These should really belong to the compilation context!
    //
    void RegisterBuiltinDecl(
        RefPtr<Decl>                decl,
        RefPtr<BuiltinTypeModifier> modifier);
    void RegisterMagicDecl(
        RefPtr<Decl>                decl,
        RefPtr<MagicTypeModifier>   modifier);

    // Look up a magic declaration by its name
    RefPtr<Decl> findMagicDecl(
        String const& name);

    // Create an instance of a syntax class by name
    SyntaxNodeBase* createInstanceOfSyntaxClassByName(
        String const&   name);

    //

    inline BaseType GetVectorBaseType(VectorExpressionType* vecType) {
        return vecType->elementType->AsBasicType()->BaseType;
    }

    inline int GetVectorSize(VectorExpressionType* vecType)
    {
        auto constantVal = vecType->elementCount.As<ConstantIntVal>();
        if (constantVal)
            return (int) constantVal->value;
        // TODO: what to do in this case?
        return 0;
    }

    //
    // Declarations
    //

    inline FilteredMemberRefList<Decl> getMembers(DeclRef<ContainerDecl> const& declRef)
    {
        return FilteredMemberRefList<Decl>(declRef.getDecl()->Members, declRef.substitutions);
    }

    template<typename T>
    inline FilteredMemberRefList<T> getMembersOfType(DeclRef<ContainerDecl> const& declRef)
    {
        return FilteredMemberRefList<T>(declRef.getDecl()->Members, declRef.substitutions);
    }

    inline RefPtr<ExpressionType> GetType(DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->Type.Ptr());
    }

    inline RefPtr<ExpressionSyntaxNode> getInitExpr(DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->Expr);
    }

    inline RefPtr<ExpressionType> GetTargetType(DeclRef<ExtensionDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->targetType.Ptr());
    }

    inline ExtensionDecl* GetCandidateExtensions(DeclRef<AggTypeDecl> const& declRef)
    {
        return declRef.getDecl()->candidateExtensions;
    }

    inline FilteredMemberRefList<StructField> GetFields(DeclRef<StructSyntaxNode> const& declRef)
    {
        return getMembersOfType<StructField>(declRef);
    }

    inline RefPtr<ExpressionType> getBaseType(DeclRef<InheritanceDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->base.type);
    }

    inline RefPtr<ExpressionType> GetType(DeclRef<TypeDefDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->Type.Ptr());
    }

    inline RefPtr<ExpressionType> GetResultType(DeclRef<CallableDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->ReturnType.type.Ptr());
    }

    inline FilteredMemberRefList<ParameterSyntaxNode> GetParameters(DeclRef<CallableDecl> const& declRef)
    {
        return getMembersOfType<ParameterSyntaxNode>(declRef);
    }

    inline Decl* GetInner(DeclRef<GenericDecl> const& declRef)
    {
        // TODO: Should really return a `DeclRef<Decl>` for the inner
        // declaration, and not just a raw pointer
        return declRef.getDecl()->inner.Ptr();
    }

} // namespace Slang

#endif