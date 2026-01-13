# Type Extension

An existing `struct` type or a set of `struct` types can be extended with one or more `extension`
declarations. An `extension` may be used to add static data members, member functions, constructors,
properties, subscript operators, and function call operators to an existing type. An `extension` may not
change the data layout of a `struct`, that is, it cannot be used to append non-static data members.

> Remark. An interface type itself cannot be extended. This would add new requirements to all conforming types
> in which case all the conforming types would require matching extensions.

## Struct Extension

A previously defined `struct` can be extended using an `extension` declaration. The declaration appends new
members in the `struct` definition.

A struct with a non-`static` unknown-length array member may not be extended with non-`static` data members.

Example:

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

Example:

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


## Generic Struct Extension

All structs conforming to an interface may be extended using a generic extension declaration. The generic
extension declaration adds new members to all conforming types. In case there are multiple declarations with
the same signature, the one in the concrete type takes precedence.

Example

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
