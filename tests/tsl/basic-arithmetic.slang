//TEST:SIMPLE(filecheck=TSL): -target tsl -stage compute -entry main

// Test that TSL target generates proper JavaScript/TSL output with arithmetic

// Check TSL imports are generated
// TSL: import {
// TSL: } from 'three/tsl';

// Check entry point is exported as TSL Fn
// TSL: export const main = /*@__PURE__*/ Fn

// Check arithmetic operations use TSL function calls
// TSL: add(
// TSL: mul(
// TSL: sub(

RWStructuredBuffer<float> outputBuffer;

void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    // Arithmetic operations - should emit TSL function calls
    float a = float(idx);        // use idx to prevent constant folding
    float b = float(idx + 1u);

    float sum = a + b;           // add(a, b)
    float product = a * b;       // mul(a, b)
    float diff = a - b;          // sub(a, b)

    outputBuffer[idx] = sum + product + diff;
}
