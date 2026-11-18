// error-in-nested-import.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):

// CHECK: error[E20001]: unexpected token
// CHECK: error[E39999]: import failed due to compilation error
import a;

int main()
{}