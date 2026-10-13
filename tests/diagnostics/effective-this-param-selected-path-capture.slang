//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Semantic checking provisionally represents the lambda's captured existential receiver before
// overload resolution selects the method call. Selected-path finalization must attach that capture
// and validate the selected mutating receiver use.

interface IMutable
{
    [mutating]
    void mutate();
}

void checkExistentialReceiverAfterCapture(IMutable value)
{
    let call = () => value.mutate();
//CHECK:                         ^ mutating method cannot be called on immutable value
//CHECK:                         ^ mutating method 'mutate' cannot be called on an immutable value
    call();
}
