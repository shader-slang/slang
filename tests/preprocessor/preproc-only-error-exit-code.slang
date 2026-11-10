// Preprocessor-only compilation (`-E`) writes the preprocessed text even when preprocessing
// reports an error, and it fails with the same exit code as any other compilation with errors,
// so that a build that runs `slangc -E` sees the error. A warning does not fail it unless
// `-warnings-as-errors` turns it into an error. Code that is invalid only to the parser does not
// fail it either, because `-E` stops after preprocessing.

//TEST:SIMPLE(filecheck=ERROR,exit-code=-1):-E -DTRIGGER_ERROR
//TEST:SIMPLE(filecheck=WARN,exit-code=0):-E
//TEST:SIMPLE(filecheck=WARN_AS_ERROR,exit-code=-1):-E -warnings-as-errors all
//TEST:SIMPLE(filecheck=PARSE_ERROR,exit-code=0):-E -DTRIGGER_PARSE_ERROR

#ifdef TRIGGER_ERROR
#error preprocessing failed
#elif defined(TRIGGER_PARSE_ERROR)
int parseError = ;
#else
#warning preprocessing warned
#endif

int preprocessedText;

// ERROR: error[E15900]
// ERROR: #error: preprocessing failed
// ERROR: standard output = {
// ERROR-NEXT: int preprocessedText ;

// WARN: warning[E15901]
// WARN: #warning: preprocessing warned
// WARN: standard output = {
// WARN-NEXT: int preprocessedText ;

// WARN_AS_ERROR: error[E15901]
// WARN_AS_ERROR: #warning: preprocessing warned
// WARN_AS_ERROR: standard output = {
// WARN_AS_ERROR-NEXT: int preprocessedText ;

// PARSE_ERROR: standard error = {
// PARSE_ERROR-NEXT: }
// PARSE_ERROR: standard output = {
// PARSE_ERROR-NEXT: int parseError = ;
// PARSE_ERROR: int preprocessedText ;
