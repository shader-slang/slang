//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// trying to include a non-existant file

#include "does-not-exist.h"
//diag:  ^^^^^^^^^^^^^^^^^^ include file not found
//diag:  ^^^^^^^^^^^^^^^^^^ failed to find include file 'does-not-exist.h'
