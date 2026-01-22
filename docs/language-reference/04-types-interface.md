# Interfaces

An `interface` specifies a set of member functions that a conforming type must provide. An interface can then
be used in place of a concrete type, allowing for the same code to use different concrete types via the
methods defined by the interface.

An interface consists of:

- Any number of member function prototypes, constructors, or function call operator declarations. A concrete
  type inheriting from the interface must provide compatible member function implementations with the same
  names.
- Any number of member functions with the implementation body, which are added to the inheriting type (either
  a concrete `struct` or another `interface`). Constructors are not allowed.
  - Inheriting types may override member functions by using the `override` keyword with a compatible member
    function declaration.
- Any number of property or `__subscript` declarations. Interface may declare either `get` or `set` or
  both methods without the implementation body. A concrete type inheriting from the interface must provide
  implementations for the properties and `__subscript` declarations.
  - A property or a `__subscript` declaration may be implemented with compatible `get` and `set` methods
    as required by the interface.
  - Alternatively, a property may be implemented by declaring a compatible variable with a matching name.
- Any number of associated named types, which a concrete inheriting type must provide. An associated named
  type may be provided by:
  - Declaring a nested `struct` with the same name. OR
  - Defining a type alias for the name with `typealias` or `typedef`.
- Any number of `static` `const` data members without initializers. A concrete inheriting type must provide
  compatible static data members with the same names and types.
  - The type of a `static` `const` member must be either `int` or `bool`.

The interface member functions may be static or non-static.

An object inheriting from an interface can be converted to the interface type.

An interface may also inherit from another interface. The inherited members add to the inheriting interface.

A member function implementation is compatible with an interface member function when:
- The implementation function can be called with the parameter types of the interface.; AND
- The implementation function return type can be converted to the interface type.

A member property (or variable) is compatible with an interface member property when the implementation
property (or variable) is convertible to the interface property and vice versa.

`interface` members may be declared with access control specifiers `public` or `internal`. The default member
visibility is the visibility of the `interface`. See [access control (TODO)](TODO) for further information.

When a struct implements an interface member requirement, the visibility of the member may not be higher than
the requirement. However, it can be lower.

**Example:**

```hlsl
interface IReq
{
}

interface ITest
{
    // Static data member requirement
    static const int staticDataMember;

    // Static member function requirement
    static float staticMethod(int a);

    // Property requirement
    property testProp : float
    {
        get; // must be readable
        set; // must be writable
    }

    // Constructor requirement
    __init(float f);

    // Non-static member function requirement
    float someMethod(int a);

    // Overridable non-static member function
    // with default implementation.
    float someMethodWithDefaultImplementation(int a)
    {
        return testProp + float(a);
    }

    // Function call operator requirement
    float operator () (uint x, uint y);

    // Subscript operator requirement
    __subscript (uint i0) -> float { get; set; }

    // Associated type requirement
    associatedtype AssocType;

    // Associated type requirement, provided type must
    // conform to IReq
    associatedtype AssocTypeWithRequirement : IReq;
}

struct TestClass : ITest
{
    // Required data member
    static const int staticDataMember = 5;

    // Required static member function
    static float staticMethod(int a)
    {
        return float(a) * float(a);
    }

    float propUnderlyingValue;
    float arr[10] = { };

    // Required constructor
    __init(float f)
    {
        propUnderlyingValue = f + 1.0f;
    }

    // Required property
    //
    // Note that alternatively, a data member
    // "float testProp;" could have also be provided.
    property testProp : float
    {
        get
        {
            return propUnderlyingValue - 1.0f;
        }

        set(float newVal)
        {
            propUnderlyingValue = newVal + 1.0f;
        }
    }

    // Required non-static member function
    //
    // Note that the parameters and the return value
    // are not required to match as long as they are
    // compatible.
    float someMethod(int64_t a)
    {
        return float(a) * propUnderlyingValue;
    }

    // Required function call operator
    float operator () (uint x, uint y)
    {
        return float(x * y);
    }

    // Required subscript operator
    __subscript (uint i0) -> float
    {
        get
        {
            return arr[i0];
        }

        set
        {
            arr[i0] = newValue;
        }
    }

    // Required associated type provided by using a
    // type alias.
    typealias AssocType = int;

    // Required associated type provided by a nested type.
    struct AssocTypeWithRequirement : IReq
    {
    }
}
```

> Remark 1: The test for an inheriting member function compatibility is equivalent to whether a wrapper
> function with the interface member function signature may invoke the inheriting member function, passing the
> parameters and the return value as is.
>
> For example:
> ```hlsl
> interface IBase
> {
>     // a member function that a concrete type
>     // must provide
>     int32_t someFunc(int8_t a, int16_t b);
> }
>
> struct Test : IBase
> {
>     // Implementation of IBase.someFunc(). This is
>     // compatible because the corresponding wrapper
>     // is well-formed (see below)
>     int16_t someFunc(int a, int32_t b)
>     {
>         return int16_t(a + b);
>     }
> }
>
> // A wrapper invoking Test.someFunc() with the
> // parameters and the return value of the interface
> // member function declaration
> int32_t IBase_wrapper_Test_someFunc(
>     Test obj, int8_t a, int16_t b)
> {
>     return obj.someFunc(a, b);
> }
> ```

> Remark 2: An interface can also be parameterized using generics.
>
> For example:
>
> ```hlsl
> interface ITypedReq<T>
> {
>     T someFunc(T param);
> }
>
> struct TestClass : ITypedReq<uint>
> {
>     uint someFunc(uint param)
>     {
>         return 123 + param;
>     }
> }
>
> [shader("compute")]
> void main(uint3 id : SV_DispatchThreadID)
> {
>     TestClass obj = {  };
>
>     obj.someFunc(id.x);
> }
> ```
>
> See [Generics (TODO)](TODO) for further information on generics.


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

    // a static const data member that concrete types
    // must define
    static const int8_t bias;
}

interface ITest : IBase
{
    // Note: concrete types inheriting from this interface
    // must define getA()

    // a member function defined by the interface,
    // to be overridden by ConcreteInt16
    int someFunc()
    {
        return getA() + bias;
    }

    // a static member function thet ConcreteInt16 and
    // ConcreteInt32 must provide.
    static int getUnderlyingWidth();
}

struct ConcreteInt32 : ITest
{
    static const int8_t bias = 3;

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
    static const int8_t bias = 1;

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

    outputBuffer[id.x] = ret;

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
    outputBuffer[id.x] += iobj.someFunc();

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
