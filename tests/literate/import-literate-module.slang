//TEST:INTERPRET(filecheck=CHECK):

import literate_helper;

void main()
{
    // CHECK: 42
    printf("%d\n", literateAdd(40, 2));
}
