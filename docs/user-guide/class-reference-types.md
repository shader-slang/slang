# Class handles and object mutation

Classes provide reference semantics in CPU and host code. A class value is a handle to an
object created with `new`. Copying or passing that value copies the handle, so the copies
refer to the same object.

An ordinary class method can modify the object's mutable fields without `[mutating]`:

```slang
class Counter
{
    int value;

    __init(int initialValue) { value = initialValue; }

    void increment() { value += 1; }

    property current : int
    {
        get { return value; }
        set { value = newValue; }
    }
}

void example()
{
    let counter = new Counter(10);
    let alias = counter;

    counter.increment();
    // alias.current is now 11: both handles name the same object.

    counter.current = 20;
    // alias.current is now 20.
}
```

## The handle and its object have separate mutability

`let counter` prevents replacing the handle stored in `counter`. It does not make the
referenced object immutable. Mutable fields, including fields of a struct stored inside
the object, can be updated through that handle. The same rule applies to implicit field
references inside methods, explicit `this.field` accesses, concrete class extensions,
generic class instances, and class handles returned by functions.

The field or property must still permit the write. A field declared with `let` remains
immutable, and a property with only a getter cannot be assigned. An ordinary class method
cannot assign a different object to `this`. A class handle stored in a `let` binding cannot
be rebound to a different object either.

These rules do not change struct semantics. A struct is a value, so a method that modifies
its own fields still needs `[mutating]` and an appropriate writable receiver. A struct may
contain a class handle; updating the object reached through that handle does not replace
the struct field or mutate the handle itself.

Existing `[mutating]` annotations on class methods remain accepted, but are unnecessary
for changing object fields. Here, a nonmutating receiver means that the receiver value
does not need to be written back to the caller. It does not promise that the method is
pure or that objects reachable through references will remain unchanged.

## Interfaces that operate on referenced objects

Use an ordinary interface method when an operation updates referenced storage without
replacing its receiver. For example:

```slang
interface IPositioned
{
    property position : float2 { get; }
    void setPosition(float2 value);
}

class Widget : IPositioned
{
    float2 _position;

    __init() { _position = float2(0.0, 0.0); }

    property position : float2 { get { return _position; } }

    void setPosition(float2 value) { _position = value; }
}

void moveTo<T : IPositioned>(T object, float2 position)
{
    object.setPosition(position);
}

void example()
{
    let widget = new Widget();
    moveTo(widget, float2(10.0, 20.0));
    // widget.position is now (10, 20).
}
```

Both `setPosition` declarations take their implicit receiver by value. For `Widget`, that
value is a handle, and the method updates the original object. `moveTo` therefore does
not need an `inout` parameter. Passing a class handle through an interface value has the
same object-identity behavior.

An interface does not implicitly become a reference type contract for every implementation.
A conforming struct cannot use the ordinary `setPosition` requirement to change its own
value fields and expect the caller's copy to change. It can implement the operation using
referenced storage instead. Requirements intended to mutate a value-type receiver still
need the existing mutating contract. An interface property setter is mutating by default;
it does not express the same receiver contract as the ordinary method above.

## Initialize dependencies before invoking setters

A class setter or method may read other fields while updating the object. Initialize all
of those dependencies before calling it from a constructor. For example, a text object's
setter that rebuilds a font layout needs both the text and the font to be initialized.
Assign the backing fields first, then build the initial layout once. Later setters should
invalidate any cached result before rebuilding it.
