//TEST:LANG_SERVER(workspace-search-toggle, filecheck=CHECK):

import lspAdditionalSearchPathModule;

void useDependency()
{
//COMPLETE:8,8
    get
//DISABLE_WORKSPACE_SEARCH
//COMPLETE:8,8
}

// CHECK: getConfiguredWorkspacePathValue
// CHECK: searchInAllWorkspaceDirectories: false
// CHECK-NOT: getConfiguredWorkspacePathValue
