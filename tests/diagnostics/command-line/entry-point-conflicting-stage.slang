// entry-point-conflicting-stage.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-stage vertex -stage fragment
//CHECK1 conflicting stages have been specified for entry point 'main'

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-entry vsMain -stage compute -stage vertex
//CHECK2 conflicting stages have been specified for entry point 'vsMain'
