//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target cpp -entry computeMain -stage compute

// Regression test for https://github.com/shader-slang/slang/issues/11778
//
// A local that is accumulated across loop iterations (loop-carried) is
// uninitialized on the first iteration. After SSA construction the variable
// becomes a loop-header phi merging the undefined pre-loop value with each
// iteration's update, so the first-iteration read happens through that phi.
// The uninitialized-use analysis must follow the undefined value through the
// phi to report the read.

RWStructuredBuffer<float> outputBuffer;

void normalize_array(inout float a[3])
{
    float total;
    for (int i = 0; i < 3; i++)
        total += a[i];
//CHK:        ^^ use of uninitialized variable
//CHK:        ^^ use of uninitialized variable 'total'
    for (int i = 0; i < 3; i++)
        a[i] /= total;
//CHK:       ^^ use of uninitialized variable
//CHK:       ^^ use of uninitialized variable 'total'
}

// Nested loops produce two loop-header phis, so the undefined value flows
// var -> outer-header-phi -> inner-header-phi. This exercises the multi-hop phi
// following and the cycle guard on both back-edges.
float nested_loop_accum(float a[2][3])
{
    float total;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            total += a[i][j];
//CHK:            ^^ use of uninitialized variable
//CHK:            ^^ use of uninitialized variable 'total'
    return total;
/*CHK:
    ^^^^^^ use of uninitialized variable
    ^^^^^^ use of uninitialized variable 'total'
*/
}

// Must NOT warn: the accumulator is initialized before the loop. After SSA it is
// still a loop-header phi, but the preheader phi argument is the initializing
// constant rather than an undefined value, so phi following finds no undefined
// source. Exhaustive diagnostic checking fails the test if this over-warns.
float sum_initialized(float a[3])
{
    float total = 0;
    for (int i = 0; i < 3; i++)
        total += a[i];
    return total;
}

struct Attrs { float v; };

// Must NOT warn (and must not be a never-initialized may-init report): `committed` is
// assigned only on some loop paths and then read after the loop. The loop-header phi
// merges the undefined pre-loop value with the genuinely-defined `cand`, so a real
// definition reaches the merge. That makes this a conditional (must-init) case, which the
// definite-assignment pass suppresses for a possibly-zero-trip loop -- the same shape as a
// RayQuery commit-then-read-committed loop. This pins the boundary that phi following does
// not turn such a read into a spurious "use of uninitialized variable".
float cond_assigned_in_loop(float a[3], int n)
{
    Attrs committed;
    for (int i = 0; i < n; i++)
    {
        if (a[i] > 0)
        {
            Attrs cand = { a[i] };
            committed = cand;
        }
    }
    return committed.v;
}

// A `do`-while loop lowers to a different loop topology than `for` (the body is the loop
// target and is entered unconditionally on the first iteration), so the undefined value
// reaches the body phi along the preheader edge in a structurally distinct shape. The
// first-iteration read of the accumulator is still a genuine uninitialized use.
float do_while_accum(int n)
{
    float total;
    int i = 0;
    do
    {
        total += float(i);
//CHK:        ^^ use of uninitialized variable
//CHK:        ^^ use of uninitialized variable 'total'
        i++;
    } while (i < n);
    return total;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a[3] = {1, 2, 3};
    normalize_array(a);
    float b[2][3] = {{1, 2, 3}, {4, 5, 6}};
    outputBuffer[0] = a[0] + nested_loop_accum(b) + sum_initialized(a) +
                      cond_assigned_in_loop(a, 3) + do_while_accum(3);
}
