# Optional Types

An `Optional<T>` value either holds a value of type `T` or holds no value.
The absence of a value is represented by the `none` literal.

## Declaration Syntax {#syntax}

```slang
Optional<T> varName;             // default-initialized to none
Optional<T> varName = expr;      // initialized with a value of type T
Optional<T> varName = none;      // explicitly initialized to none
```

### Parameters

- `T` is the value type. `T` may be any Slang type including interface types and generics.

## Members {#members}

| Member | Type | Description |
|---|---|---|
| `hasValue` | `bool` | `true` when the optional holds a value; `false` when it is `none`. |
| `value` | `T` | The held value. Accessing `value` when `hasValue` is `false` is undefined behavior. |

## The `none` Literal {#none}

The built-in keyword `none` represents the absent value.
`none` is implicitly convertible to any `Optional<T>`.

```slang
Optional<int> a = none;    // no value
bool absent = !a.hasValue; // true
```

## Implicit Coercions {#coercions}

Three implicit coercions are defined for `Optional<T>`:

### Value to Optional

Any value of type `T` is implicitly convertible to `Optional<T>`.
The resulting optional holds the value and `hasValue` is `true`.

```slang
int x = 42;
Optional<int> opt = x;   // opt.hasValue == true, opt.value == 42
```

This applies in function call arguments, return statements, and initializers.

### `none` to Optional

`none` is implicitly convertible to any `Optional<T>` with `hasValue == false`.

```slang
Optional<float> f = none;  // f.hasValue == false
```

### Optional to Optional coercion {#optional-coercion}

`Optional<T>` is implicitly convertible to `Optional<U>` when `T` is implicitly
convertible to `U`. The coercion preserves the `hasValue` state: if the source is
`none`, the result is `none`; if the source holds a value, the inner value is
converted from `T` to `U` and the result holds that converted value.

```slang
Optional<int>   src = 7;
Optional<float> dst = src;  // dst.hasValue == true, dst.value == 7.0

Optional<int>   empty = none;
Optional<float> emptyF = empty;  // emptyF.hasValue == false
```

This applies to all coercible inner types, including numeric promotions and
concrete-to-interface upcasts:

```slang
interface IShape { int area(); }
struct Square : IShape { int side; int area() { return side * side; } }

Optional<Square>  sq = Square{ 3 };
Optional<IShape>  sh = sq;  // sh.hasValue == true, sh.value.area() == 9
```

## Comparison with `none` {#comparison}

An `Optional<T>` value may be compared to `none` using `==` and `!=`:

```slang
Optional<int> opt = 5;
if (opt != none)
    printf("%d\n", opt.value);  // prints 5
```

This is equivalent to testing `opt.hasValue`.

## `if (let ...)` Syntax {#if-let}

The `if (let name = expr)` syntax unwraps an `Optional<T>` in a single step.
The body executes only when `expr` has a value; inside the body, `name` is
bound to the unwrapped value of type `T`.

```slang
Optional<int> getVal() { ... }

void example()
{
    if (let x = getVal())
    {
        // x has type int here
        printf("%d\n", x);
    }
}
```

## Memory Layout {#layout}

`Optional<T>` is lowered to a struct with two fields:

```slang
struct Optional<T> { T value; bool hasValue; }
```

The layout follows the same rules as a struct with those two members. The
`value` field is always present in memory regardless of `hasValue`; reading
`value` when `hasValue` is `false` is undefined behavior.

> 📝 **Remark:** The layout is target-specific and subject to standard struct
> alignment rules. Do not rely on a specific byte layout for serialization.

## Restrictions {#restrictions}

- `Optional<T>` cannot be used as the element type of a resource (e.g.,
  `StructuredBuffer<Optional<T>>` is not supported).
- Accessing `.value` when `.hasValue` is `false` is undefined behavior.
