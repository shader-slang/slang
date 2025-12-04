//TEST:LANG_SERVER(filecheck=CHECK):

// This test verifies that the language server doesn't crash
// when encountering an empty #include directive
// See issue #9167

#include

void test() {
//HOVER:10,5
    int x = 0;
}

// The important part is that the test doesn't crash
// The hover may return null due to parse errors, which is acceptable
// CHECK: null
