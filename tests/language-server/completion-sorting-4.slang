//TEST:LANG_SERVER(filecheck=CHECK):

// Test that completion candidates are sorted according to context.

int myHelper() { return 0; }


void test(int p)
{
//COMPLETE:11,13
    int x = 

}

// CHECK: p:{{.*}}sort(1:p)
// CHECK: myHelper:{{.*}}sort(1:myHelper)