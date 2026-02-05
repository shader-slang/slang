// Test that isValidGenericConstraintType() properly validates constraint types
// Tests lines 289-297 in slang-check-conformance.cpp

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute

interface IFoo {
    int getValue();
}

interface IBar {
    int getOther();
}

struct MyStruct {
    int data;
}

RWStructuredBuffer<int> output;

// Valid: interface as constraint
void validInterfaceConstraint<T : IFoo>(T value) {
    output[0] = value.getValue();
}

// Valid: interface conjunction as constraint
void validConjunctionConstraint<T : IFoo & IBar>(T value) {
    output[0] = value.getValue() + value.getOther();
}

// Invalid: struct type as constraint (not an interface)
// CHECK: ([[# @LINE+1]]): error 30401
void invalidStructConstraint<T : MyStruct>(T value) {
    output[1] = value.data;
}

// Invalid: concrete type as constraint
// CHECK: ([[# @LINE+1]]): error 30401
void invalidConcreteConstraint<T : int>(T value) {
    output[2] = value;
}

// Invalid: array type as constraint
// CHECK: ([[# @LINE+1]]): error 30401
void invalidArrayConstraint<T : int[10]>(T value) {
    output[3] = value[0];
}

[numthreads(1, 1, 1)]
void computeMain() {
    // Just testing constraint validation
}
