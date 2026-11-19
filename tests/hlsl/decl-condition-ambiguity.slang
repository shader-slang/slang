//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type -xslang -Wno-30081

// Expression conditions that begin like a type still parse as expressions, and the
// modifier form still parses as a declaration. The tests use non-`bool` condition
// types, whose conversion to `bool` is exactly the feature under test, so the 30081
// "implicit conversion not recommended" warning is disabled rather than heeded.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// `a * b` parses as a multiply: after the trial type `a*` there is no `name =` shape
// (the token after `b` is not an `=`), so the trial is discarded.
int multiplyNotPointer()
{
    int a = 4;
    int b = 3;
    int r = 0;
    if (a * b)
        r = 1;
    return r;
}

// `a == b` stays an equality: the token after the name is `==`, not `=`.
int equalityNotAssignment()
{
    int a = 4;
    int b = 4;
    int r = 0;
    if (a == b)
        r = 2;
    return r;
}

// `count < 0` stays a comparison: the trial reads `count` as a type and `<` as a
// generic application, fails on the shadow sink, and nothing leaks into the real
// diagnostics.
int comparisonNotGeneric()
{
    int count = 0;
    int r = 0;
    if (count < 1)
        r = 3;
    return r;
}

// A modifier can precede the type in a declaration condition; both the trial and the
// real parse handle the modifiers.
int modifierForm()
{
    int c = 2;
    if (const int count = c)
        return count + 10;
    return 0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = multiplyNotPointer();
    outputBuffer[1] = equalityNotAssignment();
    outputBuffer[2] = comparisonNotGeneric();
    outputBuffer[3] = modifierForm();
}

// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 12
