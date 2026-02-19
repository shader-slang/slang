//TEST:LANG_SERVER(filecheck=CHECK):

// Language server should not crash when encountering an incomplete implementing
// statement, and should gracefully handle it without crashing.

//HOVER:4,1
implementing

// CHECK: null
