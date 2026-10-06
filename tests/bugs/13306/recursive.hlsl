//TEST:SIMPLE(filecheck=CHECK): -Gec -no-codegen

// Substitution of the field type creates a different `Box` specialization at every level.
// Type-tag computation must stop inspecting this graph before exhausting the compiler stack.
// Ordinary type validation must still report the excessive nesting.
struct Box<T> { Box<Box<T> > next; };
uniform Box<uint> value;

// CHECK: maximum type nesting level exceeded
