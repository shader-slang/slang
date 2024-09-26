SP #004: Initialization
=================

This proposal documents the desired behavior of initialization related language semantics, including default constructor, initialization list and variable initialization.

Status
------

Status: Design Review

Implemtation: N/A

Author: Yong He

Reviewer: Theresa Foley, Kai Zhang

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
C++ allows using the initilization list syntax to initialize an object. The semantics of initialization lists depends on whether or not explicit constructors
are defined on the type.

Proposed Approach
-----------------

In this section, we document all concepts and rules related to initialization, constructors and initialization lists.

### `IDefaultInitializable` interface

The builtin `IDefaultInitializable` interface is defined as:
```csharp
interface IDefaultInitializable
{
    __init();
}
```

Any type that conforms to `IDefaultInitializable` is treated as having the *default-initializable* property in the rest of the discussions. 
### Default-Initializable Type

A type X is default initializable if:
- It explicitly declares that `X` implements `IDefaultInitializable`.
- It explicitly provides a default constructor `X::__init()` that takes no arguments, in which case we treat the type as implementing `IDefaultInitializable` even if
  this conformance isn't explicitly declared.
- It is a sized-array type where the element type is default-initializable.

This means that an unsized-array type or an existential type, or any composite type that contains such types without providing an explicit default constructor are not considered default-initializable.
Builtin types like `int`, `float`, `Ptr<T>`, `Optional<T>` and non-emtpy `Tuple<T...>` etc. are *not* default-initializable.

Note that `void` type should be treated as default initializable, since `void` type can have only one value that is `()` (empty tuple).

### Variable Initialization

If the type of a local variable is default-initializable, then its default initializer will be invoked at its declaration site implicitly to intialize its value:
```c++
struct T : IDefaultInitializable { __init() { ... }}
T x; // x will be default initialized by the default constructor because `T` is default-initializable.
// The above is equivalent to:
T x = T();
```

If a type is not default-initializable, it will be left in an uninitialized state after its declaration:
```csharp
struct S { int x; int y; }
S s; // s will not be default initialized, because `S` is not default-initializable.
```
Trying to use a variable without initializing it first is an error.
For backward compatibility, we will introduce a compiler option to turn this error into a warning, but we may deprecate this option in the future.

### Generic Type Parameter

A generic type parameter is not considered default-initializable by-default. As a result, the following code should leave `t` in an uninitialized state:
```csharp
void foo<T>()
{
    T t; // `t` is uninitialized at declaration.
}
```

### Automatic Synthesis of `IDefaultInitializable` Conformance

If a `struct` type provides an explicit default constructor, the compiler should automatically add `IDefaultIinitializable` to the conformance list of
the type, so it can be used for any generic parameters constrained on `IDefaultInitializble`.


### Synthesis of constructors for member initialization

If a type already defines any explicit constructors, do not synthesize any constructors for initializer list call. An intializer list expression
for the type must exactly match one of the explicitly defined constructors.

If the type doesn't provide any explicit constructors, the compiler need to synthesize the constructors for the calls that that the intializer
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

Note that if every member of a struct contains a default expression, the synthesized `__init` method can be called with 0 arguments. This should make
the type conform to `IDefaultInitializable` and behave as a default-initializable type.


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

Following the same logic, an empty initializer list will translate into a default-initialization:
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
- It does not contain any explicit constructors defined by the user.
- It does not define any initialization expressions on its members.
- All its members are legacy C-Style structs or arrays of legacy C-style structs.
In such case, we perform a legacy "read data" style consumption of the initializer list, so that the following behavior is valid:

```csharp
struct Inner { int x; int y; };
struct Outer { Inner i; Inner j; }

Outer o = {1, 2, 3}; // Initializes `o` into `{ Inner{1,2}, Inner{3,0} }`.
```

If the type is not a legacy C-Style struct, Slang should produce an error.

Examples
-------------------
```csharp
struct Empty {}
void test()
{
  Empty s0 = {}; // Works, `s` is considered initialized.
  Empty s1; // `s1` is considered initialized bcause `Empty` is `IDefaultInitializable`.
}

struct CLike {int x; int y; }
void test1()
{
  CLike c0; // `c0` is uninitialized.
  CLike c1 = {}; // initialized with legacy initializaer list logic, `c1` is now `{0,0}`.
  CLike c2 = {1}; // initialized with legacy initializaer list logic, `c1` is now `{1,0}`.
  CLike c3 = {1, 2}; // initilaized with ctor call `CLike(1,2)`, `c3` is now `{1,2}`.
}

struct ExplicitCtor {
   int x;
   int y;
   __init(int x) {...}
}
void test2()
{
  ExplicitCtor e0; // `e0` is uninitialized.
  ExplicitCtor e1 = {1}; // calls `__init`.
  ExplicitCtor e2 = {1, 2}; // error, no ctor matches initializer list.
}

struct DefaultMember {
  int x = 0;
  int y = 1;
}
void test3()
{
  DefaultMember m; // `m` is initialized to `{0, 1}`.
  DefaultMember m1 = {1}; // calls `__init(1)`, initialized to `{1,1}`.
  DefaultMember m2 = {1,2}; // calls `__init(1,2)`, initialized to `{1,2}`.
}

struct PartialInit {
  // warning: not all members are initialized.
  // members should either be all-uninitialized or all-initialized with
  // default expr.
  int x;
  int y = 1;
}
void test4()
{
  PartialInit i; // `i` is not initialized.
  PartialInit i1 = {2}; // calls `__init`, result is `{2,1}`.
  PartialInit i2 = {2, 3}; // calls `__init`, result is {2, 3}
}

struct PartialInit2 {
  int x = 1;
  int y; // warning: not all members are initialized.
}
void test5()
{
  PartialInit2 j; // `j` is not initialized.
  PartialInit2 j1 = {2}; // error, no ctor match.
  PartialInit2 j2 = {2, 3}; // calls `__init`, result is {2, 3}
}
```


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