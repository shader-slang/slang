//TEST:SIMPLE:-target spirv -entry MainPs -stage fragment -profile glsl_450 -g3 -line-directive-mode none

// make sure that the generated spirv has glsl source in it.
//CHECK: #version 450
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