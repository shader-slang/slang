// Tests the warning-level (group) mechanism. `keyword-used-as-name` (E20103) is tagged as a
// "pedantic" group warning. The pedantic group is enabled by default for now, so the warning is
// still emitted by default; -Wpedantic is therefore redundant here but must be accepted, and an
// explicit -Wno-<id> must still suppress the warning. Each directive is a separate slangc run; the
// two runs that emit the warning share the `ON` check buffer.

// 1. Default: pedantic group is on by default, so the warning is emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -target hlsl -entry computeMain -stage compute

// 2. -Wpedantic explicitly enables the (already-on) pedantic group; still emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -Wpedantic -target hlsl -entry computeMain -stage compute

// 3. -Wall / -Wextra exercise the other two flag spellings; they enable different groups, so the
// pedantic warning is unaffected (and still emitted, since pedantic is on by default).
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -Wall -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -Wextra -target hlsl -entry computeMain -stage compute

// 4. All three flags on one command line: each exact spelling wins over the -W<id> prefix, and
// the WarningLevel option is additive (allowDuplicate), so passing several does not error or
// collapse. The pedantic warning is still emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -Wall -Wextra -Wpedantic -target hlsl -entry computeMain -stage compute

// 5. An explicit per-id disable suppresses it regardless of the group being enabled.
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF): -Wno-keyword-used-as-name -target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

static int enum = 0;
//ON:      ^^^^ keyword used as a name
//ON:      ^^^^ 'enum' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
