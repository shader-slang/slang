//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry MainPs -stage fragment -profile glsl_450 -g3 -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry MainPs -stage fragment -profile glsl_450 -g3 -emit-spirv-directly

// make sure that the generated spirv has glsl source in it.
//CHECK: #version 450

/*
This is to allow spirv-direct test to pass because this source should be
included in the generated spirv module:

#version 450
*/

struct PS_OUTPUT 
{ 
    float4 vColor : SV_Target0 ; 
} ; 

PS_OUTPUT MainPs ( ) 
{ 
    PS_OUTPUT o ; 
    o . vColor = float4 ( 0 , 0 , 0 , 0 ) ; 
    return o ; 
} 