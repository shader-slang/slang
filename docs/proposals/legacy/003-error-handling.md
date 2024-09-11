Error Handling
==============

Slang should support a modern system for functions to signal, propagate, and handle errors.

Status
------

In discussion.

Background
----------

Errors happen. It is impossible for any programming language to statically rule out the possibility of unexpected situations arising at runtime.
There are a wide variety of strategies used in programming, both provided by languages and enforced by idiom in codebases.

Not all errors are alike, in that some are more expected and reasonable to handle than others.
Most errors can fit into a few broad categories like:

* Unrecoverable or nearly unrecoverable failures like resource exhaustion (out of memory), or an OS-level signal to terminate the process.

* Incorrct usage of an API in ways that violate invariants. For example, passing a negative value to a function that says it only accepts positive values.

* Out-of-range or otherwise invalid data coming from program users. For example, a console program asks the user to type a number, but the user enters some string that does not parse as a number.

* Failure of operation that will usually succeed, but for which exceptional circumstances can lead to failures. For example, when reading from an open file we typically expect success, but failure is possible for many reasons outside of a programmer's control (like network disruption when accessing a remote file). A robust program often wants to recover from such failures, but often the policy for how recovery should occur is at a higher level than the code that first detects the error.

These different categories often benefit from different strategies:

* Typically there is neither a reason nor a desire to do anything about nearly-unrecoverable errors; the program has well and truly crashed.

* When programmers violate the invariants of an API, they typically want to know about it as early as possible (during development) so they can fix their code. Breaking into the debugger is often the best answer, and in many cases trying to propagate or recover from such failures would be wasted effort.

* When an operation could fail due to mal-formed data coming from a user, programmers typically want to be forced to handle the failure case at the point where the error may arise. In languages that have an `Optional<T>` or `Maybe<T>` type, it is often easiest to return that.

* Unpredictable, exceptional, and recoverable errors are among the hardest to deal with, and often benefit from direct language support.

The Slang language currently doesn't have direct support for *any* form of error handling, but this document focuses on errors in the last of the categories above.

Related Work
------------

In the absence of language support, developers typically signal and propagate errors using *error codes*. The COM `HRESULT` type is a notable example of a well-defined system for using error codes in C/C++ and other languages.
Error codes have the benefit of being easy to implement, and relatively light-weight.
The main drawback of error codes is that developers often forget to check and/or propagate them, and when they do remember to do so it adds a lot of boilerplate.
Additonally, reserving the return value of every function for returning an error code makes code more complex because the *actual* return value must be passed via a function parameter.

C++ uses *exceptions* for errors in various categories, including unpredictable but recoverable failures.
Propagation of errors up the call stack is entirely automatic, with unwinding of call frames and destruction of their local state occuring as part of the search for a handler.
Neither functions that may throw nor call sites to such functions are syntactically marked.
Exceptions in C++ have often been implemented in ways that add overhead and require complicated support in platform ABIs and intermediate languages to support.

Java uses exceptions with similar rules to C++, but adds a restriction that functions must be marked with the types of exceptions they may throw or propagate, except for those that inherit from `RuntimeException`, which are intended to represent some of the other categories of error in our taxonomy (like simple invariant violations and nearly unrecoverable errors).
The need to mark every function that might fail (or propagate failure) was seen by most developers at the time as unreasonably onerous.
Developers often smuggled other kinds of exceptions out through `RuntimeException`s, to get them through API layers that were not designed to support exceptions.

Both Rust and Swift try to strike a balance between error codes and languages with exceptions.
At a high level, each takes an approach where the generated code is comparable to an error-code-based solution (so that no special ABI or IL support is needed), but direct syntactic support makes propagating and/or handling errors more convenient.

In Rust, a function that returns `std::Result<SomeType, SomeError>` either returns successfully with a value of type `SomeType`, or fails with an error of type `SomeError`.
The `Result` type is itself just a Rust `enum`, so that results can be handled by pattern-matching with `match`, `if let`, etc.
Direct syntactic support is added so that in the body of a `Result`-returning function, a postfix `?` operator can be applied to an expression of type `Result<X, E>` to implicitly propagate `E` on any failure, and return the `X` value otherwise.
Some higher-order functions can Just Work with `Result`-returning functions, if their signatures are compatible, but many operations like `map()`, `fold()`, etc. need distinct overloads that support `Result`s.
Functions that return `X` and those that return `Result<X, ...>` are not directly convertible.

Swift provides more syntactic support for errors than Rust, although the underlying mechanism is similar.
A Swift function may have `throws` added between the parameter list and return type to indicate that a function may yield an error.
All errors in Swift must implement the `Error` protocol, and all functions that can `throw` may produce any `Error` (although there are proposals to extend Swift with "typed `throws`").
Any call site to a `throws` function must have a prefix `try` (e.g., `try f(a, b)`), which works simiarly to Rust's `?`; any error produced by the called function is propagated, and the ordinary result is returned.
Swift provides an explicit `do { ... } catch ...` construct that allows handlers to be established.
It also provides for conversion between exceptions and an explicit `Result<T,E>` type, akin to Rust's.
Higher-order functions may be declared as `rethrows` to indicate that whether or not they throw depends on whether or not any of their function-type parameters is actually a `throws` function at a call site.
Any non-`throws` function/closure may be implicitly converted to the equivalent `throws` signature, so that non-throwing functions are subtypes of the throwing ones.


The model used in Swift is compatible with the more general notion of *effects* in type theory.
A simple model of function types like `D -> R` can be extended to support zero or more effects `E0`, `E1`, etc. that live "on the arrow": `D -{E0, E1}-> R`.
Purely functional languages like Haskell sometimes use monads as a way to represent effects: a function `D -> IO R` is effectively a function from `D` to `R` with the addition effect that it may perform IO.
Making effects more explicit allows a type system to reason about sub-typing in the presence of effects (a function type without effect `E` is a subtype of a function with that effect), and to express code that is generic over effects.

Proposed Approach
-----------------

We propose a modest starting point for error handling in Slang that can be extended over time.
The model borrows heavily from Swift, but also focuses on strongly-typed errors.

The standard library will provide a built-in interface for errors, initially empty:

```
interface IError {}
```

User code can define their own types (`struct` or `enum`) that conform to `IError`:

```
enum MyError : IError
{
    BadHandle,
    TimedOut,
    // ...
}
```

User-defined functions (in both traditional and "modern" syntax) will support a `throws ...` clause to specify the type of errors that the function may produce:

```
float f(int x) throws MyError { ... }

func g(x: int) throws -> float MyError { ... }
```

Call sites to a `throws` function must wrap any potentially-throwing expression with a `try`:

```
float g(int y) throws MyError
{
    return 1.0f + try f(y-1);
}
```

Code can explicitly raise an error using a `throw` expression:

```
throw MyError.TimedOut;
```

We will allow `catch` clauses to come at the end of any `{}`-enclosed scope, where they will apply to any errors produced by `throw` or `try` expressions in that scope.

```
{
    ...
    try f(...);
    ...

    catch( e: MyError ) { ... }
}
```

We will also want to add `defer` statements, as they are defined in Go, Rust, Swift, etc.
The statements under a `defer` will always be run when exiting a scope, even if exiting as part of error propagation.

Detailed Explanation
--------------------

Consider a function that uses most of the facilities we have defined:

```
float example(int x) throws MyError
{
    if(someCondition)
    {
        throw MyError.TimedOut;
    }
    ...
    defer { someCleanup(); }
    ...
    {
        let y : int = 1 + try g(...);

        catch(e : MyError)
        { ... }
    }
    ...
    return someValue;
}
```

We will show how a function in this form can be transformed via incremental steps into something that can be understood and compiled without specific support for errors.

### Change Signature

First, we transform the signature of the function so that it returns something akin to an `Optional<MyError>` and returns its result via an `out` parameter, and modify any `return` points to write the `out` parameter and return `null` (the not-present case of `Optional<T>`):

```
MyError example_modified(int x, out float _result)
{
    ...

    _result = someValue;
    return null;
}
```

### Desugar `try` Expressions

Next we can convert any `try` expressions into a more explicit form, to match the transformation of signature. A statement like this:

```
let y : int = 1 + try g(...);
```

transforms into something like:

```
var _tmp : int;
let _err : Optional<MyError> = g_modified(..., out _tmp);
if( _err != null )
{
    throw _err.wrappedValue;
}
let y : int = 1 + _tmp;
```

### Desugar `throw` Expressions

For every `throw` site in a function body, there will either be no in-scope `catch` clause that matches the type thrown, or there will be eactly one most-deeply-nested `catch` that statically matches.
Front-end semantic checking should be able to associate each `throw` with the appropriate `catch` if any.

For `throw` sites with no matching `catch`, the operation simply translates to a `return` of the thrown error (because of the way we transformed the function signature).

For `throw` sites with a matching `catch`, we treat the operation a a "`goto` with argument" that jumps to the `catch` clause and passes it the error.
Note that our IR structure already has a concept of "`goto` with arguments".

### Desugar `defer` Statements

Handling of `defer` statements is actually the hardest part of this proposal, and as such we should probably handle `defer` as a distinct feature that just happens to overlap with what is being proposed here.

### Subtyping: Front-End

We should (at some point) add a `Never` type to the Slang type system, which would be an uninhabitable type suitable for use as the return type of functions that never return:

```
func exit(code: int) -> Never; // C `exit()` never returns
```

`Never` is effectively a subtype of *every* type and, as such, an expression of type `Never` can be implicitly converted to any type.

A `throw` expression has the type `Never`, allowing a user to write code like:

```
// Because `Never` can convert to `int`, this is valid:
int x = value > 0 ? value : throw MyError.OutOfBounds;
```

A function without a `throws` clause is semantically equivalent to a function with `throws Never`.
If we make that equivalence concrete at the type-system level, then a higher-order function can be generic over both throwing and non-throwing functions:

```
func map<D,R,E>(
    f: (D) throws E -> R,
    l: List<D>)
    throws E -> List<R>;
```

A function type with `throws X` is a subtype of a function with `throws Y` if `X` is a subtype of `Y`.
That includes the case where `X` is `Never`, so that a non-`throws` function type is a subtype of any `throws` function type with the same parameter/result signature.

### Subtyping: Low-Level

The subtyping relationship for `Never` *values* is irrelevant to codegen. Any place in the IR that has a `Never` value available to it represents unreachable code.

The subtyping relationship for `Never` in function types is more challenging, both for result types and error types. At the most basic, we can inject trampoline/thunk functions at any points where we have a `Never`-yielding function and need a function that returns `X` to pass along.

If we were doing low-level code generation for a platform where we can define our ABI, it would be possible to have `throws` and non-`throws` functions use distinct calling conventions, such that:

* The orinary parameters and reuslts are passed in the same registers/locations in both conventions.

* The error value (if any) in the `throws` convention is passed via registers/locations that are callee-save in the non-`throws` convention.

Under that model, a call site to a potentially-`throws` function can initialize the registers/locations for the error result to `null`/zero before dispatching to the callee.
If the callee is actually a non-`throws` function it would not touch those registers, and no error would be detected.
In that case, a non-`throws` function/closure could be used directly as a `throws` one with no conversion.
Such calling-convention trickery isn't really possible to implement when emitting code in a high-level language like HLSL/GLSL or C/C++.

Questions
--------------

### Should we support the superficially simpler case of "untyped" `throws`?

Having an `IError` interface allows us to eventually decide that `throws` without an explicit type is equivalent to `throw IError`.
It doesn't seem necessary to implement that convenience for a first pass, especially when there are use cases for `throws` that might not want to get into the mess of existential types.

### Should the transformations described here be implemented during AST->IR lowering, or at the IR level?

That's a great question! My guess is that some desugaring will happen during lowering, but we will probably want to keep `throws` functions more explicitly represented in the IR until fairly late, so that we can desugar them differently for different targets (if desired).

### Do we need `Optional<T>` to be supported to make this work?

It is unlikely that we'd need it to be a user-visible feature in a first pass, but we might want it at the IR level.
For this feature to work, we really need `sizeof(Optional<X>)` to be the same as `sizeof(X)` for simple cases where `X` is an `enum` or (for suitable targets) a type that is pointer-based.

A first pass at the feature might only support cases where error types are `enum`s and where the zero value is the "no error" case.

### Should we have a `Result<T, E>` type akin to what Rust/Swift have? Should a `throws E` function be equivalent to one that returns `Result<T, E>`?

That all sounds nice, but for now it seems like overkill.
Slang doesn't really have any facilities for programming with higher-order functions, pattern matching, etc. so adding types that mostly shine in those cases seems like a stretch.

Alternatives Considered
-----------------------

We could decide that Slang shouldn't be in the business of providing error-handling sugar *at all* and make this a problem for users.
That isn't really a reasonable plan for any modern language, but it is the status quo and null hypothesis if we don't start in on a better plan.

We could try to focus on C++ interop/compatibility and decide that errors in Slang should use exceptions, and only make "proper" language-supported error handling available to platforms that support exceptions at a suitably low level.
Doing so would give us all the disadvantages of C++ exceptions, and also mean that most of our users wouldn't end up using our error-handling tools, because doing so would render code non-portable.
