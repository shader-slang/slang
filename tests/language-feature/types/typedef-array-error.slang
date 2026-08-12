//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// shader-slang/slang#11567: parseTypeDef now reads the alias name through the shared
// declarator parser (parseDeclarator -> expectIdentifier) instead of a bare
// ReadToken(Identifier). A typedef whose alias name is missing must still be diagnosed.

typedef int;
//CHECK:   ^ unexpected token
//CHECK:   ^ unexpected ';', expected identifier
