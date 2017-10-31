//

// Define all the various language "profiles" we want to support.

#ifndef LANGUAGE
#define LANGUAGE(TAG, NAME) /* emptry */
#endif

#ifndef LANGUAGE_ALIAS
#define LANGUAGE_ALIAS(TAG, NAME) /* empty */
#endif

#ifndef PROFILE_FAMILY
#define PROFILE_FAMILY(TAG) /* empty */
#endif

#ifndef PROFILE_VERSION
#define PROFILE_VERSION(TAG, FAMILY) /* empty */
#endif

#ifndef PROFILE_STAGE
#define PROFILE_STAGE(TAG, NAME, VAL) /* empty */
#endif

#ifndef PROFILE_STAGE_ALIAS
#define PROFILE_STAGE_ALIAS(TAG, NAME) /* empty */
#endif


#ifndef PROFILE
#define PROFILE(TAG, NAME, STAGE, VERSION) /* empty */
#endif

#ifndef PROFILE_ALIAS
#define PROFILE_ALIAS(TAG, DEF, NAME) /* empty */
#endif

// Source and destination languages

LANGUAGE(HLSL,				hlsl)
LANGUAGE(DXBytecode,		dxbc)
LANGUAGE(DXBytecodeAssembly,dxbc_asm)
LANGUAGE(DXIL,				dxil)
LANGUAGE(DXILAssembly,		dxil_asm)
LANGUAGE(GLSL,				glsl)
LANGUAGE(GLSL_ES,			glsl_es)
LANGUAGE(GLSL_VK,			glsl_vk)
LANGUAGE(SPIRV,				spirv)
LANGUAGE(SPIRV_GL,			spirv_gl)

LANGUAGE_ALIAS(GLSL,		glsl_gl)
LANGUAGE_ALIAS(SPIRV,		spirv_vk)


// Pipeline stages to target
PROFILE_STAGE(Vertex,	vertex,     SLANG_STAGE_VERTEX)
PROFILE_STAGE(Hull,		hull,       SLANG_STAGE_HULL)
PROFILE_STAGE(Domain,	domain,     SLANG_STAGE_DOMAIN)
PROFILE_STAGE(Geometry, geometry,   SLANG_STAGE_GEOMETRY)
PROFILE_STAGE(Fragment, fragment,   SLANG_STAGE_FRAGMENT)
PROFILE_STAGE(Compute,	compute,    SLANG_STAGE_COMPUTE)

PROFILE_STAGE_ALIAS(Fragment, pixel)

// Profile families

PROFILE_FAMILY(DX)
PROFILE_FAMILY(GLSL)
PROFILE_FAMILY(SPRIV)

// Profile versions


PROFILE_VERSION(DX_4_0,				DX)
PROFILE_VERSION(DX_4_0_Level_9_0,	DX)
PROFILE_VERSION(DX_4_0_Level_9_1,	DX)
PROFILE_VERSION(DX_4_0_Level_9_3,	DX)
PROFILE_VERSION(DX_4_1,				DX)
PROFILE_VERSION(DX_5_0,				DX)
PROFILE_VERSION(DX_6_0,				DX)
PROFILE_VERSION(DX_6_1,				DX)
PROFILE_VERSION(DX_6_2,				DX)

PROFILE_VERSION(GLSL_110,           GLSL)
PROFILE_VERSION(GLSL_120,           GLSL)
PROFILE_VERSION(GLSL_130,           GLSL)
PROFILE_VERSION(GLSL_140,           GLSL)
PROFILE_VERSION(GLSL_150,           GLSL)
PROFILE_VERSION(GLSL_330,           GLSL)
PROFILE_VERSION(GLSL_400,           GLSL)
PROFILE_VERSION(GLSL_410,           GLSL)
PROFILE_VERSION(GLSL_420,           GLSL)
PROFILE_VERSION(GLSL_430,           GLSL)
PROFILE_VERSION(GLSL_440,           GLSL)
PROFILE_VERSION(GLSL_450,           GLSL)


// Specific profiles

PROFILE(DX_Compute_4_0,				cs_4_0,				Compute,	DX_4_0)
PROFILE(DX_Compute_4_1,				cs_4_1,				Compute,	DX_4_1)
PROFILE(DX_Compute_5_0,				cs_5_0,				Compute,	DX_5_0)
PROFILE(DX_Compute_6_0,				cs_6_0,				Compute,	DX_6_0)
PROFILE(DX_Compute_6_1,				cs_6_1,				Compute,	DX_6_1)
PROFILE(DX_Compute_6_2,				cs_6_2,				Compute,	DX_6_2)

PROFILE(DX_Domain_5_0,				ds_5_0,				Domain,		DX_5_0)
PROFILE(DX_Domain_6_0,				ds_6_0,				Domain,		DX_6_0)
PROFILE(DX_Domain_6_1,				ds_6_1,				Domain,		DX_6_1)
PROFILE(DX_Domain_6_2,				ds_6_2,				Domain,		DX_6_2)

PROFILE(DX_Geometry_4_0,			gs_4_0,				Geometry,	DX_4_0)
PROFILE(DX_Geometry_4_1,			gs_4_1,				Geometry,	DX_4_1)
PROFILE(DX_Geometry_5_0,			gs_5_0,				Geometry,	DX_5_0)
PROFILE(DX_Geometry_6_0,			gs_6_0,				Geometry,	DX_6_0)
PROFILE(DX_Geometry_6_1,			gs_6_1,				Geometry,	DX_6_1)
PROFILE(DX_Geometry_6_2,			gs_6_2,				Geometry,	DX_6_2)


PROFILE(DX_Hull_5_0,				hs_5_0,				Hull,		DX_5_0)
PROFILE(DX_Hull_6_0,			    hs_6_0,				Hull,   	DX_6_0)
PROFILE(DX_Hull_6_1,			    hs_6_1,				Hull,   	DX_6_1)
PROFILE(DX_Hull_6_2,			    hs_6_2,				Hull,   	DX_6_2)


PROFILE(DX_Fragment_4_0,			ps_4_0,				Fragment,	DX_4_0)
PROFILE(DX_Fragment_4_0_Level_9_0,	ps_4_0_level_9_0,	Fragment,	DX_4_0_Level_9_0)
PROFILE(DX_Fragment_4_0_Level_9_1,	ps_4_0_level_9_1,	Fragment,	DX_4_0_Level_9_1)
PROFILE(DX_Fragment_4_0_Level_9_3,	ps_4_0_level_9_3,	Fragment,	DX_4_0_Level_9_3)
PROFILE(DX_Fragment_4_1,			ps_4_1,				Fragment,	DX_4_1)
PROFILE(DX_Fragment_5_0,			ps_5_0,				Fragment,	DX_5_0)
PROFILE(DX_Fragment_6_0,			ps_6_0,				Fragment,	DX_6_0)
PROFILE(DX_Fragment_6_1,			ps_6_1,				Fragment,	DX_6_1)
PROFILE(DX_Fragment_6_2,			ps_6_2,				Fragment,	DX_6_2)


PROFILE(DX_Vertex_4_0,				vs_4_0,				Vertex,		DX_4_0)
PROFILE(DX_Vertex_4_0_Level_9_0,	vs_4_0_level_9_0,	Vertex,		DX_4_0_Level_9_0)
PROFILE(DX_Vertex_4_0_Level_9_1,	vs_4_0_level_9_1,	Vertex,		DX_4_0_Level_9_1)
PROFILE(DX_Vertex_4_0_Level_9_3,	vs_4_0_level_9_3,	Vertex,		DX_4_0_Level_9_3)
PROFILE(DX_Vertex_4_1,				vs_4_1,				Vertex,		DX_4_1)
PROFILE(DX_Vertex_5_0,				vs_5_0,				Vertex,		DX_5_0)
PROFILE(DX_Vertex_6_0,			    vs_6_0,				Vertex,	DX_6_0)
PROFILE(DX_Vertex_6_1,			    vs_6_1,				Vertex,	DX_6_1)
PROFILE(DX_Vertex_6_2,			    vs_6_2,				Vertex,	DX_6_2)

// Define all the GLSL profiles

#define P(UPPER, LOWER, VERSION) \
PROFILE(GLSL_##UPPER##_##VERSION,   glsl_##LOWER##_##VERSION, UPPER, GLSL_##VERSION)

P(Vertex, vertex, 110)
P(Vertex, vertex, 120)
P(Vertex, vertex, 130)
P(Vertex, vertex, 140)
P(Vertex, vertex, 150)
P(Vertex, vertex, 330)
P(Vertex, vertex, 400)
P(Vertex, vertex, 410)
P(Vertex, vertex, 420)
P(Vertex, vertex, 430)
P(Vertex, vertex, 440)
P(Vertex, vertex, 450)

P(Fragment, fragment, 110)
P(Fragment, fragment, 120)
P(Fragment, fragment, 130)
P(Fragment, fragment, 140)
P(Fragment, fragment, 150)
P(Fragment, fragment, 330)
P(Fragment, fragment, 400)
P(Fragment, fragment, 410)
P(Fragment, fragment, 420)
P(Fragment, fragment, 430)
P(Fragment, fragment, 440)
P(Fragment, fragment, 450)

P(Geometry, geometry, 150)
P(Geometry, geometry, 330)
P(Geometry, geometry, 400)
P(Geometry, geometry, 410)
P(Geometry, geometry, 420)
P(Geometry, geometry, 430)
P(Geometry, geometry, 440)
P(Geometry, geometry, 450)

P(Compute, compute, 430)
P(Compute, compute, 440)
P(Compute, compute, 450)

#undef P
#define P(UPPER, LOWER, STAGE, VERSION) \
PROFILE(GLSL_##UPPER##_##VERSION,   glsl_##LOWER##_##VERSION, STAGE, GLSL_##VERSION)

P(TessControl, tess_control, Hull, 400)
P(TessControl, tess_control, Hull, 410)
P(TessControl, tess_control, Hull, 420)
P(TessControl, tess_control, Hull, 430)
P(TessControl, tess_control, Hull, 440)
P(TessControl, tess_control, Hull, 450)

P(TessEval, tess_eval, Domain, 400)
P(TessEval, tess_eval, Domain, 410)
P(TessEval, tess_eval, Domain, 420)
P(TessEval, tess_eval, Domain, 430)
P(TessEval, tess_eval, Domain, 440)
P(TessEval, tess_eval, Domain, 450)

#undef P

// Define a default profile for each GLSL stage that just
// uses the latest language version we know of

PROFILE_ALIAS(GLSL_Vertex,      GLSL_Vertex_450,        glsl_vertex)
PROFILE_ALIAS(GLSL_Fragment,    GLSL_Fragment_450,      glsl_fragment)
PROFILE_ALIAS(GLSL_Geometry,    GLSL_Geometry_450,      glsl_geometry)
PROFILE_ALIAS(GLSL_TessControl, GLSL_TessControl_450,   glsl_tess_control)
PROFILE_ALIAS(GLSL_TessEval,    GLSL_TessEval_450,      glsl_tess_eval)
PROFILE_ALIAS(GLSL_Compute,     GLSL_Compute_450,       glsl_compute)

// TODO: define a profile for each GLSL *version* that we can
// use as a catch-all when the stage can be inferred from
// something else

#undef LANGUAGE
#undef LANGUAGE_ALIAS
#undef PROFILE_FAMILY
#undef PROFILE_VERSION
#undef PROFILE_STAGE
#undef PROFILE_STAGE_ALIAS
#undef PROFILE
#undef PROFILE_ALIAS
