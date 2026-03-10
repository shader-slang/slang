// bracketed-attribute-list-placement.slang

// Tests diagnostics for deprecated/removed bracket attribute list placement

//TEST:SIMPLE(filecheck=CHECK):-std 2018 -stage compute -entry main -target hlsl -DERRTEST=0
//DIAGNOSTIC_TEST:SIMPLE(diag=WARN):-std 2025 -stage compute -entry main -target hlsl -DERRTEST=0
//DIAGNOSTIC_TEST:SIMPLE(diag=ERR):-std 2026 -stage compute -entry main -target hlsl -DERRTEST=1

#if ERRTEST

// Error expected on Slang >=2026
struct [__NonCopyableType] TestType
//ERR: ^ invalid bracketed attribute list placement. Bracketed attributes must be placed before 'struct'.
//ERR: ^ invalid placement of bracketed attributes list.
{
    int iParam;
};

#else

// Deprecation warning expected on Slang 2025
struct [__NonCopyableType] TestType
//WARN:^ deprecated bracketed attribute list placement. Bracketed attributes should be placed before 'struct'.
//WARN:^ deprecated placement of bracketed attributes list.
{
    int iParam;
};

#endif

// No diagnostics expected before Slang 2025
//CHECK: result code = 0
//CHECK-NEXT: standard error = {
//CHECK-NEXT: }

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
