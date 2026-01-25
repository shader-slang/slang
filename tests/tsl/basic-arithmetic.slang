//TEST:SIMPLE(filecheck=TSL): -target tsl

// Test that TSL target generates proper JavaScript/TSL output

// TSL: import {
// TSL: } from 'three/tsl';

// TSL: export const main = /*@__PURE__*/ Fn

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
}
