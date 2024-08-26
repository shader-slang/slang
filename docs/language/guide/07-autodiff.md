---
layout: user-guide
permalink: /user-guide/autodiff
---

# Automatic Differentiation

Neural networks and other machine learning techniques are becoming an increasingly popular way to solve many difficult problems in modern visual computing systems. However, to take advantage of these techniques, developers often need to reimplement many existing system components in a differentiable form to allow computing the derivatives of a function, or to propagate the derivative of a result backwards to each parameter. Slang provides built-in auto differentiation features to support developers adding differentiability to their existing code with as little effort as possible. In this chapter, we provide an overview of the auto differentiation features, followed by a detailed description on the new syntax and rules.

## Using Automatic Differentiation in Slang

In this section, we walk through the steps to compute forward-derivative from input, and backward propagate the derivative from output to input.

### Forward Differentiation

Suppose the user has already written a function that computes some mathematic term:

```csharp
float myFunc(float a, float x)
{
    return a * x * x;
}
```

The user can make this function *forward-differentiable* by adding a `[ForwardDerivative]` attribute:
```csharp
[ForwardDifferentiable]
float myFunc(float a, float x)
{
    return a * x * x;
}
```

This allows the function to be used in the `fwd_diff` operator, which is a higher order operation that takes in a forward-differentiable function and returns the forward-derivative of the function.

The expression `fwd_diff(myFunc)` will have the following signature:
```csharp
DifferentialPair<float> myFunc_fwd_derivative(DifferentialPair<float> a, DifferentialPair<float> x);
```

Where `DifferentialPair<T>` is a built-in type that encodes both the primal(original) value and the derivative value of a term.
To use this function to compute the derivative of `myFunc` with regard to `x`, the user can call the forward-derivative function by supplying the derivative value of `x` with `1.0` and the derivative value of `a` with `0.0`, as in the following code:

```csharp
float a = 2.0;
float x = 3.0;
// Compute derivative with regard to `x`:
let result = fwd_diff(myFunc)(diffPair(a, 0.0), diffPair(x, 1.0));
// Print the derivative.
printf("%f", result.d);

// Output: 12.0
```

In the example code above, `diffPair()` is a built-in function to construct a value of `DifferentialPair<T>` with a primal value and a derivative value. The primal value and derivative value stored in a `DifferentialPair` can be accessed with the `.p` and a `.d` property.

### Backward Propagation

The forward derivative function allows the user to compute the derivative of a function with regard to a specific combination of input parameters at a time. In many cases, we need to know how each parameter affects the output. Instead of calling the forward derivative function once for each parameter, it is more efficient to call the *backward propagation* function that propagate the derivative of outputs to each input parameter.

To allow the compiler to generate the backward propagation function, we simply mark our function with the `[Differentiable]` or `[BackwardDifferentiable]` attribute:
```csharp
[Differentiable]
float myFunc(float a, float x)
{
    return a * x * x;
}
```

> #### Note:
> When a function is marked as `[Differentiable]`, it is implied that the function is both `[ForwardDifferentiable]` and `[BackwardDifferentiable]` and can be used in the `fwd_diff` operator.


The `bwd_diff` operator applies to a backward differentiable function and returns the backward propagation function. In this case, `bwd_diff(myFunc)` will have the following signature:

```csharp
void myFunc_backProp(inout DifferentialPair<float> a, inout DifferentialPair<float> x, float dResult);
```

Where `a` is an `inout DifferentialPair` where the initial value of `a` is passed into the function as primal value (in the `.p` property), and the propagated derivative of `a` is returned via the `.d` property of the `DifferentialPair`. The same rules apply to `x`.

The additional `dResult` parameter is the derivative of the return value to be propagated to the input parameters. Note that in a backward propagation function, an input will become a `inout DifferentialPair` where the `.d` property of the pair is intended for receiving the propagation result, and the return value will become an input parameter that represents the source of backward propagation.

The backward propagation function can be called as in the following code:
```csharp
var a = diffPair(2.0); // constructs DifferentialPair{2.0, 0.0}
var x = diffPair(3.0); // constructs DifferentialPair{3.0, 0.0}

bwd_diff(myFunc)(a, x, 1.0);

// a.d is now 9.0
// x.d is now 12.0
```

This completes the walkthrough of automatic differentiation features. The following sections will cover each perspective of the auto differentiation feature in more detail.

## Mathematic Concepts and Terminologies

This section briefly reviews the mathematic theories behind differentiable programming with the intention to clarify the concepts and terminologies that will be used in the rest of this documentation. We assume the reader is already familiar with the basic theories behind neural network training, in particular the back-propagation algorithm.

A differentiable system can be represented a composition of differentiable functions (kernels) with learnable parameters, where each differentiable function has the form:

$$\mathbf{w}_{i+1} = f_i(\mathbf{w}_i) $$

Where $$f_i$$ represents a differentiable function (kernel) in the system, $$\mathbf{w}$$ represents a collection of learnable parameters defined in function $$f_i$$, and $$\mathbf{w}_{i+1}$$ is the output of $$f_i$$. We will use $$\omega$$ to denote a specific parameter in $$\mathbf{w}$$.

In a composed system, the value of $$\mathbf{w}$$ used to evaluate $$f_i$$ may come from an *upstream* function

$$ \mathbf{w}_i = f_{i-1}(\mathbf{w}_{i-1}) $$

Similarly, the value computed by $$f_i$$ may be used as argument to a *downstream* function

$$ h = f_{i+1}(\mathbf{w}_{i+1}) = f_{i+1}(f_{i}(\mathbf{w}_{i}))$$

The entire system composed from differentiable functions can be noted as

$$Y = f_1 \circ f_2 \circ \cdots \circ f_n(\mathbf{w}_0)$$

Where $$\mathbf{w}_0$$ is the first layer of parameters.

### Forward Propagation of Derivatives
When developing and training such a system, we often need to evaluate the partial derivative of a differentiable function with regard to some parameter $$\omega$$. The simplest way to obtain a partial derivative is to call a forward derivative propagation function, which is defined by:

$$ \mathbb{F}[f_i] = f_i'(\mathbf{w}_i, \mathbf{w}_i') = \sum_{\omega_i\in\mathbf{w}_i} \frac{\partial f}{\partial \omega_i} \omega_i' $$

Where $$\omega' \in \mathbf{w}'$$ represents the partial derivative of $$\omega_i$$ with regard to some upstream parameter $$\omega_{i-1}$$ that is used to compute $$\omega_i$$, i.e. $$\omega'=\frac{\partial \omega_{i}}{\partial \omega_{i-1}}$$.

Given this definition, $$\mathbb{F}[f]$$ can be used as a forward propagation function that is able to compute $$\frac{\partial f_i}{\partial \omega_0}$$ from $$\frac{\partial \omega_{i-1}}{\partial \omega_0}$$.

### Backward Propagation of Derivatives
When using the backpropagation algorithm to train a neural network, we are more interested in figuring out the partial derivative of the final system output with regard to a parameter $$\omega_i$$ in $$f_i$$. To do so, we generally utilize the backward derivative propagation function

$$\mathbb{B}[f_i] = f_i^{-1}(\frac{\partial Y}{\partial f_i}) = \frac{\partial Y}{\partial \mathbf{w}_i}$$

Where the backward propagation function $$\mathbb{B}[f_i]$$ takes as input the partial derivative of the final system output $$Y$$ with regard to the output of $$f_i$$ (i.e. $$\mathbf{w}_i$$), and computes the partial derivative of the final system output with regard to the input of $$f_i$$ (i.e. $$\mathbf{w}_{i-1}$$).

The higher order operator $$\mathbb{F}$$ and $$\mathbb{B}$$ represent the operations that converts an original or primal function $$f$$ to its forward or backward derivative propagation function. Slang's automatic differentiation feature provide built-in support for these operators to automatically generate the derivative propagation functions from a user defined primal function. The remaining documentation will discuss this feature from a programming language perspective.

## Differentiable Types
Slang will only generate differentiation code for values that has a *differentiable* type. A type is differentiable if it conforms to the built-in `IDifferentiable` interface. The definition of the `IDifferentiable` interface is:
```csharp
interface IDifferentiable
{
    associatedtype Differential : IDifferentiable
        where Differential.Differential == Differential;

    static Differential dzero();

    static Differential dadd(Differential, Differential);

    static Differential dmul(This, Differential);
}
```
As defined by the `IDifferentiable` interface, a differentiable type must have a `Differential` associated type that stores the derivative of the value. A further requirement is that the type of the second-order derivative must be the same `Differential` type. In another word, given a type `T`, `T.Differential` can be different from `T`, but `T.Differential.Differential` must equal to `T.Differential`.

In addition, a differentiable type must define the `zero` value of its derivative, and how to add and multiply derivative values.

### Builtin Differentiable Types
The following built-in types are differentiable: 
- Scalars: `float`, `double` and `half`.
- Vector/Matrix: `vector` and `matrix` of `float`, `double` and `half` types.
- Arrays: `T[n]` is differentiable if `T` is differentiable.

### User Defined Differentiable Types

The user can make any `struct` types differentiable by implementing the `IDifferentiable` interface on the type. The requirements from `IDifferentiable` interface can be fulfilled automatically or manually.

#### Automatic Fulfillment of `IDifferentiable` Requirements
Assume the user has defined the following type:

```csharp
struct MyRay
{
    float3 origin;
    float3 dir;
    int nonDifferentiablePayload;
}
```

The type can be made differentiable by adding `IDifferentiable` conformance:
```csharp
struct MyRay : IDifferentiable
{
    float3 origin;
    float3 dir;
    int nonDifferentiablePayload;
}
```

Note that this code does not provide any explicit implementation of the `IDifferentiable` requirements. In this case the compiler will automatically synthesize all the requirements. This should provide the desired behavior most of the time. The procedure for synthesizing the interface implementation is as follows:
1. A new type is generated that stores the `Differential` of all differentiable fields. This new type itself will conform to the `IDifferentiable` interface, and it will be used to satisfy the `Differential` associated type requirement.
2. Each differential field will be associated to its corresponding field in the newly synthesized `Differential` type.
3. The `zero` value of the differential type is made from the `zero` value of each field in the differential type.
4. The `dadd` and `dmul` methods simply perform `dadd` and `dmul` operations on each field.
5. If the synthesized `Differential` type contains exactly the same fields as the original type, and the type of each field is the same as the original field type, then the original type itself will be used as the `Differential` type instead of creating a new type to satisfy the `Differential` associated type requirement. This means that all the synthesized `Differential` type use itself to meet its own `IDifferentiable` requirements.

#### Manual Fulfillment of `IDifferentiable` Requirements

In rare cases where more control is desired, the user can manually provide the implementation. To do so, we will first define the `Differential` type for `MyRay`, and use it to fulfill the `Differential` requirement in `MyRay`:

```csharp
struct MyRayDifferential
{
    float3 d_origin;
    float3 d_dir;
}

struct MyRay : IDifferentiable
{
    // Specify that `MyRay.Differential` is `MyRayDifferential`.
    typealias Differential = MyRayDifferential;

    // Specify that the derivative for `origin` will be stored in `MayRayDifferential.d_origin`.
    [DerivativeMember(MayRayDifferential.d_origin)]
    float3 origin;

    // Specify that the derivative for `dir` will be stored in `MayRayDifferential.d_dir`.
    [DerivativeMember(MayRayDifferential.d_dir)]
    float3 dir;

    // This is a non-differentiable field so we don't put any attributes on it.
    int nonDifferentiablePayload;

    // Define zero derivative.
    static MyRayDifferential dzero()
    {
        return {float3(0.0), float3(0.0)};
    }

    // Define the add operation of two derivatives.
    static MyRayDifferential dadd(MyRayDifferential v1, MyRayDifferential v2)
    {
        MyRayDifferential result;
        result.d_origin = v1.d_origin + v2.d_origin;
        result.d_dir = v1.d_dir + v2.d_dir;
        return result;
    }

    // Define the multiply operation of a primal value and a derivative value.
    static MyRayDifferential dmul(MyRay p, MyRayDifferential d)
    {
        MyRayDifferential result;
        result.d_origin = p.origin * d.d_origin;
        result.d_dir = p.dir * d.d_dir;
        return result;
    }
}
```

Note that for each struct field that is differentiable, we need to use the `[DerivativeMember]` attribute to associate it with the corresponding field in the `Differential` type, so the compiler knows how to access the derivative for the field.

However, there is still a missing piece in the above code: we also need to make `MyRayDifferential` conform to `IDifferentiable` because it is required that the `Differential` of a type must itself be `Differential`. Again we can use automatic fulfillment by simply adding `IDifferentiable` conformance to `MyRayDifferential`:
```csharp
struct MyRayDifferential : IDifferentiable
{
    float3 d_origin;
    float3 d_dir;
}
```
In this case, since all fields in `MyRayDifferential` are differentiable, and the `Differential` of each field is the same as the original type of each field (i.e. `float3.Differential == float3` as defined in built-in library), the compiler will automatically use the type itself as its own `Differential`, making `MyRayDifferential` suitable for use as `Differential` of `MyRay`.

We can also choose to manually implement `IDifferentiable` interface for `MyRayDifferential` as in the following code:

```csharp
struct MyRayDifferential : IDifferentiable
{
    typealias Differential = MyRayDifferential;

    [DerivativeMember(MyRayDifferential.d_origin)]
    float3 d_origin;

    [DerivativeMember(MyRayDifferential.d_dir)]
    float3 d_dir;

    static MyRayDifferential dzero()
    {
        return {float3(0.0), float3(0.0)};
    }

    static MyRayDifferential dadd(MyRayDifferential v1, MyRayDifferential v2)
    {
        MyRayDifferential result;
        result.d_origin = v1.d_origin + v2.d_origin;
        result.d_dir = v1.d_dir + v2.d_dir;
        return result;
    }

    static MyRayDifferential dmul(MyRayDifferential p, MyRayDifferential d)
    {
        MyRayDifferential result;
        result.d_origin = p.d_origin * d.d_origin;
        result.d_dir = p.d_dir * d.d_dir;
        return result;
    }
}
```
In this specific case, the automatically generated `IDifferentiable` implementation will be exactly the same as the manually written code listed above.


## Forward Derivative Propagation Function

Functions in Slang can be marked as forward-differentiable or backward-differentiable. The `fwd_diff` operator can be used on a forward-differentiable function to obtain the forward derivative propagation function. Likewise, the `bwd_diff` operator can be used on a backward-differentiable function to obtain the backward derivative propagation function. This and the next sections cover the semantics of forward and backward propagation functions, and different ways to make a function forward and backward differentiable. 

A forward derivative propagation function computes the derivative of the result value with regard to a specific set of input parameters. 
Given an original function, the signature of its forward propagation function is determined using the following rules:
- If the return type `R` is differentiable, the forward propagation function will return `DifferentialPair<R>` that consists of both the computed original result value and the (partial) derivative of the result value. Otherwise, the return type is kept unmodified as `R`.
- If a parameter has type `T` that is differentiable, it will be translated into a `DifferentialPair<T>` parameter in the derivative function, where the differential component of the `DifferentialPair` holds the initial derivatives of each parameter with regard to their upstream parameters.
- All parameter directions are unchanged. For example, an `out` parameter in the original function will remain an `out` parameter in the derivative function.

For example, given original function:
```csharp
R original(T0 p0, inout T1 p1, T2 p2);
```
Where `R`, `T0`, and `T1` is differentiable and `T2` is non-differentiable, the forward derivative function will have the following signature:
```csharp
DifferentialPair<R> derivative(DifferentialPair<T0> p0, inout DifferentialPair<T1> p1, T2 p2);
```

This forward propagation function takes the initial primal value of `p0` in `p0.p`, and the partial derivative of `p0` with regard to some upstream parameter in `p0.d`. It takes the initial primal and derivative values of `p1` and updates `p1` to hold the newly computed value and propagated derivative. Since `p2` is not differentiable, it remains unchanged.

`DifferentialPair<T>` is a built-in type that carries both the original and derivative value of a term. It is defined as follows:
```csharp
struct DifferentialPair<T : IDifferentiable> : IDifferentiable
{
    typealias Differential = DifferentialPair<T.Differential>;
    property T p {get;}
    property T.Differential d {get;}
    static Differential dzero();
    static Differential dadd(Differential a, Differential b);
    static Differential dmul(This a, Differential b);
}
```

### Automatic Implementation of Forward Derivative Functions

A function can be made forward-differentiable with a `[ForwardDifferentiable]` attribute. This attribute will cause the compiler to automatically implement the forward propagation function. The syntax for using `[ForwardDifferentiable]` is:

```csharp
[ForwardDifferentiable]
R original(T0 p0, inout T1, p1, T2 p2);
```

Once the function is made forward-differentiable, the forward propagation function can then be called with the `fwd_diff` operator:
```csharp
DifferentialPair<R> result = fwd_diff(original)(...);
```

### User Defined Forward Derivative Functions
As an alternative to compiler-implemented forward derivatives, the user can choose to manually provide a derivative implementation to make an existing function forward-differentiable. The `[ForwardDerivative(derivative_func)]` attribute is used to associate a function with its forward derivative propagation implementation. The syntax for using `[ForwardDerivative]` attribute is:
```csharp
DifferentialPair<R> derivative(DifferentialPair<T0> p0, inout DifferentialPair<T1> p1, T2 p2)
{
    ....
}

[ForwardDerivative(derivative)]
R original(T0 p0, inout T1, p1, T2 p2);
```
If `derivative` is defined in a different scope from `original`, such as in a different namespace or `struct` type, a fully qualified name is required. For example:
```csharp
struct MyType
{
    // Implementing derivative function in a different name scope.
    static DifferentialPair<R> derivative(DifferentialPair<T0> p0, inout DifferentialPair<T1> p1, T2 p2)
    {
        ....
    }
}

// Use fully qualified name in the attribute.
[ForwardDerivative(MyType.derivative)]
R original(T0 p0, inout T1, p1, T2 p2);
```

Sometimes the derivative function needs to be defined in a different module from the original function, or the derivative function cannot be made visible from the original function. In this case, we can use the `[ForwardDerivativeOf(originalFunnc)]` attribute to inform the compiler that `originalFunc` should be treated as a forward-differentiable function, and the current function is the derivative implementation of `originalFunc`. The following code will have the same effect to associate `derivative` and the forward-derivative implementation of `original`:

```csharp
R original(T0 p0, inout T1, p1, T2 p2);

[ForwardDerivativeOf(original)]
DifferentialPair<R> derivative(DifferentialPair<T0> p0, inout DifferentialPair<T1> p1, T2 p2)
{
    ....
}
```

## Backward Derivative Propagation Function

A backward derivative propagation function propagates the derivative of the function output to all the input parameters simultaneously.

Given an original function `f`, the general rule for determining the signature of its backward propagation function is that a differentiable output `o` becomes an input parameter holding the partial derivative of a downstream output with regard to the differentiable output, i.e. $$\partial y/\partial o$$); an input differentiable parameter `i` in the original function will become an output in the backward propagation function, holding the propagated partial derivative $$\partial y/\partial i$$; and any non-differentiable outputs are dropped from the backward propagation function. This means that the backward propagation function never returns any values computed in the original function.

More specifically, the signature of its backward propagation function is determined using the following rules:
- A backward propagation function always returns `void`.
- A differentiable `in` parameter of type `T` will become an `inout DifferentialPair<T>` parameter, where the original value part of the differential pair contains the original value of the parameter to pass into the back-prop function. The original value will not be overwritten by the backward propagation function. The propagated derivative will be written to the derivative part of the differential pair after the backward propagation function returns. The initial derivative value of the pair is ignored as input.
- A differentiable `out` parameter of type `T` will become an `in T.Differential` parameter, carrying the partial derivative of some downstream term with regard to the return value.
- A differentiable `inout` parameter of type `T` will become an `inout DifferentialPair<T>` parameter, where the original value of the argument, along with the downstream partial derivative with regard to the argument is passed as input to the backward propagation function as the original and derivative part of the pair. The propagated derivative with regard to this input parameter will be written back and replace the derivative part of the pair. The primal value part of the parameter will *not* be updated.
- A differentiable return value of type `R` will become an additional `in R.Differential` parameter at the end of the backward propagation function parameter list, carrying the result derivative of a downstream term with regard to the return value of the original function.
- A non-differentiable return value of type `NDR` will be dropped.
- A non-differentiable `in` parameter of type `ND` will remain unchanged in the backward propagation function.
- A non-differentiable `out` parameter of type `ND` will be removed from the parameter list of the backward propagation function.
- A non-differentiable `inout` parameter of type `ND` will become an `in ND` parameter.

For example consider the following original function:
```csharp
struct T : IDifferentiable {...}
struct R : IDifferentiable {...}
struct ND {} // Non differentiable

[Differentiable]
R original(T p0, out T p1, inout T p2, ND p3, out ND p4, inout ND p5);
```
The signature of its backward propagation function is:
```csharp
void back_prop(
    inout DifferentialPair<T> p0,
    T.Differential p1,
    inout DifferentialPair<T> p2,
    ND p3,
    ND p5,
    R.Differential dResult);
```
Note that although `p2` is still `inout` in the backward propagation function, the backward propagation function will only write propagated derivative to `p2.d` and will not modify `p2.p`.

### Automatically Implemented Backward Propagation Functions

A function can be made backward-differentiable with a `[Differentiable]` or `[BackwardDifferentiable]` attribute. This attribute will cause the compiler to automatically implement the backward propagation function. The syntax for using `[Differentiable]` is:

```csharp
[Differentiable]
R original(T0 p0, inout T1, p1, T2 p2);
```

Once the function is made backward-differentiable, the backward propagation function can then be called with the `bwd_diff` operator:
```csharp
bwd_diff(original)(...);
```

### User Defined Backward Propagation Functions
Similar to user-defined forward derivative functions, the `[BackwardDerivative]` and `[BackwardDerivativeOf]` attributes can be used to supply a function with user defined backward propagation function.

The syntax for using `[BackwardDerivative]` attribute is:
```csharp
void back_prop(
    inout DifferentialPair<T> p0,
    T1.Differential p1,
    inout DifferentialPair<T> p2,
    ND p3,
    ND p5,
    R.Differential dResult)
{
    ...
}

[BackwardDerivative(back_prop)]
R original(T0 p0, inout T1, p1, T2 p2);
```

Similarly, the `[BackwardDerivativeOf]` attribute can be used on the back-prop function in case it is not convenient to modify the definition of the original function, or the back-prop function can't be made visible from the original function:

```csharp
R original(T0 p0, inout T1, p1, T2 p2);

[BackwardDerivativeOf(original)]
void back_prop(
    inout DifferentialPair<T> p0,
    T1.Differential p1,
    inout DifferentialPair<T> p2,
    ND p3,
    ND p5,
    R.Differential dResult)
{
    ...
}
```

## Builtin Differentiable Functions

The following built-in functions are backward differentiable and both their forward-derivative and backward-propagation functions are already defined in the built-in library:

- Arithmetic functions: `abs`, `max`, `min`, `sqrt`, `rcp`, `rsqrt`, `fma`, `mad`, `fmod`, `frac`, `radians`, `degrees`
- Interpolation and clamping functions: `lerp`, `smoothstep`, `clamp`, `saturate`
- Trigonometric functions: `sin`, `cos`, `sincos`, `tan`, `asin`, `acos`, `atan`, `atan2`
- Hyperbolic functions: `sinh`, `cosh`, `tanh`
- Exponential and logarithmic functions: `exp`, `exp2`, `pow`, `log`, `log2`, `log10`
- Vector functions: `dot`, `cross`, `length`, `distance`, `normalize`, `reflect`, `refract`
- Matrix transforms: `mul(matrix, vector)`, `mul(vector, matrix)`, `mul(matrix, matrix)`
- Matrix operations: `transpose`, `determinant`
- Legacy blending and lighting intrinsics: `dst`, `lit`

## Primal Substitute Functions

Sometimes it is desirable to replace a function with another when generating forward or backward derivative propagation code. For example, the following code shows a function that computes the integral of some term by sampling and we want to use a different sampling stragegy when computing the derivatives.
```csharp
float myTerm(float x)
{
     return someComplexComputation(x);
}

float getSample(float a, float b) { ... }

[Differentiable]
float computeIntegralOverMyTerm(float x, float a, float b)
{
     float sum = 0.0;
     for (int i = 0; i < SAMPLE_COUNT; i++)
     {
          let s = no_diff getSample(a, b);
          let y = myTerm(s);
          sum += y * ((b-a)/SAMPLE_COUNT);
     }
     return sum;
}
```

In this code, the `getSample` function returns a random sample in the range of `[a,b]`. Assume we have another sampling function `getSampleForDerivativeComputation(a,b)` that we wish to use instead in derivative computation, we can do so by marking it as a primal-substitute of `getSample`, as in the following code:
```csharp
[PrimalSubstituteOf(getSample)]
float getSampleForDerivativeComputation(float a, float b)
{
     ...
}
```

Here, the `[PrimalSubstituteOf(getSample)]` attributes marks the `getSampleForDerivativeComputation` function as the substitute for `getSample` in derivative propagation functions. When a function has a primal substitute, the compiler will treat all calls to that function as if it is a call to the substitute function when generating derivative code. Note that this only applies to compiler generated derivative function and does not affect user provided derivative functions. If a user provided derivative function calls `getSample`, it will not be replaced by `getSampleForDerivativeComputation` by the compiler.

Similar to `[ForwardDerivative]` and `[ForwardDerivativeOf]` attributes, The `[PrimalSubsitute(substFunc)]` attribute works the other way around: it specifies the primal substitute function of the function being marked.

Primal substitute can be used as another way to make a function differentiable. A function is considered differentiable if it has a primal substitute that is differentiable. The following code illustrates this mechanism.
```csharp
float myFunc(float x) {...}

[PrimalSubstituteOf(myFunc)]
[Differentiable]
float myFuncSubst(float x) {...}

// myFunc is now considered backward differentiable.
```

The following example shows in more detail on how primal substitute affects derivative computation.
```csharp
float myFunc(float x) { return x*x; }

[PrimalSubstituteOf(myFunc)]
[ForwardDifferentiable]
float myFuncSubst(float x) { return x*x*x; }

[ForwardDifferentiable]
float caller(float x) { return myFunc(x); }

let a = caller(4.0); // a == 16.0 (calling myFunc)
let b = fwd_diff(caller)(diffPair(4.0, 1.0)).p; // b == 64.0 (calling myFuncSubst)
let c = fwd_diff(caller)(diffPair(4.0, 1.0)).d; // c == 48.0 (calling derivative of myFuncSubst)
```

In case that a function has both custom defined derivatives and a differentiable primal substitute, the primal substitute overrides the custom defined derivative on the original function. All calls to the original function will be translated into calls to the primal substitute first, and differentiation step follows after. This means that the derivatives of the primal substitute function will be used instead of the derivatives defined on the original function.

## Working with Mixed Differentiable and Non-Differentiable Code

Introducing differentiability to an existing system often involves dealing with code that mixes differentiable and non-differentiable logic.
Slang provides type checking and code analysis features to allow users to clarify the intention and guard against unexpected behaviors involving when to propagate derivatives through operations.

### Excluding Parameters from Differentiation

Sometimes we do not wish a parameter to be considered differentiable despite it has a differentiable type. We can use the `no_diff` modifier on the parameter to inform the compiler to treat the parameter as non-differentiable and skip generating differentiation code for the parameter. The syntax is:

```csharp
// Only differentiate this function with regard to `x`.
float myFunc(no_diff float a, float x);
```

The forward derivative and backward propagation functions of `myFunc` should have the following signature:
```csharp
DifferentialPair<float> fwd_derivative(float a, DifferentialPair<float> x);
void back_prop(float a, inout DifferentialPair<float> x, float dResult);
```

In addition, the `no_diff` modifier can also be used on the return type to indicate the return value should be considered non-differentiable. For example, the function
```csharp
no_diff float myFunc(no_diff float a, float x, out float y);
```
Will have the following forward derivative and backward propagation function signatures:

```csharp
float fwd_derivative(float a, DifferentialPair<float> x);
void back_prop(float a, inout DifferentialPair<float> x, float d_y);
```

By default, the implicit `this` parameter will be treated as differentiable if the enclosing type of the member method is differentiable. If you wish to exclude `this` parameter from differentiation, use `[NoDiffThis]` attribute on the method:
```csharp
struct MyDifferentiableType : IDifferentiable
{
    [NoDiffThis]   // Make `this` parameter `no_diff`.
    float compute(float x) { ... }
}
```

### Excluding Struct Members from Differentiation

When using automatic `IDifferentiable` conformance synthesis for a `struct` type, Slang will by-default treat all struct members that have a differentiable type as differentiable, and thus include a corresponding field in the generated `Differential` type for the struct.
For example, given the following definition
```csharp
struct MyType : IDifferentiable
{
    float member1;
    float2 member2;
}
```
Slang will generate:
```csharp
struct MyType.Differential : IDifferentiable
{
    float member1;  // derivative for MyType.member1
    float2 member2; // derivative for MyType.member2
}
```
If the user does not want a certain member to be treated as differentiable despite it has a differentiable type, a `no_diff` modifier can be used on the struct member to exclude it from differentiation.
For example, the following code excludes `member1` from differentiation:
```csharp
struct MyType : IDifferentiable
{
    no_diff float member1;  // excluded from differentiation
    float2 member2;
}
```
The generated `Differential` in this case will be:
```csharp
struct MyType.Differential : IDifferentiable
{
    float2 member2;
}
```

### Assigning Differentiable Values into a Non-Differentiable Location

When a value with derivatives is being assigned to a location that is not differentiable, such as a struct member that is marked as `no_diff`, the derivative info is discarded and any derivative propagation is stopped at the assignment site.
This may lead to unexpected results. For example:
```csharp
struct MyType : IDifferentiable
{
    no_diff float member;
    float someOtherMemther;
}
[ForwardDifferentiable]
float f(float x)
{
    MyType t;
    t.member = x * x; // Error: assigning value with derivative into a non-differentiable location.
    return t.member;
}
...
let result = fwd_diff(f)(diffPair(3.0, 1.0)).d; // result == 0.0
```
In this case, we are assigning the value `x*x`, which carries a derivative, into a non-differentiable location `MyType.member`, thus throwing away any derivative info. When `f` returns `t.member`, there will be no derivative associated with it, so the function will not propagate the derivative through. This code is most likely not intending to discard the derivative through the assignment. To help avoid this kind of unintentional behavior, Slang will treat any assignments of a value with derivative info into a non-differentiable location as a compile-time error. To eliminate this error, the user should either make `t.member` differentiable, or to force the assignment by clarifying the intention to discard any derivatives using the built-in `detach` method.
The following code will compile, and the derivatives will be discarded:
```csharp
[ForwardDifferentiable]
float f(float x)
{
    MyType t;
    // OK: the code has expressed clearly the intention to discard the derivative and perform the assignment.
    t.member = detach(x * x);
    return t.member;
}
```

### Calling Non-Differentiable Functions from a Differentiable Function
Calling non-differentiable function from a differentiable function is allowed. However, derivatives will not be propagated through the call. The user is required to clarify the intention by prefixing the call with the `no_diff` keyword. An un-clarified call to non-differentiable function will result in a compile-time error.

For example, consider the following code:
```csharp
float g(float x)
{
    return 2*x;
}

[ForwardDifferentiable]
float f(float x)
{
    // Error: implicit call to non-differentiable function g.
    return g(x) + x * x;
}
```
The derivative will not propagate through the call to `g` in `f`. As a result, `fwd_diff(f)(diffPair(1.0, 1.0))` will return
`{3.0, 2.0}` instead of `{3.0, 4.0}` as the derivative from `2*x` is lost through the non-differentiable call. To prevent unintended error, it is treated as a compile-time error to call `g` from `f`. If such a non-differentiable call is intended, a `no_diff` prefix is required in the call:
```csharp
[ForwardDifferentiable]
float f(float x)
{
    // OK. The intention to call a non-differentiable function is clarified.
    return no_diff g(x) + x * x;
}
```

However, the `no_diff` keyword is not required in a call if a non-differentiable function does not take any differentiable parameters, or if the result of the differentiable function is not dependent on the derivative being propagated through the call.

### Treat Non-Differentiable Functions as Differentiable
Slang allows functions to be marked with a `[TreatAsDifferentiable]` attribute for them to be considered as differentiable functions by the type-system. When a function is marked as `[TreatAsDifferentiable]`, the compiler will not generate derivative propagation code from the original function body or perform any additional checking on the function definition. Instead, it will generate trivial forward and backward propagation functions that returns 0.

This feature can be useful if the user marked an `interface` method as forward or backward differentiable, but only wish to provide non-trivial derivative propagation functions for a subset of types that implement the interface. For other types that does not actually need differentiation, the user can simply put `[TreatAsDifferentiable]` on the method implementations for them to satisfy the interface requirement.

See the following code for an example of `[TreatAsDifferentiable]`:
```csharp
interface IFoo
{
    [Differentiable]
    float f(float v);
}

struct B : IFoo
{
    [TreatAsDifferentiable]
    float f(float v)
    {
        return v * v;
    }
}

[Differentiable]
float use(IFoo o, float x)
{
    return o.f(x);
}

// Test:
B obj;
float result = fwd_diff(use)(obj, diffPair(2.0, 1.0)).d;
// result == 0.0, since `[TreatAsDifferentiable]` causes a trivial derivative implementation
// being generated regardless of the original code.
```

## Higher Order Differentiation

Slang supports generating higher order forward and backward derivative propagation functions. It is allowed to use `fwd_diff` and `bwd_diff` operators inside a forward or backward differentiable function, or to nest `fwd_diff` and `bwd_diff` operators. For example, `fwd_diff(fwd_diff(sin))` will have the following signature:

```csharp
DifferentialPair<DifferentialPair<float>> sin_diff2(DifferentialPair<DifferentialPair<float>> x);
```

The input parameter `x` contains four fields: `x.p.p`, `x.p.d,`, `x.d.p`, `x.d.d`, where `x.p.p` specifies the original input value, both `x.p.d` and `x.d.p` store the first order derivative if `x`, and `x.d.d` stores the second order derivative of `x`. Calling `fwd_diff(fwd_diff(sin))` with `diffPair(diffPair(pi/2, 1.0), DiffPair(1.0, 0.0))` will result `{ { 1.0, 0.0 }, { 0.0, -1.0 } }`.

User defined higher-order derivative functions can be specified by using `[ForwardDerivative]` or `[BackwardDerivative]` attribute on the derivative function, or by using `[ForwardDerivativeOf]` or `[BackwardDerivativeOf]` attribute on the higher-order derivative function.

## Interactions with Generics and Interfaces

Automatic differentiation for generic functions is supported. The forward-derivative and backward propagation functions of a generic function is also a generic function with the same set of generic parameters and constraints. Using `[ForwardDerivative]`, `[ForwardDerivativeOf]`, `[BackwardDerivative]` or `[BackwardDerivativeOf]` attributes to associate a derivative function with different set of generic parameters or constraints is a compile-time error.

An interface method requirement can be marked as `[ForwardDifferentiable]` or `[Differentiable]`, so they may be called in a forward or backward differentiable function and have the derivatives propagate through the call. This works regardless of whether the call can be specialized or has to go through dynamic dispatch. However, calls to interface methods are only differentiable once. Higher order differentiation through interface method calls are not supported.

## Restrictions of Automatic Differentiation

The compiler can generate forward derivative and backward propagation implementations for most uses of array and struct types, including arbitrary read and write access at dynamic array indices, and supports uses of all types of control flows, mutable parameters, generics and interfaces. This covers the set of operations that is sufficient for a lot of functions. However, the user needs to be aware of the following restrictions when using automatic differentiation:

- All operations to global resources, global variables and shader parameters, including texture reads or atomic writes, are treating as a non-differentiable operation.
- If a differentiable function contains calls that cause side-effects such as updates to global memory, there will not be a guarantee on how many times the side-effect will occur during the resulting derivative function or back-propagation function.
- Loops: Loops must use the attribute `[MaxIters(<count>)]` to specify a maximum number of iterations. This will be used by compiler to allocate space to store intermediate data. If the actual number of iterations exceeds the provided maximum, the behavior is undefined. You can always mark a loop with the `[ForceUnroll]` attribute to instruct the Slang compiler to unroll the loop before generating derivative propagation functions. Unrolled loops will be treated the same way as ordinary code and are not subject to any additional restrictions.

The above restrictions do not apply if a user-defined derivative or backward propagation function is provided.
