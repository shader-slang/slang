//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

// Both out and inout parameters
// should have this treated as writes
void out_test(int tmp, out int x)
{
    x = tmp;
}

// Permuting arguments to ensure that
// the correct argument is checked
void inout_test(inout int x, int tmp)
{
    x = tmp;
}

// __ref parameters should also be fine
void ref_test(__ref int x, int tmp)
{
    x = tmp;
}

void undefined_function_use()
{
    int x;
    int tmp = 1;

    out_test(x, tmp);
//CHK:      ^ use of uninitialized variable
//CHK:      ^ use of uninitialized variable 'x'

    inout_test(tmp, x);
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'x'
}

void out_function_use()
{
    int x;
    int tmp = 1;

    // Acts as a write
    out_test(tmp, x);

    // Rest are fine now
    out_test(x, tmp);
    inout_test(tmp, x);
}

void inout_function_use()
{
    int x;
    int tmp = 1;

    // Acts as a write
    inout_test(x, tmp);

    // Rest are fine now
    out_test(x, tmp);
    inout_test(tmp, x);
}

void ref_function_use()
{
    int x;
    int tmp = 1;

    // Acts as a write
    ref_test(x, tmp);

    // Rest are fine now
    out_test(x, tmp);
    inout_test(tmp, x);
}

// Likewise for generic functions
static int ord_gen_result;

__generic<T>
void ordinary_generic(T x)
{
    ord_gen_result = __slang_noop_cast<int, T>(x);
}

__generic<T>
void unordinary_generic(out T x)
{
    x = __slang_noop_cast<T, int>(1);
}

void undefined_generic_use()
{
    int x;

    ordinary_generic(x);
//CHK:              ^ use of uninitialized variable
//CHK:              ^ use of uninitialized variable 'x'
}

void ok_generic_use()
{
    int x;
    unordinary_generic(x);
    ordinary_generic(x);
}

// A generic interface method lowers through an unresolved witness-method specialization while this
// diagnostic pass runs. Its call-site type still records each parameter direction. We use that
// signature to classify the first argument as a possible write and the second argument as a read,
// without requiring a concrete method body.
interface IGenericWriter
{
    __generic<T>
    void writeOut(out int value, T tag);

    __generic<T>
    void writeInOut(inout int value, T tag);

    __generic<T>
    void writeRef(__ref int value, T tag);
}

// Exercise by-value reads and writable-argument warning suppression on generic interface calls.
//
// The first case diagnoses the uninitialized tag. The remaining cases exercise the heuristic's
// possible-write classification without claiming that it proves definite initialization.
__generic<TWriter : IGenericWriter>
int useGenericInterfaceWriter(TWriter writer)
{
    float initializedInput = 0.0;

    // A by-value tag is read even when the other argument is an output destination.
    float uninitializedInput;
    int valueFromUninitializedInput;
    writer.writeOut(valueFromUninitializedInput, uninitializedInput);
//CHK:             ^ use of uninitialized variable
//CHK:             ^ use of uninitialized variable 'uninitializedInput'

    // The single-category heuristic treats each writable first argument as a possible write,
    // including `inout` and `__ref`; it does not separately record their incoming reads.
    int outValue;
    writer.writeOut(outValue, initializedInput);

    int inOutValue;
    writer.writeInOut(inOutValue, initializedInput);

    int refValue;
    writer.writeRef(refValue, initializedInput);

    // Later reads do not produce warnings after these possible writes. This checks suppression,
    // not a proof that the unresolved methods definitely initialize their arguments.
    return outValue + inOutValue + refValue;
}

// Check proper handling of aliases passed
void f()
{
    int3 dim;

    // Should have no warnings
    out_test(1, dim.x);
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}
