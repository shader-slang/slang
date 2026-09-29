//TEST:LANG_SERVER(workspace-search-path-order, filecheck=CHECK):

__include lspSearchPathOrderInclude;

void useDependency()
{
//COMPLETE:8,13
    getWorkspace
}

// CHECK: getWorkspaceFirstRootIncludedValue
// CHECK-NOT: getWorkspaceSecondRootIncludedValue
