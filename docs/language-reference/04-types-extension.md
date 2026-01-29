# Type Extension

## Syntax

[Struct extension](#struct) declaration:
> **`'extension'`** *`type-expr`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> **`'{'`** *`member-list`* **`'}'`**

[Generic struct extension](#generic-struct) declaration:
> **`'extension'`** *`generic-params-decl`* *`type-expr`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> **`'{'`** *`member-list`* **`'}'`**

### Parameters

- *`type-expr`* is the type to extend.
- *`generic-params-decl`* are the generic parameters for a [generic struct extension](#generic-struct).
- *`bases-clause`* is an optional list of [interface](04-types-interface.md) conformance specifications to be added.
- *`where-clause`* is an optional generic constraint expression. See [Generics (TODO)](TODO).
- *`member-list`* is a list of struct members to be added. A member is one of:
  - *`var-decl`* is a member static variable declaration. See [Variables (TODO)](TODO)
  - *`type-decl`* is a nested [type declaration](04-types.md).
  - *`function-decl`* is a member function declaration. See [Functions (TODO)](TODO)
  - *`constructor-decl`* is a [constructor declaration](04-types-struct.md#constructor).
  - *`property-decl`* is a [property declaration](04-types-struct.md#property).
  - *`subscript-op-decl`* is a [subscript operator declaration](04-types-struct.md#subscript-op).
  - *`function-call-op-decl`* is a [function call operator declaration](04-types-struct.md#function-call-op).


## Description

An existing `struct` type or a set of `struct` types can be extended with one or more `extension`
declarations. An `extension` may be used to add static data members, member functions, constructors,
properties, subscript operators, and function call operators to an existing type. An `extension` may not
change the data layout of a `struct`, that is, it cannot be used to append non-static data members.

> 📝 **Remark:** An [interface](04-types-interface.md) type cannot be extended. This would add new requirements
> to all conforming types, which would invalidate existing conformances.

## Struct Extension {#struct}

A previously defined `struct` can be extended using an `extension` declaration. The declaration appends new
members to the `struct` definition.

**Example 1:**
```hlsl
struct ExampleStruct
{
    uint32_t a;

    uint32_t getASquared()
    {
        return a * a;
    }
}

extension ExampleStruct
{
    // add a member function to ExampleStruct
    [mutating] void addToA(uint32_t x)
    {
        a = a + x;
    }
}
```

An extension can also be used to provide interface requirements to a struct.

**Example 2:**
```hlsl
interface IReq
{
    int requiredFunc();
}

struct TestClass : IReq
{
}

extension TestClass
{
    int requiredFunc()
    {
        return 42;
    }
}

[shader("compute")]
void main(uint3 id : SV_DispatchThreadID)
{
    TestClass obj = {  };

    obj.requiredFunc();
}
```

And finally, an extension can add new interface conformances to a struct:

**Example 3:**
```hlsl
interface IReq
{
    int requiredFunc();
}

struct TestClass
{
}

extension TestClass : IReq
{
    int requiredFunc()
    {
        return 42;
    }
}

[shader("compute")]
void main(uint3 id : SV_DispatchThreadID)
{
    IReq obj = TestClass();

    obj.requiredFunc();
}
```

> ⚠️ **Warning:** When an extension and the base structure contain a member with the same signature, it is
> currently undefined which member is effective. ([Issue #9660](https://github.com/shader-slang/slang/issues/9660))


## Generic Struct Extension {#generic-struct}

All structs conforming to an interface may be extended using a generic extension declaration. The generic
extension declaration adds new members to all conforming types. In case there are multiple declarations with
the same signature, the one in the concrete type takes precedence.

**Example:**
```hlsl
interface IBase
{
    int getA();
}

struct ConcreteInt16 : IBase
{
    int16_t a;

    int getA()
    {
        return a;
    }
}

struct ConcreteInt32 : IBase
{
    int32_t a;

    int getA()
    {
        return a;
    }
}

extension<T : IBase> T
{
    // added to all types conforming to
    // interface IBase
    int getASquared()
    {
        return getA() * getA();
    }
}
```

See [Generics (TODO)](TODO) for further information on generics.
