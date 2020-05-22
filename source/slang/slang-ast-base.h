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
   
    SyntaxClass<NodeBase> getClass() { return SyntaxClass<NodeBase>(&getClassInfo()); }
};

// Casting of NodeBase

template<typename T>
SLANG_FORCE_INLINE T* dynamicCast(NodeBase* node)
{
    return (node && node->getClassInfo().isSubClassOf(T::kReflectClassInfo)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE const T* dynamicCast(const NodeBase* node)
{
    return (node && node->getClassInfo().isSubClassOf(T::kReflectClassInfo)) ? static_cast<const T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE T* as(NodeBase* node)
{
    return (node && node->getClassInfo().isSubClassOf(T::kReflectClassInfo)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
SLANG_FORCE_INLINE const T* as(const NodeBase* node)
{
    return (node && node->getClassInfo().isSubClassOf(T::kReflectClassInfo)) ? static_cast<const T*>(node) : nullptr;
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
    RefPtr<Val> substitute(SubstitutionSet subst);

    // Lower-level interface for substitution. Like the basic
    // `Substitute` above, but also takes a by-reference
    // integer parameter that should be incremented when
    // returning a modified value (this can help the caller
    // decide whether they need to do anything).
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff);

    virtual bool equalsVal(Val* val) = 0;
    virtual String toString() = 0;
    virtual HashCode getHashCode() = 0;
    bool operator == (const Val & v)
    {
        return equalsVal(const_cast<Val*>(&v));
    }
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

public:
    Session* getSession() { return this->session; }
    void setSession(Session* s) { this->session = s; }

    bool equals(Type* type);
    
    Type* getCanonicalType();

    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;

    virtual bool equalsVal(Val* val) override;

    ~Type();

protected:
    virtual bool equalsImpl(Type* type) = 0;

    virtual RefPtr<Type> createCanonicalType() = 0;
    Type* canonicalType = nullptr;

    SLANG_UNREFLECTED
    Session* session = nullptr;
};

template <typename T>
SLANG_FORCE_INLINE T* as(Type* obj) { return obj ? dynamicCast<T>(obj->getCanonicalType()) : nullptr; }
template <typename T>
SLANG_FORCE_INLINE const T* as(const Type* obj) { return obj ? dynamicCast<T>(const_cast<Type*>(obj)->getCanonicalType()) : nullptr; }

// A substitution represents a binding of certain
// type-level variables to concrete argument values
class Substitutions: public RefObject
{
    SLANG_ABSTRACT_CLASS(Substitutions)

    // The next outer that this one refines.
    RefPtr<Substitutions> outer;

    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff) = 0;

    // Check if these are equivalent substitutions to another set
    virtual bool equals(Substitutions* subst) = 0;
    virtual HashCode getHashCode() const = 0;
};

class GenericSubstitution : public Substitutions
{
    SLANG_CLASS(GenericSubstitution)

    // The generic declaration that defines the
    // parameters we are binding to arguments
    GenericDecl* genericDecl;

    // The actual values of the arguments
    List<RefPtr<Val> > args;

    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)  override;

    // Check if these are equivalent substitutions to another set
    virtual bool equals(Substitutions* subst) override;

    virtual HashCode getHashCode() const override
    {
        HashCode rs = 0;
        for (auto && v : args)
        {
            rs ^= v->getHashCode();
            rs *= 16777619;
        }
        return rs;
    }
};

class ThisTypeSubstitution : public Substitutions
{
    SLANG_CLASS(ThisTypeSubstitution)

    // The declaration of the interface that we are specializing
    InterfaceDecl* interfaceDecl = nullptr;

    // A witness that shows that the concrete type used to
    // specialize the interface conforms to the interface.
    RefPtr<SubtypeWitness> witness;

    // The actual type that provides the lookup scope for an associated type
    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)  override;

    // Check if these are equivalent substitutions to another set
    virtual bool equals(Substitutions* subst) override;

    virtual HashCode getHashCode() const override;
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

    // Apply a set of substitutions to the bindings in this substitution
    virtual RefPtr<Substitutions> applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)  override;

    // Check if these are equivalent substitutions to another set
    virtual bool equals(Substitutions* subst) override;

    virtual HashCode getHashCode() const override
    {
        HashCode rs = actualType->getHashCode();
        for (auto && a : constraintArgs)
        {
            rs = combineHash(rs, a.val->getHashCode());
        }
        return rs;
    }
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
