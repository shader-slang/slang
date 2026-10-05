//TEST:SIMPLE(filecheck=CHECK): -Gec -no-codegen

// Substituting the field type creates a different Box specialization at every level.
// Copy classification must stop so that ordinary checking can diagnose excessive nesting.
struct Box<T> { Box<Box<T> > next; };
uniform Box<uint> value;

// CHECK: maximum type nesting level exceeded
