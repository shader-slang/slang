Interfaces and Generics
===========================

This chapter covers two interrelated Slang language features: interfaces and generics. We will talk about what they are, how do they relate to similar features in other languages, how are they parsed and translated by the compiler, and how examples on how these features simplifies and modularizes shader code.

Interfaces
----------

Interfaces are used to define the methods and services a type should provide. You can define a interface as the following example:
```C#
interface IFoo
{
    int myMethod(float arg);
}
```

Slang's syntax for defining interfaces are similar to `interface`s in C# and `protocal`s in Swift. In this example, the `IFoo` interface establishes a contract that any type conforming to this interface must provide a method named `myMethod` that accepts a `float` argument and returns an `int` value.

A `struct` type may declare its conformance to an `interface` via the following syntax:
```C#
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
```C#
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

```C#
int myGenericMethod<T: IFoo>(T arg)
{
    return arg.myMethod(1.0);
}
```

The above listing defines a generic method named `myGenericMethod`, which accepts an argument that can be of any type `T` as long as `T` conforms to the `IFoo` interface. The `T` here is called a _generic type parameter_, and it is associated with an _type constraint_ that any type represented by `T` must conform to the interface `IFoo`.

The following listing shows how to invoke a generic method:
```
C#
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

While C++ templates is a powerful language mechanism, Slang has followed the path of many other modern programming langauges to adopt the more structural and restricted generics feature instead. This enables the Slang compiler to perform type checking early to give more readable error messages, and to speed-up compilation by reusing a lot of work for different instantiations of `myGenericMethod`.


Supported Constructs in Interface Definitions
-----------------------------------------------------

Slang supports many other constructs in addition to ordinary methods as a part of an interface definition.

### Properties

```C#
interface IFoo
{
    property int count {get; set;}
}
```
The above listing declares that any conforming type must define a property named `count` with both a `getter` and a `setter` method.

### Generic Methods

```C#
interface IFoo
{
    int compute<T:IBar>(T val);
}
```
The above listing declares that any conforming type must define a generic method named `compute` that has one generic type parameter conforming to the `IBar` interface.

### Static Methods

```C#
interface IFoo
{
    static int compute(int val);
};
```

The above listing declares that any conforming type must define a static method named `compute`. This allows the following generic method to pass type-checking:
```C#
void f<T:IFoo>()
{
    T.compute(5); // OK, T has a static method `compute`.
}
```

### `This` Type

You may use a special keyword `This` in interface definitions to refer to the type that is conforming to the interface. The following examples demonstrate a use of `This` type:
```C#
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
```C#
void f<T:IFoo>()
{
    T obj = /*a newly initialized T*/
}
```
One way to implement this is to introduce a static method requirement in `IFoo`:
```C#
interface IFoo
{
    static This create();
}
```
With this interface definition, we can define `f` as following:
```C#
void f<T:IFoo>()
{
    T obj = T.create();
}
```

This solution works just fine, but it would be nicer if you can just write:
```C#
T obj = T();
```
Or simply
```C#
T obj;
```
And let the compiler invoke the default initializer defined in the type.
To enable this, you can include an initializer requirement in the interface definition:
```C#
interface IFoo
{
    __init();
}
```

Initializers with parameters are supported as well. For example:
```C#
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
```C#
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
```C#
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
```C#
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
```C#
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
```C#
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
```C#
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
```C#
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
==========================

So far we have demonstrated generics with _type parameters_. Additionally, Slang also supports generic _value_ parameters.
The following listing shows an example of generic value parameters.
```C#
struct Array<T, let N : int>
{
    T arrayContent[N];
}
```
In this example, the `Array` type has a generic type parameter, `T`, that is used as the element type of the `arrayContent` array, and a generic value parameter `N` of integer type.

Note that the builtin `vector<float, N>` type also has an generic value parameter `N`.

> #### Note ####
> The only type of generic value parameters are integer values. `bool`, `float` and
> other types cannot be used in a generic value parameter. Computations in a type
> expression are not supported, for example, `vector<float, 1+1>` is not allowed.

