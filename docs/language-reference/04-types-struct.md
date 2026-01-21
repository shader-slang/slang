# Structures

## Syntax

Struct *no-body* declaration:
> *`struct-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'struct'`** [*`identifier`*] [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*] [**`'='`** *`type-expr`*] **`';'`**

Struct *with-members* declaration:
> *`struct-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'struct'`** [*`identifier`*] [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> **`'{'`** *`member-list`* **`'}'`**

Struct *link-time extern type* declaration:
> *`struct-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'extern'`** **`'struct'`** [*`identifier`*] [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*] [**`'='`** *`type-expr`*] **`';'`**

Struct *link-time export type alias* declaration:
> *`struct-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'export'`** **`'struct'`** [*`identifier`*] [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*] **`'='`** *`type-expr`* **`';'`**

Member list:
> *`member-list`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;( *`var-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;| *`type-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;| *`function-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;| *`constructor-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;| *`property-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;| *`subscript-op-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;| *`function-call-op-decl`* )*

### Parameters

- *`modifier-list`* is an optional list of modifiers (TODO: link)
- *`identifier`* is an optional name of the declared struct type
- *`generic-params-decl`* is an optional generic parameters declaration. See [Generics (TODO)](TODO).
- *`bases-clause`* is an optional list of inherited [interfaces](04-types-interface.md).
- *`type-expr`* is an optional type expression for an alias type. See [Modules (TODO)](TODO).
- *`where-clause`* is an optional generic constraint expression. See [Generics (TODO)](TODO).
- *`member-list`* is a list of struct members. A member is one of:
  - *`var-decl`* is a member variable declaration. See [Variables (TODO)](TODO)
  - *`type-decl`* is a nested [type declaration](04-types.md).
  - *`function-decl`* is a member function declaration. See [Functions (TODO)](TODO)
  - *`constructor-decl`* is a [constructor declaration](#constructor).
  - *`property-decl`* is a [property declaration](#property).
  - *`subscript-op-decl`* is a [subscript operator declaration](#subscript-op).
  - *`function-call-op-decl`* is a [function call operator declaration](#function-call-op).

> ⚠️ **Warning:** `Slangc` currently accepts bracketed attributes right after the **`'struct'`** keyword. This
> is deprecated syntax and expected to be removed. Bracketed attributes should be added in *`modifier-list`*,
> instead. ([Issue #9691](https://github.com/shader-slang/slang/issues/9691))


## Description

A structure is a type consisting of an ordered sequence of members. A `struct` declaration has the following
forms:

- The *no-body* declaration specifies an existence of a structure type. The declaration simply specifies that
  a structure type with the specified name exists. This enables its use in type expressions without the member
  declarations.
- The *with-members* declaration defines the structure type with a layout and an [extensible](04-types-extension.md)
  list of non-layout members.
- The *link-time extern type* declaration specifies the existence of a structure type that is defined in
  another module. See [Modules (TODO)](TODO).
- The *link-time export type* declaration specifies that a structure type is exported with a type alias. See
  [Modules (TODO)](TODO).

When the *`identifier`* is specified, it is the name of the structure. Otherwise, the structure is anonymous,
which means that it is assigned an unspecified unique name. The main use of anonymous structures is in inline
type definition expressions. For example, `struct { int a; } obj;` defines variable `obj` with an anonymous
structure type that has field `int a;`. Anonymous structure declarations are meaningful only in the
*with-members* form.

A structure member is declared in the structure body and is one of the following:
- A static data member; declared as a variable with the `static` keyword.
- A non-static data member (*aka.* field); declared as a variable.
- A [constructor](#constructor).
- A [static member function](#static-member-function); declared as a function with the `static` modifier
  keyword.
- A [non-static member function](#nonstatic-member-function); declared as a function without the `static`
  modifier keyword.
- A nested type; declared as a type or a type alias.
- A [`property` declaration](#property).
- A [`__subscript` declaration](#subscript-op).
- A [function call operator declaration](#function-call-op).

A data member and a member function can be declared with the `static` keyword.

- The storage for a static data member is allocated from the global storage. A static member function may:
  - Access static data members of the structure.
  - Invoke other static member functions of the structure.
- The storage for a non-static data member is allocated as part of the structure. A non-static member function may:
  - Access both the static and the non-static data members.
  - Invoke both the static and the non-static member functions.

Data members may be assigned with a default initializer. The following rules apply:
- When an object is initialized using an initializer list, the default initializer of a non-static data member
  specifies the initial value when the initializer list does not provide one.
- When an object is initialized using a constructor, the default initializer of a non-static data member
  specifies the initial value of the data member. A constructor may override this, unless the member is
  `const`.
- `static const` data members must have a default initializer.

For further information, see [Initialization (TODO)](TODO).

The non-static data members are allocated sequentially within the `struct` when a variable of this type is
allocated. See [Variables (TODO)](TODO).

A nested type is a regular type enclosed within the scope of the outer `struct`.

A structure may conform to one or more [interface](04-types-interface.md) types.

A structure may be extended with a [type extension](04-types-extension.md).

`struct` members may be declared with access control specifiers `public`, `internal`, or `private` (specified
in *`modifier-list`*). The default member visibility is `internal`. Nested `struct` members have access to
`private` members of the enclosing `struct`. See [access control (TODO)](TODO) for further information.


> ⚠️ **Warning:** Structure inheriting from another structure is deprecated. It may not work as expected.


## Objects {#object}

An object is an *instance* of a `struct`. An instance consists of all non-static data members defined in a
`struct`. The data members may be initialized using an initializer list or a constructor. For details, see
[variable declarations](07-declarations.md).

## Constructors {#constructor}

### Syntax

Declaration without body: (interfaces only)
> **`'__init'`** **`'('`** *`param-list`* **`')'`** (**`'where'`** *`where-clause`*)\* **`';'`**

Declaration with body:
> **`'__init'`** **`'('`** *`param-list`* **`')'`** (**`'where'`** *`where-clause`*)\*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'{'`** *`body-stmt`*\*  **`'}'`**


### Description

When a user-provided constructor is defined for a `struct`, a constructor is executed on object
instantiation. A constructor can have any number of parameters. A constructor does not have a return
type. More than one constructors may be defined in which case overload resolution is performed to select the
most appropriate constructor given the initialization parameters.

The constructor parameters are provided in the optional initializer list. When an initializer is not provided,
the no-parameter constructor is invoked.

If a non-static data member is not initialized by the constructor, it has an undefined state after object
instantiation.

`const` data members cannot be initialized by the constructor.

*`where-clause`* is an optional generic constraint expression, discussed in [Generics (TODO)](TODO).


**Example:**
```hlsl
struct TestClass
{
    int a, b;

    __init()
    {
        a = 1;
        b = 2;
    }

    __init(int _a)
    {
        a = 1;
        b = 2;
    }

    __init(int _a, int _b)
    {
        a = _a;
        b = _b;
    }
}

TestClass obj1;
// obj1.a = 1;
// obj1.b = 2;
//
// Note: TestClass obj1 = { }; also calls the constructor
// without parameters

TestClass obj2 = { 42 };
// obj2.a = 42;
// obj2.b = 2;

TestClass obj3 = { 42, 43 };
// obj3.a = 42;
// obj3.b = 43;

```

When no user-provided constructor is defined, an aggregate initialization is performed, instead. In aggregate
initialization, an initializer list contains values for the `struct` non-static data members. If the
initializer list does not contain enough values, the remaining data members are default-initialized. If no
initializer list is provided, a class without a user-provided constructor is instantiated in an undefined
state.

> Remark 1: When a class without user-provided constructor is instantiated without an initializer list, the
> object's initial state is undefined. This includes data members which have members with user-provided
> constructors.
>
> ```hlsl
> struct TestField
> {
>     int x;
>     __init() { x = 5; }
> }
>
> struct TestClass
> {
>     int a, b;
>     TestField f;
> }
>
> // note: obj is instantiated with an undefined state
> // regardless of TestField having a user-provided constructor.
>
> TestClass obj;
> ```

> Remark 2: Accessing data members that are in undefined state is undefined behavior.


## Static Member Functions {#static-member-function}

A static member function is a regular function enclosed within the `struct` name space. Static member
functions may access only static structure members.

Invocation of a static member function does not require an object.


## Non-static Member Functions {#nonstatic-member-function}

A non-static member function has a hidden parameter `this` that refers to an object. The hidden parameter
is used to reference the object data members and to invoke other non-static member functions.

In the function body, other members may be referenced using `this.`, although it is optional.

By default, only a read access to the object members is allowed by a member function. If write access is
required, the member function must be declared with the `[mutating]` attribute.

Non-static member functions cannot be invoked without an object.

> Remark: In C++ terminology, a member function is `const` by default. Attribute `[mutating]` makes it
> a non-`const` member function.


## Properties {#property}

### Syntax

Modern syntax, implicit `get` declaration: (interfaces only)
> **`'property'`** *`identifier`* **`':'`** *`type-expr`* **`';'`**

Modern syntax, explicit accessor declarations:
> **`'property'`** *`identifier`* **`':'`** *`type-expr`*<br>
> **`'{'`** *`accessor-decl`*\*  **`'}'`**

Traditional syntax, implicit `get` declaration: (interfaces only)
> **`'property'`** *`traditional-var-decl`* **`';'`**

Traditional syntax, explicit accessor declarations:
> **`'property'`** *`traditional-var-decl`*<br>
> **`'{'`** *`accessor-decl`*\*  **`'}'`**

Accessor declaration syntax, no body: (interfaces only)
> *`accessor-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'get'`** \| **`'set'`**) [**`'('`** *`param-list`* **`')'`**] **`';'`**

Accessor declaration syntax, with body:
> *`accessor-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'get'`** \| **`'set'`**) [**`'('`** *`param-list`* **`')'`**]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'{'`** *`body-stmt`*\*  **`'}'`**

### Description

A property is a non-static member that provides a data member access interface. Properties of objects are
accessed similarly to data members: reading a property is directed to the `get` accessor of the property and
writes are directed to the `set` accessor, respectively.

A property that only provides the `get` accessor is a read-only property. A property that only provides the
`set` accessor is a write-only property. A property that provides both is a read/write property.

The parentheses in the `get` accessor declaration are optional. The `get` accessor accepts no parameters.

The parentheses and the parameter in the `set` accessor declaration are optional. In case the parameter is not
specified in the declaration, parameter `newValue` with the same type as the property is provided to the `set`
body.

The property declaration forms without accessor or accessor body declarations are useful only in
[interface](04-interface.md) declarations.

> ⚠️ **Warning:** Property reference accessor `ref` is a Slang internal language feature. It is subject to
> change and may not work as expected.


**Example:**
```hlsl
struct TestClass
{
    float m_val;

    // automatically updated derivative of m_val
    bool m_valIsPositive;

    property someProp : float
    {
        get
        {
            return m_val;
        }

        set
        {
            m_val = newValue;
            m_valIsPositive = (newValue > 0.0f);
        }
    }
}

[shader("compute")]
void main(uint3 id : SV_DispatchThreadID)
{
    TestClass obj = { };

    // this sets both obj.m_val and obj.m_valIsPositive
    obj.someProp = 3.0f;
}
```

> Remark 1: A property can be used to replace a non-`static` data member when additional logic is desired to
> be added systematically to data member access. This can avoid refactoring call sites.

> Remark 2: A non-static data member can be used to implement an interface property requirement. See
> [interfaces](04-types-interface.md) for details.

> Remark 3: In the example above, the property could have also been declared as:
>
> ```hlsl
> struct TestClass
> {
>     // ...
>
>     property someProp : float
>     {
>         get()
>         {
>             return m_val;
>         }
>
>         set(float newVal)
>         {
>             m_val = newVal;
>             m_valIsPositive = (newVal > 0.0f);
>         }
>     }
> }
> ```

## Accessing Members and Nested Types

The static and non-static structure members and nested types are accessed using \``.`\`.

> ⚠️ **Warning:** The C++-style scope resolution operator \``::`\` is deprecated. It should not be used.


**Example:**

```hlsl
// struct type declaration
struct TestStruct
{
    // data member
    int a;

    // static data member, initial value 5
    static int b = 5;

    // static constant data member, initial value 6
    static const int c = 6;

     // nested type
    struct NestedStruct
    {
        static int c = 6;
        int d;
    }

    // member function with read-only access
    // to non-static data members
    int getA()
    {
        // also just plain "return a" would do
        return this.a;
    }

    // member function with read/write access
    // to non-static data members
    [mutating] int incrementAndReturnA()
    {
        // modification of data member
        // requires [mutating]
        a = a + 1;

        return a;
    }

    // static member function
    static int getB()
    {
        return b;
    }

    static int incrementAndReturnB()
    {
        // [mutating] not needed for
        // modifying static data member
        b = b + 1;

        return b;
    }
}

// instantiate an object of type TestStruct using defaults
TestStruct obj = { };

// instantiate an object of type NestedStruct
TestStruct.NestedStruct obj2 = { };

// access an object data member directly
obj.a = 42;

// access a static data member directly
int tmp0 = TestStruct.b + TestStruct.NestedStruct.c;

// invoke object member functions
int tmp1 = obj.getA();
int tmp2 = obj.incrementAndReturnA();

// invoke static members functions

// '.' can be used to resolve scope
int tmp3 = TestStruct.getB();

// '::' is equivalent to '.' for static member access,
// but '.' is recommended.
int tmp4 = TestStruct::incrementAndReturnB();
```

## Subscript operator {#subscript-op}

### Syntax

Implicit `get` declaration: (interfaces only)
> **`'__subscript'`** [**`'('`** *`param-list`* **`')'`**]<br>

Explicit accessor declarations:
> **`'__subscript'`** [**`'('`** *`param-list`* **`')'`**] **`'->'`** *`type-expr`*<br>
> **`'{'`** *`accessor-decl`*\*  **`'}'`**

See [properties](#property) for *`accessor-decl`* syntax.

### Description

A subscript `[]` operator can be added in a structure using a `__subscript` declaration. It is conceptually
similar to a `property` with the main differences being that it operates on the instance of a `struct`
(instead of a member) and it accepts parameters.

A subscript declaration may have any number of parameters, including no parameters at all.

The `get` accessor of a `__subscript` declaration is invoked when the subscript operator is applied to an
object to return a value. The parentheses in the `get` accessor declaration are optional.

The `set` accessor of a `__subscript` declaration is invoked when the subscript operator is applied to an
object to assign a value. The parentheses and the parameter in the `set` accessor declaration are optional. In
case the parameter is not specified in the declaration, a parameter `newValue` with the same type as specified
for the subscript operator is provided to the `set` body.

Multiple `__subscript` declarations are allowed as long as the declarations have different
signatures. Overload resolution is the same as overload resolution with function invocations.

> ⚠️ **Warning:** Subscript operator reference accessor `ref` is a Slang internal language feature. It is
> subject to change and may not work as expected.

**Example:**

```hlsl
RWStructuredBuffer<float> outputBuffer;

struct TestStruct
{
    var arr : float[10][10];

    // declare a 0-parameter subscript operator
    __subscript () -> float
    {
        get { return arr[0][0]; }
        set { arr[0][0] = newValue; }
    }

    // declare a 1-parameter subscript operator
    __subscript (int i) -> float
    {
        get { return arr[0][i]; }
        set { arr[0][i] = newValue; }
    }

    // declare a 2-paramater subscript operator
    __subscript (int i0, int i1) -> float
    {
        get { return arr[i1][i0]; }
        set { arr[i1][i0] = newValue; }
    }
}

void main(uint3 id : SV_DispatchThreadID)
{
    TestStruct x = { };

    x[] = id.z;
    x[id.y] = id.z;
    x[id.x, id.y] = id.z;

    outputBuffer[id.x] = x[];
    outputBuffer[id.y] = x[id.x];
    outputBuffer[id.z] = x[id.x, id.y];
}
```

## Function call operator {#function-call-op}

### Syntax

Declaration without body: (interfaces only)
> *`type-expr`* **`'operator'`** **`'(' ')'`** **`'('`** *`param-list`* **`')'`** **`';'`**

Declaration with body:
> *`type-expr`* **`'operator'`** **`'(' ')'`** **`'('`** *`param-list`* **`')'`**<br>
> **`'{'`** *`body-stmt`*\* **`'}'`**

### Description

A function call `()` operator can be added using an `operator ()` declaration. This allows applying parameters
to an object as if the object was a function.

Multiple declarations are allowed as long as the declarations have different signatures. Overload resolution
is the same as overload resolution with function invocations.

**Example:**

```hlsl
RWStructuredBuffer<float> outputBuffer;

struct TestStruct
{
    float base;

    float operator () ()
    {
        return base;
    }

    float operator () (uint x)
    {
        return base * float(x);
    }

    float operator () (uint x, uint y)
    {
        return base * float(x) * float(y);
    }
}

void main(uint3 id : SV_DispatchThreadID)
{
    TestStruct obj = { 42.0f };

    outputBuffer[0] += obj();
    outputBuffer[0] += obj(id.y);
    outputBuffer[0] += obj(id.z, id.z * 2);
}
```


# Memory Layout

## Natural Layout

The *natural layout* for a structure type uses the following rules:

- The alignment of a structure is the maximum of 1, alignment of any member, and alignment of any parent type.
- The data is laid out in order of:
  - Parent types
  - Non-static data members
- Offset of the data items:
  - The offset of the first data item is 0
  - The offset of the *Nth* data item is the offset+size of the previous item rounded up to the alignment of
    the item
- The size of the structure is offset+size of the last item. That is, the structure is not tail-padded and rounded
  up to the alignment of the structure.

The following algorithm may be used:

1. Initialize variables `size` and `alignment` to zero and one, respectively
2. For each field `f` of the structure type:
   1. Update `alignment` to be the maximum of `alignment` and the alignment of `f`
   2. Set `size` to the smallest multiple of `alignment` not less than `size`
   3. Set the offset of field `f` to `size`
   4. Add the size of `f` to `size`

When this algorithm completes, `size` and `alignment` will be the size and alignment of the structure type.

> Remark: Most target platforms do not use the natural layout directly, but it provides a baseline for
> defining other layouts. Any layout for a structure type must guarantee an alignment at least as large as the
> standard layout.

## C-Style Layout

The C-style layout of a structure type differs from the natural layout in that the structure size is rounded
up to the structure alignment. This mirrors the layout rules used by typical C/C++ compilers.

## D3D Constant Buffer Layout

D3D constant buffer layout is similar to the natural layout with two differences:

- The minimum alignment is 16.
- If a data member crosses a 16-byte boundary and its offset is not aligned by 16, the offset is rounded up to the
  next multiple of 16.
  - In HLSL, this is called an _improper straddle_.

This Type
---------

Within the body of a structure or interface declaration, the keyword `This` may be used to refer to the
enclosing type. Inside of a structure type declaration, `This` refers to the structure type itself.  Inside
of an interface declaration, `This` refers to the concrete type that is conforming to the interface (that is,
the type of `this`).
