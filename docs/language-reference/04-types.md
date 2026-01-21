# Types

The types in the Slang language consist of the following categories:

* [4a - Fundamental Types](04-types-fundamental.md)
* [4b - Vector and Matrix Types](04-types-vector-and-matrix.md)
* [4c - Structures](04-types-struct.md)
* [4d - Interfaces](04-types-interface.md)
* [4e - Extensions](04-types-extension.md)
* [4f - Array types](04-types-array.md)
* [4g - Opaque types](04-types-opaque.md)

A [type specifier](#specifier) names a type. Type specifiers are used in variable declarations, function
parameter and return type declarations, and elsewhere where a type is required. Type specifiers are divided
into two categories:

- A **simple type specifier** is a type expression that names a type but never declares one. Simple type
  specifiers are used in function parameter and return type declarations, modern variable declarations, type
  constraints, and other places where the ability of declaring new types is not expected. Two main forms
  exist:
  - *Simple type identifier specifier* based on a previously declared type, optionally with an array
    declaration and generic parameters.
  - *Simple function type specifier* specifying a function type.
- A **type specifier** is a type expression that names a type, possibly by declaring it. A simple type
  specifier is a subset of the full type specifier. A type specifier is a part of the
  [variable declaration](07-declarations.md) syntax, which is used to declare variables, as the name suggests.

A type is incomplete when it is declared but not defined. An incomplete type cannot be used to declare
variables. An incomplete type other than `void` may be completed with a subsequent definition. For further
information, see [declarations](07-declarations.md).

A [type alias](#alias) is a name that refers to a previously declared type.


> 📝 **Remark:** Unlike in C++, `const`, `inline`, `volatile`, and similar keywords are modifiers. This
> restricts their allowed placement to the left of the type specifier. For example, `const int a = 5;` is a
> valid variable declaration but `int const a = 5;` is not.

## Type Specifiers {#specifier}

### Syntax

Simple type specifier:
> *`simple-type-spec`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(*`simple-type-id-spec`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;|*`simple-type-func-spec`*)

Type specifier for named non-array and array types:
> *`simple-type-id-spec`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`type-identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'['`** [*`constant-index-expr`*] **`']'`**)*

Type specifier for function types:
> *`simple-type-func-spec`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'functype'`** **`'('`** *`param-list`* **`')'`** **`'->'`** *`simple-type-id-spec`*

Full type specifier, possibly declaring a new type:
> Simple type specifier:<br>
> *`type-spec`* = *`simple-type-spec`*
> <br><br>
> struct/class/enum type specifier:<br>
> *`type-spec`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(*`struct-decl`* | *`class-decl`* | *`enum-decl`*)<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'['`** [*`constant-index-expr`*] **`']'`** )*<br>

#### Parameters

- *`modifier-list`* is an optional list of modifiers (TODO: link)
- *`type-identifier`* is an identifier that names an existing type or a generic type. This may be, *e.g.*, a
  [fundamental type](04-types-fundamental.md), [vector/matrix generic type](04-types-vector-and-matrix.md),
  user-defined type such as a named [structure type](04-types-struct.md),
  [interface type](04-types-interface.md), [enumeration type](04-types-enum.md), type alias, or a type
  provided by a module.
- *`generic-params-decl`* is a generic parameters declaration. See [Generics (TODO)](TODO).
- *`constant-index-expr`* is an optional constant integral expression for an [array](04-types-array.md) type
  declaration.
- *`param-list`* is a function parameter list. See [function parameter list (TODO)](TODO).
- *`struct-decl`* is a [structure](04-types-struct.md) type declaration, possibly also defining the type.
- *`class-decl`* is a [class (TODO)](04-types-class.md) type declaration, possibly also defining the type.
- *`enum-decl`* is an [enumeration (TODO)](04-types-enum.md) type declaration, possibly also defining the type.
- *`where-clause`* is an optional generic constraint expression. See [Generics (TODO)](TODO).

### Description

A type specifier names a type and possibly also declares a new type.

The named type is always a non-generic type. In case *`type-identifier`* specifies a generic type, generic
parameters *`generic-params-decl`* must be provided to fully specialize the type.

Simple type specifiers *`simple-type-spec`* only name types but never declare new types. Simple type
specifiers are used in:
- [modern variable (TODO)](TODO) declarations
- [function parameter (TODO)](TODO) declarations
- [function return value type (TODO)](TODO) declarations
- [structure property](04-types-struct#property)
- [structure subscript operator](04-types-struct#subscript)
- [generic type parameter declarations (TODO)](TODO)
- [typealias](#alias) declarations

Declaration of new types is allowed in:
- Global declaration statements (TODO: link)
- Function body declaration statements (TODO: link)
- Traditional variable declarations (TODO: link)
- [structure](04-types-struct.md) members declaring nested types
- [extension](04-types-extension.md) members declaring nested types
- [typedef](#alias) declarations

> Remark 1: *`simple-type-spec`* is a syntactical subset of the full *`type-expr`*. The subset only names a
> type but never declares one.

> Remark 2: The combined nature of the type expression of naming and possibly declaring a type is a
> side-effect of the C-style grammar for type declarations. This extends to traditional variable declarations
> where a single declaration can declare a type and one or more variables. (TODO: link)


## Type Alias Declarations {#alias}

### Syntax

Type alias declaration:
> **`'typealias'`** *`identifier`* **`'='`** *`simple-type-spec`* **`';'`**

Typedef declaration:
> **`'typedef'`** *`type-spec`* *`identifier`* **`';'`**

Generic type alias declaration:
> **`'typealias'`** *`identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp; *`generic-params-decl`* (**`'where'`** *`where-clause`*)\* **`'='`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`simple-type-spec`* [*`generic-params-decl`*] **`';'`**

### Description

A `typealias` declaration introduces a name for a type. A `typedef` declaration is an alternative syntax that
also allows declaring a new type.

A generic type alias declaration declares a parameterized alias for a generic type. This is described in
[Generics (TODO)](TODO).


## Memory Layout

Types in Slang do not generally have identical memory layouts in different targets. Any unspecified details on
layout may depend on the target language, the target device, the declared extensions, the compiler options,
and the context in which a type is used.


## Known and Unknown Size

Every type has either a known or an unknown size. Types with unknown size generally stem from unknown-length
arrays:
* An unknown-length array type has an unknown size.
* The size of a structure type is unknown if it has a non-static data member with unknown size.

The use of types with unknown size is restricted as follows:
* A type with unknown size cannot be used as the element type of an array
* A type with unknown size can only be used as the last field of a structure type
* A type with unknown size cannot be used as a generic argument to specialize a user-defined type, function,
  etc. Specific built-in generic types/functions may support unknown-size types, and this will be documented
  on the specific type/function.
* A type with unknown size cannot be instantiated as a variable.

> Remark: Unknown size is different from unspecified or target-specified size. [Opaque types](04-types-opaque.md)
> have target-specified sizes, sizes of [structures](04-types-struct.md) and [arrays](04-types-array.md) are subject to
> target-specific alignment rules, and certain [4a - Fundamental Types](04-types-fundamental.md) such as
> `bool` have target-specified size. Unspecified-sized types are not subject to the restrictions of
> unknown-sized types.
