SP #004: Initialization
=================

This proposal documents the desired behavior of initialization related language semantics, including default constructor, initialization list and variable initialization.

Status
------

Status: Design Approved, implementation in-progress.

Implementation: N/A

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
C++ allows using the initialization list syntax to initialize an object. The semantics of initialization lists depends on whether or not explicit constructors
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

If a type already defines any explicit constructors, do not synthesize any constructors for initializer list call. An initializer list expression
for the type must exactly match one of the explicitly defined constructors.

If the type doesn't provide any explicit constructors, the compiler need to synthesize the constructors for the calls that that the initializer
lists translate into, so that an initializer list expression can be used to initialize a variable of the type.

For each type, we will synthesize one constructor at the same visibility of the type itself:

The signature for the synthesized initializer for type `V struct T` is:
```csharp
V T.__init(member0: typeof(member0) = default(member0), member1 : typeof(member1) = default(member1), ...)
```
where `V` is a visibility modifier, `(member0, member1, ... memberN)` is the set of members that has visibility `V`, and `default(member0)`
is the value defined by the initialization expression in `member0` if it exist, or the default value of `member0`'s type.
If `member0`'s type is not default initializable and the the member doesn't provide an initial value, then the parameter will not have a default value.

The synthesized constructor will be marked as `[Synthesized]` by the compiler, so the call site can inject additional compatibility logic when calling a synthesized constructor.

The body of the constructor will initialize each member with the value coming from the corresponding constructor argument if such argument exists,
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

### Inheritance Initialization
For derived structs, slang will synthesized the constructor by bringing the parameters from the base struct's constructor if the base struct also has a synthesized constructor. For example:
```csharp
struct Base
{
  int x;
  // compiler synthesizes:
  // __init(int x) { ... }
}
struct Derived : Base
{
  int y;
  // compiler synthesizes:
  // __init(int x, int y) { ... }
}
```

However, if the base struct has explicit ctors, the compiler will not synthesize a constructor for the derived struct.
For example, given
```csharp
struct Base { int x; __init(int x) { this.x = x; } }
struct Derived : Base { int y;}
```
The compiler will not synthesize a constructor for `Derived`, and the following code will fail to compile:
```csharp

Derived d = {1};            // error, no matching ctor.
Derived d = {1, 2};         // error, no matching ctor.
Derived d = Derived(1);     // error, no matching ctor.
Derived d = Derived(1, 2);  // error, no matching ctor.
```


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

Note that initializer list of a single argument does not translate into a type cast, unlike the constructor call syntax. Initializing with a single element in the initializer list always translates directly into a constructor call. For example:
```csharp
void test()
{
  MyType t = {1};
  // translates to direct constructor call:
  // MyType t = MyType.__init(1);
  // which is NOT the same as:
  // MyType t = MyType(t)
  // or:
  // MyType t = (MyType)t;
}
```

If the above code passes type check, then it will be used as the way to initialize `obj`.

If the above code does not pass type check, and if there is only one constructor for`MyType` that is synthesized as described in the previous section (and therefore marked as `[Synthesized]`, Slang continues to check if `S` meets the standard of a "legacy C-style struct` type. A type is a "legacy C-Style" type if it is a:
- Basic scalar type (e.g. `int`, `float`).
- Enum type.
- Sized array type where the element type is C-style type.
- Tuple type where all member types are C-style types.
- A "C-Style" struct.

A struct is C-Style if all of the following conditions are met:
- It does not inherit from any other types.
- It does not contain any explicit constructors defined by the user.
- All its members have the same visibility as the type itself.
- All its members are legacy C-Style types.
Note that C-Style structs are allowed to have member default values.
In such case, we perform a legacy "read data" style consumption of the initializer list to synthesize the arguments to call the constructor, so that the following behavior is valid:

```csharp
struct Inner { int x; int y; };
struct Outer { Inner i; Inner j; }

// Initializes `o` into `{ Inner{1,2}, Inner{3,0} }`, by synthesizing the
// arguments to call `Outer.__init(Inner(1,2), Inner(3, 0))`.
Outer o = {1, 2, 3};
```

If the type is not a legacy C-Style struct, Slang should produce an error.

### Legacy HLSL syntax to cast from 0

HLSL allows a legacy syntax to cast from literal `0` to a struct type, for example:
```hlsl
MyStruct s { int x; }
void test()
{
  MyStruct s = (MyStruct)0;
}
```

Slang treats this as equivalent to a empty-initialization:
```csharp
MyStruct s = (MyStruct)0;
// is equivalent to
MyStruct s = {};
```

Examples
-------------------
```csharp

// Assume everything below is public unless explicitly declared.

struct Empty
{
  // compiler synthesizes:
  // __init();
}
void test()
{
  Empty s0 = {}; // Works, `s` is considered initialized via ctor call.
  Empty s1; // `s1` is considered uninitialized.
}

struct CLike
{
  int x; int y;
  // compiler synthesizes:
  // __init(int x, int y);
}
void test1()
{
  CLike c0; // `c0` is uninitialized.

  // case 1: initialized with synthesized ctor call using legacy logic to form arguments,
  // and `c1` is now `{0,0}`.
  // (we will refer to this scenario as "initialized with legacy logic" for
  // the rest of the examples):
  CLike c1 = {}; 

  // case 2: initialized with legacy initializaer list logic, `c1` is now `{1,0}`:
  CLike c2 = {1}; 

  // case 3: initilaized with ctor call `CLike(1,2)`, `c3` is now `{1,2}`:
  CLike c3 = {1, 2};
}

struct ExplicitCtor
{
   int x;
   int y;
   __init(int x) {...}
  // compiler does not synthesize any ctors.
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
  // compiler synthesizes:
  // __init(int x = 0, int y = 1);
}
void test3()
{
  DefaultMember m; // `m` is uninitialized.
  DefaultMember m1 = {}; // calls `__init()`, initialized to `{0,1}`.
  DefaultMember m2 = {1}; // calls `__init(1)`, initialized to `{1,1}`.
  DefaultMember m3 = {1,2}; // calls `__init(1,2)`, initialized to `{1,2}`.
}

struct PartialInit {
  // warning: not all members are initialized.
  // members should either be all-uninitialized or all-initialized with
  // default expr.
  int x;
  int y = 1;
  // compiler synthesizes:
  // __init(int x, int y = 1);
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
  // compiler synthesizes:
  // __init(int x, int y);
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
  // the compiler does not synthesize any ctor.
  // the compiler will try to synthesize:
  //     public __init(int y);
  // but then it will find that `x` cannot be initialized.
  // so this synthesis will fail and no ctor will be added
  // to the type.
}
void test6()
{
  Visibility1 t = {0, 0}; // error, no matching ctor
  Visibility1 t1 = {}; // error, no matching ctor
  Visibility1 t2 = {1}; // error, no matching ctor
}

public struct Visibility2
{
  // Visibility2 type contains members of different visibility,
  // which disqualifies it from being considered as C-style struct.
  // Therefore we will not attempt the legacy fallback logic for
  // initializer-list syntax.
  internal int x = 1;
  public int y = 0;
  // compiler synthesizes:
  // public __init(int y = 0);
}
void test7()
{
  Visibility2 t = {0, 0}; // error, no matching ctor.
  Visibility2 t1 = {}; // OK, initialized to {1,0} via ctor call.
  Visibility2 t2 = {1}; // OK, initialized to {1,1} via ctor call.
}

internal struct Visibility3
{
  // Visibility3 type is considered as C-style struct.
  // Because all members have the same visibility as the type.
  // Therefore we will attempt the legacy fallback logic for
  // initializer-list syntax.
  // Note that c-style structs can still have init exprs on members.
  internal int x;
  internal int y = 2;
  // compiler synthesizes:
  // internal __init(int x, int y = 2);
}
internal void test8()
{
  Visibility3 t = {0, 0}; // OK, initialized to {0,0} via ctor call.
  Visibility3 t1 = {1}; // OK, initialized to {1,2} via ctor call.
  Visibility3 t2 = {}; // OK, initialized to {0, 2} via legacy logic.
}

internal struct Visibility4
{
  // Visibility4 type is considered as C-style struct.
  // And we still synthesize a ctor for member initialization.
  // Because Visibility4 has no public members, the synthesized
  // ctor will take 0 arguments.
  internal int x = 1;
  internal int y = 2;
  // compiler synthesizes:
  // internal __init(int x = 1, int y = 2);
}
internal void test9()
{
  Visibility4 t = {0, 0}; // OK, initialized to {0,0} via ctor call.
  Visibility4 t1 = {3}; // OK, initialized to {3,2} via ctor call.
  Visibility4 t2 = {}; // OK, initialized to {1,2} via ctor call.
}
```

### Zero Initialization

The Slang compiler supported an option to force zero-initialization of all local variables.
This is currently implemented by adding `IDefaultInitializable` conformance to all user
defined types. With the direction we are heading, we should remove this option in the future.
For now we can continue to provide this functionality but through an IR rewrite pass instead
of changing the frontend semantics. 

When users specifies `-zero-initialize`, we should still use the same front-end logic for
all the checking. After lowering to IR, we should insert a `store` after all `IRVar : T` to
initialize them to `defaultConstruct(T)`.


Q&A
-----------

### Should global static and groupshared variables be default initialized?

Similar to local variables, all declarations are not default initialized at its declaration site.
In particular, it is difficult to efficiently initialized global variables safely and correctly in a general way on platforms such as Vulkan,
so implicit initialization for these variables can come with serious performance consequences.

### Should `out` parameters be default initialized?

Following the same philosophy of not initializing any declarations, `out` parameters are also not default-initialized.

Alternatives Considered
-----------------------

One important decision point is whether or not Slang should allow variables to be left in uninitialized state after its declaration as it is allowed in C++. In contrast, C# forces everything to be default initialized at its declaration site, which come at the cost of incurring the burden to developers to come up with a way to define the default value for each type.
Our opinion is we want to allow things as uninitialized, and to have the compiler validation checks to inform
the developer something is wrong if they try to use a variable in uninitialized state. We believe it is desirable to tell the developer what's wrong instead of using a heavyweight mechanism to ensure everything is initialized at declaration sites, which can have non-trivial performance consequences for GPU programs, especially when the variable is declared in groupshared memory.
