// Tests the `extra` warning group via `keyword-used-as-name` (E20103), which is tagged `extra` and
// is ON by default. Each directive is a separate slangc run. Runs that expect the warning share the
// exhaustive `WARN` buffer (its annotations assert the warning IS emitted and that nothing else is);
// the run that suppresses it uses the `OFF` buffer, which has no annotations and therefore asserts
// that NOTHING is emitted (exhaustive matching would fail on any unannotated diagnostic).
//
// The independent `pedantic` group (off by default) is covered in warning-levels-pedantic.slang.

// 1. Default: the extra group is on, so the warning is emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=WARN): -target hlsl -entry computeMain -stage compute

// 2. -Wextra is redundant with the default (extra already on); still emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=WARN): -Wextra -target hlsl -entry computeMain -stage compute

// 3. -Wall / -Wpedantic enable *other* groups; the independent extra group stays on, so the warning
// is still emitted (the groups are not nested).
//DIAGNOSTIC_TEST:SIMPLE(diag=WARN): -Wall -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=WARN): -Wpedantic -target hlsl -entry computeMain -stage compute

// 4. An explicit per-id disable suppresses it even though its group is on.
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF): -Wno-keyword-used-as-name -target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

static int enum = 0;
//WARN:    ^^^^ keyword used as a name
//WARN:    ^^^^ 'enum' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
