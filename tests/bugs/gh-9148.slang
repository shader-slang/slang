//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK1): -target spirv -O3
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK2): -target spirv -O3

// Test for issue #9148: passing a pointer to a buffer element containing
// a fixed-size array as a function parameter should not assert due to
// mismatched field keys between logical and storage array wrapper structs.

// CHECK1-NOT: assert failure: resultType
// CHECK2-NOT: field_address requires base and result to have same address space

struct foo {
    uint64_t vals[64];
};

RWStructuredBuffer<foo> buf;

uint64_t bar(foo* f) {
    return f->vals[0];
}

[numthreads(1, 1, 1)]
[shader("compute")]
void main() {
    bar(&buf[0]);
}
