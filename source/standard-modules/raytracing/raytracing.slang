/**
Experimental structural pipeline ray tracing API.

The initial module exposes source-level type contracts only. Compiler-backed stage inputs,
descriptor resources, trace operations, and target adapters are added with their corresponding
lowering implementations.
*/
[ExperimentalModule]
module raytracing;

__include "ray-types";
__include "contexts";
__include "stage-inputs";
__include "stage-contracts";
__include "program-layout";
