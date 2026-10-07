//TEST:LANG_SERVER(filecheck=CHECK):

// `slang.format.enableFormatOnType` defaults to on. Turning it off and then setting it to null
// restores the default.

void formatMe()
{
}

//CONFIG:slang.format.enableFormatOnType=false
//ON_TYPE_FORMAT:8,2
//CONFIG:slang.format.enableFormatOnType=null
//ON_TYPE_FORMAT:8,2

// CHECK: config: slang.format.enableFormatOnType=false
// CHECK-NEXT: --------
// CHECK-NEXT: on-type formatting: disabled
// CHECK: config: slang.format.enableFormatOnType=null
// CHECK-NEXT: --------
// CHECK-NEXT: on-type formatting: enabled
