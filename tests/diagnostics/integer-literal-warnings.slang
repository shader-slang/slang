//TEST:SIMPLE(filecheck=CHECK): -target dxbc-assembly

// Test that UINT64_MAX literals produce appropriate warnings
// This test verifies the fix from commit 0a3c1c8dbd6aa9501ea94a8de69d1a5967034b2c
// which ensures literals equivalent to UINT64_MAX produce appropriate warnings

void main()
{
    // Decimal form of UINT64_MAX
    // CHECK: warning 39999: integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    uint64_t a = 18446744073709551615;
    
    // Hex form of UINT64_MAX
    // CHECK: warning 39999: integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    uint64_t b = 0xFFFFFFFFFFFFFFFF;
    
    // Assigning UINT64_MAX to int - should also warn about size
    // CHECK: warning 39999: integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    int c = 18446744073709551615;
    
    // Also hex form to int
    // CHECK: warning 39999: integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    int d = 0xFFFFFFFFFFFFFFFF;
} 