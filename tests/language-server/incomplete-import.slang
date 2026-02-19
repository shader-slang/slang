//TEST:LANG_SERVER(filecheck=CHECK):

// Language server should not crash when encountering an incomplete import
// statement, and should gracefully handle it without crashing.

//HOVER:4,1
import

// CHECK: null
