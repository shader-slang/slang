//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain -g -verify-debug-serial-ir

// An `out` parameter definitely initializes its argument when the call returns normally.
void out_test(int tmp, out int x)
{
    x = tmp;
}

// An `inout` parameter makes the incoming value available to the callee and may write a replacement
// value back to the argument. Caller-side definite-assignment analysis therefore treats the
// argument as a read and only a possible write.
void inout_test(inout int x, int tmp)
{
    x = tmp;
}

// A `__ref` parameter has the same caller-side read and possible-write contract as `inout`.
void ref_test(__ref int x, int tmp)
{
    x = tmp;
}

void write_uint(out uint value)
{
    value = 5;
}

int out_implicit_cast_initializes()
{
    // The implicit l-value cast preserves the `out` parameter's definite-write contract for the
    // original variable.
    int value;
    write_uint(value);
    return value;
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

    // An `out` argument receives a complete value when the call returns.
    out_test(tmp, x);

    // Both calls can therefore read `x` without a diagnostic.
    out_test(x, tmp);
    inout_test(tmp, x);
}

void inout_function_use()
{
    int x;
    int tmp = 1;

    // Caller-side analysis uses the `inout` parameter contract; it does not inspect this helper's
    // body to prove that the assignment always executes. The first call therefore reads `x` while
    // it is uninitialized, and the possible write does not prove that `x` is initialized afterward.
    inout_test(x, tmp);
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'x'

    // These later reads can still observe an uninitialized `x`.
    out_test(x, tmp);
//CHK:      ^ possible use of uninitialized variable
//CHK:      ^ variable 'x' may be uninitialized on some paths; it is only conditionally assigned
    inout_test(tmp, x);
//CHK:        ^ possible use of uninitialized variable
//CHK:        ^ variable 'x' may be uninitialized on some paths; it is only conditionally assigned
}

void ref_function_use()
{
    int x;
    int tmp = 1;

    // Caller-side analysis likewise uses the `__ref` contract rather than proving that the helper's
    // assignment always executes.
    ref_test(x, tmp);
//CHK:      ^ use of uninitialized variable
//CHK:      ^ use of uninitialized variable 'x'

    // Its possible write does not prove that these later reads are safe.
    out_test(x, tmp);
//CHK:      ^ possible use of uninitialized variable
//CHK:      ^ variable 'x' may be uninitialized on some paths; it is only conditionally assigned
    inout_test(tmp, x);
//CHK:        ^ possible use of uninitialized variable
//CHK:        ^ variable 'x' may be uninitialized on some paths; it is only conditionally assigned
}

[Differentiable]
void differentiable_inout(inout float value)
{
    value = value;
}

void undefined_differentiated_inout_use()
{
    DifferentialPair<float> value;

    // The function produced by `__fwd_diff` preserves the primal function's `inout` parameter
    // direction. Calling it therefore reads `value` and does not definitely initialize `value`.
    __fwd_diff(differentiable_inout)(value);
//CHK:                              ^ use of uninitialized variable
//CHK:                              ^ use of uninitialized variable 'value'
}

[Differentiable]
void differentiable_out(out float value)
{
    value = 1.0;
}

float differentiated_out_initializes()
{
    DifferentialPair<float> value;

    // The corresponding `out` direction proves that the derivative call initializes the complete
    // differential pair.
    __fwd_diff(differentiable_out)(value);
    return value.p;
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
