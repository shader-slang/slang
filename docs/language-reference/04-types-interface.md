# Interfaces

An `interface` specifies a set of member functions that a conforming type must provide. An interface can then
be used in place of a concrete type, allowing for the same code to use different concrete types via the
methods defined by the interface.

An interface consists of:

- Any number of member function prototypes, which a concrete type inheriting from the interface must provide.
- Any number of member functions, which are added to the inheriting type (either a concrete `struct` or
  another `interface`).
  - Member functions may be overridden by inheriting types using the `override` keyword with the member
    function declaration.

The interface member functions may be static or non-static.

An object inheriting from an interface can be converted to the interface type.

An interface may also inherit from another interface. The inherited members add to the inheriting interface.


## Interface-Conforming Variants

A variable declared with an interface type is an *interface-conforming variant*–or *interface variant* for
short. An interface variant may have any type conforming to the interface. When an interface variant is
instantiated, the following restrictions apply:

- The types conforming to the interface type may not have data members with opaque types such as `Texture2D`
- The types conforming to the interface type may not have data members with non-copyable types
- The types conforming to the interface type may not have data members with unsized types

Further, invoking a member function of an interface variant has performance overhead due to dynamic
dispatching.

> Remark 1: Function parameters with interface types do not impose the above restrictions when invoked with
> variables with types known at compile time.

> Remark 2: Initializing an interface variant using the default initializer is deprecated. Invoking a
> default-initialized interface variant is undefined behavior.

> Remark 3: In `slangc`, an interface variant is said to have an
> [existential type](https://en.wikipedia.org/wiki/Type_system#Existential_types), meaning that its type
> exists such that it conforms to the specified interface.


## Example

```hlsl
RWStructuredBuffer<int> outputBuffer;

interface IBase
{
    // a member function that concrete types
    // must define
    int getA();
}

interface ITest : IBase
{
    // Note: concrete types inheriting from this interface
    // must define getA()

    // a member function defined by the interface,
    // to be overridden by ConcreteInt16
    int someFunc()
    {
        return getA() + 2;
    }

    static int getUnderlyingWidth();
}

struct ConcreteInt32 : ITest
{
    int32_t a;

    int32_t getA()
    {
        return a;
    }

    static int getUnderlyingWidth()
    {
        return 32;
    }
}

struct ConcreteInt16 : ITest
{
    int16_t a;

    int getA()
    {
        return a;
    }

    // override default implementation of someFunc()
    override int someFunc()
    {
        return a + 5;
    }

    static int getUnderlyingWidth()
    {
        return 16;
    }
}

// This function accepts any object conforming to
// interface ITest
int getValSquared(ITest i)
{
    int lhs = i.getA();
    return i.getA() * i.getA();
}

// This function creates a concrete object and returns
// it as an interface variant.
ITest createConcrete(bool is32, uint initialValue)
{
    if (is32)
        return ConcreteInt32(initialValue);
    else
        return ConcreteInt16(initialValue);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    int ret;

    // Pass an object to a function
    // with interface-typed argument
    if ((id.x & 1) == 1)
    {
        ConcreteInt32 val = { id.y };

        // A copy of getValSquared() is specialized for
        // ConcreteInt32.
        ret = getValSquared(val);
    }
    else
    {
        ConcreteInt16 val = { id.y };

        // A copy of getValSquared() is specialized for
        // ConcreteInt16.
        ret = getValSquared(val);
    }

    outputBuffer[id.x] = float(ret);

    // An interface variant. Declaring this imposes
    // restrictions on the types conforming to the
    // interface.
    ITest iobj;

    if ((id.x & 1) == 1)
    {
        // create and assign a ConcreteInt32 object to the
        // interface variant
        iobj = createConcrete(true, id.y);
    }
    else
    {
        // create and assign a ConcreteInt16 object to the
        // interface variant
        iobj = createConcrete(false, id.y);
    }

    // Note: Invoking a member function of an interface variant
    // has the overhead of dynamic dispatching.
    outputBuffer[id.x] += float(iobj.someFunc());

    // Dynamic dispatch overhead also here.
    outputBuffer[id.x] += iobj.getUnderlyingWidth();
}
```


# Memory Layout and Dispatch Mechanism

The memory layout of an interface variant is unspecified. Type-based dynamic dispatching of a member function
invocation is unspecified. Both are subject to change in future versions of `slangc`.

## Non-Normative Description of Interface Variants

> Remark: The contents of this section is informational only and subject to change.

In the current implementation, the layout of an interface is a tagged union, conceptually as follows:

```hlsl
// Note: unions do not exist in Slang
union InterfaceConcreteObjectTypes
{
    ConcreteType1 obj1;
    ConcreteType2 obj2;
    ConcreteType3 obj3;
    // ...
}

struct InterfaceImplementationType
{
    uint32_t typeTag;
    InterfaceConcreteObjectTypes tuple;
}
```

However, since Slang does not have union types where the underlying data is reinterpreted as one of the union
types, union types are emulated. Emulation is performed by packing/unpacking the data for a concretely-typed
object to/from the underlying representation. This involves memory copies.

When the type is not known at compile-time, dynamic dispatch based on the type tag is performed to invoke
member functions. Internally, this performed as follows:

1. `switch` statement based on the type tag selects the correct type-specific implementation for the follow-up
   steps.
2. The concretely-typed object from the union is unpacked (non-static member functions only)
3. The member function of the concrete type is invoked
4. The concretely-typed object is packed back to the union (non-static mutating member functions only)

When the type is known at compile-time, the code using an interface type is specialized for the concrete
type. This avoids the performance overhead of dynamic dispatching and union types.

## Non-Normative Description of Interface-typed Function Parameters

> Remark: The contents of this section is informational only and subject to change.

In the current implementation, when a function with an interface-typed parameter is invoked with a type known
at compile time, the function is specialized for the concrete type. This essentially creates a copy of the
function with the interface-typed parameter replaced with a concrete-typed parameter.

See also [Generic Functions](TODO-Generics.md).
