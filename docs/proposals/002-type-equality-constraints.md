Allow Type Equality Constraints on Generics
===========================================

We propose to allow *type equality* constraints in `where` clauses.

Status
------

In progress.

Background
----------

As of proposal [001](001-where-clauses.md), Slang allows for generic declarations to include a *`where` clause* which enumerates constraints on the generic parameters that must be satisfied by any arguments provided to that generic:

    V findOrDefault<K, V>( HashTable<K,V> table, K key )
        where K : IHashable,
              V : IDefaultInitializable
    { ... }

Currently, the language only accepts *conformance* constraints of the form `T : IFoo`, where `T` is one of the parameters of the generic, and `IFoo` is either an `interface` or a conjunction of interfaces, which indicate that the type `T` must conform to `IFoo`.

This proposal is motivated by the observation that when an interface has associated types, there is currently no way for a programmer to introduce a generic that is only applicable when an associated type satisfies certain constraints.

As an example, consider an interface for types that can be "packed" into a smaller representation for in-memory storage (instead of a default representation optimized for access from registers):

    interface IPackable
    {
        associatedtype Packed;

        init(Packed packed);
        Packed pack();
    }

Next, consider an hypothetical interface for types that can be deserialized from a stream:

    interface IDeserializable
    {
        init( InputStream stream );
    }

Given these definitions, we might want to define a function that takes a packable type, and deserializes it from a stream:

    T deserializePackable<T>( InputStream stream )
        where T : IPackable
    {
        return T( T.Packed(stream) );
    }

As written, this function will fail to compile because the compiler cannot assume that `T.Packed` conforms to `IDeserializable`, in order to support initialization from a stream.

A brute-force solution would be to add the `IDeserializable` constraint to the `IPackable.Packed` associated type, but doing so may not be consistent with the vision the designer of `IPackable` had in mind. Indeed, there is no reason to assume that `IPackable` and `IDeserializable` even have the same author, or are things that the programmer trying to write `deserializePackable` can change.

It might seem that we could improve the situation by introducing another generic type parameter, so that we can explicitly constraint it to be deserializable:

    T deserializePackable<T, U>( InputStream stream )
        where T : IPackable,
              P : IDeserializable
    {
        return T( U(stream) );
    }

This second attempt *also* fails to compile.
In this case, there is no way for the compiler to know that `T` can be initialized from a `P`, because it cannot intuit that `P` is meant to be `T.Packed`.

Our two failed attempts can each be fixed by introducing two new kinds of constraints:

* Conformance constraints on associated types: `T.A : IFoo`

* Equality constraints on associated types: `T.A == X`

Related Work
------------

Both Rust and Swift support additional kinds of constraints on generics, including the cases proposed here.
The syntax in those languages matches what we propose.

Proposed Approach
-----------------

In addition to conformance constraints on generic type parameters (`T : IFoo`), the compiler will also support constraints on associated types of those parameters (`T.A : IFoo`), and associated types of those associated types (`T.A.B : IFoo`), etc.

In addition, the compiler will accept constraints that restrict an associated type (`T.A`, `T.A.B`, etc.) to be equal to some other type.
The other type may be a concrete type, another generic parameter, or another associated type.

Detailed Explanation
--------------------

### Parser

The parser already supports nearly arbitrary type exprssions on both sides of a conformance constraint, and then validates that the types used are allowed during semantic checking.
The only change needed at that level is to split `GenericTypeConstraintDecl` into two cases: one for conformance constraints, and another for equality constraints, and then to support constraints with `==` instead of `:`.

### Semantic Checking

During semantic checking, instead of checking that the left-hand type in a constraint is always one of the generic type parameters, we could instead check that the left-hand type expression is either a generic type parameter or `X.AssociatedType` where `X` would be a valid left-hand type.

The right-hand type for conformance constraints should be checked the same as before.

The right-hand type for an equality constraint should be allowed to be an arbitrary type expression that names a proper (and non-`interface`) type.

One subtlety is that in a type expression like `T.A.B` where both `A` and `B` are associated types, it may be that the `B` member of `T.A` can only be looked up because of another constraint like `T.A : IFoo`.
When performing semantic checking of a constraint in a `where` clause, we need to decide which of the constraints may inform lookup when resolving a type expression like `X.A`.
Some options are:

* We could consider only constraints that appear before the constraint that includes that type expression. In this case, a programmer must always introduce a constraint `X : IFoo` before a constraint that names `X.A`, if `A` is an associated type introduced by `IFoo`.

* We could consider *all* of the constraints simultaneously (except, perhaps, the constraint that we are in the middle of checking).

The latter option is more flexible, but may be (much) harder to implement in practice.
We propose that for now we use for first option, but remain open to implementing the more general case in the future.

Given an equality constraint like `T.A.B == X`, semantic checking needs detect cases where an `X` is used and a `T.A.B` is expected, or vice versa.
These cases should introduce some kind of cast-like expression, which references the type equality witness as evidence that the cast is valid (and should, in theory, be a no-op).

Semantic checking of equality constraints should identify contradictory sets of constraints.
Such contradictions can be simple to spot:

    interface IThing { associatedtype A; }
    void f<T>()
        where T : IThing,
              T.A == String,
              T.A == Float,
    { ... }

but they can also be more complicated:

    void f<T,U>()
        where T : IThing,
              U : IThing,
              T.A == String,
              U.A == Float,
              T.A == U.A
    { ... }

In each case, an associated type is being constrained to be equal to two *different* concrete types.
The is no possible set of generic arguments that could satisfy these constraints, so declarations like these should be rejected.

We propose that the simplest way to identify and diagnose contradictory constraints like this is during canonicalization, as described below.

### IR

At the IR level, a conformance constraint on an associated type is no different than any other conformance constraint: it lowers to an explicit generic parameter that will accept a witness table as an argument.

The choice of how to represent equality constraints is more subtle.
One option is to lower an equality constraint to *nothing* at the IR level, under the assumption that the casts that reference these constraints should lower to nothing.
Doing so would introduce yet another case where the IR we generate doesn't "type-check."
The other option is to lower a type equality constraint to an explicit generic parameter which is then applied via an explicit op to convert between the associated type and its known concrete equivalent.
The representation of the witnesses required to provide *arguments* for such parameters is something that hasn't been fully explored, so for now we propose to take the first (easier) option.

### Canonicalization

Adding new kinds of constraints affects *canonicalization*, which was discussed in proposal 0001.
Conformane constraints involving associated types should already be order-able according to the rules in that proposal, so we primarily need to concern ourselves with equality constraints.

We propose the following approach:

* Take all of the equality constraints that arise after any expansion steps
* Divide the types named on either side of any equality constraint into *equivalence classes*, where if `X == Y` is a constraint, then `X` and `Y` must in the same equivalence class
  * Each type in an equivalence class will either be an associated type of the form `T.A.B...Z`, derived from a generic type parameter, or a *independent* type, which here means anything other than those associated types.
  * Because of the rules enforced during semantic checking, each equivalence class must have at least one associated type in it.
  * Each equivalence class may have zero or more independent types in it.
* For each equivalence class with more than one independent type in it, diagnose an error; the application is attempting to constrain one or more associated types to be equal to multiple distinct types at once
* For each equivalence class with exactly one independent type in it, produce new constraints of the form `T.A.B...Z == C`, one for each associated type in the equivalence class, where `C` is the independent type
* For each equivalence class with zero independent types in it, pick the *minimal* associated type (according to the type ordering), and produce new constraints of the form `T.A... == U.B...` for each *other* associated type in the equivalence class, where `U.B...` is the minimal associated type.
* Sort the new constraints by the associated type on their left-hand side.

Alternatives Considered
-----------------------

The main alternative here would be to simply not have these kinds of constraints, and push programmers to use type parameters instead of associated types in cases where they want to be able to enforce constraints on those types.
E.g., the `IPackable` interface from earlier could be rewritten into this form:


    interface IPackable<Packed>
    {
        init(Packed packed);
        Packed pack();
    }

With this form for `IPackable`, it becomes possible to use additional type parameters to constraint the `Packed` type:

    T deserializePackable<T, U>( InputStream stream )
        where T : IPackable<U>,
              P : IDeserializable
    {
        return T( U(stream) );
    }

While this workaround may seem reasomable in an isolated example like this, there is a strong reason why languages like Slang choose to have both generic type parameters (which act as *inputs* to an abstraction) and associated types (which act as *outputs*).
We believe that associated types are an important feature, and that they justify the complexity of these new kinds of constraints.