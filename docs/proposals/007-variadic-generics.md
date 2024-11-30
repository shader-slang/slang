SP #007: Variadic Generics
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

Status: Implemented.

Author: Yong He.

Implementation:
    [PR 4833](https://github.com/shader-slang/slang/pull/4833),
    [PR 4849](https://github.com/shader-slang/slang/pull/4849),
    [PR 4850](https://github.com/shader-slang/slang/pull/4850),
    [PR 4856](https://github.com/shader-slang/slang/pull/4856)

Reviewed by: Kai Zhang, Jay Kwak, Ariel Glasroth.

Background
----------

We have several cases that will benefit from variadic generics. One simplest example is the `printf` function is currently
defined to have different overloads for each number of arguments. The downside of duplicating overloads is the bloating the core
module size and a predefined upper limit of argument count. If users are to build their own functions that wraps the `printf`
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

For now, we require that all variadic generic type packs to appear in the end of a parameter list, after any ordinary parameters. This means that the following
definitions are invalid:

```
void f<each T, U>() {} // Error, ordinary parameter `U` after type pack.
void g<each T, U = int>() {} // Error, ordinary parameter after type pack.
void k<each T, let i : int>() {} // Error.
void h<U = int, each T>() {} // OK.
```

Additionally, we establish these restrictions on how `expand` and `each` maybe used:
- The pattern type of an `expand` type expression must capture at least one generic type pack parameter in an `each` expression.
- The type expression after `each` must refer to a generic type pack parameter, and the `each` expression can only appear inside an `expand` expression.

These rules means that expressions like `expand int`, or `each T` on its own are invalid expressions.

Similarly, when using `expand` and `each` on values, we require that:
- The pattern expression of an `expand` expression must capture at least one value whose type is a generic type pack parameter.
- The expression after `each` must refer to a value whose type is a generic type pack parameter, and the `each` expression can only appear inside an `expand` expression.

Combined with type euqality constraints, variadic generic type pack can be used to define homogeneously typed parameter pack:
```
void calcInts<each T>(expand each T values) where T == int
{
    ...
}
```

Detailed Explanation
--------------------

To implement variadic generics, we need to introduce several semantic constructs in our type system.
### `GenericTypePackParameterDecl`
When a generic parameter is defined with the `each` keyword, such as in `void f<each T>`, the parser should create a new type of AST node inside the generic, and we name this
AST node a `GenericTypePackParameterDecl`. With this additional, a generic parameter can be `GenericTypeParameterDecl`, `GenericValueParameterDecl`, `GenericTypeConstraintDecl`
and `GenericTypePackParameterDecl`. When the user defines type constraints on a generic type pack parameter, we will form a `GenericTypeConstraintDecl` whose `subType` is a
`DeclRefType` referencing the `GenericTypePackParameterDecl`.

### Type Pack
A type pack represents a pack of types. The simplest form of a type pack is a `ConcreteTypePack` that is a list of concerete type packs, such as `(int, float, float3)`.
In a generic decl such as `void f<each T>(T v)`, `T` refers to an abstract type pack represented by the generic type pack parameter `T`. The type of parameter `v` in this case
is a `DeclRefType(GenericTypePackParameterDecl "T")`.
The most general case of a type pack is defined by the `expand PatternExpr` type expression. In this case, the expression will be translated into a `ExpandType`, representing a
abstract type pack that can be evaluated by substituting all `each X` expressions in the `PatternExpr` with a corresponding element in `X`, and joining all the resulting element types
into a type pack.

Note that a `ConcreteTypePack` is very similar in semantic meaning to a `Tuple`, with the exception that `ConcreteTypePack` also bears the automatic flattening semantic, such that
`ConcreteTypePack(ConcreteTypePack(a,b), c)` is equivalent and can be simplified to `ConcreteTypePack(a,b,c)`.

In summary, a type pack can be represented by one of:
- `ConcreteTypePack`, a simple concrete list of element types.
- `DeclRefType(GenericTypePackParameterDecl)`, a simple reference to a generic type pack parameter.
- `ExpandType(PatternType)`, an abstract type pack resulting from expanding and evaluating `PatternType`.

### `ExpandType` and `EachType`
The type expression `expand each T` should be translated into `ExpandType(EachType(T), T)`. Here the first argument in `ExpandType` is the `PatternType`, which is what we will
use to expand into a concrete type pack. The second argument `T` represents all the generic type pack parameters that is being captured by `PatternType`. The reason to explicitly
keep track of captured generic type pack parameters is to make it easy to determine the size of the type pack without having to look into `PatternType`, and to ensure we never lose
the size info even when the pattern type itself is substituted into something that is independent of any generic type pack parameters.

For example, consider the substitution process on following case:

```
typealias F<T> = int; // result of F<T> is not dependent on T.
typealias MyPack<each T : IFoo> = expand F<each T>;
typealias Pack3 = MyPack<float, double, void>;
```

We can know from this definition that `Pack3` should evaluate to `(int, int, int)`. But let's see step-by-step how this is done in the type system.

First, `Pack3` is evaluated to `MyPack<ConcreteTypePack(float, double, void)>`. To further resolve this, we will plugin the argument `ConcreteTypePack(float, double, void)` into the
definition of `MyPack`. The definition `expand F<each T>` is represented as:
```
ExpandType(
    pattern = DeclRefType(
        GenericAppDeclRef(F,
            args = [EachType(DeclRefType(GenericTypePackParamDecl "T"))])
    ),
    capture = DeclRefType(GenericTypePackParamDecl "T")
)
```

But this type is simplifiable because `F` refers to a type alias whose definition is:

```
DeclRefType(StructDecl "int")
```

So the `expand F<eachT>` type can be further simplified down to:

```
ExpandType(
    pattern = DeclRefType(StructDecl "int"),
    capture = DeclRefType(GenericTypePackParamDecl "T")
)
```

Note that in this definition, the pattern type no longer contains any references to any `GenericTypePackParamDecl` so there is no way for us
to know how many elements the `ExpandType` should expand into just from the pattern type itself. Fortunately, we still kept a reference to
the generic type param decl through the `capture` argument in the `ExpandType`. This will allow us to evaluate it into `(int, int, int)` when
we apply substitution `T=ConcreteTypePack(float, double, void)` to it.

Let's take a look at another more contrived example to understand the substitution process. Assume we have:

```
interface IFoo
{
    associatedtype Assoc;
};
struct Foo : IFoo
{
    typealias Assoc = int;
};
struct Foo2 : IFoo
{
    typealias Assoc = float;
};
typealias MyPack<each T : IFoo> = expand (each T).Assoc;
typealias Pack2 = MyPack<Foo, Foo2>;
```

When evaluating `Pack2`, we will first form a `ConcreteTypePack(Foo, Foo2)` and use it to substitute the `T` parameter in `MyPack`. This will result in `MyPack<ConcreteTypePack(Foo, Foo2)>`.
Then we continue to resolve this type alias by substituting `expand (each T).Assoc` with `T = ConcreteTypePack(Foo, Foo2)`. 

The expression `expand (each T).Assoc` is translated into

```
ExpandType(
    pattern =
        DeclRefType(
            LookupDeclRef(
                EachType(DeclRefType(GenericTypePackParamDecl "T")),
                IFoo::assoc
            )
        ),
    capture = DeclRefType(GenericTypePackParamDecl "T")
)
```

Substituting this with `DeclRefType(T) = ConcreteTypePack(Foo, Foo2)` we will get:

```
ExpandType(
    pattern =
        DeclRefType(
            LookupDeclRef(
                EachType(ConcreteTypePack(Foo, Foo2)),
                IFoo::assoc
            )
        ),
    capture = ConcreteTypePack(Foo, Foo2)
)
```

Since the captured type pack in the `ExpandType` is already a concrete type pack, we should be able to turn this `ExpandType` into a
`ConcreteTypePack`, by substituting `pattern` twice, with `EachType(...)` replaced with a corresponding element in the input `ConcreteTypePack` to form:
```
ConcreteTypePack(
    DeclRefType(
            LookupDeclRef(
                Foo,
                IFoo::assoc
            )
        ),
    DeclRefType(
            LookupDeclRef(
                Foo2,
                IFoo::assoc
            )
        )
)
```

And by resolving the `LookupDeclRef`, we will get:
```
ConcreteTypePack(
    DeclRefType(StructDecl "int"),
    DeclRefType(StructDecl "float")
)
```

Which is the correct representation for type pack `(int, float)`.

#### Simplification Rules of `Expand` and `Each` Types

By the definition of `expand` and `each`, we have these simplification rules:
- `expand each T` => `T`
- `each expand T` => `T`


### Type Constraints for Subtype Relationships

We define the sub-type relationship for type packs so that: given type pack `TPack`, we say
`TPack` is a subtype of `IFoo` (noted as `TPack:IFoo`) if every type in `TPack` is a subtype of `IFoo`.

In a generic definition `__generic<each T : IFoo>`, we will say the type pack `T` is a subtype of
`IFoo`. In the generic definition, we will have a `GenericTypeConstraintDecl` where
`subType = DeclRefType(GenericTypePackParamDecl "T")` and `supType = IFoo`. The fact that `T:IFoo` is
represented by a `DeclaredSubtypeWitness` whose `declRef` will point to this
`GenericTypeConstraintDecl`.

The subtype witness for a `ConcreteTypePack(T0, T1, ... Tn) : IBase` is represented by
`TypePackSubtypeWitness(SubtypeWitness(T0:IBase), SubtypeWitness(T1:IBase), ..., SubtypeWitness(Tn:IBase))`.

If a type pack `T` is a subtype of `IBase`, then `each T` is also a subtype of `IBase`. 
The subtype witness for a `EachType(typePack) : IBase` is represented by
`EachTypeWitness(SubtypeWitness(typePack : IBase))`.

If a pattern type `P` is a subtype of `IBase`, then `expand P` is also a subtype of `IBase`.
The subtype witness for a `ExpandType(patternType, capture) : IBase` is represented by
`ExpandSubtypeWitness(SubtypeWitness(pattern : IBase))`.

Similar to `ExpandType` and `EachType`, we will have simplification rules such that:

- `ExpandSubtypeWitness(EachSubtypeWitness(x))` => `x`
- `EachSubtypeWitness(ExpandSubtypeWitness(x))` => `x`.

#### Canonical Representation of `TransitiveSubtypeWitness` for Type Packs

Given:
```
interface IBase
{
}

interface IDerived : IBase
{    
}

__generic<each T : IDerived> ...
```

The witness of `DeclRefType("T")` conforming to `IDerived` will be represented by
```
DeclaredSubtypeWitness(
    sub = DeclRefType(GenericTypePackParamDecl "T")
    sup = `IBase`.
)
```

To represent the witness of `DeclRefType("T")` conforming to `IBase`, we will need to make use
of the `TransitiveSubtypeWitness`. For simplicity of IR generation, we would like to have `TransitiveSubtypeWitness`
not to deal with the case that `sub` is a type pack.

Therefore, instead of representing `DeclRefType("T") : IBase` as something like:

```
TransitiveSubtypeWitness(
    subIsMid = DeclaredSubtypeWitness(
        sub = DeclRefType(GenericTypePackParamDecl "T")
        sup = `IBase`),
    midIsSup = DeclaredSubtypeWitness(DeclRef(Iderived:IBase))
)
```

In the above definition, the `subType` of the witness is a type pack, which isn't very convenient to work with.
Instead, we will represent the same witness as:

```
ExpandSubtypeWitness(
    TransitiveSubtypeWitness(
        subIsMid = EachWitness(DeclaredSubtypeWitness(
            sub = DeclRefType(GenericTypePackParamDecl "T")
            sup = `IBase`)),
        midIsSup = DeclaredSubtypeWitness(DeclRef(Iderived:IBase))
    )
)
```

Note that in this second representation is effectively representing `expand ((each T) : IBase)`, where the `subType` of the `TransitiveSubtypeWitness`
is now a `EachType` and no longer a type pack. Doing this transformation will allow us to avoid the situation where we transitive witness lookup
is done on a pack of witnesses, and therefore simplifying the IR.

### Matching Arguments to Packs

When resolving overload to form a `DeclRef` to a generic decl or resolving overload in a function call, we need to match arguments to generic/function
parameters. Before introducing variadic type packs, this matching is trivial: an argument at index `i` will match to a parameter at index `i`.

With type packs, we need to generalize this logic. Because we have required that all type pack parameters to appear at the end of the generic or function parameter
list, we can still match argument 1:1 to parameters for all the non type pack parameters first. Once we have matched arguments to non type pack parameters and there
are additional arguments remaining, they must be for type pack parameters. If an argument is itself a concrete or abstract type pack, then we can continue to match
that argument 1:1 to the parameter. If not, then we require all the remaining arguments are individual types and not type packs. Because we require all type pack
parameters to have equal size, we can divide the remaining arguments evenly by the number of type pack parameters, and form a `TypePack`/`ValuePack` from that number
of arguments and supply it to each type pack parameter.

For example, assume we have:
```
struct S<T, each U, each V>
```

When resolving the overload for `S<int, int, void, float, bool>`, we have three parameters: `T`, `U`, `V` and five arguments: `int`, `int`, `void`, `float`, `bool`.
We will first perform argument match and match `T=int`. Now we have four arguments remaining and two type pack parameters. We can then divide 4 by 2 to get the
number of elements for each type pack argument, and form a `TypePack(int, void)` and use it as the matched argument for `U`, and form a `TypePack(float, bool)`
and use it as the matched argument for `V`.

After matching and the remaining overload resolution logic, `S<int, int, void, float, bool>` will be represented as:
```
GenericAppDeclRef
    genericDecl = "S"
    args = [
        DeclRefType("int"), // For `T`
        TypePack(DeclRefType("int"), DeclRefType("void")), // For `U`
        TypePack(DeclRefType("float"), DeclRefType("bool")) // For `V`
    ]
```

Similarly, when resolving a function call with variadic parameters, we will perform argument matching and create `PackExpr` to use as argument to a packed parameter. Given:
```
void f<each T, each U>(int x, expand each T t, expand each U u) {...}
```

A call in the form of `f(3, Foo(), Bar(), 1.0f, false)` will be converted to:

```
f(3, Pack(Foo(), Bar()), Pack(1.0f, false))
```

After resolving the call. The `Pack(...)` represents the `PackExpr` synthesized by the compiler to create a `ValuePack` whose type is a `TypePack`, so
it can be used as argument to a `TypePack` parameter.

### IR Representation

#### Expressing Types

A concrete type pack is represented as `IRTypePack(T0, T1, ..., Tn)` in the IR, and an abstract type pack such as an `expand` type will eventually be specialized into an `IRTypePack`. This means that a function parameter whose type is a type pack is translated into a single parameter of `IRTypePack` type. Again, `IRTypePack` is in many ways similar to `IRTupleType`, except that `IRTypePack` are automatically flattened into enclosing type packs during specialization.

We will represent `expand` and `each` types in the IR almost 1:1 as they are represented in the AST. Note that types are hoistable insts in Slang IR and is globally deduplicated based on their operands, representing it in the natural way will allow these types to take advantage from Slang IR's global deduplication service.

This means that `each T` is represented as `IREachType(T)`, and `expand patternType` is represented as `IRExpandType(PatternType, capturedTypePacks)`
in the IR.

For example, the type `expand vector<each T, each U>`, where `T` and `U` are generic type pack parameters, is represented in the IR as:
```
%T = IRParam : IRTypePackParameterKind;
%U = IRParam : IRTypePackParameterKind;

%et = IREach %T;
%eu = IREach %U;

%v = IRVectorType(%et, %eu)
%expandType = IRExpandType(%v, %T, %U) // v is pattern; T,U are captured type packs.
```

Note that this kind of type hierarchy representation is only used during IR lowering in order to benefit from IR global deduplication of type definitions. The representation in this form isn't convenient for specialization.
Once lowered to IR step is complete, we will convert all type representation to the same form as value represenataion described in the following section.

#### Expressing Values

A value whose type is a type pack is called a value pack. A value pack is represented in the IR as a `IRMakeValuePack` inst.
For example, the value pack `(1,2,3)` will be represented in the IR as:
```
IRMakeValuePack(1,2,3) : IRTypePack(int, int, int)
```

An `expand(PatternExpr)` expression should be represented in the IR as:
```
%e = IRExpand : IRExpandType(...)
{
    IRBlock
    {
        %index = IRParam : int;
        yield PatternExpr; // may use `index` here.
    }
}
```
The `IRExpand` is treated like an compile-time for loop where the loop body is expressed as basic blocks as the children of the `IRExpand` inst.
The body starts with a `%index` parameter that represents the loop index within the value pack, and the CFG inside `IRExpand` should end with a single
`yield` that is a terminal instruction "returning" the mapped value for element at `%index` in the input value pack.

For example, given `v` as value pack whose type is a type pack, `let x = expand (each v) + 1` will be represented in the IR as:

```
%v = /*some value pack whose type is a TypePack*/
%x = IRExpand : IRTypePack(...)
{
    IRBlock
    {
        %index = IRParam : int;
        %e = IRGetTupleElement(%v, %index);
        %r = IRAdd %e 1;
        IRYield %r;
    }
}
```

In this simple example, the `IRExpand` contains only one basic block. It is possible for `IRExpand` to have more than one basic blocks if the pattern expression
contains a `?:` operator, in which case there will be a branching CFG structure inside the `IRExpand`.

Also note that `each v` is translated into `IRGetTupleElement(%v, %index)` that extacts the element at `%index` from the tuple value represented by `%v`.

#### IR Specialization

Specializing the IR for an `IRExpand` inst with a concrete value pack is very similar to loop unrolling. Given the example in the previous section
on expression `expand (each v) + 1`, we can specialize the `IRExpand` inst with `v` being an known value pack such as `IRMakeTuple(1,2,3)` in two steps.

Step 1 is to copy the children of the `IRExpand` inst three times into where the `IRExpand` inst itself is located, and during each copy, we replace
all references to `IRParam` with the concrete index for the copy. Therefore, specializing the above IR code with `IRMakeTuple(1,2,3)` will lead to:

```
%block0 = IRBlock
{
    %e0 = IRGetTupleElement(%v, 0);
    %r0 = IRAdd %e0 1;
    yield %r0;
}
%block1 = IRBlock
{
    %e1 = IRGetTupleElement(%v, 1);
    %r1 = IRAdd %e1 1;
    yield %r1;
}
%block2 = IRBlock
{
    %e2 = IRGetTupleElement(%v, 2);
    %r2 = IRAdd %e2 1;
    yield %r2;
}
%mergeBlock = IRBlock
{
    ...
}
```

Step 2 is to hookup each copied blocks by replacing all the `yield` instructions with `branch` instructions, and form the final result of the value pack
by packing up all the values computed at each "loop iteration" in an `IRMakeValuePack` inst:

```
%block0 = IRBlock
{
    %e0 = IRGetTupleElement(%v, 0);
    %r0 = IRAdd %e0 1;
    branch %block1;
}
%block1 = IRBlock
{
    %e1 = IRGetTupleElement(%v, 1);
    %r1 = IRAdd %e1 1;
    branch %block2;
}
%block2 = IRBlock
{
    %e2 = IRGetTupleElement(%v, 2);
    %r2 = IRAdd %e2 1;
    branch %mergeBlock;
}
%mergeBlock = IRBlock
{
    %expand = IRMakeValuePack(%r0, %r1, %r2);
}
```

With this, we can replace the original `IRExpand` inst with `%expand` and specialization is done. The specialized instructions like `IRGetTupleElement(%v, 0)` will be picked up
in the follow-up step during specialization and replaced with the actual value at the specified index since `%v` is a known value pack represented by `IRMakeValuePack`. So after
folding and other simplifications, we should result in
```
%expand = IRMakeValuePack(2,3,4)
```
When specializing the original expression with `IRMakeValuePack(1,2,3)` in the IR.

Specialization of types and witness follows the same idea of value specialization, but since types and witnesses are represented directly as ordinary insts and operands instead of the
nested children of an `IRExpand`, we will use a recursive process on the type structure to perform the specialization. Most of the recursion logic should be trivial, and the only
interesting case is when specializing `IRExpandType` and `IREachType`. During the recursion process, we should maintain a state called `indexInPack` to represent the current expansion
index when specializing the pattern type of an `IRExpandType`, and then when we get to specialize an `IREachType(TPack)`, we should know which index in the pack we are currently
expanding by looking at the `indexInPack` context variable, and replace `IREachType(TypePack(T0, T1, ... Tn))` with the `T` at `indexInPack`.

After the specialization pass, there should be no more `IRExpand` and `IRExpandType` instructions in the IR. And we can lower t he remaining `IRTypePack` the same way as `IRTupleType`s.


Alternatives Considered
-----------------------

We considered the C++ `...` operator syntax and Swift's `repeat each` syntax and ended up picking Swift's design because it is easier to parse and is less ambiguous. Swift is strict about requiring `each` to precede a generic type pack parameter so `void f<each T>(T v)` is not a valid syntax to prevent confusion on what `T` is in this context. In Slang we don't require this because `expand each T` is always simplified down to `T`, and refer to the type pack.

We also considered not adding variadic generics support to the language at all, and just implement `Tuple` and `IFunc` as special system builtin types, like how it is done in C#. However we
believe that this approach is too limited when it comes to what the user can do with tuples and `IFunc`. Given Slang's position as a high performance GPU-first language, it is more important for Slang than other CPU languages to have a powerful type system that can provide zero-cost abstraction for meta-programming tasks. That lead us to believe that the language and the users can benefit from proper support of variadic generics.
