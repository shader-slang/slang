// decl-defs.h

// Syntax class definitions for declarations.

// A group of declarations that should be treated as a unit
SYNTAX_CLASS(DeclGroup, DeclBase)
    SYNTAX_FIELD(List<RefPtr<Decl>>, decls)
END_SYNTAX_CLASS()

// A "container" decl is a parent to other declarations
ABSTRACT_SYNTAX_CLASS(ContainerDecl, Decl)
    SYNTAX_FIELD(List<RefPtr<Decl>>, Members)

    RAW(
    template<typename T>
    FilteredMemberList<T> getMembersOfType()
    {
        return FilteredMemberList<T>(Members);
    }


    // Dictionary for looking up members by name.
    // This is built on demand before performing lookup.
    Dictionary<Name*, Decl*> memberDictionary;

    // Whether the `memberDictionary` is valid.
    // Should be set to `false` if any members get added/remoed.
    bool memberDictionaryIsValid = false;

    // A list of transparent members, to be used in lookup
    // Note: this is only valid if `memberDictionaryIsValid` is true
    List<TransparentMemberInfo> transparentMembers;
    )
END_SYNTAX_CLASS()

// Base class for all variable declarations
ABSTRACT_SYNTAX_CLASS(VarDeclBase, Decl)

    // type of the variable
    SYNTAX_FIELD(TypeExp, type)

    RAW(
    Type* getType() { return type.type.Ptr(); }
    )

    // Initializer expression (optional)
    SYNTAX_FIELD(RefPtr<Expr>, initExpr)
END_SYNTAX_CLASS()

// Ordinary potentially-mutable variables (locals, globals, and member variables)
SYNTAX_CLASS(VarDecl, VarDeclBase)
END_SYNTAX_CLASS()

// A variable declaration that is always immutable (whether local, global, or member variable)
SYNTAX_CLASS(LetDecl, VarDecl)
END_SYNTAX_CLASS()

// An `AggTypeDeclBase` captures the shared functionality
// between true aggregate type declarations and extension
// declarations:
//
// - Both can container members (they are `ContainerDecl`s)
// - Both can have declared bases
// - Both expose a `this` variable in their body
//
ABSTRACT_SYNTAX_CLASS(AggTypeDeclBase, ContainerDecl)
END_SYNTAX_CLASS()

// An extension to apply to an existing type
SYNTAX_CLASS(ExtensionDecl, AggTypeDeclBase)
    SYNTAX_FIELD(TypeExp, targetType)

    // next extension attached to the same nominal type
    DECL_FIELD(ExtensionDecl*, nextCandidateExtension RAW(= nullptr))
END_SYNTAX_CLASS()

// Declaration of a type that represents some sort of aggregate
ABSTRACT_SYNTAX_CLASS(AggTypeDecl, AggTypeDeclBase)

RAW(
    // extensions that might apply to this declaration
    ExtensionDecl* candidateExtensions = nullptr;
    FilteredMemberList<VarDecl> GetFields()
    {
        return getMembersOfType<VarDecl>();
    }
    )
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(StructDecl, AggTypeDecl)

SIMPLE_SYNTAX_CLASS(ClassDecl, AggTypeDecl)

// TODO: Is it appropriate to treat an `enum` as an aggregate type?
// Most code that looks for, e.g., conformances assumes user-defined
// types are all `AggTypeDecl`, so this is the right choice for now
// if we want `enum` types to be able to implement interfaces, etc.
//
SYNTAX_CLASS(EnumDecl, AggTypeDecl)
RAW(
    RefPtr<Type> tagType;
)
END_SYNTAX_CLASS()

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
SYNTAX_CLASS(EnumCaseDecl, Decl)

    // type of the parent `enum`
    SYNTAX_FIELD(TypeExp, type)

    RAW(
    Type* getType() { return type.type.Ptr(); }
    )

    // Tag value
    SYNTAX_FIELD(RefPtr<Expr>, tagExpr)
END_SYNTAX_CLASS()

// An interface which other types can conform to
SIMPLE_SYNTAX_CLASS(InterfaceDecl, AggTypeDecl)

ABSTRACT_SYNTAX_CLASS(TypeConstraintDecl, Decl)
    RAW(
    virtual TypeExp& getSup() = 0;
    )
END_SYNTAX_CLASS()

// A kind of pseudo-member that represents an explicit
// or implicit inheritance relationship.
//
SYNTAX_CLASS(InheritanceDecl, TypeConstraintDecl)
// The type expression as written
    SYNTAX_FIELD(TypeExp, base)

    RAW(
    // After checking, this dictionary will map members
    // required by the base type to their concrete
    // implementations in the type that contains
    // this inheritance declaration.
    RefPtr<WitnessTable> witnessTable;
    virtual TypeExp& getSup() override
    {
        return base;
    }
    )
END_SYNTAX_CLASS()

// TODO: may eventually need sub-classes for explicit/direct vs. implicit/indirect inheritance


// A declaration that represents a simple (non-aggregate) type
//
// TODO: probably all types will be aggregate decls eventually,
// so that we can easily store conformances/constraints on type variables
ABSTRACT_SYNTAX_CLASS(SimpleTypeDecl, Decl)
END_SYNTAX_CLASS()

// A `typedef` declaration
SYNTAX_CLASS(TypeDefDecl, SimpleTypeDecl)
    SYNTAX_FIELD(TypeExp, type)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(TypeAliasDecl, TypeDefDecl)

// An 'assoctype' declaration, it is a container of inheritance clauses
SYNTAX_CLASS(AssocTypeDecl, AggTypeDecl)
END_SYNTAX_CLASS()

// A 'type_param' declaration, which defines a generic
// entry-point parameter. Is a container of GenericTypeConstraintDecl
SYNTAX_CLASS(GlobalGenericParamDecl, AggTypeDecl)
END_SYNTAX_CLASS()

// A scope for local declarations (e.g., as part of a statement)
SIMPLE_SYNTAX_CLASS(ScopeDecl, ContainerDecl)

// A function/initializer/subscript parameter (potentially mutable)
SIMPLE_SYNTAX_CLASS(ParamDecl, VarDeclBase)

// A parameter of a function declared in "modern" types (immutable unless explicitly `out` or `inout`)
SIMPLE_SYNTAX_CLASS(ModernParamDecl, ParamDecl)

// Base class for things that have parameter lists and can thus be applied to arguments ("called")
ABSTRACT_SYNTAX_CLASS(CallableDecl, ContainerDecl)
    RAW(
    FilteredMemberList<ParamDecl> GetParameters()
    {
        return getMembersOfType<ParamDecl>();
    })

    SYNTAX_FIELD(TypeExp, ReturnType)

    // Fields related to redeclaration, so that we
    // can support multiple specialized varaitions
    // of the "same" logical function.
    //
    // This should also help us to support redeclaration
    // of functions when handling HLSL/GLSL.

    // The "primary" declaration of the function, which will
    // be used whenever we need to unique things.
    FIELD_INIT(CallableDecl*, primaryDecl, nullptr)

    // The next declaration of the "same" function (that is,
    // with the same `primaryDecl`).
    FIELD_INIT(CallableDecl*, nextDecl, nullptr);

END_SYNTAX_CLASS()

// Base class for callable things that may also have a body that is evaluated to produce their result
ABSTRACT_SYNTAX_CLASS(FunctionDeclBase, CallableDecl)
    SYNTAX_FIELD(RefPtr<Stmt>, Body)
END_SYNTAX_CLASS()

// A constructor/initializer to create instances of a type
SIMPLE_SYNTAX_CLASS(ConstructorDecl, FunctionDeclBase)

// A subscript operation used to index instances of a type
SIMPLE_SYNTAX_CLASS(SubscriptDecl, CallableDecl)

// An "accessor" for a subscript or property
SIMPLE_SYNTAX_CLASS(AccessorDecl, FunctionDeclBase)

SIMPLE_SYNTAX_CLASS(GetterDecl, AccessorDecl)
SIMPLE_SYNTAX_CLASS(SetterDecl, AccessorDecl)
SIMPLE_SYNTAX_CLASS(RefAccessorDecl, AccessorDecl)

SIMPLE_SYNTAX_CLASS(FuncDecl, FunctionDeclBase)

// A "module" of code (essentiately, a single translation unit)
// that provides a scope for some number of declarations.
SYNTAX_CLASS(ModuleDecl, ContainerDecl)
    FIELD(RefPtr<Scope>, scope)

    // The API-level module that this declaration belong to.
    //
    // This field allows lookup of the `Module` based on a
    // declaration nested under a `ModuleDecl` by following
    // its chain of parents.
    //
    RAW(Module* module = nullptr;)
END_SYNTAX_CLASS()

SYNTAX_CLASS(ImportDecl, Decl)
    // The name of the module we are trying to import
    FIELD(NameLoc, moduleNameAndLoc)

    // The scope that we want to import into
    FIELD(RefPtr<Scope>, scope)

    // The module that actually got imported
    DECL_FIELD(RefPtr<ModuleDecl>, importedModuleDecl)
END_SYNTAX_CLASS()

// A generic declaration, parameterized on types/values
SYNTAX_CLASS(GenericDecl, ContainerDecl)
    // The decl that is genericized...
    SYNTAX_FIELD(RefPtr<Decl>, inner)
END_SYNTAX_CLASS()

SYNTAX_CLASS(GenericTypeParamDecl, SimpleTypeDecl)
    // The bound for the type parameter represents a trait that any
    // type used as this parameter must conform to
//            TypeExp bound;

    // The "initializer" for the parameter represents a default value
    SYNTAX_FIELD(TypeExp, initType)
END_SYNTAX_CLASS()

// A constraint placed as part of a generic declaration
SYNTAX_CLASS(GenericTypeConstraintDecl, TypeConstraintDecl)
    // A type constraint like `T : U` is constraining `T` to be "below" `U`
    // on a lattice of types. This may not be a subtyping relationship
    // per se, but it makes sense to use that terminology here, so we
    // think of these fields as the sub-type and sup-ertype, respectively.
    SYNTAX_FIELD(TypeExp, sub)
    SYNTAX_FIELD(TypeExp, sup)
    RAW(
    virtual TypeExp& getSup() override
    {
        return sup;
    }
    )
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(GenericValueParamDecl, VarDeclBase)

// An empty declaration (which might still have modifiers attached).
//
// An empty declaration is uncommon in HLSL, but
// in GLSL it is often used at the global scope
// to declare metadata that logically belongs
// to the entry point, e.g.:
//
//     layout(local_size_x = 16) in;
//
SIMPLE_SYNTAX_CLASS(EmptyDecl, Decl)

// A declaration used by the implementation to put syntax keywords
// into the current scope.
//
SYNTAX_CLASS(SyntaxDecl, Decl)
    // What type of syntax node will be produced when parsing with this keyword?
    FIELD(SyntaxClass<RefObject>, syntaxClass)

    // Callback to invoke in order to parse syntax with this keyword.
    FIELD(SyntaxParseCallback,  parseCallback)
    FIELD(void*,                parseUserData)
END_SYNTAX_CLASS()

// A declaration of an attribute to be used with `[name(...)]` syntax.
//
SYNTAX_CLASS(AttributeDecl, ContainerDecl)
    // What type of syntax node will be produced to represent this attribute.
    FIELD(SyntaxClass<RefObject>, syntaxClass)
END_SYNTAX_CLASS()
