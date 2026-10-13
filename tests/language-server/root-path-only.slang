//TEST:LANG_SERVER(root-path-only, filecheck=CHECK):

__include rootUriOnlyInclude;

void useDependency()
{
//COMPLETE:8,8
    get
}

// CHECK: getRootUriOnlyIncludedValue
