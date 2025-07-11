//TEST:LANG_SERVER(filecheck=CHECK):

// Test that completion candidates are sorted according to context.

int myHelper() { return 0; }


void test(int p)
{
//COMPLETE:11,17
    int x = p + 

}

// CHECK: p:{{.*}}sort(1:p)
// CHECK: myHelper:{{.*}}sort(1:myHelper)