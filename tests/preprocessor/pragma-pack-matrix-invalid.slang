//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

#pragma pack_matrix(invalid)
//CHECK: expected 'row_major' or 'column_major' in 'pragma' directive

#pragma pack_matrix row_major
//CHECK: expected ''('' in 'pragma' directive

#pragma pack_matrix(row_major
//CHECK: expected '')'' in 'pragma' directive

#pragma pack_matrix(row_major) extra
//CHECK: unexpected tokens following 'pragma' directive
