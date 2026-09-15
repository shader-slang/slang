//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Functional smoke test for shader-slang/slang#12604: the specialization work-list's
// forward-use-closure walk is now iterative (`expandUseClosure`) instead of mutually
// recursive, so it descends a long linear dependent chain without recursing once per level.
// This builds a modest 64-deep chain (each v_i uses v_(i-1)) to exercise that multi-level
// descent and confirm the output is still correct. It does NOT force the stack overflow the
// former mutual recursion risked: that threshold is platform-dependent (thousands of frames
// on an 8 MB stack) and cannot be triggered portably from a committed CPU test, so this is
// coverage of the iterative walk, not a fail-on-revert stack test. The seed is read from a
// buffer so the front-end cannot constant-fold the chain away before specialization runs.

//TEST_INPUT:ubuffer(data=[1056964608], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float v0 = inputBuffer[0];
    float v1 = v0 + 1.0;
    float v2 = v1 + 1.0;
    float v3 = v2 + 1.0;
    float v4 = v3 + 1.0;
    float v5 = v4 + 1.0;
    float v6 = v5 + 1.0;
    float v7 = v6 + 1.0;
    float v8 = v7 + 1.0;
    float v9 = v8 + 1.0;
    float v10 = v9 + 1.0;
    float v11 = v10 + 1.0;
    float v12 = v11 + 1.0;
    float v13 = v12 + 1.0;
    float v14 = v13 + 1.0;
    float v15 = v14 + 1.0;
    float v16 = v15 + 1.0;
    float v17 = v16 + 1.0;
    float v18 = v17 + 1.0;
    float v19 = v18 + 1.0;
    float v20 = v19 + 1.0;
    float v21 = v20 + 1.0;
    float v22 = v21 + 1.0;
    float v23 = v22 + 1.0;
    float v24 = v23 + 1.0;
    float v25 = v24 + 1.0;
    float v26 = v25 + 1.0;
    float v27 = v26 + 1.0;
    float v28 = v27 + 1.0;
    float v29 = v28 + 1.0;
    float v30 = v29 + 1.0;
    float v31 = v30 + 1.0;
    float v32 = v31 + 1.0;
    float v33 = v32 + 1.0;
    float v34 = v33 + 1.0;
    float v35 = v34 + 1.0;
    float v36 = v35 + 1.0;
    float v37 = v36 + 1.0;
    float v38 = v37 + 1.0;
    float v39 = v38 + 1.0;
    float v40 = v39 + 1.0;
    float v41 = v40 + 1.0;
    float v42 = v41 + 1.0;
    float v43 = v42 + 1.0;
    float v44 = v43 + 1.0;
    float v45 = v44 + 1.0;
    float v46 = v45 + 1.0;
    float v47 = v46 + 1.0;
    float v48 = v47 + 1.0;
    float v49 = v48 + 1.0;
    float v50 = v49 + 1.0;
    float v51 = v50 + 1.0;
    float v52 = v51 + 1.0;
    float v53 = v52 + 1.0;
    float v54 = v53 + 1.0;
    float v55 = v54 + 1.0;
    float v56 = v55 + 1.0;
    float v57 = v56 + 1.0;
    float v58 = v57 + 1.0;
    float v59 = v58 + 1.0;
    float v60 = v59 + 1.0;
    float v61 = v60 + 1.0;
    float v62 = v61 + 1.0;
    float v63 = v62 + 1.0;
    float v64 = v63 + 1.0;
    outputBuffer[0] = v64; // CHECK: 64.5
}
