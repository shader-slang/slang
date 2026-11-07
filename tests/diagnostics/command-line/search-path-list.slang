//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MISSING_FILE):-search-path-list tests/diagnostics/command-line/missing-search-paths.txt
//CHECK_MISSING_FILE: cannot load search path list
//CHECK_MISSING_FILE: Cannot read search path list: tests/diagnostics/command-line/missing-search-paths.txt

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MISSING_ARGUMENT):-search-path-list
//CHECK_MISSING_ARGUMENT: expected an argument for command-line option '-search-path-list'
