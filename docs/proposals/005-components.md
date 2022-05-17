Components
==========

We propose to extend Slang with a construct for defining coarse-grained *components* that can be used to assemble shader programs.

Status
------

Under discussion.

Background
----------

First, a bit of terminology. In the context of a specific language like Slang, a term like "component" or "module" will often have a narrow meaning, but when we want to discuss "modularity" broadly we need to have a way to refer to the *things* we want to have be modular: the units of modularity.
In this document we will use the term *unit* to refer abstractly to anything that is a unit of modularity for some context/purpose/system, and try to reserve other terms for cases where we mean something more specific.

While Slang has many features that address modularity for "small" units, it is still lacking in constructs that adequately address the needs of "large" units.
These are qualitative distinctions, but some examples may clarify the kind of distinction we mean.
An interface `ILight` for light sources, and a `struct` type `OmniLight` that conforms to it are small units.
An entire `LightSampling` module in a path tracer is a much larger unit.

The main tools that Slang provides for "small" units are: `interface`s, `struct`s, and `ParameterBlock`s.
Interfaces allow a developer to codify the types and operations that clients of a unit may rely on, as well as the requirements that implementations must provide.
Structure types are the main way developers can implement an interface, and it is important for GPU efficiency that Slang `struct`s are *value types*.
By using `ParameterBlock`s, developers can connect the units of modularity used *within* shader code with the parameters passed from *outside* their shaders.

When we talk about "large" units of modularity, the main thing Slang provides are, well, modules.
A Slang module is basically just a collection of global-scope declarations: types, functions, shader parameters, and entry points.
The `import` construct allows modules to express a dependency on one another, and if/when we add `public`/`internal` visibility qualifiers it will also be able to restrict clients of a module to its defined interface.

What modules *don't* provide is any of the flexibility that `interface`s provide for "small" units like `struct` types.
There is no first-class way for a Slang programmer to define a common interface that multiple modules implement, and then to express another piece of code that can work with any of those implementations.
Aside from falling back to preprocessor hackery (which negates many of the benefits of Slang in terms of separate compilation), the only way for developers to try to recoup those benefits is to use the tools for "small" units.

Let's consider a placeholder/pseudocode set of Slang modules that work together:

```
// Lighting.slang

interface ILight { ... }

StructuredBuffer<ILight> gLights;

void doLightingStuff() { ... }
```

```
// Materials.slang
import Lighting;

interface IMaterial { ... }
StructuredBuffer<IMaterial> gMaterials;

void doMaterialStuff()
{
    doLightingStuff();
}
```

```
// Integrator.slang
import Lighting;
import Materials;

ParameterBlock<IntegratorParams> gIntegratorParams;

void doIntegeratorStuff()
{
    doLightingStuff();
    doMaterialsStuff();
}
```

The details of the module implementations is not the important part here.
The key is that each module defines a collection of types, operations, and shader parameters, and there is a dependency relationship between the modules.
Note that the `Integrator` module depends on both `Lighting` and `Materials`, but it does *not* need to be actively concerned with the fact that `Materials` *also* depends on `Lighting`.

If we look only at a leaf module like `Lighting` it seems simple enough to translate it into something based on our "small" modularity features:

```
// Lighting.slang

interface ILight { ... }

interface ILightingSystem
{
    void doLightingStuff();
}

struct DefaultLightingSystem : ILightingSystem
{
    StructuredBuffer<ILight> lights;

    void doLightingStuff() { ... }
}
```

Here we were able to move most of the global-scope code in `Lighting.slang` into a `struct` type called `DefaultLightingSystem`.
We were also able to define an explicit `interface` for the system, which makes explicit that we don't consider `gLights` part of the public interface of the system (only `doLighting()`).
By defining the interface, we also create the possibility of plugging in other implementations of `ILightingSystem` - for example, we can imagine a `NullLightingSystem` that actually doesn't perform any lighting (perhaps useful for performane analysis).

Translating `Materials` in the same way that we did for `Lighting` leads to some immediate questions:

```
// Materials.slang
import Lighting;

interface IMaterial { ... }

interface IMaterialSystem
{
    void doMaterialStuff();
}

struct DefaultMaterialSystem
{
    StructuredBuffer<IMaterial> materials;

    void doMaterialStuff()
    {
        /* ???WHAT GOES HERE??? */.doLightingStuff();
    }
}
```

When our `DefaultMaterialSystem` wants to invoke code for lighting, it needs a way to refer to the lighting system.
Beyond that, we want it to be able to work with *any* implementation of `ILightingSystem`.

A naive first attempt might be to give `DefaultMaterialSystem` field that refers to an `ILightingSystem`:

```
struct DefaultMaterialSystem
{
    ILightingSystem lighting;
    ...
    void doMaterialStuff()
    {
        lighting.doLightingStuff();
    }
}
```

An approach light that (or even one using a `ParameterBlock<ILightingSystem>`) runs into the problem that it is going to force the Slang compiler to use its layout strategy for dynamic dispatch, which cannot handle the resource types in `DefaultLightingSystem` when compiling for most current GPU targets.
There are other problems with directly aggregating an `ILightingSystem` into our material system, but those will need to wait for a bit.

If we want to allow the code in our `DefaultMaterialSystem` to statically specialize to the type of the lighting system, we end up to use generics, either by making the whole type generic:

```
struct DefaultMaterialSystem< L : ILightingSystem >
{
    ParameterBlock<L> lighting;
    ...
}
```

or by just making the `doMaterialStuff()` operation generic, with the lighting system being passed in as a parameter.

```
struct DefaultMaterialSystem
{
    ...
    void doMaterialStuff< L : ILightingSystem>( L lighting )
    {
        lighting.doLightingStuff();
    }
}
```

Each of those options moves the responsibility for managing the lighting system type up a level of abstraction: whatever code works with a material system needs to manage the details.

When we now step up the next level to the `Integrator` module, the approach using `struct`s really starts to show cracks.
We have the option of making the `DefaultIntegeratorSystem` a generic on the type of *both* subsystems, and reference them via parameter blocks:

```
struct DefaultIntegratorSystem< L : ILightingSystem, M : IMaterialSystem >
{
    ParameterBlock<L> lighting;
    ParameterBlock<M> material;
    ...
}
```

or we have to make all the relevant operations on the integrator take both subsystems as pass the relevant subsystem instances in as parameters:

```
struct DefaultIntegratorSystem
{
    ...
    void doIntegeratorStuff< L : ILightingSystem, M : IMaterialSystem >(
        L lighting,
        M material)
    {
        lighting.doLightingStuff();
        material.doMaterialsStuff(lighting);
    }
}
```

In each case, more and more responsibiity for configuration of implementation details is being punted up to the next higher level of abstraction.
In the first case, somebody else is responsible for instantiating a type like:

```
DefaultIntegeratorSystem<DefaultLightingSystem, DefaultMaterialSystem<DefaultLightingSystem>>
```

Also, the application code that works with that messy type needs to make sure to fill in *one* parameter block for the lighting system, but set it into *both* the material system and integrator.

In the second case, note how the integrator already has to ensure that it passes along the `lighting` subsystem to the `doMaterialStuff` operation, and anybody who invokes `doIntegratorSutff` would have to do the same, but for *two* subsystems.

The whole thing doesn't scale and becomes intractable with more than a few subsystems.
Trying to work anything like inheritance into the mix just falls flat completely.

Related Work
------------

There is a lot of work in general-purpose programming languages around defining larger-scale modularity units.

SML (Standard ML) has both modules and *signatures*, which are effectively interfaces for modules.
Modules can be parameterized on other modules based on signatures, and instantiated to use different concrete implementations.

Beta and gbeta unify both classes and functions into a single construct called a *pattern*, and show that patterns (including pattern inheritance) can be used for things akin to traditional modules.
A variety of techniques for *family polymorphism* in world of Java and similar languages followed on from that tradition.
The Scala language continues in the same vein, with papers and presentations on Scala advocating for using `class`es to model large units of modularity akin to modules.

In the world of "enterprise" software using Java, C#, JavaScript, etc. there is a large family of techniques and system for "dependency inversion" which is used to automate some or all of the process of "wiring up" the concrete implementations of various subsystems/components based on explicit representations of dependencies (often attached as metadata on the fields of a type).

Modern general-purpose game engines like Unity and Unreal often use a "component" concept, where a game entity/object is composed of multiple loosely-coupled sub-objects (components).
Often these systems allow dependencies between component types to be stated explicitly, with runtime or tools support for ensuring that objects are not created with unsatisifed dependencies.

Note that almost all of the approaches enumerated above rely deeply on the fact that a dependency of unit `X` on unit `Y` can be handily represented as a single pointer/reference in most general-purpose programming languages. For example, in C++:

```
class Y { ... };
class X
{
    Y* y;
    ...
};
```

In the above, an instance of `X` can always find the `Y` it depends on easily and (relatively) efficiently.
There is no particularly high overhead to having `X` diretly store an indirect reference to `Y` (at least not for coarse-grained units), and it is trivial for multiple units like `X` to all share the same *instance* of `Y` (potentially even including mutable state, for applications that like that sort of thing).

In general most CPU languages (and especially OOP ones) can express the concepts of "is-a" and "has-a" but they often don't distinguish between when "has-as" means "refers-to-and-depends-on-a" vs. when it means "aggregates-and-owns-a".
This is important when looking at a GPU language like Slang, where "aggergates-and-owns-a" is easy (we have `struct` types), but "refers-to-and-depends-on-a" is harder (not all of our targets can really support pointers).

Proposed Approach
-----------------

We propose to introduce a new construct called a *component type* to Slang, which can be used to describe units of modularity larger than what `struct`s are good for, but that is intentionally defined in a way that allows it to be used in cases where fully general `class` types could not be supported.

To render our earlier examples in terms of component types:

```
// Lighting.slang

interface ILight { ... }

interface ILightingSystem
{
    void doLightingStuff();
}

__component_type DefaultLightingSystem : ILightingSystem
{
    StructuredBuffer<ILight> lights;

    void doLightingStuff() { ... }
}
```

```
// Materials.slang
import Lighting;

interface IMaterial { ... }
interface IMaterialSystem
{
    void doMaterialStuff();
}

__component_type DefualtMaterialSystem : IMaterialSystem
{
    __require lighting : ILightingSystem;

    StructuredBuffer<IMaterial> materials;

    void doMaterialStuff()
    {
        lighting.doLightingStuff();
    }
}
```

```
// Integrator.slang
import Lighting;
import Materials;

interface IIntegratorSystem
{
    void doIntegratorStuff();
}

__component_type DefaultIntegratorSystem : IIntegratorSystem
{
    __require ILightingSystem;
    __require IMaterialSystem;

    ParameterBlock<IntegratorParams> params;

    void doIntegeratorStuff()
    {
        doLightingStuff();
        doMaterialsStuff();
    }
}
```

The `__component_type` keyword is akin to `struct` or `class`, but introduces a component type.
Component types are similar to both structure and class types in that they can:

* Conform to zero or more interfaces
* Define fields, methods, properties, and nested types
* Eventually: optionally inherit from one (or more) other component types

The key thing that a `__component_type` can do that a `struct` cannot (but that a `class` might be allowed to) is include nested `__require` declarations.
In the simplest form, a require declaration is of the form:

```
__require someName : ISomeInterface;
```

Within the scope where the `__require` is visible, `someName` will refer to a value that conforms to `ISomeInterface`, but code need not know what the value is (nor what its type is).
The other form of `__require`:

```
__require ISomeInterface;
```

Can be seen as shorthand for something like:

```
__require _anon : ISomeInterface;
using anon.*; // NOTE: not actual Slang syntax
```

One more construct is needed to complete the feature, and it can introduced by illustrating a concerete type that pulls together our default implementations:

```
__component_type MyProgram
{
    __aggregate lighting : DefaultLightingSystem;
    __aggregate DefaultMaterialSystem;
    __aggregate DefaultIntegeratorSystem;
}
```

An `__aggregate` declaration is only allowed inside a `__component_type` (or a `class`, if we allow it).
Similar to `__require`, an `__aggregate` can either name the thing being aggregated, or leave it anonymous (and have its members imported into the current scope).
The semantics of `__aggregate SomeType` are similar to just declaring a field of `SomeType`, but the key distinction is that the aggregated sub-object is conceptually allocated and initialized as *part* of the outer object (one alternative name for the keyword would be `__part`).
It is not possible to assign to an `__aggregate` member like it is a field (although if the type is a reference type, *its* fields are visible and might still be mutable).

At the point where an `__aggregate SomeType` member is declared, the front-end semantic checking must be able to find/infer the identity of a value to use to satisfy each `__require` member in `SomeType`.
For example, because `DefaultIntegratorSystem` declares `__require IMaterialSystem`, the compiler searches in the current context for a value that can provide that interface.
It finds a single suitable value: the value implicitly defined by `__aggregate DefaultMaterialSystem`, and thus "wires up" the input dependency of the `DefaultIntegratorSystem`.

It is posible for a `__require` in an `__aggregate`d member to be satisfied via another `__require` of its outer type:

```
__component_type MyUnit
{
    __require ILightingSystem;
    __aggregate DefaultMaterialSystem;
}
```

In the above example, the `ILightingSystem` requirement in `DefaultMaterialSystem` will be satisfied using the `ILightingSystem` `__require`d by `MyUnit`.

In cases where automatic search and connection of dependencies does not work (or yields an ambiguity error), the user will need some mechanism to be able to explicitly specify how dependencies should be satisfied.

While the above examples do not show it, component types should be allowed to contain shader entry points.

Detailed Explanation
--------------------

Component types need to be restricted in where and how they can be used, to avoid creating situations that would give them all the flexiblity of arbitrary `class`es.
The only places where a component type may be used are:

* `__require` and `__aggregate` declarations
* Function parameters
* Generic arguments (??? Need to double-check how this can go wrong)

Any given `__component_type` either has no `__require`s and is thus concrete, or it has a nonzero number of `__require`s and is abstract.

We can ignore the `__require`s in a component type (if any) and form an equivalent `struct` type.
In that `struct` type, `__aggregate`s turn into ordinary fields.
For example, the `MyProgram` (concrete) and `MyUnit` (abstract) types above become:

```
struct MyProgram
{
    DefaultLightingSystem lighting;
    DefaultMaterialSystem _anon0;
    DefaultIntegeratorSystem _anon1;
};
struct MyUnit
{
    DefaultMaterialSystem _anon2;
};
```

When a component type is used as a function parameter (including an implicit `this` parameter) it effectively maps to a function that takes additional (generic) parameters corresponding to each `__require`.
For example, given:

```
void doStuff( MyUnit u ) { ... u.doMaterialStuff(); ... }
```

we would generate something like:

```
void doStuff< T : ILightingSystem >(
    T _anon3, // for the `__require : ILightingSystem` in `MyUnit`
    MyUnit u)
{
    ...
    DefaultMaterialSystem_doMaterialStuff(_anon3, u._anon2);
    ...
}
```

Note that when the generated code invokes an operation through one of the `__aggregate`d members of a component type, where the aggregated type had `__require`ments, the compiler must pass along the additional parameters that represent those requirements in the current context.

Effectively, the compiler generates all of the boilerplate parameter-passing that the programmer would have otherwise had to write by hand.

It might or might not be obvious that the notion of "component type" being described here has a clear correspondance to the `IComponentType` interface provided by the Slang runtime/compilation API.
It should be possible for us to provide reflection services that allow a programmer to look up a component type by name and get an `IComponentType`.
The existing APIs for composing, specializing, and linking `IComponentType`s should Just Work for explicit `__component_type`s.
Large aggregates akin to `MyProgram` above can be defined entirely via the C++ `IComponentType` API at program runtime.

Questions
---------

### How do we explain to users when to use component types vs. when to use modules? Or `struct`s?

The basic advice should be something like:

* If the thing feels subsystem-y, favor modules or component types. If it feels object-y or value-y, use a `struct`. This is loose, but intuition is good here.

* If the thing wants to depend on other subsystems through `interface`s, to allow mix-and-match flexibility, it should probably be a component type and not a module.

* If the thing wants to have its own shader parameters, then we encourage users to consider that a component type is likely to be what they want, so that they don't pollute the global scope.

That last point is important, since a component type allows users to define a collection of global shader parameters and entry points that use them as a unit, without putting those parameters in the global scope, which is something that was not really possible before.

### Can the `__component_type` construct just be subsumed by either `struct` or `class`?

Maybe. The key challenge is that component types need to provide the "look and feel" of by-refernece re-use rather than by-value copying. A `__require T` should effectively act like a `T*` and not a bare `T` value, so I am reluctant to say that should map to `struct`.

### But what about `[mutating]` operations and writing to fields of component types, then?

Yeah... that's messy. If component types really are by-reference, then they should be implicitly mutable even without passing as `inout`, and should ideally also support aliasing. We need to make sure we get clarity on this.

### Is `__aggregate` really required? Isn't it basically just a field?

An `__aggregate X` acts a lot like a field if `X` is a *value* type, but in cases where `X` is a *reference* type, there is a large semantic distinction.

### How does all this stuff relate to inheritance?

There are some things that can be done with (multiple) inheritance that can also be expressed via `__require`s. For example, both can represent the "diamond" pattern:

```
class A { ... }
class B : A { ... }
class C : A { ... }
class D : B, C { ... }
```

```
__component_type A { ... }
__component_type B { __require A; ... }
__component_type C { __require A; ... }
__component_type D { __require B; __require C; ... }
```

The Spark shading language research project used multiple mixin class inheritance to compose units of shader code akin to what are being proposed here as coponent types (hmm... I guess that should go into related work...).

In general, using inheritance to model something that isn't an "is-a" relationship is poor modeling.
Inheritance as a modelling tool cannot capture some patterns that are possible with `__aggregate` (notably, with mixin inheritance you can't get multiple "copies" of a component).
Most importantly, when inheritance is abused for modeling like this, the resulting code can be confusing. Consider:

```
abstract class MyFeature : ISystemA, ISystemB
{ ... }
```

From this declaration, it is not possible to tell whether `MyFeature` implements just `ISystemA`, just `ISystemB`, both, or neither.
The distinction between an inheritance clause ("I implement this thing") vs. a `__require` ("I need *somebody else* to implement this thing") is important documentation.

Alternatives Considered
-----------------------

I'm not aware of any big design alternatives that don't amount to more or less the same thing with different syntax.
One alternatives is to try to do something like ML-style "signature" for our modules, and allow something like `import ILighting` to allow module-level dependencies on abstracted interfaces.

Another alternative is to do what this document proposes, but make it work with the existing `struct` keyword (or `class`) instead of adding a new one.
