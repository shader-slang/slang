SP #011: Structured Binding
=================

Tuple types can reduce boilterplate code of defining auxiliary structs, but they can introduce readability issues because the elements are not named.
To mitigate this issue, we should support structured binding as a convenient way to access tuple elements with meaningful names.

# Status

Status: Proposal in review.

Implementation: N/A

# Proposed Approach

Users should be able to use `let` syntax to assign a composite type to a binding structure:

```
let tuple = makeTuple(1.0f, 2, 3);
let [a, b, c] = tuple;
```

Where the `let [...]` statement is a syntactic sugar of:
```
let a = tuple._0;
let b = tuple._1;
let c = tuple._2;
```

The right hand side of a structured binding can be a tuple, an array, or a struct type.
It is not an error if the composite value has more elements than the binding structure.

Mutable bindings are not allowed.

# Alternatives Considered

We could have allowed mutable bindings in the syntax of:
```
var [a,b,c] = ...
```
That defines mutable variables a,b,c whose values are copied from the structure.
However, mutable bindings can lead to confusions when modifying `a` doesn't change the value
int the source composite object from the binding. To avoid this confusion, we simply disallow
it.

Supporting mutation on the original composite object can be tricky as it involves reference types
that are not existent in the language. For simplicity we consider that to be out of scope of this
proposal.