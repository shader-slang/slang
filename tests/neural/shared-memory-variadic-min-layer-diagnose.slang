//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

import slang.neural;

typealias TooFewLayers =
    SharedMemorySize<float, TargetEnum.CUDA, ExecutionMode.Inference, 32, 2, 64>;
/*CHECK:
    ^^^^^^^^^^^^^^^^ cannot specialize generic
    ^^^^^^^^^^^^^^^^ cannot specialize generic 'SharedMemorySize' with the provided arguments.
*/
