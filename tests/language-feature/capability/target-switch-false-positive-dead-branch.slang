// Regression test for a bug where calling a function that itself contains a
// __target_switch with only a `case _sm_6_10:` arm (no fallback) inside one
// outer __target_switch case causes sibling cases (e.g. `case hlsl:`) to be
// rejected even when compiling for a profile that does not include sm_6_10.
//
// The pattern that triggers the bug:
//
//   [require(hlsl, sm_6_10)]
//   int helper() {
//       __target_switch {
//       case _sm_6_10:         // <-- no fallback case
//           __intrinsic_asm "...";
//       }
//   }
//
//   int myFunc()
//   {
//       __target_switch
//       {
//       case _sm_6_10:
//           return helper();       // <-- helper's inner switch leaks out
//       case hlsl:
//           __intrinsic_asm "..."; // <-- should be reachable with cs_6_9
//       }
//   }
//
// Expected:   cs_6_9  selects `case hlsl:`,    output contains "SM69_RESULT"
//             cs_6_10 selects `case _sm_6_10:`, output contains "SM610_RESULT"
// Observed (bug): cs_6_9 errors with "__target_switch has no compatable target"

//TEST:SIMPLE(filecheck=SM69):-target hlsl -profile cs_6_9 -entry main
//TEST:SIMPLE(filecheck=SM610):-target hlsl -profile cs_6_10 -entry main

RWStructuredBuffer<int> output;

// A helper that is only available on SM 6.10.
// Crucially, its body uses __target_switch with *only* a `case _sm_6_10:` arm
// and no fallback — mirroring the structure of __MatMul_linalg in hlsl.meta.slang.
[require(hlsl, sm_6_10)]
int helper610()
{
    __target_switch
    {
    case _sm_6_10:
        __intrinsic_asm "SM610_RESULT";
    }
}

int myFunc()
{
    __target_switch
    {
    case _sm_6_10:
        // Calling helper610() here should NOT prevent the `case hlsl:` branch
        // below from being selected when compiling for cs_6_9.
        return helper610();
    case hlsl:
    case _sm_6_9:
        // This branch should be selected for any HLSL profile, including cs_6_9.
        __intrinsic_asm "return SM69_RESULT";
    }
}

[numthreads(1, 1, 1)]
void main()
{
    output[0] = myFunc();
}

// SM69: SM69_RESULT
// SM610: SM610_RESULT
