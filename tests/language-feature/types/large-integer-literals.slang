// large-integer-literals.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test that large integer literals produce warnings when targeting SPIR-V

int test(int inVal)
{
    // Test the specific integer values that should produce warnings when targeting SPIR-V
    int x1 = 0xffffffffffff;
    // CHECK: tests/language-feature/types/large-integer-literals.slang([[#@LINE-1]]): warning 39999: integer literal '0xffffffffffff' too large for type 'int' truncated to '281474976710655'
    // CHECK: tests/language-feature/types/large-integer-literals.slang([[#@LINE-2]]): warning 30081: implicit conversion from 'int64_t' to 'int' is not recommended
    
    int x2 = -9223372036854775807;
    // CHECK: tests/language-feature/types/large-integer-literals.slang([[#@LINE-1]]): warning 30081: implicit conversion from 'int64_t' to 'int' is not recommended
    
    int x3 = 18446744073709551615;
    // CHECK: tests/language-feature/types/large-integer-literals.slang([[#@LINE-1]]): warning 39999: integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    
    int x4 = 0xFFFFFFFFFFFFFFFF;
    // CHECK: tests/language-feature/types/large-integer-literals.slang([[#@LINE-1]]): warning 39999: integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    
    // Use the values in a way that doesn't cause overflow
    // Just use the input value to ensure the function isn't optimized away
    int result = inVal;
    return result;
}
