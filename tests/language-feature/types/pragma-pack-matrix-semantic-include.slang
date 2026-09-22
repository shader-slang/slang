//TEST_IGNORE_FILE:

implementing pragma_pack_matrix_test;

// Semantic includes are preprocessed independently, so this declaration uses the command-line
// column-major default instead of the including file's row-major default.
cbuffer SemanticIncludeInitial
{
    float2x3 semanticIncludeInitial;
    float semanticIncludeInitialTail;
}

#pragma pack_matrix(row_major)
static float2x3 semanticIncludeRow;

// This final state must not change the default in the including source.
#pragma pack_matrix(column_major)
