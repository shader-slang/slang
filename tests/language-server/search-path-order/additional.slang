//TEST:LANG_SERVER(additional-search-path-order, filecheck=CHECK):

import lspAdditionalSearchPathModule;

void useDependency()
{
//COMPLETE:8,14
    getConfigured
}

// CHECK: getConfiguredAdditionalPathValue
// CHECK-NOT: getConfiguredWorkspacePathValue
