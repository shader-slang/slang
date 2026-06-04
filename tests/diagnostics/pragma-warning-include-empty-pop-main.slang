//TEST:SIMPLE(filecheck=CHECK):

module pragma_warning_include_empty_pop;

#pragma warning(disable: 30856)

__include "pragma-warning-include-empty-pop-body.slang";
__include "pragma-warning-include-empty-pop-footer.slang";

// CHECK-NOT: warning 30856
