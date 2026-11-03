// error-in-nested-import.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):

// CHECK: error
import a;

int main()
{}