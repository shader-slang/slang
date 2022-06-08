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
    SLANG_FORCE_INLINE void init(ASTNodeType inAstNodeType, ASTBuilder* /* astBuilder*/ ) { astNodeType = inAstNodeType; }

        /// Get the class info 
    SLANG_FORCE_INLINE const ReflectClassInfo& getClassInfo() const { return *ASTClassInfo::getInfo(astNodeType); }

    SyntaxClass<NodeBase> getClass() { return SyntaxClass<NodeBase>(&getClassInfo()); }

        /// The type of the node. ASTNodeType(-1) is an invalid node type, and shouldn't appear on any
        /// correctly constructed (through ASTBuilder) NodeBase derived class. 
        /// The actual type is set when constructed on the ASTBuilder. 
    ASTNodeType astNodeType = ASTNodeType(-1);

    // Handy when debugging, shouldn't be checked in though!
    // virtual ~NodeBase() {}
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

SLANG_FORCE_INLINE StringBuilder& operator<<(StringBuilder& io, Val* val) { SLANG_ASSERT(val); val->toText(io); return io; }

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
// In order to operation on types, though, we often want
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
    SLANG_FORCE_INLINE void init(ASTNodeType inAstNodeType, ASTBuilder* inAstBuilder) { m_astBuilder = inAstBuilder; astNodeType = inAstNodeType; }

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

    // The next outer that this one refines.
    Substitutions* outer = nullptr;

    // Apply a set of substitutions to the bindings in this substitution
    Substitutions* applySubstitutionsShallow(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);

    // Check if these are equivalent substitutions to another set
    bool equals(Substitutions* subst);
    HashCode getHashCode() const;

    // Overrides should be public so base classes can access
    Substitutions* _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;
};

class GenericSubstitution : public Substitutions
{
    SLANG_AST_CLASS(GenericSubstitution)

    // The generic declaration that defines the
    // parameters we are binding to arguments
    GenericDecl* genericDecl = nullptr;

    // The actual values of the arguments
    List<Val* > args;

    // Overrides should be public so base classes can access
    Substitutions* _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, Substitutions* substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;
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
};

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

    NameLoc nameAndLoc;

    Name*     getName()       { return nameAndLoc.name; }
    SourceLoc getNameLoc()    { return nameAndLoc.loc ; }
    NameLoc   getNameAndLoc() { return nameAndLoc     ; }

    DeclCheckStateExt checkState = DeclCheckState::Unchecked;

    // The next declaration defined in the same container with the same name
    Decl* nextInContainerWithSameName = nullptr;

    bool isChecked(DeclCheckState state) { return checkState >= state; }
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


} // namespace Slang
