# Evaluation of Expressions

Expressions in a Slang program are evaluated during [translation](basics-translation-overview.md), runtime, or
both. The result of an expression
is a value. The earliest point when an expression can be evaluated depends on its evaluation class. All
executed expressions are evaluated during runtime at the latest.

The compiler toolchain, consisting of `slangc` and the target compilers, determines when an expression is
evaluated. Unless other constraints are present, the timing is implementation-defined. In general, the
compilers attempt to move evaluation to translation time when possible, as part of optimization.

An expression can be forced to be evaluated during Slang translation by assigning it to a `static const`
global variable, assigning it to a `static const` member variable of a struct or an interface, or providing it
as an argument to a generic value parameter.

When targeting SPIR-V, a global `const` variable can be forced to be a specialization constant with the
[\[SpecializationConstant\]](../../../core-module-reference/attributes/specializationconstant-0e.html) or
[\[vk::specialization_constant\]](../../../core-module-reference/attributes/vk_specialization_constant.html)
modifier. Specialization constants are evaluated by the target compiler before runtime.


> 📝 **Remark 1:** Currently, only limited support exists to force an expression to be evaluated at a
> specific translation stage. The full classification is provided here for completeness.

> 📝 **Remark 2:** A value resulting from an evaluation is also an expression, and a constant value is usually
> simply referred to as a *constant*. Hence, the expression evaluation classes are named *compile-time constants*,
> *uniform values*, and so forth.

> 📝 **Remark 3:** Translation-time constants are often referred to as compile-time constants. In Slang, there
> are translation-time constants beyond just those evaluated during module compilation. Hence, the term
> translation-time constant is used.


## Evaluation Classes

The evaluation class of an expression determines the earliest [translation](basics-translation-overview.md) or
runtime phase when an expression can be evaluated to a value. The classes are as follows:

1. Translation-time constants
   1. Compile-time constants
   2. Link-time constants
   3. Codegen constants
2. Runtime values
   1. Rate-specified values
      1. Uniform values
      2. Values related to execution hierarchy
         1. Thread-group-rate values
         2. Wave-rate values
      3. Graphics pipeline rate-specified values
      4. Thread-rate values
   2. Non-rate-specified values


### Description

**Compile-time constants**

A compile-time constant expression can be fully evaluated during module translation. All Slang literals belong in
this category. Compile-time constants do not depend on values in other modules.

**Link-time constants**

A link-time constant expression can be fully evaluated during linking. A link-time constant may depend on compile-time and
link-time constants in other modules.

An expression can be forced to be a link-time constant, and thus, evaluated during the specialization phase of
linking at the latest. This is achieved by assigning the expression to a `static const` global variable or a
`static const` member variable of a struct or an interface. Similarly, the argument expression for a generic
value parameter is always forced to be a link-time constant. If the compiler cannot evaluate the expression
during linking, an error is raised.

**Codegen constants**

A codegen constant expression can be fully evaluated during target program compilation.

A special class of codegen constants is Vulkan specialization constants. See
[\[vk::specialization_constant\]](../../../core-module-reference/attributes/vk_specialization_constant.html)
for details.

**Uniform values**

Uniform values are runtime values. They are immutable over a graphics launch or a compute dispatch. Uniform
values are passed to a Slang program using
[ConstantBuffer\<T\>](../../../core-module-reference/types/constantbuffer-08/index.html),
[ParameterBlock\<T\>](../../../core-module-reference/types/parameterblock-09/index.html), non-static global
`const` variables, or `uniform` entry point parameters.

**Thread-group-rate values**

Thread-group-rate values are defined per thread group instance. For example, the thread group identifier
`SV_GroupID`.

**Wave-rate values**

Wave-rate values are defined per wave instance. For example, the return value of `WaveGetActiveMask()` on the
[wave-uniform](basics-execution-divergence-reconvergence.md) path.

**Thread-rate values**

Thread-rate values are defined per thread instance. For example, the thread identifiers `SV_DispatchThreadID`
and `SV_GroupThreadID`.

**Graphics pipeline rate-specified values**

The graphics pipeline has a number of rate-specified values, often related to a specific stage. For example,
inputs from the previous stage or system-value semantics such as `SV_Position`, `SV_IsFrontFace`, and
`SV_VertexID`. Graphics pipeline rate-specified values are not further classified here.

**Non-rate-specified values**

This class is for values that do not belong in any of the aforementioned classes. For example, a read access
to a mutable shared variable that is not rate-specified results in a non-rate-specified value.


> 📝 **Remark 1:** Rate in "rate-specified" refers to the rate of change.

> 📝 **Remark 2:** Immutable (`const`) variables are separate from value classification. An immutable variable
> can hold a constant, a rate-specified runtime value, or a non-rate-specified runtime value.

> 📝 **Remark 3:** TODO: Add references to system-value semantics.


### Evaluation Class Hierarchy

Evaluation classes are partially ordered as shown in the table below, with a higher position signifying higher
"constant-ness". Ordering is not defined between execution hierarchy classes (thread-group-rate, wave-rate)
and graphics pipeline classes.

<table>
<tr>
  <td rowspan=3>Constants</td>
  <td colspan=2>Compile-time constants</td>
</tr>
<tr>
  <td colspan=2>Link-time constants</td>
</tr>
<tr>
  <td colspan=2>Codegen constants</td>
</tr>
<tr>
  <td rowspan=5>Runtime values</td>
  <td colspan=2>Uniform values</td>
</tr>
<tr>
  <td>Thread-group-rate values</td>
  <td rowspan=2>Graphics pipeline rate-specified values</td>
</tr>
<tr>
  <td>Wave-rate values</td>
</tr>
<tr>
  <td colspan=2>Thread-rate values</td>
</tr>
<tr>
  <td colspan=2>Non-rate-specified values</td>
</tr>
</table>

An expression of a higher class is always eligible to be evaluated as an expression of a lower class.

> 📝 **Remark:** In the evaluation class hierarchy, all compile-time constants are also link-time constants. All link-time
> constants are also codegen constants. All constants are also runtime values. The class of non-rate-specified
> values encompasses all other classes.


## Value Evaluation Class of an Expression

The evaluation class of an expression depends on:
- the evaluation classes of the input values
- the evaluation classes of the functions and operations in the expression

The evaluation class of an expression is the highest class in the hierarchy such that no part of the
expression has a lower value evaluation class.

> 📝 **Remark 1:** An expression that consists of wave-rate values, graphics pipeline rate-specified values,
> and functions evaluable at translation time has the evaluation class of thread-rate values. This is the
> highest class that is equal to or lower than any component in the expression.

> 📝 **Remark 2:** The list of evaluation classes of functions is unspecified at this time. In general, one
> can expect that basic built-in operations such as addition, subtraction, multiplication, and division are
> always eligible for compile-time constant expressions. When in doubt, assign the expression containing a
> function call to a `static const` global variable or to a `static const` member variable of a struct or an
> interface.

> 📝 **Remark 3:** In practice, complex expressions are often evaluated both at translation time and at
> runtime. This is a result of compiler optimizations where the subexpressions eligible for translation-time
> evaluation are reduced to constants.

