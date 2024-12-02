SP #001: `where` Clauses
===============

We propose to allow generic declarations in Slang to move the constraints on generic type parameters outside of the `<>` and onto distinct `where` clauses.

Status
------

Status: Partially implemented. The only unimplemented case is the canonicalization of generic constraints.

Implementation: [PR 4986](https://github.com/shader-slang/slang/pull/4986)

Reviewed by: Theresa Foley, Yong He


Background
----------

Slang supports generic type parameters with *constraints* on them.
Currently constraints can only be written as part of the declaration of the type parameter itself, e.g.:

    void resolve<T: IResolvable, U: IResolver<T>, V: IResolveDestination<T>>(
        ResolutionContext<U> context, List<T> stuffToResolve, out V destination)
    { ... }

The above example illustrates how intermixing the declaration of the type parameters with their constraints can make for long declarations that can be difficult for programmers to read and understand.

Introducing `where` clauses allows a programmer to state the constraints *after* the rest of the declaration header, e.g.:

    void resolve<T, U, V>(ResolutionContext<U> context, List<T> stuffToResolve, out V destination)
        where T : IResolvable,
        where U : IResolver<T>,
        where V : IResolveDestination<T>
    { ... }

This latter form makes it easier to quickly glean the overall shape of the function signature.

A second important benefit of `where` clauses is that they open the door to expressing more complicated constraints on and between type parameters, such as allowing constraints on *associated types*, e.g.:

    void writePackedData<T, U>(T src, out U dst)
        where T : IPackable,
        where T.Packed : IWritable<U>
    { .. }

Related Work
------------

Many other languages with support for generics have introduced `where` clauses, and most follow a broadly similar shape. To present our `resolve` example in various other languages:

### Rust

Rust supports `where` clauses with a comma-separated list of constraints:

    fn resolve<T, U, V>(context: ResolutionContext<U>, stuffToResolve: List<T>, destination: mut& V)
        where T : IResolvable,
              U : IResolver<T>,
              V : IResolveDestination<T>,
    { ... }

### Swift

Swift's `where` clauses are nearly identical to Rust's:

    fn resolve<T, U, V>(context: ResolutionContext<U>, stuffToResolve: List<T>, destination: out V)
        where T : IResolvable,
              U : IResolver<T>,
              V : IResolveDestination<T>,
    { ... }

### C#

C# is broadly similar, but uses multiple `where` clauses, one per constraint:

    void resolve<T, U, V>(ResolutionContext<U> context, List<T> stuffToResolve, out V destination)
        where T : IResolvable
        where U : IResolver<T>
        where V : IResolveDestination<T>
    { ... }

### Haskell

While Haskell is a quite different language from the others mentioned here, Haskell typeclasses have  undeniably influenced the concept of traits/protocols in Rust/Swift.

In Haskell a typeclass is not something a type "inherits" from, and instead uses type parameter for even the `This` type.
Type parameters in Haskell are also introduced implicitly rather than explicitly.
The `resolve` example above would become something like:

    resolve :: (Resolvable t, Resolver u t, ResolveDestination v t) =>
        ResolutionContext u -> List t -> v

We see here that the constraints are all grouped together in the `(...) =>` clause before the actual type signature of the function.
That clause serves a similar semantic role to `where` clauses in these other languages.

Proposed Approach
-----------------

For any kind of declaration that Slang allows to have generic parameters, we will allow a `where` clause to appear after the *header* of that declaration.
A `where` clause consists of the (contextual) keyword `where`, following by a comma-separated list of *constraints*:
```csharp
    struct MyStuff<T, U> : IFoo
        where T : IFoo, IBar
        where T : IBaz
        where U : IArray<T>
    { ... }
```
A `where` clause is only allowed after the header of a declaration that has one or more generic parameters.

Each constraint must take the form of one of the type parameters from the immediately enclosing generic parameter list, followed by a colon (`:`), and then followed by a type expression that names an interface or a conjunction of interfaces.
Multiple constraints can be defined for the same parameter.

We haven't previously defined what the header of a declaration is, so we briefly illustrate what we mean by showing where the split between the header and the *body* of a declaration is for each of the major kinds of declarations that are supported. In each case a comment `/****/` is placed between the header and body:

```csharp
// variables:
let v : Int /****/ = 99;
var v : Int /****/ = 99;
Int v /****/ = 99;

// simple type declarations:
typealias X : IFoo /****/ = Y;
associatedtype X : IFoo /****/;

// functions and other callables:
Int f(Float y) /****/ { ... }
func f(Float y) -> Int /****/ { ... }
init(Float y) /****/ { ... }
subscript(Int idx) -> Float /****/ { ... }

// properties
property p : Int /****/ { ... }

// aggregates
extension Int : IFoo /****/ { ... }
struct Thing : Base /****/ { ... }
class Thing : Base /****/ { ... }
interface IThing : IBase /****/ { ... }
enum Stuff : Int /****/ { ... }
```
In practice, the body of a declaration starts at the `=` for declarations with an initial-value expression, at the opening `{` for declarations with a `{}`-enclosed body, or at the closing `;` for any other declarations.

With introduction of `where` clauses, we can extend type system to allow more kinds of type constraints. In this proposal,
we allow type constraints followed by `where` to be one of:
- Type conformance constraint, in the form of `T : IBase`
- Type equality constraint, in the form of `T == X`

In both cases, the left hand side of a constraint can be a simple generic type parameter, or any types that are dependent on some
generic type parameter. For example, the following is allowed:
```csharp
interface IFoo { associatedtype A; }
struct S<T, U>
    where T : IFoo
    where T.A == U
{}
```

Detailed Explanation
--------------------

### Implementation

The compiler implementation already represents generics in a form where the type parameters are encoded separately from the constraints that depend on them.
The constraints act somewhat like additional unnamed parameters of a generic.
At the Slang IR level these constraint parameters are made into explicit parameters used to pass around *witness tables*.

During parsing, a `where` clause can simply add the constraints to the outer generic (and error out if there isn't one).
The actual representation of constraints will be no different than before, so many downstream compilation steps should be unaffected.

Some parts of the codebase have historically assumed that a given generic type parameter can have at most *one* constraint;
these cases will need to be identified and fixed to allow for zero or more constraints per parameter.

Semantic checking of generics will need to validate that the left-hand side of each constraint is a direct reference to one of the type parameters of the immediately enclosing generic;
previously, the semantic checking logic could *assume* that this was the case, since the parser would only create constraints in that form.

### Interaction With Overloading and Redeclaration

Probably the most important semantic issue that arises from `where` clauses is deciding whether two different function declarations count as distinct overloads, or as redeclarations (or redfinitions) of the same function signature.

The existing form for declaring constraints:

    void f<T : IFoo>( ... )
    { ... }

should be treated as sugar for the equivalent `where`-based form:

    void f<T>( ... )
        where T : IFoo
    { ... }

The two declarations of `f` there should not only be counted as redeclarations/redefinitions, but they should also be *indistinguishable* to all clients of the module where they appear.
A module that `import`s the module defining `f` should not be able to tell which form it was declared with.
Both forms of the definition should result in the *same* signature and mangled name in Slang IR.

Furthermore, with `where` clauses it becomes possible to write equivalent constraints in more than one way.
A `where` clause can be used instead of a conjunction of interfaces:

    void f<T : IFoo & IBar>( ... )
    { ... }

    void f<T>( ... )
        where T : IFoo,
              T : IBar
    { ... }

It is also possible to use `where` clauses to introduce constraints that are *redundant*, either by repeating the same constraint:

    void f<T>( ... )
        where T : IFoo,
              T : IFoo
    { ... }

or by constraining a type to two interfaces, where one inherits from the other:

    interface IBase {}
    interface IDerived : IBase {}

    void f<T>( ... )
        where T : IBase,
              T : IDerived
    { ... }

Technically it was already possible to have redundancy in a constraint by using a conjunction of two interfaces where one inherits from the other:

    void f<T : IBase & IDerived>( ... )
    { ... }

One question that is raised by the possibility of redundant constraints is whether the compiler should produce a diagnostic for them and, if so, whether it should be a warning or an error.
While it may seem obvious that redundant constraints are to be avoided, it is possible that refactoring of `interface` hierarchies could change whether existing constraints are redundant or not, potentially forcing widespread edits to code that is semantically unambiguous (and just a little more verbose than necessary).
We propose that redundant constraints should probably produce a warning, with a way to silence that warning easily.

### Canonicalization

The long and short of the above section is that there can be multiple ways to write semantically equivalent generic declarations, by changing the form, order, etc. of constraints.
We want the signature of a function (and its mangled name, etc.) to be identical for semantically equivalent declaration syntax.
In order to ensure that a declaration's mangled name is independent of the form of its constraints, we must have a way to *canonicalize* those constraints.

The Swift compiler codebase includes a document that details the rules used for canonicalization of constraints for that compiler, and we can take inspiration from it.
Our constraints are currently much more restricted, so canonicalization can follow a much simpler process, such as:

* Start with the list of user-written constraints, in declaration order
* Iterate the following to convergence:
  * For each constraint of the form `T : ILeft & IRight`, replace that constraint with constraints `T : ILeft` and `T : IRight`
* Remove each constraint that is implied by another constraint
  * For now, that means removing `T : IBase` if there is already a constraint `T : IDerived` where `IDerived` inherits from `IBase`
* Sort the constraints
  * For constraints `T : IFoo` and `U : IBar` on different type parameters, order them based on the order of the type parameters `T` and `U`
  * For constraints `T : IFoo` and `T : IBar` on the *same* type parameter, order them based on a canonicalized ordering on the interfaces `IFoo` and `IBar`

The above ordering assumes that we can produce a canonical ordering of `interface`s.
More generally, we will eventually want a canonical ordering on all types and *values* that might appear in constraints.
For now, we will limit ourselves to an ordering on nominal types, and other declaration references:

* A generic parameter is always ordered before anything other than generic parameters
  * Parameters from outer generics are ordered before those from inner generics
  * Parameters from the same generic are ordered based on their order in the parameter list
* Two declaration references to distinct declarations are ordered based on a lexicographic order for their qualified names, meaning:
  * If one qualified name is a prefix of the other (e.g., `A.B` and `A.B.C`), then the prefix is ordered first
  * Otherwise, compare the first name component (from left to right) where the names differ, and order them based on a lexicographic string comparison of the name at that component.

Alternatives Considered
-----------------------

There really aren't any compelling alternatives to `where` clauses among the languages that Slang takes design influence from.
We could try to design something to solve the same problems from first principles, but the hypothetical benefits of doing so are unclear.

When it comes to the syntactic details, we could consider disallow type lists in the right hand side of a conformance constraint, and return allow multiple constraints to be separated with comma and sharing with one `where` keyword:

    struct MyStuff<T> : Base, IFoo
        where T : IFoo,
              T : IBar
    { ... }

This alternative form may result in more compact code without needing duplicated `where` clause, but may be harder to achieve tidy diffs when editing the constraints on declarations.

Future Directions
-----------------

### Allow more general types on the right-hand side of `:`

Currently, the only constraints allowed using `:` have a concrete (non-`interface`) type on the left-hand side, and an `interface` (or conjunction of interfaces) on the right-hand side.
In the context of `class`-based hierarchies, we can also consider having constraints that limit a type parameter to subtypes of a specific concrete type:

    class Base { ... }
    class Derived : Base { ... }

    void f<T>( ... )
        where T : Base
    { ... }

### Allow `where` clauses on non-generic declarations

We could consider allowing `where` clauses to appear on any declaration nested under a generic, such that those declarations are only usable when certain additional constraints are met.
E.g.,:

    struct MyDictionary<K,V>
    {
        ...

        K minimumKeyUsed()
            where K : IComparable
        { ... }
    }

In this example, the user's dictionary type can be queried for the minimum key that is used for any entry, but *only* if the keys are comparable.

Most of what can be done with this more flexible placement of `where` clauses can *also* be accomplished using extensions.
E.g., the above example could instead be written:

    struct MyDictionary<K,V>
    { ... }

    extension<K,V> MyDictionary<K,v>
        where K : IComparable
    {
        K minimumKeyUsed()
        { ... }
    }

### Implied Constraints

In many cases a generic function signature will use the type parameters as explicit arguments to generic types that impose their own requirements.
To be concrete, consider:

    struct Dictionary<K, V>
        where K : IHashable
    { ... }

    V myLookupFunc<K,V>(
        Dictionary<K,V> dictionary, K key, V default)
    { ... }

In this case, the current Slang language rules will reject `myLookupFunc`. The type of the `dictionary` parameter is passing `K` as an argument to `Dictionary<...>` but does not have an in-scope constraint that ensures that `K : IHashable`.
The current compiler requires the function to be rewritten as:

    V myLookupFunc<K,V>(
        Dictionary<K,V> dictionary, K key, V default)
        where K : IHashable
    { ... }

But this additional constraint ends up being pointless; in order to invoke `myLookupFunc` the programmer must have a `Dictionary<K,V>` to pass as argument for the `dictionary` parameter, which means that the `Dictionary<K,V>` type must already be well-formed based on the information the caller function has.

The compiler can eliminate the need for such constraints by adding additional rules for expanding the set of constraints on a generic during canonicalization.
For any generic type `X<A, B, C, ...>` appearing in:

* the signature of a function declaration
* the bases of a type declaration
* the existing generic constraints

The expansion step would add whatever constraints are required by `X`, with the arguments `A, B, C, ...` substituted in for the parameters of `X`.
