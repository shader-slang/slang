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
#define PROFILE_ALIAS(TAG, NAME) /* empty */
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

PROFILE_VERSION(GLSL,               GLSL)


// Specific profiles

PROFILE(DX_Compute_4_0,				cs_4_0,				Compute,	DX_4_0)
PROFILE(DX_Compute_4_1,				cs_4_1,				Compute,	DX_4_1)
PROFILE(DX_Compute_5_0,				cs_5_0,				Compute,	DX_5_0)
PROFILE(DX_Domain_5_0,				ds_5_0,				Domain,		DX_5_0)
PROFILE(DX_Geometry_4_0,			gs_4_0,				Geometry,	DX_4_0)
PROFILE(DX_Geometry_4_1,			gs_4_1,				Geometry,	DX_4_1)
PROFILE(DX_Geometry_5_0,			gs_5_0,				Geometry,	DX_5_0)
PROFILE(DX_Hull_5_0,				hs_5_0,				Hull,		DX_5_0)
PROFILE(DX_Fragment_4_0,			ps_4_0,				Fragment,	DX_4_0)
PROFILE(DX_Fragment_4_0_Level_9_0,	ps_4_0_level_9_0,	Fragment,	DX_4_0_Level_9_0)
PROFILE(DX_Fragment_4_0_Level_9_1,	ps_4_0_level_9_1,	Fragment,	DX_4_0_Level_9_1)
PROFILE(DX_Fragment_4_0_Level_9_3,	ps_4_0_level_9_3,	Fragment,	DX_4_0_Level_9_3)
PROFILE(DX_Fragment_4_1,			ps_4_1,				Fragment,	DX_4_1)
PROFILE(DX_Fragment_5_0,			ps_5_0,				Fragment,	DX_5_0)
PROFILE(DX_Vertex_4_0,				vs_4_0,				Vertex,		DX_4_0)
PROFILE(DX_Vertex_4_0_Level_9_0,	vs_4_0_level_9_0,	Vertex,		DX_4_0_Level_9_0)
PROFILE(DX_Vertex_4_0_Level_9_1,	vs_4_0_level_9_1,	Vertex,		DX_4_0_Level_9_1)
PROFILE(DX_Vertex_4_0_Level_9_3,	vs_4_0_level_9_3,	Vertex,		DX_4_0_Level_9_3)
PROFILE(DX_Vertex_4_1,				vs_4_1,				Vertex,		DX_4_1)
PROFILE(DX_Vertex_5_0,				vs_5_0,				Vertex,		DX_5_0)

//

PROFILE(GLSL_Compute,               glsl_compute,       Compute,    GLSL)
PROFILE(GLSL_Vertex,                glsl_vertex,        Vertex,     GLSL)
PROFILE(GLSL_Fragment,              glsl_fragment,      Fragment,   GLSL)
PROFILE(GLSL_Geometry,              glsl_geometry,      Geometry,   GLSL)
PROFILE(GLSL_TessControl,           glsl_tess_control,  Hull,       GLSL)
PROFILE(GLSL_TessEval,              glsl_tess_eval,     Domain,     GLSL)

#undef LANGUAGE
#undef LANGUAGE_ALIAS
#undef PROFILE_FAMILY
#undef PROFILE_VERSION
#undef PROFILE_STAGE
#undef PROFILE_STAGE_ALIAS
#undef PROFILE
#undef PROFILE_ALIAS
