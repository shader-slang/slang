# Type Extension

## Syntax

[Extension](#extension) declaration:
> **`'extension'`** *`type-expr`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> **`'{'`** *`member-list`* **`'}'`**

[Generic extension](#generic) declaration:
> **`'extension'`** *`generic-params-decl`* *`type-expr`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> **`'{'`** *`member-list`* **`'}'`**

### Parameters

- *`type-expr`* is the type to extend.
- *`generic-params-decl`* are the generic parameters for a [generic extension](#generic).
- *`bases-clause`* is an optional list of [interface](types-interface.md) conformance specifications to be added.
- *`where-clause`* is an optional [generic constraint expression](generics.md).
- *`member-list`* is a list of members to be added. A member is one of:
  - *`var-decl`* is a static member variable declaration. See [Variables (TODO)](TODO)
  - *`type-decl`* is a nested [type declaration](types.md).
  - *`function-decl`* is a member function declaration. See [Functions (TODO)](TODO)
  - *`constructor-decl`* is a [constructor declaration](types-struct.md#constructor).
  - *`property-decl`* is a [property declaration](types-struct.md#property).
  - *`subscript-op-decl`* is a [subscript operator declaration](types-struct.md#subscript-op).
  - *`function-call-op-decl`* is a [function call operator declaration](types-struct.md#function-call-op).


## Description {#extension}

An `extension` declaration extends a `struct` type, an `enum` type, or a set of such types. An `extension` may
be used to add static data members, member functions, constructors, properties, subscript operators, function
call operators, and conformances to an existing type. An `extension` may not change the data layout, that is,
it cannot be used to append non-static data members.

> 📝 **Remark:** An [interface type](types-interface.md) cannot be extended. Doing so would add new
> requirements to all conforming types, which would invalidate existing conformances.

## Struct Extension {#struct}

A previously defined [struct type](types-struct.md) can be extended using an extension declaration. In the
following example, an extension is used to add a new member function.

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

Finally, an extension can add new interface conformances to a struct.

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
> currently undefined which member takes effect. ([Issue #9660](https://github.com/shader-slang/slang/issues/9660))


## Enumeration Extension {#enum}

Similar to a `struct`, a previously defined [enum type](types-enum.md) can be extended using an `extension`
declaration. For non-static member functions, `this` is the value of the enumeration object.

An enumerator is an immutable instance of an enumeration type.

The first example shows basic enumeration extension.

**Example 1:**
```hlsl
enum TestEnum
{
    VALUE1 = 1,
    VALUE2,
    VALUE3,
    VALUE4,
}

extension TestEnum
{
    static int staticValue = 42;
    static int staticFunction()
    {
        return 123;
    }

    int times2()
    {
        // 'this' is the value of the
        // enumeration object
        return this * 2;
    }
}


RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    TestEnum b = TestEnum.VALUE1;

    // static members work the same as in structs
    output[0] = TestEnum.staticValue;      // 42
    output[1] = TestEnum.staticFunction(); // 123

    // member function returning double the current
    // value
    output[2] = b.times2();           // 1 * 2 = 2

    // member functions also work on enumerators
    output[3] = b.VALUE4.times2();    // 4 * 2 = 8
}
```

The second example shows various ways an enum can be extended.

**Example 2:**

```hlsl
interface IBase
{
    static const int requiredConstant;
    property int requiredProp { get; set; }
    int requiredFunc();
}

enum TestEnum
{
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3,
}

enum AnotherEnum
{
    SomeValue = 42,
}

extension TestEnum : IBase
{
    // constant value required by IBase
    static const int requiredConstant = 42;

    // property required by IBase
    property int requiredProp {
        get() { return this; }
        set(int newVal) { this = (TestEnum)newVal; }
    }

    // member function required by IBase
    int requiredFunc()
    {
        return -this;
    }

    // subscript operator
    __subscript(int index) -> int {
        get() { return index * this; }
    }

    // incrementing member function
    [mutating] void increment()
    {
        this = (TestEnum)(this + 1);
    }

    // function call operator
    float operator () (float scale)
    {
        return (float)this * scale;
    }

    __init(AnotherEnum other)
    {
        // first, capture the underlying value to
        // avoid recursion
        int underlying = other;
        this = (TestEnum)(underlying * 2);

        // Note: the following would recursively
        // call this function again
        // this = (TestEnum)other;
    }
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    TestEnum tmp = TestEnum.Two;
    output[0] = tmp.requiredProp; // 2

    tmp.requiredProp = 55;
    output[1] = tmp.requiredProp; // 55

    tmp.increment();
    output[2] = tmp.requiredProp; // 56

    output[3] = TestEnum.Three.requiredFunc(); // -3

    output[4] = tmp[4];           // 56 * 5 = 224
    output[5] = (int)(TestEnum.Two(0.5));  // 1

    tmp = TestEnum(AnotherEnum.SomeValue); // 42 * 2
    output[6] = tmp;                       // 84

    // Illegal, since enumerators are immutable
    // TestEnum.One.increment();

}
```

## Generic Extension {#generic}

All types conforming to an interface may be extended using a generic extension declaration, which adds new
members to all conforming types. If multiple declarations share the same signature, the one in the concrete
type takes precedence.

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

See [Generics](generics.md) for further information on generics.
