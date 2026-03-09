// bracketed-attribute-list-placemenet.slang

// Tests diagnostics for deprecated/removed bracket attribute list placement

//DIAGNOSTIC_TEST:SIMPLE(diag=WARN):-std 2025 -stage compute -entry main -target hlsl -DERRTEST=0
//DIAGNOSTIC_TEST:SIMPLE(diag=ERR):-std 2026 -stage compute -entry main -target hlsl -DERRTEST=1

#if ERRTEST

struct [__NonCopyableType] TestType
//ERR: ^ invalid bracketed attribute placement. Bracketed attributes must be placed before 'struct'.
//ERR: ^ invalid placement bracketed attributes list.
{
    int iParam;
};

#else

struct [__NonCopyableType] TestType
//WARN:^ deprecated bracketed attribute placement. Bracketed attributes should be placed before 'struct'.
//WARN:^ deprecated placement of bracketed attributes list.
{
    int iParam;
};

#endif

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
