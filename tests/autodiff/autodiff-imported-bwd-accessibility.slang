//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import "autodiff-imported-bwd-accessibility-lib";

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4);

[numthreads(1, 1, 1)]
void computeMain()
{
    var grad = diffPair<float>({});
    __bwd_diff(add)(grad);
    // CHECK: 10
}
