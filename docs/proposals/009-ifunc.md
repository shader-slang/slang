SP #009 - IFunc interface
==============

Now that we have variadic generics in the language following [SP #007], we should now be able to define a builtin `IFunc` interface that represent
things that can be called with the `()` operator. This will allow users to write generic functions that takes a callback object and adopt more
functional programming idioms.

Status
------

Author: Yong He

Implementation: Planned.

Reviewed by: N/A

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

We propose `IFunc` and `IMutatingFunc` that is defined as follows:

```
// Function objects that does not have a mutating state.
interface IFunc<TResult, each TParams>
{
    TResult __call(expand each TParams params);
}

// Function objects with a mutating state.
interface IMutatingFunc<TFunc, each TParams>
{
    [mutating]
    TResult __call(expand each TParams params);
}
```

Ordinary functions are treated as conforming to `IFunc` and `IMutatingFunc` automatically,
so the following code is valid:

```
int countElement(int data[100], IFunc<bool, int> condition)
{
    int count = 0;
    for (int i = 0; i < data.getCount(); i++)
        if (condition(data[i]))
            count++;
    return count;
}

int myCondition(int x) { return x%2 == 0; } // select all even numbers.

void test()
{
    int data[100] = ...;
    int count = countElement(data, myCondition);
}
```

An ordinary function or static function with type `(T0, T1, ... Tn)->TR` is coerceable to `IFunc<TIR, TI0, TI1, ... TIn>` if
`TI0, TI1, ... TIn` are coerceable to `T0, T1, ... Tn` and `TR` is coerceable to `TIR`.
To achieve this, the compiler will synthesize a wrapper struct type conforms to the `IFunc` interface, and calls the original function
in its `__call` method.

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
    void __call(int data)
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
