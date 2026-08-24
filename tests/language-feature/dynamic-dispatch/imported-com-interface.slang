//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain

// An interface owned by another module is lowered as a declaration and filled in
// by `prelinkIR`. That declaration carries almost none of the decorations the
// full definition has, because the definition arrives before anything reads it --
// with one exception. `visitInheritanceDecl` reads `IRComInterfaceDecoration` off
// the interface while lowering the witness table for the conformance below, and
// that happens *before* prelink runs. A declaration without it silently takes the
// wrong branch and the witness table is not kept alive.
//
// The emitted GUID is the observable end of that chain: it can only be correct if
// the decoration and its GUID survived the module boundary.

import "imported-com-interface-lib";

class Impl : IComService
{
    __init() {}

    float query(float x) { return x * 3.0; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IComService s = new Impl();
    outputBuffer[0] = s.query(2.0);
}

// The GUID from the imported module, reproduced on the borrowed declaration.
//CHECK: SLANG_COM_INTERFACE(0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB })
