//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import "autodiff-imported-dyn-interface-bwd-accessibility-lib";

// Buffer encodes a serialized Foo existential: tag=1, no payload.
//TEST_INPUT: set fPtr = ubuffer(data=[0 0 1 0], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: type_conformance Bar:IFoo = 2;
uniform IFoo* fPtr;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4);

[numthreads(1, 1, 1)]
void computeMain()
{
    var grad = diffPair<float>({});
    __bwd_diff(add)(fPtr[0], grad);
    // CHECK: 10
}
