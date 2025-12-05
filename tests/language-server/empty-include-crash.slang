//TEST:LANG_SERVER(filecheck=CHECK):

// This test verifies that the language server doesn't crash
// when encountering an empty #include directive
// See issue #9167

#include

void test() {
//HOVER:11,9
    int x = 0;
}

// The important part is that the test doesn't crash
// CHECK: int x
