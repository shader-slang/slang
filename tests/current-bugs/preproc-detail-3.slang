//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// Undefining a macro that is not defined within C/C++ is defined as *not* an error.
// On checking with DXC/FXC they also have this behavior.

#undef C