// entry-point-conflicting-stage.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage vertex -stage fragment

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-entry vsMain -stage compute -stage vertex
