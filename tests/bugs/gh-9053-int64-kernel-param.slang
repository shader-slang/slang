//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain
// Test int64_t as kernel parameter with platform-aware typedef
// Verifies that kernel parameter uses 'longlong' type, which adapts to platform:
//   - On Linux: longlong = long (matches Linux's int64_t)
//   - On Windows: longlong = long long (matches Windows' int64_t)
// This ensures ABI compatibility between kernel and wrapper code on all platforms
//CHECK: computeMain(longlong value_{{[0-9]+}})

[numthreads(1, 1, 1)]
void computeMain(int64_t value)
{
    // Simple kernel that takes int64_t parameter
    // This should compile and match the platform's int64_t type
}
