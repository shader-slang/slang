//TEST:LANG_SERVER(filecheck=CHECK):

// Language server should not crash when encountering an incomplete __include
// statement, and should gracefully handle it without crashing.

//HOVER:4,1
__include

// CHECK: null
