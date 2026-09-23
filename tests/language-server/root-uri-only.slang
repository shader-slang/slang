//TEST:LANG_SERVER(root-uri-only, filecheck=CHECK):

__include rootUriOnlyInclude;

void useDependency()
{
//COMPLETE:8,8
    get
}

// CHECK: getRootUriOnlyIncludedValue
