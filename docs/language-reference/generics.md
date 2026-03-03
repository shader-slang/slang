# Generics

Generics in Slang enable parameterization of [structures](types-struct.md),
[interfaces](types-interface.md), [type aliases](types.md#alias), [functions and member functions](TODO.md),
[subscript operators](types-struct.md#subscript-op), and
[constructors](types-struct.md#constructor). A generic parameter can be a type, a [Boolean](types-fundamental.md#boolean)
value, an [integer](types-fundamental.md#integer) value, or an [enumeration (TODO)](TODO.md) value.
In addition, Slang supports [generic extension](types-extension.md#generic-struct), covered
in [type extensions](types-extension.md).

When the generic parameters are bound, a generic type or function is specialized. A specialized generic is a
concrete type or function, which can be used like any other concrete type or function. Generic parameters are
bound by providing arguments (explicit binding), by inference (implicit binding), or by a combination of both.
Value-typed arguments to the generic parameters must be [link-time constants (TODO)](TODO.md).
Conceptually, partial parameter binding can be done by defining a generic type alias for a generic type or
function, but this does not specialize the generic.

> 📝 **Remark 1:** Slang does not support explicit specialization of generics where a Slang program
> would provide a definition for a specific combination of arguments. However,
> [generic extension](types-extension.md#generic-struct) can be used to extend generic structures to
> similar effect.

> 📝 **Remark 2:** Slang does not currently support using interface-typed variables that require dynamic dispatch as
> generic parameters. See GitHub issue [#10263](https://github.com/shader-slang/slang/issues/10263).


## Syntax

Generic [structure](types-struct.md) syntax:
> **`'struct'`** [*`identifier`*] [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> **`'{'`** *`member-list`* **`'}'`**

Generic [interface](types-interface.md) syntax:
> **`'interface'`** *`identifier`* [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`bases-clause`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> **`'{'`** *`member-list`* **`'}'`**

Generic type alias syntax:
> **`'typealias'`** *`identifier`* [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'='`** *`simple-type-spec`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\* **`';'`**

Generic function and member function declaration (traditional syntax):
> *`simple-type-spec`* *`identifier`* [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'('`** *`param-list`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`';'`** | **`'{'`** *`body-stmt`*\*  **`'}'`**)

Generic function and member function declaration (modern syntax):
> **`'func'`** *`identifier`* [*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'('`** *`param-list`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'throws'`** *`simple-type-spec`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'->'`** *`simple-type-spec`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`';'`** | **`'{'`** *`body-stmt`*\*  **`'}'`**)

Generic [constructor](types-struct.md#constructor) declaration:
> **`'__init'`** [*`generic-params-decl`*] **`'('`** *`param-list`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`';'`** | **`'{'`** *`body-stmt`*\*  **`'}'`**)

Generic [subscript operator](types-struct.md#subscript-op) declaration:
> **`'__subscript'`** [*`generic-params-decl`*] **`'('`** *`param-list`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'->'`** *`simple-type-spec`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'where'`** *`where-clause`*)\*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`';'`** | **`'{'`** *`body-stmt`*\*  **`'}'`**)

Generic parameters declaration:
> *`generic-params-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'<'`** [*`generic-param-decl`* (**`','`** *`generic-param-decl`*)\* ] **`'>'`**
>
> *`generic-param-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`generic-value-param-decl`* |<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`generic-value-param-trad-decl`* |<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`generic-type-param-decl`* |<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`generic-type-param-pack-decl`*
>
> *`generic-value-param-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'let'`** *`identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`simple-type-spec`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'='`** *`init-expr`*]<br>
>
> *`generic-value-param-trad-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`simple-type-spec`* <br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'='`** *`init-expr`*]<br>
>
> *`generic-type-param-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'typename'`**] *`identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`simple-type-spec`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'='`** *`simple-type-spec`*]<br>
>
> *`generic-type-param-pack-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'each'`** *`identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`':'`** *`simple-type-spec`*]<br>


Generic parameter constraint clause:
> *`where-clause`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'optional'`**]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`simple-type-spec`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(*`generic-type-constraint-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|*`generic-type-constraint-eq-decl`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|*`generic-type-constraint-coercion-decl`*)<br>
>
> *`generic-type-constraint-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`':'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`simple-type-spec`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`','`** *`simple-type-spec`*)\*<br>
>
> *`generic-type-constraint-eq-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'=='`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`simple-type-spec`*<br>
>
> *`generic-type-constraint-coercion-decl`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'('`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`simple-type-spec`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[**`'implicit'`**]<br>

> 📝 **Remark:** Generic enumeration declarations are also supported by the parser. However, they are not
> currently useful. See GitHub issue [#10078](https://github.com/shader-slang/slang/issues/10078).

### Parameters

- *`generic-params-decl`* declares a list of generic parameters.
- *`generic-param-decl`* declares a generic value parameter, type parameter, or type parameter pack.
- *`generic-value-param-decl`* declares a generic value parameter.
- *`generic-value-param-trad-decl`* declares a generic value parameter using traditional syntax.
- *`generic-type-param-decl`* declares a generic type parameter.
- *`generic-type-param-pack-decl`* declares a generic type parameter pack.
- *`where-clause`* is a generic parameter constraint clause.
- *`generic-type-constraint-decl`* declares a generic type conformance constraint, requiring the left-hand-side
  type expression to conform to one or more constraining type expressions.
- *`generic-type-constraint-eq-decl`* declares a generic type equality constraint, requiring the left-hand-side
  type expression to be equal to the right-hand-side type expression.
- *`generic-type-constraint-coercion-decl`* declares a generic type coercion constraint, requiring the type
  expression in parentheses to be coercible to the type expression outside the parentheses.
  This constraint may be used only in [generic extensions](types-extension.md#generic-struct).
  See GitHub issue [#10087](https://github.com/shader-slang/slang/issues/10087).
- *`identifier`*: see the respective syntax for a description.
- *`bases-clause`*: see the respective syntax for a description.
- *`member-list`*: see the respective syntax for a description.
- *`simple-type-spec`*: see the respective syntax for a description.
- *`param-list`*: see the respective syntax for a description.
- *`body-stmt`*: see the respective syntax for a description.


## Description

A generic parameter declaration list *`generic-params-decl`* adds any number of parameters to structures,
interfaces, type aliases, functions, subscript operators, and constructors. These parameterized constructs are
called *generic structures*, *generic interfaces*, *generic type aliases*, *generic functions*, *generic
subscript operators*, and *generic constructors*.

A generic parameter declaration is one of:

- Generic value parameter declaration *`generic-value-param-decl`* or *`generic-value-param-trad-decl`*, which
  adds a value parameter with an optional default value. The value type must be a [Boolean](types-fundamental.md#boolean),
  an [integer](types-fundamental.md#integer), or an [enumeration (TODO)](TODO.md).
- Generic type parameter declaration *`generic-type-param-decl`*, which adds a type parameter with an optional
  type constraint and an optional default type. The keyword `typename` is optional.
- Generic type parameter pack declaration *`generic-type-param-pack-decl`*, which adds a type parameter
  pack. A type parameter pack is a variable-length list of types.

Types may be constrained by:

- Specifying an inline type constraint in *`generic-type-param-decl`* using the form
  `TypeParam : ConstrainingType`. This adds a single conformance requirement such that `TypeParam` must conform to
  `ConstrainingType`.
- Specifying one or more `where` clauses (*`where-clause`*). A `where` clause adds a single requirement using
  one of the following forms:
  - Conformance constraint declaration *`generic-type-constraint-decl`* adds a requirement that the left-hand-side type
    expression must conform to the right-hand-side type expressions.
  - Equivalence constraint declaration *`generic-type-constraint-eq-decl`* adds a requirement that the left-hand-side
    type expression must be equal to the right-hand-side type expression.
  - Coercion constraint declaration *`generic-type-constraint-coercion-decl`* adds a requirement that the parenthesized
    type expression must be coercible to the left-hand-side type expression.

Conformance and equivalence constraints may be declared as optional. When optional, the expression `ParamType is
ConstrainingType` returns `true` when `ParamType` conforms to or equals `ConstrainingType`. When the expression is used in
an `if` statement using the form `if (ParamType is ConstrainingType) { ... }`, then any variable of type `ParamType` may
be used as type `ConstrainingType` in the "then" branch.

The coercion requirement is usable only in generic extensions.

A constraint on a type parameter pack applies to every type in the pack.

Value parameters cannot be constrained.

> 📝 **Remark 1:** In Slang, a conformance requirement `TypeParam : ConstrainingType` means that `TypeParam` must
> have `ConstrainingType` as a base (either directly or transitively), and `ConstrainingType` must be an interface.

> 📝 **Remark 2:** Slang also has the `__generic` modifier, which can be used to declare generic parameters as
> an alternative to *`generic-params-decl`*. Using *`generic-params-decl`* is recommended.

> 📝 **Remark 3:** Optional conformance constraints are currently an experimental feature. See GitHub issues
> [#10078](https://github.com/shader-slang/slang/issues/10078) and
> [#10185](https://github.com/shader-slang/slang/issues/10185).


### Type Parameter Packs {#type-param-packs}

A type parameter pack is declared using the `each TypeIdentifier` syntax. When a generic construct is
specialized, a (possibly empty) sequence of type arguments is bound to the parameter pack.

A type parameter pack is expanded using the `expand`/`each` construct with the following syntax:

> Expand-expression:<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`expand-expr`* = **`'expand'`** *`expr`*

> Each-expression:<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`each-expr`* = **`'each'`** *`expr`*

An expression in an each-expression evaluates to a type parameter pack, tuple, or variable whose type is a
type parameter pack.

There must be at least one each-expression within an expand-expression. An each-expression must always be
enclosed within an expand-expression except in a generic type declaration. If there are multiple
each-expressions within an expand-expression, the referenced parameter packs must all have an equal number of
parameters.

An expand-expression evaluates to a comma-separated value sequence whose length is the number of type
parameters of the embedded each-expressions. Each element of the sequence is the expand expression with every
embedded each-expression replaced by the Nth element of its corresponding pack.

That is, `expand-expr` is substituted with the following sequence:

```hlsl
expr, // every each-expr is substituted by pack element 0
expr, // every each-expr is substituted by pack element 1
expr, // every each-expr is substituted by pack element 2
...
expr  // every each-expr is substituted by pack element N-1
```

In function parameter lists, expand/each constructs must come after all other parameters. There may be
multiple declared expand/each parameters, in which case the type parameter packs must have equal lengths.


## Type Checking

Type checking of parameterized types is performed based on their type constraints
before specialization. In general, an operation on a parameterized generic type or a generic-typed variable is legal if it is
legal for all possible concrete types conforming to the declared constraints.

The rules are as follows:
- If a parameterized type `T` has a type equality constraint `T == U`, type `T` is considered to be type `U`
  for all intents and purposes.
- If a parameterized type `T` has a type conformance constraint `T : U`, type `T` is considered to conform to
  `U`. That is, `T` implements all requirements of `U`.
- If a parameterized type `T` has a type constraint `U(T)`, type `T` may be converted to type `U`.

Type constraints may be declared for generic type parameters and type expressions that include generic type
parameters. For example, `where T : IFace where T.AssocT == int` requires that `T.AssocT` is `int`. Note that
`IFace` must declare associated type `AssocT`. (See [interfaces](types-interface.md) for associated type
declarations.)

No assumptions are made about generic value parameters other than their declared type.


> 📝 **Remark:** In contrast to C++ templates, type checking of Slang generics is performed before
> specialization. In C++, type checking is performed after template specialization and instantiation.


## Parameter Binding

Arguments to generic parameters can be bound explicitly, implicitly, or as a combination of both. Binding is
done at the call site.

In explicit binding, the arguments to the generic parameters are listed in angle brackets after the generic type or function
identifier. Explicit binding cannot be used in constructs that do not use a named identifier at call sites
(e.g., operator overloading).

In implicit binding, the arguments to the generic parameters are inferred. Inference is performed by matching the generic
parameters against the call site argument expressions. It is an error if an argument to a generic parameter cannot
be inferred from the call site.

If inference is ambiguous for a generic type parameter, the following rules are used to determine the type:
- If all inferred types are [fundamental scalar types](types-fundamental.md#scalar) or
  [vector types](types-vector-and-matrix.md) of the same length, the element type with the highest promotion rank is
  used. The promotion ranks from the lowest to the highest are: `int8_t`, `uint8_t`, `int16_t`,
  `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `float`, `double`.
  - A fundamental type is promoted to a 1-dimensional vector if necessary.
- In all other cases, an ambiguous generic type argument is an error.

It is an error when inference yields multiple options for a generic value argument.

Mixing explicit and implicit parameter binding is allowed. The leftmost generic parameters use the provided
generic arguments and the rest are inferred.

> 📝 **Remark:** If the generic argument inference is ambiguous and `bool` is inferred as a fundamental
> or element type, the behavior is currently undefined. See GitHub issue
> [#10164](https://github.com/shader-slang/slang/issues/10164) for details.


## Examples

### Generic structure with type and value parameters
```hlsl
RWStructuredBuffer<float> outputBuffer;

struct TestStruct<T, let size : uint>
{
    var arr : T[size];
}

[numthreads(10,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    TestStruct<float, 10> obj =
        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    outputBuffer[id.x] = obj.arr[id.x];
}
```

### Generic type alias for partial type binding
```hlsl
struct ArrayOfElements<T, let size : uint>
{
    typealias ElementType = T;
    ElementType elems[size];
}

typealias ArrayOfFiveElements<T> = ArrayOfElements<T, 5>;

RWStructuredBuffer<float> outputBuffer;

[numthreads(5,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    ArrayOfFiveElements<float> myArray =
        { 1.0, 2.0, 3.0, 4.0, 5.0 };

    outputBuffer[id.x] = myArray.elems[id.x];
}
```

### Generic function with type parameter
```hlsl
RWStructuredBuffer<float> outputBuffer;

// Note: IFunc<ReturnType, Param1Type, Param2Type, ...>
func performOp<T>(IFunc<T, T, T> binaryOp, T a, T b) -> T
{
    return binaryOp(a, b);
}

func add2<T : IArithmetic>(T a, T b) -> T
{
    return a + b;
}

struct Adder : IFunc<int, int, int>
{
    int bias;

    int operator() (int a, int b)
    {
        return add2(add2(a, b), bias);
    }
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    Adder addTwoWithBias = { 5 };
    outputBuffer[id.x] = performOp(addTwoWithBias, 234, 456);
}
```

### Generic constructor
```hlsl
RWStructuredBuffer<float> outputBuffer;

struct TestStruct
{
    int val;

    __init<T : IInteger>(T initval1, T initval2)
    {
        T tmp = initval1 + initval2;
        val = tmp.toInt();
    }
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    TestStruct obj = { 123, 345 };
    outputBuffer[id.x] = obj.val;
}
```

### Generic subscript
```hlsl
struct TestStruct
{
    var arr : float[10];

    // declare a 1-parameter subscript operator
    __subscript<T> (T i) -> float where T : IInteger
    {
        get { return arr[i.toInt()]; }
        set { arr[i.toInt()] = newValue; }
    }
}
```

### Type constraint
```hlsl
RWStructuredBuffer<float> outputBuffer;

func addTwo<T>(T a, T b) -> T where T : IArithmetic
{
    return a + b;
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x]  = addTwo(1, 2);
    outputBuffer[id.x] += addTwo(1.1, 2.2);
}
```

### Optional type constraint
```hlsl
RWStructuredBuffer<float> outputBuffer;

func addTwoIfInts<T>(T a, T b) -> T
    where T : IArithmetic       // T must be IArithmetic
    where optional T : IInteger // T may also be IInteger
{
    if (T is IInteger)
        return a + b; // return sum if T is IInteger
    else
        return a - a; // return 0 otherwise
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x]  = addTwoIfInts(1, 2);
    outputBuffer[id.x] += addTwoIfInts(1.1, 2.2);
}
```

### Type equality constraint for associated type
```hlsl
RWStructuredBuffer<float> outputBuffer;

interface IFace
{
    associatedtype PropertyType;
    property prop : PropertyType;
}

struct IntProperty : IFace
{
    typealias PropertyType = int;
    PropertyType prop;
}

struct FloatProperty : IFace
{
    typealias PropertyType = float;
    PropertyType prop;
}

int addTwoInts<T:IFace>(T a, T b) where T.PropertyType == int
{
    return a.prop + b.prop;
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    IntProperty intObj1 = { 1 };
    IntProperty intObj2 = { 2 };

    FloatProperty floatObj1 = { 1.0 };
    FloatProperty floatObj2 = { 2.0 };

    outputBuffer[id.x]  = addTwoInts(intObj1, intObj2);

    // FloatProperty does not satisfy the equivalence requirement
    // "T.PropertyType == int". Hence, the following line will
    // not compile.
    // outputBuffer[id.x] += addTwoInts(floatObj1, floatObj2);
}
```

### Type coercion constraint
```hlsl
struct Foo<A>
{
}

// adds method to the struct if A is convertible to int
extension<A> Foo<A> where int(A)
{
    int extensionMethod(int x)
    {
        return x + 42;
    }
}
```

### Type parameter pack
```hlsl
RWStructuredBuffer<float> outputBuffer;

void sumHelper(inout int acc, int term)
{
    acc += term;
}

int sumInts<each T>(expand each T terms)
    where T == int // every 'T' type pack member must be int
{
    int acc = 0;
    expand sumHelper(acc, each terms);

    // expands to:
    //
    // sumHelper(acc, term0),
    // sumHelper(acc, term1),
    // ...

    return acc;
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x] += sumInts(1, 2, 3, 4, 5, 6, 7);
}
```

### Multiple function parameter packs
```hlsl
void dotProductHelper(float a, float b, inout float ret)
{
    ret += a * b;
}

float dotProduct<each T>
    (expand each T a, expand each T b)
    where T == float
{
    float r = 0.0f;

    expand dotProductHelper(each a, each b, r);
    return r;
}

RWStructuredBuffer<double> outputBuffer;

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float a[3] = { 4, 5, 6 };
    float b[3] = { 2, 1, 0 };

    outputBuffer[0] =
        dotProduct(
            a[0], a[1], a[2],
            b[0], b[1], b[2]);
}
```

### Generic extension for generic types
```hlsl
struct GenericStruct<T>
{
}

extension<T> GenericStruct<T> where T : IFloat
{
    int isInt() { return 0; }
}

extension<T> GenericStruct<T> where T : IInteger
{
    int isInt() { return 1; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    GenericStruct<float> floatObj = { };
    GenericStruct<uint> uintObj = { };

    outputBuffer[0] = floatObj.isInt();
    outputBuffer[1] = uintObj.isInt();
}
```
> 📝 **Remark:** An extension cannot currently be used to override a more generic implementation.
> See GitHub issue [#10146](https://github.com/shader-slang/slang/issues/10146).

### Explicit and implicit generic arguments

```hlsl
// Return type Ret is listed first, since it cannot be
// inferred.
func diffSingle<Ret : IInteger, T : IInteger, U : IInteger>
    (T a, U b) -> Ret
{
    return Ret(a.toInt64() - b.toInt64());
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    int8_t a = 3;
    int16_t b = 5;

    // Return type Ret is bound explicitly. Parameter
    // types T and U can be inferred from function
    // arguments
    outputBuffer[0] = diffSingle<int32_t>(a, b);
}
```

### Implicit parameter binding for type and value
```hlsl
// Note: assumes N >= 1
ElementType sumElements<ElementType : IArithmetic, let N : uint>
    (ElementType arr[N])
{
    ElementType acc = arr[0];

    for (uint i = 1; i < N; ++i)
        acc = acc + arr[i];

    return acc;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    int elements[7] = { 1,2,3,4,5,6,7 };

    // generic parameters ElementType and N are bound implicitly
    outputBuffer[0] = sumElements(elements);
}
```

### Implicit parameter binding, ambiguous value argument
```hlsl
uint len<let N : uint>(int[N] arr, int[N] arr2)
{
    return N;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    int arr1[7] = { };
    int arr2[6] = { };

    // error: generic argument N cannot be inferred
    outputBuffer[0] = len(arr1, arr2);
}
```

### Implicit parameter binding, ambiguous type argument
```hlsl
interface IBase
{
}

struct A : IBase
{
}

struct B : IBase
{
}

void testFunc<T>(T x, T y)
{
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    A a = { };
    B b = { };

    // Explicit parameter type binding must be used,
    // since inferred type for T is ambiguous and
    // non-fundamental.
    testFunc<IBase>(a, b);
}
```
