//TEST:INTERPRET(filecheck=CHECK):

#include "b.slang"

#include "b.slang"

void main()
{
    // CHECK: result: 23
    printf("result: %d\n", foo());
}
