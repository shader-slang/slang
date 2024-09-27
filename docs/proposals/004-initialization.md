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

### Default Initializable type

A type is considered "default-initializable" if it provides a constructor that can take 0 arguments, so that it can be constructed with `T()`.

### Variable Initialization

Generally, a variable is considered uninitialized at its declaration site without an explicit value expression.
For example,
```csharp
struct MyType { int x ; }

void foo()
{
  MyType t; // t is uninitialized.
  var t1 : MyType; // same in modern syntax, t1 is uninitialized.
}
```

However, the Slang language has been allowing implicit initialization of variables whose types are default initializable types.
For example,
```csharp
struct MyType1 {
  int x;
  __init() { x = 0; }
}
void foo() {
  MyType t1; // `t1` is initialized with a call to `__init`.
}
```

We would like to move away from this legacy behavior towards a consistent semantics of never implicitly initializing a variable.
To maintain backward compatibility, we will keep the legacy behavior, but remove the implicit initialization when the variable is defined
in modern syntax:
```csharp
void foo() {
  var t1: MyType; // `t1` will no longer be initialized.
}
```
We will also remove the default initilaization semantics for traditional syntax in modern Slang modules that comes with an explicit `module` declaration.

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

### Synthesis of constructors for member initialization

If a type already defines any explicit constructors, do not synthesize any constructors for initializer list call. An intializer list expression
for the type must exactly match one of the explicitly defined constructors.

If the type doesn't provide any explicit constructors, the compiler need to synthesize the constructors for the calls that that the intializer
lists translate into, so that an initializer list expression can be used to initialize a variable of the type.

For each type, we will synthesize one `public` constructor:

The signature for the synthesized initializer for type `T` is:
```csharp
public T.__init(member0: typeof(member0) = default(member0), member1 : typeof(member1) = default(member1), ...)
```
where `(member0, member1, ... memberN)` is the set of members of `public` visibility, and `default(member0)`
is the value defined by the initialization expression in `member0` if it exist, or the default value of `member0`'s type.
If `member0`'s type is not default initializable and the the member doesn't provide an initial value, then the parameter will not have a default value.

The body of the constructor will initialize each member with the value comming from the corresponding constructor argument if such argument exists,
otherwise the member will be initialized to its default value either defined by the init expr of the member, or the default value of the type if the 
type is default-initializable. If the member type is not default-initializable and a default value isn't provided on the member, then such the constructor
synthesis will fail and the constructor will not be added to the type. Failure to synthesis a constructor is not an error, and an error will appear
if the user is trying to initialize a value of the type in question assuming such a constructor exist.

Note that if every member of a struct contains a default expression, the synthesized `__init` method can be called with 0 arguments, however, this will not cause a variable declaration to be implicitly initialized. Implicit initialization is a backward compatibility feature that only work for user-defined `__init()` methods.

### Single argument constructor call

Call to a constructor with a single argument is always treated as a syntactic sugar of type cast:
```csharp
int x = int(1.0f); // is treated as (int) 1.0f;
MyType y = MyType(arg); // is treated as (MyType)arg;
MyType x = MyType(y); // equivalent to `x = y`.
```

The compiler will attempt to resolve all type casts using type coercion rules, if that failed, will fall back to resolve it as a constructor call.

### Initialization List

Slang allows initialization of a variable by assigning it with an initialization list. 
Generally, Slang will always try to resolve initialization list coercion as if it is an explicit constructor invocation.
For example, given:
```csharp
S obj = {1,2};
```
Slang will try to convert the code into:
```csharp
S obj = S(1,2);
```

Following the same logic, an empty initializer list will translate into a default-initialization:
```csharp
S obj = {};
// equivalent to:
S obj = S();
```

Note that initializer list of a single argument still translates directly into a constructor call and not a type cast. For example:
```csharp
void test()
{
  MyType t = {1};
  // translates to:
  // MyType t = MyType.__init(1);
  // which is not
  // MyType t = MyType(t)
  // or
  // MyType t = (MyType)t;
}
```

If the above code passes type check, then it will be used as the way to initialize `obj`.

If the above code does not pass type check, Slang continues to check if `S` meets the standard of a "legacy C-style struct` type.
A type is a "legacy C-Style struct" if all of the following conditions are met:
- It is a user-defined struct type or a basic scalar, vector or matrix type, e.g. `int`, `float4x4`.
- It does not contain any explicit constructors defined by the user.
- All its members have the same visibility as the type itself.
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

// Assume everything below is public unless explicitly declared.

struct Empty {}
void test()
{
  Empty s0 = {}; // Works, `s` is considered initialized.
  Empty s1; // `s1` is considered uninitialized.
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
  DefaultMember m; // `m` is uninitialized.
  DefaultMember m1 = {}; // `m1` is initialized to `{0,1}`.
  DefaultMember m2 = {1}; // calls `__init(1)`, initialized to `{1,1}`.
  DefaultMember m3 = {1,2}; // calls `__init(1,2)`, initialized to `{1,2}`.
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

public struct Visibility1
{
  internal int x;
  public int y = 0;
}
void test6()
{
  Visibility1 t = {0, 0}; // error, no matching ctor
  Visibility1 t1 = {}; // error, no matching ctor
}

public struct Visibility2
{
  internal int x = 1;
  public int y = 0;
}
void test7()
{
  Visibility2 t = {0, 0}; // error, no matching ctor
  Visibility2 t1 = {}; // OK, initialized to {1,0}
}

internal struct Visibility3
{
  internal int x;
  internal int y = 2;
}
internal void test8()
{
  Visibility3 t = {0, 0}; // OK, initialized to {0,0} via legacy C-Style initialization.
  Visibility3 t1 = {1}; // OK, initialized to {1,2} via legacy logic.
}

internal struct Visibility4
{
  internal int x = 1;
  internal int y = 2;
}
internal void test9()
{
  Visibility4 t = {0, 0}; // OK, initialized to {0,0} via legacy C-Style initialization.
  Visibility4 t1 = {1}; // OK, initialized to {1,2} via legacy logic.
  Visibility4 t2 = {}; // OK, initialized to {1,2} via ctor match.
}
```


Q&A
-----------

### Should global static and groupshared variables be default initialized?

Similar to local variables, all declarations are not default initialized at its declaration site.
In particular, it is difficult to efficiently initialized global variables safely and correctly in a general way on platforms such as Vulkan,
so implicit initialization for these variables can come with serious performance consequences.

### Should `out` parameters be default initialized?

Following the same philosphy of not initializing any declarations, `out` parameters are also not default-initialized.

Alternatives Considered
-----------------------

One important decision point is whether or not Slang should allow variables to be left in uninitialized state after its declaration as it is allowed in C++.
Our opinion is that this is not what we want to have in the long term and Slang should take the opportunity as a new language to not inherit from this
undesired C++ legacy behavior.