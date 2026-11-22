//TEST:SIMPLE: -fhlsl-methods-mutating-by-default -no-codegen

// The HLSL receiver default must not affect methods checked in an imported Slang module.
// The imported method is unannotated and remains callable on an immutable receiver.

import hlsl_methods_mutating_imported;

cbuffer C
{
    ImportedCounter counter;
};

int readCounter()
{
    return counter.read();
}
