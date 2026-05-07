//DIAGNOSTIC_TEST:SIMPLE: -target spirv -no-codegen

// Regression test: SIGSEGV on malformed function bodies after parse error recovery
// Issue: https://github.com/shader-slang/slang/issues/10248
// Found by AFL++ fuzzing

// Pattern 1: function body with %( and commas
// t s(){%(,,,,,,,,,,,,,,,,

// Pattern 2: (struct: cast syntax
// t e(){(struct:

// Pattern 3: __intrinsic_asm at end of body
// t s(){__intrinsic_asm

// Minimal reproducer (pattern 3):
t s(){__intrinsic_asm
