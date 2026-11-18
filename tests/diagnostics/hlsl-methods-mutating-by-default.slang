//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-methods-mutating-by-default.slang -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang slang tests/diagnostics/hlsl-methods-mutating-by-default.slang -fhlsl-methods-mutating-by-default -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/hlsl-methods-mutating-by-default.slang -fhlsl-methods-mutating-by-default -no-codegen
//TEST:SIMPLE_EX: -lang hlsl tests/diagnostics/hlsl-methods-mutating-by-default.slang -fhlsl-methods-mutating-by-default -no-codegen

// Only HLSL with the opt-in allows an unannotated method to modify its receiver.
// The default HLSL policy and both other dialects require an explicit receiver-mode attribute.

struct Counter
{
    int value;

    void increment()
    {
        value = value + 1;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }
};
