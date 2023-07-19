// slang-ast-base.h

#pragma once

#include "slang-ast-support-types.h"

#include "slang-generated-ast.h"
#include "slang-ast-reflect.h"

#include "slang-serialize-reflection.h"

// This file defines the primary base classes for the hierarchy of
// AST nodes and related objects. For example, this is where the
// basic `Decl`, `Stmt`, `Expr`, `type`, etc. definitions come from.

namespace Slang
{  

class NodeBase 
{
    SLANG_ABSTRACT_AST_CLASS(NodeBase)

        // MUST be called before used. Called automatically via the ASTBuilder.
        // Note that the astBuilder is not stored in the NodeBase derived types by default.
    SLANG_FORCE_INLINE void init(ASTNodeType inAstNodeType, ASTBuilder* /*astBuilder*/)
    {
        astNodeType = inAstNodeType;
#ifdef _DEBUG
        static uint32_t uidCounter = 0;
        static uint32_t breakValue = 0;
        uidCounter++;
        _debugUID = uidCounter;
        if (breakValue != 0 && _debugUID == breakValue)
            SLANG_BREAKPOINT(0)
#endif
    }

        /// Get the class info 
    SLANG_FORCE_INLINE const ReflectClassInfo& getClassInfo() const { return *ASTClassInfo::getInfo(astNodeType); }

    SyntaxClass<NodeBase> getClass() { return SyntaxClass<NodeBase>(&getClassInfo()); }

        /// The type of the node. ASTNodeType(-1) is an invalid node type, and shouldn't appear on any
        /// correctly constructed (through ASTBuilder) NodeBase derived class. 
        /// The actual type is set when constructed on the ASTBuilder. 
    ASTNodeType astNodeType = ASTNodeType(-1);

    // Handy when debugging, shouldn't be checked in though!
    // virtual ~NodeBase() {}
#ifdef _DEBUG
    SLANG_UNREFLECTED uint32_t _debugUID = 0;
#endif
};

// Casting of NodeBase

template<typename T>
SLANG_FORCE_INLINE T* dynamicCast(NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(uint32_t(node->astNodeType), T::kReflectClassInfo)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE const T* dynamicCast(const NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(uint32_t(node->astNodeType), T::kReflectClassInfo)) ? static_cast<const T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE T* as(NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(uint32_t(node->astNodeType), T::kReflectClassInfo)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE const T* as(const NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(uint32_t(node->astNodeType), T::kReflectClassInfo)) ? static_cast<const T*>(node) : nullptr;
}

// Because DeclRefBase is now a `Val`, we prevent casting it directly into other nodes
// to avoid confusion and bugs. Instead, use the `as<>()` method on `DeclRefBase` to
// get a `DeclRef<T>` for a specific node type.
template<typename T>
T* as(const DeclRefBase* declRefBase) = delete;

template<typename T, typename U>
DeclRef<T> as(DeclRef<U> declRef) { return DeclRef<T>(declRef); }

struct Scope : public NodeBase
{
    SLANG_AST_CLASS(Scope)
    
    // The container to use for lookup
    //
    // Note(tfoley): This is kept as an unowned pointer
    // so that a scope can't keep parts of the AST alive,
    // but the opposite it allowed.
    ContainerDecl*          containerDecl = nullptr;

    SLANG_UNREFLECTED
    // The parent of this scope (where lookup should go if nothing is found locally)
    Scope*                  parent = nullptr;

    // The next sibling of this scope (a peer for lookup)
    Scope*                  nextSibling = nullptr;
};

// Base class for all nodes representing actual syntax
// (thus having a location in the source code)
class SyntaxNodeBase : public NodeBase
{
    SLANG_ABSTRACT_AST_CLASS(SyntaxNodeBase)

    // The primary source location associated with this AST node
    SourceLoc loc;
};

// Base class for compile-time values (most often a type).
// These are *not* syntax nodes, because they do not have
// a unique location, and any two `Val`s representing
// the same value should be conceptually equal.

class Val : public NodeBase
{
    SLANG_ABSTRACT_AST_CLASS(Val)

    typedef IValVisitor Visitor;

    void accept(IValVisitor* visitor, void* extra);

    // construct a new value by applying a set of parameter
    // substitutions to this one
    Val* substitute(ASTBuilder* astBuilder, SubstitutionSet subst);

    // Lower-level interface for substitution. Like the basic
    // `Substitute` above, but also takes a by-reference
    // integer parameter that should be incremented when
    // returning a modified value (this can help the caller
    // decide whether they need to do anything).
    Val* substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

    bool equalsVal(Val* val);

    // Appends as text to the end of the builder
    void toText(StringBuilder& out);
    String toString();

    HashCode getHashCode();
    bool operator == (const Val & v)
    {
        return equalsVal(const_cast<Val*>(&v));
    }

    // Overrides should be public so base classes can access
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
    bool _equalsValOverride(Val* val);
    void _toTextOverride(StringBuilder& out);
    HashCode _getHashCodeOverride();
};

struct ValSet
{
    struct ValItem
    {
        Val* val = nullptr;
        ValItem() = default;
        ValItem(Val* v) : val(v) {}

        HashCode getHashCode()
        {
            return val ? val->getHashCode() : 0;
        }
        bool operator==(ValItem other)
        {
            if (val == other.val)
                return true;
            if (val)
                return val->equalsVal(other.val);
            else if (other.val)
                return other.val->equalsVal(val);
            return false;
        }
    };
    HashSet<ValItem> set;
    bool add(Val* val)
    {
        return set.add(ValItem(val));
    }
    bool contains(Val* val)
    {
        return set.contains(ValItem(val));
    }
};


SLANG_FORCE_INLINE StringBuilder& operator<<(StringBuilder& io, Val* val) { SLANG_ASSERT(val); val->toText(io); return io; }

    /// Given a `value` that refers to a `param` of some generic, attempt to apply
    /// the `subst` to it and produce a new `Val` as a result.
    ///
    /// If the `subst` does not include anything to replace `value`, then this function
    /// returns null.
    ///
Val* maybeSubstituteGenericParam(Val* value, Decl* param, SubstitutionSet subst, int* ioDiff);

class Type;

template <typename T>
SLANG_FORCE_INLINE T* as(Type* obj);
template <typename T>
SLANG_FORCE_INLINE const T* as(const Type* obj);
    
// A type, representing a classifier for some term in the AST.
//
// Types can include "sugar" in that they may refer to a
// `typedef` which gives them a good name when printed as
// part of diagnostic messages.
//
// In order to operate on types, though, we often want
// to look past any sugar, and operate on an underlying
// "canonical" type. The representation caches a pointer to
// a canonical type on every type, so we can easily
// operate on the raw representation when needed.
class Type: public Val
{
    SLANG_ABSTRACT_AST_CLASS(Type)

    typedef ITypeVisitor Visitor;

    void accept(ITypeVisitor* visitor, void* extra);

        /// Type derived types store the AST builder they were constructed on. The builder calls this function
        /// after constructing.
    SLANG_FORCE_INLINE void init(ASTNodeType inAstNodeType, ASTBuilder* inAstBuilder) { Val::init(inAstNodeType, inAstBuilder); m_astBuilder = inAstBuilder; }

        /// Get the ASTBuilder that was used to construct this Type
    SLANG_FORCE_INLINE ASTBuilder* getASTBuilder() const { return m_astBuilder; }

    bool equals(Type* type);
    
    Type* getCanonicalType();

    // Overrides should be public so base classes can access
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
    bool _equalsValOverride(Val* val);
    bool _equalsImplOverride(Type* type);
    Type* _createCanonicalTypeOverride();

    void _setASTBuilder(ASTBuilder* astBuilder) { m_astBuilder = astBuilder; }

protected:
    bool equalsImpl(Type* type);
    Type* createCanonicalType();

    Type* canonicalType = nullptr;

    SLANG_UNREFLECTED
    ASTBuilder* m_astBuilder = nullptr;
};

template <typename T>
SLANG_FORCE_INLINE T* as(Type* obj) { return obj ? dynamicCast<T>(obj->getCanonicalType()) : nullptr; }
template <typename T>
SLANG_FORCE_INLINE const T* as(const Type* obj) { return obj ? dynamicCast<T>(const_cast<Type*>(obj)->getCanonicalType()) : nullptr; }

// A substitution represents a binding of certain
// type-level variables to concrete argument values
class Substitutions: public NodeBase
{
    SLANG_ABSTRACT_AST_CLASS(Substitutions)


    // Apply a set of substitutions to the bindings in this substitution
    Substitutions* applySubstitutionsShallow(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);

    // Check if these are equivalent substitutions to another set
    bool equals(Substitutions* subst);
    HashCode getHashCode() const;

    // Overrides should be public so base classes can access
    Substitutions* _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;

    Substitutions* getOuter() const { return outer; }
protected:
    // The next outer that this one refines.
    Substitutions* outer = nullptr;
};

class GenericSubstitution : public Substitutions
{
    SLANG_AST_CLASS(GenericSubstitution)

private:
    // The generic declaration that defines the
    // parameters we are binding to arguments
    GenericDecl* genericDecl = nullptr;

    // The actual values of the arguments
    List<Val* > args;
public:
    GenericDecl* getGenericDecl() const { return genericDecl; }
    List<Val*>& getArgs() { return args; }
    const List<Val*>& getArgs() const { return args; }

    // Overrides should be public so base classes can access
    Substitutions* _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;

    GenericSubstitution(Substitutions* outerSubst, GenericDecl* decl, ArrayView<Val*> argVals)
    {
        outer = outerSubst;
        genericDecl = decl;
        args.addRange(argVals);
    }
};

class ThisTypeSubstitution : public Substitutions
{
    SLANG_AST_CLASS(ThisTypeSubstitution)

    // The declaration of the interface that we are specializing
    InterfaceDecl* interfaceDecl = nullptr;

    // A witness that shows that the concrete type used to
    // specialize the interface conforms to the interface.
    SubtypeWitness* witness = nullptr;

    // Overrides should be public so base classes can access
    // The actual type that provides the lookup scope for an associated type
    Substitutions* _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;

    ThisTypeSubstitution(Substitutions* outerSubst, InterfaceDecl* inInterfaceDecl, SubtypeWitness* inWitness)
        : interfaceDecl(inInterfaceDecl), witness(inWitness)
    {
        outer = outerSubst;
    }
};

class Decl;

// A reference to a declaration, which may include
// substitutions for generic parameters.
class DeclRefBase : public Val
{
    SLANG_AST_CLASS(DeclRefBase)

    Decl* getDecl() const { return decl; }

    Substitutions* getSubst() const { return substitutions; }

    DeclRefBase(Decl* decl)
        :decl(decl)
    {
    }

    DeclRefBase(Decl* decl, Substitutions* subst)
        :decl(decl), substitutions(subst)
    {
    }

    // Apply substitutions to a type or declaration
    Type* substitute(ASTBuilder* astBuilder, Type* type) const;

    DeclRefBase* substitute(ASTBuilder* astBuilder, DeclRefBase* declRef) const;

    // Apply substitutions to an expression
    SubstExpr<Expr> substitute(ASTBuilder* astBuilder, Expr* expr) const;

    // Apply substitutions to this declaration reference
    DeclRefBase* substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff) const;

    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
    {
        return substituteImpl(astBuilder, subst, ioDiff);
    }
    bool _equalsValOverride(Val* val);

    bool _equalsImplOverride(DeclRefBase* declRef) { return equals(declRef); }

    void _toTextOverride(StringBuilder& out) { toText(out); }

    // Returns true if 'as' will return a valid cast
    template <typename T>
    bool is() const { return Slang::as<T>(decl) != nullptr; }

    // Check if this is an equivalent declaration reference to another
    bool equals(DeclRefBase* declRef) const;

    // Convenience accessors for common properties of declarations
    Name* getName() const;
    SourceLoc getNameLoc() const;
    SourceLoc getLoc() const;
    DeclRefBase* getParent(ASTBuilder* astBuilder) const;

    HashCode getHashCode() const;

    // Debugging:
    String toString() const;
    void toText(StringBuilder& out) const;

private:

    // The underlying declaration
    Decl* decl = nullptr;
    // Optionally, a chain of substitutions to perform
    Substitutions* substitutions = nullptr;

};

SLANG_FORCE_INLINE StringBuilder& operator<<(StringBuilder& io, const DeclRefBase* declRef) { declRef->toText(io); return io; }

SLANG_FORCE_INLINE StringBuilder& operator<<(StringBuilder& io, const Decl* decl)
{
    if (decl)
        _printNestedDecl(nullptr, decl, io);
    return io;
}

class SyntaxNode : public SyntaxNodeBase
{
    SLANG_ABSTRACT_AST_CLASS(SyntaxNode);
};

//
// All modifiers are represented as full-fledged objects in the AST
// (that is, we don't use a bitfield, even for simple/common flags).
// This ensures that we can track source locations for all modifiers.
//
class Modifier : public SyntaxNode
{
    SLANG_ABSTRACT_AST_CLASS(Modifier)
    typedef IModifierVisitor Visitor;

    void accept(IModifierVisitor* visitor, void* extra);

    // Next modifier in linked list of modifiers on same piece of syntax
    Modifier* next = nullptr;

    // The keyword that was used to introduce t that was used to name this modifier.
    Name* keywordName = nullptr;

    Name* getKeywordName() { return keywordName; }
    NameLoc getKeywordNameAndLoc() { return NameLoc(keywordName, loc); }
};

// A syntax node which can have modifiers applied
class ModifiableSyntaxNode : public SyntaxNode
{
    SLANG_ABSTRACT_AST_CLASS(ModifiableSyntaxNode)

    Modifiers modifiers;

    template<typename T>
    FilteredModifierList<T> getModifiersOfType() { return FilteredModifierList<T>(modifiers.first); }

    // Find the first modifier of a given type, or return `nullptr` if none is found.
    template<typename T>
    T* findModifier()
    {
        return *getModifiersOfType<T>().begin();
    }

    template<typename T>
    bool hasModifier() { return findModifier<T>() != nullptr; }
};


// An intermediate type to represent either a single declaration, or a group of declarations
class DeclBase : public ModifiableSyntaxNode
{
    SLANG_ABSTRACT_AST_CLASS(DeclBase)

    typedef IDeclVisitor Visitor;

    void accept(IDeclVisitor* visitor, void* extra);
};

class Decl : public DeclBase
{
public:
    SLANG_ABSTRACT_AST_CLASS(Decl)

    ContainerDecl* parentDecl = nullptr;

    // A direct DeclRef to this Decl.
    // For every Decl, we create a DeclRef node representing a direct reference to it
    // upon the creation of the Decl (implemented in ASTBuilder), and store that
    // DeclRef here so we can get a direct DeclRef from a Decl* (by calling makeDeclRef)
    // without requiring a ASTBuilder to be available.
    DeclRefBase* defaultDeclRef = nullptr;

    NameLoc nameAndLoc;

    RefPtr<MarkupEntry> markup;

    Name*     getName() const      { return nameAndLoc.name; }
    SourceLoc getNameLoc() const   { return nameAndLoc.loc ; }
    NameLoc   getNameAndLoc() const { return nameAndLoc     ; }

    DeclCheckStateExt checkState = DeclCheckState::Unchecked;

    // The next declaration defined in the same container with the same name
    Decl* nextInContainerWithSameName = nullptr;

    bool isChecked(DeclCheckState state) const { return checkState >= state; }
    void setCheckState(DeclCheckState state)
    {
        SLANG_RELEASE_ASSERT(state >= checkState.getState());
        checkState.setState(state);
    }
};

class Expr : public SyntaxNode
{
    SLANG_ABSTRACT_AST_CLASS(Expr)

    typedef IExprVisitor Visitor;

    QualType type;

    void accept(IExprVisitor* visitor, void* extra);
};

class Stmt : public ModifiableSyntaxNode
{
    SLANG_ABSTRACT_AST_CLASS(Stmt)

    typedef IStmtVisitor Visitor;

    void accept(IStmtVisitor* visitor, void* extra);
};

template<typename T>
void DeclRef<T>::init(DeclRefBase* base)
{
    if (base && !Slang::as<T>(base->getDecl()))
        declRefBase = nullptr;
    else
        declRefBase = base;
}

template<typename T>
DeclRef<T>::DeclRef(Decl* decl)
{
    DeclRefBase* declRef = nullptr;
    if (decl)
    {
        SLANG_ASSERT(decl->defaultDeclRef);
        declRef = decl->defaultDeclRef;
    }
    init(declRef);
}

template<typename T>
T* DeclRef<T>::getDecl() const
{
    return declRefBase ? (T*)declRefBase->getDecl() : nullptr;
}

template<typename T>
Substitutions* DeclRef<T>::getSubst() const
{
    return declRefBase ? declRefBase->getSubst() : nullptr;
}

template<typename T>
Name* DeclRef<T>::getName() const
{
    if (declRefBase)
        return declRefBase->getName();
    return nullptr;
}

template<typename T>
SourceLoc DeclRef<T>::getNameLoc() const
{
    if (declRefBase) return declRefBase->getNameLoc();
    return SourceLoc();
}

template<typename T>
SourceLoc DeclRef<T>::getLoc() const
{
    if (declRefBase) return declRefBase->getLoc();
    return SourceLoc();
}

template<typename T>
DeclRef<ContainerDecl> DeclRef<T>::getParent(ASTBuilder* astBuilder) const
{
    if (declRefBase) return DeclRef<ContainerDecl>(declRefBase->getParent(astBuilder));
    return DeclRef<ContainerDecl>((DeclRefBase*)nullptr);
}

template<typename T>
HashCode DeclRef<T>::getHashCode() const
{
    if (declRefBase) return declRefBase->getHashCode();
    return 0;
}

template<typename T>
Type* DeclRef<T>::substitute(ASTBuilder* astBuilder, Type* type) const
{
    if (!declRefBase) return type;
    return declRefBase->substitute(astBuilder, type);
}

template<typename T>
SubstExpr<Expr> DeclRef<T>::substitute(ASTBuilder* astBuilder, Expr* expr) const
{
    if (!declRefBase) return expr;
    return declRefBase->substitute(astBuilder, expr);
}

// Apply substitutions to a type or declaration
template<typename T>
template<typename U>
DeclRef<U> DeclRef<T>::substitute(ASTBuilder* astBuilder, DeclRef<U> declRef) const
{
    if (!declRefBase) return declRef;
    return DeclRef<U>(declRefBase->substitute(astBuilder, declRef.declRefBase));
}

// Apply substitutions to this declaration reference
template<typename T>
DeclRef<T> DeclRef<T>::substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff) const
{
    if (!declRefBase) return *this;
    return DeclRef<T>(declRefBase->substituteImpl(astBuilder, subst, ioDiff));
}

template<typename T>
template<typename U>
bool DeclRef<T>::equals(DeclRef<U> other) const
{
    return declRefBase == other.declRefBase || (declRefBase && declRefBase->equals(other.declRefBase));
}

} // namespace Slang
