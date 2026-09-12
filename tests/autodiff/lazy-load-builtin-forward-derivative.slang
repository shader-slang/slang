// Loading the supplement checks derivative registrations whose primals live in the deserialized
// base module. The synthesized forward extension must be owned by the supplement, and both the
// forward and backward builtin registrations must produce the expected derivative values.
//
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let forwardResult = fwd_diff(radians)(diffPair(0.0, 1.0));
    // CHECK: 0.017453
    printf("%f\n", forwardResult.d);

    var backwardInput = diffPair(0.0, 0.0);
    bwd_diff(radians)(backwardInput, 1.0);
    // CHECK-NEXT: 0.017453
    printf("%f\n", backwardInput.d);
}
