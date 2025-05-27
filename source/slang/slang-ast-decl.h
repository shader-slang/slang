// slang-decl-defs.h

#pragma once

#include "slang-ast-base.h"
#include "slang-ast-decl.h.fiddle"

FIDDLE()
namespace Slang
{

// Syntax class definitions for declarations.

// A group of declarations that should be treated as a unit
FIDDLE()
class DeclGroup : public DeclBase
{
    FIDDLE(...)
    FIDDLE() List<Decl*> decls;
};

FIDDLE()
class UnresolvedDecl : public Decl
{
    FIDDLE(...)
};

/// Holds the direct member declarations of a `ContainerDecl`.
///
/// This type is used to encapsulate the logic the creating
/// and maintaing the acceleration structures used for member
/// lookup.
///
struct ContainerDeclDirectMemberDecls
{
public:
    List<Decl*> const& getDecls() const { return decls; }

    List<Decl*>& _refDecls() { return decls; }

private:
    friend class ContainerDecl;
    friend struct ASTDumpContext;

    List<Decl*> decls;

    struct
    {
        Count declCountWhenLastUpdated = 0;

        Dictionary<Name*, Decl*> mapNameToLastDeclOfThatName;
        List<Decl*> filteredListOfTransparentDecls;
    } accelerators;
};

/// A conceptual list of declarations of the same name, in the same container.
struct DeclsOfNameList
{
public:
    DeclsOfNameList() {}

    explicit DeclsOfNameList(Decl* decl)
        : _lastDecl(decl)
    {
    }

    struct Iterator
    {
    public:
        Iterator() {}
        Iterator(Decl* decl)
            : _decl(decl)
        {
        }

        Decl* operator*() const { return _decl; }

        Iterator& operator++()
        {
            SLANG_ASSERT(_decl);
            _decl = _decl->_prevInContainerWithSameName;
            return *this;
        }

        bool operator!=(const Iterator& other) const { return _decl != other._decl; }

    private:
        Decl* _decl = nullptr;
    };

    Iterator begin() const { return _lastDecl; }
    Iterator end() const { return nullptr; }

private:
    Decl* _lastDecl = nullptr;
};

// A "container" decl is a parent to other declarations
FIDDLE(abstract)
class ContainerDecl : public Decl
{
    FIDDLE(...)

    SourceLoc closingSourceLoc;

    // The associated scope owned by this decl.
    Scope* ownedScope = nullptr;

    /// Get all of the direct member declarations inside this container decl.
    ///
    List<Decl*> const& getDirectMemberDecls();

    /// Get the number of direct member declarations inside this container decl.
    ///
    Count getDirectMemberDeclCount();

    /// Get the direct member declaration of this container decl at the given `index`.
    ///
    Decl* getDirectMemberDecl(Index index);

    /// Get the first direct member declaration inside of this container decl, if any.
    ///
    /// If the container has no direct member declarations, returns null.
    ///
    Decl* getFirstDirectMemberDecl();

    /// Get all of the direct member declarations inside of this container decl
    /// that are instances of the specified AST node type `T`.
    ///
    template<typename T>
    FilteredMemberList<T> getDirectMemberDeclsOfType()
    {
        return FilteredMemberList<T>(getDirectMemberDecls());
    }

    /// Find the first direct member declaration of this container decl that
    /// is an instance of the specified AST node type `T`.
    ///
    /// If there are no direct member declrations of type `T`, then returns null.
    /// Otherwise, returns the first matching member, in declaration order.
    ///
    template<typename T>
    T* findFirstDirectMemberDeclOfType()
    {
        auto count = getDirectMemberDeclCount();
        for (Index i = 0; i < count; ++i)
        {
            auto decl = getDirectMemberDecl(i);
            if (auto found = as<T>(decl))
                return found;
        }
        return nullptr;
    }

    /// Find all direct member declarations of this container decl that have the given name.
    ///
    DeclsOfNameList getDirectMemberDeclsOfName(Name* name);

    /// Find the last direct member declaration of this container decl that has the given name.
    ///
    Decl* findLastDirectMemberDeclOfName(Name* name);

    /// Get the previous direct member declaration that has the same name as `decl`.
    ///
    Decl* getPrevDirectMemberDeclWithSameName(Decl* decl);

    /// Append the given `decl` to the direct member declarations of this container decl.
    ///
    void addDirectMemberDecl(Decl* decl);

    /// Get the subset of direct member declarations that are "transparent."
    ///
    /// Transparent members will themselves be considered when performing
    /// looking on the parent. E.g., if `a` has a transparent member `b`,
    /// then a lookup like `a.x` will also consider `a.b.x` as a possible
    /// result.
    ///
    List<Decl*> const& getTransparentDirectMemberDecls();

    // Note: Just an alias for `getDirectMemberDecls()`,
    // but left in place because of just how many call sites were
    // already using this name.
    //
    List<Decl*> const& getMembers() { return getDirectMemberDecls(); }

    // Note: Just an alias for `getDirectMemberDeclsOfType()`,
    // but left in place because of just how many call sites were
    // already using this name.
    //
    template<typename T>
    FilteredMemberList<T> getMembersOfType()
    {
        return getDirectMemberDeclsOfType<T>();
    }

    // Note: Just an alias for `addDirectMemberDecl()`,
    // but left in place because of just how many call sites were
    // already using this name.
    //
    void addMember(Decl* member) { addDirectMemberDecl(member); }

    //
    // NOTE: The operations after this point are *not* considered part of
    // the public API of `ContainerDecl`, and new code should not be
    // written that uses them.
    //
    // They are being left in place because the existing code that uses
    // them would be difficult or impossible to refactor to use the public
    // API, but such parts of the codebase should *not* be considered as
    // examples of how to interact with the Slang AST.
    //

    /// Invalidate the acceleration structures used for declaration lookup,
    /// because the code is about to replace `unscopedEnumAttr` with `transparentModifier`
    /// as part of semantic checking of an `EnumDecl` nested under this `ContainerDecl`.
    ///
    /// Cannot be expressed in terms of the rest of the public API because the
    /// existing assumption has been that any needed `TransparentModifier`s would
    /// be manifestly obvious just from the syntax being parsed, so that they would
    /// already be in place on parsed ASTs. the `UnscopedEnumAttribute` is a `TransparentModifier`
    /// in all but name, but the two don't share a common base class such that code
    /// could check for them together.
    ///
    /// TODO: In the long run, the obvious fix is to eliminate `UnscopedEnumAttribute`,
    /// becuase it only exists to enable legacy code to be expressed in Slang, rather than
    /// representing anything we want/intend to support long-term.
    ///
    /// TODO: In the even *longer* run, we should eliminate `TransparentModifier` as well,
    /// because it only exists to support legacy `cbuffer` declarations and similar syntax,
    /// and those should be deprecated over time.
    ///
    void _invalidateLookupAcceleratorsBecauseUnscopedEnumAttributeWillBeTurnedIntoTransparentModifier(
        UnscopedEnumAttribute* unscopedEnumAttr,
        TransparentModifier* transparentModifier);

    /// Remove a constructor declaration from the direct member declarations of this container.
    ///
    /// This operation is seemingly used when a default constructor declaration has been synthesized
    /// for a type, but that type already contained a default constructor of its own.
    ///
    /// TODO: Somebody should investigate why this operation is even needed; it seems like an
    /// indication that we are doing something Deeply Wrong in the way that default constructors are
    /// being handled and synthesized (on top of the things that we know are Wrong By Design).
    ///
    void _removeDirectMemberConstructorDeclBecauseSynthesizedAnotherDefaultConstructorInstead(
        ConstructorDecl* decl);

    /// Replace the given `oldVarDecl` with the given `newPropertyDecl` in the direct member
    /// declarations of this container decl, because the variable declaration had a bit-field
    /// specification on it, and the property was synthesized to stand in for that variable
    /// by providing a getter/settter pair.
    ///
    /// This operation cannot be expressed in terms of the rest of the public API, because there
    /// is currently no other example of parsing a declaration as one AST node class, and
    /// then determining as part of semantic checking that it should *actually* be represented
    /// as a different class of declaration entirely.
    ///
    /// TODO: In the long run we would either eliminate support for C-style bit-field specifications
    /// on what would otherwise be an ordinary member variable declaration. Some other syntax, or
    /// a type-based solution, should be introduced to server the same use cases, without requiring
    /// us to parse something as a variable that is semantically *not* a variable in almost any
    /// of the ways that count.
    ///
    void _replaceDirectMemberBitFieldVariableDeclAtIndexWithPropertyDeclThatWasSynthesizedForIt(
        Index index,
        VarDecl* oldVarDecl,
        PropertyDecl* newPropertyDecl);

    /// Insert `backingVarDecl` into this container declaration at `index`, to handle the case
    /// where the backing variable has been synthesized to store the bits of one or more bitfield
    /// properties.
    ///
    /// This operation cannot be expressed in terms of the rest of the public API because the usual
    /// assumption is that member declarations may be added to a declaration but that, once added,
    /// their indices in the member list are consistent and stable.
    ///
    /// The reason the code that calls this operation can't just add `backingVarDecl` to the end of
    /// the declaration is that there is an underlying assumption made by users that any
    /// non-bitfield members before/after a bitfield declaration will have their storage laid out
    /// before/after the storage for that bitfield.
    ///
    /// TODO: A simple cleanup would be to *not* guarantee the order of storage layout for bitfield
    /// members relative to other member variables, but the real long-term fix is to have an
    /// alternative means for users to define bitfields that does not inherit the C-like syntax, and
    /// that can make the storage that is introduced clear at parse time, so that the relevant
    /// declaration(s) can already be in the correct order.
    ///
    void _insertDirectMemberDeclAtIndexForBitfieldPropertyBackingMember(
        Index index,
        VarDecl* backingVarDecl);

    // TODO: The following should be a private member, but currently
    // we have auto-generated code for things like dumping and serialization
    // that expect to have access to it.
    //
    FIDDLE() ContainerDeclDirectMemberDecls _directMemberDecls;

private:
    bool _areLookupAcceleratorsValid();
    void _invalidateLookupAccelerators();
    void _ensureLookupAcceleratorsAreValid();
};

// Base class for all variable declarations
FIDDLE(abstract)
class VarDeclBase : public Decl
{
    FIDDLE(...)

    // type of the variable
    FIDDLE() TypeExp type;

    Type* getType() { return type.type; }

    // Initializer expression (optional)
    FIDDLE() Expr* initExpr = nullptr;

    // Folded IntVal if the initializer is a constant integer.
    FIDDLE() IntVal* val = nullptr;
};

// Ordinary potentially-mutable variables (locals, globals, and member variables)
FIDDLE()
class VarDecl : public VarDeclBase
{
    FIDDLE(...)
};

// A variable declaration that is always immutable (whether local, global, or member variable)
FIDDLE()
class LetDecl : public VarDecl
{
    FIDDLE(...)
};

// An `AggTypeDeclBase` captures the shared functionality
// between true aggregate type declarations and extension
// declarations:
//
// - Both can contain members (they are `ContainerDecl`s)
// - Both can have declared bases
// - Both expose a `this` variable in their body
//
FIDDLE(abstract)
class AggTypeDeclBase : public ContainerDecl
{
    FIDDLE(...)
};

// An extension to apply to an existing type
FIDDLE()
class ExtensionDecl : public AggTypeDeclBase
{
    FIDDLE(...)
    FIDDLE() TypeExp targetType;
};

enum class TypeTag
{
    None = 0,
    Unsized = 1,
    Incomplete = 2,
    LinkTimeSized = 4,
    Opaque = 8,
};

// Declaration of a type that represents some sort of aggregate
FIDDLE(abstract)
class AggTypeDecl : public AggTypeDeclBase
{
    FIDDLE(...)
    FIDDLE() TypeTag typeTags = TypeTag::None;

    // Used if this type declaration is a wrapper, i.e. struct FooWrapper:IFoo = Foo;
    TypeExp wrappedType;
    bool hasBody = true;

    void unionTagsWith(TypeTag other);
    void addTag(TypeTag tag);
    bool hasTag(TypeTag tag);

    FilteredMemberList<VarDecl> getFields() { return getMembersOfType<VarDecl>(); }
};

FIDDLE()
class StructDecl : public AggTypeDecl
{
    FIDDLE(...)

    // We will use these auxiliary to help in synthesizing the member initialize constructor.
    Slang::HashSet<VarDeclBase*> m_membersVisibleInCtor;
};

FIDDLE()
class ClassDecl : public AggTypeDecl
{
    FIDDLE(...)
};

FIDDLE()
class GLSLInterfaceBlockDecl : public AggTypeDecl
{
    FIDDLE(...)
};

// TODO: Is it appropriate to treat an `enum` as an aggregate type?
// Most code that looks for, e.g., conformances assumes user-defined
// types are all `AggTypeDecl`, so this is the right choice for now
// if we want `enum` types to be able to implement interfaces, etc.
//
FIDDLE()
class EnumDecl : public AggTypeDecl
{
    FIDDLE(...)
    FIDDLE() Type* tagType = nullptr;
};

// A single case in an enum.
//
// E.g., in a declaration like:
//
//      enum Color { Red = 0, Green, Blue };
//
// The `Red = 0` is the declaration of the `Red`
// case, with `0` as an explicit expression for its
// _tag value_.
//
FIDDLE()
class EnumCaseDecl : public Decl
{
    FIDDLE(...)
    // type of the parent `enum`
    FIDDLE() TypeExp type;

    Type* getType() { return type.type; }

    // Tag value
    FIDDLE() Expr* tagExpr = nullptr;

    FIDDLE() IntVal* tagVal = nullptr;
};

// A member of InterfaceDecl representing the abstract ThisType.
FIDDLE()
class ThisTypeDecl : public AggTypeDecl
{
    FIDDLE(...)
};

// An interface which other types can conform to
FIDDLE()
class InterfaceDecl : public AggTypeDecl
{
    FIDDLE(...)
    ThisTypeDecl* getThisTypeDecl();
};

FIDDLE(abstract)
class TypeConstraintDecl : public Decl
{
    FIDDLE(...)
    const TypeExp& getSup() const;
    // Overrides should be public so base classes can access
    // Implement _getSupOverride on derived classes to change behavior of getSup, as if getSup is
    // virtual
    const TypeExp& _getSupOverride() const;
};

FIDDLE()
class ThisTypeConstraintDecl : public TypeConstraintDecl
{
    FIDDLE(...)
    FIDDLE() TypeExp base;
    const TypeExp& _getSupOverride() const { return base; }
    InterfaceDecl* getInterfaceDecl();
};

// A kind of pseudo-member that represents an explicit
// or implicit inheritance relationship.
//
FIDDLE()
class InheritanceDecl : public TypeConstraintDecl
{
    FIDDLE(...)
    // The type expression as written
    FIDDLE() TypeExp base;

    // After checking, this dictionary will map members
    // required by the base type to their concrete
    // implementations in the type that contains
    // this inheritance declaration.
    FIDDLE() RefPtr<WitnessTable> witnessTable;

    // Overrides should be public so base classes can access
    const TypeExp& _getSupOverride() const { return base; }
};

// TODO: may eventually need sub-classes for explicit/direct vs. implicit/indirect inheritance


// A declaration that represents a simple (non-aggregate) type
//
// TODO: probably all types will be aggregate decls eventually,
// so that we can easily store conformances/constraints on type variables
FIDDLE(abstract)
class SimpleTypeDecl : public Decl
{
    FIDDLE(...)
};

// A `typedef` declaration
FIDDLE()
class TypeDefDecl : public SimpleTypeDecl
{
    FIDDLE(...)
    FIDDLE() TypeExp type;
};

FIDDLE()
class TypeAliasDecl : public TypeDefDecl
{
    FIDDLE(...)
};

// An 'assoctype' declaration, it is a container of inheritance clauses
FIDDLE()
class AssocTypeDecl : public AggTypeDecl
{
    FIDDLE(...)
};

// A 'type_param' declaration, which defines a generic
// entry-point parameter. Is a container of GenericTypeConstraintDecl
FIDDLE()
class GlobalGenericParamDecl : public AggTypeDecl
{
    FIDDLE(...)
};

// A `__generic_value_param` declaration, which defines an existential
// value parameter (not a type parameter.
FIDDLE()
class GlobalGenericValueParamDecl : public VarDeclBase
{
    FIDDLE(...)
};

// A scope for local declarations (e.g., as part of a statement)
FIDDLE()
class ScopeDecl : public ContainerDecl
{
    FIDDLE(...)
};

// A function/initializer/subscript parameter (potentially mutable)
FIDDLE()
class ParamDecl : public VarDeclBase
{
    FIDDLE(...)
};

// A parameter of a function declared in "modern" types (immutable unless explicitly `out` or
// `inout`)
FIDDLE()
class ModernParamDecl : public ParamDecl
{
    FIDDLE(...)
};

// Base class for things that have parameter lists and can thus be applied to arguments ("called")
FIDDLE(abstract)
class CallableDecl : public ContainerDecl
{
    FIDDLE(...)
    FilteredMemberList<ParamDecl> getParameters() { return getMembersOfType<ParamDecl>(); }

    FIDDLE() TypeExp returnType;

    // If this callable throws an error code, `errorType` is the type of the error code.
    FIDDLE() TypeExp errorType;

    // Fields related to redeclaration, so that we
    // can support multiple specialized variations
    // of the "same" logical function.
    //
    // This should also help us to support redeclaration
    // of functions when handling HLSL/GLSL.

    // The "primary" declaration of the function, which will
    // be used whenever we need to unique things.
    FIDDLE() CallableDecl* primaryDecl = nullptr;

    // The next declaration of the "same" function (that is,
    // with the same `primaryDecl`).
    FIDDLE() CallableDecl* nextDecl = nullptr;
};

// Base class for callable things that may also have a body that is evaluated to produce their
// result
FIDDLE(abstract)
class FunctionDeclBase : public CallableDecl
{
    FIDDLE(...)
    FIDDLE() Stmt* body = nullptr;
};

// A constructor/initializer to create instances of a type
FIDDLE()
class ConstructorDecl : public FunctionDeclBase
{
    FIDDLE(...)
    enum class ConstructorFlavor : int
    {
        UserDefined = 0x00,
        // Indicates whether the declaration was synthesized by
        // Slang and not explicitly provided by the user
        SynthesizedDefault = 0x01,
        // Member initialize constructor is a synthesized ctor,
        // but it takes parameters.
        SynthesizedMemberInit = 0x02
    };

    FIDDLE() int m_flavor = (int)ConstructorFlavor::UserDefined;
    void addFlavor(ConstructorFlavor flavor) { m_flavor |= (int)flavor; }
    bool containsFlavor(ConstructorFlavor flavor) { return m_flavor & (int)flavor; }
};

FIDDLE()
class LambdaDecl : public StructDecl
{
    FIDDLE(...)

    FIDDLE() FunctionDeclBase* funcDecl;
};

// A subscript operation used to index instances of a type
FIDDLE()
class SubscriptDecl : public CallableDecl
{
    FIDDLE(...)
};

/// A property declaration that abstracts over storage with a getter/setter/etc.
FIDDLE()
class PropertyDecl : public ContainerDecl
{
    FIDDLE(...)
    FIDDLE() TypeExp type;
};

// An "accessor" for a subscript or property
FIDDLE(abstract)
class AccessorDecl : public FunctionDeclBase
{
    FIDDLE(...)
};

FIDDLE()
class GetterDecl : public AccessorDecl
{
    FIDDLE(...)
};

FIDDLE()
class SetterDecl : public AccessorDecl
{
    FIDDLE(...)
};

FIDDLE()
class RefAccessorDecl : public AccessorDecl
{
    FIDDLE(...)
};

FIDDLE()
class FuncDecl : public FunctionDeclBase
{
    FIDDLE(...)
};

FIDDLE(abstract)
class NamespaceDeclBase : public ContainerDecl
{
    FIDDLE(...)
};

// A `namespace` declaration inside some module, that provides
// a named scope for declarations inside it.
//
// Note: Multiple `namespace` declarations with the same name
// in a given module/file will be collapsed into a single
// `NamespaceDecl` during parsing, so this declaration does
// not directly represent what is present in the input syntax.
//
FIDDLE()
class NamespaceDecl : public NamespaceDeclBase
{
    FIDDLE(...)
};

// A "module" of code (essentially, a single translation unit)
// that provides a scope for some number of declarations.
FIDDLE()
class ModuleDecl : public NamespaceDeclBase
{
    FIDDLE(...)

    // The API-level module that this declaration belong to.
    //
    // This field allows lookup of the `Module` based on a
    // declaration nested under a `ModuleDecl` by following
    // its chain of parents.
    //
    Module* module = nullptr;

    /// Map a decl to the list of its associated decls.
    ///
    /// This mapping is filled in during semantic checking, as the decl declarations get checked or
    /// generated.
    ///
    FIDDLE() OrderedDictionary<Decl*, RefPtr<DeclAssociationList>> mapDeclToAssociatedDecls;

    /// Whether the module is defined in legacy language.
    /// The legacy Slang language does not have visibility modifiers and everything is treated as
    /// `public`. Newer version of the language introduces visibility and makes `internal` as the
    /// default. To prevent this from breaking existing code, we need to know whether a module is
    /// written in the legacy language. We detect this by checking whether the module has any
    /// visibility modifiers, or if the module uses new language constructs, e.g. `module`,
    /// `__include`,
    /// `__implementing` etc.
    FIDDLE() bool isInLegacyLanguage = true;

    FIDDLE() DeclVisibility defaultVisibility = DeclVisibility::Internal;

    /// Map a type to the list of extensions of that type (if any) declared in this module
    ///
    /// This mapping is filled in during semantic checking, as `ExtensionDecl`s get checked.
    ///
    FIDDLE() Dictionary<AggTypeDecl*, RefPtr<CandidateExtensionList>> mapTypeToCandidateExtensions;
};

// Represents a transparent scope of declarations that are defined in a single source file.
FIDDLE()
class FileDecl : public ContainerDecl
{
    FIDDLE(...)
};

/// A declaration that brings members of another declaration or namespace into scope
FIDDLE()
class UsingDecl : public Decl
{
    FIDDLE(...)

    /// An expression that identifies the entity (e.g., a namespace) to be brought into `scope`
    Expr* arg = nullptr;

    /// The scope that the entity named by `arg` will be brought into
    Scope* scope = nullptr;
};

FIDDLE()
class FileReferenceDeclBase : public Decl
{
    FIDDLE(...)

    // The name of the module we are trying to import
    NameLoc moduleNameAndLoc;

    SourceLoc startLoc;
    SourceLoc endLoc;

    // The scope that we want to import into
    Scope* scope = nullptr;
};

FIDDLE()
class ImportDecl : public FileReferenceDeclBase
{
    FIDDLE(...)

    // The module that actually got imported
    FIDDLE() ModuleDecl* importedModuleDecl = nullptr;
};

FIDDLE(abstract)
class IncludeDeclBase : public FileReferenceDeclBase
{
    FIDDLE(...)
    FileDecl* fileDecl = nullptr;
};

FIDDLE()
class IncludeDecl : public IncludeDeclBase
{
    FIDDLE(...)
};

FIDDLE()
class ImplementingDecl : public IncludeDeclBase
{
    FIDDLE(...)
};

FIDDLE()
class ModuleDeclarationDecl : public Decl
{
    FIDDLE(...)
};

FIDDLE()
class RequireCapabilityDecl : public Decl
{
    FIDDLE(...)
};

// A generic declaration, parameterized on types/values
FIDDLE()
class GenericDecl : public ContainerDecl
{
    FIDDLE(...)
    // The decl that is genericized...
    FIDDLE() Decl* inner = nullptr;
};

FIDDLE(abstract)
class GenericTypeParamDeclBase : public SimpleTypeDecl
{
    FIDDLE(...)
    // The index of the generic parameter.
    int parameterIndex = -1;
};

FIDDLE()
class GenericTypeParamDecl : public GenericTypeParamDeclBase
{
    FIDDLE(...)
    // The bound for the type parameter represents a trait that any
    // type used as this parameter must conform to
    //            TypeExp bound;

    // The "initializer" for the parameter represents a default value
    FIDDLE() TypeExp initType;
};

FIDDLE()
class GenericTypePackParamDecl : public GenericTypeParamDeclBase
{
    FIDDLE(...)
};

// A constraint placed as part of a generic declaration
FIDDLE()
class GenericTypeConstraintDecl : public TypeConstraintDecl
{
    FIDDLE(...)
    // A type constraint like `T : U` is constraining `T` to be "below" `U`
    // on a lattice of types. This may not be a subtyping relationship
    // per se, but it makes sense to use that terminology here, so we
    // think of these fields as the sub-type and super-type, respectively.
    FIDDLE() TypeExp sub;
    FIDDLE() TypeExp sup;

    // If this decl is defined in a where clause, store the source location of the where token.
    SourceLoc whereTokenLoc = SourceLoc();

    FIDDLE() bool isEqualityConstraint = false;

    // Overrides should be public so base classes can access
    const TypeExp& _getSupOverride() const { return sup; }
};

FIDDLE()
class TypeCoercionConstraintDecl : public Decl
{
    FIDDLE(...)
    SourceLoc whereTokenLoc = SourceLoc();
    FIDDLE() TypeExp fromType;
    FIDDLE() TypeExp toType;
};

FIDDLE()
class GenericValueParamDecl : public VarDeclBase
{
    FIDDLE(...)
    // The index of the generic parameter.
    int parameterIndex = 0;
};

// An empty declaration (which might still have modifiers attached).
//
// An empty declaration is uncommon in HLSL, but
// in GLSL it is often used at the global scope
// to declare metadata that logically belongs
// to the entry point, e.g.:
//
//     layout(local_size_x = 16) in;
//
FIDDLE()
class EmptyDecl : public Decl
{
    FIDDLE(...)
};

// A declaration used by the implementation to put syntax keywords
// into the current scope.
//
FIDDLE()
class SyntaxDecl : public Decl
{
    FIDDLE(...)
    // What type of syntax node will be produced when parsing with this keyword?
    FIDDLE() SyntaxClass<NodeBase> syntaxClass;

    // Callback to invoke in order to parse syntax with this keyword.
    SyntaxParseCallback parseCallback = nullptr;
    void* parseUserData = nullptr;
};

// A declaration of an attribute to be used with `[name(...)]` syntax.
//
FIDDLE()
class AttributeDecl : public ContainerDecl
{
    FIDDLE(...)
    // What type of syntax node will be produced to represent this attribute.
    FIDDLE() SyntaxClass<NodeBase> syntaxClass;
};

// A synthesized decl used as a placeholder for a differentiable function requirement. This decl
// will be a child of interface decl. This allows us to form an interface requirement key for the
// derivative of an interface function. The synthesized `DerivativeRequirementDecl` will be a child
// of the original function requirement decl after an interface type is checked.
FIDDLE()
class DerivativeRequirementDecl : public FunctionDeclBase
{
    FIDDLE(...)
    // The original requirement decl.
    FIDDLE() Decl* originalRequirementDecl = nullptr;

    // Type to use for 'ThisType'
    FIDDLE() Type* diffThisType;
};

// A reference to a synthesized decl representing a differentiable function requirement, this decl
// will be a child in the orignal function.
FIDDLE()
class DerivativeRequirementReferenceDecl : public Decl
{
    FIDDLE(...)
    FIDDLE() DerivativeRequirementDecl* referencedDecl;
};

FIDDLE()
class ForwardDerivativeRequirementDecl : public DerivativeRequirementDecl
{
    FIDDLE(...)
};

FIDDLE()
class BackwardDerivativeRequirementDecl : public DerivativeRequirementDecl
{
    FIDDLE(...)
};

bool isInterfaceRequirement(Decl* decl);
InterfaceDecl* findParentInterfaceDecl(Decl* decl);

bool isLocalVar(const Decl* decl);


// Add a sibling lookup scope for `dest` to refer to `source`.
void addSiblingScopeForContainerDecl(
    ASTBuilder* builder,
    ContainerDecl* dest,
    ContainerDecl* source);
void addSiblingScopeForContainerDecl(ASTBuilder* builder, Scope* destScope, ContainerDecl* source);

} // namespace Slang
