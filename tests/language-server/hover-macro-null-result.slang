//TEST:LANG_SERVER(filecheck=CHECK):

// Regression for #12869: an empty hover must serialize as JSON `null`, not `{}`,
// because `Hover.contents` is required/non-nullable.

//HOVER:7,4
#define MY_PI 3.14159

// CHECK: null
