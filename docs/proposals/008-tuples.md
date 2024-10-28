SP #008 - Tuples
==============

Now that we have variadic generics in the language following [SP #007], we should now be able to support `Tuple` type as a core language feature.
`Tuple` types are useful in many places to reduce boilerplate in user code, such as in function return types to eliminate the need of defining
`struct`s that are used only for invoking the function. Adding `Tuple` types to Slang will also simplify interop with other languages such as Python
and C++ that have tuple types.

Status
------

Author: Yong He

Status: Implemented.

Implementation: [PR 4856](https://github.com/shader-slang/slang/pull/4856).

Reviewed by: Jay Kwak, Kai Zhang, Ariel Glasroth.

Background
----------

Tuple type is widely supported in almost all of the modern programming languages including C++, C#, Swift, Rust, Python. Supporting tuple types
in Slang will bring the language to parity with other languages and allow users to practice the same coding idioms in Slang, and allow Slang code
to interop more directly with other parts of the user application written in other languages.


Proposed Approach
-----------------

With variadic generics support, we can now easily define a Tuple type in the core module as:
```
__generic<each T>
__magic_type(TupleType)
struct Tuple
{
    __intrinsic_op($(kIROp_MakeTuple))
    __init(expand each T);
}
```

This will allow users to instantiate tuple types from their code with `Tuple<T0, T1, T2>(v0, v1, v2)`.

### Constructing Tuple Values

To make it easy to construct tuples, we will define a `makeTuple` function in the core module as:
```
__intrinsic_op($(kIROp_MakeTuple))
Tuple<expand each T> makeTuple(expand each T values);
```

With generic argument inferencing, this will enable user to write:
```
makeTuple(1, 2.0f) // returns Tuple<int, float>(1, 2.0f)
```

### Accessing Tuple Elements

We can extend the logic of vector element accessing to access tuple elements. Given `t` as a tuple, these expressions are valid:
```
t._0 // Access the first element
t._1 // Access the second element
```

### Swizzling

We can easily support tuple swizzles:
```
let t = Tuple<int, float>(1, 2.0f);
let v = t._1_0;
// v == Tuple<float, int>(2.0f, 1)
```

### Concatenation

We can define tuple concatenation operation in the core module as:
```
Tuple<expand each T, expand each U> concat<each T, each U>(Tuple<expand each T> first, Tuple<expand each U> second)
{
    return makeTuple<expand each T, expand each U>(expand each first, expand each second);
}
```


### Counting

The `countof` expression can be used on type packs or tuple values to obtain the number of elements in a type pack or tuple.
And this result should be usable as a compile-time constant such as in a generic argument.

```
int bar<let n : int>()
{
}
int foo<each T>()
{
    bar<countof T>(); // OK, countof T is a compile time constant.
    
    Tuple<expand each T> t;
    let c = countof t; // OK, countof can be used on tuple values.
}
```

### Operator Overloads

We should have builtin operator overloads for all comparison operators if every element type of a tuple conforms to `IComparable`.
This can be supported by defining an overload for these operators in the core module in the form of:
```
bool assign(inout bool r, bool v) { r = v;  return v; }

__generic<each T : IComparable>
bool operator < (Tuple<T> t0, Tuple<T> t1)
{
    bool greater = false;
    bool equals = true;
    expand greater || assign(equals, equals && (each t0) == (each t1)) && assign(greater, (each t0) > (each t1));
    return !greater && !equals;
}
```


Alternatives Considered
----------------

Should we allow other operator overloads for tuples? This seems useful to have, but right now this is a bit tricky
because we haven't really settled on builtin interfaces. We need to finalize things like `IFloat`, `IInteger`,
`IArithmetic`, `ILogic` etc. first.

Should we automatically treat `Tuple` type to conform to any interface `IFoo` if every element in the tuple conforms to
`IFoo`? We can't because this is not well-defined. For example, if `IFoo` has a method that returns `int`,
should the tuple type's equivalent method return `Tuple<expand(int, T)>` or just `int`? In some cases you want one but
other times you want the other. And if the method returns a tuple, it is no longer consistent with the base interface
definition so this is all ill-formed.

We also considered having an overload of `concat` that appends individual elements to the end of a tuple, such as:
```
Tuple<T, U> concat<each T, each U>(Tuple<T> t, each U values);
```
However, this could lead to surprising behavior when the user writes `concat(t0, t1, t2)` where t1 and t2 are also tuples.
Having this overload means the result would be `(t0_0, t0_1, ... t0_n, t1, t2)` where the user could be expecting `t1` and `t2`
to be flattened into the resulting tuple. To avoid this surprising behavior, we decide to not include this overload in the core module.
