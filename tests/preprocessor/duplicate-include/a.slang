//TEST:INTERPRET(filecheck=CHECK):

// Test that it is OK to include the same file multiple times.

#include "b.slang"

#include "b.slang"

void main()
{
    // CHECK: result: 23
    printf("result: %d\n", foo());
}
