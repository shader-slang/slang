//TEST:LANG_SERVER(filecheck=CHECK):

// A `cbuffer` inside `__generic` is a parse error, but the language server still checks the
// module afterwards. The redeclaration check for buffer-block members must not crash when
// the transparent variable is the inner declaration of a generic (shader-slang/slang#13876).
//
// In Debug builds, an existing assertion (`foundParent` in `ASTBuilder::getMemberDeclRef`)
// stops checking this module before the redeclaration check runs, and the hover is empty. In
// Release builds, checking reaches that check and the hover shows `afterBuffer`. Either answer
// shows that the language server did not crash.

__generic<T>
cbuffer GenericBuffer { int member; }

static const int afterBuffer = 1;

//HOVER:15,18
// CHECK: {{null|afterBuffer}}
