SP #004: Initialization
=================

This proposal documents the desired behavior of initialization related language semantics, including default constructor, initialization list and variable initialization.

Status
------

Status: Design Review

Implemtation: N/A

Author: Yong He

Reviewer: 

Background
----------

Slang has introduced several different syntax around initialization to provide syntactic compatibility with HLSL/C++. As the language evolve, there are many corners where
the semantics around initialization are not well-defined, and causing confusion or leading to surprising behaviors.

This proposal attempts to provide a design on where we want the language to be in terms of how initialization is handled in all different places.

Related Work
------------

C++ has many different ways and syntax to initialize an object: through explicit constructor calls, initialization list, or implicitly in a member/variable declaration.
A variable in C++ can also be in an uninitialized state after its declaration. HLSL inherits most of these behvior from C++ by allowing variables to be uninitialized.

On the other hand, languages like C# and Swift has a set of well defined rules to ensure every variable is initialized after its declaration.

Proposed Approach
-----------------

The high level direction for Slang is it needs to help ensuring variables are properly initialized at its declaration site, while still providing a reasonable level of backward compatibility with existing Slang/HLSL code and avoid initialization behaviors that may have non-trivial performance impact. In this section, we document all concepts and rules to achieve this goal.

### `IDefaultInitializable` interface

The builtin `IDefaultInitializable` interface is defined as:
```csharp
interface IDefaultInitializable
{
    __init();
}
```

Any type that conforms to `IDefaultInitializable` is treated as having the *default-initializable* property in the rest of the discussions. By default, all builtin
types, such as `int`, `float[]`, `float4` are default-initializable. The values for builtin numeric types after default-initialization are 0.
The default value for pointers is `nullptr`, and default value for `Optional<T>` is `none`.

### Default-Initializable Type

A type X is default initializable if:
- It explicitly declares that `X` implements `IDefaultInitializable`.
- It explicitly provides a default constructor `X::__init()` that takes no arguments, in which case we treat the type as implementing `IDefaultInitializable` even if
  this conformance isn't explicitly declared.
- It is a struct type where all its members are default-initializable. A member is considered default-initializable if the type of the member is default-initializable,
  or if the member has an initialization expression that defines its default value.
- It is a sized-array type where the element type is default-initializable.
- It is a tuple type where all element types are default-initializable.
- It is an `Optional<T>` type for any `T`.

This means that an unsized-array type or an existential type, or any composite type that contains such types without providing an explicit default constructor are not considered default-initializable.

Note that `void` type should be treated as default initializable, since `void` type can have only one value that is `()` (empty tuple).

### Variable Initialization

If the type of a local variable is default-initializable, then its default initializer will be invoked at its declaration site implicitly to intialize its value:
```c++
int x; // x will be default initialized to 0 because `int` is default-initializable.
// The above is equivalent to:
int x = int();

struct S { int x; int y; }
S s; // s will be default initialized to {0, 0} because `S` is default-initializable.
```

If a type is not default-initializable, and the declaration site does not provide an intial value for the variable, the compiler should generate an error:
```csharp
struct V { int[] arr; }

V v; // error: `v` must be initialized at declaration.
```

For backward compatibility, we will introduce a compiler option to turn this error into a warning, but we may deprecate this option in the future.

### Generic Type Parameter

A generic type parameter is not considered default-initializable by-default. As a result, the following code should produce error:
```csharp
void foo<T>()
{
    T t; // error, `t` is uninitialized.
}
```

### Automatic Synthesis of Default-Initializer

If a `struct` type is determined to be default-initializable but a default constructor isn't explicitly provided by the user, the Slang compiler should
synthesize such a constructor for the type. The synthesis logic should be recursively invoke default initializer on all members.

### Automatic Synthesis of `IDefaultInitializable` Conformance

If a `struct` type provides an explicit default constructor, the compiler should automatically add `IDefaultIinitializable` to the conformance list of
the type, so it can be used for any generic parameters constrained on `IDefaultInitializble`.

### Initialization List

Slang allows initialization of a variable by assigning it with an initialization list. 
Generally, Slang will always try to resolve initialization list coercion as if it is an explicit constructor invocation.
For example, given:
```
S obj = {1,2};
```
Slang will try to convert the code into:
```
S obj = S(1,2);
```

As a special case, an empty initializer list will translate into a default-initialization:
```csharp
S obj = {};
// equivalent to:
S obj = S();
```

If the above code passes type check, then it will be used as the way to initialize `obj`.

If the above code does not pass type check, Slang continues to check if `S` meets the standard of a "legacy C-style struct` type.
A type is a "legacy C-Style struct" iff:
- It is a struct type.
- It is a basic scalar, vector or matrix type, e.g. `int`, `float4x4`.
- It does not define any explicit constructors
- It does not define any initialization expressions on its members.
- All its members are legacy C-Style structs or arrays of legacy C-style structs.
In such case, we perform a legacy "read data" style consumption of the initializer list, so that the following behavior is valid:

```csharp
struct Inner { int x; int y; };
struct Outer { Inner i; Inner j; }

Outer o = {1, 2, 3, 4}; // Initializes `o` into `{ Inner{1,2}, Inner{3,4} }`.
```

### Synthesis of constructors for member initialization

If a type already defines any explicit constructors, do not synthesize any constructors for initializer list call. An intializer list expression
for the type must exactly match one of the explicitly defined constructors.

If the type doesn't provide any explicit constructors, the compiler need to synthesis the constructors for the calls that that the intializer
lists translate into, so that an initializer list expression can be used to initialize a variable of the type.

For each visibilty level `V` in (`private`, `internal`, `public`), we will synthesize one constructor at that visiblity level.

The signature for the synthesized initializer for type `T` is:
```csharp
V T.__init(member0: typeof(member0) = default(member0), member1 : typeof(member1) = default(member1), ...)
```
where `V` is the visibilty level, and `(member0, member1, ... memberN)` is the set of members at or above visiblity level `V`, and `default(member0)`
is the value defined by the initialization expression in `member0` if it exist, or the default value of `member0`'s type.
If `member0`'s type is not default initializable and the the member doesn't provide an initial value, then the parameter will not have a default value.

The body of the constructor will initialize each member with the value comming from the corresponding constructor argument if such argument exists,
otherwise the member will be initialized to its default value either defined by the init expr of the member, or the default value of the type if the 
type is default-initializable. If the member type is not default-initializable and a default value isn't provided on the member, then such the constructor
synthesis will fail and the constructor will not be added to the type. Failure to synthesis a constructor is not an error, and an error will appear
if the user is trying to initialize a value of the type in question assuming such a constructor exist.


Q&A
-----------

### Should global static and groupshared variables be defualt initialized?

It is difficult to efficiently initialized global variables safely and correctly in a general way on platforms such as Vulkan.
To avoid the performance issues, the current decision is to not to default initialized these global variables.

### Should `out` parameters be default initialized?

The source of an `out` parameter is either comming from a local variable that is already default-initialized, or from a 
global variable where we can't default-initialize efficiently. For this reason, we should leave `out` parameter to not
be default initialized implicitly by the compiler.

Alternatives Considered
-----------------------

One important decision point is whether or not Slang should allow variables to be left in uninitialized state after its declaration as it is allowed in C++.
Our opinion is that this is not what we want to have in the long term and Slang should take the opportunity as a new language to not inherit from this
undesired C++ legacy behavior.