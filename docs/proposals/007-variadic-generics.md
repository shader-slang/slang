Variadic Generics
=================

Variadic generics is the ability to define and use generic types and functions that has arbitrary number of generic type parameters.
For example, a tuple type can be represented as a generic type that has zero or any number of type parameters, i.e. a variadic generic.
Variadic types and functions are key building blocks to allow tuple types in the language, and will also enable us to define a
`IFunc<TResult, TParam...>` interface that represents a callable value. `IFunc` interface can allow users to start writing code that
takes "callback" functions as parameters and start to functors or adopting more functional programming idioms.

Supporting variadic generics is a big step up in Slang type system's expressive power, and will allow more meta programming logic to be
written in native Slang code rather than on top of it with macros or custom code generation tools.

Status
------

Status: prototype branch available.
Author: Yong He
Implemented by: Yong He
Reviewed by: N/A

Background
----------

We have several cases that will benefit from variadic generics. One simplest example is the `printf` function is currently
defined to have different overloads for each number of arguments. The downside of duplicating overloads is the bloating standard
library size and a predefined upper limit of argument count. If users are to build their own functions that wraps the `printf`
function, they will have to define a set of overloads for each number of arguments too, further bloating code size.

Some of our users would like to implement the functor idiom in their shader code with interfaces. This is almost possible
with existing support of generics and interfaces. For example:
```
// Define an interface for the callback function
interface IProcessor
{
    void process(int data);
}

// The callback function `p` is represented as a functor conforming to the `IProcessor` interface.
void process<TProcessor:IProcessor>(TProcessor p, int data[N])
{
    for (int i = 0; i < N; i++)
        p.process(data[i]);
}

// Define the functor as a type that conforms to `IProcessor`.
struct MyProcessorFunc : IProcessor
{
    void process(int data) { ... }
}

void user(int myData[N])
{
    // Define an instance of the functor, and pass it to `process`.
    MyProcessorFunc functor = ...;
    process(functor, myData);
}
```

While this can work, it requires a lot of boilterplate from the user. For each shape of callback, the user must define
a separate interface. We can reduce this boilterplate if the system has builtin support for `IFunc`:

```
// The callback function `p` is represented as a functor conforming to the `IProcessor` interface.
void process<TProcessor:IFunc<void, int>>(TProcessor p, int data[N])
{
    for (int i = 0; i < N; i++)
        p.process(data[i]);
}

// Define the functor as a type that conforms to `IProcessor`.
struct MyProcessorFunc : IFunc<void, int>
{
    void process(int data) { ... }
}

void user(int myData[N])
{
    // Define an instance of the functor, and pass it to `process`.
    MyProcessorFunc functor = ...;
    process(functor, myData);
}
```

The above code eliminates the user defined interface by using the builtin `IFunc` interface. By making `IFunc` builtin,
we can open the path for the compiler to synthesize conformances to `IFunc` for ordinary functions and in the future
add support for lambda expressions that automatically conform to `IFunc`, further simplify the user code into something like:
```
// The callback function `p` is represented as a functor conforming to the `IProcessor` interface.
void process<TProcessor:IFunc<void, int>>(TProcessor p, int data[N])
{
    for (int i = 0; i < N; i++)
        p.process(data[i]);
}

void user(int myData[N])
{
    process((int x)=>{...}, myData);
}
```

Related Work
------------

Variadic generics is an advance type system feature that is missinng in many modern languages including C# and Rust.
Swift adds support for variadic generics recently in late 2022/2023, and this proposal largely follows Swift's design.
C++ has variadic templates that achieves similar results within the template system.
Rust supports variadics in a macro system layered above its core type system. While this can solve many user issues,
we decided to not go through this path because macros and templates must be expanded before core type checking, which means that
they can't integrate nicely in modules and compiled into IR independently of their use sites.


Proposed Approach
-----------------

Slang can follow Swift's solution for variadic generics. A user can define a variadic generic with the syntax:

```
void myFunc<each T>(expand each T v) {...}
```

The code above defines a generic function that has a __generic type pack parameter__ `T` with the `each` keyword before `T`.
The function's parameter list is defined as `expand each T v`, which should be interpreted as a parameter `v` whose type is
`expand each T`. `expand each T` is a type that represents a pack of types. A parameter whose type is a pack of types can
accept zero or more arguments during function call resolution.

`myFunc` can be called with arbitrary number of arguments:
```
myFunc(); // OK, zero arguments
myFunc(1, 2.0f, 3.0h); // OK, three arguments with different types.
```

A function can forward its variadic parameter to another function that accepts variadic parameter with the `expand` expression:
```
void caller<each T>(expand each T v)
{
    myFunc(expand each v);
}
```

Generic type pack parameters can be nested, and there can be more than one variadic generic parameters in a single generic decl:
```
struct Parent<each T>
{
    void f<each U>(...) {...} // OK, nested generics with type pack parameters
}
void g<each T, each U>(...) { ... } // OK, more than one type pack parameter in a single generic.
```

However, when more than one generic type pack parameters is referenced in a single `expand` expression, there is an implicit
requirement that these type packs will have the same number of elements. For example:
```
 // implicitly requiring T and U to have same number of elements.
void g<each T, each U>(expand Pair<each T, each U> pairs) {...}

void user()
{
    // We will match (int, float) to `T`, and (uint16_t, half) to `U`:
    g<int, float, uint16_t, half>(
        Pair<int, uint16_t>(1, 2),
        Pair<float, half>(1.0f, 2.0h) );
}
```

In the example above, the type `expand Pair<each T, each U>` defines a pack of types where each element in the pack is formed by
replacing `each T` and `each U` in the __pattern type__ `Pair<each T, each U>` with the corresponding elements in type pack `T` and `U`.
Because the pattern type `Pair<each T, each U>` references two different type pack parameters `T` and `U`, we require that `T` and `U`
has the same number of types, this allows us to resolve `g<int, float, uint16_t, half>` by evenly dividing the the argument list
into two parts, such that `T = (int, float)` and `U = (uint16_t, half)`. With that, `expand Pair<each T, each U>` is then substituted
into a type pack `(Pair<int, uint16_t>, Pair<float, half>)`.

Generic type pack parameters can have type constraints:

```
void f<each T : IFloat>(expand each T v) {}
```

This means that every type in the type pack `T` must conform to the interface `IFloat`.
You can use any expression inside `expand` when it is used on values:
```
interface IGetValue
{
    int getValue();
}

void print(each T)(expand each T) {...}

void f<each T : IFloat>(expand each T v)
{
    print(expand (each v).getValue());
}
```
Here, `expand (each v).getValue()` will expand the pattern expression `(each v).getValue()` into a pack of values. The result of this `expand` expression
is a pack of values where each element of the pack is computed by substituting `each v` in the pattern expression with each element in `v`. The resulting
pack of `int` values is then passed to `print` function that also takes a pack of values.

Detailed Explanation
--------------------

Here's where you go into the messy details related to language semantics, implementation, corner cases and gotchas, etc.
Ideally this section provides enough detail that a contributor who wasn't involved in the proposal process could implement the feature in a way that is faithful to the original.

Alternatives Considered
-----------------------

Any important alternative designs should be listed here.
If somebody comes along and says "that proposal is neat, but you should just do X" you want to be able to show that X was considered, and give enough context on why we made the decision we did.
This section doesn't need to be defensive, or focus on which of various options is "best".
Ideally we can acknowledge that different designs are suited for different circumstances/constraints.
