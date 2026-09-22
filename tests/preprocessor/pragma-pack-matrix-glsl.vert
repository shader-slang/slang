//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

#pragma pack_matrix(row_major)
//CHECK:^^^^^^^^^^^ ignoring unknown directive '#pragma pack_matrix'
