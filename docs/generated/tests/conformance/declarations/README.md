---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T07:58:38+00:00
source_commit: a01e79278cf0be9772deb8b2be52ee87687697a3
watched_paths_digest: baa6fac8ba9ed5ca13a1a8e7c88d05af57311f436bd5f74976903baabe34556f
source_doc: docs/language-reference/declarations.md
source_doc_digest: 132f3f2073ca652a1ccb7c503f59b91bb65f4228adcc40915e269f77d6a985a1
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/declarations

## Intent

Tests verify declaration claims in the **language reference** at
[`docs/language-reference/declarations.md`](../../../../language-reference/declarations.md).
Coverage strategy: one functional test (INTERPRET or emit) per claim, plus negative tests for
every "is rejected" / "is an error" sentence, expanded across documented declaration forms
(`let`/`var`/`const`, `func` keyword, parameter directions, enumeration cases, `typealias`/`typedef`,
`cbuffer`/`tbuffer`, interfaces, associated types, `__init`, `__subscript`, extensions, generics).
Static global and function-scope variables are tested via HLSL emission only because the slangi
VM bytecode emitter does not support non-const module-scope global instructions.

## Claims

### Modules (`#modules`)

- **C1** A module consists of one or more source units compiled together; the global declarations
  in those source units comprise the body of the module.
- **C2** The order of declarations within a source unit does not matter; declarations can freely
  refer to other declarations later in the same source unit.
- **C3** Declarations may freely be defined in any source unit in a module; declarations in one
  source unit may freely refer to declarations in other source units.

### Imports (`#imports`)

- **C4** An `import` declaration brings the declarations in the named module into scope in the
  current source unit.
- **C5** An `import` declaration only applies to the scope of the current source unit; it does not
  make the imported module visible to code that later imports the current module.
- **C6** The name of the module being imported may use a compound name (e.g. `import MyApp.Shadowing`).

### Variables (`#variables`)

- **C7** A `let` declaration introduces an immutable variable; it may not be assigned to.
- **C8** A `let` variable may not be used as the argument for an `out` or `in out` parameter.
- **C9** A `var` declaration introduces a mutable variable.
- **C10** An explicit type may be given for a variable by placing it after the variable name and a colon (`:`).
- **C11** If no type is specified for a variable, a type will be inferred from the initial-value expression.
- **C12** It is an error to declare a variable that has neither a type specifier nor an initial-value expression (E30620).
- **C13** A variable declared with `var` may be declared without an initial-value expression if it has an explicit type specifier.
- **C14** Traditional `const` variable declarations are immutable; traditional declarations without `const` are mutable.
- **C15** A variable declared at global scope and marked with `static` and `const` is a global constant; it must have an initial-value expression that is a compile-time constant.
- **C16** A variable declared at global scope and marked with `static` (but not `const`) is a static global variable; assignments from one invocation do not affect other invocations.
- **C17** A variable declared at global scope and not marked with `static` (even if marked with `const`) is a global shader parameter.
- **C18** A variable declared at function scope and marked with both `static` and `const` is a function-scope constant; it behaves like a global constant but is visible only in the local scope.
- **C19** A variable declared at function scope and marked with `static` (but not `const`) is a function-scope static variable; it behaves like a global static variable but is visible only in the local scope.
- **C20** A variable declared at function scope and not marked with `static` is a local variable; each activation of the function has its own copy.

### Functions (`#functions`, `#parameters`, `#body`)

- **C21** Functions are declared using the `func` keyword with `name: type` parameter pairs and an optional `-> result-type` result clause.
- **C22** Parameters may be given a default value by including an initial-value-expression clause.
- **C23** The `in` direction (the default) indicates pass-by-value (copy-in) semantics.
- **C24** The `out` direction indicates copy-out semantics; the callee writes to the parameter and the value is copied to the caller's argument on return.
- **C25** The `in out` / `inout` direction indicates copy-in-and-copy-out semantics.
- **C26** A function declaration without a body is a forward declaration; it must be terminated with a semicolon.
- **C27** If the function result type is `void`, the result type clause may be elided.
- **C28** Functions can also be declared with traditional C-style syntax (`return-type name(params)`).

### Enumeration Types (`#enumeration-types`)

- **C29** If the first case declaration elides an initial-value expression, the value `0` is used as the tag value.
- **C30** If any subsequent case declaration elides an initial-value expression, its tag value is one greater than the tag value of the immediately preceding case declaration.
- **C31** An explicit initial-value expression in a case declaration sets that case's tag value; the next implicit case increments from it.
- **C32** An enumeration case is referred to as if it were a `static` member of the enumeration type (e.g., `Color.Red`).
- **C33** A value of an enumeration type can be implicitly converted to a value of its tag type.
- **C34** Values of the tag type can be explicitly converted to the enumeration type (e.g., `Color(r)`).
- **C35** If no explicit tag type is specified, the type `int` is used.

### Type Aliases (`#type-aliases`)

- **C36** A `typealias` declaration defines a name that is equivalent to the type to the right of `=`.
- **C37** The traditional `typedef` syntax also declares a type alias equivalent to the named type.

### Constant Buffers and Texture Buffers (`#constant-buffers-and-texture-buffers`)

- **C38** A `cbuffer` declaration is expanded to an anonymous struct in a `ConstantBuffer<AnonType>` with a `__transparent` modifier; fields are accessible by unqualified name.
- **C39** A `tbuffer` declaration uses the same expansion as `cbuffer` but with `TextureBuffer<T>` instead of `ConstantBuffer<T>`.

### Interfaces (`#interfaces`)

- **C40** An interface declaration uses the `interface` keyword; the body may contain function, initializer, subscript, and associated type declarations, each of which introduces a requirement.
- **C41** Types that declare conformance to the interface must provide matching implementations of the requirements.
- **C42** An interface declaration may have an inheritance clause; a type must also conform to all base interfaces.

### Associated Types (`#associated-types`)

- **C43** An `associatedtype` declaration introduces a type into the signature of an interface without specifying the exact concrete type.
- **C44** It is an error to declare an `associatedtype` anywhere other than the body of an interface declaration (E30102).
- **C45** An associated type may have an inheritance clause; a concrete type satisfying the associated type must conform to all required interfaces.

### Initializers (`#initializers`)

- **C46** An `__init` declaration defines a way to initialize an instance; it may only appear in the body of an interface or a structure type.
- **C47** An initializer is invoked by calling the enclosing type as if it were a function.
- **C48** An initializer has access to an implicit mutable `this` variable representing the instance being initialized.

### Subscripts (`#subscripts`)

- **C49** A `__subscript` declaration introduces a way for a user-defined type to support subscripting with `[]`; it must list parameters and a result type clause starting with `->`.
- **C50** The result type clause of a subscript declaration cannot be elided (E30901).
- **C51** A `get` accessor in the subscript body introduces a getter; reading via `[]` invokes the getter.

### Extensions (`#extensions`)

- **C52** An `extension` declaration adds behavior to an existing type; instance and static methods in the extension are accessible through the extended type.
- **C53** The body of an extension may not include variable declarations (E31400).
- **C54** An extension may include an inheritance clause; the extension then asserts a new conformance of the extended type to the listed interface.
- **C55** Currently, extensions can only apply to structure types.

### Generics (`#generics`)

- **C56** A generic declaration introduces a generic parameter list enclosed in angle brackets `<>`.
- **C57** A single identifier like `T` is a generic type parameter with no constraints.
- **C58** A clause like `T : IFoo` introduces a constrained generic type parameter; the argument must conform to `IFoo`.
- **C59** A clause like `let N : int` introduces a generic value parameter.
- **C60** Generic parameters may declare a default value with `=`.
- **C61** A generic is explicitly specialized by supplying generic arguments in angle brackets `<>`.
- **C62** If a generic is used where a non-generic is expected, the compiler performs implicit specialization by inferring generic arguments from the context.

## Functional coverage

| Claim                                                                                                                                           | Intent     | Anchor                                                                                                                       | Tests                                                                                                |
| ----------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ---------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| C7 — A `let` variable's value is readable but cannot be assigned to.                                                                            | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-let-immutable-functional.slang`](var-let-immutable-functional.slang)                           |
| C7 — Assigning to a `let` variable is rejected with E30011.                                                                                     | negative   | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-let-assign-rejected.slang`](var-let-assign-rejected.slang)                                     |
| C8 — Passing a `let` variable as `out` argument is rejected with E30047.                                                                        | negative   | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-let-out-param-rejected.slang`](var-let-out-param-rejected.slang)                               |
| C8 — Passing a `let` variable as `inout` argument is rejected with E30047.                                                                      | negative   | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-let-inout-param-rejected.slang`](var-let-inout-param-rejected.slang)                           |
| C9 — A `var` declaration introduces a mutable variable that can be reassigned.                                                                  | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-mutable-functional.slang`](var-mutable-functional.slang)                                       |
| C12 — Declaring a variable with neither a type nor an initializer is rejected with E30620.                                                      | negative   | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-no-type-no-init-rejected.slang`](var-no-type-no-init-rejected.slang)                           |
| C13 — A `var` with an explicit type and no initializer is allowed; the value is assigned later.                                                 | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-explicit-type-no-init-functional.slang`](var-explicit-type-no-init-functional.slang)           |
| C14 — Traditional `const` is immutable; traditional without `const` is mutable.                                                                 | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-traditional-const-functional.slang`](var-traditional-const-functional.slang)                   |
| C15 — A `static const` global variable folds to a compile-time constant in emitted HLSL.                                                        | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-global-const-emission.slang`](var-global-const-emission.slang)                                 |
| C16 — A `static` (non-`const`) global variable emits as a module-scope `static` in HLSL.                                                        | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-static-global-emission.slang`](var-static-global-emission.slang)                               |
| C18 — A function-scope `static const` constant folds to its compile-time value in emitted HLSL.                                                 | functional | [#variables](../../../../language-reference/declarations.md#variables)                                                       | [`var-func-scope-static-const-emission.slang`](var-func-scope-static-const-emission.slang)           |
| C21 — The `func` keyword declares a function with `name: type` params and `->` result clause.                                                   | functional | [#functions](../../../../language-reference/declarations.md#functions)                                                       | [`func-func-keyword-functional.slang`](func-func-keyword-functional.slang)                           |
| C22 — Parameters with default values use the default when the argument is omitted.                                                              | functional | [#parameters](../../../../language-reference/declarations.md#parameters)                                                     | [`func-default-param-functional.slang`](func-default-param-functional.slang)                         |
| C24 — `out` parameters use copy-out semantics; the written value is observable after the call.                                                  | functional | [#parameters](../../../../language-reference/declarations.md#parameters)                                                     | [`func-param-out-functional.slang`](func-param-out-functional.slang)                                 |
| C25 — `inout` parameters use copy-in-copy-out semantics; accumulated changes are visible after the call.                                        | functional | [#parameters](../../../../language-reference/declarations.md#parameters)                                                     | [`func-param-inout-functional.slang`](func-param-inout-functional.slang)                             |
| C26 — A function declaration without a body (forward declaration) is allowed and terminated with `;`.                                           | functional | [#body](../../../../language-reference/declarations.md#body)                                                                 | [`func-forward-decl-functional.slang`](func-forward-decl-functional.slang)                           |
| C27 — The `void` result type clause may be elided from a `func` declaration.                                                                    | functional | [#body](../../../../language-reference/declarations.md#body)                                                                 | [`func-void-elide-functional.slang`](func-void-elide-functional.slang)                               |
| C28 — Traditional C-style function syntax compiles and produces correct results.                                                                | functional | [#functions](../../../../language-reference/declarations.md#functions)                                                       | [`func-traditional-syntax-functional.slang`](func-traditional-syntax-functional.slang)               |
| C29 — First enum case without explicit value gets tag value `0`.                                                                                | functional | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types)                                       | [`enum-tag-values-functional.slang`](enum-tag-values-functional.slang)                               |
| C30 — Subsequent cases without explicit values increment from the previous case.                                                                | functional | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types)                                       | [`enum-tag-values-functional.slang`](enum-tag-values-functional.slang)                               |
| C31 — An explicit tag value mid-enum sets that case's value; subsequent implicit cases continue from there.                                     | functional | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types)                                       | [`enum-explicit-tag-functional.slang`](enum-explicit-tag-functional.slang)                           |
| C32 — Enumeration cases are accessed as static members of the enumeration type (`Color.Red`).                                                   | functional | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types)                                       | [`enum-case-static-member-emission.slang`](enum-case-static-member-emission.slang)                   |
| C33 — A value of an enumeration type is implicitly convertible to its tag type (`int`).                                                         | functional | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types)                                       | [`enum-implicit-conv-to-int-functional.slang`](enum-implicit-conv-to-int-functional.slang)           |
| C34 — Values of the tag type can be explicitly converted back to the enumeration type.                                                          | functional | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types)                                       | [`enum-explicit-conv-from-int-functional.slang`](enum-explicit-conv-from-int-functional.slang)       |
| C36 — A `typealias` declaration makes the alias and the original type interchangeable.                                                          | functional | [#type-aliases](../../../../language-reference/declarations.md#type-aliases)                                                 | [`typealias-functional.slang`](typealias-functional.slang)                                           |
| C37 — The traditional `typedef` syntax also declares a valid type alias.                                                                        | functional | [#type-aliases](../../../../language-reference/declarations.md#type-aliases)                                                 | [`typedef-functional.slang`](typedef-functional.slang)                                               |
| C38 — A `cbuffer` expands to an anonymous struct in `ConstantBuffer<>` and is emitted as a `cbuffer` block in HLSL.                             | functional | [#constant-buffers-and-texture-buffers](../../../../language-reference/declarations.md#constant-buffers-and-texture-buffers) | [`cbuffer-emission.slang`](cbuffer-emission.slang)                                                   |
| C39 — A `tbuffer` uses `TextureBuffer<T>` and emits a `tbuffer` keyword in HLSL.                                                                | functional | [#constant-buffers-and-texture-buffers](../../../../language-reference/declarations.md#constant-buffers-and-texture-buffers) | [`tbuffer-emission.slang`](tbuffer-emission.slang)                                                   |
| C40/C41 — An interface introduces requirements; a conforming struct must implement them and can be used polymorphically.                        | functional | [#interfaces](../../../../language-reference/declarations.md#interfaces)                                                     | [`interface-basic-functional.slang`](interface-basic-functional.slang)                               |
| C42 — An interface may inherit from another interface; a type must also satisfy all base interface requirements.                                | functional | [#interfaces](../../../../language-reference/declarations.md#interfaces)                                                     | [`interface-inheritance-functional.slang`](interface-inheritance-functional.slang)                   |
| C43 — An `associatedtype` in an interface introduces a type requirement satisfied by different concrete types per implementation.               | functional | [#associated-types](../../../../language-reference/declarations.md#associated-types)                                         | [`associatedtype-basic-functional.slang`](associatedtype-basic-functional.slang)                     |
| C44 — Declaring an `associatedtype` outside an interface body is rejected with E30102.                                                          | negative   | [#associated-types](../../../../language-reference/declarations.md#associated-types)                                         | [`associatedtype-outside-interface-rejected.slang`](associatedtype-outside-interface-rejected.slang) |
| C46/C47 — An `__init` initializer is invoked by calling the type as a function; the instance fields have the initialized values.                | functional | [#initializers](../../../../language-reference/declarations.md#initializers)                                                 | [`init-invocation-functional.slang`](init-invocation-functional.slang)                               |
| C48 — Inside `__init`, `this` refers to the mutable instance being initialized; fields are set via `this.field`.                                | functional | [#initializers](../../../../language-reference/declarations.md#initializers)                                                 | [`init-this-access-functional.slang`](init-this-access-functional.slang)                             |
| C49/C51 — A `__subscript` with a `get` accessor enables `[]` syntax; reading invokes the getter.                                                | functional | [#subscripts](../../../../language-reference/declarations.md#subscripts)                                                     | [`subscript-get-functional.slang`](subscript-get-functional.slang)                                   |
| C50 — Omitting the `->` result type clause on `__subscript` is a compile error (E30901).                                                        | negative   | [#subscripts](../../../../language-reference/declarations.md#subscripts)                                                     | [`subscript-result-type-mandatory-check.slang`](subscript-result-type-mandatory-check.slang)         |
| C52 — An extension adds instance and static methods to an existing type; both are accessible through the extended type.                         | functional | [#extensions](../../../../language-reference/declarations.md#extensions)                                                     | [`extension-add-method-functional.slang`](extension-add-method-functional.slang)                     |
| C53 — Including a non-static variable declaration in an extension body is rejected with E31400.                                                 | negative   | [#extensions](../../../../language-reference/declarations.md#extensions)                                                     | [`extension-no-variable-rejected.slang`](extension-no-variable-rejected.slang)                       |
| C54 — An extension with an inheritance clause introduces a new conformance; the extended type can then be used where the interface is required. | functional | [#extensions](../../../../language-reference/declarations.md#extensions)                                                     | [`extension-conformance-functional.slang`](extension-conformance-functional.slang)                   |
| C56/C57 — A generic function with a type parameter `T` is implicitly specialized to different types at each call site.                          | functional | [#generics](../../../../language-reference/declarations.md#generics)                                                         | [`generic-type-param-functional.slang`](generic-type-param-functional.slang)                         |
| C58 — A constrained type parameter `T : IFoo` allows calling interface members on `T` within the generic.                                       | functional | [#generics](../../../../language-reference/declarations.md#generics)                                                         | [`generic-constrained-type-param-functional.slang`](generic-constrained-type-param-functional.slang) |
| C59 — A generic value parameter `let N : int` can be used to size arrays or bound loops at compile-time.                                        | functional | [#generics](../../../../language-reference/declarations.md#generics)                                                         | [`generic-value-param-functional.slang`](generic-value-param-functional.slang)                       |
| C61 — A generic is explicitly specialized by supplying type arguments in angle brackets `<>`; the result can be used as a non-generic.          | functional | [#generics](../../../../language-reference/declarations.md#generics)                                                         | [`generic-explicit-specialize-functional.slang`](generic-explicit-specialize-functional.slang)       |
| C62 — When a generic is used without explicit arguments, the compiler infers them from call-site context (implicit specialization).             | functional | [#generics](../../../../language-reference/declarations.md#generics)                                                         | [`generic-implicit-specialize-functional.slang`](generic-implicit-specialize-functional.slang)       |

## Untested claims

| Claim                                                                                                             | Reason                | Anchor                                                                                 | Why untested                                                                                                                                                                                                                                        |
| ----------------------------------------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1/C2/C3 — Module source-unit order independence and cross-unit declaration visibility.                           | needs-multi-file-test | [#modules](../../../../language-reference/declarations.md#modules)                     | Requires two `.slang` source units compiled together as a module; not expressible in a single `//TEST` directive.                                                                                                                                   |
| C4/C5/C6 — `import` declaration scoping and compound import names.                                                | needs-multi-file-test | [#imports](../../../../language-reference/declarations.md#imports)                     | Requires a separate module file to import; not expressible in a single self-contained test.                                                                                                                                                         |
| C11 — Type is inferred from the initial-value expression when no explicit type is given.                          | out-of-bundle         | [#variables](../../../../language-reference/declarations.md#variables)                 | Covered by `var-let-immutable-functional.slang` and `var-mutable-functional.slang` which both use type-inferred declarations.                                                                                                                       |
| C17 — A global variable not marked `static` (even if `const`) is a global shader parameter.                       | out-of-bundle         | [#variables](../../../../language-reference/declarations.md#variables)                 | Covered in the types-struct and expressions bundles which use global shader parameters.                                                                                                                                                             |
| C19 — A function-scope `static` (non-`const`) variable behaves like a per-invocation global static at runtime.    | implementation-detail | [#variables](../../../../language-reference/declarations.md#variables)                 | The key behavioral claim (thread-local semantics) is not observable in slangi or CPU compute without multiple concurrent threads. Emission test covers the emitted `static` keyword.                                                                |
| C20 — Each activation of a function produces a distinct local variable with its own storage.                      | out-of-bundle         | [#variables](../../../../language-reference/declarations.md#variables)                 | Recursive-call storage isolation requires recursion support; Slang does not guarantee recursion in all targets. The basic "local variable per call" claim is observable in the functional tests.                                                    |
| C23 — The `in` direction is the default (no modifier needed).                                                     | out-of-bundle         | [#parameters](../../../../language-reference/declarations.md#parameters)               | Covered by all functional tests that call functions without direction modifiers; the default `in` behavior is tested implicitly.                                                                                                                    |
| C25 — `func` syntax with `in out` / `inout` parameter direction crashes the compiler (ICE).                       | (unclassified)        | [#parameters](../../../../language-reference/declarations.md#parameters)               | Compiler emits internal error E99999 ("unknown type modifier in semantic checking"). Finding: `docs/generated/tests/_meta/findings/declarations-func-inout-ice.yaml`. Traditional C-style `inout` is tested in `func-param-inout-functional.slang`. |
| C35 — If no explicit tag type is specified for an enum, the type `int` is used as the default.                    | out-of-bundle         | [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types) | All enum tests use `int` as the implicit tag type (default int conversion verified in `enum-implicit-conv-to-int-functional.slang`).                                                                                                                |
| C45 — An associated type with an inheritance clause requires the satisfying type to conform to listed interfaces. | out-of-bundle         | [#associated-types](../../../../language-reference/declarations.md#associated-types)   | Covered by the generics and types-interface bundles which exercise constrained associated types.                                                                                                                                                    |
| C55 — Extensions currently can only apply to structure types; extending an enum or interface is not supported.    | out-of-bundle         | [#extensions](../../../../language-reference/declarations.md#extensions)               | A negative test (extend-enum-rejected) would verify the limit, but the doc states this as a current limitation without a specific diagnostic code. Adding a `## Doc gaps observed` row instead.                                                     |
| C60 — Generic parameters may declare a default value with `=`.                                                    | out-of-bundle         | [#generics](../../../../language-reference/declarations.md#generics)                   | Covered in the conformance/generics bundle which tests default generic parameters explicitly.                                                                                                                                                       |

## Doc gaps observed

| Anchor                                                                                 | Kind                  | Gap                                                                                                                                                                                                                                                                                                                          | Suggested addition                                                                                                                                                                                                                                   |
| -------------------------------------------------------------------------------------- | --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#parameters](../../../../language-reference/declarations.md#parameters)               | drift-from-source     | The doc gives `func add(x: in out int, y : float)` as an example of `in out` direction in `func` syntax. The compiler emits internal error E99999 ("unknown type modifier in semantic checking") when `inout` or `in out` appears in a `func`-keyword parameter list. The traditional C-style equivalent compiles correctly. | Add a note that `in out`/`inout` in `func`-syntax parameter lists is not yet implemented; direct readers to the traditional C-style syntax for mutable parameters. (Finding: `docs/generated/tests/_meta/findings/declarations-func-inout-ice.yaml`) |
| [#interfaces](../../../../language-reference/declarations.md#interfaces)               | drift-from-source     | The doc states "Functions, initializers, and subscripts declared inside an interface must not have bodies." The compiler actually accepts function bodies inside interfaces without error.                                                                                                                                   | Clarify whether interface method bodies are planned for a future version or intentionally accepted; update the doc to match actual compiler behavior.                                                                                                |
| [#extensions](../../../../language-reference/declarations.md#extensions)               | missing-surface       | The doc states "extensions cannot apply to enumeration types or interfaces" but does not specify which diagnostic code is emitted when you try. A reader cannot write a negative test without knowing the expected error code.                                                                                               | Add the diagnostic code emitted when an extension is applied to an enum or interface to the doc.                                                                                                                                                     |
| [#enumeration-types](../../../../language-reference/declarations.md#enumeration-types) | undocumented-behavior | The doc contains a Note: "The current Slang implementation has bugs that prevent explicit tag types from working correctly." No diagnostic code, no description of what goes wrong. The claim is partially blocked.                                                                                                          | Replace the Note with a specific known-bugs table or remove it and file a bug.                                                                                                                                                                       |
