//TEST:LANG_SERVER(multi-root, filecheck=CHECK):

import rootUriOnlyDependency;

void useDependency()
{
//COMPLETE:8,8
    get
}

// CHECK: getRootUriOnlyValue
