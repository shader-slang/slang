// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime;

[host]
void main()
{
    makeRaytracingPipeline(1);
}

// CHECK: undefined identifier 'makeRaytracingPipeline'
