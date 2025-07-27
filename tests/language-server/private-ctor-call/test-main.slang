//TEST:LANG_SERVER(filecheck=CHECK):

module "test-main";

import bsdfs;

LambertDiffuseBTDF createTranslucent(float3 color, float3 normal)
{
//HOVER:10,21
    return LambertDiffuseBTDF( color );
}

//CHECK: LambertDiffuseBTDF.init
