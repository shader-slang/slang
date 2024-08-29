SP #009 - IFunc interface
==============

Now that we have variadic generics in the language following [SP #007], we should now be able to define a builtin `IFunc` interface that represent
things that can be called with the `()` operator. This will allow users to write generic functions that takes a callback object and adopt more
functional programming idioms.

Status
------

Author: Yong He

Status: Implemented.

Implementation: [PR 4905](https://github.com/shader-slang/slang/pull/4905) [PR 4926](https://github.com/shader-slang/slang/pull/4926)

Reviewed by: Kai Zhang, Jay Kwak

Background
----------

Callback is an idiom that frequently show up in complex codebases. Currently, Slang users can implement this idiom with
interfaces:

```
interface ICondition
{
    bool test(int x);
}

int countElement(int data[100], ICondition condition)
{
    int count = 0;
    for (int i = 0; i < data.getCount(); i++)
        if (condition.test(data[i]))
            count++;
    return count;
}

int myCondition(int x) { return x%2 == 0; } // select all even numbers.

struct MyConditionWrapper : ICondition
{
    bool test(int x) { return myCondition(x); }
};

void test()
{
    int data[100] = ...;
    int count = countElement(data, MyConditionWrapper());
}
```

As can be seen, this is a lot of boilerplate. With a builtin `IFunc` interface, we can
allow the compiler to automatically make ordinary functions conform to the interface,
eliminating the need for defining interfaces and wrapper types.


Proposed Approach
-----------------

We should support overloading of `operator()`, and use the function call syntax to call the `operator()` member, similar to C++:
```
struct Functor
{
    int operator()(float p) {}
}

void test()
{
    Functor f;
    f(1.0f);
}
```

We propose `IFunc`, `IMutatingFunc`, `IDifferentiableFunc` and `IDiffernetiableMutatingFunc` that is defined as follows:

```
// Function objects that does not have a mutating state.
interface IMutatingFunc<TR, each TP>
{
    [mutating]
    TR operator()(expand each TP p);
}

// Function objects with a mutating state.
interface IFunc<TR, each TP> : IMutatingFunc<TR, expand each TP>
{
    TR operator()(expand each TP p);
}

// Differentiable functions
interface IDifferentiableMutatingFunc<TR : IDifferentiable, each TP : IDifferentiable> : IMutatingFunc<TR, expand each TP>
{
    [Differentiable]
    [mutating]
    TR operator()(expand each TP p);
}

interface IDifferentiableFunc<TR : IDifferentiable, each TP : IDifferentiable> : IFunc<TR, expand each TP>, IDifferentiableMutatingFunc<TR, expand each TP>
{
    [Differentiable]
    TR operator()(expand each TP p);
}
```

The `IMutatingFunc` interface is for defining functors that has a mutable state. The following example demonstrates its use:

```
void forEach(int data[100], inout IMutatingFunc<void, int> f)
{
    for (int i = 0; i < data.getCount(); i++)
        f(data[i]);
}

struct CounterFunc : IMutatingFunc<void, int>
{
    int count;

    [mutating]
    void operator()(int data)
    {
        if (data % 2 == 0)
            count++;
    }
};

void test()
{
    int data[100] = ...;
    CounterFunc f;
    f.count = 0;
    forEach(data, f);
    printf("%d", f.count);
}
```

# Coercion of ordinary functions

Eventually, we should allow ordinary functions to be automatically coerceable to `IFunc` interfaces. But this is scoped out
for the initial `IFunc` work, because we believe the implementation can be simpler if we support lambda function first, then
implement ordinary function coercion as a special case of lambda expressions.