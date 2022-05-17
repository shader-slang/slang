COM Support
===========

When Slang is used as a host/CPU programming language, it is likely that users will want to use interact with COM interfaces, either by consuming them or implementing them.
The Slang language and compiler should provide some first-class features to make working with COM interfaces feel lightweight and natural.

Status
------

In discussion.

Background
----------

COM is not perfect, but it is one of the only real solutions for cross-platform portable C++ APIs that care about binary compatibility and versioning.
Developers who use Slang are likely to write code that uses COM, whether to interact with Slang itself (and/or GFX), or with platform APIs like D3D.

While COM provides idioms for addressing many practical challenges, it is also inconvenient in that it introduces a lot of *boilerplate*:

* COM types all need to implement the core `IUnknown` operations for casting/querying and reference counting.

* Code using COM interfaces needs to perform `AddRef` and `Release` operations manually, or use smart pointer types to automate lifetime management.

* Code that calls into COM interfaces typically needs to use boilerplate code and/or macros to deal with `HRESULT` error codes, handling or propagating them as needed.

Our in-progress work on supporting CPU programming in Slang emphasizes supporting idiomatic code without a lot of boilerplate.
Our intended path includes things that are compatible with the COM philosophy, like reference-counted lifetime management and idiomatic use of result/error codes, but those features don't currently align with the more explicit style used by COM in C/C++.

Related Work
------------

The .NET platform includes some support for allowing .NET `interface`s and COM interfaces to interoperate.
TODO: Need to study this and learn how it works.

Proposed Approach
-----------------

We propose to allow COM interfaces to be declared using the Slang `interface` construct, with an appropriate attribute or modifier:

```
[COM] interface IDevice
{
    ITexture createTexture(__read TextureDesc desc) throws HRESULT;

    void setTexture(int index, ITexture texture);
}
```

A declaration like the above will translate into output C++ along the lines of:

```
struct IDevice : public IUnknown
{
    virtual HRESULT SLANG_MCALL createTexture(TextureDesc const& desc, ITexture** _result) = 0;

    virtual void SLANG_MCALL setTexture(int index, ITexture* texture) = 0;
};
```

Key things to note:

* The `[COM] interface` becomes a C++ `struct` that inherits from `IUnknown`
* Methods defined in the `[COM] interface` become pure-virtual `SLANG_MCALL` methods in C++
* Parametes/values of a `[COM] interface` type `IFoo` in Slang translate to `IFoo*` in C++
* Methods that have a `throws HRESULT` clause are transformed to have an `HRESULT` return type and an output parameter for their result

A Slang `class` can declare that it implements zero or more `[COM] interface`s. Code like this:

```
class MyTexture : ITexture
{
    // ...
}

class MyDevice : IDevice
{
    ITexture createTexture(__read TextureDesc desc) throws HRESULT
    {
        return ...;
    }

    void setTexture(int index, ITexture texture)
    {
        ...;
    }
}
```

translates into output C++ like this:

```
class MyTexture : public slang::Object, public ITexture
{
    // ...
};

class MyDevice : public slang::Object, public  IDevice
{
    virtual HRESULT QueryInterface(REFIID riid, void **ppvObject) { ... }
    virtual ULONG AddRef() { ... }
    virtual ULONG Release() { ... }

    HRESULT createTexture(TextureDesc const& desc, ITexture** _result) SLANG_OVERRIDE
    {
        _result = ...;
        return S_OK;
    }

    void setTexture(int index, ITexture* texture) SLANG_OVERRIDE
    {
        ...
    }
}
```

All Slang `class`es translate to C++ classes that inherit from `slang::Object` (equivalent to the `RefObject` type within the current Slang implementation).
A `class` that inherits any `[COM]` interfaces includes an implementation of `IUnknown` plus the methods to override all the interface requirements.

In ordinary code that makes use of `[COM] interface` types:

```
struct Stuff
{
    ITexture t;
}
ITexture getTexture(Stuff stuff)
{
    return stuff.t;
}
ITexture someOperations(IDevice device)
{
    let t = device.createTexture(...);
    return t;
}
```

the C++ output uses C++ smart-pointer types for local variables, `struct` fields, and function results:

```
struct Stuff
{
    ComPtr<ITexture> t;
};
ComPtr<ITexture> getTexture(Stuff stuff)
{
    return stuff.t;
}
HRESULT someOperations(IDevice* device, ComPtr<ITexture>& _result)
{
    ComPtr<ITexture> t;
    HRESULT _err = device->createTexture(&t);
    if(_err < 0) return _err;

    _result = t;
    return 0;
}
```

As a small optimization, an `in` parameter of a `[COM] interface` type translates as a C++ parameter of just the matching pointer type (see `device` above).

Note that the translation of idiomatic `HRESULT` return codes into `throws HRESULT` functions in Slang allows code working with COM interfaces to benefit from the convenience of the Slang error handling model.


Detailed Explanation
--------------------

This is a case where the simple explanation above covers most of the interesting stuff.

There are a lot of semantic checks we'd need/want to implement to make sure `[COM]` interfaces are used correctly:

* Any `interface` that inherits from one or more other `[COM]` interfaces must itself be `[COM]`

* Any concrete type that implements a `[COM]` interface must be a `class`

There are also detailed implementation questions to be answered around the in-memory layout of `class` types that implement `[COM]` interfaces.
In particular, we might want to be able to optimize for the case of a single-inheritance `class` hierarchy that mirrors a `[COM] interface` hierarchy, since this comes up often for COM-based APIs:

```
interface IBase { void baseFunc(); }
interface IDerived : IBase { void derivedFunc(); }

class BaseImpl : IBase { ... }
class DerivedImpl : BaseImpl, IDerived { ... }
```

Using a naive translation to C++, the `DerivedImpl` type could end up with *three* different virtual function table (`vtbl`) pointers embedded in it: one for `slang::Object`, one for `IBase`, and one for `IDerived`.
Clearly the `vtbl`s for `IBase` and `IDerived` could be shared, but C++ `class`es can't easily express this.
Furthermore, if we are able to tune our strategy for layout, we can set things up so that `[COM] interface`s consume `vtbl` slots starting at index `0` and counting up, while any `virtual` methods in `class`es consume slots starting at index `-1` and counting *down*.
Using such a layout strategy we can actually allow a type like `DerivedImpl` above to use only a *single* `vtbl` pointer.

Questions
--------------

### Should we emit COM code that works at the plain C level, or idiomatic C++?

I honestly don't know. Emitting idomatic C++ (and using things like smart pointers) certainly makes the output code easier to understand.

### Can we make this work with more advanced features of Slang interfaces?

Some Slang features don't have perfect analogues. For example, given that `[COM] interface`s can only be conformed to by `class` types, the use of `[mutating]` isn't especially meaningful.

There is no reason why a `[COM] interface` couldn't make use of `static` methods, but there would be no way to call those from C++ without an instance of the interface type.

A `[COM] interface` could include `property` declarations, provided that we define the rules for how they translate into getter/setter operations in the generated output.

One interesting case is that a `[COM] interface` could allow use of `This`, as well as `associatedtype`s that are themselves constrained by a `[COM] interface`.
For example, we could instead device our `IDevice` interface from before as:

```
[COM] interface IDevice
{
    associatedtype Texture : ITexture;

    Texture createTexture(__read TextureDesc desc) throws HRESULT;

    void setTexture(int index, Texture texture);
}
```

We could set things up so that the `associatedtype` has no impact on the C++ translation of `IDevice`: all parameter/result types that use the `Texture` associated type would translate to `ITexture*` in C++.
As such, a more refined `interface` like this would not disrupt the binary interface of a COM-based API, but could be allowed to express more of the constraints of the underlying API at compile time.
For example, the above use of `associatedtype` would prevent Slang code from mixing up textures across devices:

```
void someFunc( IDevice a, IDevice b )
{
    let x = a.createTexture(...);
    let y = b.createTexture(...);

    a.setTexture(0, y); // COMPILE TIME ERROR!
}
```

In this example, `y` has type `b.Texture`, while `a.setTexture` expects an argument of type `a.Texture` (a distinct type, even if it also conforms to `ITexture`).
The benefits of this approach are probably purely hypothetical until we make it a lot easier to work with dependent types like `a.Texture` in Slang code.

Alternatives Considered
-----------------------

The main alternative would be to have Slang's model for interop with C/C++ focus primarily on C alone, and only allow use of COM-based APIs through C-compatible interfaces.
