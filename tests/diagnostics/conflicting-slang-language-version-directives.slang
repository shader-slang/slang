// A source segment cannot select two module-wide Slang language versions.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language slang 2025
#language slang 2026
//CHECK: conflicting Slang language-version directives in one translation unit
//CHECK: this directive selects a different Slang language version
//CHECK: the translation unit's first Slang language-version directive is here

void main()
{}
