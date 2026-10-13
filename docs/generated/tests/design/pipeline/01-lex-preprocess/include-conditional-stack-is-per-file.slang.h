// Helper header for include-conditional-stack-is-per-file.slang.
// It deliberately opens a conditional it never closes, so the parent
// file can show that the open conditional did not leak upward.
static const int per_file_value = 3;
#if 1
static const int per_file_inner = 4;
