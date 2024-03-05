// slang-decl-defs.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

// Syntax class definitions for declarations.

// A group of declarations that should be treated as a unit
class DeclGroup: public DeclBase
{
    SLANG_AST_CLASS(DeclGroup)

    List<Decl*> decls;
};

class UnresolvedDecl : public Decl
{
    SLANG_AST_CLASS(UnresolvedDecl)
};

// A "container" decl is a parent to other declarations
class ContainerDecl: public Decl
{
    SLANG_ABSTRACT_AST_CLASS(ContainerDecl)

    List<Decl*> members;
    SourceLoc closingSourceLoc;

    // The associated scope owned by this decl.
    Scope* ownedScope = nullptr;

    template<typename T>
    FilteredMemberList<T> getMembersOfType()
    {
        return FilteredMemberList<T>(members);
    }

    void buildMemberDictionary();

    bool isMemberDictionaryValid() const { return dictionaryLastCount == members.getCount(); }

    void invalidateMemberDictionary() { dictionaryLastCount = -1; }

    Dictionary<Name*, Decl*>& getMemberDictionary()
    {
        buildMemberDictionary();
        return memberDictionary;
    }

    List<TransparentMemberInfo>& getTransparentMembers()
    {
        buildMemberDictionary();
        return transparentMembers;
    }

    void addMember(Decl* member)
    {
        if (member)
        {
            member->parentDecl = this;
            members.add(member);
        }
    }

    SLANG_UNREFLECTED   // We don't want to reflect the following fields

private:
    // Denotes how much of Members has been placed into the dictionary/transparentMembers.
    // If this value equals the Members.getCount(), the dictionary is completely full and valid.
    // If it's >= 0, then the Members after dictionaryLastCount are all that need to be added.
    // If it < 0 it means that the dictionary/transparentMembers is invalid and needs to be recreated.
    Index dictionaryLastCount = 0;

    // Dictionary for looking up members by name.
    // This is built on demand before performing lookup.
    Dictionary<Name*, Decl*> memberDictionary;
    
    // A list of transparent members, to be used in lookup
    // Note: this is only valid if `memberDictionaryIsValid` is true
    List<TransparentMemberInfo> transparentMembers;
};

// Base class for all variable declarations
class VarDeclBase : public Decl
{
    SLANG_ABSTRACT_AST_CLASS(VarDeclBase)

    // type of the variable
    TypeExp type;

    Type* getType() { return type.type; }

    // Initializer expression (optional)
    Expr* initExpr = nullptr;
};

// Ordinary potentially-mutable variables (locals, globals, and member variables)
class VarDecl : public VarDeclBase
{
    SLANG_AST_CLASS(VarDecl)
};

// A variable declaration that is always immutable (whether local, global, or member variable)
class LetDecl : public VarDecl
{
    SLANG_AST_CLASS(LetDecl)
};

    // An `AggTypeDeclBase` captures the shared functionality
    // between true aggregate type declarations and extension
    // declarations:
    //
    // - Both can contain members (they are `ContainerDecl`s)
    // - Both can have declared bases
    // - Both expose a `this` variable in their body
    //
class AggTypeDeclBase : public ContainerDecl
{
    SLANG_ABSTRACT_AST_CLASS(AggTypeDeclBase);
};

// An extension to apply to an existing type
class ExtensionDecl : public AggTypeDeclBase
{
    SLANG_AST_CLASS(ExtensionDecl)

    TypeExp targetType;
};

enum class TypeTag
{
    None = 0,
    Unsized = 1,
    Incomplete = 2
};

// Declaration of a type that represents some sort of aggregate
class AggTypeDecl : public  AggTypeDeclBase
{
    SLANG_ABSTRACT_AST_CLASS(AggTypeDecl)

    TypeTag typeTags = TypeTag::None;

    // Used if this type declaration is a wrapper, i.e. struct FooWrapper:IFoo = Foo;
    TypeExp wrappedType;
    bool hasBody = true;

    void unionTagsWith(TypeTag other);
    void addTag(TypeTag tag);
    bool hasTag(TypeTag tag);

    FilteredMemberList<VarDecl> getFields()
    {
        return getMembersOfType<VarDecl>();
    }
};

class StructDecl: public AggTypeDecl
{
    SLANG_AST_CLASS(StructDecl);
};

class ClassDecl : public AggTypeDecl
{
    SLANG_AST_CLASS(ClassDecl)
};

class GLSLInterfaceBlockDecl : public AggTypeDecl
{
    SLANG_AST_CLASS(GLSLInterfaceBlockDecl);
};

// TODO: Is it appropriate to treat an `enum` as an aggregate type?
// Most code that looks for, e.g., conformances assumes user-defined
// types are all `AggTypeDecl`, so this is the right choice for now
// if we want `enum` types to be able to implement interfaces, etc.
//
class EnumDecl : public AggTypeDecl
{
    SLANG_AST_CLASS(EnumDecl)

    Type* tagType = nullptr;
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
class EnumCaseDecl : public Decl
{
    SLANG_AST_CLASS(EnumCaseDecl)

    // type of the parent `enum`
    TypeExp type;

    Type* getType() { return type.type; }

    // Tag value
    Expr* tagExpr = nullptr;
};

// A member of InterfaceDecl representing the abstract ThisType.
class ThisTypeDecl : public AggTypeDecl
{
    SLANG_AST_CLASS(ThisTypeDecl)
};

// An interface which other types can conform to
class InterfaceDecl : public  AggTypeDecl
{
    SLANG_AST_CLASS(InterfaceDecl)

    ThisTypeDecl* getThisTypeDecl();
};

class TypeConstraintDecl : public  Decl
{
    SLANG_ABSTRACT_AST_CLASS(TypeConstraintDecl)

    const TypeExp& getSup() const;
    // Overrides should be public so base classes can access
    // Implement _getSupOverride on derived classes to change behavior of getSup, as if getSup is virtual
    const TypeExp& _getSupOverride() const;
};

class ThisTypeConstraintDecl : public TypeConstraintDecl
{
    SLANG_AST_CLASS(ThisTypeConstraintDecl)

    TypeExp base;
    const TypeExp& _getSupOverride() const { return base; }
    InterfaceDecl* getInterfaceDecl();
};

// A kind of pseudo-member that represents an explicit
// or implicit inheritance relationship.
//
class InheritanceDecl : public TypeConstraintDecl
{
    SLANG_AST_CLASS(InheritanceDecl)

    // The type expression as written
    TypeExp base;

    // After checking, this dictionary will map members
    // required by the base type to their concrete
    // implementations in the type that contains
    // this inheritance declaration.
    RefPtr<WitnessTable> witnessTable;

    // Overrides should be public so base classes can access
    const TypeExp& _getSupOverride() const { return base; }
};

// TODO: may eventually need sub-classes for explicit/direct vs. implicit/indirect inheritance


// A declaration that represents a simple (non-aggregate) type
//
// TODO: probably all types will be aggregate decls eventually,
// so that we can easily store conformances/constraints on type variables
class SimpleTypeDecl : public Decl
{
    SLANG_ABSTRACT_AST_CLASS(SimpleTypeDecl)
};

// A `typedef` declaration
class TypeDefDecl : public SimpleTypeDecl
{
    SLANG_AST_CLASS(TypeDefDecl)
   
    TypeExp type;
};

class TypeAliasDecl : public TypeDefDecl
{
    SLANG_AST_CLASS(TypeAliasDecl)
};

// An 'assoctype' declaration, it is a container of inheritance clauses
class AssocTypeDecl : public AggTypeDecl
{
    SLANG_AST_CLASS(AssocTypeDecl)
};

// A 'type_param' declaration, which defines a generic
// entry-point parameter. Is a container of GenericTypeConstraintDecl
class GlobalGenericParamDecl : public AggTypeDecl
{
    SLANG_AST_CLASS(GlobalGenericParamDecl)
};

// A `__generic_value_param` declaration, which defines an existential
// value parameter (not a type parameter.
class GlobalGenericValueParamDecl : public VarDeclBase
{
    SLANG_AST_CLASS(GlobalGenericValueParamDecl)
};

// A scope for local declarations (e.g., as part of a statement)
class ScopeDecl : public  ContainerDecl
{
    SLANG_AST_CLASS(ScopeDecl)
};

// A function/initializer/subscript parameter (potentially mutable)
class ParamDecl : public VarDeclBase
{
    SLANG_AST_CLASS(ParamDecl)
};

// A parameter of a function declared in "modern" types (immutable unless explicitly `out` or `inout`)
class ModernParamDecl : public ParamDecl
{
    SLANG_AST_CLASS(ModernParamDecl)
};

// Base class for things that have parameter lists and can thus be applied to arguments ("called")
class CallableDecl : public ContainerDecl
{
    SLANG_ABSTRACT_AST_CLASS(CallableDecl)

    FilteredMemberList<ParamDecl> getParameters()
    {
        return getMembersOfType<ParamDecl>();
    }

    TypeExp returnType;
        
    // If this callable throws an error code, `errorType` is the type of the error code.
    TypeExp errorType;

    // Fields related to redeclaration, so that we
    // can support multiple specialized variations
    // of the "same" logical function.
    //
    // This should also help us to support redeclaration
    // of functions when handling HLSL/GLSL.

    // The "primary" declaration of the function, which will
    // be used whenever we need to unique things.
    CallableDecl* primaryDecl = nullptr;

    // The next declaration of the "same" function (that is,
    // with the same `primaryDecl`).
    CallableDecl* nextDecl = nullptr;
};

// Base class for callable things that may also have a body that is evaluated to produce their result
class FunctionDeclBase : public CallableDecl
{
    SLANG_ABSTRACT_AST_CLASS(FunctionDeclBase)

    Stmt* body = nullptr;
};

// A constructor/initializer to create instances of a type
class ConstructorDecl : public FunctionDeclBase
{
    SLANG_AST_CLASS(ConstructorDecl)
};

// A subscript operation used to index instances of a type
class SubscriptDecl : public CallableDecl
{
    SLANG_AST_CLASS(SubscriptDecl)
};

    /// A property declaration that abstracts over storage with a getter/setter/etc.
class PropertyDecl : public ContainerDecl
{
    SLANG_AST_CLASS(PropertyDecl)

    TypeExp type;
};

// An "accessor" for a subscript or property
class AccessorDecl : public FunctionDeclBase
{
    SLANG_AST_CLASS(AccessorDecl)
};

class GetterDecl : public AccessorDecl
{
    SLANG_AST_CLASS(GetterDecl)
};
class SetterDecl : public AccessorDecl
{
    SLANG_AST_CLASS(SetterDecl)
};
class RefAccessorDecl : public AccessorDecl
{
    SLANG_AST_CLASS(RefAccessorDecl)
};
class FuncDecl : public FunctionDeclBase
{
    SLANG_AST_CLASS(FuncDecl)
};

class NamespaceDeclBase : public ContainerDecl
{
    SLANG_AST_CLASS(NamespaceDeclBase)
};

    // A `namespace` declaration inside some module, that provides
    // a named scope for declarations inside it.
    //
    // Note: Multiple `namespace` declarations with the same name
    // in a given module/file will be collapsed into a single
    // `NamespaceDecl` during parsing, so this declaration does
    // not directly represent what is present in the input syntax.
    //
class NamespaceDecl : public NamespaceDeclBase
{
    SLANG_AST_CLASS(NamespaceDecl)
};

    // A "module" of code (essentially, a single translation unit)
    // that provides a scope for some number of declarations.
class ModuleDecl : public NamespaceDeclBase
{
    SLANG_AST_CLASS(ModuleDecl)
    // The API-level module that this declaration belong to.
    //
    // This field allows lookup of the `Module` based on a
    // declaration nested under a `ModuleDecl` by following
    // its chain of parents.
    //
    Module* module = nullptr;

        /// Map a decl to the list of its associated decls.
        ///
        /// This mapping is filled in during semantic checking, as the decl declarations get checked or generated.
        ///
    OrderedDictionary<Decl*, RefPtr<DeclAssociationList>> mapDeclToAssociatedDecls;

        /// Whether the module is defined in legacy language.
        /// The legacy Slang language does not have visibility modifiers and everything is treated as
        /// `public`. Newer version of the language introduces visibility and makes `internal` as the
        /// default. To prevent this from breaking existing code, we need to know whether a module is
        /// written in the legacy language. We detect this by checking whether the module has any
        /// visibility modifiers, or if the module uses new language constructs, e.g. `module`, `__include`,
        /// `__implementing` etc.
    bool isInLegacyLanguage = true;

    SLANG_UNREFLECTED

        /// Map a type to the list of extensions of that type (if any) declared in this module
        ///
        /// This mapping is filled in during semantic checking, as `ExtensionDecl`s get checked.
        ///
    Dictionary<AggTypeDecl*, RefPtr<CandidateExtensionList>> mapTypeToCandidateExtensions;

};

// Represents a transparent scope of declarations that are defined in a single source file.
class FileDecl : public ContainerDecl
{
    SLANG_AST_CLASS(FileDecl);
};

    /// A declaration that brings members of another declaration or namespace into scope
class UsingDecl : public Decl
{
    SLANG_AST_CLASS(UsingDecl)

        /// An expression that identifies the entity (e.g., a namespace) to be brought into `scope`
    Expr* arg = nullptr;

    SLANG_UNREFLECTED
        /// The scope that the entity named by `arg` will be brought into
    Scope* scope = nullptr;
};

class FileReferenceDeclBase : public Decl
{
    SLANG_AST_CLASS(FileReferenceDeclBase)

    // The name of the module we are trying to import
    NameLoc moduleNameAndLoc;

    SourceLoc startLoc;
    SourceLoc endLoc;

    SLANG_UNREFLECTED
    // The scope that we want to import into
    Scope* scope = nullptr;
};

class ImportDecl : public FileReferenceDeclBase
{
    SLANG_AST_CLASS(ImportDecl)

    // The module that actually got imported
    ModuleDecl* importedModuleDecl = nullptr;
};

class IncludeDeclBase : public FileReferenceDeclBase
{
    SLANG_AST_CLASS(IncludeDeclBase)

    FileDecl* fileDecl = nullptr;
};

class IncludeDecl : public IncludeDeclBase
{
    SLANG_AST_CLASS(IncludeDecl)
};

class ImplementingDecl : public IncludeDeclBase
{
    SLANG_AST_CLASS(ImplementingDecl)
};

class ModuleDeclarationDecl : public Decl
{
    SLANG_AST_CLASS(ModuleDeclarationDecl)
};

class RequireCapabilityDecl : public Decl
{
    SLANG_AST_CLASS(RequireCapabilityDecl)
};

// A generic declaration, parameterized on types/values
class GenericDecl : public ContainerDecl
{
    SLANG_AST_CLASS(GenericDecl)
    // The decl that is genericized...
    Decl* inner = nullptr;
};

class GenericTypeParamDecl : public SimpleTypeDecl
{
    SLANG_AST_CLASS(GenericTypeParamDecl)
    // The bound for the type parameter represents a trait that any
    // type used as this parameter must conform to
//            TypeExp bound;

    // The "initializer" for the parameter represents a default value
    TypeExp initType;
};

// A constraint placed as part of a generic declaration
class GenericTypeConstraintDecl : public TypeConstraintDecl
{
    SLANG_AST_CLASS(GenericTypeConstraintDecl)

    // A type constraint like `T : U` is constraining `T` to be "below" `U`
    // on a lattice of types. This may not be a subtyping relationship
    // per se, but it makes sense to use that terminology here, so we
    // think of these fields as the sub-type and super-type, respectively.
    TypeExp sub;
    TypeExp sup;

    // Overrides should be public so base classes can access
    const TypeExp& _getSupOverride() const { return sup; }
};

class GenericValueParamDecl : public VarDeclBase
{
    SLANG_AST_CLASS(GenericValueParamDecl)
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
class EmptyDecl : public Decl
{
    SLANG_AST_CLASS(EmptyDecl)
};

// A declaration used by the implementation to put syntax keywords
// into the current scope.
//
class SyntaxDecl : public Decl
{
    SLANG_AST_CLASS(SyntaxDecl)

    // What type of syntax node will be produced when parsing with this keyword?
    SyntaxClass<NodeBase> syntaxClass;

    SLANG_UNREFLECTED

    // Callback to invoke in order to parse syntax with this keyword.
    SyntaxParseCallback  parseCallback = nullptr;
    void*                parseUserData = nullptr;
};

// A declaration of an attribute to be used with `[name(...)]` syntax.
//
class AttributeDecl : public ContainerDecl
{
    SLANG_AST_CLASS(AttributeDecl)
    // What type of syntax node will be produced to represent this attribute.
    SyntaxClass<NodeBase> syntaxClass;
};

// A synthesized decl used as a placeholder for a differentiable function requirement. This decl will
// be a child of interface decl.
// This allows us to form an interface requirement key for the derivative of an interface function.
// The synthesized `DerivativeRequirementDecl` will be a child of the original function requirement
// decl after an interface type is checked.
class DerivativeRequirementDecl : public FunctionDeclBase
{
    SLANG_AST_CLASS(DerivativeRequirementDecl)
    
    // The original requirement decl.
    Decl* originalRequirementDecl = nullptr;

    // Type to use for 'ThisType'
    Type* diffThisType;
};

// A reference to a synthesized decl representing a differentiable function requirement, this decl will
// be a child in the orignal function.
class DerivativeRequirementReferenceDecl : public FunctionDeclBase
{
    SLANG_AST_CLASS(DerivativeRequirementReferenceDecl)
    DerivativeRequirementDecl* referencedDecl;
};

class ForwardDerivativeRequirementDecl : public DerivativeRequirementDecl
{
    SLANG_AST_CLASS(ForwardDerivativeRequirementDecl)
};

class BackwardDerivativeRequirementDecl : public DerivativeRequirementDecl
{
    SLANG_AST_CLASS(BackwardDerivativeRequirementDecl)
};

bool isInterfaceRequirement(Decl* decl);
InterfaceDecl* findParentInterfaceDecl(Decl* decl);

bool isLocalVar(const Decl* decl);


// Add a sibling lookup scope for `dest` to refer to `source`.
void addSiblingScopeForContainerDecl(ASTBuilder* builder, ContainerDecl* dest, ContainerDecl* source);
void addSiblingScopeForContainerDecl(ASTBuilder* builder, Scope* destScope, ContainerDecl* source);

} // namespace Slang
