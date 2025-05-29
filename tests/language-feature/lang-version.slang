
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -std 2026
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2026
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2026_2
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_LATEST1
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_LATEST2
//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv
//TEST:SIMPLE(filecheck=ILLEGAL_SLANG_VER): -target spirv -DVER_ILLEGAL
//TEST:SIMPLE(filecheck=ILLEGAL_SLANG_VER2): -target spirv -DVER_ILLEGAL2
//TEST:SIMPLE(filecheck=ILLEGAL_LANG): -target spirv -DLANG_ILLEGAL

#ifdef VER_2026
#language slang 2026
#endif

#ifdef VER_2026_2
#language 2026
#endif

#ifdef VER_LATEST1
#lang slang latest
#endif

#ifdef VER_LATEST2
#lang latest
#endif

#ifdef VER_ILLEGAL
// ILLEGAL_SLANG_VER: ([[# @LINE+1]]): error 15207
#lang slang illegal
#endif

#ifdef VER_ILLEGAL2
// ILLEGAL_SLANG_VER2: ([[# @LINE+1]]): error 15207
#lang 9987
#endif

#ifdef LANG_ILLEGAL
// ILLEGAL_LANG: ([[# @LINE+1]]): error 15207
#lang illegal
#endif

// CHECK_2026: OpEntryPoint

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK_LEGACY: ([[# @LINE+1]]): error 30027
    let x = (1,2,3)._1_2;
    
}