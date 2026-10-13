//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

#lang 2026

// A bound method is selected as a callable even when a variable or aggregate delays its call.
// Selected-expression finalization must therefore validate the immutable receiver exactly once for
// each use below. Semantic checking propagates the selected callable through parentheses, typed
// wrappers, and variadic packaging without hiding the receiver or causing another
// `finalizeSelectedExpression` call.

typealias Callback = functype() -> int;

[__NonCopyableType]
struct Receiver
{
    [mutating]
    int read()
    {
        return 0;
    }
}

struct CallbackAggregate
{
    Callback callback;
}

int defaultCallback()
{
    return 0;
}

struct CallbackHolder
{
    property Callback callback
    {
        [mutating]
        get { return defaultCallback; }
    }
}

void acceptPack<each T>(expand each T arguments)
{}

void checkInferredBinding()
{
    let receiver = Receiver();
    let callback = receiver.read;
//CHECK:                    ^^^^ mutating method cannot be called on immutable value
//CHECK:                    ^^^^ mutating method 'read' cannot be called on an immutable value
//CHECK-NOT: mutating method
    callback();
}

void checkAggregateInitializer()
{
    let receiver = Receiver();
    CallbackAggregate aggregate = {receiver.read};
//CHECK:                                    ^^^^ mutating method cannot be called on immutable value
//CHECK:                                    ^^^^ mutating method 'read' cannot be called on an immutable value
//CHECK-NOT: mutating method
}

void checkParenthesizedArrayInitializer()
{
    let receiver = Receiver();
    Callback callbacks[1] = {(receiver.read)};
//CHECK:                     ^ mutating method cannot be called on immutable value
//CHECK:                     ^ mutating method 'read' cannot be called on an immutable value
//CHECK-NOT: mutating method
}

void checkVariadicArgumentPack()
{
    let receiver = Receiver();
    acceptPack(receiver.read);
//CHECK:                ^^^^ mutating method cannot be called on immutable value
//CHECK:                ^^^^ mutating method 'read' cannot be called on an immutable value
//CHECK-NOT: mutating method
}

void checkAsTypeWrapper()
{
    let receiver = Receiver();
    let maybeCallback = receiver.read as Callback;
//CHECK:                         ^^^^ mutating method cannot be called on immutable value
//CHECK:                         ^^^^ mutating method 'read' cannot be called on an immutable value
//CHECK-NOT: mutating method
}

void checkAsTypeWrapperWithMutableReceiver()
{
    var receiver = Receiver();
    let maybeCallback = receiver.read as Callback;
}

void checkFunctionTypedPropertyBinding()
{
    let holder = CallbackHolder();
    let callback = holder.callback;
//CHECK:                  ^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                  ^^^^^^^^ mutating method 'get' cannot be called on an immutable value
//CHECK-NOT: mutating method
    callback();
}
