---
layout: user-guide
permalink: /user-guide/interfaces-generics
---

Interfaces and Generics
===========================

This chapter covers two interrelated Slang language features: interfaces and generics. We will talk about what they are, how do they relate to similar features in other languages, how are they parsed and translated by the compiler, and show examples on how these features simplifies and modularizes shader code.

Interfaces
----------

Interfaces are used to define the methods and services a type should provide. You can define a interface as the following example:
```csharp
interface IFoo
{
    int myMethod(float arg);
}
```

Slang's syntax for defining interfaces are similar to `interface`s in C# and `protocol`s in Swift. In this example, the `IFoo` interface establishes a contract that any type conforming to this interface must provide a method named `myMethod` that accepts a `float` argument and returns an `int` value.

A `struct` type may declare its conformance to an `interface` via the following syntax:
```csharp
struct MyType : IFoo
{
    int myMethod(float arg)
    {
        return (int)arg + 1;
    }
}
```
By declaring the conformance to `IFoo`, the definition of `MyType` must include a method named `myMethod` with a matching signature to that defined in the `IFoo` interface to satisfy the declared conformance. If a type misses any methods required by the interface, the Slang compiler will generate an error message.

A `struct` type may declare multiple interface conformances:
```csharp
interface IBar { uint myMethod2(uint2 x); }

struct MyType : IFoo, IBar
{
    int myMethod(float arg) {...}
    uint myMethod2(uint2 x) {...}
}
```
In this case, the definition of `MyType` must satisfy the requirements from both the `IFoo` and `IBar` interfaces by providing both the `myMethod` and `myMethod2` methods.

Generics
---------------------

Generics can be used to eliminate duplicate code for shared logic that operates on different types. The following example shows how to define a generic method in Slang.

```csharp
int myGenericMethod<T: IFoo>(T arg)
{
    return arg.myMethod(1.0);
}
```

The above listing defines a generic method named `myGenericMethod`, which accepts an argument that can be of any type `T` as long as `T` conforms to the `IFoo` interface. The `T` here is called a _generic type parameter_, and it is associated with an _type constraint_ that any type represented by `T` must conform to the interface `IFoo`.

The following listing shows how to invoke a generic method:
```csharp
MyType obj;
int a = myGenericMethod<MyType>(obj); // OK, explicit type argument
int b = myGenericMethod(obj); // OK, automatic type deduction
```

You may explicitly specify the concrete type to used for the generic type argument, by providing the types in angular brackets after the method name, or leave it to the compiler to automatically deduce the type from the argument list.

> #### Note ####
> Slang currently does not support partial type argument list deduction.
> For example if you have a generic method that accepts two type arguments:
> ```
> void g<T:IFoo, U:IBar>(T a, U b) {...}
> ```
> You may either call this method with no explicit type arguments:
> ```
> MyType a, b;
> g(a, b);
> ```
> Or with explicit arguments for both generic type parameters:
> ```
> g<MyType, MyType>(a,b);
> ```
> If you only provide first type argument, Slang will generate an error:
> ```
> g<MyType>(a,b); // error, does not work today.
> ```
> We plan to support such use in a future version.


Note that it is important to associate a generic type parameter with a type constraint. In the above example, although the definition of `myGenericMethod` is agnostic of the concrete type `T` will stand for, knowing that `T` conforms to `IFoo` allows the compiler to type-check and pre-compile `myGenericMethod` without needing to substitute `T` with any concrete types first. Similar to languages like C#, Rust, Swift and Java, leaving out the type constraint declaration on type parameter `T` will result in a compile error at the line calling `arg.myMethod` since the compiler cannot verify that `arg` has a member named `myMethod` without any knowledge on `T`. This is a major difference of Slang's generics compared to _templates_ in C++. 

While C++ templates are a powerful language mechanism, Slang has followed the path of many other modern programming languages to adopt the more structural and restricted generics feature instead. This enables the Slang compiler to perform type checking early to give more readable error messages, and to speed-up compilation by reusing a lot of work for different instantiations of `myGenericMethod`.


Supported Constructs in Interface Definitions
-----------------------------------------------------

Slang supports many other constructs in addition to ordinary methods as a part of an interface definition.

### Properties

```csharp
interface IFoo
{
    property int count {get; set;}
}
```
The above listing declares that any conforming type must define a property named `count` with both a `getter` and a `setter` method.

### Generic Methods

```csharp
interface IFoo
{
    int compute<T:IBar>(T val);
}
```
The above listing declares that any conforming type must define a generic method named `compute` that has one generic type parameter conforming to the `IBar` interface.

### Static Methods

```csharp
interface IFoo
{
    static int compute(int val);
};
```

The above listing declares that any conforming type must define a static method named `compute`. This allows the following generic method to pass type-checking:
```csharp
void f<T:IFoo>()
{
    T.compute(5); // OK, T has a static method `compute`.
}
```

### Static Constants

You can define static constant requirements in an interface. The constants can be accessed in places where a compile-time constant is needed.
```csharp
interface IMyValue
{
    static const int value;
}
struct MyObject2 : IMyValue
{
    static const int value = 2;
}
struct GetValuePlus1<T:IMyValue>
{
    static const int value = T.value + 1;
}

static const int result = GetValuePlus1<MyObject2>.value;  // result == 3
```

### `This` Type

You may use a special keyword `This` in interface definitions to refer to the type that is conforming to the interface. The following examples demonstrate a use of `This` type:
```csharp
interface IComparable
{
    int comparesTo(This other);
}
struct MyObject : IComparable
{
    int val;
    int comparesTo(MyObject other)
    {
        return val < other.val ? -1 : 1;
    }
}
```
In this example, the `IComparable` interface declares that any conforming type must provide a `comparesTo` method that performs a comparison between an object to another object of the same type. The `MyObject` type satisfies this requirement by providing a `comparesTo` method that accepts a `MyObject` typed argument, since in the scope of `MyObject`, `This` type is equivalent to `MyObject`.

### Initializers

Consider a generic method that wants to create and initialize a new instance of generic type `T`:
```csharp
void f<T:IFoo>()
{
    T obj = /*a newly initialized T*/
}
```
One way to implement this is to introduce a static method requirement in `IFoo`:
```csharp
interface IFoo
{
    static This create();
}
```
With this interface definition, we can define `f` as following:
```csharp
void f<T:IFoo>()
{
    T obj = T.create();
}
```

This solution works just fine, but it would be nicer if you can just write:
```csharp
T obj = T();
```
Or simply
```csharp
T obj;
```
And let the compiler invoke the default initializer defined in the type.
To enable this, you can include an initializer requirement in the interface definition:
```csharp
interface IFoo
{
    __init();
}
```

Initializers with parameters are supported as well. For example:
```csharp
interface IFoo
{
    __init(int a, int b);
}
void g<T:IFoo>()
{
    T obj = {1, 2}; // OK, invoking the initializer on T.
}
```

Associated Types
-------------------------

When writing code using interfaces and generics, there are some situations where the an interface method needs to return an object whose type is implementation-dependent. For example, consider the following `IFloatContainer` interface that represents a container of `float` values:
```csharp
// Represents a container of float values.
interface IFloatContainer
{
    // Returns the number of elements in this container.
    uint getCount();
    // Returns an iterator representing the start of the container.
    Iterator begin();
    // Returns an iterator representing the end of the container.
    Iterator end();
    // Return the element at the location represented by `iter`.
    float getElementAt(Iterator iter);
}
```
An implementation of the `IFloatContainer` interface may use different types of iterators. For example, an implementation that is simply an array of `float`s can expose `Iterator` as a simple integer index:
```csharp
struct ArrayFloatContainer : IFloatContainer
{
    float content[10];
    uint getCount() { return 10; }
    uint begin() { return 0; }
    uint end() { return 10; }
    float getElementAt(uint iter) { return content[iter]; }
}
```
On the other hand, an implementation that uses multiple buffers as the backing storage may use a more complex type to locate an element:
```csharp
// Exposes values in two `StructuredBuffer`s as a single container.
struct MultiArrayFloatContainer : IFloatContainer
{
    StructuredBuffer<float> firstBuffer;
    StructuredBuffer<float> secondBuffer;
    uint getCount() { return getBufferSize(firstBuffer) + getBufferSize(secondBuffer); }

    // `uint2.x` indicates which buffer, `uint2.y` indicates the index within the buffer.
    uint2 begin() { return uint2(0,0); }
    uint2 end() { return uint2 (1, getBufferSize(secondBuffer)); }
    float getElementAt(uint2 iter)
    {
        if (iter.x == 0) return firstBuffer[iter.y];
        else return secondBuffer[iter.y];
    }
}
```

Ideally, a generic function that wishes to enumerate values in a `IFloatContainer` shouldn't need to care about the implementation details on what the concrete type of `Iterator` is, and we would like to be able to write the following:
```csharp
float sum<T:IFloatContainer>(T container)
{
    float result = 0.0f;
    for (T.Iterator iter = container.begin(); iter != container.end(); iter=iter.next())
    {
        float val = container.getElementAt(iter);
        result += val;
    }
    return result;
}
```
Here the `sum` function simply wants to access all the elements and sum them up. The details of what the `Iterator` type actually is does not matter to the definition of `sum`.

The problem is that the `IFloatContainer` interface definition requires methods like `begin()`, `end()` and `getElementAt()` to refer to a iterator type that is implementation dependent. How should the signature of these methods be defined in the interface? The answer is to use _associated types_.

In addition to constructs listed in the previous section, Slang also supports defining associated types in an `interface` definition. An associated type can be defined as following.
```csharp
// The interface for an iterator type.
interface IIterator
{
    // An iterator needs to know how to move to the next element.
    This next();
}

interface IFloatContainer
{
    // Requires an implementation to define a typed named `Iterator` that
    // conforms to the `IIterator` interface.
    associatedtype Iterator : IIterator;

    // Returns the number of elements in this container.
    uint getCount();
    // Returns an iterator representing the start of the container.
    Iterator begin();
    // Returns an iterator representing the end of the container.
    Iterator end();
    // Return the element at the location represented by `iter`.
    float getElementAt(Iterator iter);
};
```

This `associatedtype` definition in `IFloatContainer` requires that all types conforming to this interface must also define a type in its scope named `Iterator`, and this iterator type must conform to the `IIterator` interface. An implementation to the `IFloatContainer` interface by using either a `typedef` declaration or a `struct` definition inside its scope to satisfy the associated type requirement. For example, the `ArrayFloatContainer` can be implemented as following:
```csharp
struct ArrayIterator : IIterator
{
    uint index;
    __init(int x) { index = x; }
    ArrayIterator next()
    {
        return ArrayIterator(index + 1);
    }
}
struct ArrayFloatContainer : IFloatContainer
{
    float content[10];

    // Specify that the associated `Iterator` type is `ArrayIterator`.
    typedef ArrayIterator Iterator;

    Iterator getCount() { return 10; }
    Iterator begin() { return ArrayIterator(0); }
    Iterator end() { return ArrayIterator(10); }
    float getElementAt(Iterator iter) { return content[iter.index]; }
}
```

Alternatively, you may also define the `Iterator` type directly inside a `struct` implementation, as in the following definition for `MultiArrayFloatContainer`:
```csharp
// Exposes values in two `StructuredBuffer`s as a single container.
struct MultiArrayFloatContainer : IFloatContainer
{
    // Represents an iterator of this container
    struct Iterator : IIterator
    {
        // `index.x` indicates which buffer the element is located in.
        // `index.y` indicates which the index of the element inside the buffer.
        uint2 index;

        // We also need to keep a size of the first buffer so we know when to
        // switch to the second buffer.
        uint firstBufferSize;

        // Implementation of IIterator.next()
        Iterator next()
        {
            Iterator result;
            result.index.x = index.x;
            result.index.y = index.y + 1;
            // If we are at the end of the first buffer,
            // move to the head of the second buffer
            if (result.index.x == 0 && result.index.y == firstBufferSize)
            {
                result.index = uint2(1, 0);
            }
            return result;
        }
    }

    StructuredBuffer<float> firstBuffer;
    StructuredBuffer<float> secondBuffer;
    uint getCount() { return getBufferSize(firstBuffer) + getBufferSize(secondBuffer); }

    Iterator begin()
    {
        Iterator iter;
        iter.index = uint2(0, 0);
        iter.firstBufferSize = getBufferSize(firstBuffer);
        return iter;
    }
    Iterator end()
    {
        Iterator iter;
        iter.index = uint2(1, getBufferSize(secondBuffer));
        iter.firstBufferSize = 0;
        return iter;
    }
    float getElementAt(Iterator iter)
    {
        if (ite.indexr.x == 0) return firstBuffer[iter.index.y];
        else return secondBuffer[iter.index.y];
    }
}
```

In summary, an `asssociatedtype` requirement in an interface is similar to other types of requirements: a method requirement means that an implementation must provide a method matching the interface signature, while an `associatedtype` requirement means that an implementation must provide a type in its scope with the matching name and interface constraint. In general, when defining an interface that is producing and consuming an object whose actual type is implementation-dependent, the type of this object can often be modeled as an associated type in the interface.

### Comparison to the C++ Approach
Readers who are familiar with C++ could easily relate the `Iterator` example in previous subsection to the implementation of STL. In C++, the `sum` function can be easily written with templates:
```C++
template<typename TContainer>
float sum(const TContainer& container)
{
    float result = 0.0f;
    // Assumes `TContainer` has a type `Iterator` that supports `operator++`.
    for (TContainer::Iterator iter = container.begin(); iter != container.end(); ++iter)
    {
        result += container.getElementAt(iter);
    }
    return result;
}
```

A C++ programmer can implement `ArrayFloatContainer` as following:
```C++
struct ArrayFloatContainer
{
    float content[10];

    typedef uint32_t Iterator;

    Iterator getCount() { return 10; }
    Iterator begin() { return 0; }
    Iterator end() { return 10; }
    float getElementAt(Iterator iter) { return content[iter]; }
};
```
Because C++ does not require a template function to define _constraints_ on the templated type, there are no interfaces or inheritances involved in the definition of `ArrayFloatContainer`. However `ArrayFloatContainer` still needs to define what its `Iterator` type is, so the `sum` function can be successfully specialized with an `ArrayFloatContainer`.

Note that the biggest difference between C++ templates and generics is that templates are not type-checked prior to specialization, and therefore the code that consumes a templated type (`TContainer` in this example) can simply assume `container` has a method named `getElementAt`, and the `TContainer` scope provides a type definition for `TContainer::Iterator`. Compiler error only arises when the programmer is attempting to specialize the `sum` function with a type that does not meet these assumptions. Contrarily, Slang requires all possible uses of a generic type be declared through an interface. By stating that `TContainer:IContainer` in the generics declaration, the Slang compiler can verify that `container.getElementAt` is calling a valid function. Similarily, the interface also tells the compiler that `TContainer.Iterator` is a valid type and enables the compiler to fully type check the `sum` function without specializing it first.

### Similarity to Swift and Rust

Slang's `associatedtype` shares the same semantic meaning with `associatedtype` in a Swift `protocol` or `type` in a Rust `trait`, except that Slang currently does not support the more general `where` clause in these languages. C# does not have an equivalent to `associatedtype`, and programmers need to resort to generic interfaces to achieve similar goals.

Generic Value Parameters
-------------------------------

So far we have demonstrated generics with _type parameters_. Additionally, Slang also supports generic _value_ parameters.
The following listing shows an example of generic value parameters.
```csharp
struct Array<T, let N : int>
{
    T arrayContent[N];
}
```
In this example, the `Array` type has a generic type parameter, `T`, that is used as the element type of the `arrayContent` array, and a generic value parameter `N` of integer type.

Note that the builtin `vector<float, N>` type also has an generic value parameter `N`.

> #### Note ####
> The only type of generic value parameters are `int`, `uint` and `bool`. `float` and
> other types cannot be used in a generic value parameter. Computations in a type
> expression are supported as long as they can be evaluated at compile time. For example,
`vector<float, 1+1>` is allowed and considered equivalent to `vector<float, 2>`.


Interface-typed Values
-------------------------------

So far we have been using interfaces as constraints to generic type parameters. For example, the following listing defines a generic function with a type parameter `TTransform` constrained by interface `ITransform`:

```csharp
interface ITransform
{
    int compute(MyObject obj);
}

// Defining a generic method:
int apply<TTransform : ITransform>(TTransform transform, MyObject object)
{
    return transform.compute(object);
}
```

While Slang's syntax for defining generic methods bears similarity to generics in C#/Java and templates in C++ and should be easy to users who are familiar with these languages, codebases that make heavy use of generics can quickly become verbose and difficult to read. To reduce the amount of boilerplate, Slang supports an alternate way to define the `apply` method by using the interface type `ITransform` as parameter type directly:

```csharp
// A method that is equivalent to `apply` but uses simpler syntax:
int apply_simple(ITransform transform, MyObject object)
{
    return transform.compute(object);
}
```

Instead of defining a generic type parameter `TTransform` and a method parameter `transform` that has `TTransform` type, you can simply define the same `apply` function like a normal method, with a `transform` parameter whose type is an interface. From the Slang compiler's view, `apply` and `apply_simple` will be compiled to the same target code.

In addition to parameters, Slang allows variables, and function return values to have an interface type as well:
```csharp
ITransform test(ITransform arg)
{
    ITransform v = arg;
    return v;
}
```

### Restrictions and Caveats

The Slang compiler always attempts to determine the actual type of an interface-typed value at compile time and specialize the code with the actual type. As long as the compiler can successfully determine the actual type, code that uses interface-typed values are equivalent to code written in the generics syntax. However, when interface types are used in function return values, the compiler will not be able to trivially propagate type information. For example:
```csharp
ITransform getTransform(int x)
{
    if (x == 0)
    {
        Type1Transform rs = {};
        return rs;
    }
    else
    {
        Type2Transform rs = {};
        return rs;
    }
}
```
In this example, the actual type of the return value is dependent on the value of `x`, which may not be known at compile time. This means that the concrete type of the return value at invocation sites of `getTransform` may not be statically determinable. When the Slang compiler cannot infer the concrete type of an interface-type value, it will generate code that performs a dynamic dispatch based on the concrete type of the value at runtime, which may introduce performance overhead. Note that this behavior applies to function return values in the form of `out` parameters as well:

```csharp
void getTransform(int x, out ITransform transform)
{
    if (x == 0)
    {
        Type1Transform rs = {};
        transform = rs;
    }
    else
    {
        Type2Transform rs = {};
        transform = rs;
    }
}
```
This `getTransform` definition can also result in dynamic dispatch code since the type of `transform` may not be statically determinable.

When the compiler is generating dynamic dispatch code for interface-typed values, it requires the concrete type of the interface-typed value to be free of any opaque-typed fields (e.g. resources and buffer types). A compiler error will generated upon such attempts:
```csharp
struct MyTransform : ITransform
{
    StructuredBuffer<int> buffer;
    int compute(MyObject obj)
    {
        return buffer[0];
    }
}

ITransform getTransform(int x)
{
    MyTransform rs;
    // Error: cannot use an opaque value as an interface-typed return value.
    return rs;
}
```

Assigning different values to a mutable interface-typed variable also undermines the compiler's ability to statically determine the type of the variable, and is not supported by the Slang compiler today:
```csharp
void test(int x)
{
    ITransform t = Type1Transform();
    // Do something ...
    // Assign a different type of transform to `t`:
    // (Not supported by Slang today)
    t = Type2Transform();
    // Do something else...
}
```

In general, if the use of interface-typed values is restricted to function parameters only, then the all code that involves interface-typed values will be compiled the same way as if the code is written using standard generics syntax.


Extending a Type with Additional Interface Conformances
-----------------------------
In the previous chapter, we introduced the `extension` feature that lets you define new members to an existing type in a separate location outside the original definition of the type. 

`extensions` can be used to make an existing type conform to additional interfaces. Suppose we have an interface `IFoo` and a type `MyObject` that implements the interface:

```csharp
interface IFoo
{
    int foo();
};

struct MyObject : IFoo
{
    int foo() { return 0; }
}
```

Now we introduce another interface, `IBar`:
```csharp
interface IBar
{
    float bar();
}
```

We can define an `extension` to make `MyObject` conform to `IBar` as well:
```csharp
extension MyObject : IBar
{
    float bar() { return 1.0f }
}
```

With this extension, we can use `MyObject` in places that expects an `IBar` as well:
```csharp
void use(IBar b)
{
    b.bar();
}

void test()
{
    MyObject obj;
    use(obj); // OK, `MyObject` is extended to conform to `IBar`.
}
```

You may define more than one interface conformances in a single `extension`:
```csharp
interface IBar2
{
    float bar2();
}
extension MyObject : IBar, IBar2
{
    float bar() { return 1.0f }
    float bar2() { return 2.0f }
}
```

`is` and `as` Operator
----------------------------

You can use `is` operator to test if an interface-typed value is of a specific concrete type, and use `as` operator to downcast the value into a specific type.
The `as` operator returns an `Optional<T>` that is not `none` if the downcast succeeds.

```csharp
interface IFoo
{
    int foo();
}
struct MyImpl : IFoo
{
    int foo() { return 0; }
}
void test(IFoo foo)
{
    bool t = foo is MyImpl; // true
    Optional<MyImpl> optV = foo as MyImpl;
    if (t == (optV != none))
        printf("success");
    else
        printf("fail");
}
void main()
{
    MyImpl v;
    test(v);
}
// Result:
// "success"
```

In addition to casting from an interface type to a concrete type, `as` and `is` operator can be used on generic types as well to cast a generic type into a concrete type. For example:
```csharp
T compute<T>(T a1, T a2)
{
    if (a1 is float)
    {
        return reinterpret<T>((a1 as float).value + (a2 as float).value);
    }
    else if (T is int)
    {
        return reinterpret<T>((a1 as int).value - (a2 as int).value);
    }
    return T();
}
// compute(1.0f, 2.0f) == 3.0f
// compute(3, 1) == 2
```

Extensions to Interfaces
-----------------------------

In addtion to extending ordinary types, you can define extensions on interfaces as well:
```csharp
// An example interface.
interface IFoo
{
    int foo();
}

// Extending `IFoo` with a new method requirement
// with a default implementation.
extension IFoo
{
    int bar() { return 0; }
}

int use(IFoo foo)
{
    // With the extension, all uses of `IFoo` typed values
    // can assume there is a `bar` method.
    return foo.bar();
}
```

Although the syntax of above listing suggests that we are extending an interface with additional requirements, this interpretation does not make logical sense in many ways. Consider a type `MyType` that exists before the extension is defined:
```csharp
struct MyType : IFoo
{
    int foo() { return 0; }
}
```

If we extend the `IFoo` with new requirements, the existing `MyType` definition would become invalid since `MyType` no longer provides implementations to all interface requirements. Instead, what an `extension` on an interface `IFoo` means is that for all types that conforms to the `IFoo` interface and does not have a `bar` method defined, add a `bar` method defined in this extension to that type so that all `IFoo` typed values have a `bar` method defined. If a type already defines a matching `bar` method, then the existing method will always override the default method provided in the extension:

```csharp
interface IFoo
{
    int foo();
}
struct MyFoo1 : IFoo
{
    int foo() { return 0; }
}
extension IFoo
{
    int bar() { return 0; }
}
struct MyFoo2 : IFoo
{
    int foo() { return 0; }
    int bar() { return 1; }
}
void test()
{
    MyFoo1 f1;
    MyFoo2 f2;
    int a = f1.bar(); // a == 0, calling the method in the extension.
    int b = f2.bar(); // b == 1, calling the existing method in `MyFoo2`.
}
```
This feature is similar to extension traits in Rust.
