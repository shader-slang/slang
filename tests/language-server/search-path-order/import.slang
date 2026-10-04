//TEST:LANG_SERVER(workspace-search-path-order, filecheck=CHECK):

import lspSearchPathOrderModule;

void useDependency()
{
//COMPLETE:8,13
    getWorkspace
}

// CHECK: getWorkspaceFirstRootValue
// CHECK-NOT: getWorkspaceSecondRootValue
