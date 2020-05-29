// slang-ast-base.h

#pragma once

#include "slang-ast-support-types.h"

#include "slang-ast-generated.h"
#include "slang-ast-reflect.h"

// This file defines the primary base classes for the hierarchy of
// AST nodes and related objects. For example, this is where the
// basic `Decl`, `Stmt`, `Expr`, `type`, etc. definitions come from.

namespace Slang
{

// Signals to C++ extractor that RefObject is a base class, that isn't reflected to C++ extractor
SLANG_REFLECT_BASE_CLASS(RefObject)

struct ReflectClassInfo;

class NodeBase : public RefObject
{
    SLANG_ABSTRACT_CLASS(NodeBase)

        // MUST be called before used. Called automatically via the ASTBuilder.
        // Note that the astBuilder is not stored in the NodeBase derived types by default.
    SLANG_FORCE_INLINE void init(ASTNodeType inAstNodeType, ASTBuilder* /* astBuilder*/ ) { astNodeType = inAstNodeType; }

        /// Get the class info 
    SLANG_FORCE_INLINE const ReflectClassInfo& getClassInfo() const { return *ReflectClassInfo::getInfo(astNodeType); }

    SyntaxClass<NodeBase> getClass() { return SyntaxClass<NodeBase>(&getClassInfo()); }

        /// The type of the node. ASTNodeType(-1) is an invalid node type, and shouldn't appear on any
        /// correctly constructed (through ASTBuilder) NodeBase derived class. 
        /// The actual type is set when constructed on the ASTBuilder. 
    ASTNodeType astNodeType = ASTNodeType(-1);
};

// Casting of NodeBase

template<typename T>
SLANG_FORCE_INLINE T* dynamicCast(NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(node->astNodeType, T::kReflectClassInfo)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE const T* dynamicCast(const NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(node->astNodeType, T::kReflectClassInfo)) ? static_cast<const T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE T* as(NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(node->astNodeType, T::kReflectClassInfo)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE const T* as(const NodeBase* node)
{
    return (node && ReflectClassInfo::isSubClassOf(node->astNodeType, T::kReflectClassInfo)) ? static_cast<const T*>(node) : nullptr;
}



// Base class for all nodes representing actual syntax
// (thus having a location in the source code)
class SyntaxNodeBase : public NodeBase
{
    SLANG_ABSTRACT_CLASS(SyntaxNodeBase)

    // The primary source location associated with this AST node
    SourceLoc loc;
};

// Base class for compile-time values (most often a type).
// These are *not* syntax nodes, because they do not have
// a unique location, and any two `Val`s representing
// the same value should be conceptually equal.

class Val : public NodeBase
{
    SLANG_ABSTRACT_CLASS(Val)

    typedef IValVisitor Visitor;

    void accept(IValVisitor* visitor, void* extra);

    // construct a new value by applying a set of parameter
    // substitutions to this one
    RefPtr<Val> substitute(ASTBuilder* astBuilder, SubstitutionSet subst);

    // Lower-level interface for substitution. Like the basic
    // `Substitute` above, but also takes a by-reference
    // integer parameter that should be incremented when
    // returning a modified value (this can help the caller
    // decide whether they need to do anything).
    RefPtr<Val> substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

    bool equalsVal(Val* val);
    String toString();
    HashCode getHashCode();
    bool operator == (const Val & v)
    {
        return equalsVal(const_cast<Val*>(&v));
    }
protected:

    RefPtr<Val> _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
    bool _equalsValOverride(Val* val);
    String _toStringOverride();
    HashCode _getHashCodeOverride();
};

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
    SLANG_ABSTRACT_CLASS(Type)

    friend struct ASTDumpAccess;

    typedef ITypeVisitor Visitor;

    void accept(ITypeVisitor* visitor, void* extra);

        /// Type derived types store the AST builder they were constructed on. The builder calls this function
        /// after constructing.
    SLANG_FORCE_INLINE void init(ASTNodeType inAstNodeType, ASTBuilder* inAstBuilder) { m_astBuilder = inAstBuilder; astNodeType = inAstNodeType; }

        /// Get the ASTBuilder that was used to construct this Type
    SLANG_FORCE_INLINE ASTBuilder* getASTBuilder() const { return m_astBuilder; }

    bool equals(Type* type);
    
    Type* getCanonicalType();

    ~Type();

protected:
    bool equalsImpl(Type* type);
    RefPtr<Type> createCanonicalType();

    RefPtr<Val> _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
    bool _equalsValOverride(Val* val);

    bool _equalsImplOverride(Type* type);
    RefPtr<Type> _createCanonicalTypeOverride();

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
    SLANG_ABSTRACT_CLASS(Substitutions)

    // The next outer that this one refines.
    RefPtr<Substitutions> outer;

    // Apply a set of substitutions to the bindings in this substitution
    RefPtr<Substitutions> applySubstitutionsShallow(ASTBuilder* astBuilder, SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff);

    // Check if these are equivalent substitutions to another set
    bool equals(Substitutions* subst);
    HashCode getHashCode() const;

protected:
    RefPtr<Substitutions> _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;
};

class GenericSubstitution : public Substitutions
{
    SLANG_CLASS(GenericSubstitution)

    // The generic declaration that defines the
    // parameters we are binding to arguments
    GenericDecl* genericDecl;

    // The actual values of the arguments
    List<RefPtr<Val> > args;

protected:
    RefPtr<Substitutions> _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;
};

class ThisTypeSubstitution : public Substitutions
{
    SLANG_CLASS(ThisTypeSubstitution)

    // The declaration of the interface that we are specializing
    InterfaceDecl* interfaceDecl = nullptr;

    // A witness that shows that the concrete type used to
    // specialize the interface conforms to the interface.
    RefPtr<SubtypeWitness> witness;

protected:
    // The actual type that provides the lookup scope for an associated type

    RefPtr<Substitutions> _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;
};

class GlobalGenericParamSubstitution : public Substitutions
{
    SLANG_CLASS(GlobalGenericParamSubstitution)
    // the type_param decl to be substituted
    GlobalGenericParamDecl* paramDecl;

    // the actual type to substitute in
    RefPtr<Type> actualType;

    struct ConstraintArg
    {
        RefPtr<Decl>    decl;
        RefPtr<Val>     val;
    };

    // the values that satisfy any constraints on the type parameter
    List<ConstraintArg> constraintArgs;

protected:
    RefPtr<Substitutions> _applySubstitutionsShallowOverride(ASTBuilder* astBuilder, SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff);
    bool _equalsOverride(Substitutions* subst);
    HashCode _getHashCodeOverride() const;
};

class SyntaxNode : public SyntaxNodeBase
{
    SLANG_ABSTRACT_CLASS(SyntaxNode);
};

//
// All modifiers are represented as full-fledged objects in the AST
// (that is, we don't use a bitfield, even for simple/common flags).
// This ensures that we can track source locations for all modifiers.
//
class Modifier : public SyntaxNode
{
    SLANG_ABSTRACT_CLASS(Modifier)
    typedef IModifierVisitor Visitor;

    void accept(IModifierVisitor* visitor, void* extra);

    // Next modifier in linked list of modifiers on same piece of syntax
    RefPtr<Modifier> next;

    // The keyword that was used to introduce t that was used to name this modifier.
    Name* name;

    Name* getName() { return name; }
    NameLoc getNameAndLoc() { return NameLoc(name, loc); }
};

// A syntax node which can have modifiers applied
class ModifiableSyntaxNode : public SyntaxNode
{
    SLANG_ABSTRACT_CLASS(ModifiableSyntaxNode)

    Modifiers modifiers;

    template<typename T>
    FilteredModifierList<T> getModifiersOfType() { return FilteredModifierList<T>(modifiers.first.Ptr()); }

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
    SLANG_ABSTRACT_CLASS(DeclBase)

    typedef IDeclVisitor Visitor;

    void accept(IDeclVisitor* visitor, void* extra);
};

class Decl : public DeclBase
{
public:
    SLANG_ABSTRACT_CLASS(Decl)

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
    SLANG_ABSTRACT_CLASS(Expr)

    typedef IExprVisitor Visitor;

    QualType type;

    void accept(IExprVisitor* visitor, void* extra);
};

class Stmt : public ModifiableSyntaxNode
{
    SLANG_ABSTRACT_CLASS(Stmt)

    typedef IStmtVisitor Visitor;

    void accept(IStmtVisitor* visitor, void* extra);
};


} // namespace Slang
