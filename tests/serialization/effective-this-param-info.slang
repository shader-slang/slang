//TEST:COMPILE: tests/serialization/effective-this-param-info-module.slang -o tests/serialization/effective-this-param-info-module.slang-module
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -slang -compute -xslang -r -xslang tests/serialization/effective-this-param-info-module.slang-module -shaderobj
//TEST:SIMPLE(filecheck=IR): -r tests/serialization/effective-this-param-info-module.slang-module -no-codegen -dump-ir

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer

// Calling methods and differentiating a method after loading the binary module exercises
// deserialization of their effective `this` parameter information and its use by lookup,
// lowering, and autodiff.

import "effective-this-param-info-module";

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Receiver receiver = {1};
    int original = receiver.read();
    receiver.increment();
    outputBuffer[0] = original + receiver.value;

    DifferentiableReceiver differentiableReceiver = {2.0};
    let derivative = fwd_diff(DifferentiableReceiver::scale)(
        differentiableReceiver,
        diffPair(3.0, 1.0));
    outputBuffer[1] = int(derivative.p + derivative.d);
}

// BUF: 3
// BUF-NEXT: 8

// These imported signatures come from the serialized ThisParamInfoAttribute rather than from a
// fresh semantic check of the source declarations.
// IR-LABEL: [nameHint("Receiver.read")]
// IR-NEXT: func {{.*}} : Func(Int, BorrowInParam(%Receiver,
// IR-LABEL: [nameHint("Receiver.increment")]
// IR-NEXT: func {{.*}} : Func(Void, RefParam(%Receiver,

// `no_diff` is encoded in the effective parameter value type, separately from its mode. Keeping
// that modifier through serialization makes the derivative accept an ordinary receiver rather
// than a DifferentialPair<DifferentiableReceiver>.
// IR-LABEL: [nameHint("DifferentiableReceiver.scale.fwd_diff")]
// IR: Func(DiffPair(Float,
// IR-SAME: Attributed(%DifferentiableReceiver,
// IR-SAME: DiffPair(Float,
