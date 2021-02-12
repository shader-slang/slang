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

Slang's syntax for defining interfaces are similar to C# and Swift. In this example, the `IFoo` interface establishes a contract that any type conforming to this interface must provide a method named `myMethod` that accepts a `float` argument and returns an `int` value.

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
