module material;

import runtime;
import types;

// user defined structs
struct structtype1 {
    float m_0;
    float m_1;
    float m_2;
    float m_3;
};
struct float22 {
    float m_0;
    float m_1;
};
struct _ZN4base23texture_coordinate_infoE {
    float3 position;
    float3 tangent_u;
    float3 tangent_v;
    int source_flags;
};
struct deriv_type0 {
    _ZN4base23texture_coordinate_infoE val;
    _ZN4base23texture_coordinate_infoE dx;
    _ZN4base23texture_coordinate_infoE dy;
};
struct structtype0 {
    float3 m_0;
    float m_1;
};
struct deriv_type97 {
    float4x4 val;
    float4x4 dx;
    float4x4 dy;
};
// globals
static const float3 glob_cnst91[16] = { { 0.02985999919f, 0.003100000089f, 0.1360899955f }, { 0.2071499974f, 0.02304000035f, 0.995840013f }, { 0.367170006f, 0.06469000131f, 1.895499945f }, { 0.2854900062f, 0.1366100013f, 1.672359943f }, { 0.08233000338f, 0.2685599923f, 0.7665299773f }, { 0.01723000035f, 0.4862099886f, 0.2188899964f }, { 0.1439999938f, 0.7734100223f, 0.05886000022f }, { 0.4095700085f, 0.9585000277f, 0.01279999968f }, { 0.7420099974f, 0.9796699882f, 0.0006000000285f }, { 1.033249974f, 0.8459100127f, 0.0f }, { 1.083850026f, 0.622420013f, 0.0f }, { 0.7920299768f, 0.3674899936f, 0.0f }, { 0.3875100017f, 0.1613499969f, 0.0f }, { 0.134010002f, 0.05297999829f, 0.0f }, { 0.03531000018f, 0.01374999993f, 0.0f }, { 0.008170000277f, 0.003169999924f, 0.0f } };
// functions
structtype1 constr_structtype1(
    float m_0,
    float m_1,
    float m_2,
    float m_3)
{
    structtype1 res;
    res.m_0 = m_0;
    res.m_1 = m_1;
    res.m_2 = m_2;
    res.m_3 = m_3;
    return res;
}

Derived_float3 constr_Derived_float3(float3 val, float3 dx, float3 dy)
{
    Derived_float3 res;
    res.val = val;
    res.dx = dx;
    res.dy = dy;
    return res;
}

deriv_type97 constr_deriv_type97(float4x4 val, float4x4 dx, float4x4 dy)
{
    deriv_type97 res;
    res.val = val;
    res.dx = dx;
    res.dy = dy;
    return res;
}

_ZN4base23texture_coordinate_infoE constr__ZN4base23texture_coordinate_infoE(
    float3 position,
    float3 tangent_u,
    float3 tangent_v,
    int source_flags)
{
    _ZN4base23texture_coordinate_infoE res;
    res.position = position;
    res.tangent_u = tangent_u;
    res.tangent_v = tangent_v;
    res.source_flags = source_flags;
    return res;
}

deriv_type0 constr_deriv_type0(_ZN4base23texture_coordinate_infoE val, _ZN4base23texture_coordinate_infoE dx, _ZN4base23texture_coordinate_infoE dy)
{
    deriv_type0 res;
    res.val = val;
    res.dx = dx;
    res.dy = dy;
    return res;
}

Derived_float2 constr_Derived_float2(float2 val, float2 dx, float2 dy)
{
    Derived_float2 res;
    res.val = val;
    res.dx = dx;
    res.dy = dy;
    return res;
}

structtype0 constr_structtype0(float3 m_0, float m_1)
{
    structtype0 res;
    res.m_0 = m_0;
    res.m_1 = m_1;
    return res;
}

Derived_float constr_Derived_float(float val, float dx, float dy)
{
    Derived_float res;
    res.val = val;
    res.dx = dx;
    res.dy = dy;
    return res;
}

Derived_float3 _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(
    in Shading_state_material state,
    in int from,
    in int to,
    in Derived_float3 vector0)
{
    int tmp1;
    Derived_float3 phi_in;
    Derived_float3 phi_out;
    Derived_float3 tmp8;
    tmp1 = from == 0 ? 2 : from;
    phi_in = vector0;
    if (tmp1 != (to == 0 ? 2 : to)) {
        float4x4 tmp2 = float4x4(state.object_to_world[0], state.object_to_world[1], state.object_to_world[2], state.object_to_world[3]);
        float4x4 tmp3 = float4x4(state.world_to_object[0], state.world_to_object[1], state.world_to_object[2], state.world_to_object[3]);
        float4x4 tmp4 = tmp1 == 2 ? tmp3 : tmp2;
        float3 val = vector0.val;
        float4 tmp5 = tmp4[0];
        float4 tmp6 = tmp4[1];
        float4 tmp7 = tmp4[2];
        float3 dx = vector0.dx;
        float3 dy = vector0.dy;
        tmp8 = constr_Derived_float3(float3(tmp5.x * val.x + tmp5.y * val.y + tmp5.z * val.z, tmp6.x * val.x + tmp6.y * val.y + tmp6.z * val.z, tmp7.x * val.x + tmp7.y * val.y + tmp7.z * val.z), float3(tmp5.x * dx.x + tmp5.y * dx.y + tmp5.z * dx.z, tmp6.x * dx.x + tmp6.y * dx.y + tmp6.z * dx.z, tmp7.x * dx.x + tmp7.y * dx.y + tmp7.z * dx.z), float3(tmp5.x * dy.x + tmp5.y * dy.y + tmp5.z * dy.z, tmp6.x * dy.x + tmp6.y * dy.y + tmp6.z * dy.z, tmp7.x * dy.x + tmp7.y * dy.y + tmp7.z * dy.z));
        phi_in = tmp8;
    }
    phi_out = phi_in;
    return phi_out;
}

deriv_type0 _ZN11OmniSurface9OmniImage28compute_texture_coordinate_2EU7uniformN4base25texture_coordinate_systemEU7uniformiU7uniformfU7uniformfU7uniformfU7uniformfU7uniformbU7uniformbU7uniformbU7uniformN11OmniSurface9OmniImage15projection_modeEU7uniformu6float3U7uniformu6float3U7uniformu6float3(
    in Shading_state_material state,
    in int texture_coordinate_system,
    in int uv_set,
    in Derived_float s_offset,
    in Derived_float t_offset,
    in Derived_float s_scale,
    in Derived_float t_scale,
    in bool s_flip,
    in bool t_flip,
    in bool swap_st,
    in int extended_projection_mode,
    in Derived_float3 projection_translate,
    in Derived_float3 projection_rotate,
    in Derived_float3 projection_scale)
{
    bool tmp0;
    float3 phi_in;
    float3 phi_out;
    float3 tmp1;
    float3 phi_in2;
    float3 phi_out3;
    float3 tmp4;
    float3 phi_in5;
    float3 phi_out6;
    float3 phi_in7;
    float3 phi_out8;
    int phi_in9;
    int phi_out10;
    float3 phi_in11;
    float3 phi_out12;
    float3 tmp13;
    int tmp14;
    float3 tmp15;
    float3 phi_in16;
    float3 phi_out17;
    float3 tmp21;
    float3 tmp24;
    float3 tmp27;
    float3 tmp53;
    bool phi_in54;
    bool phi_out55;
    int phi_in56;
    int phi_out57;
    float tmp80;
    float tmp81;
    float tmp82;
    float4 tmp83;
    float tmp84;
    float tmp85;
    float tmp86;
    float4 tmp87;
    float tmp88;
    float tmp89;
    float tmp90;
    float4 tmp91;
    float tmp92;
    float tmp93;
    float tmp94;
    float4 tmp95;
    deriv_type97 tmp96;
    float3 tmp104;
    float3 tmp105;
    float3 tmp106;
    float3 tmp111;
    float3 tmp113;
    float3 phi_in114;
    float3 phi_out115;
    float3 phi_in116;
    float3 phi_out117;
    float3 phi_in118;
    float3 phi_out119;
    float3 phi_in120;
    float3 phi_out121;
    float3 phi_in122;
    float3 phi_out123;
    float3 tmp124;
    float3 tmp125;
    float3 tmp126;
    float3 tmp127;
    float3 tmp128;
    int phi_in129;
    int phi_out130;
    float phi_in131;
    float phi_out132;
    float phi_in133;
    float phi_out134;
    float phi_in135;
    float phi_out136;
    deriv_type97 phi_in137;
    deriv_type97 phi_out138;
    int phi_in139;
    int phi_out140;
    int phi_in141;
    int phi_out142;
    float phi_in143;
    float phi_out144;
    float phi_in145;
    float phi_out146;
    float phi_in147;
    float phi_out148;
    deriv_type97 phi_in149;
    deriv_type97 phi_out150;
    int phi_in151;
    int phi_out152;
    int phi_in153;
    int phi_out154;
    int phi_in155;
    int phi_out156;
    float phi_in157;
    float phi_out158;
    float phi_in159;
    float phi_out160;
    float phi_in161;
    float phi_out162;
    deriv_type97 phi_in163;
    deriv_type97 phi_out164;
    float4 tmp166;
    float tmp167;
    float tmp168;
    int phi_in169;
    int phi_out170;
    int phi_in171;
    int phi_out172;
    int phi_in173;
    int phi_out174;
    float phi_in175;
    float phi_out176;
    float phi_in177;
    float phi_out178;
    float phi_in179;
    float phi_out180;
    deriv_type97 phi_in181;
    deriv_type97 phi_out182;
    deriv_type97 tmp183;
    deriv_type97 tmp188;
    float tmp189;
    deriv_type97 tmp190;
    deriv_type97 tmp195;
    deriv_type97 tmp196;
    float tmp197;
    float tmp198;
    float tmp199;
    float tmp200;
    float tmp201;
    float tmp202;
    float tmp203;
    float tmp204;
    float tmp205;
    bool tmp206;
    float tmp207;
    float tmp208;
    float tmp209;
    float tmp210;
    float tmp211;
    float tmp212;
    float tmp213;
    float tmp214;
    float tmp215;
    float phi_in216;
    float phi_out217;
    float phi_in218;
    float phi_out219;
    float phi_in220;
    float phi_out221;
    float tmp224;
    float tmp227;
    float tmp228;
    float3 phi_in229;
    float3 phi_out230;
    float3 phi_in231;
    float3 phi_out232;
    float3 phi_in233;
    float3 phi_out234;
    float3 tmp236;
    float3 tmp237;
    float3 tmp238;
    float tmp243;
    float3 tmp244;
    float3 tmp245;
    float3 tmp246;
    float phi_in247;
    float phi_out248;
    float phi_in249;
    float phi_out250;
    float phi_in251;
    float phi_out252;
    float phi_in253;
    float phi_out254;
    float phi_in255;
    float phi_out256;
    float3 phi_in257;
    float3 phi_out258;
    float3 phi_in259;
    float3 phi_out260;
    float3 phi_in261;
    float3 phi_out262;
    float tmp263;
    float tmp264;
    float tmp265;
    float tmp266;
    float tmp267;
    deriv_type97 tmp364;
    float phi_in365;
    float phi_out366;
    float phi_in367;
    float phi_out368;
    float phi_in369;
    float phi_out370;
    deriv_type97 phi_in371;
    deriv_type97 phi_out372;
    bool tmp373;
    float tmp375;
    float tmp376;
    float tmp377;
    float tmp378;
    float tmp379;
    float tmp380;
    float tmp381;
    float tmp382;
    float tmp383;
    float tmp384;
    float tmp385;
    float tmp386;
    float tmp387;
    float4 tmp388;
    int phi_in389;
    int phi_out390;
    float4 phi_in391;
    float4 phi_out392;
    float4 phi_in393;
    float4 phi_out394;
    float4 phi_in395;
    float4 phi_out396;
    float4 phi_in397;
    float4 phi_out398;
    float4 phi_in399;
    float4 phi_out400;
    float4 phi_in401;
    float4 phi_out402;
    float4 phi_in403;
    float4 phi_out404;
    float4 phi_in405;
    float4 phi_out406;
    float4 phi_in407;
    float4 phi_out408;
    float4 phi_in409;
    float4 phi_out410;
    float4 phi_in411;
    float4 phi_out412;
    float4 phi_in413;
    float4 phi_out414;
    float tmp415;
    float4 tmp416;
    float4 tmp417;
    float4 tmp418;
    float4 tmp419;
    float4 phi_in420;
    float4 phi_out421;
    float4 phi_in422;
    float4 phi_out423;
    float4 phi_in424;
    float4 phi_out425;
    float4 phi_in426;
    float4 phi_out427;
    float4 phi_in428;
    float4 phi_out429;
    float4 phi_in430;
    float4 phi_out431;
    float4 phi_in432;
    float4 phi_out433;
    float4 phi_in434;
    float4 phi_out435;
    float4 phi_in436;
    float4 phi_out437;
    float4 phi_in438;
    float4 phi_out439;
    float4 phi_in440;
    float4 phi_out441;
    float4 phi_in442;
    float4 phi_out443;
    float tmp444;
    float tmp445;
    float tmp447;
    float tmp448;
    float phi_in449;
    float phi_out450;
    float phi_in451;
    float phi_out452;
    float phi_in453;
    float phi_out454;
    float phi_in455;
    float phi_out456;
    float phi_in457;
    float phi_out458;
    float phi_in459;
    float phi_out460;
    float phi_in461;
    float phi_out462;
    float tmp463;
    float tmp464;
    float tmp466;
    float tmp467;
    float tmp469;
    float tmp470;
    float4 tmp525;
    float4 tmp527;
    float4 tmp529;
    float4 tmp531;
    float4 tmp533;
    float4 tmp535;
    float4 tmp537;
    float4 tmp539;
    float4 tmp541;
    float4 tmp542;
    float4 tmp543;
    float4 tmp544;
    deriv_type97 tmp545;
    float4x4 tmp546;
    float4x4 tmp547;
    float4 tmp549;
    float3 tmp553;
    bool tmp554;
    float3 tmp556;
    float tmp557;
    float tmp558;
    float phi_in559;
    float phi_out560;
    float phi_in561;
    float phi_out562;
    float3 phi_in563;
    float3 phi_out564;
    float3 phi_in565;
    float3 phi_out566;
    float3 tmp574;
    float3 tmp575;
    float tmp576;
    float tmp577;
    float3 tmp579;
    float3 tmp580;
    int phi_in582;
    int phi_out583;
    float3 phi_in584;
    float3 phi_out585;
    float3 phi_in586;
    float3 phi_out587;
    float3 tmp596;
    float3 tmp598;
    float3 tmp600;
    float3 phi_in601;
    float3 phi_out602;
    float3 phi_in603;
    float3 phi_out604;
    int tmp605;
    float3 tmp607;
    float3 tmp608;
    float3 tmp609;
    float tmp610;
    float tmp611;
    float tmp612;
    float tmp613;
    float tmp614;
    float tmp615;
    float3 tmp631;
    float3 tmp632;
    float3 tmp634;
    float3 tmp636;
    float3 tmp637;
    float3 phi_in638;
    float3 phi_out639;
    float3 phi_in640;
    float3 phi_out641;
    float3 phi_in642;
    float3 phi_out643;
    float3 phi_in644;
    float3 phi_out645;
    float3 phi_in646;
    float3 phi_out647;
    float3 tmp663;
    float3 tmp664;
    float3 tmp666;
    float3 tmp668;
    float3 tmp669;
    float3 tmp690;
    float3 tmp693;
    float3 tmp694;
    if (texture_coordinate_system == 0) {
        tmp0 = uv_set == 0;
        phi_in = float3(0.0f, 0.0f, 0.0f);
        if (tmp0) {
            tmp1 = state.text_coords[0].val;
            phi_in = tmp1;
        }
        phi_out = phi_in;
        phi_in2 = float3(0.0f, 0.0f, 0.0f);
        if (tmp0) {
            tmp4 = state.tangent_u[0];
            phi_in2 = tmp4;
        }
        phi_out3 = phi_in2;
        phi_in5 = phi_out3;
        phi_in7 = float3(0.0f, 0.0f, 0.0f);
        phi_in9 = 0;
        phi_in11 = phi_out;
        if (tmp0) {
            tmp13 = state.tangent_v[0];
            phi_in5 = phi_out3;
            phi_in7 = tmp13;
            phi_in9 = 0;
            phi_in11 = phi_out;
        }
    } else if (extended_projection_mode == 10) {
        bool cond = texture_coordinate_system == 1;
        tmp14 = cond ? 2 : 1;
        tmp15 = state.position.val;
        phi_in16 = tmp15;
        if (!cond) {
            float4 tmp18 = state.world_to_object[0];
            float4 tmp19 = state.world_to_object[1];
            float4 tmp20 = state.world_to_object[2];
            tmp21 = float3(tmp18.y * tmp15.y + tmp18.w + tmp18.x * tmp15.x + tmp18.z * tmp15.z, tmp19.y * tmp15.y + tmp19.w + tmp19.x * tmp15.x + tmp19.z * tmp15.z, tmp20.y * tmp15.y + tmp20.w + tmp20.x * tmp15.x + tmp20.z * tmp15.z);
            phi_in16 = tmp21;
        }
        phi_out17 = phi_in16;
        float3 tmp22 = _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(state, 0, tmp14, constr_Derived_float3(state.tangent_u[0], float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f))).val;
        float3 tmp23 = float3(sqrt(tmp22.x * tmp22.x + tmp22.y * tmp22.y + tmp22.z * tmp22.z), 0.0f, 0.0f);
        tmp24 = tmp22 / tmp23.xxx;
        float3 tmp25 = _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(state, 0, tmp14, constr_Derived_float3(state.tangent_v[0], float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f))).val;
        float3 tmp26 = float3(sqrt(tmp25.x * tmp25.x + tmp25.y * tmp25.y + tmp25.z * tmp25.z), 0.0f, 0.0f);
        tmp27 = tmp25 / tmp26.xxx;
        float3 tmp28 = float3(sqrt(phi_out17.x * phi_out17.x + phi_out17.y * phi_out17.y + phi_out17.z * phi_out17.z), 0.0f, 0.0f);
        float3 tmp29 = phi_out17 / tmp28.xxx;
        float3 tmp30 = projection_rotate.val;
        float tmp31 = tmp30.x * 6.283185482f;
        float tmp32 = cos(tmp31);
        float tmp33 = sin(tmp31);
        float tmp34 = tmp29.y * tmp32 + tmp29.z * tmp33;
        float tmp35 = tmp29.z * tmp32 - tmp29.y * tmp33;
        float tmp36 = tmp30.y * 6.283185482f + -1.570796371f;
        float tmp37 = cos(tmp36);
        float tmp38 = sin(tmp36);
        float tmp39 = tmp34 * tmp38 + tmp29.x * tmp37;
        float tmp40 = tmp34 * tmp37 - tmp29.x * tmp38;
        float3 tmp41 = float3(0.0f, tmp40, 0.0f);
        float tmp42 = tmp30.z * 6.283185482f;
        float tmp43 = cos(tmp42);
        float tmp44 = sin(tmp42);
        float tmp45 = tmp39 * tmp43 + tmp35 * tmp44;
        float tmp46 = tmp35 * tmp43 - tmp39 * tmp44;
        float3 tmp47 = float3(tmp45, tmp46, 0.0f);
        float3 tmp48 = float3(tmp47.x, tmp41.y, tmp47.y);
        float3 tmp49 = float3(-tmp45, -tmp46, 0.0f);
        float3 tmp50 = float3(sqrt(tmp46 * tmp46 + tmp40 * tmp40 + tmp45 * tmp45), 0.0f, 0.0f);
        float3 tmp51 = float3(tmp49.x, tmp49.y, tmp48.y) / tmp50.xxx;
        float3 tmp52 = float3(atan2(tmp51.x, -tmp51.z) * 0.1591549367f + 0.5f, asin(-tmp51.y) * 0.3183098733f + 0.5f, 0.0f);
        tmp53 = float3(tmp52.x, tmp52.y, tmp48.z);
        phi_in5 = tmp24;
        phi_in7 = tmp27;
        phi_in9 = texture_coordinate_system;
        phi_in11 = tmp53;
    } else {
        if (extended_projection_mode == 1) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 2) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 3) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 4) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 9) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 6) {
            phi_in54 = true;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 7) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else if (extended_projection_mode == 8) {
            phi_in54 = false;
            phi_in56 = extended_projection_mode;
        } else {
            phi_in54 = false;
            phi_in56 = 5;
        }
        phi_out55 = phi_in54;
        phi_out57 = phi_in56;
        float3 tmp58 = projection_scale.val;
        float3 tmp59 = projection_translate.val;
        float tmp60 = tmp59.x + -0.5f;
        float tmp61 = tmp59.y + -0.5f;
        float tmp62 = tmp59.z + -0.5f;
        float3 tmp63 = projection_rotate.val;
        float tmp64 = sin(tmp63.x);
        float tmp65 = sin(tmp63.y);
        float tmp66 = sin(tmp63.z);
        float tmp67 = cos(tmp63.x);
        float tmp68 = cos(tmp63.y);
        float tmp69 = cos(tmp63.z);
        float tmp70 = tmp68 * tmp69;
        float tmp71 = tmp64 * tmp65;
        float tmp72 = tmp71 * tmp69 - tmp67 * tmp66;
        float tmp73 = tmp67 * tmp65;
        float tmp74 = tmp73 * tmp69 + tmp64 * tmp66;
        float tmp75 = tmp68 * tmp66;
        float tmp76 = tmp71 * tmp66 + tmp67 * tmp69;
        float tmp77 = tmp73 * tmp66 - tmp64 * tmp69;
        float tmp78 = tmp64 * tmp68;
        float tmp79 = tmp67 * tmp68;
        tmp80 = tmp70 * tmp58.x;
        tmp81 = tmp72 * tmp58.x;
        tmp82 = tmp74 * tmp58.x;
        tmp83 = float4(tmp80, tmp81, tmp82, 0.0f);
        tmp84 = tmp75 * tmp58.y;
        tmp85 = tmp76 * tmp58.y;
        tmp86 = tmp77 * tmp58.y;
        tmp87 = float4(tmp84, tmp85, tmp86, 0.0f);
        tmp88 = tmp58.z * -tmp65;
        tmp89 = tmp78 * tmp58.z;
        tmp90 = tmp79 * tmp58.z;
        tmp91 = float4(tmp88, tmp89, tmp90, 0.0f);
        tmp92 = 0.5f - tmp65 * tmp62 + tmp75 * tmp61 + tmp70 * tmp60;
        tmp93 = tmp78 * tmp62 + 0.5f + tmp76 * tmp61 + tmp72 * tmp60;
        tmp94 = tmp79 * tmp62 + 0.5f + tmp77 * tmp61 + tmp74 * tmp60;
        tmp95 = float4(tmp92, tmp93, tmp94, 1.0f);
        tmp96 = constr_deriv_type97(float4x4(tmp83, tmp87, tmp91, tmp95), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
        if (texture_coordinate_system == 2) {
            float3 tmp98 = state.position.val;
            float3 tmp99 = state.position.dx;
            float3 tmp100 = state.position.dy;
            float4 tmp101 = state.world_to_object[0];
            float4 tmp102 = state.world_to_object[1];
            float4 tmp103 = state.world_to_object[2];
            tmp104 = float3(tmp101.y * tmp98.y + tmp101.w + tmp101.x * tmp98.x + tmp101.z * tmp98.z, tmp102.y * tmp98.y + tmp102.w + tmp102.x * tmp98.x + tmp102.z * tmp98.z, tmp103.y * tmp98.y + tmp103.w + tmp103.x * tmp98.x + tmp103.z * tmp98.z);
            tmp105 = float3(tmp101.x * tmp99.x + tmp101.y * tmp99.y + tmp101.z * tmp99.z, tmp102.x * tmp99.x + tmp102.y * tmp99.y + tmp102.z * tmp99.z, tmp103.x * tmp99.x + tmp103.y * tmp99.y + tmp103.z * tmp99.z);
            tmp106 = float3(tmp101.x * tmp100.x + tmp101.y * tmp100.y + tmp101.z * tmp100.z, tmp102.x * tmp100.x + tmp102.y * tmp100.y + tmp102.z * tmp100.z, tmp103.x * tmp100.x + tmp103.y * tmp100.y + tmp103.z * tmp100.z);
            float3 tmp107 = state.normal;
            float4 tmp108 = state.object_to_world[0];
            float4 tmp109 = state.object_to_world[1];
            float4 tmp110 = state.object_to_world[2];
            tmp111 = float3(tmp109.x * tmp107.y + tmp108.x * tmp107.x + tmp110.x * tmp107.z, tmp109.y * tmp107.y + tmp108.y * tmp107.x + tmp110.y * tmp107.z, tmp109.z * tmp107.y + tmp108.z * tmp107.x + tmp110.z * tmp107.z);
            float3 tmp112 = state.geom_normal;
            tmp113 = float3(tmp112.y * tmp109.x + tmp112.x * tmp108.x + tmp112.z * tmp110.x, tmp112.y * tmp109.y + tmp112.x * tmp108.y + tmp112.z * tmp110.y, tmp112.y * tmp109.z + tmp112.x * tmp108.z + tmp112.z * tmp110.z);
            phi_in114 = tmp113;
            phi_in116 = tmp111;
            phi_in118 = tmp106;
            phi_in120 = tmp105;
            phi_in122 = tmp104;
        } else if (texture_coordinate_system == 1) {
            tmp124 = state.position.val;
            tmp125 = state.position.dx;
            tmp126 = state.position.dy;
            tmp127 = state.normal;
            tmp128 = state.geom_normal;
            phi_in114 = tmp128;
            phi_in116 = tmp127;
            phi_in118 = tmp126;
            phi_in120 = tmp125;
            phi_in122 = tmp124;
        } else {
            phi_in114 = float3(0.0f, 0.0f, 0.0f);
            phi_in116 = float3(0.0f, 0.0f, 0.0f);
            phi_in118 = float3(0.0f, 0.0f, 0.0f);
            phi_in120 = float3(0.0f, 0.0f, 0.0f);
            phi_in122 = float3(0.0f, 0.0f, 0.0f);
        }
        phi_out115 = phi_in114;
        phi_out117 = phi_in116;
        phi_out119 = phi_in118;
        phi_out121 = phi_in120;
        phi_out123 = phi_in122;
        phi_in129 = 1;
        phi_in131 = 0.0f;
        phi_in133 = 0.0f;
        phi_in135 = 0.0f;
        phi_in137 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
        if (phi_out57 != 3) {
            phi_in129 = 1;
            phi_in131 = 0.0f;
            phi_in133 = 0.0f;
            phi_in135 = 0.0f;
            phi_in137 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
            if (phi_out57 != 4) {
                phi_in129 = 1;
                phi_in131 = 0.0f;
                phi_in133 = 0.0f;
                phi_in135 = 0.0f;
                phi_in137 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                if (phi_out57 != 7) {
                    phi_in129 = 1;
                    phi_in131 = 0.0f;
                    phi_in133 = 0.0f;
                    phi_in135 = 0.0f;
                    phi_in137 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                    if (phi_out57 != 8) {
                        phi_in139 = 0;
                        phi_in141 = 0;
                        phi_in143 = 0.0f;
                        phi_in145 = 0.0f;
                        phi_in147 = 0.0f;
                        phi_in149 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                        if (phi_out57 != 2) {
                            phi_in139 = 0;
                            phi_in141 = 0;
                            phi_in143 = 0.0f;
                            phi_in145 = 0.0f;
                            phi_in147 = 0.0f;
                            phi_in149 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                            if (phi_out57 != 6) {
                                phi_in151 = 0;
                                phi_in153 = 0;
                                phi_in155 = 0;
                                phi_in157 = 0.0f;
                                phi_in159 = 0.0f;
                                phi_in161 = 0.0f;
                                phi_in163 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                                if (phi_out57 != 9) {
                                    phi_in151 = 0;
                                    phi_in153 = 0;
                                    phi_in155 = 0;
                                    phi_in157 = 0.0f;
                                    phi_in159 = 0.0f;
                                    phi_in161 = 0.0f;
                                    phi_in163 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                                    if (phi_out57 != 1) {
                                        phi_in151 = 1;
                                        phi_in153 = 1;
                                        phi_in155 = 0;
                                        phi_in157 = phi_out123.z;
                                        phi_in159 = phi_out123.y;
                                        phi_in161 = phi_out123.x;
                                        phi_in163 = tmp96;
                                    }
                                }
                                phi_out152 = phi_in151;
                                phi_out154 = phi_in153;
                                phi_out156 = phi_in155;
                                phi_out158 = phi_in157;
                                phi_out160 = phi_in159;
                                phi_out162 = phi_in161;
                                phi_out164 = phi_in163;
                                phi_in139 = phi_out154;
                                phi_in141 = phi_out156;
                                phi_in143 = phi_out158;
                                phi_in145 = phi_out160;
                                phi_in147 = phi_out162;
                                phi_in149 = phi_out164;
                                if (phi_out152 == 0) {
                                    float4 tmp165 = float4(phi_out117.x * tmp80 + phi_out117.y * tmp84 + phi_out117.z * tmp88, phi_out117.x * tmp81 + phi_out117.y * tmp85 + phi_out117.z * tmp89, phi_out117.x * tmp82 + phi_out117.y * tmp86 + phi_out117.z * tmp90, 0.0f);
                                    tmp166 = (phi_out123.y * tmp84 + tmp92 + phi_out123.x * tmp80 + phi_out123.z * tmp88) * (phi_out115.x * tmp80 + phi_out115.y * tmp84 + phi_out115.z * tmp88) + (phi_out123.y * tmp85 + tmp93 + phi_out123.x * tmp81 + phi_out123.z * tmp89) * (phi_out115.x * tmp81 + phi_out115.y * tmp85 + phi_out115.z * tmp89) + (phi_out123.y * tmp86 + tmp94 + phi_out123.x * tmp82 + phi_out123.z * tmp90) * (phi_out115.x * tmp82 + phi_out115.y * tmp86 + phi_out115.z * tmp90) < 0.0f ? -tmp165 : tmp165;
                                    tmp167 = abs(tmp166.x);
                                    tmp168 = abs(tmp166.y);
                                    phi_in169 = 0;
                                    phi_in171 = 0;
                                    phi_in173 = 0;
                                    phi_in175 = 0.0f;
                                    phi_in177 = 0.0f;
                                    phi_in179 = 0.0f;
                                    phi_in181 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                                    if (!(tmp167 < tmp168)) {
                                        phi_in169 = 0;
                                        phi_in171 = 0;
                                        phi_in173 = 0;
                                        phi_in175 = 0.0f;
                                        phi_in177 = 0.0f;
                                        phi_in179 = 0.0f;
                                        phi_in181 = constr_deriv_type97(float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                                        if (!(tmp167 < abs(tmp166.z))) {
                                            if (tmp166.x > 0.0f) {
                                                tmp183 = constr_deriv_type97(float4x4(tmp83.yzxw, tmp87.yzxw, tmp91.yzxw, tmp95.yzxw), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                                                phi_in169 = 1;
                                                phi_in171 = 1;
                                                phi_in173 = 0;
                                                phi_in175 = phi_out123.z;
                                                phi_in177 = phi_out123.y;
                                                phi_in179 = phi_out123.x;
                                                phi_in181 = tmp183;
                                            } else {
                                                float4 tmp184 = float4(-tmp81, 0.0f, 0.0f, 0.0f);
                                                float4 tmp185 = float4(-tmp85, 0.0f, 0.0f, 0.0f);
                                                float4 tmp186 = float4(-tmp89, 0.0f, 0.0f, 0.0f);
                                                float4 tmp187 = float4(-tmp93, 0.0f, 0.0f, 0.0f);
                                                tmp188 = constr_deriv_type97(float4x4(float4(tmp184.x, tmp83.z, tmp83.x, tmp83.w), float4(tmp185.x, tmp87.z, tmp87.x, tmp87.w), float4(tmp186.x, tmp91.z, tmp91.x, tmp91.w), float4(tmp187.x, tmp95.z, tmp95.x, tmp95.w)), float4x4(float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f)));
                                                phi_in169 = 1;
                                                phi_in171 = 1;
                                                phi_in173 = 0;
                                                phi_in175 = phi_out123.z;
                                                phi_in177 = phi_out123.y;
                                                phi_in179 = phi_out123.x;
                                                phi_in181 = tmp188;
                                            }
                                        }
                                    }
                                    phi_out170 = phi_in169;
                                    phi_out172 = phi_in171;
                                    phi_out174 = phi_in173;
                                    phi_out176 = phi_in175;
                                    phi_out178 = phi_in177;
                                    phi_out180 = phi_in179;
                                    phi_out182 = phi_in181;
                                    phi_in139 = phi_out172;
                                    phi_in141 = phi_out174;
                                    phi_in143 = phi_out176;
                                    phi_in145 = phi_out178;
                                    phi_in147 = phi_out180;
                                    phi_in149 = phi_out182;
                                    if (phi_out170 == 0) {
                                        tmp189 = abs(tmp166.z);
                                        if (tmp168 < tmp167 || tmp168 < tmp189) {
                                            phi_in139 = 1;
                                            phi_in141 = 0;
                                            phi_in143 = phi_out123.z;
                                            phi_in145 = phi_out123.y;
                                            phi_in147 = phi_out123.x;
                                            phi_in149 = tmp96;
                                            if (!(tmp189 < tmp167)) {
                                                phi_in139 = 1;
                                                phi_in141 = 0;
                                                phi_in143 = phi_out123.z;
                                                phi_in145 = phi_out123.y;
                                                phi_in147 = phi_out123.x;
                                                phi_in149 = tmp96;
                                                if (tmp189 >= tmp168 && tmp166.z < 0.0f) {
                                                    tmp190 = constr_deriv_type97(float4x4(float4(-tmp80, tmp83.y, tmp83.z, tmp83.w), float4(-tmp84, tmp87.y, tmp87.z, tmp87.w), float4(-tmp88, tmp91.y, tmp91.z, tmp91.w), float4(-tmp92, tmp95.y, tmp95.z, tmp95.w)), float4x4(float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f)));
                                                    phi_in139 = 1;
                                                    phi_in141 = 0;
                                                    phi_in143 = phi_out123.z;
                                                    phi_in145 = phi_out123.y;
                                                    phi_in147 = phi_out123.x;
                                                    phi_in149 = tmp190;
                                                }
                                            }
                                        } else if (tmp166.y > 0.0f) {
                                            float4 tmp191 = float4(-tmp80, 0.0f, 0.0f, 0.0f);
                                            float4 tmp192 = float4(-tmp84, 0.0f, 0.0f, 0.0f);
                                            float4 tmp193 = float4(-tmp88, 0.0f, 0.0f, 0.0f);
                                            float4 tmp194 = float4(-tmp92, 0.0f, 0.0f, 0.0f);
                                            tmp195 = constr_deriv_type97(float4x4(float4(tmp191.x, tmp83.z, tmp83.y, tmp83.w), float4(tmp192.x, tmp87.z, tmp87.y, tmp87.w), float4(tmp193.x, tmp91.z, tmp91.y, tmp91.w), float4(tmp194.x, tmp95.z, tmp95.y, tmp95.w)), float4x4(float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f), float4(-0.0f, 0.0f, 0.0f, 0.0f)));
                                            phi_in139 = 1;
                                            phi_in141 = 0;
                                            phi_in143 = phi_out123.z;
                                            phi_in145 = phi_out123.y;
                                            phi_in147 = phi_out123.x;
                                            phi_in149 = tmp195;
                                        } else {
                                            tmp196 = constr_deriv_type97(float4x4(tmp83.xzyw, tmp87.xzyw, tmp91.xzyw, tmp95.xzyw), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)), float4x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)));
                                            phi_in139 = 1;
                                            phi_in141 = 0;
                                            phi_in143 = phi_out123.z;
                                            phi_in145 = phi_out123.y;
                                            phi_in147 = phi_out123.x;
                                            phi_in149 = tmp196;
                                        }
                                    }
                                }
                            }
                        }
                        phi_out140 = phi_in139;
                        phi_out142 = phi_in141;
                        phi_out144 = phi_in143;
                        phi_out146 = phi_in145;
                        phi_out148 = phi_in147;
                        phi_out150 = phi_in149;
                        phi_in129 = phi_out142;
                        phi_in131 = phi_out144;
                        phi_in133 = phi_out146;
                        phi_in135 = phi_out148;
                        phi_in137 = phi_out150;
                        if (phi_out140 == 0) {
                            tmp197 = phi_out123.y * tmp84 + tmp92 + phi_out123.x * tmp80 + phi_out123.z * tmp88;
                            tmp198 = phi_out121.z * tmp88 + phi_out121.x * tmp80 + phi_out121.y * tmp84;
                            tmp199 = phi_out119.z * tmp88 + phi_out119.x * tmp80 + phi_out119.y * tmp84;
                            tmp200 = phi_out123.y * tmp85 + tmp93 + phi_out123.x * tmp81 + phi_out123.z * tmp89;
                            tmp201 = phi_out121.z * tmp89 + phi_out121.x * tmp81 + phi_out121.y * tmp85;
                            tmp202 = phi_out119.z * tmp89 + phi_out119.x * tmp81 + phi_out119.y * tmp85;
                            tmp203 = phi_out123.y * tmp86 + tmp94 + phi_out123.x * tmp82 + phi_out123.z * tmp90;
                            tmp204 = phi_out121.z * tmp90 + phi_out121.x * tmp82 + phi_out121.y * tmp86;
                            tmp205 = phi_out119.z * tmp90 + phi_out119.x * tmp82 + phi_out119.y * tmp86;
                            tmp206 = tmp197 != 0.0f || tmp200 != 0.0f;
                            tmp207 = tmp197 * tmp197;
                            tmp208 = tmp200 * tmp200;
                            if (tmp203 != 0.0f || tmp206) {
                                tmp209 = tmp200 * tmp202;
                                tmp210 = tmp197 * tmp199;
                                tmp211 = (tmp210 + tmp209) * 2.0f;
                                tmp212 = tmp200 * tmp201;
                                tmp213 = tmp197 * tmp198;
                                tmp214 = (tmp213 + tmp212) * 2.0f;
                                tmp215 = tmp207 + tmp208;
                                phi_in216 = 0.3183098733f;
                                phi_in218 = 0.0f;
                                phi_in220 = 0.0f;
                                if (!phi_out55) {
                                    float tmp222 = tmp203 * 2.0f;
                                    float tmp223 = tmp215 + tmp203 * tmp203;
                                    tmp224 = sqrt(tmp223);
                                    float tmp225 = 0.5f / tmp224;
                                    bool tmp226 = tmp223 <= 0.0f;
                                    tmp227 = tmp226 ? 0.0f : tmp225 * (tmp214 + tmp222 * tmp204);
                                    tmp228 = tmp226 ? 0.0f : tmp225 * (tmp211 + tmp222 * tmp205);
                                    phi_in216 = tmp224;
                                    phi_in218 = tmp227;
                                    phi_in220 = tmp228;
                                }
                                phi_out217 = phi_in216;
                                phi_out219 = phi_in218;
                                phi_out221 = phi_in220;
                                phi_in229 = float3(0.0f, 0.0f, 0.0f);
                                phi_in231 = float3(0.0f, 0.0f, 0.0f);
                                phi_in233 = float3(0.0f, 0.0f, 0.0f);
                                if (tmp206) {
                                    float tmp235 = atan2(tmp200, tmp197);
                                    tmp236 = float3(tmp235 * phi_out217, 0.0f, 0.0f);
                                    tmp237 = float3(tmp235 * phi_out219 + phi_out217 * (tmp197 * tmp201 - tmp200 * tmp198) / tmp215, 0.0f, 0.0f);
                                    tmp238 = float3(tmp235 * phi_out221 + phi_out217 * (tmp197 * tmp202 - tmp200 * tmp199) / tmp215, 0.0f, 0.0f);
                                    phi_in229 = tmp238;
                                    phi_in231 = tmp237;
                                    phi_in233 = tmp236;
                                }
                                phi_out230 = phi_in229;
                                phi_out232 = phi_in231;
                                phi_out234 = phi_in233;
                                float tmp239 = sqrt(tmp215);
                                float tmp240 = 0.5f / tmp239;
                                bool tmp241 = tmp215 <= 0.0f;
                                float tmp242 = atan2(tmp203, tmp239);
                                tmp243 = tmp215 + tmp203 * tmp203;
                                tmp244 = float3(phi_out234.x, tmp242 * phi_out217, phi_out217);
                                tmp245 = float3(phi_out232.x, tmp242 * phi_out219 + phi_out217 * (tmp239 * tmp204 - (tmp241 ? 0.0f : tmp240 * tmp214) * tmp203) / tmp243, phi_out219);
                                tmp246 = float3(phi_out230.x, tmp242 * phi_out221 + phi_out217 * (tmp239 * tmp205 - (tmp241 ? 0.0f : tmp240 * tmp211) * tmp203) / tmp243, phi_out221);
                                phi_in247 = tmp210;
                                phi_in249 = tmp209;
                                phi_in251 = tmp213;
                                phi_in253 = tmp212;
                                phi_in255 = tmp243;
                                phi_in257 = tmp246;
                                phi_in259 = tmp245;
                                phi_in261 = tmp244;
                            } else {
                                tmp263 = tmp207 + tmp208 + tmp203 * tmp203;
                                tmp264 = tmp200 * tmp201;
                                tmp265 = tmp197 * tmp198;
                                tmp266 = tmp200 * tmp202;
                                tmp267 = tmp197 * tmp199;
                                phi_in247 = tmp267;
                                phi_in249 = tmp266;
                                phi_in251 = tmp265;
                                phi_in253 = tmp264;
                                phi_in255 = tmp263;
                                phi_in257 = float3(0.0f, 0.0f, 0.0f);
                                phi_in259 = float3(0.0f, 0.0f, 0.0f);
                                phi_in261 = float3(0.0f, 0.0f, 0.0f);
                            }
                            phi_out248 = phi_in247;
                            phi_out250 = phi_in249;
                            phi_out252 = phi_in251;
                            phi_out254 = phi_in253;
                            phi_out256 = phi_in255;
                            phi_out258 = phi_in257;
                            phi_out260 = phi_in259;
                            phi_out262 = phi_in261;
                            float tmp268 = -phi_out123.x;
                            float3 tmp269 = float3(tmp197, tmp200, tmp203);
                            float tmp270 = sqrt(phi_out256);
                            float tmp271 = 1.0f / tmp270;
                            bool tmp272 = phi_out256 <= 0.0f;
                            float3 tmp273 = float3(tmp270, 0.0f, 0.0f);
                            float3 tmp274 = tmp273.xxx;
                            float3 tmp275 = tmp273 * tmp273;
                            float3 tmp276 = tmp275.xxx;
                            float3 tmp277 = tmp269 / tmp274;
                            float3 tmp278 = float3(tmp272 ? 0.0f : tmp271 * (phi_out252 + tmp203 * tmp204 + phi_out254), 0.0f, 0.0f);
                            float3 tmp279 = (tmp274 * float3(tmp198, tmp201, tmp204) - tmp278.xxx * tmp269) / tmp276;
                            float3 tmp280 = float3(tmp272 ? 0.0f : tmp271 * (phi_out248 + tmp203 * tmp205 + phi_out250), 0.0f, 0.0f);
                            float3 tmp281 = (tmp274 * float3(tmp199, tmp202, tmp205) - tmp280.xxx * tmp269) / tmp276;
                            float3 tmp282 = tmp277.zxy;
                            float3 tmp283 = tmp277.yzx;
                            float3 tmp284 = tmp282 * float3(0.0f, 1.0f, 0.0f) - tmp283 * float3(1.0f, 0.0f, 0.0f);
                            float3 tmp285 = tmp279.zxy;
                            float3 tmp286 = tmp279.yzx;
                            float3 tmp287 = tmp285 * float3(0.0f, 1.0f, 0.0f) - tmp286 * float3(1.0f, 0.0f, 0.0f);
                            float3 tmp288 = tmp281.zxy;
                            float3 tmp289 = tmp281.yzx;
                            float3 tmp290 = tmp288 * float3(0.0f, 1.0f, 0.0f) - tmp289 * float3(1.0f, 0.0f, 0.0f);
                            float tmp291 = tmp284.x * tmp284.x + tmp284.y * tmp284.y + tmp284.z * tmp284.z;
                            float tmp292 = sqrt(tmp291);
                            float tmp293 = 1.0f / tmp292;
                            bool tmp294 = tmp291 <= 0.0f;
                            float3 tmp295 = float3(tmp292, 0.0f, 0.0f);
                            float3 tmp296 = tmp295.xxx;
                            float3 tmp297 = tmp295 * tmp295;
                            float3 tmp298 = tmp297.xxx;
                            float3 tmp299 = tmp284 / tmp296;
                            float3 tmp300 = float3(tmp294 ? 0.0f : (tmp287.z * tmp284.z + tmp287.x * tmp284.x + tmp287.y * tmp284.y) * tmp293, 0.0f, 0.0f);
                            float3 tmp301 = (tmp296 * tmp287 - tmp300.xxx * tmp284) / tmp298;
                            float3 tmp302 = float3(tmp294 ? 0.0f : (tmp290.z * tmp284.z + tmp290.x * tmp284.x + tmp290.y * tmp284.y) * tmp293, 0.0f, 0.0f);
                            float3 tmp303 = (tmp296 * tmp290 - tmp302.xxx * tmp284) / tmp298;
                            float3 tmp304 = tmp299.zxy;
                            float3 tmp305 = tmp299.yzx;
                            float3 tmp306 = tmp304 * tmp283 - tmp305 * tmp282;
                            float3 tmp307 = tmp301.zxy * tmp283 + tmp304 * tmp286 - (tmp301.yzx * tmp282 + tmp305 * tmp285);
                            float3 tmp308 = tmp303.zxy * tmp283 + tmp304 * tmp289 - (tmp303.yzx * tmp282 + tmp305 * tmp288);
                            float tmp309 = tmp306.x * tmp306.x + tmp306.y * tmp306.y + tmp306.z * tmp306.z;
                            float tmp310 = sqrt(tmp309);
                            float tmp311 = 1.0f / tmp310;
                            bool tmp312 = tmp309 <= 0.0f;
                            float3 tmp313 = float3(tmp310, 0.0f, 0.0f);
                            float3 tmp314 = tmp313.xxx;
                            float3 tmp315 = tmp313 * tmp313;
                            float3 tmp316 = tmp315.xxx;
                            float3 tmp317 = tmp306 / tmp314;
                            float3 tmp318 = float3(tmp312 ? 0.0f : (tmp307.z * tmp306.z + tmp307.x * tmp306.x + tmp307.y * tmp306.y) * tmp311, 0.0f, 0.0f);
                            float3 tmp319 = (tmp307 * tmp314 - tmp318.xxx * tmp306) / tmp316;
                            float3 tmp320 = float3(tmp312 ? 0.0f : (tmp308.z * tmp306.z + tmp308.x * tmp306.x + tmp308.y * tmp306.y) * tmp311, 0.0f, 0.0f);
                            float3 tmp321 = (tmp308 * tmp314 - tmp320.xxx * tmp306) / tmp316;
                            float4 tmp322 = tmp317.zzzz;
                            float4 tmp323 = tmp317.yyyy;
                            float4 tmp324 = tmp317.xxxx;
                            float4 tmp325 = tmp319.zzzz;
                            float4 tmp326 = tmp319.yyyy;
                            float4 tmp327 = tmp319.xxxx;
                            float4 tmp328 = tmp321.zzzz;
                            float4 tmp329 = tmp321.yyyy;
                            float4 tmp330 = tmp321.xxxx;
                            float4 tmp331 = tmp299.zzzz;
                            float4 tmp332 = tmp299.yyyy;
                            float4 tmp333 = tmp301.zzzz;
                            float4 tmp334 = tmp301.yyyy;
                            float4 tmp335 = tmp303.zzzz;
                            float4 tmp336 = tmp303.yyyy;
                            float4 tmp337 = tmp277.zzzz;
                            float4 tmp338 = tmp277.yyyy;
                            float4 tmp339 = tmp277.xxxx;
                            float4 tmp340 = tmp279.zzzz;
                            float4 tmp341 = tmp279.yyyy;
                            float4 tmp342 = tmp279.xxxx;
                            float4 tmp343 = tmp281.zzzz;
                            float4 tmp344 = tmp281.yyyy;
                            float4 tmp345 = tmp281.xxxx;
                            float4 tmp346 = float4(tmp299.xxxx.x, tmp299.xxxx.y, tmp299.xxxx.z, 0.0f);
                            float4 tmp347 = float4(tmp346.x, tmp324.x, tmp346.x, tmp346.w);
                            float4 tmp348 = float4(tmp301.xxxx.x, tmp301.xxxx.y, tmp301.xxxx.z, 0.0f);
                            float4 tmp349 = float4(tmp348.x, tmp327.x, tmp348.x, tmp348.w);
                            float4 tmp350 = float4(tmp303.xxxx.x, tmp303.xxxx.y, tmp303.xxxx.z, 0.0f);
                            float4 tmp351 = float4(tmp350.x, tmp330.x, tmp350.x, tmp350.w);
                            float4 tmp352 = float4(tmp332.y, tmp332.y, tmp332.y, 0.0f);
                            float4 tmp353 = float4(tmp352.x, tmp323.y, tmp352.x, tmp352.w);
                            float4 tmp354 = float4(tmp334.y, tmp334.y, tmp334.y, 0.0f);
                            float4 tmp355 = float4(tmp354.x, tmp326.y, tmp354.x, tmp354.w);
                            float4 tmp356 = float4(tmp336.y, tmp336.y, tmp336.y, 0.0f);
                            float4 tmp357 = float4(tmp356.x, tmp329.y, tmp356.x, tmp356.w);
                            float4 tmp358 = float4(tmp331.z, tmp331.z, tmp331.z, 0.0f);
                            float4 tmp359 = float4(tmp358.x, tmp322.z, tmp358.x, tmp358.w);
                            float4 tmp360 = float4(tmp333.z, tmp333.z, tmp333.z, 0.0f);
                            float4 tmp361 = float4(tmp360.x, tmp325.z, tmp360.x, tmp360.w);
                            float4 tmp362 = float4(tmp335.z, tmp335.z, tmp335.z, 0.0f);
                            float4 tmp363 = float4(tmp362.x, tmp328.z, tmp362.x, tmp362.w);
                            tmp364 = constr_deriv_type97(float4x4(float4(tmp347.x, tmp347.y, tmp339.x, tmp347.w), float4(tmp353.x, tmp353.y, tmp338.y, tmp353.w), float4(tmp359.x, tmp359.y, tmp337.z, tmp359.w), float4(phi_out262.x - (tmp299.y * phi_out123.y + tmp299.x * phi_out123.x + tmp299.z * phi_out123.z), phi_out262.y - (tmp317.y * phi_out123.y + tmp317.x * phi_out123.x + tmp317.z * phi_out123.z), -(tmp277.z * phi_out123.z + tmp277.y * phi_out123.y + tmp277.x * phi_out123.x), 1.0f)), float4x4(float4(tmp349.x, tmp349.y, tmp342.x, tmp349.w), float4(tmp355.x, tmp355.y, tmp341.y, tmp355.w), float4(tmp361.x, tmp361.y, tmp340.z, tmp361.w), float4(phi_out260.x - (tmp299.x * phi_out121.x + tmp299.z * phi_out121.z + tmp299.y * phi_out121.y + tmp301.z * phi_out123.z + tmp301.x * phi_out123.x + tmp301.y * phi_out123.y), phi_out260.y - (tmp317.x * phi_out121.x + tmp317.z * phi_out121.z + tmp317.y * phi_out121.y + tmp319.z * phi_out123.z + tmp319.x * phi_out123.x + tmp319.y * phi_out123.y), tmp279.x * tmp268 - (tmp277.x * phi_out121.x + tmp277.z * phi_out121.z + tmp277.y * phi_out121.y + tmp279.y * phi_out123.y + tmp279.z * phi_out123.z), 0.0f)), float4x4(float4(tmp351.x, tmp351.y, tmp345.x, tmp351.w), float4(tmp357.x, tmp357.y, tmp344.y, tmp357.w), float4(tmp363.x, tmp363.y, tmp343.z, tmp363.w), float4(phi_out258.x - (tmp299.x * phi_out119.x + tmp299.z * phi_out119.z + tmp299.y * phi_out119.y + tmp303.z * phi_out123.z + tmp303.x * phi_out123.x + tmp303.y * phi_out123.y), phi_out258.y - (tmp317.x * phi_out119.x + tmp317.z * phi_out119.z + tmp317.y * phi_out119.y + tmp321.z * phi_out123.z + tmp321.x * phi_out123.x + tmp321.y * phi_out123.y), tmp281.x * tmp268 - (tmp277.x * phi_out119.x + tmp277.z * phi_out119.z + tmp277.y * phi_out119.y + tmp281.y * phi_out123.y + tmp281.z * phi_out123.z), 0.0f)));
                            phi_in129 = 0;
                            phi_in131 = phi_out123.z;
                            phi_in133 = phi_out123.y;
                            phi_in135 = phi_out123.x;
                            phi_in137 = tmp364;
                        }
                    }
                }
            }
        }
        phi_out130 = phi_in129;
        phi_out132 = phi_in131;
        phi_out134 = phi_in133;
        phi_out136 = phi_in135;
        phi_out138 = phi_in137;
        phi_in365 = phi_out132;
        phi_in367 = phi_out134;
        phi_in369 = phi_out136;
        phi_in371 = phi_out138;
        if (phi_out130 != 0) {
            tmp373 = uint(phi_out57 + -7) < 2u;
            float4 tmp374 = float4(phi_out117.x * tmp80 + phi_out117.y * tmp84 + phi_out117.z * tmp88, phi_out117.x * tmp81 + phi_out117.y * tmp85 + phi_out117.z * tmp89, phi_out117.x * tmp82 + phi_out117.y * tmp86 + phi_out117.z * tmp90, 0.0f);
            tmp375 = phi_out123.y * tmp84 + tmp92 + phi_out123.x * tmp80 + phi_out123.z * tmp88;
            tmp376 = phi_out121.z * tmp88 + phi_out121.x * tmp80 + phi_out121.y * tmp84;
            tmp377 = phi_out119.z * tmp88 + phi_out119.x * tmp80 + phi_out119.y * tmp84;
            tmp378 = phi_out123.y * tmp85 + tmp93 + phi_out123.x * tmp81 + phi_out123.z * tmp89;
            tmp379 = phi_out121.z * tmp89 + phi_out121.x * tmp81 + phi_out121.y * tmp85;
            tmp380 = phi_out119.z * tmp89 + phi_out119.x * tmp81 + phi_out119.y * tmp85;
            tmp381 = phi_out121.x * tmp82;
            tmp382 = phi_out119.x * tmp82;
            tmp383 = phi_out121.y * tmp86;
            tmp384 = phi_out119.y * tmp86;
            tmp385 = phi_out121.z * tmp90;
            tmp386 = phi_out119.z * tmp90;
            tmp387 = phi_out123.y * tmp86 + tmp94 + phi_out123.x * tmp82 + phi_out123.z * tmp90;
            tmp388 = tmp375 * (phi_out115.x * tmp80 + phi_out115.y * tmp84 + phi_out115.z * tmp88) + tmp378 * (phi_out115.x * tmp81 + phi_out115.y * tmp85 + phi_out115.z * tmp89) + tmp387 * (phi_out115.x * tmp82 + phi_out115.y * tmp86 + phi_out115.z * tmp90) < 0.0f ? -tmp374 : tmp374;
            phi_in389 = 1;
            phi_in391 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in393 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in395 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in397 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in399 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in401 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in403 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in405 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in407 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in409 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in411 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            phi_in413 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            if (phi_out57 != 8) {
                phi_in389 = 1;
                phi_in391 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in393 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in395 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in397 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in399 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in401 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in403 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in405 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in407 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in409 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in411 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                phi_in413 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                if (phi_out57 != 4) {
                    tmp415 = abs(tmp388.z);
                    phi_in389 = 1;
                    phi_in391 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in393 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in395 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in397 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in399 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in401 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in403 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in405 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in407 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in409 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in411 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    phi_in413 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    if (!(tmp415 < abs(tmp388.x))) {
                        phi_in389 = 1;
                        phi_in391 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in393 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in395 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in397 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in399 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in401 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in403 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in405 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in407 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in409 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in411 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        phi_in413 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                        if (!(tmp415 < abs(tmp388.y))) {
                            phi_in389 = 0;
                            phi_in391 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in393 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in395 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in397 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in399 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in401 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in403 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in405 = float4(0.0f, 0.0f, 0.0f, 0.0f);
                            phi_in407 = tmp95;
                            phi_in409 = tmp91;
                            phi_in411 = tmp87;
                            phi_in413 = tmp83;
                            if (tmp388.z < 0.0f) {
                                tmp416 = float4(-tmp80, tmp83.y, tmp83.z, tmp83.w);
                                tmp417 = float4(-tmp84, tmp87.y, tmp87.z, tmp87.w);
                                tmp418 = float4(-tmp88, tmp91.y, tmp91.z, tmp91.w);
                                tmp419 = float4(-tmp92, tmp95.y, tmp95.z, tmp95.w);
                                phi_in389 = 0;
                                phi_in391 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in393 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in395 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in397 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in399 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in401 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in403 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in405 = float4(-0.0f, 0.0f, 0.0f, 0.0f);
                                phi_in407 = tmp419;
                                phi_in409 = tmp418;
                                phi_in411 = tmp417;
                                phi_in413 = tmp416;
                            }
                        }
                    }
                }
            }
            phi_out390 = phi_in389;
            phi_out392 = phi_in391;
            phi_out394 = phi_in393;
            phi_out396 = phi_in395;
            phi_out398 = phi_in397;
            phi_out400 = phi_in399;
            phi_out402 = phi_in401;
            phi_out404 = phi_in403;
            phi_out406 = phi_in405;
            phi_out408 = phi_in407;
            phi_out410 = phi_in409;
            phi_out412 = phi_in411;
            phi_out414 = phi_in413;
            phi_in420 = phi_out392;
            phi_in422 = phi_out394;
            phi_in424 = phi_out396;
            phi_in426 = phi_out398;
            phi_in428 = phi_out400;
            phi_in430 = phi_out402;
            phi_in432 = phi_out404;
            phi_in434 = phi_out406;
            phi_in436 = phi_out408;
            phi_in438 = phi_out410;
            phi_in440 = phi_out412;
            phi_in442 = phi_out414;
            if (phi_out390 != 0) {
                if (tmp373) {
                    tmp444 = tmp375 * tmp375 + tmp378 * tmp378;
                    tmp445 = sqrt(tmp444);
                    float tmp446 = 1.0f / tmp445;
                    tmp447 = (tmp375 * tmp376 + tmp378 * tmp379) * tmp446;
                    tmp448 = tmp446 * (tmp375 * tmp377 + tmp378 * tmp380);
                    phi_in449 = tmp448;
                    phi_in451 = tmp447;
                    phi_in453 = tmp445;
                    phi_in455 = tmp444;
                    phi_in457 = 0.0f;
                    phi_in459 = 0.0f;
                    phi_in461 = 0.3183098733f;
                } else {
                    tmp463 = tmp375 * tmp375 + tmp378 * tmp378;
                    tmp464 = sqrt(tmp463);
                    float tmp465 = 1.0f / tmp464;
                    tmp466 = tmp465 * (tmp375 * tmp376 + tmp378 * tmp379);
                    tmp467 = tmp465 * (tmp375 * tmp377 + tmp378 * tmp380);
                    bool tmp468 = tmp463 <= 0.0f;
                    tmp469 = tmp468 ? 0.0f : tmp466;
                    tmp470 = tmp468 ? 0.0f : tmp467;
                    phi_in449 = tmp467;
                    phi_in451 = tmp466;
                    phi_in453 = tmp464;
                    phi_in455 = tmp463;
                    phi_in457 = tmp470;
                    phi_in459 = tmp469;
                    phi_in461 = tmp464;
                }
                phi_out450 = phi_in449;
                phi_out452 = phi_in451;
                phi_out454 = phi_in453;
                phi_out456 = phi_in455;
                phi_out458 = phi_in457;
                phi_out460 = phi_in459;
                phi_out462 = phi_in461;
                float tmp471 = atan2(tmp378, tmp375);
                float tmp472 = -phi_out123.x;
                float3 tmp473 = float3(tmp375, tmp378, 0.0f);
                bool tmp474 = phi_out456 <= 0.0f;
                float3 tmp475 = float3(phi_out454, 0.0f, 0.0f);
                float3 tmp476 = tmp475.xxx;
                float3 tmp477 = tmp475 * tmp475;
                float3 tmp478 = tmp477.xxx;
                float3 tmp479 = tmp473 / tmp476;
                float3 tmp480 = float3(tmp474 ? 0.0f : phi_out452, 0.0f, 0.0f);
                float3 tmp481 = (tmp476 * float3(tmp376, tmp379, 0.0f) - tmp480.xxx * tmp473) / tmp478;
                float3 tmp482 = float3(tmp474 ? 0.0f : phi_out450, 0.0f, 0.0f);
                float3 tmp483 = (tmp476 * float3(tmp377, tmp380, 0.0f) - tmp482.xxx * tmp473) / tmp478;
                float3 tmp484 = tmp479 * float3(1.0f, 0.0f, 0.0f);
                float3 tmp485 = tmp479 * float3(0.0f, 1.0f, 0.0f);
                float3 tmp486 = tmp484.zxy - tmp485.yzx;
                float3 tmp487 = tmp481 * float3(1.0f, 0.0f, 0.0f);
                float3 tmp488 = tmp481 * float3(0.0f, 1.0f, 0.0f);
                float3 tmp489 = tmp487.zxy - tmp488.yzx;
                float3 tmp490 = tmp483 * float3(1.0f, 0.0f, 0.0f);
                float3 tmp491 = tmp483 * float3(0.0f, 1.0f, 0.0f);
                float3 tmp492 = tmp490.zxy - tmp491.yzx;
                float tmp493 = tmp486.x * tmp486.x + tmp486.y * tmp486.y + tmp486.z * tmp486.z;
                float tmp494 = sqrt(tmp493);
                float tmp495 = 1.0f / tmp494;
                bool tmp496 = tmp493 <= 0.0f;
                float3 tmp497 = float3(tmp494, 0.0f, 0.0f);
                float3 tmp498 = tmp497.xxx;
                float3 tmp499 = tmp497 * tmp497;
                float3 tmp500 = tmp499.xxx;
                float3 tmp501 = tmp486 / tmp498;
                float3 tmp502 = float3(tmp496 ? 0.0f : (tmp489.z * tmp486.z + tmp489.x * tmp486.x + tmp489.y * tmp486.y) * tmp495, 0.0f, 0.0f);
                float3 tmp503 = (tmp498 * tmp489 - tmp502.xxx * tmp486) / tmp500;
                float3 tmp504 = float3(tmp496 ? 0.0f : (tmp492.z * tmp486.z + tmp492.x * tmp486.x + tmp492.y * tmp486.y) * tmp495, 0.0f, 0.0f);
                float3 tmp505 = (tmp498 * tmp492 - tmp504.xxx * tmp486) / tmp500;
                float4 tmp506 = tmp501.zzzz;
                float4 tmp507 = tmp501.yyyy;
                float4 tmp508 = tmp501.xxxx;
                float4 tmp509 = tmp503.zzzz;
                float4 tmp510 = tmp503.yyyy;
                float4 tmp511 = tmp503.xxxx;
                float4 tmp512 = tmp505.zzzz;
                float4 tmp513 = tmp505.yyyy;
                float4 tmp514 = tmp505.xxxx;
                float4 tmp515 = tmp479.zzzz;
                float4 tmp516 = tmp479.yyyy;
                float4 tmp517 = tmp479.xxxx;
                float4 tmp518 = tmp481.zzzz;
                float4 tmp519 = tmp481.yyyy;
                float4 tmp520 = tmp481.xxxx;
                float4 tmp521 = tmp483.zzzz;
                float4 tmp522 = tmp483.yyyy;
                float4 tmp523 = tmp483.xxxx;
                float4 tmp524 = float4(tmp508.x, 0.0f, tmp508.x, 0.0f);
                tmp525 = float4(tmp524.x, tmp524.y, tmp517.x, tmp524.w);
                float4 tmp526 = float4(tmp511.x, 0.0f, tmp511.x, 0.0f);
                tmp527 = float4(tmp526.x, tmp526.y, tmp520.x, tmp526.w);
                float4 tmp528 = float4(tmp514.x, 0.0f, tmp514.x, 0.0f);
                tmp529 = float4(tmp528.x, tmp528.y, tmp523.x, tmp528.w);
                float4 tmp530 = float4(tmp507.y, 0.0f, tmp507.y, 0.0f);
                tmp531 = float4(tmp530.x, tmp530.y, tmp516.y, tmp530.w);
                float4 tmp532 = float4(tmp510.y, 0.0f, tmp510.y, 0.0f);
                tmp533 = float4(tmp532.x, tmp532.y, tmp519.y, tmp532.w);
                float4 tmp534 = float4(tmp513.y, 0.0f, tmp513.y, 0.0f);
                tmp535 = float4(tmp534.x, tmp534.y, tmp522.y, tmp534.w);
                float4 tmp536 = float4(tmp506.z, 1.0f, tmp506.z, 0.0f);
                tmp537 = float4(tmp536.x, tmp536.y, tmp515.z, tmp536.w);
                float4 tmp538 = float4(tmp509.z, 0.0f, tmp509.z, 0.0f);
                tmp539 = float4(tmp538.x, tmp538.y, tmp518.z, tmp538.w);
                float4 tmp540 = float4(tmp512.z, 0.0f, tmp512.z, 0.0f);
                tmp541 = float4(tmp540.x, tmp540.y, tmp521.z, tmp540.w);
                tmp542 = float4(tmp471 * phi_out462 - (tmp501.y * phi_out123.y + tmp501.x * phi_out123.x + tmp501.z * phi_out123.z), tmp387 - phi_out123.z, -(tmp479.z * phi_out123.z + tmp479.y * phi_out123.y + tmp479.x * phi_out123.x), 1.0f);
                tmp543 = float4(phi_out462 * (tmp375 * tmp379 - tmp378 * tmp376) / phi_out456 + tmp471 * phi_out460 - (tmp501.x * phi_out121.x + tmp501.z * phi_out121.z + tmp501.y * phi_out121.y + tmp503.z * phi_out123.z + tmp503.x * phi_out123.x + tmp503.y * phi_out123.y), tmp385 - phi_out121.z + tmp381 + tmp383, tmp481.x * tmp472 - (tmp479.x * phi_out121.x + tmp479.z * phi_out121.z + tmp479.y * phi_out121.y + tmp481.y * phi_out123.y + tmp481.z * phi_out123.z), 0.0f);
                tmp544 = float4(phi_out462 * (tmp375 * tmp380 - tmp378 * tmp377) / phi_out456 + tmp471 * phi_out458 - (tmp501.x * phi_out119.x + tmp501.z * phi_out119.z + tmp501.y * phi_out119.y + tmp505.z * phi_out123.z + tmp505.x * phi_out123.x + tmp505.y * phi_out123.y), tmp386 - phi_out119.z + tmp382 + tmp384, tmp483.x * tmp472 - (tmp479.x * phi_out119.x + tmp479.z * phi_out119.z + tmp479.y * phi_out119.y + tmp483.y * phi_out123.y + tmp483.z * phi_out123.z), 0.0f);
                phi_in420 = tmp544;
                phi_in422 = tmp541;
                phi_in424 = tmp535;
                phi_in426 = tmp529;
                phi_in428 = tmp543;
                phi_in430 = tmp539;
                phi_in432 = tmp533;
                phi_in434 = tmp527;
                phi_in436 = tmp542;
                phi_in438 = tmp537;
                phi_in440 = tmp531;
                phi_in442 = tmp525;
            }
            phi_out421 = phi_in420;
            phi_out423 = phi_in422;
            phi_out425 = phi_in424;
            phi_out427 = phi_in426;
            phi_out429 = phi_in428;
            phi_out431 = phi_in430;
            phi_out433 = phi_in432;
            phi_out435 = phi_in434;
            phi_out437 = phi_in436;
            phi_out439 = phi_in438;
            phi_out441 = phi_in440;
            phi_out443 = phi_in442;
            tmp545 = constr_deriv_type97(float4x4(phi_out443, phi_out441, phi_out439, phi_out437), float4x4(phi_out435, phi_out433, phi_out431, phi_out429), float4x4(phi_out427, phi_out425, phi_out423, phi_out421));
            phi_in365 = phi_out123.z;
            phi_in367 = phi_out123.y;
            phi_in369 = phi_out123.x;
            phi_in371 = tmp545;
        }
        phi_out366 = phi_in365;
        phi_out368 = phi_in367;
        phi_out370 = phi_in369;
        phi_out372 = phi_in371;
        tmp546 = phi_out372.dy;
        tmp547 = phi_out372.dx;
        float4x4 tmp548 = phi_out372.val;
        tmp549 = tmp548[3];
        float4 tmp550 = tmp548[2];
        float4 tmp551 = tmp548[1];
        float4 tmp552 = tmp548[0];
        tmp553 = float3(tmp552.y, tmp551.y, tmp550.y);
        tmp554 = texture_coordinate_system == 1;
        if (tmp554) {
            float3 tmp555 = float3(sqrt(tmp552.x * tmp552.x + tmp551.x * tmp551.x + tmp550.x * tmp550.x), 0.0f, 0.0f);
            tmp556 = float3(tmp552.x, tmp551.x, tmp550.x) / tmp555.xxx;
            tmp557 = tmp552.y * tmp552.y + tmp551.y * tmp551.y;
            tmp558 = tmp550.y * tmp550.y;
            phi_in559 = tmp558;
            phi_in561 = tmp557;
            phi_in563 = tmp553;
            phi_in565 = tmp556;
        } else {
            float4 tmp567 = state.object_to_world[0];
            float4 tmp568 = state.object_to_world[1];
            float4 tmp569 = state.object_to_world[2];
            float tmp570 = tmp567.x * tmp552.x + tmp567.y * tmp551.x + tmp567.z * tmp550.x;
            float tmp571 = tmp568.x * tmp552.x + tmp568.y * tmp551.x + tmp568.z * tmp550.x;
            float tmp572 = tmp569.x * tmp552.x + tmp569.y * tmp551.x + tmp569.z * tmp550.x;
            float3 tmp573 = float3(sqrt(tmp571 * tmp571 + tmp570 * tmp570 + tmp572 * tmp572), 0.0f, 0.0f);
            tmp574 = float3(tmp570, tmp571, tmp572) / tmp573.xxx;
            tmp575 = _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(state, 1, 0, constr_Derived_float3(tmp553, float3(tmp547[0].y, tmp547[1].y, tmp547[2].y), float3(tmp546[0].y, tmp546[1].y, tmp546[2].y))).val;
            tmp576 = tmp575.x * tmp575.x + tmp575.y * tmp575.y;
            tmp577 = tmp575.z * tmp575.z;
            phi_in559 = tmp577;
            phi_in561 = tmp576;
            phi_in563 = tmp575;
            phi_in565 = tmp574;
        }
        phi_out560 = phi_in559;
        phi_out562 = phi_in561;
        phi_out564 = phi_in563;
        phi_out566 = phi_in565;
        float3 tmp578 = float3(sqrt(phi_out562 + phi_out560), 0.0f, 0.0f);
        tmp579 = phi_out564 / tmp578.xxx;
        tmp580 = state.normal;
        float3 tmp581 = tmp580 * tmp579;
        phi_in582 = 1;
        phi_in584 = float3(0.0f, 0.0f, 0.0f);
        phi_in586 = float3(0.0f, 0.0f, 0.0f);
        if (!(abs(tmp581.x + tmp581.y + tmp581.z) > 0.9990000129f)) {
            float3 tmp588 = tmp580 * phi_out566;
            phi_in582 = 1;
            phi_in584 = float3(0.0f, 0.0f, 0.0f);
            phi_in586 = float3(0.0f, 0.0f, 0.0f);
            if (!(abs(tmp588.x + tmp588.y + tmp588.z) > 0.9990000129f)) {
                float3 tmp589 = tmp580.zxy;
                float3 tmp590 = tmp580.yzx;
                float3 tmp591 = tmp589 * tmp579.yzx - tmp590 * tmp579.zxy;
                float3 tmp592 = float3(sqrt(tmp591.x * tmp591.x + tmp591.y * tmp591.y + tmp591.z * tmp591.z), 0.0f, 0.0f);
                float3 tmp593 = tmp591 / tmp592.xxx;
                float3 tmp594 = tmp593.zxy * tmp590 - tmp593.yzx * tmp589;
                float3 tmp595 = float3(sqrt(tmp594.x * tmp594.x + tmp594.y * tmp594.y + tmp594.z * tmp594.z), 0.0f, 0.0f);
                tmp596 = tmp594 / tmp595.xxx;
                float3 tmp597 = tmp593 * phi_out566;
                tmp598 = tmp597.x + tmp597.y + tmp597.z < 0.0f ? -tmp593 : tmp593;
                float3 tmp599 = tmp596 * tmp579;
                phi_in582 = 0;
                phi_in584 = tmp598;
                phi_in586 = tmp596;
                if (tmp599.x + tmp599.y + tmp599.z < 0.0f) {
                    tmp600 = -tmp596;
                    phi_in582 = 0;
                    phi_in584 = tmp598;
                    phi_in586 = tmp600;
                }
            }
        }
        phi_out583 = phi_in582;
        phi_out585 = phi_in584;
        phi_out587 = phi_in586;
        phi_in601 = phi_out585;
        phi_in603 = phi_out587;
        if (phi_out583 != 0) {
            tmp605 = tmp554 ? 2 : 1;
            Derived_float3 tmp606 = _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(state, 0, tmp605, constr_Derived_float3(tmp580, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f)));
            tmp607 = tmp606.val;
            tmp608 = tmp606.dx;
            tmp609 = tmp606.dy;
            tmp610 = tmp607.z * -tmp607.y;
            tmp611 = tmp607.z * -tmp608.y - tmp607.y * tmp608.z;
            tmp612 = tmp607.z * -tmp609.y - tmp607.y * tmp609.z;
            tmp613 = -tmp609.x;
            tmp614 = -tmp608.x;
            tmp615 = -tmp607.x;
            if (abs(tmp607.z) > 0.9999899864f) {
                float tmp616 = tmp607.y * tmp615;
                float tmp617 = tmp607.y * tmp614 - tmp607.x * tmp608.y;
                float tmp618 = tmp607.y * tmp613 - tmp607.x * tmp609.y;
                float tmp619 = tmp607.y * 2.0f;
                float tmp620 = tmp619 * tmp608.y;
                float tmp621 = tmp619 * tmp609.y;
                float tmp622 = 1.0f - tmp607.y * tmp607.y;
                float3 tmp623 = float3(tmp616, tmp622, tmp610);
                float tmp624 = tmp616 * tmp616 + tmp622 * tmp622 + tmp610 * tmp610;
                float tmp625 = sqrt(tmp624);
                float tmp626 = 1.0f / tmp625;
                bool tmp627 = tmp624 <= 0.0f;
                float3 tmp628 = float3(tmp625, 0.0f, 0.0f);
                float3 tmp629 = tmp628.xxx;
                float3 tmp630 = tmp628 * tmp628;
                tmp631 = tmp630.xxx;
                tmp632 = tmp623 / tmp629;
                float3 tmp633 = float3(tmp627 ? 0.0f : tmp626 * (tmp617 * tmp616 - tmp620 * tmp622 + tmp611 * tmp610), 0.0f, 0.0f);
                tmp634 = (tmp629 * float3(tmp617, -tmp620, tmp611) - tmp633.xxx * tmp623) / tmp631;
                float3 tmp635 = float3(tmp627 ? 0.0f : tmp626 * (tmp618 * tmp616 - tmp621 * tmp622 + tmp612 * tmp610), 0.0f, 0.0f);
                tmp636 = tmp635.xxx * tmp623;
                tmp637 = tmp629 * float3(tmp618, -tmp621, tmp612);
                phi_in638 = tmp636;
                phi_in640 = tmp637;
                phi_in642 = tmp631;
                phi_in644 = tmp632;
                phi_in646 = tmp634;
            } else {
                float tmp648 = tmp607.z * tmp615;
                float tmp649 = tmp607.z * tmp614 - tmp607.x * tmp608.z;
                float tmp650 = tmp607.z * tmp613 - tmp607.x * tmp609.z;
                float tmp651 = tmp607.z * 2.0f;
                float tmp652 = tmp651 * tmp608.z;
                float tmp653 = tmp651 * tmp609.z;
                float tmp654 = 1.0f - tmp607.z * tmp607.z;
                float3 tmp655 = float3(tmp648, tmp610, tmp654);
                float tmp656 = tmp648 * tmp648 + tmp610 * tmp610 + tmp654 * tmp654;
                float tmp657 = sqrt(tmp656);
                float tmp658 = 1.0f / tmp657;
                bool tmp659 = tmp656 <= 0.0f;
                float3 tmp660 = float3(tmp657, 0.0f, 0.0f);
                float3 tmp661 = tmp660.xxx;
                float3 tmp662 = tmp660 * tmp660;
                tmp663 = tmp662.xxx;
                tmp664 = tmp655 / tmp661;
                float3 tmp665 = float3(tmp659 ? 0.0f : tmp658 * (tmp649 * tmp648 - tmp652 * tmp654 + tmp611 * tmp610), 0.0f, 0.0f);
                tmp666 = (tmp661 * float3(tmp649, tmp611, -tmp652) - tmp665.xxx * tmp655) / tmp663;
                float3 tmp667 = float3(tmp659 ? 0.0f : tmp658 * (tmp650 * tmp648 - tmp653 * tmp654 + tmp612 * tmp610), 0.0f, 0.0f);
                tmp668 = tmp667.xxx * tmp655;
                tmp669 = tmp661 * float3(tmp650, tmp612, -tmp653);
                phi_in638 = tmp668;
                phi_in640 = tmp669;
                phi_in642 = tmp663;
                phi_in644 = tmp664;
                phi_in646 = tmp666;
            }
            phi_out639 = phi_in638;
            phi_out641 = phi_in640;
            phi_out643 = phi_in642;
            phi_out645 = phi_in644;
            phi_out647 = phi_in646;
            float3 tmp670 = (phi_out641 - phi_out639) / phi_out643;
            float3 tmp671 = phi_out645.yzx;
            float3 tmp672 = tmp607.zxy;
            float3 tmp673 = phi_out645.zxy;
            float3 tmp674 = tmp607.yzx;
            float3 tmp675 = tmp671 * tmp672 - tmp673 * tmp674;
            float3 tmp676 = phi_out647.yzx * tmp672 + tmp671 * tmp608.zxy - (phi_out647.zxy * tmp674 + tmp673 * tmp608.yzx);
            float3 tmp677 = tmp670.yzx * tmp672 + tmp671 * tmp609.zxy - (tmp670.zxy * tmp674 + tmp673 * tmp609.yzx);
            float tmp678 = tmp675.x * tmp675.x + tmp675.y * tmp675.y + tmp675.z * tmp675.z;
            float tmp679 = sqrt(tmp678);
            float tmp680 = 1.0f / tmp679;
            bool tmp681 = tmp678 <= 0.0f;
            float3 tmp682 = float3(tmp679, 0.0f, 0.0f);
            float3 tmp683 = tmp682.xxx;
            float3 tmp684 = tmp682 * tmp682;
            float3 tmp685 = tmp684.xxx;
            float3 tmp686 = float3(tmp681 ? 0.0f : tmp680 * (tmp676.z * tmp675.z + tmp676.x * tmp675.x + tmp676.y * tmp675.y), 0.0f, 0.0f);
            float3 tmp687 = float3(tmp681 ? 0.0f : (tmp677.z * tmp675.z + tmp677.x * tmp675.x + tmp677.y * tmp675.y) * tmp680, 0.0f, 0.0f);
            float3 tmp688 = _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(state, tmp605, 0, constr_Derived_float3(phi_out645, phi_out647, tmp670)).val;
            float3 tmp689 = float3(sqrt(tmp688.x * tmp688.x + tmp688.y * tmp688.y + tmp688.z * tmp688.z), 0.0f, 0.0f);
            tmp690 = tmp688 / tmp689.xxx;
            float3 tmp691 = _ZN5state16transform_vectorEN5state16coordinate_spaceEN5state16coordinate_spaceEu6float3(state, tmp605, 0, constr_Derived_float3(tmp675 / tmp683, (tmp683 * tmp676 - tmp686.xxx * tmp675) / tmp685, (tmp683 * tmp677 - tmp687.xxx * tmp675) / tmp685)).val;
            float3 tmp692 = float3(sqrt(tmp691.x * tmp691.x + tmp691.y * tmp691.y + tmp691.z * tmp691.z), 0.0f, 0.0f);
            tmp693 = tmp691 / tmp692.xxx;
            phi_in601 = tmp693;
            phi_in603 = tmp690;
        }
        phi_out602 = phi_in601;
        phi_out604 = phi_in603;
        tmp694 = float3(tmp551.x * phi_out368 + tmp549.x + tmp552.x * phi_out370 + tmp550.x * phi_out366, tmp551.y * phi_out368 + tmp549.y + tmp552.y * phi_out370 + tmp550.y * phi_out366, 0.0f);
        phi_in5 = phi_out602;
        phi_in7 = phi_out604;
        phi_in9 = 0;
        phi_in11 = tmp694;
    }
    phi_out6 = phi_in5;
    phi_out8 = phi_in7;
    phi_out10 = phi_in9;
    phi_out12 = phi_in11;
    float tmp695 = ((swap_st ? phi_out12.y : phi_out12.x) + s_offset.val) * s_scale.val;
    float tmp696 = ((swap_st ? phi_out12.x : phi_out12.y) + t_offset.val) * t_scale.val;
    float3 tmp697 = float3(s_flip ? -tmp695 : tmp695, t_flip ? -tmp696 : tmp696, 0.0f);
    float3 tmp698 = float3(s_flip ? -0.0f : 0.0f, t_flip ? -0.0f : 0.0f, 0.0f);
    return constr_deriv_type0(constr__ZN4base23texture_coordinate_infoE(float3(tmp697.x, tmp697.y, phi_out12.z), phi_out6, phi_out8, phi_out10), constr__ZN4base23texture_coordinate_infoE(tmp698, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 0), constr__ZN4base23texture_coordinate_infoE(tmp698, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 0));
}

structtype0 _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(
    in int texture0,
    in bool ignore_missing_texture,
    in float3 missing_color,
    in bool use_uv_coords,
    in Derived_float2 uv_coords,
    in int s_wrap,
    in int t_wrap,
    in deriv_type0 texture_coordinate_info)
{
    float3 tmp1;
    float3 tmp2;
    float3 tmp3;
    int phi_in;
    int phi_out;
    float phi_in4;
    float phi_out5;
    float3 phi_in6;
    float3 phi_out7;
    float phi_in8;
    float phi_out9;
    float3 phi_in10;
    float3 phi_out11;
    float3 sel;
    float3 sel4;
    float3 sel5;
    int phi_in12;
    int phi_out13;
    int phi_in14;
    int phi_out15;
    float phi_in16;
    float phi_out17;
    float3 phi_in18;
    float3 phi_out19;
    int phi_in20;
    int phi_out21;
    int phi_in22;
    int phi_out23;
    float phi_in24;
    float phi_out25;
    float3 phi_in26;
    float3 phi_out27;
    float3 tmp29;
    tmp1 = texture_coordinate_info.val.position;
    tmp2 = texture_coordinate_info.dx.position;
    tmp3 = texture_coordinate_info.dy.position;
    phi_in = 0;
    phi_in4 = 0.0f;
    phi_in6 = float3(0.0f, 0.0f, 0.0f);
    if (ignore_missing_texture) {
        phi_in = 0;
        phi_in4 = 0.0f;
        phi_in6 = float3(0.0f, 0.0f, 0.0f);
        if (!tex_texture_isvalid(texture0)) {
            phi_in = 1;
            phi_in4 = 0.0f;
            phi_in6 = missing_color;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    phi_in8 = phi_out5;
    phi_in10 = phi_out7;
    if (phi_out == 0) {
        phi_in8 = 0.0f;
        phi_in10 = missing_color;
        if (tex_width_2d(texture0, int2(0, 0), 0.0f) != 0) {
            phi_in8 = 0.0f;
            phi_in10 = missing_color;
            if (tex_height_2d(texture0, int2(0, 0), 0.0f) != 0) {
                float2 val = uv_coords.val;
                sel = use_uv_coords ? float3(0.0f, 0.0f, 0.0f) : tmp2;
                sel4 = use_uv_coords ? float3(0.0f, 0.0f, 0.0f) : tmp3;
                sel5 = use_uv_coords ? val.xyx : tmp1;
                if (s_wrap == 0) {
                    phi_in12 = 0;
                    phi_in14 = 1;
                    phi_in16 = 0.0f;
                    phi_in18 = float3(0.0f, 0.0f, 0.0f);
                } else if (s_wrap == 5) {
                    phi_in12 = 0;
                    phi_in14 = 3;
                    phi_in16 = 0.0f;
                    phi_in18 = float3(0.0f, 0.0f, 0.0f);
                    if (sel5.x < 0.0f || sel5.x > 1.0f) {
                        phi_in12 = 1;
                        phi_in14 = 0;
                        phi_in16 = 0.0f;
                        phi_in18 = missing_color;
                    }
                } else if (s_wrap == 2) {
                    phi_in12 = 0;
                    phi_in14 = 0;
                    phi_in16 = 0.0f;
                    phi_in18 = float3(0.0f, 0.0f, 0.0f);
                } else if (s_wrap == 3) {
                    phi_in12 = 0;
                    phi_in14 = 2;
                    phi_in16 = 0.0f;
                    phi_in18 = float3(0.0f, 0.0f, 0.0f);
                } else {
                    phi_in12 = 0;
                    phi_in14 = 3;
                    phi_in16 = 0.0f;
                    phi_in18 = float3(0.0f, 0.0f, 0.0f);
                }
                phi_out13 = phi_in12;
                phi_out15 = phi_in14;
                phi_out17 = phi_in16;
                phi_out19 = phi_in18;
                phi_in8 = phi_out17;
                phi_in10 = phi_out19;
                if (phi_out13 == 0) {
                    if (t_wrap == 0) {
                        phi_in20 = 0;
                        phi_in22 = 1;
                        phi_in24 = 0.0f;
                        phi_in26 = float3(0.0f, 0.0f, 0.0f);
                    } else if (t_wrap == 5) {
                        phi_in20 = 0;
                        phi_in22 = 3;
                        phi_in24 = 0.0f;
                        phi_in26 = float3(0.0f, 0.0f, 0.0f);
                        if (sel5.y < 0.0f || sel5.y > 1.0f) {
                            phi_in20 = 1;
                            phi_in22 = 0;
                            phi_in24 = 0.0f;
                            phi_in26 = missing_color;
                        }
                    } else if (t_wrap == 2) {
                        phi_in20 = 0;
                        phi_in22 = 0;
                        phi_in24 = 0.0f;
                        phi_in26 = float3(0.0f, 0.0f, 0.0f);
                    } else if (t_wrap == 3) {
                        phi_in20 = 0;
                        phi_in22 = 2;
                        phi_in24 = 0.0f;
                        phi_in26 = float3(0.0f, 0.0f, 0.0f);
                    } else {
                        phi_in20 = 0;
                        phi_in22 = 3;
                        phi_in24 = 0.0f;
                        phi_in26 = float3(0.0f, 0.0f, 0.0f);
                    }
                    phi_out21 = phi_in20;
                    phi_out23 = phi_in22;
                    phi_out25 = phi_in24;
                    phi_out27 = phi_in26;
                    phi_in8 = phi_out25;
                    phi_in10 = phi_out27;
                    if (phi_out21 == 0) {
                        float4 tmp28 = tex_lookup_deriv_float4_2d(texture0, constr_Derived_float2(sel5.xy, sel.xy, sel4.xy), phi_out15, phi_out23, float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f);
                        tmp29 = tmp28.xyz;
                        phi_in8 = tmp28.w;
                        phi_in10 = tmp29;
                    }
                }
            }
        }
    }
    phi_out9 = phi_in8;
    phi_out11 = phi_in10;
    return constr_structtype0(phi_out11, phi_out9);
}

float _ZN11OmniSurface10OmniShared19ior_preset_to_valueEN11OmniSurface10OmniShared11ior_presetsEU7uniformf(in int preset, in float custom)
{
    float phi_in;
    float phi_out;
    phi_in = 1.49000001f;
    if (preset != 0) {
        if (preset == 1)
            phi_in = 1.0f;
        else if (preset == 2)
            phi_in = 2.0f;
        else if (preset == 3)
            phi_in = 2.420000076f;
        else if (preset == 4)
            phi_in = 1.559999943f;
        else if (preset == 5)
            phi_in = 1.360000014f;
        else if (preset == 6)
            phi_in = 1.659999967f;
        else if (preset == 7)
            phi_in = 1.5f;
        else if (preset == 8)
            phi_in = 1.483999968f;
        else if (preset == 9)
            phi_in = 1.330000043f;
        else if (preset == 10)
            phi_in = 1.370000005f;
        else if (preset == 11)
            phi_in = 1.409999967f;
        else if (preset == 12)
            phi_in = 1.340000033f;
        else if (preset == 13)
            phi_in = 1.399999976f;
        else if (preset == 14)
            phi_in = 1.549999952f;
        else if (preset == 15)
            phi_in = 1.850000024f;
        else if (preset == 16)
            phi_in = 1.309999943f;
        else if (preset == 17)
            phi_in = 1.350000024f;
        else if (preset == 18)
            phi_in = 1.470000029f;
        else if (preset == 19)
            phi_in = 1.529999971f;
        else if (preset == 20)
            phi_in = 1.549999952f;
        else if (preset == 21)
            phi_in = 1.769999981f;
        else if (preset == 22)
            phi_in = 1.399999976f;
        else if (preset == 23)
            phi_in = 1.0f;
        else if (preset == 24)
            phi_in = 1.332999945f;
        else if (preset == 25)
            phi_in = 1.325000048f;
        else if (preset == 26)
            phi_in = 1.317999959f;
        else
            phi_in = custom;
    }
    phi_out = phi_in;
    return phi_out;
}

float3 _ZN11OmniSurface9OmniImage29tangent_space_normal_lookup_2EU7uniformu10texture_2dU7uniformbU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformbU7uniformbU7uniformfU7uniformbU7uniformbU7uniformbN4base23texture_coordinate_infoE(
    in Shading_state_material state,
    in int texture0,
    in bool ignore_missing_texture,
    in bool use_uv_coords,
    in Derived_float2 uv_coords,
    in int s_wrap,
    in int t_wrap,
    in bool s_flip,
    in bool t_flip,
    in float tangent_space_factor,
    in bool tangent_space_flip_r_channel,
    in bool tangent_space_flip_g_channel,
    in bool tangent_space_flip_b_channel,
    in deriv_type0 texture_coordinate_info)
{
    float3 tmp1;
    float3 tmp2;
    float3 tmp3;
    float3 tmp4;
    float3 tmp5;
    int phi_in;
    int phi_out;
    float phi_in6;
    float phi_out7;
    float phi_in8;
    float phi_out9;
    float3 phi_in10;
    float3 phi_out11;
    int phi_in12;
    int phi_out13;
    float phi_in14;
    float phi_out15;
    float phi_in16;
    float phi_out17;
    float3 phi_in18;
    float3 phi_out19;
    float3 sel;
    float3 sel175;
    float3 sel176;
    int phi_in20;
    int phi_out21;
    bool phi_in22;
    bool phi_out23;
    int phi_in24;
    int phi_out25;
    int phi_in26;
    int phi_out27;
    float phi_in28;
    float phi_out29;
    float phi_in30;
    float phi_out31;
    float3 phi_in32;
    float3 phi_out33;
    float3 tmp34;
    float tmp35;
    float tmp36;
    int phi_in37;
    int phi_out38;
    bool phi_in39;
    bool phi_out40;
    int phi_in41;
    int phi_out42;
    int phi_in43;
    int phi_out44;
    float phi_in45;
    float phi_out46;
    float phi_in47;
    float phi_out48;
    float3 phi_in49;
    float3 phi_out50;
    float3 tmp51;
    float tmp52;
    float tmp53;
    bool phi_in54;
    bool phi_out55;
    int phi_in56;
    int phi_out57;
    bool phi_in58;
    bool phi_out59;
    bool tmp60;
    bool tmp61;
    bool phi_in62;
    bool phi_out63;
    int phi_in64;
    int phi_out65;
    bool phi_in66;
    bool phi_out67;
    bool tmp68;
    bool tmp69;
    float3 tmp75;
    float tmp76;
    float tmp77;
    float phi_in78;
    float phi_out79;
    float phi_in80;
    float phi_out81;
    float3 phi_in82;
    float3 phi_out83;
    float3 tmp84;
    float tmp85;
    float tmp86;
    tmp1 = texture_coordinate_info.val.position;
    tmp2 = texture_coordinate_info.val.tangent_u;
    tmp3 = texture_coordinate_info.val.tangent_v;
    tmp4 = texture_coordinate_info.dx.position;
    tmp5 = texture_coordinate_info.dy.position;
    phi_in = 0;
    phi_in6 = 0.0f;
    phi_in8 = 0.0f;
    phi_in10 = float3(0.0f, 0.0f, 0.0f);
    if (ignore_missing_texture) {
        phi_in = 0;
        phi_in6 = 0.0f;
        phi_in8 = 0.0f;
        phi_in10 = float3(0.0f, 0.0f, 0.0f);
        if (!tex_texture_isvalid(texture0)) {
            phi_in = 1;
            phi_in6 = 0.0f;
            phi_in8 = 0.0f;
            phi_in10 = float3(0.0f, 0.0f, 0.0f);
        }
    }
    phi_out = phi_in;
    phi_out7 = phi_in6;
    phi_out9 = phi_in8;
    phi_out11 = phi_in10;
    if (phi_out == 0) {
        phi_in12 = 0;
        phi_in14 = 0.0f;
        phi_in16 = 0.0f;
        phi_in18 = float3(0.0f, 0.0f, 0.0f);
        if (tex_width_2d(texture0, int2(0, 0), 0.0f) != 0) {
            phi_in12 = 0;
            phi_in14 = 0.0f;
            phi_in16 = 0.0f;
            phi_in18 = float3(0.0f, 0.0f, 0.0f);
            if (tex_height_2d(texture0, int2(0, 0), 0.0f) != 0) {
                float2 val = uv_coords.val;
                sel = use_uv_coords ? float3(0.0f, 0.0f, 0.0f) : tmp5;
                sel175 = use_uv_coords ? float3(0.0f, 0.0f, 0.0f) : tmp4;
                sel176 = use_uv_coords ? val.xyx : tmp1;
                if (s_wrap == 0) {
                    phi_in20 = 0;
                    phi_in22 = false;
                    phi_in24 = 1;
                    phi_in26 = 0;
                    phi_in28 = 0.0f;
                    phi_in30 = 0.0f;
                    phi_in32 = float3(0.0f, 0.0f, 0.0f);
                } else if (s_wrap == 5) {
                    phi_in20 = 0;
                    phi_in22 = false;
                    phi_in24 = 3;
                    phi_in26 = 0;
                    phi_in28 = 0.0f;
                    phi_in30 = 0.0f;
                    phi_in32 = float3(0.0f, 0.0f, 0.0f);
                    if (sel176.x < 0.0f || sel176.x > 1.0f) {
                        tmp34 = tmp2 + tmp3 + state.normal;
                        tmp35 = tmp34.x * tmp34.x + tmp34.y * tmp34.y;
                        tmp36 = tmp34.z * tmp34.z;
                        phi_in20 = 1;
                        phi_in22 = false;
                        phi_in24 = 0;
                        phi_in26 = 1;
                        phi_in28 = tmp36;
                        phi_in30 = tmp35;
                        phi_in32 = tmp34;
                    }
                } else if (s_wrap == 2) {
                    phi_in20 = 0;
                    phi_in22 = false;
                    phi_in24 = 0;
                    phi_in26 = 0;
                    phi_in28 = 0.0f;
                    phi_in30 = 0.0f;
                    phi_in32 = float3(0.0f, 0.0f, 0.0f);
                } else if (s_wrap == 3) {
                    phi_in20 = 0;
                    phi_in22 = true;
                    phi_in24 = 2;
                    phi_in26 = 0;
                    phi_in28 = 0.0f;
                    phi_in30 = 0.0f;
                    phi_in32 = float3(0.0f, 0.0f, 0.0f);
                } else {
                    phi_in20 = 0;
                    phi_in22 = false;
                    phi_in24 = 3;
                    phi_in26 = 0;
                    phi_in28 = 0.0f;
                    phi_in30 = 0.0f;
                    phi_in32 = float3(0.0f, 0.0f, 0.0f);
                }
                phi_out21 = phi_in20;
                phi_out23 = phi_in22;
                phi_out25 = phi_in24;
                phi_out27 = phi_in26;
                phi_out29 = phi_in28;
                phi_out31 = phi_in30;
                phi_out33 = phi_in32;
                phi_in12 = phi_out27;
                phi_in14 = phi_out29;
                phi_in16 = phi_out31;
                phi_in18 = phi_out33;
                if (phi_out21 == 0) {
                    if (t_wrap == 0) {
                        phi_in37 = 0;
                        phi_in39 = false;
                        phi_in41 = 1;
                        phi_in43 = 0;
                        phi_in45 = 0.0f;
                        phi_in47 = 0.0f;
                        phi_in49 = float3(0.0f, 0.0f, 0.0f);
                    } else if (t_wrap == 5) {
                        phi_in37 = 0;
                        phi_in39 = false;
                        phi_in41 = 3;
                        phi_in43 = 0;
                        phi_in45 = 0.0f;
                        phi_in47 = 0.0f;
                        phi_in49 = float3(0.0f, 0.0f, 0.0f);
                        if (sel176.y < 0.0f || sel176.y > 1.0f) {
                            tmp51 = tmp2 + tmp3 + state.normal;
                            tmp52 = tmp51.x * tmp51.x + tmp51.y * tmp51.y;
                            tmp53 = tmp51.z * tmp51.z;
                            phi_in37 = 1;
                            phi_in39 = false;
                            phi_in41 = 0;
                            phi_in43 = 1;
                            phi_in45 = tmp53;
                            phi_in47 = tmp52;
                            phi_in49 = tmp51;
                        }
                    } else if (t_wrap == 2) {
                        phi_in37 = 0;
                        phi_in39 = false;
                        phi_in41 = 0;
                        phi_in43 = 0;
                        phi_in45 = 0.0f;
                        phi_in47 = 0.0f;
                        phi_in49 = float3(0.0f, 0.0f, 0.0f);
                    } else if (t_wrap == 3) {
                        phi_in37 = 0;
                        phi_in39 = true;
                        phi_in41 = 2;
                        phi_in43 = 0;
                        phi_in45 = 0.0f;
                        phi_in47 = 0.0f;
                        phi_in49 = float3(0.0f, 0.0f, 0.0f);
                    } else {
                        phi_in37 = 0;
                        phi_in39 = false;
                        phi_in41 = 3;
                        phi_in43 = 0;
                        phi_in45 = 0.0f;
                        phi_in47 = 0.0f;
                        phi_in49 = float3(0.0f, 0.0f, 0.0f);
                    }
                    phi_out38 = phi_in37;
                    phi_out40 = phi_in39;
                    phi_out42 = phi_in41;
                    phi_out44 = phi_in43;
                    phi_out46 = phi_in45;
                    phi_out48 = phi_in47;
                    phi_out50 = phi_in49;
                    phi_in12 = phi_out44;
                    phi_in14 = phi_out46;
                    phi_in16 = phi_out48;
                    phi_in18 = phi_out50;
                    if (phi_out38 == 0) {
                        phi_in54 = s_flip;
                        if (phi_out23) {
                            phi_in56 = 0;
                            phi_in58 = false;
                            if (sel176.x > 0.0f) {
                                phi_in56 = 0;
                                phi_in58 = false;
                                if (int(sel176.x) % 2 == 1) {
                                    tmp60 = s_flip != true;
                                    phi_in56 = 1;
                                    phi_in58 = tmp60;
                                }
                            }
                            phi_out57 = phi_in56;
                            phi_out59 = phi_in58;
                            phi_in54 = phi_out59;
                            if (phi_out57 == 0) {
                                phi_in54 = s_flip;
                                if (sel176.x < 0.0f) {
                                    tmp61 = (int(sel176.x) & 1) == 0 != s_flip;
                                    phi_in54 = tmp61;
                                }
                            }
                        }
                        phi_out55 = phi_in54;
                        phi_in62 = t_flip;
                        if (phi_out40) {
                            phi_in64 = 0;
                            phi_in66 = false;
                            if (sel176.y > 0.0f) {
                                phi_in64 = 0;
                                phi_in66 = false;
                                if (int(sel176.y) % 2 == 1) {
                                    tmp68 = t_flip != true;
                                    phi_in64 = 1;
                                    phi_in66 = tmp68;
                                }
                            }
                            phi_out65 = phi_in64;
                            phi_out67 = phi_in66;
                            phi_in62 = phi_out67;
                            if (phi_out65 == 0) {
                                phi_in62 = t_flip;
                                if (sel176.y < 0.0f) {
                                    tmp69 = (int(sel176.y) & 1) == 0 != t_flip;
                                    phi_in62 = tmp69;
                                }
                            }
                        }
                        phi_out63 = phi_in62;
                        float3 tmp70 = tex_lookup_deriv_float3_2d(texture0, constr_Derived_float2(sel176.xy, sel175.xy, sel.xy), phi_out25, phi_out42, float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f);
                        float3 result = tangent_space_flip_r_channel ? float3(1.0f - tmp70.x, tmp70.y, tmp70.z) : tmp70;
                        float3 result71 = tangent_space_flip_g_channel ? result : float3(result.x, 1.0f - result.y, result.z);
                        float3 tmp72 = float3(tangent_space_factor * 2.0f, 0.0f, 0.0f);
                        float3 tmp73 = ((tangent_space_flip_b_channel ? float3(result71.x, result71.y, 1.0f - result71.z) : result71) + float3(-0.5f, -0.5f, -0.5f)) * tmp72.xxx;
                        float3 tmp74 = float3(1.0f - tangent_space_factor + tmp73.z, 0.0f, 0.0f);
                        tmp75 = tmp73.xxx * (phi_out55 ? -tmp2 : tmp2) + tmp73.yyy * (phi_out63 ? -tmp3 : tmp3) + tmp74.xxx * state.normal;
                        tmp76 = tmp75.x * tmp75.x + tmp75.y * tmp75.y;
                        tmp77 = tmp75.z * tmp75.z;
                        phi_in12 = 1;
                        phi_in14 = tmp77;
                        phi_in16 = tmp76;
                        phi_in18 = tmp75;
                    }
                }
            }
        }
    } else {
        phi_in12 = 0;
        phi_in14 = 0.0f;
        phi_in16 = 0.0f;
        phi_in18 = float3(0.0f, 0.0f, 0.0f);
        if (phi_out != 1) {
            phi_in12 = 1;
            phi_in14 = phi_out7;
            phi_in16 = phi_out9;
            phi_in18 = phi_out11;
        }
    }
    phi_out13 = phi_in12;
    phi_out15 = phi_in14;
    phi_out17 = phi_in16;
    phi_out19 = phi_in18;
    phi_in78 = phi_out15;
    phi_in80 = phi_out17;
    phi_in82 = phi_out19;
    if (phi_out13 == 0) {
        tmp84 = tmp2 + tmp3 + state.normal;
        tmp85 = tmp84.x * tmp84.x + tmp84.y * tmp84.y;
        tmp86 = tmp84.z * tmp84.z;
        phi_in78 = tmp86;
        phi_in80 = tmp85;
        phi_in82 = tmp84;
    }
    phi_out79 = phi_in78;
    phi_out81 = phi_in80;
    phi_out83 = phi_in82;
    float3 tmp87 = float3(sqrt(phi_out81 + phi_out79), 0.0f, 0.0f);
    return phi_out83 / tmp87.xxx;
}

public void mdl_init(inout Shading_state_material state)
{
    deriv_type0 tmp14;
    int tmp27;
    int tmp28;
    bool tmp29;
    float3 tmp30;
    bool tmp31;
    Derived_float2 tmp32;
    int tmp33;
    int tmp34;
    structtype0 tmp35;
    float3 tmp36;
    int tmp37;
    float tmp38;
    float phi_in;
    float phi_out;
    float tmp39;
    float tmp40;
    float tmp41;
    int tmp42;
    structtype0 tmp43;
    float3 tmp44;
    int tmp45;
    float tmp46;
    float phi_in47;
    float phi_out48;
    float tmp49;
    float tmp50;
    float tmp51;
    float3 tmp53;
    int tmp54;
    structtype0 tmp55;
    float3 tmp56;
    int tmp57;
    float tmp58;
    float phi_in59;
    float phi_out60;
    float tmp61;
    float tmp62;
    float tmp63;
    int tmp64;
    int tmp65;
    int tmp66;
    float3 phi_in67;
    float3 phi_out68;
    float3 phi_in69;
    float3 phi_out70;
    float3 tmp71;
    float3 tmp72;
    float tmp73;
    bool tmp75;
    structtype0 tmp76;
    float3 tmp77;
    int tmp78;
    float tmp79;
    float phi_in80;
    float phi_out81;
    float tmp82;
    float tmp83;
    float3 tmp85;
    float3 tmp86;
    float3 tmp89;
    float3 tmp90;
    bool tmp92;
    structtype0 tmp93;
    float3 tmp94;
    int tmp95;
    float tmp96;
    float phi_in97;
    float phi_out98;
    float tmp99;
    float tmp100;
    float3 tmp103;
    float3 tmp105;
    float tmp106;
    float3 phi_in107;
    float3 phi_out108;
    float3 tmp116;
    int tmp119;
    structtype0 tmp120;
    float3 tmp121;
    int tmp122;
    float tmp123;
    float phi_in124;
    float phi_out125;
    float tmp126;
    float tmp127;
    float tmp129;
    int tmp130;
    structtype0 tmp131;
    float3 tmp132;
    int tmp133;
    float tmp134;
    float phi_in135;
    float phi_out136;
    float tmp137;
    float tmp138;
    float tmp139;
    int tmp140;
    structtype0 tmp141;
    float3 tmp142;
    int tmp143;
    float tmp144;
    float phi_in145;
    float phi_out146;
    float tmp147;
    float tmp148;
    float tmp149;
    bool tmp151;
    structtype0 tmp152;
    float3 tmp153;
    int tmp154;
    float tmp155;
    float phi_in156;
    float phi_out157;
    float tmp158;
    float tmp159;
    float tmp160;
    int tmp161;
    structtype0 tmp162;
    float3 tmp163;
    int tmp164;
    float tmp165;
    float phi_in166;
    float phi_out167;
    float tmp168;
    float tmp169;
    float2 tmp174;
    float3 tmp177;
    bool tmp179;
    structtype0 tmp180;
    float3 tmp181;
    int tmp182;
    float tmp183;
    float phi_in184;
    float phi_out185;
    float tmp186;
    float tmp187;
    float tmp193;
    float tmp194;
    int tmp195;
    int tmp196;
    structtype0 tmp197;
    float3 tmp198;
    int tmp199;
    float tmp200;
    float phi_in201;
    float phi_out202;
    float tmp203;
    float tmp204;
    int tmp205;
    structtype0 tmp206;
    float3 tmp207;
    int tmp208;
    float tmp209;
    float phi_in210;
    float phi_out211;
    float tmp212;
    float tmp213;
    int tmp216;
    structtype0 tmp217;
    float3 tmp218;
    int tmp219;
    float tmp220;
    float phi_in221;
    float phi_out222;
    float tmp223;
    float tmp224;
    int tmp226;
    structtype0 tmp227;
    float3 tmp228;
    int tmp229;
    float tmp230;
    float phi_in231;
    float phi_out232;
    float tmp233;
    float tmp234;
    int tmp235;
    structtype0 tmp236;
    float3 tmp237;
    int tmp238;
    float tmp239;
    float phi_in240;
    float phi_out241;
    float tmp242;
    float tmp243;
    int tmp244;
    structtype0 tmp245;
    float3 tmp246;
    int tmp247;
    float tmp248;
    float phi_in249;
    float phi_out250;
    float tmp251;
    float tmp252;
    int tmp253;
    structtype0 tmp254;
    float3 tmp255;
    int tmp256;
    float tmp257;
    float phi_in258;
    float phi_out259;
    float tmp260;
    float tmp261;
    int tmp262;
    structtype0 tmp263;
    float3 tmp264;
    int tmp265;
    float tmp266;
    float phi_in267;
    float phi_out268;
    float tmp269;
    float tmp270;
    int tmp271;
    structtype0 tmp272;
    float3 tmp273;
    int tmp274;
    float tmp275;
    float phi_in276;
    float phi_out277;
    float tmp278;
    float tmp279;
    float2 tmp283;
    int tmp286;
    int tmp287;
    structtype0 tmp288;
    float3 tmp289;
    int tmp290;
    float tmp291;
    float phi_in292;
    float phi_out293;
    float tmp294;
    float tmp295;
    bool2 tmp298;
    int tmp299;
    structtype0 tmp300;
    float3 tmp301;
    int tmp302;
    float tmp303;
    float phi_in304;
    float phi_out305;
    float tmp306;
    float tmp307;
    float3 tmp309;
    float3 tmp313;
    float3 tmp315;
    bool2 tmp318;
    int tmp319;
    structtype0 tmp320;
    float3 tmp321;
    int tmp322;
    float tmp323;
    float phi_in324;
    float phi_out325;
    float tmp326;
    float tmp327;
    int tmp333;
    structtype0 tmp334;
    float3 tmp335;
    int tmp336;
    float tmp337;
    float phi_in338;
    float phi_out339;
    float tmp340;
    float tmp341;
    int tmp343;
    float3 tmp350;
    int tmp351;
    int tmp352;
    float3 tmp353;
    float3 phi_in354;
    float3 phi_out355;
    float3 tmp359;
    int tmp361;
    float3 tmp374;
    int tmp375;
    int tmp376;
    float3 tmp377;
    float3 phi_in378;
    float3 phi_out379;
    float3 tmp383;
    float3 tmp393;
    float tmp396;
    float tmp397;
    float3 tmp399;
    float3 tmp401;
    float3 phi_in402;
    float3 phi_out403;
    float3 phi_in404;
    float3 phi_out405;
    float3 tmp407;
    int tmp0 = state.arg_block_offset;
    int tmp1 = mdl_read_argblock_as_int(tmp0 + 72);
    int tmp2 = mdl_read_argblock_as_int(tmp0 + 76);
    Derived_float tmp3 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 80), 0.0f, 0.0f);
    Derived_float tmp4 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 84), 0.0f, 0.0f);
    Derived_float tmp5 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 88), 0.0f, 0.0f);
    Derived_float tmp6 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 92), 0.0f, 0.0f);
    bool tmp7 = mdl_read_argblock_as_bool(tmp0 + 96);
    bool tmp8 = mdl_read_argblock_as_bool(tmp0 + 97);
    bool tmp9 = mdl_read_argblock_as_bool(tmp0 + 98);
    int tmp10 = mdl_read_argblock_as_int(tmp0 + 100);
    Derived_float3 tmp11 = constr_Derived_float3(float3(mdl_read_argblock_as_float(tmp0 + 112), mdl_read_argblock_as_float(tmp0 + 116), mdl_read_argblock_as_float(tmp0 + 120)), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    Derived_float3 tmp12 = constr_Derived_float3(float3(mdl_read_argblock_as_float(tmp0 + 128), mdl_read_argblock_as_float(tmp0 + 132), mdl_read_argblock_as_float(tmp0 + 136)), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    Derived_float3 tmp13 = constr_Derived_float3(float3(mdl_read_argblock_as_float(tmp0 + 144), mdl_read_argblock_as_float(tmp0 + 148), mdl_read_argblock_as_float(tmp0 + 152)), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    tmp14 = _ZN11OmniSurface9OmniImage28compute_texture_coordinate_2EU7uniformN4base25texture_coordinate_systemEU7uniformiU7uniformfU7uniformfU7uniformfU7uniformfU7uniformbU7uniformbU7uniformbU7uniformN11OmniSurface9OmniImage15projection_modeEU7uniformu6float3U7uniformu6float3U7uniformu6float3(state, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12, tmp13);
    _ZN4base23texture_coordinate_infoE tmp15 = tmp14.val;
    float3 tmp16 = tmp15.position;
    state.text_results[0].x = tmp16.x;
    state.text_results[0].y = tmp16.y;
    state.text_results[0].z = tmp16.z;
    float3 tmp17 = tmp15.tangent_u;
    state.text_results[1].x = tmp17.x;
    state.text_results[1].y = tmp17.y;
    state.text_results[1].z = tmp17.z;
    float3 tmp18 = tmp15.tangent_v;
    state.text_results[2].x = tmp18.x;
    state.text_results[2].y = tmp18.y;
    state.text_results[2].z = tmp18.z;
    state.text_results[3].x = asfloat(tmp15.source_flags);
    _ZN4base23texture_coordinate_infoE tmp19 = tmp14.dx;
    float3 tmp20 = tmp19.position;
    state.text_results[4].x = tmp20.x;
    state.text_results[4].y = tmp20.y;
    state.text_results[4].z = tmp20.z;
    float3 tmp21 = tmp19.tangent_u;
    state.text_results[5].x = tmp21.x;
    state.text_results[5].y = tmp21.y;
    state.text_results[5].z = tmp21.z;
    float3 tmp22 = tmp19.tangent_v;
    state.text_results[6].x = tmp22.x;
    state.text_results[6].y = tmp22.y;
    state.text_results[6].z = tmp22.z;
    state.text_results[7].x = asfloat(tmp19.source_flags);
    _ZN4base23texture_coordinate_infoE tmp23 = tmp14.dy;
    float3 tmp24 = tmp23.position;
    state.text_results[8].x = tmp24.x;
    state.text_results[8].y = tmp24.y;
    state.text_results[8].z = tmp24.z;
    float3 tmp25 = tmp23.tangent_u;
    state.text_results[9].x = tmp25.x;
    state.text_results[9].y = tmp25.y;
    state.text_results[9].z = tmp25.z;
    float3 tmp26 = tmp23.tangent_v;
    state.text_results[10].x = tmp26.x;
    state.text_results[10].y = tmp26.y;
    state.text_results[10].z = tmp26.z;
    state.text_results[11].x = asfloat(tmp23.source_flags);
    tmp27 = state.arg_block_offset;
    tmp28 = mdl_read_argblock_as_int(tmp27 + 384);
    tmp29 = mdl_read_argblock_as_bool(tmp27 + 16);
    tmp30 = float3(mdl_read_argblock_as_float(tmp27 + 32), mdl_read_argblock_as_float(tmp27 + 36), mdl_read_argblock_as_float(tmp27 + 40));
    tmp31 = mdl_read_argblock_as_bool(tmp27 + 48);
    tmp32 = constr_Derived_float2(float2(mdl_read_argblock_as_float(tmp27 + 56), mdl_read_argblock_as_float(tmp27 + 60)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
    tmp33 = mdl_read_argblock_as_int(tmp27 + 64);
    tmp34 = mdl_read_argblock_as_int(tmp27 + 68);
    tmp35 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp28, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp36 = tmp35.m_0;
    tmp37 = mdl_read_argblock_as_int(tmp27 + 388);
    if (tmp37 == 0) {
        tmp38 = tmp35.m_1;
        phi_in = tmp38;
    } else if (tmp37 == 1)
        phi_in = tmp36.x;
    else if (tmp37 == 2)
        phi_in = tmp36.y;
    else if (tmp37 == 3)
        phi_in = tmp36.z;
    else if (tmp37 == 4)
        phi_in = 1.0f;
    else if (tmp37 == 7) {
        tmp39 = (tmp36.x + tmp36.y + tmp36.z) * 0.3333333433f;
        phi_in = tmp39;
    } else if (tmp37 == 6) {
        tmp40 = tmp36.x * 0.2126709968f + tmp36.y * 0.7151600122f + tmp36.z * 0.07216899842f;
        phi_in = tmp40;
    } else
        phi_in = 0.0f;
    phi_out = phi_in;
    tmp41 = mdl_read_argblock_as_bool(tmp27 + 380) ? tex_texture_isvalid(tmp28) ? phi_out : mdl_read_argblock_as_float(tmp27 + 392) : 0.0f;
    state.text_results[12].x = tmp41;
    tmp42 = mdl_read_argblock_as_int(tmp27 + 396);
    tmp43 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp42, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp44 = tmp43.m_0;
    tmp45 = mdl_read_argblock_as_int(tmp27 + 400);
    if (tmp45 == 0) {
        tmp46 = tmp43.m_1;
        phi_in47 = tmp46;
    } else if (tmp45 == 1)
        phi_in47 = tmp44.x;
    else if (tmp45 == 2)
        phi_in47 = tmp44.y;
    else if (tmp45 == 3)
        phi_in47 = tmp44.z;
    else if (tmp45 == 4)
        phi_in47 = 1.0f;
    else if (tmp45 == 7) {
        tmp49 = (tmp44.x + tmp44.y + tmp44.z) * 0.3333333433f;
        phi_in47 = tmp49;
    } else if (tmp45 == 6) {
        tmp50 = tmp44.x * 0.2126709968f + tmp44.y * 0.7151600122f + tmp44.z * 0.07216899842f;
        phi_in47 = tmp50;
    } else
        phi_in47 = 0.0f;
    phi_out48 = phi_in47;
    tmp51 = tex_texture_isvalid(tmp42) ? phi_out48 : mdl_read_argblock_as_float(tmp27 + 404);
    state.text_results[12].y = tmp51;
    int tmp52 = mdl_read_argblock_as_int(tmp27 + 408);
    tmp53 = tex_texture_isvalid(tmp52) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp52, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 416), mdl_read_argblock_as_float(tmp27 + 420), mdl_read_argblock_as_float(tmp27 + 424));
    state.text_results[12].z = tmp53.x;
    state.text_results[12].w = tmp53.y;
    state.text_results[13].x = tmp53.z;
    tmp54 = mdl_read_argblock_as_int(tmp27 + 516);
    tmp55 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp54, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp56 = tmp55.m_0;
    tmp57 = mdl_read_argblock_as_int(tmp27 + 520);
    if (tmp57 == 0) {
        tmp58 = tmp55.m_1;
        phi_in59 = tmp58;
    } else if (tmp57 == 1)
        phi_in59 = tmp56.x;
    else if (tmp57 == 2)
        phi_in59 = tmp56.y;
    else if (tmp57 == 3)
        phi_in59 = tmp56.z;
    else if (tmp57 == 4)
        phi_in59 = 1.0f;
    else if (tmp57 == 7) {
        tmp61 = (tmp56.x + tmp56.y + tmp56.z) * 0.3333333433f;
        phi_in59 = tmp61;
    } else if (tmp57 == 6) {
        tmp62 = tmp56.x * 0.2126709968f + tmp56.y * 0.7151600122f + tmp56.z * 0.07216899842f;
        phi_in59 = tmp62;
    } else
        phi_in59 = 0.0f;
    phi_out60 = phi_in59;
    tmp63 = mdl_read_argblock_as_bool(tmp27 + 512) ? tex_texture_isvalid(tmp54) ? phi_out60 : mdl_read_argblock_as_float(tmp27 + 524) : 0.0f;
    state.text_results[13].y = tmp63;
    tmp64 = mdl_read_argblock_as_int(tmp27 + 528);
    tmp65 = mdl_read_argblock_as_int(tmp27 + 532);
    tmp66 = mdl_read_argblock_as_int(tmp27 + 560);
    phi_in67 = float3(11.60999966f, 3.880000114f, 1.75f);
    phi_in69 = float3(0.4300000072f, 0.2099999934f, 0.1700000018f);
    if (tmp64 != 0) {
        if (tmp64 == 1) {
            phi_in67 = float3(9.43999958f, 3.349999905f, 1.789999962f);
            phi_in69 = float3(0.4399999976f, 0.2199999988f, 0.1400000006f);
        } else if (tmp64 == 2) {
            phi_in67 = float3(15.02999973f, 4.659999847f, 2.539999962f);
            phi_in69 = float3(0.9900000095f, 0.9399999976f, 0.8299999833f);
        } else if (tmp64 == 3) {
            phi_in67 = float3(4.760000229f, 0.5699999928f, 0.3899999857f);
            phi_in69 = float3(0.2199999988f, 0.009999999776f, 0.001000000047f);
        } else if (tmp64 == 4) {
            phi_in67 = float3(8.510000229f, 5.570000172f, 3.950000048f);
            phi_in69 = float3(0.9300000072f, 0.9100000262f, 0.8799999952f);
        } else if (tmp64 == 5) {
            phi_in67 = float3(14.27000046f, 7.230000019f, 2.039999962f);
            phi_in69 = float3(0.8600000143f, 0.7400000095f, 0.2899999917f);
        } else if (tmp64 == 6) {
            phi_in67 = float3(18.42000008f, 10.43999958f, 3.5f);
            phi_in69 = float3(0.8899999857f, 0.8899999857f, 0.8000000119f);
        } else if (tmp64 == 7) {
            phi_in67 = float3(10.89999962f, 6.579999924f, 2.50999999f);
            phi_in69 = float3(0.9499999881f, 0.9300000072f, 0.8500000238f);
        } else if (tmp64 == 8) {
            phi_in67 = float3(3.670000076f, 1.370000005f, 0.6800000072f);
            phi_in69 = float3(0.5699999928f, 0.3100000024f, 0.1700000018f);
        } else if (tmp64 == 9) {
            phi_in67 = float3(4.820000172f, 1.690000057f, 1.090000033f);
            phi_in69 = float3(0.75f, 0.5699999928f, 0.4699999988f);
        } else if (tmp64 == 10) {
            phi_in67 = float3(1.0f, 0.3000000119f, 0.1000000015f);
            phi_in69 = float3(0.9990000129f, 0.6150000095f, 0.5210000277f);
        } else if (tmp64 == 11) {
            phi_in67 = float3(0.72299999f, 0.2639999986f, 0.1270000041f);
            phi_in69 = float3(0.07800000161f, 0.04300000146f, 0.02500000037f);
        } else {
            tmp71 = tex_texture_isvalid(tmp66) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp66, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 576), mdl_read_argblock_as_float(tmp27 + 580), mdl_read_argblock_as_float(tmp27 + 584));
            tmp72 = tex_texture_isvalid(tmp65) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp65, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 544), mdl_read_argblock_as_float(tmp27 + 548), mdl_read_argblock_as_float(tmp27 + 552));
            phi_in67 = tmp71;
            phi_in69 = tmp72;
        }
    }
    phi_out68 = phi_in67;
    phi_out70 = phi_in69;
    state.text_results[14].x = phi_out70.x;
    state.text_results[14].y = phi_out70.y;
    state.text_results[14].z = phi_out70.z;
    state.text_results[15].x = phi_out68.x;
    state.text_results[15].y = phi_out68.y;
    state.text_results[15].z = phi_out68.z;
    tmp73 = state.meters_per_scene_unit;
    int tmp74 = mdl_read_argblock_as_int(tmp27 + 728);
    tmp75 = tex_texture_isvalid(tmp74);
    tmp76 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp74, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp77 = tmp76.m_0;
    tmp78 = mdl_read_argblock_as_int(tmp27 + 732);
    if (tmp78 == 0) {
        tmp79 = tmp76.m_1;
        phi_in80 = tmp79;
    } else if (tmp78 == 1)
        phi_in80 = tmp77.x;
    else if (tmp78 == 2)
        phi_in80 = tmp77.y;
    else if (tmp78 == 3)
        phi_in80 = tmp77.z;
    else if (tmp78 == 4)
        phi_in80 = 1.0f;
    else if (tmp78 == 7) {
        tmp82 = (tmp77.x + tmp77.y + tmp77.z) * 0.3333333433f;
        phi_in80 = tmp82;
    } else if (tmp78 == 6) {
        tmp83 = tmp77.x * 0.2126709968f + tmp77.y * 0.7151600122f + tmp77.z * 0.07216899842f;
        phi_in80 = tmp83;
    } else
        phi_in80 = 0.0f;
    phi_out81 = phi_in80;
    float3 tmp84 = float3(max(tmp75 ? phi_out81 : mdl_read_argblock_as_float(tmp27 + 736), 9.999999747e-06f) * tmp73, 0.0f, 0.0f);
    tmp85 = tmp84.xxx * phi_out68;
    tmp86 = float3(1.0f, 1.0f, 1.0f) / tmp85;
    float3 tmp87 = (phi_out70 * float3(17.71260071f, 17.71260071f, 17.71260071f) + float3(41.68080139f, 41.68080139f, 41.68080139f)) * phi_out70 + float3(9.592169762f, 9.592169762f, 9.592169762f);
    float3 tmp88 = phi_out70 * float3(4.208630085f, 4.208630085f, 4.208630085f) + float3(4.097119808f, 4.097119808f, 4.097119808f) - float3(sqrt(tmp87.x), sqrt(tmp87.y), sqrt(tmp87.z));
    tmp89 = tmp88 * tmp88;
    tmp90 = float3(1.0f, 1.0f, 1.0f) - tmp89;
    int tmp91 = mdl_read_argblock_as_int(tmp27 + 704);
    tmp92 = tex_texture_isvalid(tmp91);
    tmp93 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp91, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp94 = tmp93.m_0;
    tmp95 = mdl_read_argblock_as_int(tmp27 + 708);
    if (tmp95 == 0) {
        tmp96 = tmp93.m_1;
        phi_in97 = tmp96;
    } else if (tmp95 == 1)
        phi_in97 = tmp94.x;
    else if (tmp95 == 2)
        phi_in97 = tmp94.y;
    else if (tmp95 == 3)
        phi_in97 = tmp94.z;
    else if (tmp95 == 4)
        phi_in97 = 1.0f;
    else if (tmp95 == 7) {
        tmp99 = (tmp94.x + tmp94.y + tmp94.z) * 0.3333333433f;
        phi_in97 = tmp99;
    } else if (tmp95 == 6) {
        tmp100 = tmp94.x * 0.2126709968f + tmp94.y * 0.7151600122f + tmp94.z * 0.07216899842f;
        phi_in97 = tmp100;
    } else
        phi_in97 = 0.0f;
    phi_out98 = phi_in97;
    float3 tmp101 = float3(tmp92 ? phi_out98 : mdl_read_argblock_as_float(tmp27 + 712), 0.0f, 0.0f);
    float3 tmp102 = float3(tmp63, 0.0f, 0.0f);
    tmp103 = (tmp86 - tmp90 / ((float3(1.0f, 1.0f, 1.0f) - tmp101.xxx * tmp89) * tmp85)) * tmp102.xxx;
    int tmp104 = mdl_read_argblock_as_int(tmp27 + 740);
    tmp105 = tex_texture_isvalid(tmp104) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp104, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 752), mdl_read_argblock_as_float(tmp27 + 756), mdl_read_argblock_as_float(tmp27 + 760));
    tmp106 = tmp73 * tmp51;
    phi_in107 = float3(0.0f, 0.0f, 0.0f);
    if (!(tmp106 == 0.0f)) {
        float3 tmp109 = float3(max(max(tmp105.x, tmp105.y), tmp105.z) + 1.0f, 0.0f, 0.0f);
        float3 tmp110 = tmp53 / tmp109.xxx;
        float3 tmp111 = float3(-tmp106, 0.0f, 0.0f);
        float3 tmp112 = tmp105 * tmp53;
        float3 tmp113 = (tmp112 * float3(17.71260071f, 17.71260071f, 17.71260071f) + float3(41.68080139f, 41.68080139f, 41.68080139f)) * tmp112 + float3(9.592169762f, 9.592169762f, 9.592169762f);
        float3 tmp114 = tmp112 * float3(4.208630085f, 4.208630085f, 4.208630085f) + float3(4.097119808f, 4.097119808f, 4.097119808f) - float3(sqrt(tmp113.x), sqrt(tmp113.y), sqrt(tmp113.z));
        float3 tmp115 = tmp114 * tmp114;
        tmp116 = tmp115 * tmp115 * float3(log(tmp110.x), log(tmp110.y), log(tmp110.z)) / tmp111.xxx;
        phi_in107 = tmp116;
    }
    phi_out108 = phi_in107;
    float3 tmp117 = float3(tmp41, 0.0f, 0.0f);
    float3 tmp118 = tmp41 == 0.0f && tmp63 == 0.0f || mdl_read_argblock_as_bool(tmp27) ? float3(0.0f, 0.0f, 0.0f) : (phi_out108 - tmp103) * tmp117.xxx + tmp103;
    state.text_results[16].x = tmp118.x;
    state.text_results[16].y = tmp118.y;
    state.text_results[16].z = tmp118.z;
    tmp119 = mdl_read_argblock_as_int(tmp27 + 12);
    tmp120 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp119, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp121 = tmp120.m_0;
    tmp122 = mdl_read_argblock_as_int(tmp27 + 160);
    if (tmp122 == 0) {
        tmp123 = tmp120.m_1;
        phi_in124 = tmp123;
    } else if (tmp122 == 1)
        phi_in124 = tmp121.x;
    else if (tmp122 == 2)
        phi_in124 = tmp121.y;
    else if (tmp122 == 3)
        phi_in124 = tmp121.z;
    else if (tmp122 == 4)
        phi_in124 = 1.0f;
    else if (tmp122 == 7) {
        tmp126 = (tmp121.x + tmp121.y + tmp121.z) * 0.3333333433f;
        phi_in124 = tmp126;
    } else if (tmp122 == 6) {
        tmp127 = tmp121.x * 0.2126709968f + tmp121.y * 0.7151600122f + tmp121.z * 0.07216899842f;
        phi_in124 = tmp127;
    } else
        phi_in124 = 0.0f;
    phi_out125 = phi_in124;
    float tmp128 = tex_texture_isvalid(tmp119) ? phi_out125 : mdl_read_argblock_as_float(tmp27 + 164);
    tmp129 = tmp128 * tmp128;
    state.text_results[16].w = tmp129;
    tmp130 = mdl_read_argblock_as_int(tmp27 + 168);
    tmp131 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp130, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp132 = tmp131.m_0;
    tmp133 = mdl_read_argblock_as_int(tmp27 + 172);
    if (tmp133 == 0) {
        tmp134 = tmp131.m_1;
        phi_in135 = tmp134;
    } else if (tmp133 == 1)
        phi_in135 = tmp132.x;
    else if (tmp133 == 2)
        phi_in135 = tmp132.y;
    else if (tmp133 == 3)
        phi_in135 = tmp132.z;
    else if (tmp133 == 4)
        phi_in135 = 1.0f;
    else if (tmp133 == 7) {
        tmp137 = (tmp132.x + tmp132.y + tmp132.z) * 0.3333333433f;
        phi_in135 = tmp137;
    } else if (tmp133 == 6) {
        tmp138 = tmp132.x * 0.2126709968f + tmp132.y * 0.7151600122f + tmp132.z * 0.07216899842f;
        phi_in135 = tmp138;
    } else
        phi_in135 = 0.0f;
    phi_out136 = phi_in135;
    tmp139 = tex_texture_isvalid(tmp130) ? phi_out136 : mdl_read_argblock_as_float(tmp27 + 176);
    state.text_results[17].x = tmp139;
    tmp140 = mdl_read_argblock_as_int(tmp27 + 344);
    tmp141 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp140, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp142 = tmp141.m_0;
    tmp143 = mdl_read_argblock_as_int(tmp27 + 348);
    if (tmp143 == 0) {
        tmp144 = tmp141.m_1;
        phi_in145 = tmp144;
    } else if (tmp143 == 1)
        phi_in145 = tmp142.x;
    else if (tmp143 == 2)
        phi_in145 = tmp142.y;
    else if (tmp143 == 3)
        phi_in145 = tmp142.z;
    else if (tmp143 == 4)
        phi_in145 = 1.0f;
    else if (tmp143 == 7) {
        tmp147 = (tmp142.x + tmp142.y + tmp142.z) * 0.3333333433f;
        phi_in145 = tmp147;
    } else if (tmp143 == 6) {
        tmp148 = tmp142.x * 0.2126709968f + tmp142.y * 0.7151600122f + tmp142.z * 0.07216899842f;
        phi_in145 = tmp148;
    } else
        phi_in145 = 0.0f;
    phi_out146 = phi_in145;
    tmp149 = tex_texture_isvalid(tmp140) ? phi_out146 : mdl_read_argblock_as_float(tmp27 + 352);
    state.text_results[17].y = tmp149;
    int tmp150 = mdl_read_argblock_as_int(tmp27 + 332);
    tmp151 = tex_texture_isvalid(tmp150);
    tmp152 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp150, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp153 = tmp152.m_0;
    tmp154 = mdl_read_argblock_as_int(tmp27 + 336);
    if (tmp154 == 0) {
        tmp155 = tmp152.m_1;
        phi_in156 = tmp155;
    } else if (tmp154 == 1)
        phi_in156 = tmp153.x;
    else if (tmp154 == 2)
        phi_in156 = tmp153.y;
    else if (tmp154 == 3)
        phi_in156 = tmp153.z;
    else if (tmp154 == 4)
        phi_in156 = 1.0f;
    else if (tmp154 == 7) {
        tmp158 = (tmp153.x + tmp153.y + tmp153.z) * 0.3333333433f;
        phi_in156 = tmp158;
    } else if (tmp154 == 6) {
        tmp159 = tmp153.x * 0.2126709968f + tmp153.y * 0.7151600122f + tmp153.z * 0.07216899842f;
        phi_in156 = tmp159;
    } else
        phi_in156 = 0.0f;
    phi_out157 = phi_in156;
    tmp160 = tmp151 ? phi_out157 : mdl_read_argblock_as_float(tmp27 + 340);
    tmp161 = mdl_read_argblock_as_int(tmp27 + 356);
    tmp162 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp161, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp163 = tmp162.m_0;
    tmp164 = mdl_read_argblock_as_int(tmp27 + 360);
    if (tmp164 == 0) {
        tmp165 = tmp162.m_1;
        phi_in166 = tmp165;
    } else if (tmp164 == 1)
        phi_in166 = tmp163.x;
    else if (tmp164 == 2)
        phi_in166 = tmp163.y;
    else if (tmp164 == 3)
        phi_in166 = tmp163.z;
    else if (tmp164 == 4)
        phi_in166 = 1.0f;
    else if (tmp164 == 7) {
        tmp168 = (tmp163.x + tmp163.y + tmp163.z) * 0.3333333433f;
        phi_in166 = tmp168;
    } else if (tmp164 == 6) {
        tmp169 = tmp163.x * 0.2126709968f + tmp163.y * 0.7151600122f + tmp163.z * 0.07216899842f;
        phi_in166 = tmp169;
    } else
        phi_in166 = 0.0f;
    phi_out167 = phi_in166;
    float tmp170 = tmp139 * tmp129 * (tex_texture_isvalid(tmp161) ? phi_out167 : mdl_read_argblock_as_float(tmp27 + 364));
    float tmp171 = tmp149 * tmp149 * (1.0f - tmp170) + tmp170;
    float tmp172 = sqrt(1.0f - min(max(tmp160, 0.0f), 0.9800000191f));
    float2 tmp173 = float2(tmp171, 0.0f);
    tmp174 = tmp160 != 0.0f ? float2(min(tmp171 / tmp172, 1.0f), tmp171 * tmp172) : tmp173.xx;
    state.text_results[17].z = tmp174.x;
    state.text_results[17].w = tmp174.y;
    int tmp175 = mdl_read_argblock_as_int(tmp27 + 204);
    float3 tmp176 = float3(tmp139, 0.0f, 0.0f);
    tmp177 = ((tex_texture_isvalid(tmp175) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp175, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 208), mdl_read_argblock_as_float(tmp27 + 212), mdl_read_argblock_as_float(tmp27 + 216))) + float3(-1.0f, -1.0f, -1.0f)) * tmp176.xxx + float3(1.0f, 1.0f, 1.0f);
    state.text_results[18].x = tmp177.x;
    state.text_results[18].y = tmp177.y;
    state.text_results[18].z = tmp177.z;
    int tmp178 = mdl_read_argblock_as_int(tmp27 + 628);
    tmp179 = tex_texture_isvalid(tmp178);
    tmp180 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp178, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp181 = tmp180.m_0;
    tmp182 = mdl_read_argblock_as_int(tmp27 + 632);
    if (tmp182 == 0) {
        tmp183 = tmp180.m_1;
        phi_in184 = tmp183;
    } else if (tmp182 == 1)
        phi_in184 = tmp181.x;
    else if (tmp182 == 2)
        phi_in184 = tmp181.y;
    else if (tmp182 == 3)
        phi_in184 = tmp181.z;
    else if (tmp182 == 4)
        phi_in184 = 1.0f;
    else if (tmp182 == 7) {
        tmp186 = (tmp181.x + tmp181.y + tmp181.z) * 0.3333333433f;
        phi_in184 = tmp186;
    } else if (tmp182 == 6) {
        tmp187 = tmp181.x * 0.2126709968f + tmp181.y * 0.7151600122f + tmp181.z * 0.07216899842f;
        phi_in184 = tmp187;
    } else
        phi_in184 = 0.0f;
    phi_out185 = phi_in184;
    float tmp188 = tmp179 ? phi_out185 : mdl_read_argblock_as_float(tmp27 + 636);
    float tmp189 = tmp188 < 500.0f ? 500.0f : tmp188;
    float tmp190 = tmp189 * tmp189;
    float tmp191 = tmp190 * tmp189;
    float tmp192 = tmp190 * tmp190;
    tmp193 = (tmp189 * -1.446884639e+11f + 1.0f + tmp190 * 365139168.0f + tmp191 * -52449.67969f + tmp192 * 70.34281158f) / (tmp189 * -3.899100774e+10f + 1.0f + tmp190 * 77312264.0f + tmp191 * 19901.70508f + tmp192 * 67.80271912f);
    tmp194 = (tmp189 * -1.390761247e+11f + 1.0f + tmp190 * 608958592.0f + tmp191 * -868632.9375f + tmp192 * 408.2635498f) / (tmp189 * -9.494124298e+11f + -8568.175781f + tmp190 * 2402142464.0f + tmp191 * 147983.6094f + tmp192 * 181.1112061f);
    tmp195 = mdl_read_argblock_as_int(tmp27 + 640);
    tmp196 = mdl_read_argblock_as_int(tmp27 + 676);
    tmp197 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp196, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp198 = tmp197.m_0;
    tmp199 = mdl_read_argblock_as_int(tmp27 + 680);
    if (tmp199 == 0) {
        tmp200 = tmp197.m_1;
        phi_in201 = tmp200;
    } else if (tmp199 == 1)
        phi_in201 = tmp198.x;
    else if (tmp199 == 2)
        phi_in201 = tmp198.y;
    else if (tmp199 == 3)
        phi_in201 = tmp198.z;
    else if (tmp199 == 4)
        phi_in201 = 1.0f;
    else if (tmp199 == 7) {
        tmp203 = (tmp198.x + tmp198.y + tmp198.z) * 0.3333333433f;
        phi_in201 = tmp203;
    } else if (tmp199 == 6) {
        tmp204 = tmp198.x * 0.2126709968f + tmp198.y * 0.7151600122f + tmp198.z * 0.07216899842f;
        phi_in201 = tmp204;
    } else
        phi_in201 = 0.0f;
    phi_out202 = phi_in201;
    tmp205 = mdl_read_argblock_as_int(tmp27 + 688);
    tmp206 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp205, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp207 = tmp206.m_0;
    tmp208 = mdl_read_argblock_as_int(tmp27 + 692);
    if (tmp208 == 0) {
        tmp209 = tmp206.m_1;
        phi_in210 = tmp209;
    } else if (tmp208 == 1)
        phi_in210 = tmp207.x;
    else if (tmp208 == 2)
        phi_in210 = tmp207.y;
    else if (tmp208 == 3)
        phi_in210 = tmp207.z;
    else if (tmp208 == 4)
        phi_in210 = 1.0f;
    else if (tmp208 == 7) {
        tmp212 = (tmp207.x + tmp207.y + tmp207.z) * 0.3333333433f;
        phi_in210 = tmp212;
    } else if (tmp208 == 6) {
        tmp213 = tmp207.x * 0.2126709968f + tmp207.y * 0.7151600122f + tmp207.z * 0.07216899842f;
        phi_in210 = tmp213;
    } else
        phi_in210 = 0.0f;
    phi_out211 = phi_in210;
    float3 tmp214 = float3((mdl_read_argblock_as_int(tmp27 + 672) == 0 ? 1.0f : 3.141592741f) * (tex_texture_isvalid(tmp196) ? phi_out202 : mdl_read_argblock_as_float(tmp27 + 684)) * (tex_texture_isvalid(tmp205) ? phi_out211 : mdl_read_argblock_as_float(tmp27 + 696)), 0.0f, 0.0f);
    float3 tmp215 = (mdl_read_argblock_as_bool(tmp27 + 627) ? float3(max(tmp193 * 3.240600109f + -1.537199974f + tmp194 * -0.4986000061f, 0.0f), max(tmp193 * -0.9689000249f + 1.875800014f + tmp194 * 0.04149999842f, 0.0f), max(tmp193 * 0.05570000038f + -0.2039999962f + tmp194 * 1.057000041f, 0.0f)) : tex_texture_isvalid(tmp195) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp195, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 656), mdl_read_argblock_as_float(tmp27 + 660), mdl_read_argblock_as_float(tmp27 + 664))) * tmp177 * tmp214.xxx;
    state.text_results[18].w = tmp215.x;
    state.text_results[19].x = tmp215.y;
    state.text_results[19].y = tmp215.z;
    tmp216 = mdl_read_argblock_as_int(tmp27 + 468);
    tmp217 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp216, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp218 = tmp217.m_0;
    tmp219 = mdl_read_argblock_as_int(tmp27 + 472);
    if (tmp219 == 0) {
        tmp220 = tmp217.m_1;
        phi_in221 = tmp220;
    } else if (tmp219 == 1)
        phi_in221 = tmp218.x;
    else if (tmp219 == 2)
        phi_in221 = tmp218.y;
    else if (tmp219 == 3)
        phi_in221 = tmp218.z;
    else if (tmp219 == 4)
        phi_in221 = 1.0f;
    else if (tmp219 == 7) {
        tmp223 = (tmp218.x + tmp218.y + tmp218.z) * 0.3333333433f;
        phi_in221 = tmp223;
    } else if (tmp219 == 6) {
        tmp224 = tmp218.x * 0.2126709968f + tmp218.y * 0.7151600122f + tmp218.z * 0.07216899842f;
        phi_in221 = tmp224;
    } else
        phi_in221 = 0.0f;
    phi_out222 = phi_in221;
    float tmp225 = tex_texture_isvalid(tmp216) ? phi_out222 : mdl_read_argblock_as_float(tmp27 + 476);
    state.text_results[19].z = tmp225 * tmp225;
    tmp226 = mdl_read_argblock_as_int(tmp27 + 240);
    tmp227 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp226, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp228 = tmp227.m_0;
    tmp229 = mdl_read_argblock_as_int(tmp27 + 244);
    if (tmp229 == 0) {
        tmp230 = tmp227.m_1;
        phi_in231 = tmp230;
    } else if (tmp229 == 1)
        phi_in231 = tmp228.x;
    else if (tmp229 == 2)
        phi_in231 = tmp228.y;
    else if (tmp229 == 3)
        phi_in231 = tmp228.z;
    else if (tmp229 == 4)
        phi_in231 = 1.0f;
    else if (tmp229 == 7) {
        tmp233 = (tmp228.x + tmp228.y + tmp228.z) * 0.3333333433f;
        phi_in231 = tmp233;
    } else if (tmp229 == 6) {
        tmp234 = tmp228.x * 0.2126709968f + tmp228.y * 0.7151600122f + tmp228.z * 0.07216899842f;
        phi_in231 = tmp234;
    } else
        phi_in231 = 0.0f;
    phi_out232 = phi_in231;
    state.text_results[19].w = tex_texture_isvalid(tmp226) ? phi_out232 : mdl_read_argblock_as_float(tmp27 + 248);
    tmp235 = mdl_read_argblock_as_int(tmp27 + 320);
    tmp236 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp235, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp237 = tmp236.m_0;
    tmp238 = mdl_read_argblock_as_int(tmp27 + 324);
    if (tmp238 == 0) {
        tmp239 = tmp236.m_1;
        phi_in240 = tmp239;
    } else if (tmp238 == 1)
        phi_in240 = tmp237.x;
    else if (tmp238 == 2)
        phi_in240 = tmp237.y;
    else if (tmp238 == 3)
        phi_in240 = tmp237.z;
    else if (tmp238 == 4)
        phi_in240 = 1.0f;
    else if (tmp238 == 7) {
        tmp242 = (tmp237.x + tmp237.y + tmp237.z) * 0.3333333433f;
        phi_in240 = tmp242;
    } else if (tmp238 == 6) {
        tmp243 = tmp237.x * 0.2126709968f + tmp237.y * 0.7151600122f + tmp237.z * 0.07216899842f;
        phi_in240 = tmp243;
    } else
        phi_in240 = 0.0f;
    phi_out241 = phi_in240;
    state.text_results[20].x = tex_texture_isvalid(tmp235) ? phi_out241 : mdl_read_argblock_as_float(tmp27 + 328);
    tmp244 = mdl_read_argblock_as_int(tmp27 + 224);
    tmp245 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp244, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp246 = tmp245.m_0;
    tmp247 = mdl_read_argblock_as_int(tmp27 + 228);
    if (tmp247 == 0) {
        tmp248 = tmp245.m_1;
        phi_in249 = tmp248;
    } else if (tmp247 == 1)
        phi_in249 = tmp246.x;
    else if (tmp247 == 2)
        phi_in249 = tmp246.y;
    else if (tmp247 == 3)
        phi_in249 = tmp246.z;
    else if (tmp247 == 4)
        phi_in249 = 1.0f;
    else if (tmp247 == 7) {
        tmp251 = (tmp246.x + tmp246.y + tmp246.z) * 0.3333333433f;
        phi_in249 = tmp251;
    } else if (tmp247 == 6) {
        tmp252 = tmp246.x * 0.2126709968f + tmp246.y * 0.7151600122f + tmp246.z * 0.07216899842f;
        phi_in249 = tmp252;
    } else
        phi_in249 = 0.0f;
    phi_out250 = phi_in249;
    state.text_results[20].y = tex_texture_isvalid(tmp244) ? phi_out250 : mdl_read_argblock_as_float(tmp27 + 232);
    tmp253 = mdl_read_argblock_as_int(tmp27 + 456);
    tmp254 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp253, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp255 = tmp254.m_0;
    tmp256 = mdl_read_argblock_as_int(tmp27 + 460);
    if (tmp256 == 0) {
        tmp257 = tmp254.m_1;
        phi_in258 = tmp257;
    } else if (tmp256 == 1)
        phi_in258 = tmp255.x;
    else if (tmp256 == 2)
        phi_in258 = tmp255.y;
    else if (tmp256 == 3)
        phi_in258 = tmp255.z;
    else if (tmp256 == 4)
        phi_in258 = 1.0f;
    else if (tmp256 == 7) {
        tmp260 = (tmp255.x + tmp255.y + tmp255.z) * 0.3333333433f;
        phi_in258 = tmp260;
    } else if (tmp256 == 6) {
        tmp261 = tmp255.x * 0.2126709968f + tmp255.y * 0.7151600122f + tmp255.z * 0.07216899842f;
        phi_in258 = tmp261;
    } else
        phi_in258 = 0.0f;
    phi_out259 = phi_in258;
    state.text_results[20].z = tex_texture_isvalid(tmp253) ? phi_out259 : mdl_read_argblock_as_float(tmp27 + 464);
    tmp262 = mdl_read_argblock_as_int(tmp27 + 604);
    tmp263 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp262, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp264 = tmp263.m_0;
    tmp265 = mdl_read_argblock_as_int(tmp27 + 608);
    if (tmp265 == 0) {
        tmp266 = tmp263.m_1;
        phi_in267 = tmp266;
    } else if (tmp265 == 1)
        phi_in267 = tmp264.x;
    else if (tmp265 == 2)
        phi_in267 = tmp264.y;
    else if (tmp265 == 3)
        phi_in267 = tmp264.z;
    else if (tmp265 == 4)
        phi_in267 = 1.0f;
    else if (tmp265 == 7) {
        tmp269 = (tmp264.x + tmp264.y + tmp264.z) * 0.3333333433f;
        phi_in267 = tmp269;
    } else if (tmp265 == 6) {
        tmp270 = tmp264.x * 0.2126709968f + tmp264.y * 0.7151600122f + tmp264.z * 0.07216899842f;
        phi_in267 = tmp270;
    } else
        phi_in267 = 0.0f;
    phi_out268 = phi_in267;
    state.text_results[20].w = tex_texture_isvalid(tmp262) ? phi_out268 : mdl_read_argblock_as_float(tmp27 + 612);
    tmp271 = mdl_read_argblock_as_int(tmp27 + 180);
    tmp272 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp271, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp273 = tmp272.m_0;
    tmp274 = mdl_read_argblock_as_int(tmp27 + 184);
    if (tmp274 == 0) {
        tmp275 = tmp272.m_1;
        phi_in276 = tmp275;
    } else if (tmp274 == 1)
        phi_in276 = tmp273.x;
    else if (tmp274 == 2)
        phi_in276 = tmp273.y;
    else if (tmp274 == 3)
        phi_in276 = tmp273.z;
    else if (tmp274 == 4)
        phi_in276 = 1.0f;
    else if (tmp274 == 7) {
        tmp278 = (tmp273.x + tmp273.y + tmp273.z) * 0.3333333433f;
        phi_in276 = tmp278;
    } else if (tmp274 == 6) {
        tmp279 = tmp273.x * 0.2126709968f + tmp273.y * 0.7151600122f + tmp273.z * 0.07216899842f;
        phi_in276 = tmp279;
    } else
        phi_in276 = 0.0f;
    phi_out277 = phi_in276;
    float tmp280 = tex_texture_isvalid(tmp271) ? phi_out277 : mdl_read_argblock_as_float(tmp27 + 188);
    float tmp281 = sqrt(1.0f - min(max(tmp280, 0.0f), 0.9800000191f));
    float2 tmp282 = float2(tmp129, 0.0f);
    tmp283 = tmp280 != 0.0f ? float2(min(tmp129 / tmp281, 1.0f), tmp281 * tmp129) : tmp282.xx;
    state.text_results[21].x = tmp283.x;
    state.text_results[21].y = tmp283.y;
    float tmp284 = _ZN11OmniSurface10OmniShared19ior_preset_to_valueEN11OmniSurface10OmniShared11ior_presetsEU7uniformf(mdl_read_argblock_as_int(tmp27 + 4), mdl_read_argblock_as_float(tmp27 + 8));
    float tmp285 = (tmp284 + -1.0f) / (tmp284 + 1.0f);
    state.text_results[21].z = tmp285 * tmp285;
    tmp286 = mdl_read_argblock_as_int(tmp27 + 260);
    tmp287 = mdl_read_argblock_as_int(tmp27 + 288);
    tmp288 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp287, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp289 = tmp288.m_0;
    tmp290 = mdl_read_argblock_as_int(tmp27 + 292);
    if (tmp290 == 0) {
        tmp291 = tmp288.m_1;
        phi_in292 = tmp291;
    } else if (tmp290 == 1)
        phi_in292 = tmp289.x;
    else if (tmp290 == 2)
        phi_in292 = tmp289.y;
    else if (tmp290 == 3)
        phi_in292 = tmp289.z;
    else if (tmp290 == 4)
        phi_in292 = 1.0f;
    else if (tmp290 == 7) {
        tmp294 = (tmp289.x + tmp289.y + tmp289.z) * 0.3333333433f;
        phi_in292 = tmp294;
    } else if (tmp290 == 6) {
        tmp295 = tmp289.x * 0.2126709968f + tmp289.y * 0.7151600122f + tmp289.z * 0.07216899842f;
        phi_in292 = tmp295;
    } else
        phi_in292 = 0.0f;
    phi_out293 = phi_in292;
    float3 tmp296 = float3(tex_texture_isvalid(tmp287) ? phi_out293 : mdl_read_argblock_as_float(tmp27 + 296), 0.0f, 0.0f);
    float3 tmp297 = tmp296.xxx * (tex_texture_isvalid(tmp286) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp286, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14).m_0 : float3(mdl_read_argblock_as_float(tmp27 + 272), mdl_read_argblock_as_float(tmp27 + 276), mdl_read_argblock_as_float(tmp27 + 280)));
    state.text_results[21].w = tmp297.x;
    state.text_results[22].x = tmp297.y;
    state.text_results[22].y = tmp297.z;
    tmp298 = tmp283 != float2(0.0f, 0.0f);
    tmp299 = mdl_read_argblock_as_int(tmp27 + 192);
    tmp300 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp299, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp301 = tmp300.m_0;
    tmp302 = mdl_read_argblock_as_int(tmp27 + 196);
    if (tmp302 == 0) {
        tmp303 = tmp300.m_1;
        phi_in304 = tmp303;
    } else if (tmp302 == 1)
        phi_in304 = tmp301.x;
    else if (tmp302 == 2)
        phi_in304 = tmp301.y;
    else if (tmp302 == 3)
        phi_in304 = tmp301.z;
    else if (tmp302 == 4)
        phi_in304 = 1.0f;
    else if (tmp302 == 7) {
        tmp306 = (tmp301.x + tmp301.y + tmp301.z) * 0.3333333433f;
        phi_in304 = tmp306;
    } else if (tmp302 == 6) {
        tmp307 = tmp301.x * 0.2126709968f + tmp301.y * 0.7151600122f + tmp301.z * 0.07216899842f;
        phi_in304 = tmp307;
    } else
        phi_in304 = 0.0f;
    phi_out305 = phi_in304;
    float tmp308 = tex_texture_isvalid(tmp299) ? phi_out305 : mdl_read_argblock_as_float(tmp27 + 200);
    tmp309 = state.tangent_u[0];
    float tmp310 = tmp308 * 6.283185482f;
    float3 tmp311 = float3(cos(tmp310), 0.0f, 0.0f);
    float3 tmp312 = state.normal;
    tmp313 = tmp312.yzx * tmp309.zxy - tmp312.zxy * tmp309.yzx;
    float3 tmp314 = float3(sqrt(tmp313.x * tmp313.x + tmp313.y * tmp313.y + tmp313.z * tmp313.z), 0.0f, 0.0f);
    tmp315 = tmp314.xxx;
    float3 tmp316 = float3(sin(tmp310), 0.0f, 0.0f);
    float3 tmp317 = (tmp298.x || tmp298.y) && tmp308 != 0.0f ? tmp311.xxx * tmp309 - tmp313 * tmp316.xxx / tmp315 : tmp309;
    state.text_results[22].z = tmp317.x;
    state.text_results[22].w = tmp317.y;
    state.text_results[23].x = tmp317.z;
    tmp318 = tmp174 != float2(0.0f, 0.0f);
    tmp319 = mdl_read_argblock_as_int(tmp27 + 368);
    tmp320 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp319, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp321 = tmp320.m_0;
    tmp322 = mdl_read_argblock_as_int(tmp27 + 372);
    if (tmp322 == 0) {
        tmp323 = tmp320.m_1;
        phi_in324 = tmp323;
    } else if (tmp322 == 1)
        phi_in324 = tmp321.x;
    else if (tmp322 == 2)
        phi_in324 = tmp321.y;
    else if (tmp322 == 3)
        phi_in324 = tmp321.z;
    else if (tmp322 == 4)
        phi_in324 = 1.0f;
    else if (tmp322 == 7) {
        tmp326 = (tmp321.x + tmp321.y + tmp321.z) * 0.3333333433f;
        phi_in324 = tmp326;
    } else if (tmp322 == 6) {
        tmp327 = tmp321.x * 0.2126709968f + tmp321.y * 0.7151600122f + tmp321.z * 0.07216899842f;
        phi_in324 = tmp327;
    } else
        phi_in324 = 0.0f;
    phi_out325 = phi_in324;
    float tmp328 = tex_texture_isvalid(tmp319) ? phi_out325 : mdl_read_argblock_as_float(tmp27 + 376);
    float tmp329 = tmp328 * 6.283185482f;
    float3 tmp330 = float3(cos(tmp329), 0.0f, 0.0f);
    float3 tmp331 = float3(sin(tmp329), 0.0f, 0.0f);
    float3 tmp332 = (tmp318.x || tmp318.y) && tmp328 != 0.0f ? tmp330.xxx * tmp309 - tmp331.xxx * tmp313 / tmp315 : tmp309;
    state.text_results[23].y = tmp332.x;
    state.text_results[23].z = tmp332.y;
    state.text_results[23].w = tmp332.z;
    tmp333 = mdl_read_argblock_as_int(tmp27 + 592);
    tmp334 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp333, tmp29, tmp30, tmp31, tmp32, tmp33, tmp34, tmp14);
    tmp335 = tmp334.m_0;
    tmp336 = mdl_read_argblock_as_int(tmp27 + 596);
    if (tmp336 == 0) {
        tmp337 = tmp334.m_1;
        phi_in338 = tmp337;
    } else if (tmp336 == 1)
        phi_in338 = tmp335.x;
    else if (tmp336 == 2)
        phi_in338 = tmp335.y;
    else if (tmp336 == 3)
        phi_in338 = tmp335.z;
    else if (tmp336 == 4)
        phi_in338 = 1.0f;
    else if (tmp336 == 7) {
        tmp340 = (tmp335.x + tmp335.y + tmp335.z) * 0.3333333433f;
        phi_in338 = tmp340;
    } else if (tmp336 == 6) {
        tmp341 = tmp335.x * 0.2126709968f + tmp335.y * 0.7151600122f + tmp335.z * 0.07216899842f;
        phi_in338 = tmp341;
    } else
        phi_in338 = 0.0f;
    phi_out339 = phi_in338;
    float tmp342 = (tex_texture_isvalid(tmp333) ? phi_out339 : mdl_read_argblock_as_float(tmp27 + 600)) * min(max(tmp139, 0.0f), 1.0f) + 1.0f;
    state.text_results[24].x = pow(tmp297.x, tmp342);
    state.text_results[24].y = pow(tmp297.y, tmp342);
    state.text_results[24].z = pow(tmp297.z, tmp342);
    state.text_results[24].w = _ZN11OmniSurface10OmniShared19ior_preset_to_valueEN11OmniSurface10OmniShared11ior_presetsEU7uniformf(mdl_read_argblock_as_int(tmp27 + 448), mdl_read_argblock_as_float(tmp27 + 452));
    tmp343 = mdl_read_argblock_as_int(tmp27 + 432);
    bool tmp344 = mdl_read_argblock_as_bool(tmp27 + 96);
    bool tmp345 = mdl_read_argblock_as_bool(tmp27 + 97);
    float tmp346 = mdl_read_argblock_as_float(tmp27 + 436);
    bool tmp347 = mdl_read_argblock_as_bool(tmp27 + 440);
    bool tmp348 = mdl_read_argblock_as_bool(tmp27 + 441);
    bool tmp349 = mdl_read_argblock_as_bool(tmp27 + 442);
    tmp350 = _ZN11OmniSurface9OmniImage29tangent_space_normal_lookup_2EU7uniformu10texture_2dU7uniformbU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformbU7uniformbU7uniformfU7uniformbU7uniformbU7uniformbN4base23texture_coordinate_infoE(state, tmp343, tmp29, tmp31, tmp32, tmp33, tmp34, tmp344, tmp345, tmp346, tmp347, tmp348, tmp349, tmp14);
    tmp351 = state.arg_block_offset;
    tmp352 = mdl_read_argblock_as_int(tmp351 + 444);
    tmp353 = state.normal;
    phi_in354 = tmp353;
    if (sqrt(tmp350.x * tmp350.x + tmp350.y * tmp350.y + tmp350.z * tmp350.z) > 0.0f) {
        int tmp356 = tmp352 / 4;
        int tmp357 = tmp352 & 3;
        int tmp358 = 3 - tmp356 - tmp357;
        tmp359 = float3(tmp350[uint(tmp356) < 3u ? tmp356 : 0], tmp350[tmp357 == 3 ? 0 : tmp357], tmp350[uint(tmp358) < 3u ? tmp358 : 0]);
        phi_in354 = tmp359;
    }
    phi_out355 = phi_in354;
    float3 tmp360 = tex_texture_isvalid(tmp343) ? phi_out355 : tmp353;
    state.text_results[25].x = tmp360.x;
    state.text_results[25].y = tmp360.y;
    state.text_results[25].z = tmp360.z;
    tmp361 = mdl_read_argblock_as_int(tmp351 + 616);
    bool tmp362 = mdl_read_argblock_as_bool(tmp351 + 16);
    bool tmp363 = mdl_read_argblock_as_bool(tmp351 + 48);
    Derived_float2 tmp364 = constr_Derived_float2(float2(mdl_read_argblock_as_float(tmp351 + 56), mdl_read_argblock_as_float(tmp351 + 60)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
    int tmp365 = mdl_read_argblock_as_int(tmp351 + 64);
    int tmp366 = mdl_read_argblock_as_int(tmp351 + 68);
    bool tmp367 = mdl_read_argblock_as_bool(tmp351 + 96);
    bool tmp368 = mdl_read_argblock_as_bool(tmp351 + 97);
    float tmp369 = mdl_read_argblock_as_float(tmp351 + 620);
    bool tmp370 = mdl_read_argblock_as_bool(tmp351 + 624);
    bool tmp371 = mdl_read_argblock_as_bool(tmp351 + 625);
    bool tmp372 = mdl_read_argblock_as_bool(tmp351 + 626);
    deriv_type0 tmp373 = constr_deriv_type0(constr__ZN4base23texture_coordinate_infoE(state.text_results[0].xyz, state.text_results[1].xyz, state.text_results[2].xyz, asint(state.text_results[3].x)), constr__ZN4base23texture_coordinate_infoE(state.text_results[4].xyz, state.text_results[5].xyz, state.text_results[6].xyz, asint(state.text_results[7].x)), constr__ZN4base23texture_coordinate_infoE(state.text_results[8].xyz, state.text_results[9].xyz, state.text_results[10].xyz, asint(state.text_results[11].x)));
    tmp374 = _ZN11OmniSurface9OmniImage29tangent_space_normal_lookup_2EU7uniformu10texture_2dU7uniformbU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformbU7uniformbU7uniformfU7uniformbU7uniformbU7uniformbN4base23texture_coordinate_infoE(state, tmp361, tmp362, tmp363, tmp364, tmp365, tmp366, tmp367, tmp368, tmp369, tmp370, tmp371, tmp372, tmp373);
    tmp375 = state.arg_block_offset;
    tmp376 = mdl_read_argblock_as_int(tmp375 + 444);
    tmp377 = state.normal;
    phi_in378 = tmp377;
    if (sqrt(tmp374.x * tmp374.x + tmp374.y * tmp374.y + tmp374.z * tmp374.z) > 0.0f) {
        int tmp380 = tmp376 / 4;
        int tmp381 = tmp376 & 3;
        int tmp382 = 3 - tmp380 - tmp381;
        tmp383 = float3(tmp374[uint(tmp380) < 3u ? tmp380 : 0], tmp374[tmp381 == 3 ? 0 : tmp381], tmp374[uint(tmp382) < 3u ? tmp382 : 0]);
        phi_in378 = tmp383;
    }
    phi_out379 = phi_in378;
    float3 tmp384 = tex_texture_isvalid(tmp361) ? phi_out379 : tmp377;
    state.text_results[25].w = tmp384.x;
    state.text_results[26].x = tmp384.y;
    state.text_results[26].y = tmp384.z;
    int tmp385 = mdl_read_argblock_as_int(tmp375 + 300);
    bool tmp386 = mdl_read_argblock_as_bool(tmp375 + 16);
    float3 tmp387 = float3(mdl_read_argblock_as_float(tmp375 + 32), mdl_read_argblock_as_float(tmp375 + 36), mdl_read_argblock_as_float(tmp375 + 40));
    bool tmp388 = mdl_read_argblock_as_bool(tmp375 + 48);
    Derived_float2 tmp389 = constr_Derived_float2(float2(mdl_read_argblock_as_float(tmp375 + 56), mdl_read_argblock_as_float(tmp375 + 60)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
    int tmp390 = mdl_read_argblock_as_int(tmp375 + 64);
    int tmp391 = mdl_read_argblock_as_int(tmp375 + 68);
    deriv_type0 tmp392 = constr_deriv_type0(constr__ZN4base23texture_coordinate_infoE(state.text_results[0].xyz, state.text_results[1].xyz, state.text_results[2].xyz, asint(state.text_results[3].x)), constr__ZN4base23texture_coordinate_infoE(state.text_results[4].xyz, state.text_results[5].xyz, state.text_results[6].xyz, asint(state.text_results[7].x)), constr__ZN4base23texture_coordinate_infoE(state.text_results[8].xyz, state.text_results[9].xyz, state.text_results[10].xyz, asint(state.text_results[11].x)));
    tmp393 = tex_texture_isvalid(tmp385) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp385, tmp386, tmp387, tmp388, tmp389, tmp390, tmp391, tmp392).m_0 : float3(mdl_read_argblock_as_float(tmp375 + 304), mdl_read_argblock_as_float(tmp375 + 308), mdl_read_argblock_as_float(tmp375 + 312));
    state.text_results[26].z = tmp393.x;
    state.text_results[26].w = tmp393.y;
    state.text_results[27].x = tmp393.z;
    int tmp394 = mdl_read_argblock_as_int(tmp375 + 480);
    float3 tmp395 = tex_texture_isvalid(tmp394) ? _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp394, tmp386, tmp387, tmp388, tmp389, tmp390, tmp391, tmp392).m_0 : float3(mdl_read_argblock_as_float(tmp375 + 496), mdl_read_argblock_as_float(tmp375 + 500), mdl_read_argblock_as_float(tmp375 + 504));
    state.text_results[27].y = tmp395.x;
    state.text_results[27].z = tmp395.y;
    state.text_results[27].w = tmp395.z;
    tmp396 = mdl_read_argblock_as_float(tmp375 + 700);
    tmp397 = state.text_results[24].w;
    if (tmp396 > 0.0f) {
        float tmp398 = (tmp397 + -1.0f) / (tmp396 * -170.1999817f);
        tmp399 = float3(tmp398 * 110.7999878f + tmp397, tmp397 - tmp398 * 43.20001221f, tmp397 - tmp398 * 153.4000244f);
        float3 tmp400 = float3(tmp397, 0.0f, 0.0f);
        tmp401 = tmp400.xxx;
        phi_in402 = tmp401;
        phi_in404 = tmp399;
    } else {
        float3 tmp406 = float3(tmp397, 0.0f, 0.0f);
        tmp407 = tmp406.xxx;
        phi_in402 = tmp407;
        phi_in404 = tmp407;
    }
    phi_out403 = phi_in402;
    phi_out405 = phi_in404;
    float3 tmp408 = tmp396 > 0.0f ? phi_out405 : phi_out403;
    state.text_results[28].x = tmp408.x;
    state.text_results[28].y = tmp408.y;
    state.text_results[28].z = tmp408.z;
    float3 tmp409 = float3(state.text_results[21].w, state.text_results[22].x, state.text_results[22].y) * float3(0.9800000191f, 0.9800000191f, 0.9800000191f);
    float3 tmp410 = tmp409 + float3(0.009999999776f, 0.009999999776f, 0.009999999776f);
    float3 tmp411 = float3(sqrt(tmp410.x), sqrt(tmp410.y), sqrt(tmp410.z));
    float3 tmp412 = (tmp411 + float3(1.0f, 1.0f, 1.0f)) / (float3(1.0f, 1.0f, 1.0f) - tmp411);
    float3 tmp413 = float3(0.9900000095f, 0.9900000095f, 0.9900000095f) - tmp409;
    float3 tmp414 = float3(state.text_results[20].x, 0.0f, 0.0f);
    float3 tmp415 = tmp414.xxx * tmp393 * (tmp413 / (tmp409 + float3(1.00999999f, 1.00999999f, 1.00999999f)) - tmp412) + tmp412;
    state.text_results[28].w = tmp415.x;
    state.text_results[29].x = tmp415.y;
    state.text_results[29].y = tmp415.z;
    float3 tmp416 = tmp415 + float3(1.0f, 1.0f, 1.0f);
    float3 tmp417 = tmp415 + float3(-1.0f, -1.0f, -1.0f);
    float3 tmp418 = (tmp416 * tmp416 * tmp410 - tmp417 * tmp417) / tmp413;
    state.text_results[29].z = sqrt(max(tmp418.x, 0.0f));
    state.text_results[29].w = sqrt(max(tmp418.y, 0.0f));
    state.text_results[30].x = sqrt(max(tmp418.z, 0.0f));
    int tmp419 = state.arg_block_offset;
    float tmp420 = _ZN11OmniSurface10OmniShared19ior_preset_to_valueEN11OmniSurface10OmniShared11ior_presetsEU7uniformf(mdl_read_argblock_as_int(tmp419 + 252), mdl_read_argblock_as_float(tmp419 + 256));
    state.text_results[30].y = tmp420;
    state.text_results[30].z = tmp420;
    state.text_results[30].w = tmp420;
    return;
}

structtype1 _ZNK23Fresnel_function_coated4evalERK6float2f(in float22 p_00, in float22 p_11, in float p_22)
{
    float tmp3;
    int phi_in;
    int phi_out;
    float phi_in4;
    float phi_out5;
    float phi_in6;
    float phi_out7;
    float phi_in8;
    float phi_out9;
    float phi_in10;
    float phi_out11;
    float tmp12;
    float tmp13;
    float tmp14;
    float tmp15;
    float tmp17;
    float3 phi_in18;
    float3 phi_out19;
    float tmp22;
    float tmp27;
    float tmp28;
    float tmp33;
    float tmp34;
    float tmp36;
    float tmp37;
    float tmp38;
    float tmp40;
    float tmp42;
    float tmp43;
    float tmp44;
    float tmp45;
    float phi_in46;
    float phi_out47;
    float tmp48;
    float tmp49;
    float phi_in50;
    float phi_out51;
    float tmp52;
    float tmp53;
    float tmp54;
    float tmp55;
    float tmp56;
    float tmp64;
    float tmp67;
    float tmp68;
    float tmp69;
    float tmp70;
    float tmp71;
    float tmp72;
    float phi_in73;
    float phi_out74;
    int phi_in75;
    int phi_out76;
    float phi_in77;
    float phi_out78;
    float phi_in79;
    float phi_out80;
    float phi_in81;
    float phi_out82;
    float tmp90;
    float tmp92;
    float tmp93;
    float phi_in95;
    float phi_out96;
    float phi_in97;
    float phi_out98;
    float phi_in99;
    float phi_out100;
    float3 tmp101;
    float tmp102;
    float tmp103;
    float tmp104;
    float tmp105;
    float phi_in106;
    float phi_out107;
    float phi_in108;
    float phi_out109;
    float phi_in110;
    float phi_out111;
    float phi_in112;
    float phi_out113;
    float tmp114;
    float tmp115;
    float tmp121;
    tmp3 = p_00.m_1;
    phi_in = 0;
    phi_in4 = 0.0f;
    phi_in6 = 0.0f;
    phi_in8 = 0.0f;
    phi_in10 = 0.0f;
    if (tmp3 > 0.0f) {
        tmp12 = p_00.m_0;
        phi_in = 0;
        phi_in4 = 0.0f;
        phi_in6 = 0.0f;
        phi_in8 = 0.0f;
        phi_in10 = 0.0f;
        if (!(tmp12 == 1.0f)) {
            tmp13 = p_11.m_1;
            tmp14 = p_11.m_0;
            tmp15 = max(tmp3, 0.0f);
            float tmp16 = tmp14 / tmp12;
            tmp17 = tmp16 * tmp16 * max(1.0f - p_22 * p_22, 0.0f);
            phi_in18 = float3(1.0f, 1.0f, 1.0f);
            if (!(tmp17 > 1.0f)) {
                float tmp20 = sqrt(max(1.0f - tmp17, 0.0f));
                float tmp21 = tmp14 * p_22;
                tmp22 = tmp20 * tmp12;
                float tmp23 = (tmp21 - tmp22) / (tmp22 + tmp21);
                float tmp24 = tmp20 * tmp14;
                float tmp25 = tmp12 * p_22;
                float tmp26 = (tmp25 - tmp24) / (tmp24 + tmp25);
                tmp27 = tmp23 * tmp23;
                tmp28 = tmp26 * tmp26;
                float tmp29 = tmp13 * tmp13;
                float tmp30 = tmp12 * tmp12;
                float tmp31 = (tmp29 - tmp17 * tmp30) * 0.5f;
                float tmp32 = sqrt(max(tmp31 * tmp31, 0.0f));
                tmp33 = tmp32 - tmp31;
                tmp34 = sqrt(max(tmp32 + tmp31, 0.0f));
                float tmp35 = sqrt(max(tmp33, 0.0f));
                tmp36 = tmp20 * tmp29;
                tmp37 = tmp34 * tmp12;
                tmp38 = tmp35 * tmp12;
                float tmp39 = tmp22 * 2.0f;
                tmp40 = tmp35 * tmp39;
                float tmp41 = tmp32 * 2.0f;
                tmp42 = tmp41 - tmp22 * tmp22;
                tmp43 = tmp39 * -tmp29 * tmp35;
                tmp44 = tmp36 * tmp36 - tmp41 * tmp30;
                tmp45 = tmp40 * tmp40 + tmp42 * tmp42;
                phi_in46 = 0.0f;
                if (tmp45 > 0.0f) {
                    tmp48 = 1.0f / sqrt(tmp45);
                    phi_in46 = tmp48;
                }
                phi_out47 = phi_in46;
                tmp49 = tmp43 * tmp43 + tmp44 * tmp44;
                phi_in50 = 0.0f;
                if (tmp49 > 0.0f) {
                    tmp52 = 1.0f / sqrt(tmp49);
                    phi_in50 = tmp52;
                }
                phi_out51 = phi_in50;
                tmp53 = phi_out47 * tmp42;
                tmp54 = phi_out51 * tmp44;
                tmp55 = phi_out47 * tmp40;
                tmp56 = phi_out51 * tmp43;
                float tmp57 = tmp22 - tmp34;
                float tmp58 = tmp34 + tmp22;
                float tmp59 = (tmp57 * tmp57 + tmp33) / (tmp58 * tmp58 + tmp33);
                float tmp60 = tmp36 - tmp37;
                float tmp61 = tmp38 * tmp38;
                float tmp62 = tmp37 + tmp36;
                float tmp63 = (tmp60 * tmp60 + tmp61) / (tmp62 * tmp62 + tmp61);
                tmp64 = tmp15 * 12.56637096f * tmp22;
                float tmp65 = max(tmp59 * tmp27, 0.0f);
                float tmp66 = max(tmp63 * tmp28, 0.0f);
                tmp67 = sqrt(tmp65) * 2.0f;
                tmp68 = tmp59 + tmp27;
                tmp69 = tmp65 + 1.0f;
                tmp70 = sqrt(tmp66) * 2.0f;
                tmp71 = tmp63 + tmp28;
                tmp72 = tmp66 + 1.0f;
                phi_in73 = 409.375f;
                phi_in75 = 0;
                phi_in77 = 0.0f;
                phi_in79 = 0.0f;
                phi_in81 = 0.0f;
                do {
                    phi_out74 = phi_in73;
                    phi_out76 = phi_in75;
                    phi_out78 = phi_in77;
                    phi_out80 = phi_in79;
                    phi_out82 = phi_in81;
                    float tmp83 = tmp64 / phi_out74;
                    float tmp84 = sin(tmp83);
                    float tmp85 = cos(tmp83);
                    float tmp86 = tmp67 * (tmp53 * tmp85 - tmp55 * tmp84);
                    float tmp87 = tmp70 * (tmp54 * tmp85 - tmp56 * tmp84);
                    int tmp88 = phi_out76;
                    float tmp89 = (tmp68 + tmp86) / (tmp69 + tmp86) + (tmp71 + tmp87) / (tmp72 + tmp87);
                    tmp90 = tmp89 * glob_cnst91[tmp88].x + phi_out82;
                    tmp92 = tmp89 * glob_cnst91[tmp88].y + phi_out80;
                    tmp93 = tmp89 * glob_cnst91[tmp88].z + phi_out78;
                    int tmp94 = phi_out76 + 1;
                    phi_in95 = tmp90;
                    phi_in97 = tmp92;
                    phi_in99 = tmp93;
                    phi_in73 = phi_out74 + 18.75f;
                    phi_in75 = tmp94;
                    phi_in77 = tmp93;
                    phi_in79 = tmp92;
                    phi_in81 = tmp90;
                    if (tmp94 == 16)
                        break;
                } while (true);
                phi_out96 = phi_in95;
                phi_out98 = phi_in97;
                phi_out100 = phi_in99;
                tmp101 = float3(min(max(phi_out98 * -0.1108108461f + phi_out96 * 0.2336024195f + phi_out100 * -0.03594215959f, 0.0f), 1.0f), min(max(phi_out98 * 0.1716095954f + phi_out96 * -0.08864086866f + phi_out100 * 0.003796672449f, 0.0f), 1.0f), min(max(phi_out98 * -0.0194983948f + phi_out96 * 0.005323826335f + phi_out100 * 0.1010284424f, 0.0f), 1.0f));
                phi_in18 = tmp101;
            }
            phi_out19 = phi_in18;
            tmp102 = phi_out19.x;
            tmp103 = phi_out19.y;
            tmp104 = phi_out19.z;
            tmp105 = (tmp102 + tmp103 + tmp104) * 0.3333333433f;
            phi_in = 1;
            phi_in4 = tmp102;
            phi_in6 = tmp103;
            phi_in8 = tmp104;
            phi_in10 = tmp105;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    phi_out9 = phi_in8;
    phi_out11 = phi_in10;
    phi_in106 = phi_out5;
    phi_in108 = phi_out7;
    phi_in110 = phi_out9;
    phi_in112 = phi_out11;
    if (phi_out == 0) {
        tmp114 = p_11.m_1 / p_11.m_0;
        tmp115 = 1.0f - (1.0f - p_22 * p_22) / (tmp114 * tmp114);
        phi_in106 = 1.0f;
        phi_in108 = 1.0f;
        phi_in110 = 1.0f;
        phi_in112 = 1.0f;
        if (!(tmp115 < 0.0f)) {
            float tmp116 = sqrt(tmp115);
            float tmp117 = tmp114 * p_22;
            float tmp118 = tmp116 * tmp114;
            float tmp119 = (tmp116 - tmp117) / (tmp116 + tmp117);
            float tmp120 = (p_22 - tmp118) / (tmp118 + p_22);
            tmp121 = min(max((tmp120 * tmp120 + tmp119 * tmp119) * 0.5f, 0.0f), 1.0f);
            phi_in106 = tmp121;
            phi_in108 = tmp121;
            phi_in110 = tmp121;
            phi_in112 = tmp121;
        }
    }
    phi_out107 = phi_in106;
    phi_out109 = phi_in108;
    phi_out111 = phi_in110;
    phi_out113 = phi_in112;
    return constr_structtype1(phi_out107, phi_out109, phi_out111, phi_out113);
}

void gen_weighted_layer_sample1(inout Bsdf_sample_data p_00, in Shading_state_material p_11)
{
    float tmp2;
    float tmp3;
    float tmp4;
    float tmp5;
    float tmp6;
    bool tmp7;
    float tmp8;
    float phi_in;
    float phi_out;
    float tmp9;
    float3 tmp10;
    float3 tmp12;
    float tmp14;
    float tmp15;
    float tmp16;
    float tmp17;
    float3 tmp18;
    float3 tmp19;
    float3 tmp20;
    float3 tmp21;
    float tmp23;
    float tmp25;
    float tmp26;
    float tmp27;
    float tmp30;
    float tmp31;
    float tmp32;
    float3 tmp33;
    float tmp35;
    float tmp36;
    float tmp37;
    float tmp38;
    bool tmp39;
    float tmp40;
    float phi_in41;
    float phi_out42;
    float tmp43;
    float tmp44;
    float3 phi_in45;
    float3 phi_out46;
    float tmp49;
    float tmp50;
    float tmp51;
    float tmp52;
    float tmp53;
    float phi_in54;
    float phi_out55;
    float phi_in56;
    float phi_out57;
    float phi_in58;
    float phi_out59;
    float tmp60;
    float tmp61;
    float3 tmp62;
    float tmp63;
    float tmp64;
    float tmp65;
    float phi_in66;
    float phi_out67;
    float phi_in68;
    float phi_out69;
    float phi_in70;
    float phi_out71;
    float phi_in72;
    float phi_out73;
    float phi_in74;
    float phi_out75;
    float phi_in76;
    float phi_out77;
    float phi_in82;
    float phi_out83;
    float phi_in84;
    float phi_out85;
    float tmp86;
    float tmp87;
    float tmp88;
    int phi_in95;
    int phi_out96;
    float3 tmp98;
    float tmp99;
    float tmp100;
    float tmp101;
    float3 tmp102;
    float tmp104;
    float tmp105;
    float tmp106;
    float tmp107;
    float3 tmp108;
    float3 tmp109;
    float3 tmp110;
    float3 tmp111;
    float tmp113;
    float tmp115;
    float tmp116;
    float tmp117;
    float tmp120;
    float tmp121;
    float tmp122;
    float tmp123;
    float3 tmp124;
    float tmp126;
    float tmp127;
    float tmp128;
    float tmp129;
    bool tmp130;
    float tmp131;
    float phi_in132;
    float phi_out133;
    float tmp134;
    float tmp135;
    float3 phi_in136;
    float3 phi_out137;
    float tmp140;
    float tmp141;
    float tmp142;
    float tmp143;
    float tmp144;
    float phi_in145;
    float phi_out146;
    float phi_in147;
    float phi_out148;
    float phi_in149;
    float phi_out150;
    float tmp151;
    float tmp152;
    float3 tmp153;
    float tmp154;
    float tmp155;
    float tmp156;
    float phi_in157;
    float phi_out158;
    float phi_in159;
    float phi_out160;
    float phi_in161;
    float phi_out162;
    float phi_in163;
    float phi_out164;
    float phi_in165;
    float phi_out166;
    float phi_in167;
    float phi_out168;
    float phi_in173;
    float phi_out174;
    float phi_in175;
    float phi_out176;
    float tmp177;
    float tmp178;
    float tmp179;
    float3 tmp184;
    float tmp187;
    float tmp189;
    float tmp190;
    float3 tmp191;
    float3 tmp193;
    float tmp195;
    float phi_in196;
    float phi_out197;
    float tmp199;
    tmp2 = p_11.text_results[25].x;
    tmp3 = p_11.text_results[25].y;
    tmp4 = p_11.text_results[25].z;
    tmp5 = min(max(p_11.text_results[13].y, 0.0f), 1.0f);
    tmp6 = p_00.xi.z;
    tmp7 = tmp6 < tmp5;
    if (tmp7) {
        tmp8 = tmp6 / tmp5;
        phi_in = tmp8;
    } else {
        tmp9 = (tmp6 - tmp5) / (1.0f - tmp5);
        phi_in = tmp9;
    }
    phi_out = phi_in;
    p_00.xi.z = phi_out;
    if (tmp7) {
        tmp10 = mdl_read_argblock_as_bool(p_11.arg_block_offset) ? p_11.text_results[14].xyz : float3(1.0f, 1.0f, 1.0f);
        float3 tmp11 = p_11.tangent_u[0];
        tmp12 = p_11.geom_normal;
        float3 tmp13 = p_00.k1 * tmp12;
        tmp14 = asfloat((asint(tmp13.x + tmp13.y + tmp13.z) & -2147483648) | 1065353216);
        tmp15 = tmp2 * tmp14;
        tmp16 = tmp3 * tmp14;
        tmp17 = tmp4 * tmp14;
        tmp18 = float3(tmp15, tmp16, tmp17);
        tmp19 = tmp18.zxy;
        tmp20 = tmp18.yzx;
        tmp21 = tmp19 * tmp11.yzx - tmp20 * tmp11.zxy;
        float3 tmp22 = tmp21 * tmp21;
        tmp23 = tmp22.x + tmp22.y + tmp22.z;
        if (tmp23 < 9.999999939e-09f) {
            p_00.pdf = 0.0f;
            p_00.event_type = 0;
        } else {
            float tmp24 = 1.0f / sqrt(tmp23);
            tmp25 = tmp24 * tmp21.x;
            tmp26 = tmp24 * tmp21.y;
            tmp27 = tmp24 * tmp21.z;
            float3 tmp28 = float3(tmp25, tmp26, tmp27);
            float3 tmp29 = tmp28.zxy * tmp20 - tmp28.yzx * tmp19;
            tmp30 = min(max(tmp10.x, 0.0f), 1.0f);
            tmp31 = min(max(tmp10.y, 0.0f), 1.0f);
            tmp32 = min(max(tmp10.z, 0.0f), 1.0f);
            tmp33 = float3(tmp12.x * tmp14, tmp12.y * tmp14, tmp12.z * tmp14);
            float3 tmp34 = tmp33 * tmp18;
            tmp35 = max(tmp34.x + tmp34.y + tmp34.z, 0.0f);
            tmp36 = 3.141592741f / (tmp35 * 1.570796371f + 1.570796371f);
            tmp37 = tmp36 + -1.0f;
            tmp38 = p_00.xi.x;
            tmp39 = tmp38 < tmp37;
            if (tmp39) {
                tmp40 = tmp38 / tmp37;
                phi_in41 = tmp40;
            } else {
                tmp43 = (tmp38 - tmp37) / (2.0f - tmp36);
                phi_in41 = tmp43;
            }
            phi_out42 = phi_in41;
            p_00.xi.x = phi_out42;
            tmp44 = p_00.xi.y;
            phi_in45 = float3(0.0f, 1.0f, 0.0f);
            if (!(phi_out42 == 0.0f && tmp44 == 0.0f)) {
                float tmp47 = phi_out42 * 2.0f;
                float tmp48 = tmp44 * 2.0f;
                tmp49 = tmp47 < 1.0f ? tmp47 : tmp47 + -2.0f;
                tmp50 = tmp48 < 1.0f ? tmp48 : tmp48 + -2.0f;
                tmp51 = tmp49 * tmp49;
                tmp52 = tmp50 * tmp50;
                if (tmp51 > tmp52) {
                    tmp53 = tmp50 * -0.7853981853f / tmp49;
                    phi_in54 = tmp49;
                    phi_in56 = tmp53;
                    phi_in58 = tmp51;
                } else {
                    tmp60 = tmp49 * 0.7853981853f / tmp50 + -1.570796371f;
                    phi_in54 = tmp50;
                    phi_in56 = tmp60;
                    phi_in58 = tmp52;
                }
                phi_out55 = phi_in54;
                phi_out57 = phi_in56;
                phi_out59 = phi_in58;
                tmp61 = 1.0f - phi_out59;
                phi_in45 = float3(0.0f, 1.0f, 0.0f);
                if (tmp61 > 0.0f) {
                    tmp62 = float3(sin(phi_out57) * phi_out55, sqrt(tmp61), cos(phi_out57) * phi_out55);
                    phi_in45 = tmp62;
                }
            }
            phi_out46 = phi_in45;
            tmp63 = phi_out46.x;
            tmp64 = phi_out46.y;
            tmp65 = phi_out46.z;
            phi_in66 = tmp29.x;
            phi_in68 = tmp29.y;
            phi_in70 = tmp29.z;
            phi_in72 = tmp25;
            phi_in74 = tmp26;
            phi_in76 = tmp27;
            if (tmp35 < 0.9990000129f) {
                float3 tmp78 = tmp19 * tmp33.yzx - tmp20 * tmp33.zxy;
                float3 tmp79 = float3(sqrt(tmp78.x * tmp78.x + tmp78.y * tmp78.y + tmp78.z * tmp78.z), 0.0f, 0.0f);
                float3 tmp80 = tmp78 / tmp79.xxx;
                float3 tmp81 = tmp80.yzx * tmp19 - tmp80.zxy * tmp20;
                phi_in66 = tmp80.x;
                phi_in68 = tmp80.y;
                phi_in70 = tmp80.z;
                phi_in72 = tmp81.x;
                phi_in74 = tmp81.y;
                phi_in76 = tmp81.z;
            }
            phi_out67 = phi_in66;
            phi_out69 = phi_in68;
            phi_out71 = phi_in70;
            phi_out73 = phi_in72;
            phi_out75 = phi_in74;
            phi_out77 = phi_in76;
            phi_in82 = tmp64;
            phi_in84 = tmp65;
            if (tmp65 > 0.0f) {
                if (tmp39) {
                    tmp86 = -tmp65;
                    phi_in82 = tmp64;
                    phi_in84 = tmp86;
                } else {
                    tmp87 = tmp65 * tmp35;
                    tmp88 = sqrt(max(1.0f - (tmp87 * tmp87 + tmp63 * tmp63), 0.0f));
                    phi_in82 = tmp88;
                    phi_in84 = tmp87;
                }
            }
            phi_out83 = phi_in82;
            phi_out85 = phi_in84;
            float tmp89 = phi_out67 * tmp63 - (phi_out83 * tmp15 + phi_out85 * phi_out73);
            float tmp90 = phi_out69 * tmp63 - (phi_out83 * tmp16 + phi_out85 * phi_out75);
            float tmp91 = phi_out71 * tmp63 - (phi_out83 * tmp17 + phi_out85 * phi_out77);
            float3 tmp92 = float3(sqrt(tmp89 * tmp89 + tmp90 * tmp90 + tmp91 * tmp91), 0.0f, 0.0f);
            float3 tmp93 = float3(tmp89, tmp90, tmp91) / tmp92.xxx;
            p_00.k2.x = tmp93.x;
            p_00.k2.y = tmp93.y;
            p_00.k2.z = tmp93.z;
            float3 tmp94 = tmp93 * tmp33;
            if (tmp94.x + tmp94.y + tmp94.z > 0.0f) {
                p_00.pdf = 0.0f;
                phi_in95 = 0;
            } else {
                p_00.bsdf_over_pdf.x = tmp30;
                p_00.bsdf_over_pdf.y = tmp31;
                p_00.bsdf_over_pdf.z = tmp32;
                p_00.handle = 0;
                phi_in95 = 17;
            }
            phi_out96 = phi_in95;
            p_00.event_type = phi_out96;
        }
    } else if (phi_out < 1.0f) {
        float3 tmp97 = p_11.tangent_u[0];
        tmp98 = p_11.geom_normal;
        tmp99 = p_00.k1.x;
        tmp100 = p_00.k1.y;
        tmp101 = p_00.k1.z;
        tmp102 = float3(tmp99, tmp100, tmp101);
        float3 tmp103 = tmp102 * tmp98;
        tmp104 = asfloat((asint(tmp103.x + tmp103.y + tmp103.z) & -2147483648) | 1065353216);
        tmp105 = tmp2 * tmp104;
        tmp106 = tmp3 * tmp104;
        tmp107 = tmp4 * tmp104;
        tmp108 = float3(tmp105, tmp106, tmp107);
        tmp109 = tmp108.zxy;
        tmp110 = tmp108.yzx;
        tmp111 = tmp109 * tmp97.yzx - tmp110 * tmp97.zxy;
        float3 tmp112 = tmp111 * tmp111;
        tmp113 = tmp112.x + tmp112.y + tmp112.z;
        if (tmp113 < 9.999999939e-09f) {
            p_00.pdf = 0.0f;
            p_00.event_type = 0;
        } else {
            float tmp114 = 1.0f / sqrt(tmp113);
            tmp115 = tmp114 * tmp111.x;
            tmp116 = tmp114 * tmp111.y;
            tmp117 = tmp114 * tmp111.z;
            float3 tmp118 = float3(tmp115, tmp116, tmp117);
            float3 tmp119 = tmp118.zxy * tmp110 - tmp118.yzx * tmp109;
            tmp120 = min(max(p_11.text_results[24].x, 0.0f), 1.0f);
            tmp121 = min(max(p_11.text_results[24].y, 0.0f), 1.0f);
            tmp122 = min(max(p_11.text_results[24].z, 0.0f), 1.0f);
            tmp123 = max(p_11.text_results[20].w, 0.0f);
            tmp124 = float3(tmp98.x * tmp104, tmp98.y * tmp104, tmp98.z * tmp104);
            float3 tmp125 = tmp124 * tmp108;
            tmp126 = max(tmp125.x + tmp125.y + tmp125.z, 0.0f);
            tmp127 = 3.141592741f / (tmp126 * 1.570796371f + 1.570796371f);
            tmp128 = tmp127 + -1.0f;
            tmp129 = p_00.xi.x;
            tmp130 = tmp129 < tmp128;
            if (tmp130) {
                tmp131 = tmp129 / tmp128;
                phi_in132 = tmp131;
            } else {
                tmp134 = (tmp129 - tmp128) / (2.0f - tmp127);
                phi_in132 = tmp134;
            }
            phi_out133 = phi_in132;
            p_00.xi.x = phi_out133;
            tmp135 = p_00.xi.y;
            phi_in136 = float3(0.0f, 1.0f, 0.0f);
            if (!(phi_out133 == 0.0f && tmp135 == 0.0f)) {
                float tmp138 = phi_out133 * 2.0f;
                float tmp139 = tmp135 * 2.0f;
                tmp140 = tmp138 < 1.0f ? tmp138 : tmp138 + -2.0f;
                tmp141 = tmp139 < 1.0f ? tmp139 : tmp139 + -2.0f;
                tmp142 = tmp140 * tmp140;
                tmp143 = tmp141 * tmp141;
                if (tmp142 > tmp143) {
                    tmp144 = tmp141 * -0.7853981853f / tmp140;
                    phi_in145 = tmp140;
                    phi_in147 = tmp144;
                    phi_in149 = tmp142;
                } else {
                    tmp151 = tmp140 * 0.7853981853f / tmp141 + -1.570796371f;
                    phi_in145 = tmp141;
                    phi_in147 = tmp151;
                    phi_in149 = tmp143;
                }
                phi_out146 = phi_in145;
                phi_out148 = phi_in147;
                phi_out150 = phi_in149;
                tmp152 = 1.0f - phi_out150;
                phi_in136 = float3(0.0f, 1.0f, 0.0f);
                if (tmp152 > 0.0f) {
                    tmp153 = float3(sin(phi_out148) * phi_out146, sqrt(tmp152), cos(phi_out148) * phi_out146);
                    phi_in136 = tmp153;
                }
            }
            phi_out137 = phi_in136;
            tmp154 = phi_out137.x;
            tmp155 = phi_out137.y;
            tmp156 = phi_out137.z;
            phi_in157 = tmp119.x;
            phi_in159 = tmp119.y;
            phi_in161 = tmp119.z;
            phi_in163 = tmp115;
            phi_in165 = tmp116;
            phi_in167 = tmp117;
            if (tmp126 < 0.9990000129f) {
                float3 tmp169 = tmp109 * tmp124.yzx - tmp110 * tmp124.zxy;
                float3 tmp170 = float3(sqrt(tmp169.x * tmp169.x + tmp169.y * tmp169.y + tmp169.z * tmp169.z), 0.0f, 0.0f);
                float3 tmp171 = tmp169 / tmp170.xxx;
                float3 tmp172 = tmp171.yzx * tmp109 - tmp171.zxy * tmp110;
                phi_in157 = tmp171.x;
                phi_in159 = tmp171.y;
                phi_in161 = tmp171.z;
                phi_in163 = tmp172.x;
                phi_in165 = tmp172.y;
                phi_in167 = tmp172.z;
            }
            phi_out158 = phi_in157;
            phi_out160 = phi_in159;
            phi_out162 = phi_in161;
            phi_out164 = phi_in163;
            phi_out166 = phi_in165;
            phi_out168 = phi_in167;
            phi_in173 = tmp155;
            phi_in175 = tmp156;
            if (tmp156 > 0.0f) {
                if (tmp130) {
                    tmp177 = -tmp156;
                    phi_in173 = tmp155;
                    phi_in175 = tmp177;
                } else {
                    tmp178 = tmp156 * tmp126;
                    tmp179 = sqrt(max(1.0f - (tmp178 * tmp178 + tmp154 * tmp154), 0.0f));
                    phi_in173 = tmp179;
                    phi_in175 = tmp178;
                }
            }
            phi_out174 = phi_in173;
            phi_out176 = phi_in175;
            float tmp180 = phi_out174 * tmp105 + phi_out158 * tmp154 + phi_out176 * phi_out164;
            float tmp181 = phi_out174 * tmp106 + phi_out160 * tmp154 + phi_out176 * phi_out166;
            float tmp182 = phi_out174 * tmp107 + phi_out162 * tmp154 + phi_out176 * phi_out168;
            float3 tmp183 = float3(sqrt(tmp180 * tmp180 + tmp181 * tmp181 + tmp182 * tmp182), 0.0f, 0.0f);
            tmp184 = float3(tmp180, tmp181, tmp182) / tmp183.xxx;
            p_00.k2.x = tmp184.x;
            p_00.k2.y = tmp184.y;
            p_00.k2.z = tmp184.z;
            float3 tmp185 = tmp184 * tmp124;
            if (tmp185.x + tmp185.y + tmp185.z > 0.0f) {
                p_00.bsdf_over_pdf.x = tmp120;
                p_00.bsdf_over_pdf.y = tmp121;
                p_00.bsdf_over_pdf.z = tmp122;
                if (tmp123 > 0.0f) {
                    float3 tmp186 = tmp184 * tmp108;
                    tmp187 = tmp186.x + tmp186.y + tmp186.z;
                    float3 tmp188 = tmp108 * tmp102;
                    tmp189 = tmp188.x + tmp188.y + tmp188.z;
                    tmp190 = tmp123 * tmp123;
                    tmp191 = float3(tmp184.x - tmp187 * tmp105, tmp184.y - tmp187 * tmp106, tmp184.z - tmp187 * tmp107);
                    float3 tmp192 = tmp191 * tmp191;
                    tmp193 = float3(tmp99 - tmp189 * tmp105, tmp100 - tmp189 * tmp106, tmp101 - tmp189 * tmp107);
                    float3 tmp194 = tmp193 * tmp193;
                    tmp195 = (tmp192.x + tmp192.y + tmp192.z) * (tmp194.x + tmp194.y + tmp194.z);
                    phi_in196 = 0.0f;
                    if (!(tmp195 == 0.0f)) {
                        float3 tmp198 = tmp191 * tmp193;
                        tmp199 = tmp190 * 0.4499999881f * max(sqrt((1.0f - tmp187 * tmp187) * (1.0f - tmp189 * tmp189) / tmp195) * (tmp198.x + tmp198.y + tmp198.z) / max(tmp187, tmp189), 0.0f) / (tmp190 + 0.09000000358f);
                        phi_in196 = tmp199;
                    }
                    phi_out197 = phi_in196;
                    float tmp200 = phi_out197 + (1.0f - tmp190 / (tmp190 * 2.0f + 0.6600000262f));
                    p_00.bsdf_over_pdf.x = tmp200 * tmp120;
                    p_00.bsdf_over_pdf.y = tmp200 * tmp121;
                    p_00.bsdf_over_pdf.z = tmp200 * tmp122;
                }
                p_00.event_type = 9;
                p_00.handle = 0;
            } else {
                p_00.pdf = 0.0f;
                p_00.event_type = 0;
            }
        }
    } else {
        p_00.xi.z = (phi_out + -1.0f) / 0.0f;
        p_00.pdf = 0.0f;
        p_00.event_type = 0;
    }
    return;
}

void gen_weighted_layer_sample(inout Bsdf_sample_data p_00, in Shading_state_material p_11)
{
    float tmp2;
    float tmp3;
    float tmp4;
    float tmp5;
    float tmp6;
    float tmp7;
    float22 tmp8;
    float22 tmp9;
    float tmp10;
    float tmp11;
    float tmp12;
    float tmp13;
    float tmp14;
    bool tmp15;
    float tmp16;
    float phi_in;
    float phi_out;
    float tmp17;
    float tmp18;
    float tmp19;
    float tmp20;
    float tmp21;
    float tmp22;
    float tmp23;
    float tmp24;
    float tmp25;
    float tmp26;
    float tmp27;
    float tmp28;
    float tmp29;
    float tmp30;
    float tmp31;
    float tmp33;
    float tmp34;
    float tmp35;
    float3 tmp36;
    float tmp39;
    float tmp40;
    float tmp41;
    float tmp42;
    float tmp43;
    float tmp44;
    float3 tmp46;
    float3 tmp47;
    float3 tmp48;
    float3 tmp49;
    float tmp51;
    int phi_in52;
    int phi_out53;
    float tmp55;
    float tmp56;
    float tmp57;
    float3 tmp58;
    float3 tmp59;
    float tmp64;
    float tmp65;
    float tmp68;
    float tmp69;
    float tmp70;
    float3 tmp72;
    float phi_in73;
    float phi_out74;
    float phi_in75;
    float phi_out76;
    float phi_in77;
    float phi_out78;
    float tmp86;
    float tmp87;
    float tmp88;
    float tmp89;
    float phi_in90;
    float phi_out91;
    float phi_in92;
    float phi_out93;
    float tmp94;
    int phi_in103;
    int phi_out104;
    bool phi_in105;
    bool phi_out106;
    float tmp107;
    float tmp108;
    float tmp109;
    float tmp111;
    float3 tmp116;
    float tmp122;
    float phi_in123;
    float phi_out124;
    float tmp125;
    float tmp126;
    float tmp127;
    float tmp146;
    float tmp147;
    float tmp148;
    float tmp149;
    float tmp150;
    float phi_in151;
    float phi_out152;
    float phi_in153;
    float phi_out154;
    float phi_in155;
    float phi_out156;
    float tmp157;
    float tmp158;
    float tmp159;
    float tmp160;
    float phi_in161;
    float phi_out162;
    float phi_in163;
    float phi_out164;
    float phi_in165;
    float phi_out166;
    float phi_in167;
    float phi_out168;
    float phi_in169;
    float phi_out170;
    int phi_in171;
    int phi_out172;
    float phi_in173;
    float phi_out174;
    float phi_in175;
    float phi_out176;
    float phi_in177;
    float phi_out178;
    float tmp180;
    float phi_in181;
    float phi_out182;
    float phi_in183;
    float phi_out184;
    float phi_in185;
    float phi_out186;
    int phi_in187;
    int phi_out188;
    float phi_in189;
    float phi_out190;
    float phi_in191;
    float phi_out192;
    float phi_in193;
    float phi_out194;
    float tmp196;
    float tmp197;
    float tmp198;
    float tmp199;
    int tmp200;
    float rmemload169;
    float rmemload173;
    float phi_in201;
    float phi_out202;
    float phi_in203;
    float phi_out204;
    float phi_in205;
    float phi_out206;
    float phi_in207;
    float phi_out208;
    float rmemload168;
    float rmemload172;
    float rmemload167;
    float rmemload171;
    float tmp210;
    float tmp211;
    float phi_in212;
    float phi_out213;
    float phi_in214;
    float phi_out215;
    float phi_in216;
    float phi_out217;
    float phi_in218;
    float phi_out219;
    float phi_in220;
    float phi_out221;
    float phi_in222;
    float phi_out223;
    float phi_in224;
    float phi_out225;
    float phi_in226;
    float phi_out227;
    int phi_in228;
    int phi_out229;
    float phi_in230;
    float phi_out231;
    float phi_in232;
    float phi_out233;
    float phi_in234;
    float phi_out235;
    int phi_in236;
    int phi_out237;
    float phi_in238;
    float phi_out239;
    float phi_in240;
    float phi_out241;
    float phi_in242;
    float phi_out243;
    float phi_in244;
    float phi_out245;
    float phi_in246;
    float phi_out247;
    float tmp250;
    float tmp255;
    float tmp256;
    float tmp263;
    float tmp264;
    float tmp266;
    float tmp267;
    float tmp269;
    float tmp270;
    float tmp272;
    float tmp274;
    float tmp275;
    float tmp277;
    float tmp278;
    float phi_in279;
    float phi_out280;
    float tmp281;
    float tmp282;
    float phi_in283;
    float phi_out284;
    float tmp285;
    float tmp286;
    float tmp287;
    float tmp288;
    float tmp289;
    float tmp298;
    float tmp301;
    float tmp302;
    float tmp303;
    float tmp304;
    float tmp305;
    float tmp306;
    float phi_in307;
    float phi_out308;
    float phi_in309;
    float phi_out310;
    float phi_in311;
    float phi_out312;
    int phi_in313;
    int phi_out314;
    float phi_in315;
    float phi_out316;
    float phi_in317;
    float phi_out318;
    float phi_in319;
    float phi_out320;
    float phi_in321;
    float phi_out322;
    float phi_in323;
    float phi_out324;
    float phi_in325;
    float phi_out326;
    float phi_in327;
    float phi_out328;
    float phi_in329;
    float phi_out330;
    int phi_in331;
    int phi_out332;
    float phi_in333;
    float phi_out334;
    float phi_in335;
    float phi_out336;
    float phi_in337;
    float phi_out338;
    float phi_in339;
    float phi_out340;
    float phi_in341;
    float phi_out342;
    float tmp350;
    float tmp351;
    float tmp352;
    float tmp353;
    int tmp354;
    float rmemload157;
    float rmemload161;
    float phi_in355;
    float phi_out356;
    float phi_in357;
    float phi_out358;
    float phi_in359;
    float phi_out360;
    float phi_in361;
    float phi_out362;
    float rmemload156;
    float rmemload160;
    float rmemload;
    float rmemload159;
    bool tmp363;
    float phi_in364;
    float phi_out365;
    float phi_in366;
    float phi_out367;
    float phi_in368;
    float phi_out369;
    float phi_in370;
    float phi_out371;
    float phi_in372;
    float phi_out373;
    float phi_in374;
    float phi_out375;
    float phi_in376;
    float phi_out377;
    float tmp378;
    float tmp379;
    float tmp380;
    float phi_in381;
    float phi_out382;
    float phi_in383;
    float phi_out384;
    float phi_in385;
    float phi_out386;
    float tmp410;
    float tmp420;
    float tmp430;
    float tmp431;
    bool tmp432;
    float tmp433;
    float phi_in434;
    float phi_out435;
    float tmp436;
    float tmp437;
    bool tmp438;
    float tmp439;
    float phi_in440;
    float phi_out441;
    float tmp442;
    int tmp443;
    bool tmp444;
    bool phi_in445;
    bool phi_out446;
    float3 tmp447;
    float tmp448;
    float tmp449;
    float tmp450;
    float tmp452;
    float tmp453;
    float tmp454;
    float tmp455;
    bool tmp456;
    float tmp457;
    float tmp458;
    float tmp459;
    float tmp461;
    float tmp462;
    float tmp463;
    float3 tmp464;
    float tmp467;
    float tmp468;
    float tmp469;
    float tmp470;
    float tmp471;
    float tmp472;
    float3 tmp474;
    float3 tmp475;
    float3 tmp476;
    float3 tmp477;
    float tmp479;
    int phi_in480;
    int phi_out481;
    float tmp483;
    float tmp484;
    float tmp485;
    float3 tmp486;
    float3 tmp487;
    float tmp488;
    float phi_in489;
    float phi_out490;
    float tmp491;
    float tmp492;
    float tmp493;
    float tmp494;
    float tmp495;
    float phi_in496;
    float phi_out497;
    float phi_in498;
    float phi_out499;
    float phi_in500;
    float phi_out501;
    float tmp502;
    float tmp503;
    float tmp504;
    float tmp507;
    float tmp512;
    float tmp513;
    float tmp516;
    float tmp517;
    float tmp518;
    float3 tmp520;
    float phi_in521;
    float phi_out522;
    float phi_in523;
    float phi_out524;
    float phi_in525;
    float phi_out526;
    float tmp534;
    float tmp535;
    float tmp536;
    float tmp537;
    float phi_in538;
    float phi_out539;
    float phi_in540;
    float phi_out541;
    float tmp542;
    int phi_in551;
    int phi_out552;
    float tmp553;
    float tmp554;
    float tmp555;
    float tmp557;
    float tmp559;
    float tmp560;
    float tmp561;
    float tmp562;
    float tmp564;
    float tmp565;
    float tmp566;
    float tmp568;
    float tmp569;
    float tmp570;
    int phi_in571;
    int phi_out572;
    float phi_in573;
    float phi_out574;
    float phi_in575;
    float phi_out576;
    float phi_in577;
    float phi_out578;
    float phi_in579;
    float phi_out580;
    float phi_in581;
    float phi_out582;
    float phi_in583;
    float phi_out584;
    float tmp585;
    float phi_in597;
    float phi_out598;
    float phi_in599;
    float phi_out600;
    float phi_in601;
    float phi_out602;
    int phi_in603;
    int phi_out604;
    float tmp605;
    float tmp606;
    float tmp608;
    float tmp609;
    float tmp610;
    int phi_in611;
    int phi_out612;
    float phi_in613;
    float phi_out614;
    float phi_in615;
    float phi_out616;
    float phi_in617;
    float phi_out618;
    float tmp626;
    float tmp627;
    float tmp628;
    float3 tmp629;
    float tmp636;
    int phi_in637;
    int phi_out638;
    float tmp639;
    float phi_in640;
    float phi_out641;
    float tmp642;
    float tmp643;
    float phi_in644;
    float phi_out645;
    float tmp647;
    float tmp648;
    float tmp665;
    bool tmp666;
    float tmp667;
    float phi_in668;
    float phi_out669;
    float tmp670;
    bool phi_in671;
    bool phi_out672;
    float3 tmp673;
    float3 tmp675;
    float tmp677;
    float tmp678;
    float tmp679;
    float tmp680;
    float3 tmp681;
    float3 tmp682;
    float3 tmp683;
    float3 tmp684;
    float tmp686;
    float tmp688;
    float tmp689;
    float tmp690;
    float tmp693;
    float tmp694;
    float tmp695;
    float3 tmp696;
    float tmp698;
    float tmp699;
    float tmp700;
    float tmp701;
    bool tmp702;
    float tmp703;
    float phi_in704;
    float phi_out705;
    float tmp706;
    float tmp707;
    float3 phi_in708;
    float3 phi_out709;
    float tmp712;
    float tmp713;
    float tmp714;
    float tmp715;
    float tmp716;
    float phi_in717;
    float phi_out718;
    float phi_in719;
    float phi_out720;
    float phi_in721;
    float phi_out722;
    float tmp723;
    float tmp724;
    float3 tmp725;
    float tmp726;
    float tmp727;
    float tmp728;
    float phi_in729;
    float phi_out730;
    float phi_in731;
    float phi_out732;
    float phi_in733;
    float phi_out734;
    float phi_in735;
    float phi_out736;
    float phi_in737;
    float phi_out738;
    float phi_in739;
    float phi_out740;
    float phi_in745;
    float phi_out746;
    float phi_in747;
    float phi_out748;
    float tmp749;
    float tmp750;
    float tmp751;
    int phi_in758;
    int phi_out759;
    float tmp760;
    float tmp761;
    float tmp762;
    bool tmp763;
    bool phi_in764;
    bool phi_out765;
    float tmp767;
    float tmp768;
    float tmp769;
    float tmp770;
    float tmp771;
    float tmp773;
    float tmp774;
    float tmp775;
    float3 tmp776;
    float tmp779;
    float tmp780;
    float tmp781;
    float tmp782;
    float tmp783;
    float tmp784;
    float3 tmp786;
    float3 tmp787;
    float3 tmp788;
    float3 tmp789;
    float tmp791;
    float tmp793;
    float tmp794;
    float tmp795;
    float3 tmp796;
    float3 tmp797;
    float tmp798;
    float phi_in799;
    float phi_out800;
    float tmp801;
    float tmp802;
    float tmp803;
    float tmp804;
    float tmp805;
    float phi_in806;
    float phi_out807;
    float phi_in808;
    float phi_out809;
    float phi_in810;
    float phi_out811;
    float tmp812;
    float tmp813;
    float tmp814;
    float tmp817;
    float tmp822;
    float tmp823;
    float tmp826;
    float tmp827;
    float tmp828;
    float3 tmp830;
    float phi_in831;
    float phi_out832;
    float phi_in833;
    float phi_out834;
    float phi_in835;
    float phi_out836;
    float tmp844;
    float tmp845;
    float tmp846;
    float tmp847;
    float phi_in848;
    float phi_out849;
    float phi_in850;
    float phi_out851;
    float tmp852;
    float tmp861;
    float tmp862;
    float tmp863;
    float tmp865;
    float phi_in877;
    float phi_out878;
    float phi_in879;
    float phi_out880;
    float phi_in881;
    float phi_out882;
    int phi_in883;
    int phi_out884;
    float tmp885;
    float tmp886;
    float tmp888;
    float tmp889;
    float tmp890;
    int phi_in891;
    int phi_out892;
    float phi_in893;
    float phi_out894;
    float phi_in895;
    float phi_out896;
    float phi_in897;
    float phi_out898;
    float3 tmp905;
    float tmp912;
    float tmp916;
    float tmp917;
    float3 tmp918;
    float tmp919;
    float tmp920;
    float tmp921;
    float3 tmp922;
    float tmp924;
    float tmp925;
    float tmp926;
    float tmp927;
    float3 tmp928;
    float tmp930;
    float tmp931;
    float tmp932;
    float tmp933;
    bool tmp934;
    float tmp935;
    float phi_in936;
    float phi_out937;
    float tmp938;
    float tmp943;
    float tmp944;
    float tmp945;
    float tmp946;
    bool tmp947;
    float tmp948;
    float tmp949;
    float tmp950;
    float tmp951;
    float tmp952;
    float tmp953;
    float tmp954;
    float tmp955;
    float tmp956;
    float3 tmp958;
    float3 tmp959;
    float3 tmp960;
    float tmp962;
    int phi_in963;
    int phi_out964;
    float tmp966;
    float tmp967;
    float tmp968;
    float3 tmp969;
    float3 tmp970;
    float tmp973;
    float tmp974;
    float tmp977;
    float tmp978;
    float tmp979;
    float3 tmp981;
    float phi_in982;
    float phi_out983;
    float phi_in984;
    float phi_out985;
    float phi_in986;
    float phi_out987;
    float tmp995;
    float tmp996;
    float tmp997;
    float tmp998;
    float phi_in999;
    float phi_out1000;
    float phi_in1001;
    float phi_out1002;
    float tmp1003;
    int phi_in1012;
    int phi_out1013;
    bool phi_in1014;
    bool phi_out1015;
    float tmp1016;
    float tmp1017;
    float tmp1018;
    float tmp1020;
    float3 tmp1025;
    float tmp1031;
    float phi_in1032;
    float phi_out1033;
    float tmp1034;
    float tmp1035;
    float tmp1036;
    float tmp1050;
    bool tmp1051;
    float tmp1052;
    float phi_in1053;
    float phi_out1054;
    float tmp1055;
    float tmp1060;
    float tmp1061;
    bool tmp1062;
    int tmp1063;
    float tmp1065;
    float tmp1066;
    float tmp1067;
    float tmp1068;
    float tmp1069;
    float tmp1070;
    float tmp1071;
    float tmp1072;
    float tmp1073;
    float3 tmp1074;
    float3 tmp1075;
    float3 tmp1076;
    float tmp1078;
    float tmp1080;
    float tmp1081;
    float tmp1082;
    float3 tmp1083;
    float3 tmp1084;
    float tmp1085;
    float tmp1086;
    float tmp1087;
    float tmp1088;
    float tmp1101;
    float tmp1102;
    float tmp1103;
    float phi_in1104;
    float phi_out1105;
    float phi_in1106;
    float phi_out1107;
    float tmp1108;
    float tmp1109;
    float tmp1110;
    float3 tmp1111;
    float tmp1113;
    float3 tmp1118;
    float tmp1123;
    float tmp1124;
    float tmp1126;
    int tmp1127;
    float tmp1142;
    float tmp1143;
    float tmp1144;
    float phi_in1145;
    float phi_out1146;
    float phi_in1147;
    float phi_out1148;
    int phi_in1149;
    int phi_out1150;
    float tmp1151;
    float tmp1152;
    float tmp1153;
    float3 tmp1154;
    float tmp1156;
    float3 tmp1161;
    float tmp1166;
    float tmp1167;
    int tmp1169;
    float tmp1170;
    float tmp1171;
    float tmp1172;
    float tmp1174;
    float tmp1175;
    float tmp1176;
    float tmp1177;
    float tmp1178;
    float phi_in1179;
    float phi_out1180;
    float phi_in1181;
    float phi_out1182;
    float phi_in1183;
    float phi_out1184;
    float phi_in1185;
    float phi_out1186;
    float phi_in1187;
    float phi_out1188;
    float tmp1190;
    float tmp1191;
    float tmp1192;
    float tmp1193;
    float tmp1194;
    float3 tmp1198;
    float tmp1199;
    float tmp1200;
    float phi_in1201;
    float phi_out1202;
    float phi_in1203;
    float phi_out1204;
    float tmp1205;
    float tmp1206;
    tmp10 = p_11.text_results[25].x;
    tmp11 = p_11.text_results[25].y;
    tmp12 = p_11.text_results[25].z;
    tmp13 = min(max(p_11.text_results[20].y, 0.0f), 1.0f);
    tmp14 = p_00.xi.z;
    tmp15 = tmp14 < tmp13;
    if (tmp15) {
        tmp16 = tmp14 / tmp13;
        phi_in = tmp16;
    } else {
        tmp17 = (tmp14 - tmp13) / (1.0f - tmp13);
        phi_in = tmp17;
    }
    phi_out = phi_in;
    p_00.xi.z = phi_out;
    if (tmp15) {
        tmp18 = p_11.text_results[30].y;
        tmp19 = p_11.text_results[30].z;
        tmp20 = p_11.text_results[30].w;
        tmp21 = mdl_read_argblock_as_bool(p_11.arg_block_offset + 236) ? p_11.text_results[19].w : 0.0f;
        tmp22 = p_11.text_results[29].z;
        tmp23 = p_11.text_results[29].w;
        tmp24 = p_11.text_results[30].x;
        tmp5 = tmp22;
        tmp6 = tmp23;
        tmp7 = tmp24;
        tmp25 = p_11.text_results[28].w;
        tmp26 = p_11.text_results[29].x;
        tmp27 = p_11.text_results[29].y;
        tmp2 = tmp25;
        tmp3 = tmp26;
        tmp4 = tmp27;
        tmp28 = p_11.text_results[17].w;
        tmp29 = p_11.text_results[17].z;
        tmp30 = max(tmp29, 1.000000012e-07f);
        tmp31 = max(tmp28, 1.000000012e-07f);
        float3 tmp32 = p_11.geom_normal;
        tmp33 = p_00.k1.x;
        tmp34 = p_00.k1.y;
        tmp35 = p_00.k1.z;
        tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36 * tmp32;
        float tmp38 = asfloat((asint(tmp37.x + tmp37.y + tmp37.z) & -2147483648) | 1065353216);
        tmp39 = tmp32.x * tmp38;
        tmp40 = tmp32.y * tmp38;
        tmp41 = tmp32.z * tmp38;
        tmp42 = tmp10 * tmp38;
        tmp43 = tmp11 * tmp38;
        tmp44 = tmp12 * tmp38;
        float3 tmp45 = p_11.text_results[23].yzw;
        tmp46 = float3(tmp42, tmp43, tmp44);
        tmp47 = tmp46.zxy;
        tmp48 = tmp46.yzx;
        tmp49 = tmp47 * tmp45.yzx - tmp48 * tmp45.zxy;
        float3 tmp50 = tmp49 * tmp49;
        tmp51 = tmp50.x + tmp50.y + tmp50.z;
        if (tmp51 < 9.999999939e-09f) {
            p_00.pdf = 0.0f;
            p_00.event_type = 0;
            phi_in52 = 0;
        } else {
            float tmp54 = 1.0f / sqrt(tmp51);
            tmp55 = tmp54 * tmp49.x;
            tmp56 = tmp54 * tmp49.y;
            tmp57 = tmp54 * tmp49.z;
            tmp58 = float3(tmp55, tmp56, tmp57);
            tmp59 = tmp58.zxy * tmp48 - tmp58.yzx * tmp47;
            if (p_00.ior1.x == -1.0f) {
                p_00.ior1.x = p_11.text_results[28].x;
                p_00.ior1.y = p_11.text_results[28].y;
                p_00.ior1.z = p_11.text_results[28].z;
            }
            if (p_00.ior2.x == -1.0f) {
                p_00.ior2.x = p_11.text_results[28].x;
                p_00.ior2.y = p_11.text_results[28].y;
                p_00.ior2.z = p_11.text_results[28].z;
            }
            float3 tmp60 = tmp46 * tmp36;
            float tmp61 = tmp60.x + tmp60.y + tmp60.z;
            float3 tmp62 = tmp59 * tmp36;
            float3 tmp63 = tmp58 * tmp36;
            tmp64 = p_00.xi.x;
            tmp65 = p_00.xi.y;
            float tmp66 = (tmp62.x + tmp62.y + tmp62.z) * tmp30;
            float tmp67 = (tmp63.x + tmp63.y + tmp63.z) * tmp31;
            tmp68 = tmp61 * tmp61;
            tmp69 = tmp67 * tmp67;
            tmp70 = tmp66 * tmp66 + tmp69;
            float3 tmp71 = float3(sqrt(tmp70 + tmp68), 0.0f, 0.0f);
            tmp72 = float3(tmp66, abs(tmp61), tmp67) / tmp71.xxx;
            phi_in73 = 1.0f;
            phi_in75 = 0.0f;
            phi_in77 = 0.0f;
            if (tmp72.y < 0.9999899864f) {
                float3 tmp79 = tmp72 * float3(1.0f, 0.0f, 0.0f);
                float3 tmp80 = tmp72 * float3(0.0f, 0.0f, 1.0f);
                float3 tmp81 = tmp79.yzx - tmp80.zxy;
                float3 tmp82 = float3(sqrt(tmp81.x * tmp81.x + tmp81.y * tmp81.y + tmp81.z * tmp81.z), 0.0f, 0.0f);
                float3 tmp83 = tmp81 / tmp82.xxx;
                phi_in73 = tmp83.x;
                phi_in75 = tmp83.y;
                phi_in77 = tmp83.z;
            }
            phi_out74 = phi_in73;
            phi_out76 = phi_in75;
            phi_out78 = phi_in77;
            float3 tmp84 = float3(phi_out74, phi_out76, phi_out78);
            float3 tmp85 = tmp84.yzx * tmp72.zxy - tmp84.zxy * tmp72.yzx;
            tmp86 = tmp72.y + 1.0f;
            tmp87 = 1.0f / tmp86;
            tmp88 = sqrt(tmp64);
            if (tmp65 < tmp87) {
                tmp89 = tmp86 * 3.141592741f * tmp65;
                phi_in90 = 1.0f;
                phi_in92 = tmp89;
            } else {
                tmp94 = (tmp65 - tmp87) * 3.141592741f / (1.0f - tmp87) + 3.141592741f;
                phi_in90 = tmp72.y;
                phi_in92 = tmp94;
            }
            phi_out91 = phi_in90;
            phi_out93 = phi_in92;
            float tmp95 = cos(phi_out93) * tmp88;
            float tmp96 = phi_out91 * tmp88 * sin(phi_out93);
            float tmp97 = sqrt(max(1.0f - (tmp96 * tmp96 + tmp95 * tmp95), 0.0f));
            float tmp98 = (tmp96 * tmp85.x + tmp95 * phi_out74 + tmp97 * tmp72.x) * tmp30;
            float tmp99 = max(tmp96 * tmp85.y + tmp95 * phi_out76 + tmp97 * tmp72.y, 0.0f);
            float tmp100 = (tmp96 * tmp85.z + tmp95 * phi_out78 + tmp97 * tmp72.z) * tmp31;
            float3 tmp101 = float3(sqrt(tmp98 * tmp98 + tmp99 * tmp99 + tmp100 * tmp100), 0.0f, 0.0f);
            float3 tmp102 = float3(tmp98, tmp99, tmp100) / tmp101.xxx;
            if (tmp102.y == 0.0f) {
                p_00.pdf = 0.0f;
                p_00.event_type = 0;
                phi_in103 = 0;
                phi_in105 = true;
            } else {
                tmp107 = tmp102.x * tmp59.x + tmp102.y * tmp42 + tmp102.z * tmp55;
                tmp108 = tmp102.x * tmp59.y + tmp102.y * tmp43 + tmp102.z * tmp56;
                tmp109 = tmp102.x * tmp59.z + tmp102.y * tmp44 + tmp102.z * tmp57;
                float3 tmp110 = float3(tmp107, tmp108, tmp109) * tmp36;
                tmp111 = tmp110.x + tmp110.y + tmp110.z;
                if (tmp111 > 0.0f) {
                    float tmp112 = tmp111 * 2.0f;
                    float tmp113 = tmp112 * tmp107 - tmp33;
                    float tmp114 = tmp112 * tmp108 - tmp34;
                    float tmp115 = tmp112 * tmp109 - tmp35;
                    p_00.k2.x = tmp113;
                    p_00.k2.y = tmp114;
                    p_00.k2.z = tmp115;
                    p_00.event_type = 10;
                    p_00.bsdf_over_pdf.x = 1.0f;
                    p_00.bsdf_over_pdf.y = 1.0f;
                    p_00.bsdf_over_pdf.z = 1.0f;
                    tmp116 = float3(tmp113, tmp114, tmp115);
                    float3 tmp117 = tmp116 * float3(tmp39, tmp40, tmp41);
                    if (tmp117.x + tmp117.y + tmp117.z > 0.0f) {
                        float3 tmp118 = tmp116 * tmp46;
                        float tmp119 = tmp118.x + tmp118.y + tmp118.z;
                        float3 tmp120 = tmp116 * tmp59;
                        float tmp121 = (tmp120.x + tmp120.y + tmp120.z) * tmp30;
                        tmp122 = 2.0f / (sqrt((tmp121 * tmp121 + tmp69) / (tmp119 * tmp119) + 1.0f) + 1.0f);
                        if (tmp122 * 2.0f / (sqrt(tmp70 / tmp68 + 1.0f) + 1.0f) > 0.0f) {
                            p_00.bsdf_over_pdf.x = tmp122;
                            p_00.bsdf_over_pdf.y = tmp122;
                            p_00.bsdf_over_pdf.z = tmp122;
                            p_00.handle = 0;
                            phi_in103 = 10;
                            phi_in105 = false;
                        } else {
                            p_00.pdf = 0.0f;
                            p_00.event_type = 0;
                            phi_in103 = 0;
                            phi_in105 = true;
                        }
                    } else {
                        p_00.pdf = 0.0f;
                        p_00.event_type = 0;
                        phi_in103 = 0;
                        phi_in105 = true;
                    }
                } else {
                    p_00.pdf = 0.0f;
                    p_00.event_type = 0;
                    phi_in103 = 0;
                    phi_in105 = true;
                }
            }
            phi_out104 = phi_in103;
            phi_out106 = phi_in105;
            phi_in123 = 0.0f;
            if (!phi_out106) {
                tmp125 = (p_00.bsdf_over_pdf.y + p_00.bsdf_over_pdf.x + p_00.bsdf_over_pdf.z) * 0.3333333433f;
                phi_in123 = tmp125;
            }
            phi_out124 = phi_in123;
            tmp126 = min(max(sqrt(tmp29 * tmp28), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
            tmp127 = p_00.xi.w;
            if (tmp127 > phi_out124) {
                float tmp128 = min(max((tmp127 - phi_out124) / (1.0f - phi_out124), 0.0f), 1.0f);
                float tmp129 = phi_out * 6.283185482f;
                float tmp130 = sin(tmp129);
                float tmp131 = cos(tmp129);
                float tmp132 = sqrt(1.0f - tmp128);
                float tmp133 = sqrt(tmp128);
                float tmp134 = tmp132 * (tmp59.x * tmp131 + tmp55 * tmp130) + tmp133 * tmp42;
                float tmp135 = tmp132 * (tmp59.y * tmp131 + tmp56 * tmp130) + tmp133 * tmp43;
                float tmp136 = tmp132 * (tmp59.z * tmp131 + tmp57 * tmp130) + tmp133 * tmp44;
                float3 tmp137 = float3(sqrt(tmp134 * tmp134 + tmp135 * tmp135 + tmp136 * tmp136), 0.0f, 0.0f);
                float3 tmp138 = float3(tmp134, tmp135, tmp136) / tmp137.xxx;
                p_00.k2.x = tmp138.x;
                p_00.k2.y = tmp138.y;
                p_00.k2.z = tmp138.z;
                p_00.event_type = 9;
                float tmp139 = (3.141592741f - tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp133, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp126, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x * 3.141592741f) * tmp133 / tex_lookup_float3_3d(1, float3(1.0f, tmp126, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                p_00.bsdf_over_pdf.x = tmp139;
                p_00.bsdf_over_pdf.y = tmp139;
                p_00.bsdf_over_pdf.z = tmp139;
                phi_in52 = 9;
            } else {
                float tmp140 = 1.0f / phi_out124;
                p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * tmp140;
                p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * tmp140;
                p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * tmp140;
                phi_in52 = phi_out104;
            }
        }
        phi_out53 = phi_in52;
        if (phi_out53 != 0) {
            float tmp141 = p_00.k2.y + tmp34;
            float tmp142 = p_00.k2.x + tmp33;
            float tmp143 = p_00.k2.z + tmp35;
            float3 tmp144 = float3(sqrt(tmp141 * tmp141 + tmp142 * tmp142 + tmp143 * tmp143), 0.0f, 0.0f);
            float3 tmp145 = float3(tmp142, tmp141, tmp143) * tmp36 / tmp144.xxx;
            tmp146 = tmp145.x + tmp145.y + tmp145.z;
            if (tmp146 < 0.0f) {
                p_00.pdf = 0.0f;
                p_00.event_type = 0;
            } else {
                tmp147 = p_00.ior1.x;
                if (tmp147 == -1.0f) {
                    tmp148 = p_11.text_results[28].x;
                    tmp149 = p_11.text_results[28].y;
                    tmp150 = p_11.text_results[28].z;
                    p_00.ior1.x = tmp148;
                    p_00.ior1.y = tmp149;
                    p_00.ior1.z = tmp150;
                    phi_in151 = tmp150;
                    phi_in153 = tmp149;
                    phi_in155 = tmp148;
                } else {
                    tmp157 = p_00.ior1.y;
                    tmp158 = p_00.ior1.z;
                    phi_in151 = tmp158;
                    phi_in153 = tmp157;
                    phi_in155 = tmp147;
                }
                phi_out152 = phi_in151;
                phi_out154 = phi_in153;
                phi_out156 = phi_in155;
                if (tmp21 > 0.0f) {
                    tmp159 = max(1.0f - tmp146 * tmp146, 0.0f);
                    tmp160 = tmp21 * 12.56637096f;
                    phi_in161 = tmp20;
                    phi_in163 = tmp27;
                    phi_in165 = tmp24;
                    phi_in167 = phi_out152;
                    phi_in169 = 409.375f;
                    phi_in171 = 0;
                    phi_in173 = 0.0f;
                    phi_in175 = 0.0f;
                    phi_in177 = 0.0f;
                    do {
                        phi_out162 = phi_in161;
                        phi_out164 = phi_in163;
                        phi_out166 = phi_in165;
                        phi_out168 = phi_in167;
                        phi_out170 = phi_in169;
                        phi_out172 = phi_in171;
                        phi_out174 = phi_in173;
                        phi_out176 = phi_in175;
                        phi_out178 = phi_in177;
                        float tmp179 = phi_out168 / phi_out162;
                        tmp180 = tmp179 * tmp179 * tmp159;
                        if (tmp180 > 1.0f) {
                            phi_in181 = phi_out162;
                            phi_in183 = phi_out168;
                            phi_in185 = phi_out170;
                            phi_in187 = phi_out172;
                            phi_in189 = phi_out174;
                            phi_in191 = phi_out176;
                            phi_in193 = phi_out178;
                            do {
                                phi_out182 = phi_in181;
                                phi_out184 = phi_in183;
                                phi_out186 = phi_in185;
                                phi_out188 = phi_in187;
                                phi_out190 = phi_in189;
                                phi_out192 = phi_in191;
                                phi_out194 = phi_in193;
                                int tmp195 = phi_out188;
                                tmp196 = glob_cnst91[tmp195].x + phi_out194;
                                tmp197 = glob_cnst91[tmp195].y + phi_out192;
                                tmp198 = glob_cnst91[tmp195].z + phi_out190;
                                tmp199 = phi_out186 + 18.75f;
                                tmp200 = phi_out188 + 1;
                                rmemload169 = tmp5;
                                rmemload173 = tmp2;
                                phi_in201 = rmemload173;
                                phi_in203 = rmemload169;
                                phi_in205 = phi_out156;
                                phi_in207 = tmp18;
                                if (!(uint(phi_out188) > 8u)) {
                                    rmemload168 = tmp7;
                                    rmemload172 = tmp4;
                                    phi_in201 = rmemload172;
                                    phi_in203 = rmemload168;
                                    phi_in205 = phi_out152;
                                    phi_in207 = tmp20;
                                    if (uint(phi_out188) > 3u) {
                                        rmemload167 = tmp6;
                                        rmemload171 = tmp3;
                                        phi_in201 = rmemload171;
                                        phi_in203 = rmemload167;
                                        phi_in205 = phi_out154;
                                        phi_in207 = tmp19;
                                    }
                                }
                                phi_out202 = phi_in201;
                                phi_out204 = phi_in203;
                                phi_out206 = phi_in205;
                                phi_out208 = phi_in207;
                                bool tmp209 = phi_out206 != phi_out184 || phi_out208 != phi_out182;
                                tmp210 = tmp209 ? phi_out206 : phi_out184;
                                tmp211 = tmp209 ? phi_out208 : phi_out182;
                                phi_in212 = phi_out202;
                                phi_in214 = phi_out204;
                                phi_in216 = tmp210;
                                phi_in218 = tmp211;
                                phi_in220 = tmp196;
                                phi_in222 = tmp197;
                                phi_in224 = tmp198;
                                phi_in226 = tmp199;
                                phi_in228 = tmp200;
                                phi_in181 = tmp211;
                                phi_in183 = tmp210;
                                phi_in185 = tmp199;
                                phi_in187 = tmp200;
                                phi_in189 = tmp198;
                                phi_in191 = tmp197;
                                phi_in193 = tmp196;
                                if (uint(phi_out188) > 14u || tmp209)
                                    break;
                            } while (true);
                            phi_out213 = phi_in212;
                            phi_out215 = phi_in214;
                            phi_out217 = phi_in216;
                            phi_out219 = phi_in218;
                            phi_out221 = phi_in220;
                            phi_out223 = phi_in222;
                            phi_out225 = phi_in224;
                            phi_out227 = phi_in226;
                            phi_out229 = phi_in228;
                            phi_in230 = phi_out221;
                            phi_in232 = phi_out223;
                            phi_in234 = phi_out225;
                            phi_in236 = phi_out229;
                            phi_in238 = phi_out227;
                            phi_in240 = phi_out217;
                            phi_in242 = phi_out215;
                            phi_in244 = phi_out213;
                            phi_in246 = phi_out219;
                        } else {
                            float tmp248 = sqrt(max(1.0f - tmp180, 0.0f));
                            float tmp249 = phi_out168 * tmp146;
                            tmp250 = tmp248 * phi_out162;
                            float tmp251 = (tmp249 - tmp250) / (tmp250 + tmp249);
                            float tmp252 = tmp248 * phi_out168;
                            float tmp253 = phi_out162 * tmp146;
                            float tmp254 = (tmp253 - tmp252) / (tmp252 + tmp253);
                            tmp255 = tmp251 * tmp251;
                            tmp256 = tmp254 * tmp254;
                            float tmp257 = phi_out166 * phi_out166;
                            float tmp258 = phi_out164 * phi_out164;
                            float tmp259 = phi_out162 * phi_out162;
                            float tmp260 = tmp258 - tmp257;
                            float tmp261 = (tmp260 - tmp180 * tmp259) * 0.5f;
                            float tmp262 = sqrt(max(tmp261 * tmp261 + tmp257 * tmp258, 0.0f));
                            tmp263 = tmp262 - tmp261;
                            tmp264 = sqrt(max(tmp262 + tmp261, 0.0f));
                            float tmp265 = sqrt(max(tmp263, 0.0f));
                            tmp266 = tmp248 * tmp260;
                            tmp267 = tmp264 * phi_out162;
                            float tmp268 = phi_out164 * 2.0f * phi_out166;
                            tmp269 = tmp248 * tmp268;
                            tmp270 = tmp265 * phi_out162;
                            float tmp271 = tmp250 * 2.0f;
                            tmp272 = tmp265 * tmp271;
                            float tmp273 = tmp262 * 2.0f;
                            tmp274 = tmp273 - tmp250 * tmp250;
                            tmp275 = (tmp264 * tmp268 - tmp265 * tmp260) * tmp271;
                            float tmp276 = tmp248 * (tmp257 + tmp258);
                            tmp277 = tmp276 * tmp276 - tmp273 * tmp259;
                            tmp278 = tmp272 * tmp272 + tmp274 * tmp274;
                            phi_in279 = 0.0f;
                            if (tmp278 > 0.0f) {
                                tmp281 = 1.0f / sqrt(tmp278);
                                phi_in279 = tmp281;
                            }
                            phi_out280 = phi_in279;
                            tmp282 = tmp275 * tmp275 + tmp277 * tmp277;
                            phi_in283 = 0.0f;
                            if (tmp282 > 0.0f) {
                                tmp285 = 1.0f / sqrt(tmp282);
                                phi_in283 = tmp285;
                            }
                            phi_out284 = phi_in283;
                            tmp286 = phi_out280 * tmp274;
                            tmp287 = phi_out284 * tmp277;
                            tmp288 = phi_out280 * tmp272;
                            tmp289 = phi_out284 * tmp275;
                            float tmp290 = tmp250 - tmp264;
                            float tmp291 = tmp264 + tmp250;
                            float tmp292 = (tmp290 * tmp290 + tmp263) / (tmp291 * tmp291 + tmp263);
                            float tmp293 = tmp266 - tmp267;
                            float tmp294 = tmp269 - tmp270;
                            float tmp295 = tmp267 + tmp266;
                            float tmp296 = tmp270 + tmp269;
                            float tmp297 = (tmp294 * tmp294 + tmp293 * tmp293) / (tmp296 * tmp296 + tmp295 * tmp295);
                            tmp298 = tmp160 * tmp250;
                            float tmp299 = max(tmp292 * tmp255, 0.0f);
                            float tmp300 = max(tmp297 * tmp256, 0.0f);
                            tmp301 = sqrt(tmp299) * 2.0f;
                            tmp302 = tmp292 + tmp255;
                            tmp303 = tmp299 + 1.0f;
                            tmp304 = sqrt(tmp300) * 2.0f;
                            tmp305 = tmp297 + tmp256;
                            tmp306 = tmp300 + 1.0f;
                            phi_in307 = phi_out178;
                            phi_in309 = phi_out176;
                            phi_in311 = phi_out174;
                            phi_in313 = phi_out172;
                            phi_in315 = phi_out170;
                            phi_in317 = phi_out168;
                            phi_in319 = phi_out166;
                            phi_in321 = phi_out164;
                            phi_in323 = phi_out162;
                            do {
                                phi_out308 = phi_in307;
                                phi_out310 = phi_in309;
                                phi_out312 = phi_in311;
                                phi_out314 = phi_in313;
                                phi_out316 = phi_in315;
                                phi_out318 = phi_in317;
                                phi_out320 = phi_in319;
                                phi_out322 = phi_in321;
                                phi_out324 = phi_in323;
                                phi_in325 = phi_out308;
                                phi_in327 = phi_out310;
                                phi_in329 = phi_out312;
                                phi_in331 = 16;
                                phi_in333 = phi_out316;
                                phi_in335 = phi_out318;
                                phi_in337 = phi_out320;
                                phi_in339 = phi_out322;
                                phi_in341 = phi_out324;
                                if (phi_out314 == 16)
                                    break;
                                else {
                                    float tmp343 = tmp298 / phi_out316;
                                    float tmp344 = sin(tmp343);
                                    float tmp345 = cos(tmp343);
                                    float tmp346 = tmp301 * (tmp286 * tmp345 - tmp288 * tmp344);
                                    float tmp347 = tmp304 * (tmp287 * tmp345 - tmp289 * tmp344);
                                    int tmp348 = phi_out314;
                                    float tmp349 = ((tmp302 + tmp346) / (tmp303 + tmp346) + (tmp305 + tmp347) / (tmp306 + tmp347)) * 0.5f;
                                    tmp350 = glob_cnst91[tmp348].x * tmp349 + phi_out308;
                                    tmp351 = glob_cnst91[tmp348].y * tmp349 + phi_out310;
                                    tmp352 = glob_cnst91[tmp348].z * tmp349 + phi_out312;
                                    tmp353 = phi_out316 + 18.75f;
                                    tmp354 = phi_out314 + 1;
                                    rmemload157 = tmp2;
                                    rmemload161 = tmp5;
                                    phi_in355 = rmemload161;
                                    phi_in357 = rmemload157;
                                    phi_in359 = tmp18;
                                    phi_in361 = phi_out156;
                                    if (!(uint(phi_out314) > 8u)) {
                                        rmemload156 = tmp4;
                                        rmemload160 = tmp7;
                                        phi_in355 = rmemload160;
                                        phi_in357 = rmemload156;
                                        phi_in359 = tmp20;
                                        phi_in361 = phi_out152;
                                        if (uint(phi_out314) > 3u) {
                                            rmemload = tmp3;
                                            rmemload159 = tmp6;
                                            phi_in355 = rmemload159;
                                            phi_in357 = rmemload;
                                            phi_in359 = tmp19;
                                            phi_in361 = phi_out154;
                                        }
                                    }
                                    phi_out356 = phi_in355;
                                    phi_out358 = phi_in357;
                                    phi_out360 = phi_in359;
                                    phi_out362 = phi_in361;
                                    tmp363 = phi_out362 != phi_out318 || (phi_out356 != phi_out320 || (phi_out360 != phi_out324 || phi_out358 != phi_out322));
                                    phi_in364 = phi_out362;
                                    phi_in366 = phi_out356;
                                    phi_in368 = phi_out358;
                                    phi_in370 = phi_out360;
                                    if (!tmp363) {
                                        phi_in364 = phi_out318;
                                        phi_in366 = phi_out320;
                                        phi_in368 = phi_out322;
                                        phi_in370 = phi_out324;
                                    }
                                    phi_out365 = phi_in364;
                                    phi_out367 = phi_in366;
                                    phi_out369 = phi_in368;
                                    phi_out371 = phi_in370;
                                    phi_in325 = tmp350;
                                    phi_in327 = tmp351;
                                    phi_in329 = tmp352;
                                    phi_in331 = tmp354;
                                    phi_in333 = tmp353;
                                    phi_in335 = phi_out365;
                                    phi_in337 = phi_out367;
                                    phi_in339 = phi_out369;
                                    phi_in341 = phi_out371;
                                    phi_in307 = tmp350;
                                    phi_in309 = tmp351;
                                    phi_in311 = tmp352;
                                    phi_in313 = tmp354;
                                    phi_in315 = tmp353;
                                    phi_in317 = phi_out365;
                                    phi_in319 = phi_out367;
                                    phi_in321 = phi_out369;
                                    phi_in323 = phi_out371;
                                    if (tmp363)
                                        break;
                                }
                            } while (true);
                            phi_out326 = phi_in325;
                            phi_out328 = phi_in327;
                            phi_out330 = phi_in329;
                            phi_out332 = phi_in331;
                            phi_out334 = phi_in333;
                            phi_out336 = phi_in335;
                            phi_out338 = phi_in337;
                            phi_out340 = phi_in339;
                            phi_out342 = phi_in341;
                            phi_in230 = phi_out326;
                            phi_in232 = phi_out328;
                            phi_in234 = phi_out330;
                            phi_in236 = phi_out332;
                            phi_in238 = phi_out334;
                            phi_in240 = phi_out336;
                            phi_in242 = phi_out338;
                            phi_in244 = phi_out340;
                            phi_in246 = phi_out342;
                        }
                        phi_out231 = phi_in230;
                        phi_out233 = phi_in232;
                        phi_out235 = phi_in234;
                        phi_out237 = phi_in236;
                        phi_out239 = phi_in238;
                        phi_out241 = phi_in240;
                        phi_out243 = phi_in242;
                        phi_out245 = phi_in244;
                        phi_out247 = phi_in246;
                        phi_in161 = phi_out247;
                        phi_in163 = phi_out245;
                        phi_in165 = phi_out243;
                        phi_in167 = phi_out241;
                        phi_in169 = phi_out239;
                        phi_in171 = phi_out237;
                        phi_in173 = phi_out235;
                        phi_in175 = phi_out233;
                        phi_in177 = phi_out231;
                        phi_in372 = phi_out231;
                        phi_in374 = phi_out233;
                        phi_in376 = phi_out235;
                        if (!(uint(phi_out237) < 16u))
                            break;
                    } while (true);
                    phi_out373 = phi_in372;
                    phi_out375 = phi_in374;
                    phi_out377 = phi_in376;
                    tmp378 = phi_out375 * -0.0389967896f + phi_out373 * 0.01064765267f + phi_out377 * 0.2020568848f;
                    tmp379 = min(max(phi_out375 * -0.2216216922f + phi_out373 * 0.467204839f + phi_out377 * -0.07188431919f, 0.0f), 1.0f);
                    tmp380 = min(max(phi_out375 * 0.3432191908f + phi_out373 * -0.1772817373f + phi_out377 * 0.007593344897f, 0.0f), 1.0f);
                    phi_in381 = tmp378;
                    phi_in383 = tmp379;
                    phi_in385 = tmp380;
                } else {
                    float tmp387 = 1.0f / phi_out156;
                    float tmp388 = 1.0f / phi_out154;
                    float tmp389 = 1.0f / phi_out152;
                    float tmp390 = tmp387 * tmp25;
                    float tmp391 = tmp388 * tmp26;
                    float tmp392 = tmp389 * tmp27;
                    float tmp393 = tmp387 * tmp22;
                    float tmp394 = tmp388 * tmp23;
                    float tmp395 = tmp389 * tmp24;
                    float tmp396 = tmp146 * tmp146;
                    float tmp397 = 1.0f - tmp396;
                    float tmp398 = -tmp397;
                    float tmp399 = tmp390 * tmp390;
                    float tmp400 = tmp393 * tmp393;
                    float tmp401 = tmp398 - tmp400 + tmp399;
                    float tmp402 = sqrt(tmp401 * tmp401 + tmp399 * 4.0f * tmp400);
                    float tmp403 = tmp402 + tmp396;
                    float tmp404 = tmp146 * 2.0f;
                    float tmp405 = sqrt(max((tmp402 + tmp401) * 0.5f, 0.0f)) * tmp404;
                    float tmp406 = (tmp403 - tmp405) / (tmp405 + tmp403);
                    float tmp407 = tmp397 * tmp397;
                    float tmp408 = tmp402 * tmp396 + tmp407;
                    float tmp409 = tmp405 * tmp397;
                    tmp410 = min(max((tmp406 * (tmp408 - tmp409) / (tmp409 + tmp408) + tmp406) * 0.5f, 0.0f), 1.0f);
                    float tmp411 = tmp391 * tmp391;
                    float tmp412 = tmp394 * tmp394;
                    float tmp413 = tmp398 - tmp412 + tmp411;
                    float tmp414 = sqrt(tmp413 * tmp413 + tmp411 * 4.0f * tmp412);
                    float tmp415 = tmp414 + tmp396;
                    float tmp416 = sqrt(max((tmp414 + tmp413) * 0.5f, 0.0f)) * tmp404;
                    float tmp417 = (tmp415 - tmp416) / (tmp416 + tmp415);
                    float tmp418 = tmp414 * tmp396 + tmp407;
                    float tmp419 = tmp416 * tmp397;
                    tmp420 = min(max((tmp417 * (tmp418 - tmp419) / (tmp419 + tmp418) + tmp417) * 0.5f, 0.0f), 1.0f);
                    float tmp421 = tmp392 * tmp392;
                    float tmp422 = tmp395 * tmp395;
                    float tmp423 = tmp398 - tmp422 + tmp421;
                    float tmp424 = sqrt(tmp423 * tmp423 + tmp421 * 4.0f * tmp422);
                    float tmp425 = tmp424 + tmp396;
                    float tmp426 = sqrt(max((tmp424 + tmp423) * 0.5f, 0.0f)) * tmp404;
                    float tmp427 = (tmp425 - tmp426) / (tmp426 + tmp425);
                    float tmp428 = tmp424 * tmp396 + tmp407;
                    float tmp429 = tmp426 * tmp397;
                    tmp430 = (tmp427 * (tmp428 - tmp429) / (tmp429 + tmp428) + tmp427) * 0.5f;
                    phi_in381 = tmp430;
                    phi_in383 = tmp410;
                    phi_in385 = tmp420;
                }
                phi_out382 = phi_in381;
                phi_out384 = phi_in383;
                phi_out386 = phi_in385;
                p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * phi_out384;
                p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * phi_out386;
                p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * min(max(phi_out382, 0.0f), 1.0f);
            }
        }
    } else {
        tmp431 = min(max(p_11.text_results[12].x, 0.0f), 1.0f);
        tmp432 = phi_out < tmp431;
        if (tmp432) {
            tmp433 = phi_out / tmp431;
            phi_in434 = tmp433;
        } else {
            tmp436 = (phi_out - tmp431) / (1.0f - tmp431);
            phi_in434 = tmp436;
        }
        phi_out435 = phi_in434;
        p_00.xi.z = phi_out435;
        if (tmp432) {
            tmp437 = min(max(p_11.text_results[20].x, 0.0f), 1.0f);
            tmp438 = phi_out435 < tmp437;
            if (tmp438) {
                tmp439 = phi_out435 / tmp437;
                phi_in440 = tmp439;
            } else {
                tmp442 = (phi_out435 - tmp437) / (1.0f - tmp437);
                phi_in440 = tmp442;
            }
            phi_out441 = phi_in440;
            p_00.xi.z = phi_out441;
            if (tmp438) {
                tmp443 = p_11.arg_block_offset;
                tmp444 = mdl_read_argblock_as_bool(tmp443);
                phi_in445 = true;
                if (!tmp444) {
                    phi_in445 = true;
                    if (!(p_11.text_results[12].y == 0.0f))
                        phi_in445 = false;
                }
                phi_out446 = phi_in445;
                tmp447 = phi_out446 ? float3(p_11.text_results[12].z, p_11.text_results[12].w, p_11.text_results[13].x) : float3(1.0f, 1.0f, 1.0f);
                tmp448 = p_11.text_results[26].z;
                tmp449 = p_11.text_results[26].w;
                tmp450 = p_11.text_results[27].x;
                float3 tmp451 = float3(tmp448, tmp449, tmp450) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                tmp452 = p_11.text_results[17].w;
                tmp453 = p_11.text_results[17].z;
                tmp454 = max(tmp453, 1.000000012e-07f);
                tmp455 = max(tmp452, 1.000000012e-07f);
                tmp9.m_0 = (p_11.text_results[30].z + p_11.text_results[30].y + p_11.text_results[30].w) * 0.3333333433f;
                tmp9.m_1 = mdl_read_argblock_as_bool(tmp443 + 236) ? p_11.text_results[19].w : 0.0f;
                tmp456 = tmp451.z > 0.0f || (tmp451.x > 0.0f || tmp451.y > 0.0f);
                tmp457 = min(max(tmp451.x, 0.0f), 1.0f);
                tmp458 = min(max(tmp451.y, 0.0f), 1.0f);
                tmp459 = min(max(tmp451.z, 0.0f), 1.0f);
                float3 tmp460 = p_11.geom_normal;
                tmp461 = p_00.k1.x;
                tmp462 = p_00.k1.y;
                tmp463 = p_00.k1.z;
                tmp464 = float3(tmp461, tmp462, tmp463);
                float3 tmp465 = tmp464 * tmp460;
                float tmp466 = asfloat((asint(tmp465.x + tmp465.y + tmp465.z) & -2147483648) | 1065353216);
                tmp467 = tmp460.x * tmp466;
                tmp468 = tmp460.y * tmp466;
                tmp469 = tmp460.z * tmp466;
                tmp470 = tmp10 * tmp466;
                tmp471 = tmp11 * tmp466;
                tmp472 = tmp12 * tmp466;
                float3 tmp473 = p_11.text_results[23].yzw;
                tmp474 = float3(tmp470, tmp471, tmp472);
                tmp475 = tmp474.zxy;
                tmp476 = tmp474.yzx;
                tmp477 = tmp475 * tmp473.yzx - tmp476 * tmp473.zxy;
                float3 tmp478 = tmp477 * tmp477;
                tmp479 = tmp478.x + tmp478.y + tmp478.z;
                if (tmp479 < 9.999999939e-09f) {
                    p_00.pdf = 0.0f;
                    p_00.event_type = 0;
                    phi_in480 = 0;
                } else {
                    float tmp482 = 1.0f / sqrt(tmp479);
                    tmp483 = tmp482 * tmp477.x;
                    tmp484 = tmp482 * tmp477.y;
                    tmp485 = tmp482 * tmp477.z;
                    tmp486 = float3(tmp483, tmp484, tmp485);
                    tmp487 = tmp486.zxy * tmp476 - tmp486.yzx * tmp475;
                    tmp488 = p_00.ior1.x;
                    phi_in489 = tmp488;
                    if (tmp488 == -1.0f) {
                        tmp491 = p_11.text_results[28].x;
                        p_00.ior1.x = tmp491;
                        p_00.ior1.y = p_11.text_results[28].y;
                        p_00.ior1.z = p_11.text_results[28].z;
                        phi_in489 = tmp491;
                    }
                    phi_out490 = phi_in489;
                    tmp492 = p_00.ior2.x;
                    if (tmp492 == -1.0f) {
                        tmp493 = p_11.text_results[28].x;
                        tmp494 = p_11.text_results[28].y;
                        tmp495 = p_11.text_results[28].z;
                        p_00.ior2.x = tmp493;
                        p_00.ior2.y = tmp494;
                        p_00.ior2.z = tmp495;
                        phi_in496 = tmp495;
                        phi_in498 = tmp494;
                        phi_in500 = tmp493;
                    } else {
                        tmp502 = p_00.ior2.y;
                        tmp503 = p_00.ior2.z;
                        phi_in496 = tmp503;
                        phi_in498 = tmp502;
                        phi_in500 = tmp492;
                    }
                    phi_out497 = phi_in496;
                    phi_out499 = phi_in498;
                    phi_out501 = phi_in500;
                    tmp504 = (p_00.ior1.y + phi_out490 + p_00.ior1.z) * 0.3333333433f;
                    float tmp505 = (phi_out499 + phi_out497 + phi_out501) * 0.3333333433f;
                    float tmp506 = tmp505 - tmp504;
                    tmp507 = abs(tmp506) < 9.999999747e-05f ? tmp504 + asfloat((asint(tmp506) & -2147483648) | 953267991) : tmp505;
                    tmp8.m_0 = tmp504;
                    tmp8.m_1 = tmp507;
                    float3 tmp508 = tmp474 * tmp464;
                    float tmp509 = tmp508.x + tmp508.y + tmp508.z;
                    float3 tmp510 = tmp487 * tmp464;
                    float3 tmp511 = tmp486 * tmp464;
                    tmp512 = p_00.xi.x;
                    tmp513 = p_00.xi.y;
                    float tmp514 = (tmp510.x + tmp510.y + tmp510.z) * tmp454;
                    float tmp515 = (tmp511.x + tmp511.y + tmp511.z) * tmp455;
                    tmp516 = tmp509 * tmp509;
                    tmp517 = tmp515 * tmp515;
                    tmp518 = tmp514 * tmp514 + tmp517;
                    float3 tmp519 = float3(sqrt(tmp518 + tmp516), 0.0f, 0.0f);
                    tmp520 = float3(tmp514, abs(tmp509), tmp515) / tmp519.xxx;
                    phi_in521 = 1.0f;
                    phi_in523 = 0.0f;
                    phi_in525 = 0.0f;
                    if (tmp520.y < 0.9999899864f) {
                        float3 tmp527 = tmp520 * float3(1.0f, 0.0f, 0.0f);
                        float3 tmp528 = tmp520 * float3(0.0f, 0.0f, 1.0f);
                        float3 tmp529 = tmp527.yzx - tmp528.zxy;
                        float3 tmp530 = float3(sqrt(tmp529.x * tmp529.x + tmp529.y * tmp529.y + tmp529.z * tmp529.z), 0.0f, 0.0f);
                        float3 tmp531 = tmp529 / tmp530.xxx;
                        phi_in521 = tmp531.x;
                        phi_in523 = tmp531.y;
                        phi_in525 = tmp531.z;
                    }
                    phi_out522 = phi_in521;
                    phi_out524 = phi_in523;
                    phi_out526 = phi_in525;
                    float3 tmp532 = float3(phi_out522, phi_out524, phi_out526);
                    float3 tmp533 = tmp532.yzx * tmp520.zxy - tmp532.zxy * tmp520.yzx;
                    tmp534 = tmp520.y + 1.0f;
                    tmp535 = 1.0f / tmp534;
                    tmp536 = sqrt(tmp512);
                    if (tmp513 < tmp535) {
                        tmp537 = tmp534 * 3.141592741f * tmp513;
                        phi_in538 = 1.0f;
                        phi_in540 = tmp537;
                    } else {
                        tmp542 = (tmp513 - tmp535) * 3.141592741f / (1.0f - tmp535) + 3.141592741f;
                        phi_in538 = tmp520.y;
                        phi_in540 = tmp542;
                    }
                    phi_out539 = phi_in538;
                    phi_out541 = phi_in540;
                    float tmp543 = cos(phi_out541) * tmp536;
                    float tmp544 = phi_out539 * tmp536 * sin(phi_out541);
                    float tmp545 = sqrt(max(1.0f - (tmp544 * tmp544 + tmp543 * tmp543), 0.0f));
                    float tmp546 = (tmp544 * tmp533.x + tmp543 * phi_out522 + tmp545 * tmp520.x) * tmp454;
                    float tmp547 = max(tmp544 * tmp533.y + tmp543 * phi_out524 + tmp545 * tmp520.y, 0.0f);
                    float tmp548 = (tmp544 * tmp533.z + tmp543 * phi_out526 + tmp545 * tmp520.z) * tmp455;
                    float3 tmp549 = float3(sqrt(tmp546 * tmp546 + tmp547 * tmp547 + tmp548 * tmp548), 0.0f, 0.0f);
                    float3 tmp550 = float3(tmp546, tmp547, tmp548) / tmp549.xxx;
                    if (tmp550.y == 0.0f) {
                        p_00.pdf = 0.0f;
                        p_00.event_type = 0;
                        phi_in551 = 0;
                    } else {
                        tmp553 = tmp550.x * tmp487.x + tmp550.y * tmp470 + tmp550.z * tmp483;
                        tmp554 = tmp550.x * tmp487.y + tmp550.y * tmp471 + tmp550.z * tmp484;
                        tmp555 = tmp550.x * tmp487.z + tmp550.y * tmp472 + tmp550.z * tmp485;
                        float3 tmp556 = float3(tmp553, tmp554, tmp555) * tmp464;
                        tmp557 = tmp556.x + tmp556.y + tmp556.z;
                        if (tmp557 > 0.0f) {
                            structtype1 tmp558 = _ZNK23Fresnel_function_coated4evalERK6float2f(tmp9, tmp8, tmp557);
                            tmp559 = tmp558.m_0;
                            tmp560 = tmp558.m_1;
                            tmp561 = tmp558.m_2;
                            tmp562 = tmp558.m_3;
                            if (phi_out441 < tmp562) {
                                float tmp563 = tmp557 * 2.0f;
                                tmp564 = tmp563 * tmp553 - tmp461;
                                tmp565 = tmp563 * tmp554 - tmp462;
                                tmp566 = tmp563 * tmp555 - tmp463;
                                p_00.k2.x = tmp564;
                                p_00.k2.y = tmp565;
                                p_00.k2.z = tmp566;
                                p_00.event_type = 10;
                                p_00.xi.z = phi_out441 / tmp562;
                                float tmp567 = 1.0f / tmp562;
                                tmp568 = tmp567 * tmp559;
                                tmp569 = tmp567 * tmp560;
                                tmp570 = tmp567 * tmp561;
                                phi_in571 = 10;
                                phi_in573 = tmp566;
                                phi_in575 = tmp565;
                                phi_in577 = tmp564;
                                phi_in579 = tmp568;
                                phi_in581 = tmp569;
                                phi_in583 = tmp570;
                            } else {
                                tmp585 = 1.0f - tmp562;
                                if (tmp444) {
                                    float tmp586 = tmp557 * 2.0f;
                                    float tmp587 = tmp586 * tmp553 - tmp461;
                                    float tmp588 = tmp586 * tmp554 - tmp462;
                                    float tmp589 = tmp586 * tmp555 - tmp463;
                                    float3 tmp590 = float3(tmp587, tmp588, tmp589) * tmp474;
                                    float tmp591 = (tmp590.x + tmp590.y + tmp590.z) * 2.0f;
                                    float tmp592 = tmp587 - tmp591 * tmp470;
                                    float tmp593 = tmp588 - tmp591 * tmp471;
                                    float tmp594 = tmp589 - tmp591 * tmp472;
                                    float3 tmp595 = float3(sqrt(tmp592 * tmp592 + tmp593 * tmp593 + tmp594 * tmp594), 0.0f, 0.0f);
                                    float3 tmp596 = float3(tmp592, tmp593, tmp594) / tmp595.xxx;
                                    p_00.k2.x = tmp596.x;
                                    p_00.k2.y = tmp596.y;
                                    p_00.k2.z = tmp596.z;
                                    phi_in597 = tmp596.z;
                                    phi_in599 = tmp596.y;
                                    phi_in601 = tmp596.x;
                                    phi_in603 = 18;
                                } else {
                                    tmp605 = tmp504 / tmp507;
                                    tmp606 = tmp605 * tmp605 * (1.0f - tmp557 * tmp557);
                                    if (tmp606 > 1.0f) {
                                        float tmp607 = tmp557 * 2.0f;
                                        tmp608 = tmp607 * tmp553 - tmp461;
                                        tmp609 = tmp607 * tmp554 - tmp462;
                                        tmp610 = tmp607 * tmp555 - tmp463;
                                        phi_in611 = 10;
                                        phi_in613 = tmp608;
                                        phi_in615 = tmp609;
                                        phi_in617 = tmp610;
                                    } else {
                                        float tmp619 = tmp557 * tmp605 - sqrt(1.0f - tmp606);
                                        float tmp620 = tmp619 * tmp553 - tmp605 * tmp461;
                                        float tmp621 = tmp619 * tmp554 - tmp605 * tmp462;
                                        float tmp622 = tmp619 * tmp555 - tmp605 * tmp463;
                                        float3 tmp623 = float3(sqrt(tmp620 * tmp620 + tmp621 * tmp621 + tmp622 * tmp622), 0.0f, 0.0f);
                                        float3 tmp624 = float3(tmp620, tmp621, tmp622) / tmp623.xxx;
                                        phi_in611 = 18;
                                        phi_in613 = tmp624.x;
                                        phi_in615 = tmp624.y;
                                        phi_in617 = tmp624.z;
                                    }
                                    phi_out612 = phi_in611;
                                    phi_out614 = phi_in613;
                                    phi_out616 = phi_in615;
                                    phi_out618 = phi_in617;
                                    p_00.k2.x = phi_out614;
                                    p_00.k2.y = phi_out616;
                                    p_00.k2.z = phi_out618;
                                    phi_in597 = phi_out618;
                                    phi_in599 = phi_out616;
                                    phi_in601 = phi_out614;
                                    phi_in603 = phi_out612;
                                }
                                phi_out598 = phi_in597;
                                phi_out600 = phi_in599;
                                phi_out602 = phi_in601;
                                phi_out604 = phi_in603;
                                p_00.event_type = phi_out604;
                                p_00.xi.z = (phi_out441 - tmp562) / tmp585;
                                float tmp625 = 1.0f / tmp585;
                                tmp626 = tmp625 * (1.0f - tmp559);
                                tmp627 = tmp625 * (1.0f - tmp560);
                                tmp628 = tmp625 * (1.0f - tmp561);
                                phi_in571 = phi_out604;
                                phi_in573 = phi_out598;
                                phi_in575 = phi_out600;
                                phi_in577 = phi_out602;
                                phi_in579 = tmp626;
                                phi_in581 = tmp627;
                                phi_in583 = tmp628;
                            }
                            phi_out572 = phi_in571;
                            phi_out574 = phi_in573;
                            phi_out576 = phi_in575;
                            phi_out578 = phi_in577;
                            phi_out580 = phi_in579;
                            phi_out582 = phi_in581;
                            phi_out584 = phi_in583;
                            p_00.bsdf_over_pdf.x = phi_out580;
                            p_00.bsdf_over_pdf.y = phi_out582;
                            p_00.bsdf_over_pdf.z = phi_out584;
                            tmp629 = float3(phi_out578, phi_out576, phi_out574);
                            float3 tmp630 = tmp629 * float3(tmp467, tmp468, tmp469);
                            float tmp631 = tmp630.x + tmp630.y + tmp630.z;
                            if ((phi_out572 == 10 ? tmp631 : -tmp631) > 0.0f) {
                                float3 tmp632 = tmp629 * tmp474;
                                float tmp633 = tmp632.x + tmp632.y + tmp632.z;
                                float3 tmp634 = tmp629 * tmp487;
                                float tmp635 = (tmp634.x + tmp634.y + tmp634.z) * tmp454;
                                tmp636 = 2.0f / (sqrt((tmp635 * tmp635 + tmp517) / (tmp633 * tmp633) + 1.0f) + 1.0f);
                                if (tmp636 * 2.0f / (sqrt(tmp518 / tmp516 + 1.0f) + 1.0f) > 0.0f) {
                                    p_00.bsdf_over_pdf.x = tmp636 * phi_out580;
                                    p_00.bsdf_over_pdf.y = tmp636 * phi_out582;
                                    p_00.bsdf_over_pdf.z = tmp636 * phi_out584;
                                    p_00.handle = 0;
                                    phi_in551 = phi_out572;
                                } else {
                                    p_00.pdf = 0.0f;
                                    p_00.event_type = 0;
                                    phi_in551 = 0;
                                }
                            } else {
                                p_00.pdf = 0.0f;
                                p_00.event_type = 0;
                                phi_in551 = 0;
                            }
                        } else {
                            p_00.pdf = 0.0f;
                            p_00.event_type = 0;
                            phi_in551 = 0;
                        }
                    }
                    phi_out552 = phi_in551;
                    phi_in637 = phi_out552;
                    if (tmp456) {
                        tmp639 = tmp444 ? -1.0f : tmp504 / tmp507;
                        phi_in640 = 0.0f;
                        if (phi_out552 != 0) {
                            tmp642 = (p_00.bsdf_over_pdf.y + p_00.bsdf_over_pdf.x + p_00.bsdf_over_pdf.z) * 0.3333333433f;
                            phi_in640 = tmp642;
                        }
                        phi_out641 = phi_in640;
                        tmp643 = min(max(sqrt(tmp453 * tmp452), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
                        phi_in644 = 0.0f;
                        if (!(tmp639 < 0.0f)) {
                            bool tmp646 = tmp639 < 1.0f;
                            tmp647 = (min(max((tmp646 ? 1.0f / tmp639 : tmp639) * 0.5333333611f + -0.6000000238f, 0.0f), 1.0f) * 15.0f + (tmp646 ? 1.5f : 17.5f)) * 0.03030303121f;
                            phi_in644 = tmp647;
                        }
                        phi_out645 = phi_in644;
                        tmp648 = p_00.xi.w;
                        if (tmp648 > phi_out641) {
                            float tmp649 = min(max((tmp648 - phi_out641) / (1.0f - phi_out641), 0.0f), 1.0f);
                            float tmp650 = p_00.xi.z * 6.283185482f;
                            float tmp651 = sin(tmp650);
                            float tmp652 = cos(tmp650);
                            float tmp653 = sqrt(1.0f - tmp649);
                            float tmp654 = sqrt(tmp649);
                            float tmp655 = (tmp651 * tmp483 + tmp652 * tmp487.x) * tmp653 + tmp654 * tmp470;
                            float tmp656 = (tmp651 * tmp484 + tmp652 * tmp487.y) * tmp653 + tmp654 * tmp471;
                            float tmp657 = (tmp651 * tmp485 + tmp652 * tmp487.z) * tmp653 + tmp654 * tmp472;
                            float3 tmp658 = float3(sqrt(tmp655 * tmp655 + tmp656 * tmp656 + tmp657 * tmp657), 0.0f, 0.0f);
                            float3 tmp659 = float3(tmp655, tmp656, tmp657) / tmp658.xxx;
                            p_00.k2.x = tmp659.x;
                            p_00.k2.y = tmp659.y;
                            p_00.k2.z = tmp659.z;
                            p_00.event_type = 9;
                            float tmp660 = (3.141592741f - tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp654, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp643, phi_out645), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x * 3.141592741f) * tmp654 / tex_lookup_float3_3d(1, float3(1.0f, tmp643, phi_out645), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                            p_00.bsdf_over_pdf.x = tmp660 * tmp457;
                            p_00.bsdf_over_pdf.y = tmp660 * tmp458;
                            p_00.bsdf_over_pdf.z = tmp660 * tmp459;
                            phi_in637 = 9;
                        } else {
                            float tmp661 = 1.0f / phi_out641;
                            p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * tmp661;
                            p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * tmp661;
                            p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * tmp661;
                            phi_in637 = phi_out552;
                        }
                    }
                    phi_out638 = phi_in637;
                    phi_in480 = phi_out638;
                }
                phi_out481 = phi_in480;
                if (phi_out481 != 0) {
                    bool tmp662 = (phi_out481 & 16) == 0;
                    p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * min(max(tmp662 ? tmp448 : tmp447.x, 0.0f), 1.0f);
                    p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * min(max(tmp662 ? tmp449 : tmp447.y, 0.0f), 1.0f);
                    p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * min(max(tmp662 ? tmp450 : tmp447.z, 0.0f), 1.0f);
                }
            } else {
                float3 tmp663 = p_11.normal;
                float tmp664 = p_11.text_results[17].y;
                tmp665 = min(max(tmp664 > 0.349999994f ? tmp664 + -0.349999994f : 0.0f, 0.0f), 1.0f);
                tmp666 = phi_out441 < tmp665;
                if (tmp666) {
                    tmp667 = phi_out441 / tmp665;
                    phi_in668 = tmp667;
                } else {
                    tmp670 = (phi_out441 - tmp665) / (1.0f - tmp665);
                    phi_in668 = tmp670;
                }
                phi_out669 = phi_in668;
                p_00.xi.z = phi_out669;
                if (tmp666) {
                    phi_in671 = true;
                    if (!mdl_read_argblock_as_bool(p_11.arg_block_offset)) {
                        phi_in671 = true;
                        if (!(p_11.text_results[12].y == 0.0f))
                            phi_in671 = false;
                    }
                    phi_out672 = phi_in671;
                    tmp673 = phi_out672 ? float3(p_11.text_results[12].z, p_11.text_results[12].w, p_11.text_results[13].x) : float3(1.0f, 1.0f, 1.0f);
                    float3 tmp674 = p_11.tangent_u[0];
                    tmp675 = p_11.geom_normal;
                    float3 tmp676 = p_00.k1 * tmp675;
                    tmp677 = asfloat((asint(tmp676.x + tmp676.y + tmp676.z) & -2147483648) | 1065353216);
                    tmp678 = tmp663.x * tmp677;
                    tmp679 = tmp663.y * tmp677;
                    tmp680 = tmp663.z * tmp677;
                    tmp681 = float3(tmp678, tmp679, tmp680);
                    tmp682 = tmp681.zxy;
                    tmp683 = tmp681.yzx;
                    tmp684 = tmp682 * tmp674.yzx - tmp683 * tmp674.zxy;
                    float3 tmp685 = tmp684 * tmp684;
                    tmp686 = tmp685.x + tmp685.y + tmp685.z;
                    if (tmp686 < 9.999999939e-09f) {
                        p_00.pdf = 0.0f;
                        p_00.event_type = 0;
                    } else {
                        float tmp687 = 1.0f / sqrt(tmp686);
                        tmp688 = tmp687 * tmp684.x;
                        tmp689 = tmp687 * tmp684.y;
                        tmp690 = tmp687 * tmp684.z;
                        float3 tmp691 = float3(tmp688, tmp689, tmp690);
                        float3 tmp692 = tmp691.zxy * tmp683 - tmp691.yzx * tmp682;
                        tmp693 = min(max(tmp673.x, 0.0f), 1.0f);
                        tmp694 = min(max(tmp673.y, 0.0f), 1.0f);
                        tmp695 = min(max(tmp673.z, 0.0f), 1.0f);
                        tmp696 = float3(tmp675.x * tmp677, tmp675.y * tmp677, tmp675.z * tmp677);
                        float3 tmp697 = tmp696 * tmp681;
                        tmp698 = max(tmp697.x + tmp697.y + tmp697.z, 0.0f);
                        tmp699 = 3.141592741f / (tmp698 * 1.570796371f + 1.570796371f);
                        tmp700 = tmp699 + -1.0f;
                        tmp701 = p_00.xi.x;
                        tmp702 = tmp701 < tmp700;
                        if (tmp702) {
                            tmp703 = tmp701 / tmp700;
                            phi_in704 = tmp703;
                        } else {
                            tmp706 = (tmp701 - tmp700) / (2.0f - tmp699);
                            phi_in704 = tmp706;
                        }
                        phi_out705 = phi_in704;
                        p_00.xi.x = phi_out705;
                        tmp707 = p_00.xi.y;
                        phi_in708 = float3(0.0f, 1.0f, 0.0f);
                        if (!(phi_out705 == 0.0f && tmp707 == 0.0f)) {
                            float tmp710 = phi_out705 * 2.0f;
                            float tmp711 = tmp707 * 2.0f;
                            tmp712 = tmp710 < 1.0f ? tmp710 : tmp710 + -2.0f;
                            tmp713 = tmp711 < 1.0f ? tmp711 : tmp711 + -2.0f;
                            tmp714 = tmp712 * tmp712;
                            tmp715 = tmp713 * tmp713;
                            if (tmp714 > tmp715) {
                                tmp716 = tmp713 * -0.7853981853f / tmp712;
                                phi_in717 = tmp712;
                                phi_in719 = tmp716;
                                phi_in721 = tmp714;
                            } else {
                                tmp723 = tmp712 * 0.7853981853f / tmp713 + -1.570796371f;
                                phi_in717 = tmp713;
                                phi_in719 = tmp723;
                                phi_in721 = tmp715;
                            }
                            phi_out718 = phi_in717;
                            phi_out720 = phi_in719;
                            phi_out722 = phi_in721;
                            tmp724 = 1.0f - phi_out722;
                            phi_in708 = float3(0.0f, 1.0f, 0.0f);
                            if (tmp724 > 0.0f) {
                                tmp725 = float3(sin(phi_out720) * phi_out718, sqrt(tmp724), cos(phi_out720) * phi_out718);
                                phi_in708 = tmp725;
                            }
                        }
                        phi_out709 = phi_in708;
                        tmp726 = phi_out709.x;
                        tmp727 = phi_out709.y;
                        tmp728 = phi_out709.z;
                        phi_in729 = tmp692.x;
                        phi_in731 = tmp692.y;
                        phi_in733 = tmp692.z;
                        phi_in735 = tmp688;
                        phi_in737 = tmp689;
                        phi_in739 = tmp690;
                        if (tmp698 < 0.9990000129f) {
                            float3 tmp741 = tmp682 * tmp696.yzx - tmp683 * tmp696.zxy;
                            float3 tmp742 = float3(sqrt(tmp741.x * tmp741.x + tmp741.y * tmp741.y + tmp741.z * tmp741.z), 0.0f, 0.0f);
                            float3 tmp743 = tmp741 / tmp742.xxx;
                            float3 tmp744 = tmp743.yzx * tmp682 - tmp743.zxy * tmp683;
                            phi_in729 = tmp743.x;
                            phi_in731 = tmp743.y;
                            phi_in733 = tmp743.z;
                            phi_in735 = tmp744.x;
                            phi_in737 = tmp744.y;
                            phi_in739 = tmp744.z;
                        }
                        phi_out730 = phi_in729;
                        phi_out732 = phi_in731;
                        phi_out734 = phi_in733;
                        phi_out736 = phi_in735;
                        phi_out738 = phi_in737;
                        phi_out740 = phi_in739;
                        phi_in745 = tmp727;
                        phi_in747 = tmp728;
                        if (tmp728 > 0.0f) {
                            if (tmp702) {
                                tmp749 = -tmp728;
                                phi_in745 = tmp727;
                                phi_in747 = tmp749;
                            } else {
                                tmp750 = tmp728 * tmp698;
                                tmp751 = sqrt(max(1.0f - (tmp750 * tmp750 + tmp726 * tmp726), 0.0f));
                                phi_in745 = tmp751;
                                phi_in747 = tmp750;
                            }
                        }
                        phi_out746 = phi_in745;
                        phi_out748 = phi_in747;
                        float tmp752 = phi_out730 * tmp726 - (phi_out746 * tmp678 + phi_out748 * phi_out736);
                        float tmp753 = phi_out732 * tmp726 - (phi_out746 * tmp679 + phi_out748 * phi_out738);
                        float tmp754 = phi_out734 * tmp726 - (phi_out746 * tmp680 + phi_out748 * phi_out740);
                        float3 tmp755 = float3(sqrt(tmp752 * tmp752 + tmp753 * tmp753 + tmp754 * tmp754), 0.0f, 0.0f);
                        float3 tmp756 = float3(tmp752, tmp753, tmp754) / tmp755.xxx;
                        p_00.k2.x = tmp756.x;
                        p_00.k2.y = tmp756.y;
                        p_00.k2.z = tmp756.z;
                        float3 tmp757 = tmp756 * tmp696;
                        if (tmp757.x + tmp757.y + tmp757.z > 0.0f) {
                            p_00.pdf = 0.0f;
                            phi_in758 = 0;
                        } else {
                            p_00.bsdf_over_pdf.x = tmp693;
                            p_00.bsdf_over_pdf.y = tmp694;
                            p_00.bsdf_over_pdf.z = tmp695;
                            p_00.handle = 0;
                            phi_in758 = 17;
                        }
                        phi_out759 = phi_in758;
                        p_00.event_type = phi_out759;
                    }
                } else {
                    tmp760 = p_11.text_results[23].y;
                    tmp761 = p_11.text_results[23].z;
                    tmp762 = p_11.text_results[23].w;
                    tmp763 = mdl_read_argblock_as_bool(p_11.arg_block_offset);
                    phi_in764 = true;
                    if (!tmp763) {
                        phi_in764 = true;
                        if (!(p_11.text_results[12].y == 0.0f))
                            phi_in764 = false;
                    }
                    phi_out765 = phi_in764;
                    float3 tmp766 = phi_out765 ? float3(p_11.text_results[12].z, p_11.text_results[12].w, p_11.text_results[13].x) : float3(1.0f, 1.0f, 1.0f);
                    tmp767 = max(p_11.text_results[17].z, 1.000000012e-07f);
                    tmp768 = max(p_11.text_results[17].w, 1.000000012e-07f);
                    tmp769 = min(max(tmp766.x, 0.0f), 1.0f);
                    tmp770 = min(max(tmp766.y, 0.0f), 1.0f);
                    tmp771 = min(max(tmp766.z, 0.0f), 1.0f);
                    float3 tmp772 = p_11.geom_normal;
                    tmp773 = p_00.k1.x;
                    tmp774 = p_00.k1.y;
                    tmp775 = p_00.k1.z;
                    tmp776 = float3(tmp773, tmp774, tmp775);
                    float3 tmp777 = tmp776 * tmp772;
                    float tmp778 = asfloat((asint(tmp777.x + tmp777.y + tmp777.z) & -2147483648) | 1065353216);
                    tmp779 = tmp772.x * tmp778;
                    tmp780 = tmp772.y * tmp778;
                    tmp781 = tmp772.z * tmp778;
                    tmp782 = tmp10 * tmp778;
                    tmp783 = tmp11 * tmp778;
                    tmp784 = tmp12 * tmp778;
                    float3 tmp785 = float3(tmp760, tmp761, tmp762);
                    tmp786 = float3(tmp782, tmp783, tmp784);
                    tmp787 = tmp786.zxy;
                    tmp788 = tmp786.yzx;
                    tmp789 = tmp787 * tmp785.yzx - tmp788 * tmp785.zxy;
                    float3 tmp790 = tmp789 * tmp789;
                    tmp791 = tmp790.x + tmp790.y + tmp790.z;
                    if (tmp791 < 9.999999939e-09f) {
                        p_00.pdf = 0.0f;
                        p_00.event_type = 0;
                    } else {
                        float tmp792 = 1.0f / sqrt(tmp791);
                        tmp793 = tmp792 * tmp789.x;
                        tmp794 = tmp792 * tmp789.y;
                        tmp795 = tmp792 * tmp789.z;
                        tmp796 = float3(tmp793, tmp794, tmp795);
                        tmp797 = tmp796.zxy * tmp788 - tmp796.yzx * tmp787;
                        tmp798 = p_00.ior1.x;
                        phi_in799 = tmp798;
                        if (tmp798 == -1.0f) {
                            tmp801 = p_11.text_results[28].x;
                            p_00.ior1.x = tmp801;
                            p_00.ior1.y = p_11.text_results[28].y;
                            p_00.ior1.z = p_11.text_results[28].z;
                            phi_in799 = tmp801;
                        }
                        phi_out800 = phi_in799;
                        tmp802 = p_00.ior2.x;
                        if (tmp802 == -1.0f) {
                            tmp803 = p_11.text_results[28].x;
                            tmp804 = p_11.text_results[28].y;
                            tmp805 = p_11.text_results[28].z;
                            p_00.ior2.x = tmp803;
                            p_00.ior2.y = tmp804;
                            p_00.ior2.z = tmp805;
                            phi_in806 = tmp805;
                            phi_in808 = tmp804;
                            phi_in810 = tmp803;
                        } else {
                            tmp812 = p_00.ior2.y;
                            tmp813 = p_00.ior2.z;
                            phi_in806 = tmp813;
                            phi_in808 = tmp812;
                            phi_in810 = tmp802;
                        }
                        phi_out807 = phi_in806;
                        phi_out809 = phi_in808;
                        phi_out811 = phi_in810;
                        tmp814 = (p_00.ior1.y + phi_out800 + p_00.ior1.z) * 0.3333333433f;
                        float tmp815 = (phi_out809 + phi_out807 + phi_out811) * 0.3333333433f;
                        float tmp816 = tmp815 - tmp814;
                        tmp817 = abs(tmp816) < 9.999999747e-05f ? tmp814 + asfloat((asint(tmp816) & -2147483648) | 953267991) : tmp815;
                        float3 tmp818 = tmp786 * tmp776;
                        float tmp819 = tmp818.x + tmp818.y + tmp818.z;
                        float3 tmp820 = tmp797 * tmp776;
                        float3 tmp821 = tmp796 * tmp776;
                        tmp822 = p_00.xi.x;
                        tmp823 = p_00.xi.y;
                        float tmp824 = (tmp820.x + tmp820.y + tmp820.z) * tmp767;
                        float tmp825 = (tmp821.x + tmp821.y + tmp821.z) * tmp768;
                        tmp826 = tmp819 * tmp819;
                        tmp827 = tmp825 * tmp825;
                        tmp828 = tmp824 * tmp824 + tmp827;
                        float3 tmp829 = float3(sqrt(tmp828 + tmp826), 0.0f, 0.0f);
                        tmp830 = float3(tmp824, abs(tmp819), tmp825) / tmp829.xxx;
                        phi_in831 = 1.0f;
                        phi_in833 = 0.0f;
                        phi_in835 = 0.0f;
                        if (tmp830.y < 0.9999899864f) {
                            float3 tmp837 = tmp830 * float3(1.0f, 0.0f, 0.0f);
                            float3 tmp838 = tmp830 * float3(0.0f, 0.0f, 1.0f);
                            float3 tmp839 = tmp837.yzx - tmp838.zxy;
                            float3 tmp840 = float3(sqrt(tmp839.x * tmp839.x + tmp839.y * tmp839.y + tmp839.z * tmp839.z), 0.0f, 0.0f);
                            float3 tmp841 = tmp839 / tmp840.xxx;
                            phi_in831 = tmp841.x;
                            phi_in833 = tmp841.y;
                            phi_in835 = tmp841.z;
                        }
                        phi_out832 = phi_in831;
                        phi_out834 = phi_in833;
                        phi_out836 = phi_in835;
                        float3 tmp842 = float3(phi_out832, phi_out834, phi_out836);
                        float3 tmp843 = tmp842.yzx * tmp830.zxy - tmp842.zxy * tmp830.yzx;
                        tmp844 = tmp830.y + 1.0f;
                        tmp845 = 1.0f / tmp844;
                        tmp846 = sqrt(tmp822);
                        if (tmp823 < tmp845) {
                            tmp847 = tmp844 * 3.141592741f * tmp823;
                            phi_in848 = 1.0f;
                            phi_in850 = tmp847;
                        } else {
                            tmp852 = (tmp823 - tmp845) * 3.141592741f / (1.0f - tmp845) + 3.141592741f;
                            phi_in848 = tmp830.y;
                            phi_in850 = tmp852;
                        }
                        phi_out849 = phi_in848;
                        phi_out851 = phi_in850;
                        float tmp853 = cos(phi_out851) * tmp846;
                        float tmp854 = phi_out849 * tmp846 * sin(phi_out851);
                        float tmp855 = sqrt(max(1.0f - (tmp854 * tmp854 + tmp853 * tmp853), 0.0f));
                        float tmp856 = (tmp854 * tmp843.x + tmp853 * phi_out832 + tmp855 * tmp830.x) * tmp767;
                        float tmp857 = max(tmp854 * tmp843.y + tmp853 * phi_out834 + tmp855 * tmp830.y, 0.0f);
                        float tmp858 = (tmp854 * tmp843.z + tmp853 * phi_out836 + tmp855 * tmp830.z) * tmp768;
                        float3 tmp859 = float3(sqrt(tmp856 * tmp856 + tmp857 * tmp857 + tmp858 * tmp858), 0.0f, 0.0f);
                        float3 tmp860 = float3(tmp856, tmp857, tmp858) / tmp859.xxx;
                        if (tmp860.y == 0.0f) {
                            p_00.pdf = 0.0f;
                            p_00.event_type = 0;
                        } else {
                            tmp861 = tmp860.x * tmp797.x + tmp860.y * tmp782 + tmp860.z * tmp793;
                            tmp862 = tmp860.x * tmp797.y + tmp860.y * tmp783 + tmp860.z * tmp794;
                            tmp863 = tmp860.x * tmp797.z + tmp860.y * tmp784 + tmp860.z * tmp795;
                            float3 tmp864 = float3(tmp861, tmp862, tmp863) * tmp776;
                            tmp865 = tmp864.x + tmp864.y + tmp864.z;
                            if (tmp865 > 0.0f) {
                                if (tmp763) {
                                    float tmp866 = tmp865 * 2.0f;
                                    float tmp867 = tmp866 * tmp861 - tmp773;
                                    float tmp868 = tmp866 * tmp862 - tmp774;
                                    float tmp869 = tmp866 * tmp863 - tmp775;
                                    float3 tmp870 = float3(tmp867, tmp868, tmp869) * tmp786;
                                    float tmp871 = (tmp870.x + tmp870.y + tmp870.z) * 2.0f;
                                    float tmp872 = tmp867 - tmp871 * tmp782;
                                    float tmp873 = tmp868 - tmp871 * tmp783;
                                    float tmp874 = tmp869 - tmp871 * tmp784;
                                    float3 tmp875 = float3(sqrt(tmp872 * tmp872 + tmp873 * tmp873 + tmp874 * tmp874), 0.0f, 0.0f);
                                    float3 tmp876 = float3(tmp872, tmp873, tmp874) / tmp875.xxx;
                                    p_00.k2.x = tmp876.x;
                                    p_00.k2.y = tmp876.y;
                                    p_00.k2.z = tmp876.z;
                                    phi_in877 = tmp876.z;
                                    phi_in879 = tmp876.y;
                                    phi_in881 = tmp876.x;
                                    phi_in883 = 18;
                                } else {
                                    tmp885 = tmp814 / tmp817;
                                    tmp886 = tmp885 * tmp885 * (1.0f - tmp865 * tmp865);
                                    if (tmp886 > 1.0f) {
                                        float tmp887 = tmp865 * 2.0f;
                                        tmp888 = tmp887 * tmp861 - tmp773;
                                        tmp889 = tmp887 * tmp862 - tmp774;
                                        tmp890 = tmp887 * tmp863 - tmp775;
                                        phi_in891 = 10;
                                        phi_in893 = tmp888;
                                        phi_in895 = tmp889;
                                        phi_in897 = tmp890;
                                    } else {
                                        float tmp899 = tmp865 * tmp885 - sqrt(1.0f - tmp886);
                                        float tmp900 = tmp899 * tmp861 - tmp885 * tmp773;
                                        float tmp901 = tmp899 * tmp862 - tmp885 * tmp774;
                                        float tmp902 = tmp899 * tmp863 - tmp885 * tmp775;
                                        float3 tmp903 = float3(sqrt(tmp900 * tmp900 + tmp901 * tmp901 + tmp902 * tmp902), 0.0f, 0.0f);
                                        float3 tmp904 = float3(tmp900, tmp901, tmp902) / tmp903.xxx;
                                        phi_in891 = 18;
                                        phi_in893 = tmp904.x;
                                        phi_in895 = tmp904.y;
                                        phi_in897 = tmp904.z;
                                    }
                                    phi_out892 = phi_in891;
                                    phi_out894 = phi_in893;
                                    phi_out896 = phi_in895;
                                    phi_out898 = phi_in897;
                                    p_00.k2.x = phi_out894;
                                    p_00.k2.y = phi_out896;
                                    p_00.k2.z = phi_out898;
                                    phi_in877 = phi_out898;
                                    phi_in879 = phi_out896;
                                    phi_in881 = phi_out894;
                                    phi_in883 = phi_out892;
                                }
                                phi_out878 = phi_in877;
                                phi_out880 = phi_in879;
                                phi_out882 = phi_in881;
                                phi_out884 = phi_in883;
                                p_00.event_type = phi_out884;
                                p_00.bsdf_over_pdf.x = 1.0f;
                                p_00.bsdf_over_pdf.y = 1.0f;
                                p_00.bsdf_over_pdf.z = 1.0f;
                                tmp905 = float3(phi_out882, phi_out880, phi_out878);
                                float3 tmp906 = tmp905 * float3(tmp779, tmp780, tmp781);
                                float tmp907 = tmp906.x + tmp906.y + tmp906.z;
                                if ((phi_out884 == 10 ? tmp907 : -tmp907) > 0.0f) {
                                    float3 tmp908 = tmp905 * tmp786;
                                    float tmp909 = tmp908.x + tmp908.y + tmp908.z;
                                    float3 tmp910 = tmp905 * tmp797;
                                    float tmp911 = (tmp910.x + tmp910.y + tmp910.z) * tmp767;
                                    tmp912 = 2.0f / (sqrt((tmp911 * tmp911 + tmp827) / (tmp909 * tmp909) + 1.0f) + 1.0f);
                                    if (tmp912 * 2.0f / (sqrt(tmp828 / tmp826 + 1.0f) + 1.0f) > 0.0f) {
                                        p_00.bsdf_over_pdf.x = tmp912;
                                        p_00.bsdf_over_pdf.y = tmp912;
                                        p_00.bsdf_over_pdf.z = tmp912;
                                        p_00.handle = 0;
                                    } else {
                                        p_00.pdf = 0.0f;
                                        p_00.event_type = 0;
                                    }
                                } else {
                                    p_00.pdf = 0.0f;
                                    p_00.event_type = 0;
                                }
                            } else {
                                p_00.pdf = 0.0f;
                                p_00.event_type = 0;
                            }
                        }
                        p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * tmp769;
                        p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * tmp770;
                        p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * tmp771;
                    }
                }
            }
        } else {
            float tmp913 = p_11.text_results[17].y;
            float tmp914 = p_11.text_results[24].w;
            float tmp915 = (tmp914 + -1.0f) / (tmp914 + 1.0f);
            tmp916 = min(max(tmp915 * tmp915, 0.0f), 1.0f);
            tmp917 = min(max(p_11.text_results[20].x, 0.0f), 1.0f);
            tmp918 = p_11.geom_normal;
            tmp919 = p_00.k1.x;
            tmp920 = p_00.k1.y;
            tmp921 = p_00.k1.z;
            tmp922 = float3(tmp919, tmp920, tmp921);
            float3 tmp923 = tmp922 * tmp918;
            tmp924 = asfloat((asint(tmp923.x + tmp923.y + tmp923.z) & -2147483648) | 1065353216);
            tmp925 = tmp10 * tmp924;
            tmp926 = tmp11 * tmp924;
            tmp927 = tmp12 * tmp924;
            tmp928 = float3(tmp925, tmp926, tmp927);
            float3 tmp929 = tmp928 * tmp922;
            tmp930 = tmp929.x + tmp929.y + tmp929.z;
            tmp931 = min(max(1.0f - tmp913 * tmp913, 0.0f), 1.0f) - tmp916;
            tmp932 = tmp931 * pow(1.0f - min(max(min(max(tmp930, 0.0f), 1.0f), 0.0f), 1.0f), 4.0f) + tmp916;
            tmp933 = tmp932 * tmp917;
            tmp934 = phi_out435 < tmp933;
            if (tmp934) {
                tmp935 = phi_out435 / tmp933;
                phi_in936 = tmp935;
            } else {
                tmp938 = (1.0f - phi_out435) / (1.0f - tmp933);
                phi_in936 = tmp938;
            }
            phi_out937 = phi_in936;
            p_00.xi.z = phi_out937;
            if (tmp934) {
                float tmp939 = p_11.text_results[26].z;
                float tmp940 = p_11.text_results[26].w;
                float tmp941 = p_11.text_results[27].x;
                float3 tmp942 = float3(tmp939, tmp940, tmp941) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                tmp943 = p_11.text_results[17].w;
                tmp944 = p_11.text_results[17].z;
                tmp945 = max(tmp944, 1.000000012e-07f);
                tmp946 = max(tmp943, 1.000000012e-07f);
                tmp947 = tmp942.z > 0.0f || (tmp942.x > 0.0f || tmp942.y > 0.0f);
                tmp948 = min(max(tmp939, 0.0f), 1.0f);
                tmp949 = min(max(tmp940, 0.0f), 1.0f);
                tmp950 = min(max(tmp941, 0.0f), 1.0f);
                tmp951 = min(max(tmp942.x, 0.0f), 1.0f);
                tmp952 = min(max(tmp942.y, 0.0f), 1.0f);
                tmp953 = min(max(tmp942.z, 0.0f), 1.0f);
                tmp954 = tmp918.x * tmp924;
                tmp955 = tmp918.y * tmp924;
                tmp956 = tmp918.z * tmp924;
                float3 tmp957 = p_11.text_results[23].yzw;
                tmp958 = tmp928.zxy;
                tmp959 = tmp928.yzx;
                tmp960 = tmp957.yzx * tmp958 - tmp957.zxy * tmp959;
                float3 tmp961 = tmp960 * tmp960;
                tmp962 = tmp961.x + tmp961.y + tmp961.z;
                if (tmp962 < 9.999999939e-09f) {
                    p_00.pdf = 0.0f;
                    p_00.event_type = 0;
                    phi_in963 = 0;
                } else {
                    float tmp965 = 1.0f / sqrt(tmp962);
                    tmp966 = tmp965 * tmp960.x;
                    tmp967 = tmp965 * tmp960.y;
                    tmp968 = tmp965 * tmp960.z;
                    tmp969 = float3(tmp966, tmp967, tmp968);
                    tmp970 = tmp969.zxy * tmp959 - tmp969.yzx * tmp958;
                    if (p_00.ior1.x == -1.0f) {
                        p_00.ior1.x = p_11.text_results[28].x;
                        p_00.ior1.y = p_11.text_results[28].y;
                        p_00.ior1.z = p_11.text_results[28].z;
                    }
                    if (p_00.ior2.x == -1.0f) {
                        p_00.ior2.x = p_11.text_results[28].x;
                        p_00.ior2.y = p_11.text_results[28].y;
                        p_00.ior2.z = p_11.text_results[28].z;
                    }
                    float3 tmp971 = tmp970 * tmp922;
                    float3 tmp972 = tmp969 * tmp922;
                    tmp973 = p_00.xi.x;
                    tmp974 = p_00.xi.y;
                    float tmp975 = (tmp971.x + tmp971.y + tmp971.z) * tmp945;
                    float tmp976 = (tmp972.x + tmp972.y + tmp972.z) * tmp946;
                    tmp977 = tmp930 * tmp930;
                    tmp978 = tmp976 * tmp976;
                    tmp979 = tmp975 * tmp975 + tmp978;
                    float3 tmp980 = float3(sqrt(tmp979 + tmp977), 0.0f, 0.0f);
                    tmp981 = float3(tmp975, abs(tmp930), tmp976) / tmp980.xxx;
                    phi_in982 = 1.0f;
                    phi_in984 = 0.0f;
                    phi_in986 = 0.0f;
                    if (tmp981.y < 0.9999899864f) {
                        float3 tmp988 = tmp981 * float3(1.0f, 0.0f, 0.0f);
                        float3 tmp989 = tmp981 * float3(0.0f, 0.0f, 1.0f);
                        float3 tmp990 = tmp988.yzx - tmp989.zxy;
                        float3 tmp991 = float3(sqrt(tmp990.x * tmp990.x + tmp990.y * tmp990.y + tmp990.z * tmp990.z), 0.0f, 0.0f);
                        float3 tmp992 = tmp990 / tmp991.xxx;
                        phi_in982 = tmp992.x;
                        phi_in984 = tmp992.y;
                        phi_in986 = tmp992.z;
                    }
                    phi_out983 = phi_in982;
                    phi_out985 = phi_in984;
                    phi_out987 = phi_in986;
                    float3 tmp993 = float3(phi_out983, phi_out985, phi_out987);
                    float3 tmp994 = tmp993.yzx * tmp981.zxy - tmp993.zxy * tmp981.yzx;
                    tmp995 = tmp981.y + 1.0f;
                    tmp996 = 1.0f / tmp995;
                    tmp997 = sqrt(tmp973);
                    if (tmp974 < tmp996) {
                        tmp998 = tmp995 * 3.141592741f * tmp974;
                        phi_in999 = 1.0f;
                        phi_in1001 = tmp998;
                    } else {
                        tmp1003 = (tmp974 - tmp996) * 3.141592741f / (1.0f - tmp996) + 3.141592741f;
                        phi_in999 = tmp981.y;
                        phi_in1001 = tmp1003;
                    }
                    phi_out1000 = phi_in999;
                    phi_out1002 = phi_in1001;
                    float tmp1004 = cos(phi_out1002) * tmp997;
                    float tmp1005 = phi_out1000 * tmp997 * sin(phi_out1002);
                    float tmp1006 = sqrt(max(1.0f - (tmp1005 * tmp1005 + tmp1004 * tmp1004), 0.0f));
                    float tmp1007 = (tmp1005 * tmp994.x + tmp1004 * phi_out983 + tmp1006 * tmp981.x) * tmp945;
                    float tmp1008 = max(tmp1005 * tmp994.y + tmp1004 * phi_out985 + tmp1006 * tmp981.y, 0.0f);
                    float tmp1009 = (tmp1005 * tmp994.z + tmp1004 * phi_out987 + tmp1006 * tmp981.z) * tmp946;
                    float3 tmp1010 = float3(sqrt(tmp1007 * tmp1007 + tmp1008 * tmp1008 + tmp1009 * tmp1009), 0.0f, 0.0f);
                    float3 tmp1011 = float3(tmp1007, tmp1008, tmp1009) / tmp1010.xxx;
                    if (tmp1011.y == 0.0f) {
                        p_00.pdf = 0.0f;
                        p_00.event_type = 0;
                        phi_in1012 = 0;
                        phi_in1014 = true;
                    } else {
                        tmp1016 = tmp1011.x * tmp970.x + tmp1011.y * tmp925 + tmp1011.z * tmp966;
                        tmp1017 = tmp1011.x * tmp970.y + tmp1011.y * tmp926 + tmp1011.z * tmp967;
                        tmp1018 = tmp1011.x * tmp970.z + tmp1011.y * tmp927 + tmp1011.z * tmp968;
                        float3 tmp1019 = float3(tmp1016, tmp1017, tmp1018) * tmp922;
                        tmp1020 = tmp1019.x + tmp1019.y + tmp1019.z;
                        if (tmp1020 > 0.0f) {
                            float tmp1021 = tmp1020 * 2.0f;
                            float tmp1022 = tmp1021 * tmp1016 - tmp919;
                            float tmp1023 = tmp1021 * tmp1017 - tmp920;
                            float tmp1024 = tmp1021 * tmp1018 - tmp921;
                            p_00.k2.x = tmp1022;
                            p_00.k2.y = tmp1023;
                            p_00.k2.z = tmp1024;
                            p_00.event_type = 10;
                            p_00.bsdf_over_pdf.x = 1.0f;
                            p_00.bsdf_over_pdf.y = 1.0f;
                            p_00.bsdf_over_pdf.z = 1.0f;
                            tmp1025 = float3(tmp1022, tmp1023, tmp1024);
                            float3 tmp1026 = tmp1025 * float3(tmp954, tmp955, tmp956);
                            if (tmp1026.x + tmp1026.y + tmp1026.z > 0.0f) {
                                float3 tmp1027 = tmp1025 * tmp928;
                                float tmp1028 = tmp1027.x + tmp1027.y + tmp1027.z;
                                float3 tmp1029 = tmp1025 * tmp970;
                                float tmp1030 = (tmp1029.x + tmp1029.y + tmp1029.z) * tmp945;
                                tmp1031 = 2.0f / (sqrt((tmp1030 * tmp1030 + tmp978) / (tmp1028 * tmp1028) + 1.0f) + 1.0f);
                                if (tmp1031 * 2.0f / (sqrt(tmp979 / tmp977 + 1.0f) + 1.0f) > 0.0f) {
                                    p_00.bsdf_over_pdf.x = tmp1031;
                                    p_00.bsdf_over_pdf.y = tmp1031;
                                    p_00.bsdf_over_pdf.z = tmp1031;
                                    p_00.handle = 0;
                                    phi_in1012 = 10;
                                    phi_in1014 = false;
                                } else {
                                    p_00.pdf = 0.0f;
                                    p_00.event_type = 0;
                                    phi_in1012 = 0;
                                    phi_in1014 = true;
                                }
                            } else {
                                p_00.pdf = 0.0f;
                                p_00.event_type = 0;
                                phi_in1012 = 0;
                                phi_in1014 = true;
                            }
                        } else {
                            p_00.pdf = 0.0f;
                            p_00.event_type = 0;
                            phi_in1012 = 0;
                            phi_in1014 = true;
                        }
                    }
                    phi_out1013 = phi_in1012;
                    phi_out1015 = phi_in1014;
                    if (tmp947) {
                        phi_in1032 = 0.0f;
                        if (!phi_out1015) {
                            tmp1034 = (p_00.bsdf_over_pdf.y + p_00.bsdf_over_pdf.x + p_00.bsdf_over_pdf.z) * 0.3333333433f;
                            phi_in1032 = tmp1034;
                        }
                        phi_out1033 = phi_in1032;
                        tmp1035 = min(max(sqrt(tmp944 * tmp943), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
                        tmp1036 = p_00.xi.w;
                        if (tmp1036 > phi_out1033) {
                            float tmp1037 = min(max((tmp1036 - phi_out1033) / (1.0f - phi_out1033), 0.0f), 1.0f);
                            float tmp1038 = phi_out937 * 6.283185482f;
                            float tmp1039 = sin(tmp1038);
                            float tmp1040 = cos(tmp1038);
                            float tmp1041 = sqrt(1.0f - tmp1037);
                            float tmp1042 = sqrt(tmp1037);
                            float tmp1043 = tmp1041 * (tmp970.x * tmp1040 + tmp966 * tmp1039) + tmp1042 * tmp925;
                            float tmp1044 = tmp1041 * (tmp970.y * tmp1040 + tmp967 * tmp1039) + tmp1042 * tmp926;
                            float tmp1045 = tmp1041 * (tmp970.z * tmp1040 + tmp968 * tmp1039) + tmp1042 * tmp927;
                            float3 tmp1046 = float3(sqrt(tmp1043 * tmp1043 + tmp1044 * tmp1044 + tmp1045 * tmp1045), 0.0f, 0.0f);
                            float3 tmp1047 = float3(tmp1043, tmp1044, tmp1045) / tmp1046.xxx;
                            p_00.k2.x = tmp1047.x;
                            p_00.k2.y = tmp1047.y;
                            p_00.k2.z = tmp1047.z;
                            p_00.event_type = 9;
                            float tmp1048 = (3.141592741f - tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp1042, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp1035, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x * 3.141592741f) * tmp1042 / tex_lookup_float3_3d(1, float3(1.0f, tmp1035, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                            p_00.bsdf_over_pdf.x = tmp1048 * tmp951;
                            p_00.bsdf_over_pdf.y = tmp1048 * tmp952;
                            p_00.bsdf_over_pdf.z = tmp1048 * tmp953;
                            phi_in963 = 9;
                        } else {
                            float tmp1049 = 1.0f / phi_out1033;
                            p_00.bsdf_over_pdf.x = tmp1049 * tmp948 * p_00.bsdf_over_pdf.x;
                            p_00.bsdf_over_pdf.y = tmp1049 * tmp949 * p_00.bsdf_over_pdf.y;
                            p_00.bsdf_over_pdf.z = tmp1049 * tmp950 * p_00.bsdf_over_pdf.z;
                            phi_in963 = phi_out1013;
                        }
                    } else {
                        p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * tmp948;
                        p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * tmp949;
                        p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * tmp950;
                        phi_in963 = phi_out1013;
                    }
                }
            } else {
                tmp1050 = min(max(p_11.text_results[20].z, 0.0f), 1.0f);
                tmp1051 = phi_out937 < tmp1050;
                if (tmp1051) {
                    tmp1052 = phi_out937 / tmp1050;
                    phi_in1053 = tmp1052;
                } else {
                    tmp1055 = (phi_out937 - tmp1050) / (1.0f - tmp1050);
                    phi_in1053 = tmp1055;
                }
                phi_out1054 = phi_in1053;
                p_00.xi.z = phi_out1054;
                if (tmp1051) {
                    float tmp1056 = p_11.text_results[27].y;
                    float tmp1057 = p_11.text_results[27].z;
                    float tmp1058 = p_11.text_results[27].w;
                    float3 tmp1059 = float3(tmp1056, tmp1057, tmp1058) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                    tmp1060 = p_11.text_results[19].z;
                    tmp1061 = 1.0f / max(tmp1060, 1.000000012e-07f);
                    tmp1062 = tmp1059.z > 0.0f || (tmp1059.x > 0.0f || tmp1059.y > 0.0f);
                    tmp1063 = tmp1062 ? 2 : 0;
                    float3 tmp1064 = p_11.tangent_u[0];
                    tmp1065 = min(max(tmp1056, 0.0f), 1.0f);
                    tmp1066 = min(max(tmp1057, 0.0f), 1.0f);
                    tmp1067 = min(max(tmp1058, 0.0f), 1.0f);
                    tmp1068 = min(max(tmp1059.x, 0.0f), 1.0f);
                    tmp1069 = min(max(tmp1059.y, 0.0f), 1.0f);
                    tmp1070 = min(max(tmp1059.z, 0.0f), 1.0f);
                    tmp1071 = tmp918.x * tmp924;
                    tmp1072 = tmp918.y * tmp924;
                    tmp1073 = tmp918.z * tmp924;
                    tmp1074 = tmp928.zxy;
                    tmp1075 = tmp928.yzx;
                    tmp1076 = tmp1064.yzx * tmp1074 - tmp1064.zxy * tmp1075;
                    float3 tmp1077 = tmp1076 * tmp1076;
                    tmp1078 = tmp1077.x + tmp1077.y + tmp1077.z;
                    if (tmp1078 < 9.999999939e-09f) {
                        p_00.pdf = 0.0f;
                        p_00.event_type = 0;
                        phi_in963 = 0;
                    } else {
                        float tmp1079 = 1.0f / sqrt(tmp1078);
                        tmp1080 = tmp1079 * tmp1076.x;
                        tmp1081 = tmp1079 * tmp1076.y;
                        tmp1082 = tmp1079 * tmp1076.z;
                        tmp1083 = float3(tmp1080, tmp1081, tmp1082);
                        tmp1084 = tmp1083.zxy * tmp1075 - tmp1083.yzx * tmp1074;
                        if (p_00.ior1.x == -1.0f) {
                            p_00.ior1.x = p_11.text_results[28].x;
                            p_00.ior1.y = p_11.text_results[28].y;
                            p_00.ior1.z = p_11.text_results[28].z;
                        }
                        if (p_00.ior2.x == -1.0f) {
                            p_00.ior2.x = p_11.text_results[28].x;
                            p_00.ior2.y = p_11.text_results[28].y;
                            p_00.ior2.z = p_11.text_results[28].z;
                        }
                        tmp1085 = abs(tmp930);
                        if (tmp1062) {
                            tmp1086 = min(max(abs(tmp1060), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
                            tmp1087 = tex_lookup_float3_3d(tmp1063, float3(min(max(acos(min(max(tmp1085, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp1086, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                            if (phi_out1054 < tmp1087) {
                                tmp1088 = phi_out1054 / tmp1087;
                                float3 tmp1089 = tmp1084 * tmp922;
                                float3 tmp1090 = tmp1083 * tmp922;
                                float tmp1091 = p_00.xi.x * 6.283185482f;
                                float tmp1092 = pow(1.0f - p_00.xi.y, 1.0f / (tmp1061 + 2.0f));
                                float tmp1093 = 1.0f - tmp1092 * tmp1092;
                                float tmp1094 = cos(tmp1091) * tmp1092;
                                float tmp1095 = sin(tmp1091) * tmp1092;
                                float3 tmp1096 = float3(sqrt(tmp1094 * tmp1094 + tmp1093 + tmp1095 * tmp1095), 0.0f, 0.0f);
                                float3 tmp1097 = float3(tmp1094, sqrt(tmp1093), tmp1095) / tmp1096.xxx;
                                float tmp1098 = tmp1097.y * tmp1085;
                                float tmp1099 = tmp1097.z * (tmp1090.x + tmp1090.y + tmp1090.z) + tmp1097.x * (tmp1089.x + tmp1089.y + tmp1089.z);
                                float tmp1100 = max(tmp1098 - tmp1099, 0.0f);
                                tmp1101 = tmp1100 / (tmp1100 + max(tmp1099 + tmp1098, 0.0f));
                                if (tmp1088 < tmp1101) {
                                    p_00.xi.z = tmp1088 / tmp1101;
                                    tmp1102 = -tmp1097.x;
                                    tmp1103 = -tmp1097.z;
                                    phi_in1104 = tmp1102;
                                    phi_in1106 = tmp1103;
                                } else {
                                    p_00.xi.z = (tmp1088 - tmp1101) / (1.0f - tmp1101);
                                    phi_in1104 = tmp1097.x;
                                    phi_in1106 = tmp1097.z;
                                }
                                phi_out1105 = phi_in1104;
                                phi_out1107 = phi_in1106;
                                if (tmp1097.y == 0.0f) {
                                    p_00.pdf = 0.0f;
                                    p_00.event_type = 0;
                                    phi_in963 = 0;
                                } else {
                                    tmp1108 = phi_out1105 * tmp1084.x + tmp1097.y * tmp925 + phi_out1107 * tmp1080;
                                    tmp1109 = phi_out1105 * tmp1084.y + tmp1097.y * tmp926 + phi_out1107 * tmp1081;
                                    tmp1110 = phi_out1105 * tmp1084.z + tmp1097.y * tmp927 + phi_out1107 * tmp1082;
                                    tmp1111 = float3(tmp1108, tmp1109, tmp1110);
                                    float3 tmp1112 = tmp1111 * tmp922;
                                    tmp1113 = tmp1112.x + tmp1112.y + tmp1112.z;
                                    if (tmp1113 > 0.0f) {
                                        float tmp1114 = tmp1113 * 2.0f;
                                        float tmp1115 = tmp1114 * tmp1108 - tmp919;
                                        float tmp1116 = tmp1114 * tmp1109 - tmp920;
                                        float tmp1117 = tmp1114 * tmp1110 - tmp921;
                                        p_00.k2.x = tmp1115;
                                        p_00.k2.y = tmp1116;
                                        p_00.k2.z = tmp1117;
                                        p_00.event_type = 10;
                                        p_00.bsdf_over_pdf.x = 1.0f;
                                        p_00.bsdf_over_pdf.y = 1.0f;
                                        p_00.bsdf_over_pdf.z = 1.0f;
                                        tmp1118 = float3(tmp1115, tmp1116, tmp1117);
                                        float3 tmp1119 = tmp1118 * float3(tmp1071, tmp1072, tmp1073);
                                        if (tmp1119.x + tmp1119.y + tmp1119.z > 0.0f) {
                                            float3 tmp1120 = tmp1118 * tmp928;
                                            float3 tmp1121 = tmp1118 * tmp1111;
                                            float tmp1122 = tmp1097.y * 2.0f;
                                            tmp1123 = min(tmp1122 * tmp1085 / tmp1113, 1.0f);
                                            tmp1124 = min(tmp1123, min(abs(tmp1120.x + tmp1120.y + tmp1120.z) * tmp1122 / abs(tmp1121.x + tmp1121.y + tmp1121.z), 1.0f));
                                            if (tmp1124 > 0.0f) {
                                                p_00.handle = 0;
                                                float tmp1125 = tmp1124 / (tmp1123 * tmp1087);
                                                p_00.bsdf_over_pdf.x = tmp1125 * tmp1065;
                                                p_00.bsdf_over_pdf.y = tmp1125 * tmp1066;
                                                p_00.bsdf_over_pdf.z = tmp1125 * tmp1067;
                                                phi_in963 = 10;
                                            } else {
                                                p_00.pdf = 0.0f;
                                                p_00.event_type = 0;
                                                phi_in963 = 0;
                                            }
                                        } else {
                                            p_00.pdf = 0.0f;
                                            p_00.event_type = 0;
                                            phi_in963 = 0;
                                        }
                                    } else {
                                        p_00.pdf = 0.0f;
                                        p_00.event_type = 0;
                                        phi_in963 = 0;
                                    }
                                }
                            } else {
                                tmp1126 = 1.0f - tmp1087;
                                p_00.xi.z = (phi_out1054 - tmp1087) / tmp1126;
                                gen_weighted_layer_sample1(p_00, p_11);
                                tmp1127 = p_00.event_type;
                                phi_in963 = 0;
                                if (tmp1127 != 0) {
                                    float3 tmp1128 = p_00.k2 * tmp928;
                                    float tmp1129 = (1.0f - max(tmp1087, tex_lookup_float3_3d(tmp1063, float3(min(max(acos(min(max(abs(tmp1128.x + tmp1128.y + tmp1128.z), 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp1086, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x)) / tmp1126;
                                    p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * tmp1068 * tmp1129;
                                    p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * tmp1069 * tmp1129;
                                    p_00.bsdf_over_pdf.z = tmp1129 * tmp1070 * p_00.bsdf_over_pdf.z;
                                    phi_in963 = tmp1127;
                                }
                            }
                        } else {
                            float3 tmp1130 = tmp1084 * tmp922;
                            float3 tmp1131 = tmp1083 * tmp922;
                            float tmp1132 = p_00.xi.x * 6.283185482f;
                            float tmp1133 = pow(1.0f - p_00.xi.y, 1.0f / (tmp1061 + 2.0f));
                            float tmp1134 = 1.0f - tmp1133 * tmp1133;
                            float tmp1135 = cos(tmp1132) * tmp1133;
                            float tmp1136 = sin(tmp1132) * tmp1133;
                            float3 tmp1137 = float3(sqrt(tmp1135 * tmp1135 + tmp1134 + tmp1136 * tmp1136), 0.0f, 0.0f);
                            float3 tmp1138 = float3(tmp1135, sqrt(tmp1134), tmp1136) / tmp1137.xxx;
                            float tmp1139 = tmp1138.y * tmp1085;
                            float tmp1140 = tmp1138.z * (tmp1131.x + tmp1131.y + tmp1131.z) + tmp1138.x * (tmp1130.x + tmp1130.y + tmp1130.z);
                            float tmp1141 = max(tmp1139 - tmp1140, 0.0f);
                            tmp1142 = tmp1141 / (tmp1141 + max(tmp1140 + tmp1139, 0.0f));
                            if (phi_out1054 < tmp1142) {
                                p_00.xi.z = phi_out1054 / tmp1142;
                                tmp1143 = -tmp1138.x;
                                tmp1144 = -tmp1138.z;
                                phi_in1145 = tmp1143;
                                phi_in1147 = tmp1144;
                            } else {
                                p_00.xi.z = (phi_out1054 - tmp1142) / (1.0f - tmp1142);
                                phi_in1145 = tmp1138.x;
                                phi_in1147 = tmp1138.z;
                            }
                            phi_out1146 = phi_in1145;
                            phi_out1148 = phi_in1147;
                            if (tmp1138.y == 0.0f) {
                                p_00.pdf = 0.0f;
                                p_00.event_type = 0;
                                phi_in1149 = 0;
                            } else {
                                tmp1151 = phi_out1146 * tmp1084.x + tmp1138.y * tmp925 + phi_out1148 * tmp1080;
                                tmp1152 = phi_out1146 * tmp1084.y + tmp1138.y * tmp926 + phi_out1148 * tmp1081;
                                tmp1153 = phi_out1146 * tmp1084.z + tmp1138.y * tmp927 + phi_out1148 * tmp1082;
                                tmp1154 = float3(tmp1151, tmp1152, tmp1153);
                                float3 tmp1155 = tmp1154 * tmp922;
                                tmp1156 = tmp1155.x + tmp1155.y + tmp1155.z;
                                if (tmp1156 > 0.0f) {
                                    float tmp1157 = tmp1156 * 2.0f;
                                    float tmp1158 = tmp1157 * tmp1151 - tmp919;
                                    float tmp1159 = tmp1157 * tmp1152 - tmp920;
                                    float tmp1160 = tmp1157 * tmp1153 - tmp921;
                                    p_00.k2.x = tmp1158;
                                    p_00.k2.y = tmp1159;
                                    p_00.k2.z = tmp1160;
                                    p_00.event_type = 10;
                                    p_00.bsdf_over_pdf.x = 1.0f;
                                    p_00.bsdf_over_pdf.y = 1.0f;
                                    p_00.bsdf_over_pdf.z = 1.0f;
                                    tmp1161 = float3(tmp1158, tmp1159, tmp1160);
                                    float3 tmp1162 = tmp1161 * float3(tmp1071, tmp1072, tmp1073);
                                    if (tmp1162.x + tmp1162.y + tmp1162.z > 0.0f) {
                                        float3 tmp1163 = tmp1161 * tmp928;
                                        float3 tmp1164 = tmp1161 * tmp1154;
                                        float tmp1165 = tmp1138.y * 2.0f;
                                        tmp1166 = min(tmp1165 * tmp1085 / tmp1156, 1.0f);
                                        tmp1167 = min(tmp1166, min(abs(tmp1163.x + tmp1163.y + tmp1163.z) * tmp1165 / abs(tmp1164.x + tmp1164.y + tmp1164.z), 1.0f));
                                        if (tmp1167 > 0.0f) {
                                            float tmp1168 = tmp1167 / tmp1166;
                                            p_00.bsdf_over_pdf.x = tmp1168;
                                            p_00.bsdf_over_pdf.y = tmp1168;
                                            p_00.bsdf_over_pdf.z = tmp1168;
                                            p_00.handle = 0;
                                            phi_in1149 = 10;
                                        } else {
                                            p_00.pdf = 0.0f;
                                            p_00.event_type = 0;
                                            phi_in1149 = 0;
                                        }
                                    } else {
                                        p_00.pdf = 0.0f;
                                        p_00.event_type = 0;
                                        phi_in1149 = 0;
                                    }
                                } else {
                                    p_00.pdf = 0.0f;
                                    p_00.event_type = 0;
                                    phi_in1149 = 0;
                                }
                            }
                            phi_out1150 = phi_in1149;
                            p_00.bsdf_over_pdf.x = p_00.bsdf_over_pdf.x * tmp1065;
                            p_00.bsdf_over_pdf.y = p_00.bsdf_over_pdf.y * tmp1066;
                            p_00.bsdf_over_pdf.z = p_00.bsdf_over_pdf.z * tmp1067;
                            phi_in963 = phi_out1150;
                        }
                    }
                } else {
                    gen_weighted_layer_sample1(p_00, p_11);
                    tmp1169 = p_00.event_type;
                    phi_in963 = tmp1169;
                }
            }
            phi_out964 = phi_in963;
            if (phi_out964 != 0) {
                tmp1170 = p_00.k2.x;
                tmp1171 = p_00.k2.y;
                tmp1172 = p_00.k2.z;
                float3 tmp1173 = float3(tmp1170, tmp1171, tmp1172) * tmp928;
                tmp1174 = abs(tmp1173.x + tmp1173.y + tmp1173.z);
                if ((phi_out964 & 16) == 0) {
                    tmp1175 = p_00.k1.x;
                    tmp1176 = tmp1175 + tmp1170;
                    tmp1177 = p_00.k1.y;
                    tmp1178 = tmp1177 + tmp1171;
                    phi_in1179 = tmp1177;
                    phi_in1181 = tmp1175;
                    phi_in1183 = tmp1172;
                    phi_in1185 = tmp1176;
                    phi_in1187 = tmp1178;
                } else {
                    float tmp1189 = tmp1174 * 2.0f;
                    tmp1190 = tmp1189 * tmp927 + tmp1172;
                    tmp1191 = p_00.k1.x;
                    tmp1192 = tmp1189 * tmp925 + tmp1170 + tmp1191;
                    tmp1193 = p_00.k1.y;
                    tmp1194 = tmp1189 * tmp926 + tmp1171 + tmp1193;
                    phi_in1179 = tmp1193;
                    phi_in1181 = tmp1191;
                    phi_in1183 = tmp1190;
                    phi_in1185 = tmp1192;
                    phi_in1187 = tmp1194;
                }
                phi_out1180 = phi_in1179;
                phi_out1182 = phi_in1181;
                phi_out1184 = phi_in1183;
                phi_out1186 = phi_in1185;
                phi_out1188 = phi_in1187;
                float tmp1195 = p_00.k1.z;
                float tmp1196 = tmp1195 + phi_out1184;
                float3 tmp1197 = float3(sqrt(phi_out1188 * phi_out1188 + phi_out1186 * phi_out1186 + tmp1196 * tmp1196), 0.0f, 0.0f);
                tmp1198 = float3(phi_out1186, phi_out1188, tmp1196) * float3(phi_out1182, phi_out1180, tmp1195) / tmp1197.xxx;
                if (tmp934) {
                    tmp1199 = (pow(1.0f - min(max(abs(tmp1198.x + tmp1198.y + tmp1198.z), 0.0f), 1.0f), 4.0f) * tmp931 + tmp916) * tmp917 / tmp933;
                    p_00.bsdf_over_pdf.x = tmp1199 * p_00.bsdf_over_pdf.x;
                    p_00.bsdf_over_pdf.y = tmp1199 * p_00.bsdf_over_pdf.y;
                    tmp1200 = p_00.bsdf_over_pdf.z;
                    phi_in1201 = tmp1199;
                    phi_in1203 = tmp1200;
                } else {
                    tmp1205 = (1.0f - max(tmp932, pow(1.0f - min(max(tmp1174, 0.0f), 1.0f), 4.0f) * tmp931 + tmp916) * tmp917) / (1.0f - tmp933);
                    p_00.bsdf_over_pdf.x = tmp1205 * p_00.bsdf_over_pdf.x;
                    p_00.bsdf_over_pdf.y = tmp1205 * p_00.bsdf_over_pdf.y;
                    tmp1206 = p_00.bsdf_over_pdf.z;
                    phi_in1201 = tmp1206;
                    phi_in1203 = tmp1205;
                }
                phi_out1202 = phi_in1201;
                phi_out1204 = phi_in1203;
                p_00.bsdf_over_pdf.z = phi_out1204 * phi_out1202;
            }
        }
    }
    return;
}

void gen_weighted_layer_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11)
{
    float tmp2;
    float tmp3;
    float tmp4;
    float tmp5;
    float phi_in;
    float phi_out;
    float tmp8;
    float3 tmp9;
    float3 tmp10;
    float phi_in12;
    float phi_out13;
    float tmp17;
    float tmp18;
    float phi_in19;
    float phi_out20;
    float tmp23;
    float3 tmp24;
    float3 tmp25;
    float phi_in27;
    float phi_out28;
    float tmp32;
    float tmp33;
    tmp2 = p_11.text_results[25].x;
    tmp3 = p_11.text_results[25].y;
    tmp4 = p_11.text_results[25].z;
    tmp5 = min(max(p_11.text_results[13].y, 0.0f), 1.0f);
    phi_in = 0.0f;
    if (tmp5 > 0.0f) {
        float3 tmp6 = p_11.geom_normal;
        float3 tmp7 = p_00.k1 * tmp6;
        tmp8 = asfloat((asint(tmp7.x + tmp7.y + tmp7.z) & -2147483648) | 1065353216);
        tmp9 = p_00.k2;
        tmp10 = float3(tmp6.x * tmp8, tmp6.y * tmp8, tmp6.z * tmp8);
        float3 tmp11 = tmp10 * tmp9;
        phi_in12 = 0.0f;
        if (tmp11.x + tmp11.y + tmp11.z < 0.0f) {
            float3 tmp14 = float3(tmp2 * tmp8, tmp3 * tmp8, tmp4 * tmp8);
            float3 tmp15 = tmp14 * tmp10;
            float3 tmp16 = tmp14 * tmp9;
            tmp17 = max(-(tmp16.x + tmp16.y + tmp16.z), 0.0f) / (max(tmp15.x + tmp15.y + tmp15.z, 0.0f) * 1.570796371f + 1.570796371f);
            phi_in12 = tmp17;
        }
        phi_out13 = phi_in12;
        tmp18 = phi_out13 * tmp5;
        phi_in = tmp18;
    }
    phi_out = phi_in;
    phi_in19 = phi_out;
    if (tmp5 < 1.0f) {
        float3 tmp21 = p_11.geom_normal;
        float3 tmp22 = p_00.k1 * tmp21;
        tmp23 = asfloat((asint(tmp22.x + tmp22.y + tmp22.z) & -2147483648) | 1065353216);
        tmp24 = p_00.k2;
        tmp25 = float3(tmp21.x * tmp23, tmp21.y * tmp23, tmp21.z * tmp23);
        float3 tmp26 = tmp25 * tmp24;
        phi_in27 = 0.0f;
        if (tmp26.x + tmp26.y + tmp26.z > 0.0f) {
            float3 tmp29 = float3(tmp2 * tmp23, tmp3 * tmp23, tmp4 * tmp23);
            float3 tmp30 = tmp29 * tmp25;
            float3 tmp31 = tmp29 * tmp24;
            tmp32 = max(tmp31.x + tmp31.y + tmp31.z, 0.0f) / (max(tmp30.x + tmp30.y + tmp30.z, 0.0f) * 1.570796371f + 1.570796371f);
            phi_in27 = tmp32;
        }
        phi_out28 = phi_in27;
        tmp33 = phi_out28 * (1.0f - tmp5) + phi_out;
        phi_in19 = tmp33;
    }
    phi_out20 = phi_in19;
    p_00.pdf = phi_out20;
    return;
}

void gen_custom_curve_layer_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11)
{
    float22 tmp2;
    float22 tmp3;
    float tmp4;
    float tmp5;
    float tmp6;
    float tmp8;
    float tmp9;
    float tmp10;
    float3 tmp11;
    float tmp13;
    float tmp14;
    float tmp15;
    float tmp16;
    float tmp17;
    float tmp18;
    float tmp19;
    float tmp20;
    float3 tmp22;
    float3 tmp23;
    float3 tmp24;
    float3 tmp25;
    float tmp27;
    float tmp29;
    float phi_in;
    float phi_out;
    float phi_in30;
    float phi_out31;
    float3 tmp33;
    float3 tmp34;
    float tmp36;
    float tmp37;
    float tmp38;
    float tmp39;
    float tmp40;
    float3 tmp41;
    float tmp43;
    float phi_in45;
    float phi_out46;
    float3 tmp51;
    float tmp53;
    float tmp67;
    float tmp69;
    float tmp70;
    float tmp71;
    float tmp72;
    float tmp73;
    float tmp74;
    float tmp75;
    float phi_in76;
    float phi_out77;
    float tmp78;
    float tmp79;
    float tmp80;
    float tmp81;
    float3 tmp83;
    float3 tmp84;
    float3 tmp85;
    float3 tmp86;
    float tmp88;
    float phi_in89;
    float phi_out90;
    float3 tmp92;
    float3 tmp93;
    float tmp95;
    float tmp96;
    float tmp97;
    float tmp98;
    float tmp99;
    float3 tmp100;
    float tmp102;
    float phi_in104;
    float phi_out105;
    float3 tmp110;
    float tmp112;
    float tmp126;
    float tmp128;
    float tmp129;
    float phi_in130;
    float phi_out131;
    float tmp132;
    float phi_in133;
    float phi_out134;
    float tmp135;
    float phi_in136;
    float phi_out137;
    int tmp138;
    float tmp140;
    float tmp141;
    float tmp142;
    float tmp143;
    bool tmp144;
    float tmp145;
    float tmp146;
    float tmp147;
    float3 tmp149;
    float3 tmp150;
    float3 tmp151;
    float3 tmp152;
    float tmp154;
    float phi_in155;
    float phi_out156;
    float3 tmp158;
    float3 tmp159;
    float tmp160;
    float phi_in161;
    float phi_out162;
    float tmp163;
    float tmp164;
    float tmp165;
    float tmp166;
    float tmp167;
    float phi_in168;
    float phi_out169;
    float phi_in170;
    float phi_out171;
    float phi_in172;
    float phi_out173;
    float tmp174;
    float tmp175;
    float tmp176;
    float tmp179;
    bool tmp180;
    float tmp182;
    float tmp183;
    float tmp184;
    float tmp185;
    float tmp186;
    float3 tmp187;
    float tmp189;
    bool tmp191;
    float tmp193;
    float tmp194;
    float tmp195;
    float phi_in196;
    float phi_out197;
    float phi_in198;
    float phi_out199;
    float phi_in200;
    float phi_out201;
    float tmp202;
    float tmp203;
    float tmp204;
    float tmp205;
    float tmp206;
    float tmp207;
    float tmp208;
    float tmp209;
    float tmp210;
    float3 tmp212;
    float tmp214;
    float tmp216;
    float tmp219;
    float phi_in220;
    float phi_out221;
    float tmp222;
    float tmp230;
    float tmp235;
    float tmp237;
    float tmp238;
    float phi_in239;
    float phi_out240;
    float phi_in241;
    float phi_out242;
    float phi_in243;
    float phi_out244;
    float tmp245;
    float tmp246;
    float phi_in247;
    float phi_out248;
    float tmp249;
    float tmp250;
    float tmp251;
    float phi_in252;
    float phi_out253;
    float tmp255;
    float tmp257;
    float tmp258;
    float phi_in259;
    float phi_out260;
    float3 tmp261;
    float tmp263;
    float phi_in264;
    float phi_out265;
    float3 tmp266;
    float3 tmp267;
    float phi_in269;
    float phi_out270;
    float tmp274;
    float tmp275;
    float phi_in276;
    float phi_out277;
    float tmp278;
    float tmp279;
    float tmp280;
    float tmp281;
    float tmp282;
    float3 tmp284;
    float3 tmp285;
    float3 tmp286;
    float3 tmp287;
    float tmp289;
    float phi_in290;
    float phi_out291;
    float3 tmp293;
    float3 tmp294;
    float tmp295;
    float phi_in296;
    float phi_out297;
    float tmp298;
    float tmp299;
    float tmp300;
    float tmp301;
    float tmp302;
    float phi_in303;
    float phi_out304;
    float phi_in305;
    float phi_out306;
    float phi_in307;
    float phi_out308;
    float tmp309;
    float tmp310;
    float tmp311;
    float tmp314;
    bool tmp315;
    float tmp317;
    float tmp318;
    float tmp319;
    float tmp320;
    float tmp321;
    float3 tmp322;
    float tmp324;
    bool tmp326;
    float tmp328;
    float tmp329;
    float tmp330;
    float phi_in331;
    float phi_out332;
    float phi_in333;
    float phi_out334;
    float phi_in335;
    float phi_out336;
    float tmp337;
    float tmp338;
    float tmp339;
    float tmp340;
    float tmp341;
    float tmp342;
    float tmp343;
    float tmp344;
    float tmp345;
    float3 tmp347;
    float tmp349;
    float tmp351;
    float tmp354;
    int phi_in355;
    int phi_out356;
    float phi_in357;
    float phi_out358;
    float phi_in359;
    float phi_out360;
    float tmp369;
    float tmp374;
    float tmp376;
    float tmp377;
    float phi_in378;
    float phi_out379;
    float phi_in380;
    float phi_out381;
    float phi_in382;
    float phi_out383;
    float tmp384;
    float tmp385;
    float tmp386;
    float tmp387;
    float tmp388;
    float phi_in389;
    float phi_out390;
    float tmp391;
    float tmp392;
    float tmp395;
    float tmp397;
    float tmp398;
    float tmp399;
    float tmp400;
    bool tmp401;
    float3 tmp403;
    float3 tmp404;
    float3 tmp405;
    float3 tmp406;
    float tmp408;
    float3 tmp409;
    float3 phi_in410;
    float3 phi_out411;
    float phi_in412;
    float phi_out413;
    float3 tmp415;
    float3 tmp416;
    float3 tmp417;
    float tmp418;
    float tmp419;
    float tmp420;
    float tmp421;
    float tmp422;
    float3 tmp423;
    float tmp425;
    float phi_in427;
    float phi_out428;
    float3 tmp433;
    float tmp435;
    float tmp449;
    float tmp451;
    float tmp452;
    float phi_in453;
    float phi_out454;
    float tmp456;
    float tmp457;
    bool tmp458;
    int tmp459;
    float tmp460;
    float tmp461;
    float tmp462;
    float tmp463;
    float3 tmp464;
    float phi_in466;
    float phi_out467;
    float tmp474;
    float tmp476;
    float tmp479;
    float phi_in480;
    float phi_out481;
    float tmp483;
    float tmp484;
    float phi_in485;
    float phi_out486;
    float tmp488;
    float tmp489;
    float tmp490;
    tmp4 = p_11.text_results[17].x;
    tmp5 = p_11.text_results[16].w;
    tmp6 = min(max(p_11.text_results[21].z, 0.0f), 1.0f);
    float3 tmp7 = p_11.geom_normal;
    tmp8 = p_00.k1.x;
    tmp9 = p_00.k1.y;
    tmp10 = p_00.k1.z;
    tmp11 = float3(tmp8, tmp9, tmp10);
    float3 tmp12 = tmp11 * tmp7;
    tmp13 = asfloat((asint(tmp12.x + tmp12.y + tmp12.z) & -2147483648) | 1065353216);
    tmp14 = p_11.text_results[21].y;
    tmp15 = p_11.text_results[21].x;
    tmp16 = max(tmp15, 1.000000012e-07f);
    tmp17 = max(tmp14, 1.000000012e-07f);
    tmp18 = tmp7.x * tmp13;
    tmp19 = tmp7.y * tmp13;
    tmp20 = tmp7.z * tmp13;
    float3 tmp21 = float3(p_11.text_results[22].z, p_11.text_results[22].w, p_11.text_results[23].x);
    tmp22 = float3(p_11.text_results[25].w * tmp13, p_11.text_results[26].x * tmp13, p_11.text_results[26].y * tmp13);
    tmp23 = tmp22.zxy;
    tmp24 = tmp22.yzx;
    tmp25 = tmp23 * tmp21.yzx - tmp24 * tmp21.zxy;
    float3 tmp26 = tmp25 * tmp25;
    tmp27 = tmp26.x + tmp26.y + tmp26.z;
    if (tmp27 < 9.999999939e-09f) {
        p_00.pdf = 0.0f;
        float3 tmp28 = tmp22 * tmp11;
        tmp29 = tmp28.x + tmp28.y + tmp28.z;
        phi_in = tmp29;
        phi_in30 = 0.0f;
    } else {
        float tmp32 = 1.0f / sqrt(tmp27);
        tmp33 = float3(tmp32 * tmp25.x, tmp32 * tmp25.y, tmp32 * tmp25.z);
        tmp34 = tmp33.zxy * tmp24 - tmp33.yzx * tmp23;
        if (p_00.ior1.x == -1.0f) {
            p_00.ior1.x = p_11.text_results[28].x;
            p_00.ior1.y = p_11.text_results[28].y;
            p_00.ior1.z = p_11.text_results[28].z;
        }
        if (p_00.ior2.x == -1.0f) {
            p_00.ior2.x = p_11.text_results[28].x;
            p_00.ior2.y = p_11.text_results[28].y;
            p_00.ior2.z = p_11.text_results[28].z;
        }
        float3 tmp35 = tmp22 * tmp11;
        tmp36 = tmp35.x + tmp35.y + tmp35.z;
        tmp37 = abs(tmp36);
        tmp38 = p_00.k2.x;
        tmp39 = p_00.k2.y;
        tmp40 = p_00.k2.z;
        tmp41 = float3(tmp38, tmp39, tmp40);
        float3 tmp42 = tmp41 * tmp22;
        tmp43 = abs(tmp42.x + tmp42.y + tmp42.z);
        float3 tmp44 = tmp41 * float3(tmp18, tmp19, tmp20);
        phi_in45 = 0.0f;
        if (!(tmp44.x + tmp44.y + tmp44.z < 0.0f)) {
            float tmp47 = tmp38 + tmp8;
            float tmp48 = tmp39 + tmp9;
            float tmp49 = tmp40 + tmp10;
            float3 tmp50 = float3(sqrt(tmp48 * tmp48 + tmp47 * tmp47 + tmp49 * tmp49), 0.0f, 0.0f);
            tmp51 = float3(tmp47, tmp48, tmp49) / tmp50.xxx;
            float3 tmp52 = tmp51 * tmp22;
            tmp53 = tmp52.x + tmp52.y + tmp52.z;
            float3 tmp54 = tmp51 * tmp11;
            float3 tmp55 = tmp51 * tmp41;
            phi_in45 = 0.0f;
            if (!(tmp55.x + tmp55.y + tmp55.z < 0.0f || (tmp53 < 0.0f || tmp54.x + tmp54.y + tmp54.z < 0.0f))) {
                float3 tmp56 = tmp51 * tmp34;
                float3 tmp57 = tmp51 * tmp33;
                float tmp58 = 1.0f / tmp16;
                float tmp59 = 1.0f / tmp17;
                float tmp60 = (tmp56.x + tmp56.y + tmp56.z) * tmp58;
                float tmp61 = (tmp57.x + tmp57.y + tmp57.z) * tmp59;
                float tmp62 = tmp60 * tmp60 + tmp53 * tmp53 + tmp61 * tmp61;
                float3 tmp63 = tmp34 * tmp11;
                float3 tmp64 = tmp33 * tmp11;
                float tmp65 = (tmp63.x + tmp63.y + tmp63.z) * tmp16;
                float tmp66 = (tmp64.x + tmp64.y + tmp64.z) * tmp17;
                tmp67 = tmp59 * 0.1591549367f * tmp58 * tmp53 / (tmp62 * tmp62 * ((sqrt((tmp65 * tmp65 + tmp66 * tmp66) / (tmp36 * tmp36) + 1.0f) + 1.0f) * tmp37 * tmp53));
                phi_in45 = tmp67;
            }
        }
        phi_out46 = phi_in45;
        float tmp68 = tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp37, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, min(max(sqrt(tmp15 * tmp14), 0.0f), 1.0f) * 0.984375f + 0.0078125f, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
        tmp69 = (0.3183098733f - tmp68 * 0.3183098733f) * tmp43 + tmp68 * phi_out46;
        p_00.pdf = tmp69;
        phi_in = tmp36;
        phi_in30 = tmp69;
    }
    phi_out = phi_in;
    phi_out31 = phi_in30;
    tmp70 = 1.0f - min(max(min(max(phi_out, 0.0f), 1.0f), 0.0f), 1.0f);
    tmp71 = tmp70 * tmp70;
    tmp72 = p_11.text_results[25].x;
    tmp73 = p_11.text_results[25].y;
    tmp74 = p_11.text_results[25].z;
    tmp75 = min(max(p_11.text_results[20].y, 0.0f), 1.0f);
    phi_in76 = 0.0f;
    if (tmp75 > 0.0f) {
        tmp78 = p_11.text_results[17].w;
        tmp79 = p_11.text_results[17].z;
        tmp80 = max(tmp79, 1.000000012e-07f);
        tmp81 = max(tmp78, 1.000000012e-07f);
        float3 tmp82 = p_11.text_results[23].yzw;
        tmp83 = float3(tmp72 * tmp13, tmp73 * tmp13, tmp74 * tmp13);
        tmp84 = tmp83.zxy;
        tmp85 = tmp83.yzx;
        tmp86 = tmp82.yzx * tmp84 - tmp82.zxy * tmp85;
        float3 tmp87 = tmp86 * tmp86;
        tmp88 = tmp87.x + tmp87.y + tmp87.z;
        phi_in89 = 0.0f;
        if (!(tmp88 < 9.999999939e-09f)) {
            float tmp91 = 1.0f / sqrt(tmp88);
            tmp92 = float3(tmp91 * tmp86.x, tmp91 * tmp86.y, tmp91 * tmp86.z);
            tmp93 = tmp92.zxy * tmp85 - tmp92.yzx * tmp84;
            if (p_00.ior1.x == -1.0f) {
                p_00.ior1.x = p_11.text_results[28].x;
                p_00.ior1.y = p_11.text_results[28].y;
                p_00.ior1.z = p_11.text_results[28].z;
            }
            if (p_00.ior2.x == -1.0f) {
                p_00.ior2.x = p_11.text_results[28].x;
                p_00.ior2.y = p_11.text_results[28].y;
                p_00.ior2.z = p_11.text_results[28].z;
            }
            float3 tmp94 = tmp83 * tmp11;
            tmp95 = tmp94.x + tmp94.y + tmp94.z;
            tmp96 = abs(tmp95);
            tmp97 = p_00.k2.x;
            tmp98 = p_00.k2.y;
            tmp99 = p_00.k2.z;
            tmp100 = float3(tmp97, tmp98, tmp99);
            float3 tmp101 = tmp100 * tmp83;
            tmp102 = abs(tmp101.x + tmp101.y + tmp101.z);
            float3 tmp103 = tmp100 * float3(tmp18, tmp19, tmp20);
            phi_in104 = 0.0f;
            if (!(tmp103.x + tmp103.y + tmp103.z < 0.0f)) {
                float tmp106 = tmp97 + tmp8;
                float tmp107 = tmp98 + tmp9;
                float tmp108 = tmp99 + tmp10;
                float3 tmp109 = float3(sqrt(tmp107 * tmp107 + tmp106 * tmp106 + tmp108 * tmp108), 0.0f, 0.0f);
                tmp110 = float3(tmp106, tmp107, tmp108) / tmp109.xxx;
                float3 tmp111 = tmp110 * tmp83;
                tmp112 = tmp111.x + tmp111.y + tmp111.z;
                float3 tmp113 = tmp110 * tmp11;
                float3 tmp114 = tmp110 * tmp100;
                phi_in104 = 0.0f;
                if (!(tmp114.x + tmp114.y + tmp114.z < 0.0f || (tmp112 < 0.0f || tmp113.x + tmp113.y + tmp113.z < 0.0f))) {
                    float3 tmp115 = tmp110 * tmp93;
                    float3 tmp116 = tmp110 * tmp92;
                    float tmp117 = 1.0f / tmp80;
                    float tmp118 = 1.0f / tmp81;
                    float tmp119 = (tmp115.x + tmp115.y + tmp115.z) * tmp117;
                    float tmp120 = (tmp116.x + tmp116.y + tmp116.z) * tmp118;
                    float tmp121 = tmp119 * tmp119 + tmp112 * tmp112 + tmp120 * tmp120;
                    float3 tmp122 = tmp93 * tmp11;
                    float3 tmp123 = tmp92 * tmp11;
                    float tmp124 = (tmp122.x + tmp122.y + tmp122.z) * tmp80;
                    float tmp125 = (tmp123.x + tmp123.y + tmp123.z) * tmp81;
                    tmp126 = tmp118 * 0.1591549367f * tmp117 * tmp112 / (tmp121 * tmp121 * ((sqrt((tmp124 * tmp124 + tmp125 * tmp125) / (tmp95 * tmp95) + 1.0f) + 1.0f) * tmp96 * tmp112));
                    phi_in104 = tmp126;
                }
            }
            phi_out105 = phi_in104;
            float tmp127 = tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp96, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, min(max(sqrt(tmp79 * tmp78), 0.0f), 1.0f) * 0.984375f + 0.0078125f, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
            tmp128 = (0.3183098733f - tmp127 * 0.3183098733f) * tmp102 + tmp127 * phi_out105;
            phi_in89 = tmp128;
        }
        phi_out90 = phi_in89;
        p_00.pdf = phi_out90;
        tmp129 = phi_out90 * tmp75;
        phi_in76 = tmp129;
    }
    phi_out77 = phi_in76;
    phi_in130 = phi_out77;
    if (tmp75 < 1.0f) {
        tmp132 = min(max(p_11.text_results[12].x, 0.0f), 1.0f);
        phi_in133 = 0.0f;
        if (tmp132 > 0.0f) {
            tmp135 = min(max(p_11.text_results[20].x, 0.0f), 1.0f);
            phi_in136 = 0.0f;
            if (tmp135 > 0.0f) {
                tmp138 = p_11.arg_block_offset;
                float3 tmp139 = float3(p_11.text_results[26].z, p_11.text_results[26].w, p_11.text_results[27].x) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                tmp140 = p_11.text_results[17].w;
                tmp141 = p_11.text_results[17].z;
                tmp142 = max(tmp141, 1.000000012e-07f);
                tmp143 = max(tmp140, 1.000000012e-07f);
                tmp3.m_0 = (p_11.text_results[30].z + p_11.text_results[30].y + p_11.text_results[30].w) * 0.3333333433f;
                tmp3.m_1 = mdl_read_argblock_as_bool(tmp138 + 236) ? p_11.text_results[19].w : 0.0f;
                tmp144 = tmp139.z > 0.0f || (tmp139.x > 0.0f || tmp139.y > 0.0f);
                tmp145 = tmp72 * tmp13;
                tmp146 = tmp73 * tmp13;
                tmp147 = tmp74 * tmp13;
                float3 tmp148 = p_11.text_results[23].yzw;
                tmp149 = float3(tmp145, tmp146, tmp147);
                tmp150 = tmp149.zxy;
                tmp151 = tmp149.yzx;
                tmp152 = tmp148.yzx * tmp150 - tmp148.zxy * tmp151;
                float3 tmp153 = tmp152 * tmp152;
                tmp154 = tmp153.x + tmp153.y + tmp153.z;
                phi_in155 = 0.0f;
                if (!(tmp154 < 9.999999939e-09f)) {
                    float tmp157 = 1.0f / sqrt(tmp154);
                    tmp158 = float3(tmp157 * tmp152.x, tmp157 * tmp152.y, tmp157 * tmp152.z);
                    tmp159 = tmp158.zxy * tmp151 - tmp158.yzx * tmp150;
                    tmp160 = p_00.ior1.x;
                    phi_in161 = tmp160;
                    if (tmp160 == -1.0f) {
                        tmp163 = p_11.text_results[28].x;
                        p_00.ior1.x = tmp163;
                        p_00.ior1.y = p_11.text_results[28].y;
                        p_00.ior1.z = p_11.text_results[28].z;
                        phi_in161 = tmp163;
                    }
                    phi_out162 = phi_in161;
                    tmp164 = p_00.ior2.x;
                    if (tmp164 == -1.0f) {
                        tmp165 = p_11.text_results[28].x;
                        tmp166 = p_11.text_results[28].y;
                        tmp167 = p_11.text_results[28].z;
                        p_00.ior2.x = tmp165;
                        p_00.ior2.y = tmp166;
                        p_00.ior2.z = tmp167;
                        phi_in168 = tmp167;
                        phi_in170 = tmp166;
                        phi_in172 = tmp165;
                    } else {
                        tmp174 = p_00.ior2.y;
                        tmp175 = p_00.ior2.z;
                        phi_in168 = tmp175;
                        phi_in170 = tmp174;
                        phi_in172 = tmp164;
                    }
                    phi_out169 = phi_in168;
                    phi_out171 = phi_in170;
                    phi_out173 = phi_in172;
                    tmp176 = (p_00.ior1.y + phi_out162 + p_00.ior1.z) * 0.3333333433f;
                    float tmp177 = (phi_out171 + phi_out169 + phi_out173) * 0.3333333433f;
                    float tmp178 = tmp177 - tmp176;
                    tmp179 = abs(tmp178) < 9.999999747e-05f ? tmp176 + asfloat((asint(tmp178) & -2147483648) | 953267991) : tmp177;
                    tmp2.m_0 = tmp176;
                    tmp2.m_1 = tmp179;
                    tmp180 = mdl_read_argblock_as_bool(tmp138);
                    float3 tmp181 = tmp149 * tmp11;
                    tmp182 = tmp181.x + tmp181.y + tmp181.z;
                    tmp183 = abs(tmp182);
                    tmp184 = p_00.k2.x;
                    tmp185 = p_00.k2.y;
                    tmp186 = p_00.k2.z;
                    tmp187 = float3(tmp184, tmp185, tmp186);
                    float3 tmp188 = tmp187 * tmp149;
                    tmp189 = abs(tmp188.x + tmp188.y + tmp188.z);
                    float3 tmp190 = tmp187 * float3(tmp18, tmp19, tmp20);
                    tmp191 = tmp190.x + tmp190.y + tmp190.z < 0.0f;
                    if (tmp191) {
                        if (tmp180) {
                            float tmp192 = tmp189 * 2.0f;
                            tmp193 = tmp184 + tmp8 + tmp192 * tmp145;
                            tmp194 = tmp185 + tmp9 + tmp192 * tmp146;
                            tmp195 = tmp186 + tmp10 + tmp192 * tmp147;
                            phi_in196 = tmp193;
                            phi_in198 = tmp194;
                            phi_in200 = tmp195;
                        } else {
                            tmp202 = tmp179 * tmp184 + tmp176 * tmp8;
                            tmp203 = tmp179 * tmp185 + tmp176 * tmp9;
                            tmp204 = tmp179 * tmp186 + tmp176 * tmp10;
                            phi_in196 = tmp202;
                            phi_in198 = tmp203;
                            phi_in200 = tmp204;
                            if (tmp179 > tmp176) {
                                tmp205 = -tmp204;
                                tmp206 = -tmp203;
                                tmp207 = -tmp202;
                                phi_in196 = tmp207;
                                phi_in198 = tmp206;
                                phi_in200 = tmp205;
                            }
                        }
                    }
                    else {
                        tmp208 = tmp184 + tmp8;
                        tmp209 = tmp185 + tmp9;
                        tmp210 = tmp186 + tmp10;
                        phi_in196 = tmp208;
                        phi_in198 = tmp209;
                        phi_in200 = tmp210;
                    }
                    phi_out197 = phi_in196;
                    phi_out199 = phi_in198;
                    phi_out201 = phi_in200;
                    float3 tmp211 = float3(sqrt(phi_out199 * phi_out199 + phi_out197 * phi_out197 + phi_out201 * phi_out201), 0.0f, 0.0f);
                    tmp212 = float3(phi_out197, phi_out199, phi_out201) / tmp211.xxx;
                    float3 tmp213 = tmp212 * tmp149;
                    tmp214 = tmp213.x + tmp213.y + tmp213.z;
                    float3 tmp215 = tmp212 * tmp11;
                    tmp216 = tmp215.x + tmp215.y + tmp215.z;
                    float3 tmp217 = tmp212 * tmp187;
                    float tmp218 = tmp217.x + tmp217.y + tmp217.z;
                    tmp219 = tmp191 ? -tmp218 : tmp218;
                    phi_in220 = 0.0f;
                    if (!(tmp214 < 0.0f || tmp216 < 0.0f || tmp219 < 0.0f)) {
                        tmp222 = _ZNK23Fresnel_function_coated4evalERK6float2f(tmp3, tmp2, tmp216).m_3;
                        float3 tmp223 = tmp212 * tmp159;
                        float3 tmp224 = tmp212 * tmp158;
                        float tmp225 = 1.0f / tmp142;
                        float tmp226 = 1.0f / tmp143;
                        float tmp227 = (tmp223.x + tmp223.y + tmp223.z) * tmp225;
                        float tmp228 = (tmp224.x + tmp224.y + tmp224.z) * tmp226;
                        float tmp229 = tmp227 * tmp227 + tmp214 * tmp214 + tmp228 * tmp228;
                        tmp230 = tmp226 * 0.3183098733f * tmp225 * tmp214 / (tmp229 * tmp229);
                        float3 tmp231 = tmp159 * tmp11;
                        float3 tmp232 = tmp158 * tmp11;
                        float tmp233 = (tmp231.x + tmp231.y + tmp231.z) * tmp142;
                        float tmp234 = (tmp232.x + tmp232.y + tmp232.z) * tmp143;
                        tmp235 = sqrt((tmp233 * tmp233 + tmp234 * tmp234) / (tmp182 * tmp182) + 1.0f) + 1.0f;
                        if (tmp191 && tmp180 != true) {
                            float tmp236 = tmp216 * tmp176 - tmp219 * tmp179;
                            tmp237 = tmp219 * tmp216;
                            tmp238 = tmp236 * tmp236 * (tmp214 * tmp183);
                            phi_in239 = tmp230;
                            phi_in241 = tmp237;
                            phi_in243 = tmp238;
                        } else {
                            tmp245 = tmp214 * tmp183;
                            phi_in239 = 0.25f;
                            phi_in241 = tmp230;
                            phi_in243 = tmp245;
                        }
                        phi_out240 = phi_in239;
                        phi_out242 = phi_in241;
                        phi_out244 = phi_in243;
                        tmp246 = (tmp191 ? 1.0f - tmp222 : tmp222) * 2.0f * phi_out240 * phi_out242 / (phi_out244 * tmp235);
                        phi_in220 = tmp246;
                    }
                    phi_out221 = phi_in220;
                    phi_in247 = phi_out221;
                    if (tmp144) {
                        tmp249 = tmp180 ? -1.0f : tmp176 / tmp179;
                        tmp250 = min(max(acos(min(max(tmp183, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f);
                        tmp251 = min(max(sqrt(tmp141 * tmp140), 0.0f), 1.0f);
                        phi_in252 = 0.0f;
                        if (!(tmp249 < 0.0f)) {
                            bool tmp254 = tmp249 < 1.0f;
                            tmp255 = (min(max((tmp254 ? 1.0f / tmp249 : tmp249) * 0.5333333611f + -0.6000000238f, 0.0f), 1.0f) * 15.0f + (tmp254 ? 1.5f : 17.5f)) * 0.03030303121f;
                            phi_in252 = tmp255;
                        }
                        phi_out253 = phi_in252;
                        float tmp256 = tex_lookup_float3_3d(1, float3(tmp250 * 0.9692307711f + 0.007692307699f, tmp251 * 0.984375f + 0.0078125f, phi_out253), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                        tmp257 = (0.3183098733f - tmp256 * 0.3183098733f) * tmp189 + tmp256 * phi_out221;
                        phi_in247 = tmp257;
                    }
                    phi_out248 = phi_in247;
                    phi_in155 = phi_out248;
                }
                phi_out156 = phi_in155;
                tmp258 = phi_out156 * tmp135;
                phi_in136 = tmp258;
            }
            phi_out137 = phi_in136;
            phi_in259 = phi_out137;
            if (tmp135 < 1.0f) {
                tmp261 = p_11.normal;
                float tmp262 = p_11.text_results[17].y;
                tmp263 = min(max(tmp262 > 0.349999994f ? tmp262 + -0.349999994f : 0.0f, 0.0f), 1.0f);
                phi_in264 = 0.0f;
                if (tmp263 > 0.0f) {
                    tmp266 = p_00.k2;
                    tmp267 = float3(tmp18, tmp19, tmp20);
                    float3 tmp268 = tmp266 * tmp267;
                    phi_in269 = 0.0f;
                    if (tmp268.x + tmp268.y + tmp268.z < 0.0f) {
                        float3 tmp271 = float3(tmp261.x * tmp13, tmp261.y * tmp13, tmp261.z * tmp13);
                        float3 tmp272 = tmp271 * tmp267;
                        float3 tmp273 = tmp266 * tmp271;
                        tmp274 = max(-(tmp273.x + tmp273.y + tmp273.z), 0.0f) / (max(tmp272.x + tmp272.y + tmp272.z, 0.0f) * 1.570796371f + 1.570796371f);
                        phi_in269 = tmp274;
                    }
                    phi_out270 = phi_in269;
                    tmp275 = phi_out270 * tmp263;
                    phi_in264 = tmp275;
                }
                phi_out265 = phi_in264;
                phi_in276 = phi_out265;
                if (tmp263 < 1.0f) {
                    tmp278 = max(p_11.text_results[17].z, 1.000000012e-07f);
                    tmp279 = max(p_11.text_results[17].w, 1.000000012e-07f);
                    tmp280 = tmp72 * tmp13;
                    tmp281 = tmp73 * tmp13;
                    tmp282 = tmp74 * tmp13;
                    float3 tmp283 = p_11.text_results[23].yzw;
                    tmp284 = float3(tmp280, tmp281, tmp282);
                    tmp285 = tmp284.zxy;
                    tmp286 = tmp284.yzx;
                    tmp287 = tmp283.yzx * tmp285 - tmp283.zxy * tmp286;
                    float3 tmp288 = tmp287 * tmp287;
                    tmp289 = tmp288.x + tmp288.y + tmp288.z;
                    phi_in290 = 0.0f;
                    if (!(tmp289 < 9.999999939e-09f)) {
                        float tmp292 = 1.0f / sqrt(tmp289);
                        tmp293 = float3(tmp292 * tmp287.x, tmp292 * tmp287.y, tmp292 * tmp287.z);
                        tmp294 = tmp293.zxy * tmp286 - tmp293.yzx * tmp285;
                        tmp295 = p_00.ior1.x;
                        phi_in296 = tmp295;
                        if (tmp295 == -1.0f) {
                            tmp298 = p_11.text_results[28].x;
                            p_00.ior1.x = tmp298;
                            p_00.ior1.y = p_11.text_results[28].y;
                            p_00.ior1.z = p_11.text_results[28].z;
                            phi_in296 = tmp298;
                        }
                        phi_out297 = phi_in296;
                        tmp299 = p_00.ior2.x;
                        if (tmp299 == -1.0f) {
                            tmp300 = p_11.text_results[28].x;
                            tmp301 = p_11.text_results[28].y;
                            tmp302 = p_11.text_results[28].z;
                            p_00.ior2.x = tmp300;
                            p_00.ior2.y = tmp301;
                            p_00.ior2.z = tmp302;
                            phi_in303 = tmp302;
                            phi_in305 = tmp301;
                            phi_in307 = tmp300;
                        } else {
                            tmp309 = p_00.ior2.y;
                            tmp310 = p_00.ior2.z;
                            phi_in303 = tmp310;
                            phi_in305 = tmp309;
                            phi_in307 = tmp299;
                        }
                        phi_out304 = phi_in303;
                        phi_out306 = phi_in305;
                        phi_out308 = phi_in307;
                        tmp311 = (p_00.ior1.y + phi_out297 + p_00.ior1.z) * 0.3333333433f;
                        float tmp312 = (phi_out306 + phi_out304 + phi_out308) * 0.3333333433f;
                        float tmp313 = tmp312 - tmp311;
                        tmp314 = abs(tmp313) < 9.999999747e-05f ? tmp311 + asfloat((asint(tmp313) & -2147483648) | 953267991) : tmp312;
                        tmp315 = mdl_read_argblock_as_bool(p_11.arg_block_offset);
                        float3 tmp316 = tmp284 * tmp11;
                        tmp317 = tmp316.x + tmp316.y + tmp316.z;
                        tmp318 = abs(tmp317);
                        tmp319 = p_00.k2.x;
                        tmp320 = p_00.k2.y;
                        tmp321 = p_00.k2.z;
                        tmp322 = float3(tmp319, tmp320, tmp321);
                        float3 tmp323 = tmp322 * tmp284;
                        tmp324 = abs(tmp323.x + tmp323.y + tmp323.z);
                        float3 tmp325 = tmp322 * float3(tmp18, tmp19, tmp20);
                        tmp326 = tmp325.x + tmp325.y + tmp325.z < 0.0f;
                        if (tmp326) {
                            if (tmp315) {
                                float tmp327 = tmp324 * 2.0f;
                                tmp328 = tmp319 + tmp8 + tmp327 * tmp280;
                                tmp329 = tmp320 + tmp9 + tmp327 * tmp281;
                                tmp330 = tmp321 + tmp10 + tmp327 * tmp282;
                                phi_in331 = tmp328;
                                phi_in333 = tmp329;
                                phi_in335 = tmp330;
                            } else {
                                tmp337 = tmp314 * tmp319 + tmp311 * tmp8;
                                tmp338 = tmp314 * tmp320 + tmp311 * tmp9;
                                tmp339 = tmp314 * tmp321 + tmp311 * tmp10;
                                phi_in331 = tmp337;
                                phi_in333 = tmp338;
                                phi_in335 = tmp339;
                                if (tmp314 > tmp311) {
                                    tmp340 = -tmp339;
                                    tmp341 = -tmp338;
                                    tmp342 = -tmp337;
                                    phi_in331 = tmp342;
                                    phi_in333 = tmp341;
                                    phi_in335 = tmp340;
                                }
                            }
                        }
                        else {
                            tmp343 = tmp319 + tmp8;
                            tmp344 = tmp320 + tmp9;
                            tmp345 = tmp321 + tmp10;
                            phi_in331 = tmp343;
                            phi_in333 = tmp344;
                            phi_in335 = tmp345;
                        }
                        phi_out332 = phi_in331;
                        phi_out334 = phi_in333;
                        phi_out336 = phi_in335;
                        float3 tmp346 = float3(sqrt(phi_out334 * phi_out334 + phi_out332 * phi_out332 + phi_out336 * phi_out336), 0.0f, 0.0f);
                        tmp347 = float3(phi_out332, phi_out334, phi_out336) / tmp346.xxx;
                        float3 tmp348 = tmp347 * tmp284;
                        tmp349 = tmp348.x + tmp348.y + tmp348.z;
                        float3 tmp350 = tmp347 * tmp11;
                        tmp351 = tmp350.x + tmp350.y + tmp350.z;
                        float3 tmp352 = tmp347 * tmp322;
                        float tmp353 = tmp352.x + tmp352.y + tmp352.z;
                        tmp354 = tmp326 ? -tmp353 : tmp353;
                        phi_in290 = 0.0f;
                        if (!(tmp349 < 0.0f || tmp351 < 0.0f || tmp354 < 0.0f)) {
                            phi_in355 = 0;
                            phi_in357 = 0.0f;
                            phi_in359 = 0.0f;
                            if (!tmp326) {
                                float tmp361 = tmp311 / tmp314;
                                phi_in355 = 0;
                                phi_in357 = 1.0f;
                                phi_in359 = 0.0f;
                                if (!(tmp361 * tmp361 * (1.0f - tmp351 * tmp351) > 1.0f)) {
                                    phi_in355 = 1;
                                    phi_in357 = 0.0f;
                                    phi_in359 = 0.0f;
                                }
                            }
                            phi_out356 = phi_in355;
                            phi_out358 = phi_in357;
                            phi_out360 = phi_in359;
                            phi_in290 = phi_out360;
                            if (phi_out356 == 0) {
                                float3 tmp362 = tmp347 * tmp294;
                                float3 tmp363 = tmp347 * tmp293;
                                float tmp364 = 1.0f / tmp278;
                                float tmp365 = 1.0f / tmp279;
                                float tmp366 = (tmp362.x + tmp362.y + tmp362.z) * tmp364;
                                float tmp367 = (tmp363.x + tmp363.y + tmp363.z) * tmp365;
                                float tmp368 = tmp366 * tmp366 + tmp349 * tmp349 + tmp367 * tmp367;
                                tmp369 = tmp365 * 0.3183098733f * tmp364 * tmp349 / (tmp368 * tmp368);
                                float3 tmp370 = tmp294 * tmp11;
                                float3 tmp371 = tmp293 * tmp11;
                                float tmp372 = (tmp370.x + tmp370.y + tmp370.z) * tmp278;
                                float tmp373 = (tmp371.x + tmp371.y + tmp371.z) * tmp279;
                                tmp374 = sqrt((tmp372 * tmp372 + tmp373 * tmp373) / (tmp317 * tmp317) + 1.0f) + 1.0f;
                                if (tmp326 && tmp315 != true) {
                                    float tmp375 = tmp351 * tmp311 - tmp354 * tmp314;
                                    tmp376 = tmp354 * tmp351;
                                    tmp377 = tmp375 * tmp375 * (tmp349 * tmp318);
                                    phi_in378 = tmp369;
                                    phi_in380 = tmp376;
                                    phi_in382 = tmp377;
                                } else {
                                    tmp384 = tmp349 * tmp318;
                                    phi_in378 = 0.25f;
                                    phi_in380 = tmp369;
                                    phi_in382 = tmp384;
                                }
                                phi_out379 = phi_in378;
                                phi_out381 = phi_in380;
                                phi_out383 = phi_in382;
                                tmp385 = (tmp326 ? 1.0f - phi_out358 : phi_out358) * 2.0f * phi_out379 * phi_out381 / (phi_out383 * tmp374);
                                phi_in290 = tmp385;
                            }
                        }
                    }
                    phi_out291 = phi_in290;
                    tmp386 = phi_out291 * (1.0f - tmp263) + phi_out265;
                    phi_in276 = tmp386;
                }
                phi_out277 = phi_in276;
                tmp387 = phi_out277 * (1.0f - tmp135) + phi_out137;
                phi_in259 = tmp387;
            }
            phi_out260 = phi_in259;
            tmp388 = phi_out260 * tmp132;
            phi_in133 = tmp388;
        }
        phi_out134 = phi_in133;
        phi_in389 = phi_out134;
        if (tmp132 < 1.0f) {
            tmp391 = p_11.text_results[20].x;
            tmp392 = p_11.text_results[17].y;
            float tmp393 = p_11.text_results[24].w;
            float tmp394 = (tmp393 + -1.0f) / (tmp393 + 1.0f);
            tmp395 = min(max(tmp394 * tmp394, 0.0f), 1.0f);
            float3 tmp396 = float3(p_11.text_results[26].z, p_11.text_results[26].w, p_11.text_results[27].x) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
            tmp397 = p_11.text_results[17].w;
            tmp398 = p_11.text_results[17].z;
            tmp399 = max(tmp398, 1.000000012e-07f);
            tmp400 = max(tmp397, 1.000000012e-07f);
            tmp401 = tmp396.z > 0.0f || (tmp396.x > 0.0f || tmp396.y > 0.0f);
            float3 tmp402 = p_11.text_results[23].yzw;
            tmp403 = float3(tmp72 * tmp13, tmp73 * tmp13, tmp74 * tmp13);
            tmp404 = tmp403.zxy;
            tmp405 = tmp403.yzx;
            tmp406 = tmp402.yzx * tmp404 - tmp402.zxy * tmp405;
            float3 tmp407 = tmp406 * tmp406;
            tmp408 = tmp407.x + tmp407.y + tmp407.z;
            if (tmp408 < 9.999999939e-09f) {
                p_00.pdf = 0.0f;
                tmp409 = tmp403 * tmp11;
                phi_in410 = tmp409;
                phi_in412 = 0.0f;
            } else {
                float tmp414 = 1.0f / sqrt(tmp408);
                tmp415 = float3(tmp414 * tmp406.x, tmp414 * tmp406.y, tmp414 * tmp406.z);
                tmp416 = tmp415.zxy * tmp405 - tmp415.yzx * tmp404;
                if (p_00.ior1.x == -1.0f) {
                    p_00.ior1.x = p_11.text_results[28].x;
                    p_00.ior1.y = p_11.text_results[28].y;
                    p_00.ior1.z = p_11.text_results[28].z;
                }
                if (p_00.ior2.x == -1.0f) {
                    p_00.ior2.x = p_11.text_results[28].x;
                    p_00.ior2.y = p_11.text_results[28].y;
                    p_00.ior2.z = p_11.text_results[28].z;
                }
                tmp417 = tmp403 * tmp11;
                tmp418 = tmp417.x + tmp417.y + tmp417.z;
                tmp419 = abs(tmp418);
                tmp420 = p_00.k2.x;
                tmp421 = p_00.k2.y;
                tmp422 = p_00.k2.z;
                tmp423 = float3(tmp420, tmp421, tmp422);
                float3 tmp424 = tmp423 * tmp403;
                tmp425 = abs(tmp424.x + tmp424.y + tmp424.z);
                float3 tmp426 = tmp423 * float3(tmp18, tmp19, tmp20);
                if (tmp426.x + tmp426.y + tmp426.z < 0.0f) {
                    p_00.pdf = 0.0f;
                    phi_in427 = 0.0f;
                } else {
                    float tmp429 = tmp420 + tmp8;
                    float tmp430 = tmp421 + tmp9;
                    float tmp431 = tmp422 + tmp10;
                    float3 tmp432 = float3(sqrt(tmp430 * tmp430 + tmp429 * tmp429 + tmp431 * tmp431), 0.0f, 0.0f);
                    tmp433 = float3(tmp429, tmp430, tmp431) / tmp432.xxx;
                    float3 tmp434 = tmp433 * tmp403;
                    tmp435 = tmp434.x + tmp434.y + tmp434.z;
                    float3 tmp436 = tmp433 * tmp11;
                    float3 tmp437 = tmp433 * tmp423;
                    if (tmp437.x + tmp437.y + tmp437.z < 0.0f || (tmp435 < 0.0f || tmp436.x + tmp436.y + tmp436.z < 0.0f)) {
                        p_00.pdf = 0.0f;
                        phi_in427 = 0.0f;
                    } else {
                        float3 tmp438 = tmp433 * tmp416;
                        float3 tmp439 = tmp433 * tmp415;
                        float tmp440 = 1.0f / tmp399;
                        float tmp441 = 1.0f / tmp400;
                        float tmp442 = (tmp438.x + tmp438.y + tmp438.z) * tmp440;
                        float tmp443 = (tmp439.x + tmp439.y + tmp439.z) * tmp441;
                        float tmp444 = tmp442 * tmp442 + tmp435 * tmp435 + tmp443 * tmp443;
                        float3 tmp445 = tmp416 * tmp11;
                        float3 tmp446 = tmp415 * tmp11;
                        float tmp447 = (tmp445.x + tmp445.y + tmp445.z) * tmp399;
                        float tmp448 = (tmp446.x + tmp446.y + tmp446.z) * tmp400;
                        tmp449 = tmp441 * 0.1591549367f * tmp440 * tmp435 / (tmp444 * tmp444 * ((sqrt((tmp447 * tmp447 + tmp448 * tmp448) / (tmp418 * tmp418) + 1.0f) + 1.0f) * tmp419 * tmp435));
                        p_00.pdf = tmp449;
                        phi_in427 = tmp449;
                    }
                }
                phi_out428 = phi_in427;
                phi_in410 = tmp417;
                phi_in412 = phi_out428;
                if (tmp401) {
                    float tmp450 = tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp419, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, min(max(sqrt(tmp398 * tmp397), 0.0f), 1.0f) * 0.984375f + 0.0078125f, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                    tmp451 = (0.3183098733f - tmp450 * 0.3183098733f) * tmp425 + tmp450 * phi_out428;
                    p_00.pdf = tmp451;
                    phi_in410 = tmp417;
                    phi_in412 = tmp451;
                }
            }
            phi_out411 = phi_in410;
            phi_out413 = phi_in412;
            tmp452 = min(max(p_11.text_results[20].z, 0.0f), 1.0f);
            phi_in453 = 0.0f;
            if (tmp452 > 0.0f) {
                float3 tmp455 = p_11.text_results[27].yzw * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                tmp456 = p_11.text_results[19].z;
                tmp457 = 1.0f / max(tmp456, 1.000000012e-07f);
                tmp458 = tmp455.z > 0.0f || (tmp455.x > 0.0f || tmp455.y > 0.0f);
                tmp459 = tmp458 ? 2 : 0;
                if (p_00.ior1.x == -1.0f) {
                    p_00.ior1.x = p_11.text_results[28].x;
                    p_00.ior1.y = p_11.text_results[28].y;
                    p_00.ior1.z = p_11.text_results[28].z;
                }
                if (p_00.ior2.x == -1.0f) {
                    p_00.ior2.x = p_11.text_results[28].x;
                    p_00.ior2.y = p_11.text_results[28].y;
                    p_00.ior2.z = p_11.text_results[28].z;
                }
                tmp460 = abs(phi_out411.x + phi_out411.y + phi_out411.z);
                tmp461 = p_00.k2.x;
                tmp462 = p_00.k2.y;
                tmp463 = p_00.k2.z;
                tmp464 = float3(tmp461, tmp462, tmp463);
                float3 tmp465 = tmp464 * float3(tmp18, tmp19, tmp20);
                if (tmp465.x + tmp465.y + tmp465.z < 0.0f) {
                    p_00.pdf = 0.0f;
                    phi_in466 = 0.0f;
                } else {
                    float tmp468 = tmp461 + tmp8;
                    float tmp469 = tmp462 + tmp9;
                    float tmp470 = tmp463 + tmp10;
                    float3 tmp471 = float3(sqrt(tmp469 * tmp469 + tmp468 * tmp468 + tmp470 * tmp470), 0.0f, 0.0f);
                    float3 tmp472 = float3(tmp468, tmp469, tmp470) / tmp471.xxx;
                    float3 tmp473 = tmp472 * tmp403;
                    tmp474 = tmp473.x + tmp473.y + tmp473.z;
                    float3 tmp475 = tmp472 * tmp11;
                    tmp476 = tmp475.x + tmp475.y + tmp475.z;
                    float3 tmp477 = tmp472 * tmp464;
                    if (tmp477.x + tmp477.y + tmp477.z < 0.0f || (tmp474 < 0.0f || tmp476 < 0.0f)) {
                        p_00.pdf = 0.0f;
                        phi_in466 = 0.0f;
                    } else {
                        float tmp478 = tmp474 * tmp460;
                        tmp479 = tmp474 * (tmp457 * 0.03978873417f + 0.07957746834f) * min(tmp478 * 2.0f / tmp476, 1.0f) * pow(sqrt(max(1.0f - tmp474 * tmp474, 0.0f)), tmp457) / tmp478;
                        p_00.pdf = tmp479;
                        phi_in466 = tmp479;
                    }
                }
                phi_out467 = phi_in466;
                phi_in480 = phi_out467;
                if (tmp458) {
                    gen_weighted_layer_pdf(p_00, p_11);
                    float tmp482 = p_00.pdf;
                    tmp483 = (phi_out467 - tmp482) * tex_lookup_float3_3d(tmp459, float3(min(max(acos(min(max(tmp460, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, min(max(abs(tmp456), 0.0f), 1.0f) * 0.984375f + 0.0078125f, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x + tmp482;
                    p_00.pdf = tmp483;
                    phi_in480 = tmp483;
                }
                phi_out481 = phi_in480;
                tmp484 = phi_out481 * tmp452;
                phi_in453 = tmp484;
            }
            phi_out454 = phi_in453;
            phi_in485 = phi_out454;
            if (tmp452 < 1.0f) {
                gen_weighted_layer_pdf(p_00, p_11);
                float tmp487 = p_00.pdf;
                tmp488 = tmp487 * (1.0f - tmp452) + phi_out454;
                phi_in485 = tmp488;
            }
            phi_out486 = phi_in485;
            tmp489 = ((phi_out413 - phi_out486) * min(max(tmp391, 0.0f), 1.0f) * (pow(1.0f - min(max(min(max(phi_out411.x + phi_out411.y + phi_out411.z, 0.0f), 1.0f), 0.0f), 1.0f), 4.0f) * (min(max(1.0f - tmp392 * tmp392, 0.0f), 1.0f) - tmp395) + tmp395) + phi_out486) * (1.0f - tmp132) + phi_out134;
            phi_in389 = tmp489;
        }
        phi_out390 = phi_in389;
        tmp490 = phi_out390 * (1.0f - tmp75) + phi_out77;
        phi_in130 = tmp490;
    }
    phi_out131 = phi_in130;
    p_00.pdf = (tmp71 * tmp71 * (tmp70 * (min(max(1.0f - tmp5, 0.0f), 1.0f) - tmp6)) + tmp6) * min(max(tmp4, 0.0f), 1.0f) * (phi_out31 - phi_out131) + phi_out131;
    return;
}

public void mdl_surface_scattering_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    Bsdf_pdf_data pdf_data;
    float tmp0;
    float tmp1;
    float3 tmp2;
    float tmp3;
    float tmp4;
    float tmp5;
    float3 tmp6;
    float tmp8;
    float tmp9;
    float tmp10;
    float tmp11;
    float3 tmp12;
    float tmp14;
    float tmp17;
    float tmp18;
    float tmp19;
    float tmp20;
    bool tmp21;
    float tmp22;
    float phi_in;
    float phi_out;
    float tmp23;
    float tmp24;
    float tmp25;
    float tmp26;
    float tmp27;
    float tmp28;
    float tmp29;
    float tmp30;
    float3 tmp32;
    float3 tmp33;
    float3 tmp34;
    float tmp36;
    int phi_in37;
    int phi_out38;
    float tmp40;
    float tmp41;
    float tmp42;
    float3 tmp43;
    float3 tmp44;
    float tmp47;
    float tmp48;
    float tmp51;
    float tmp52;
    float tmp53;
    float3 tmp55;
    float phi_in56;
    float phi_out57;
    float phi_in58;
    float phi_out59;
    float phi_in60;
    float phi_out61;
    float tmp69;
    float tmp70;
    float tmp71;
    float tmp72;
    float phi_in73;
    float phi_out74;
    float phi_in75;
    float phi_out76;
    float tmp77;
    int phi_in86;
    int phi_out87;
    bool phi_in88;
    bool phi_out89;
    float tmp90;
    float tmp91;
    float tmp92;
    float tmp94;
    float3 tmp99;
    float tmp105;
    float phi_in106;
    float phi_out107;
    float tmp108;
    float tmp109;
    float tmp110;
    int tmp127;
    float tmp128;
    float tmp129;
    float tmp130;
    float tmp131;
    float tmp132;
    float tmp133;
    float phi_in134;
    float phi_out135;
    float phi_in136;
    float phi_out137;
    float phi_in138;
    float phi_out139;
    float phi_in140;
    float phi_out141;
    float phi_in142;
    float phi_out143;
    float phi_in144;
    float phi_out145;
    float tmp146;
    float tmp147;
    float tmp148;
    float tmp150;
    float tmp151;
    float tmp152;
    float tmp153;
    float tmp154;
    float phi_in155;
    float phi_out156;
    float phi_in157;
    float phi_out158;
    float phi_in159;
    float phi_out160;
    float phi_in161;
    float phi_out162;
    float phi_in163;
    float phi_out164;
    float tmp166;
    float tmp167;
    float tmp168;
    float tmp169;
    float tmp170;
    float tmp171;
    float3 tmp174;
    float tmp177;
    float tmp178;
    float phi_in179;
    float phi_out180;
    float phi_in181;
    float phi_out182;
    float tmp185;
    float tmp186;
    tmp0 = min(max(state.text_results[21].z, 0.0f), 1.0f);
    tmp1 = min(max(state.text_results[17].x, 0.0f), 1.0f);
    tmp2 = state.geom_normal;
    tmp3 = sret_ptr.k1.x;
    tmp4 = sret_ptr.k1.y;
    tmp5 = sret_ptr.k1.z;
    tmp6 = float3(tmp3, tmp4, tmp5);
    float3 tmp7 = tmp6 * tmp2;
    tmp8 = asfloat((asint(tmp7.x + tmp7.y + tmp7.z) & -2147483648) | 1065353216);
    tmp9 = state.text_results[25].w * tmp8;
    tmp10 = state.text_results[26].x * tmp8;
    tmp11 = state.text_results[26].y * tmp8;
    tmp12 = float3(tmp9, tmp10, tmp11);
    float3 tmp13 = tmp12 * tmp6;
    tmp14 = tmp13.x + tmp13.y + tmp13.z;
    float tmp15 = 1.0f - min(max(min(max(tmp14, 0.0f), 1.0f), 0.0f), 1.0f);
    float tmp16 = tmp15 * tmp15;
    tmp17 = min(max(1.0f - state.text_results[16].w, 0.0f), 1.0f) - tmp0;
    tmp18 = tmp16 * tmp16 * (tmp15 * tmp17) + tmp0;
    tmp19 = tmp18 * tmp1;
    tmp20 = sret_ptr.xi.z;
    tmp21 = tmp20 < tmp19;
    if (tmp21) {
        tmp22 = tmp20 / tmp19;
        phi_in = tmp22;
    } else {
        tmp23 = (1.0f - tmp20) / (1.0f - tmp19);
        phi_in = tmp23;
    }
    phi_out = phi_in;
    sret_ptr.xi.z = phi_out;
    if (tmp21) {
        tmp24 = state.text_results[21].y;
        tmp25 = state.text_results[21].x;
        tmp26 = max(tmp25, 1.000000012e-07f);
        tmp27 = max(tmp24, 1.000000012e-07f);
        tmp28 = tmp2.x * tmp8;
        tmp29 = tmp2.y * tmp8;
        tmp30 = tmp2.z * tmp8;
        float3 tmp31 = float3(state.text_results[22].z, state.text_results[22].w, state.text_results[23].x);
        tmp32 = tmp12.zxy;
        tmp33 = tmp12.yzx;
        tmp34 = tmp31.yzx * tmp32 - tmp31.zxy * tmp33;
        float3 tmp35 = tmp34 * tmp34;
        tmp36 = tmp35.x + tmp35.y + tmp35.z;
        if (tmp36 < 9.999999939e-09f) {
            sret_ptr.pdf = 0.0f;
            sret_ptr.event_type = 0;
            phi_in37 = 0;
        } else {
            float tmp39 = 1.0f / sqrt(tmp36);
            tmp40 = tmp39 * tmp34.x;
            tmp41 = tmp39 * tmp34.y;
            tmp42 = tmp39 * tmp34.z;
            tmp43 = float3(tmp40, tmp41, tmp42);
            tmp44 = tmp43.zxy * tmp33 - tmp43.yzx * tmp32;
            if (sret_ptr.ior1.x == -1.0f) {
                sret_ptr.ior1.x = state.text_results[28].x;
                sret_ptr.ior1.y = state.text_results[28].y;
                sret_ptr.ior1.z = state.text_results[28].z;
            }
            if (sret_ptr.ior2.x == -1.0f) {
                sret_ptr.ior2.x = state.text_results[28].x;
                sret_ptr.ior2.y = state.text_results[28].y;
                sret_ptr.ior2.z = state.text_results[28].z;
            }
            float3 tmp45 = tmp44 * tmp6;
            float3 tmp46 = tmp43 * tmp6;
            tmp47 = sret_ptr.xi.x;
            tmp48 = sret_ptr.xi.y;
            float tmp49 = (tmp45.x + tmp45.y + tmp45.z) * tmp26;
            float tmp50 = (tmp46.x + tmp46.y + tmp46.z) * tmp27;
            tmp51 = tmp14 * tmp14;
            tmp52 = tmp50 * tmp50;
            tmp53 = tmp49 * tmp49 + tmp52;
            float3 tmp54 = float3(sqrt(tmp53 + tmp51), 0.0f, 0.0f);
            tmp55 = float3(tmp49, abs(tmp14), tmp50) / tmp54.xxx;
            phi_in56 = 1.0f;
            phi_in58 = 0.0f;
            phi_in60 = 0.0f;
            if (tmp55.y < 0.9999899864f) {
                float3 tmp62 = tmp55 * float3(1.0f, 0.0f, 0.0f);
                float3 tmp63 = tmp55 * float3(0.0f, 0.0f, 1.0f);
                float3 tmp64 = tmp62.yzx - tmp63.zxy;
                float3 tmp65 = float3(sqrt(tmp64.x * tmp64.x + tmp64.y * tmp64.y + tmp64.z * tmp64.z), 0.0f, 0.0f);
                float3 tmp66 = tmp64 / tmp65.xxx;
                phi_in56 = tmp66.x;
                phi_in58 = tmp66.y;
                phi_in60 = tmp66.z;
            }
            phi_out57 = phi_in56;
            phi_out59 = phi_in58;
            phi_out61 = phi_in60;
            float3 tmp67 = float3(phi_out57, phi_out59, phi_out61);
            float3 tmp68 = tmp67.yzx * tmp55.zxy - tmp67.zxy * tmp55.yzx;
            tmp69 = tmp55.y + 1.0f;
            tmp70 = 1.0f / tmp69;
            tmp71 = sqrt(tmp47);
            if (tmp48 < tmp70) {
                tmp72 = tmp69 * 3.141592741f * tmp48;
                phi_in73 = 1.0f;
                phi_in75 = tmp72;
            } else {
                tmp77 = (tmp48 - tmp70) * 3.141592741f / (1.0f - tmp70) + 3.141592741f;
                phi_in73 = tmp55.y;
                phi_in75 = tmp77;
            }
            phi_out74 = phi_in73;
            phi_out76 = phi_in75;
            float tmp78 = cos(phi_out76) * tmp71;
            float tmp79 = phi_out74 * tmp71 * sin(phi_out76);
            float tmp80 = sqrt(max(1.0f - (tmp79 * tmp79 + tmp78 * tmp78), 0.0f));
            float tmp81 = (tmp79 * tmp68.x + tmp78 * phi_out57 + tmp80 * tmp55.x) * tmp26;
            float tmp82 = max(tmp79 * tmp68.y + tmp78 * phi_out59 + tmp80 * tmp55.y, 0.0f);
            float tmp83 = (tmp79 * tmp68.z + tmp78 * phi_out61 + tmp80 * tmp55.z) * tmp27;
            float3 tmp84 = float3(sqrt(tmp81 * tmp81 + tmp82 * tmp82 + tmp83 * tmp83), 0.0f, 0.0f);
            float3 tmp85 = float3(tmp81, tmp82, tmp83) / tmp84.xxx;
            if (tmp85.y == 0.0f) {
                sret_ptr.pdf = 0.0f;
                sret_ptr.event_type = 0;
                phi_in86 = 0;
                phi_in88 = true;
            } else {
                tmp90 = tmp85.x * tmp44.x + tmp85.y * tmp9 + tmp85.z * tmp40;
                tmp91 = tmp85.x * tmp44.y + tmp85.y * tmp10 + tmp85.z * tmp41;
                tmp92 = tmp85.x * tmp44.z + tmp85.y * tmp11 + tmp85.z * tmp42;
                float3 tmp93 = float3(tmp90, tmp91, tmp92) * tmp6;
                tmp94 = tmp93.x + tmp93.y + tmp93.z;
                if (tmp94 > 0.0f) {
                    float tmp95 = tmp94 * 2.0f;
                    float tmp96 = tmp95 * tmp90 - tmp3;
                    float tmp97 = tmp95 * tmp91 - tmp4;
                    float tmp98 = tmp95 * tmp92 - tmp5;
                    sret_ptr.k2.x = tmp96;
                    sret_ptr.k2.y = tmp97;
                    sret_ptr.k2.z = tmp98;
                    sret_ptr.event_type = 10;
                    sret_ptr.bsdf_over_pdf.x = 1.0f;
                    sret_ptr.bsdf_over_pdf.y = 1.0f;
                    sret_ptr.bsdf_over_pdf.z = 1.0f;
                    tmp99 = float3(tmp96, tmp97, tmp98);
                    float3 tmp100 = tmp99 * float3(tmp28, tmp29, tmp30);
                    if (tmp100.x + tmp100.y + tmp100.z > 0.0f) {
                        float3 tmp101 = tmp99 * tmp12;
                        float tmp102 = tmp101.x + tmp101.y + tmp101.z;
                        float3 tmp103 = tmp99 * tmp44;
                        float tmp104 = (tmp103.x + tmp103.y + tmp103.z) * tmp26;
                        tmp105 = 2.0f / (sqrt((tmp104 * tmp104 + tmp52) / (tmp102 * tmp102) + 1.0f) + 1.0f);
                        if (tmp105 * 2.0f / (sqrt(tmp53 / tmp51 + 1.0f) + 1.0f) > 0.0f) {
                            sret_ptr.bsdf_over_pdf.x = tmp105;
                            sret_ptr.bsdf_over_pdf.y = tmp105;
                            sret_ptr.bsdf_over_pdf.z = tmp105;
                            sret_ptr.handle = 0;
                            phi_in86 = 10;
                            phi_in88 = false;
                        } else {
                            sret_ptr.pdf = 0.0f;
                            sret_ptr.event_type = 0;
                            phi_in86 = 0;
                            phi_in88 = true;
                        }
                    } else {
                        sret_ptr.pdf = 0.0f;
                        sret_ptr.event_type = 0;
                        phi_in86 = 0;
                        phi_in88 = true;
                    }
                } else {
                    sret_ptr.pdf = 0.0f;
                    sret_ptr.event_type = 0;
                    phi_in86 = 0;
                    phi_in88 = true;
                }
            }
            phi_out87 = phi_in86;
            phi_out89 = phi_in88;
            phi_in106 = 0.0f;
            if (!phi_out89) {
                tmp108 = (sret_ptr.bsdf_over_pdf.y + sret_ptr.bsdf_over_pdf.x + sret_ptr.bsdf_over_pdf.z) * 0.3333333433f;
                phi_in106 = tmp108;
            }
            phi_out107 = phi_in106;
            tmp109 = min(max(sqrt(tmp25 * tmp24), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
            tmp110 = sret_ptr.xi.w;
            if (tmp110 > phi_out107) {
                float tmp111 = min(max((tmp110 - phi_out107) / (1.0f - phi_out107), 0.0f), 1.0f);
                float tmp112 = phi_out * 6.283185482f;
                float tmp113 = sin(tmp112);
                float tmp114 = cos(tmp112);
                float tmp115 = sqrt(1.0f - tmp111);
                float tmp116 = sqrt(tmp111);
                float tmp117 = tmp115 * (tmp44.x * tmp114 + tmp40 * tmp113) + tmp116 * tmp9;
                float tmp118 = tmp115 * (tmp44.y * tmp114 + tmp41 * tmp113) + tmp116 * tmp10;
                float tmp119 = tmp115 * (tmp44.z * tmp114 + tmp42 * tmp113) + tmp116 * tmp11;
                float3 tmp120 = float3(sqrt(tmp117 * tmp117 + tmp118 * tmp118 + tmp119 * tmp119), 0.0f, 0.0f);
                float3 tmp121 = float3(tmp117, tmp118, tmp119) / tmp120.xxx;
                sret_ptr.k2.x = tmp121.x;
                sret_ptr.k2.y = tmp121.y;
                sret_ptr.k2.z = tmp121.z;
                sret_ptr.event_type = 9;
                float tmp122 = tmp116 * 0.9200000167f * (3.141592741f - tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp116, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp109, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x * 3.141592741f) / tex_lookup_float3_3d(1, float3(1.0f, tmp109, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                sret_ptr.bsdf_over_pdf.x = tmp122;
                sret_ptr.bsdf_over_pdf.y = tmp122;
                sret_ptr.bsdf_over_pdf.z = tmp122;
                phi_in37 = 9;
            } else {
                float tmp123 = 1.0f / phi_out107;
                sret_ptr.bsdf_over_pdf.x = sret_ptr.bsdf_over_pdf.x * tmp123;
                sret_ptr.bsdf_over_pdf.y = sret_ptr.bsdf_over_pdf.y * tmp123;
                sret_ptr.bsdf_over_pdf.z = sret_ptr.bsdf_over_pdf.z * tmp123;
                phi_in37 = phi_out87;
            }
        }
    } else {
        gen_weighted_layer_sample(sret_ptr, state);
        float tmp124 = sret_ptr.bsdf_over_pdf.x;
        sret_ptr.bsdf_over_pdf.x = tmp124 * min(max(state.text_results[18].x, 0.0f), 1.0f);
        float tmp125 = sret_ptr.bsdf_over_pdf.y;
        sret_ptr.bsdf_over_pdf.y = tmp125 * min(max(state.text_results[18].y, 0.0f), 1.0f);
        float tmp126 = sret_ptr.bsdf_over_pdf.z;
        sret_ptr.bsdf_over_pdf.z = tmp126 * min(max(state.text_results[18].z, 0.0f), 1.0f);
        tmp127 = sret_ptr.event_type;
        phi_in37 = tmp127;
    }
    phi_out38 = phi_in37;
    if (phi_out38 == 0) {
        tmp128 = sret_ptr.k1.x;
        tmp129 = sret_ptr.k1.y;
        tmp130 = sret_ptr.k1.z;
        tmp131 = sret_ptr.k2.x;
        tmp132 = sret_ptr.k2.y;
        tmp133 = sret_ptr.k2.z;
        phi_in134 = tmp133;
        phi_in136 = tmp132;
        phi_in138 = tmp131;
        phi_in140 = tmp130;
        phi_in142 = tmp129;
        phi_in144 = tmp128;
    } else {
        tmp146 = sret_ptr.k2.x;
        tmp147 = sret_ptr.k2.y;
        tmp148 = sret_ptr.k2.z;
        float3 tmp149 = float3(tmp146, tmp147, tmp148) * tmp12;
        tmp150 = abs(tmp149.x + tmp149.y + tmp149.z);
        if ((phi_out38 & 16) == 0) {
            tmp151 = sret_ptr.k1.x;
            tmp152 = tmp151 + tmp146;
            tmp153 = sret_ptr.k1.y;
            tmp154 = tmp153 + tmp147;
            phi_in155 = tmp153;
            phi_in157 = tmp151;
            phi_in159 = tmp148;
            phi_in161 = tmp152;
            phi_in163 = tmp154;
        } else {
            float tmp165 = tmp150 * 2.0f;
            tmp166 = tmp165 * tmp11 + tmp148;
            tmp167 = sret_ptr.k1.x;
            tmp168 = tmp165 * tmp9 + tmp146 + tmp167;
            tmp169 = sret_ptr.k1.y;
            tmp170 = tmp165 * tmp10 + tmp147 + tmp169;
            phi_in155 = tmp169;
            phi_in157 = tmp167;
            phi_in159 = tmp166;
            phi_in161 = tmp168;
            phi_in163 = tmp170;
        }
        phi_out156 = phi_in155;
        phi_out158 = phi_in157;
        phi_out160 = phi_in159;
        phi_out162 = phi_in161;
        phi_out164 = phi_in163;
        tmp171 = sret_ptr.k1.z;
        float tmp172 = tmp171 + phi_out160;
        float3 tmp173 = float3(sqrt(phi_out164 * phi_out164 + phi_out162 * phi_out162 + tmp172 * tmp172), 0.0f, 0.0f);
        tmp174 = float3(phi_out162, phi_out164, tmp172) * float3(phi_out158, phi_out156, tmp171) / tmp173.xxx;
        if (tmp21) {
            float tmp175 = 1.0f - min(max(abs(tmp174.x + tmp174.y + tmp174.z), 0.0f), 1.0f);
            float tmp176 = tmp175 * tmp175;
            tmp177 = (tmp176 * tmp176 * (tmp175 * tmp17) + tmp0) * tmp1 / tmp19;
            sret_ptr.bsdf_over_pdf.x = sret_ptr.bsdf_over_pdf.x * tmp177;
            sret_ptr.bsdf_over_pdf.y = sret_ptr.bsdf_over_pdf.y * tmp177;
            tmp178 = sret_ptr.bsdf_over_pdf.z;
            phi_in179 = tmp177;
            phi_in181 = tmp178;
        } else {
            float tmp183 = 1.0f - min(max(tmp150, 0.0f), 1.0f);
            float tmp184 = tmp183 * tmp183;
            tmp185 = (1.0f - max(tmp18, tmp184 * tmp184 * (tmp183 * tmp17) + tmp0) * tmp1) / (1.0f - tmp19);
            sret_ptr.bsdf_over_pdf.x = sret_ptr.bsdf_over_pdf.x * tmp185;
            sret_ptr.bsdf_over_pdf.y = sret_ptr.bsdf_over_pdf.y * tmp185;
            tmp186 = sret_ptr.bsdf_over_pdf.z;
            phi_in179 = tmp186;
            phi_in181 = tmp185;
        }
        phi_out180 = phi_in179;
        phi_out182 = phi_in181;
        sret_ptr.bsdf_over_pdf.z = phi_out182 * phi_out180;
        phi_in134 = tmp148;
        phi_in136 = tmp147;
        phi_in138 = tmp146;
        phi_in140 = tmp171;
        phi_in142 = phi_out156;
        phi_in144 = phi_out158;
    }
    phi_out135 = phi_in134;
    phi_out137 = phi_in136;
    phi_out139 = phi_in138;
    phi_out141 = phi_in140;
    phi_out143 = phi_in142;
    phi_out145 = phi_in144;
    pdf_data.ior1.x = sret_ptr.ior1.x;
    pdf_data.ior1.y = sret_ptr.ior1.y;
    pdf_data.ior1.z = sret_ptr.ior1.z;
    pdf_data.ior2.x = sret_ptr.ior2.x;
    pdf_data.ior2.y = sret_ptr.ior2.y;
    pdf_data.ior2.z = sret_ptr.ior2.z;
    pdf_data.k1.x = phi_out145;
    pdf_data.k1.y = phi_out143;
    pdf_data.k1.z = phi_out141;
    pdf_data.k2.x = phi_out139;
    pdf_data.k2.y = phi_out137;
    pdf_data.k2.z = phi_out135;
    gen_custom_curve_layer_pdf(pdf_data, state);
    float tmp187 = pdf_data.pdf;
    sret_ptr.pdf = tmp187;
    return;
}

void gen_weighted_layer_evaluate(inout Bsdf_evaluate_data p_00, in Shading_state_material p_11, in float3 p_22)
{
    float tmp3;
    float tmp4;
    float tmp5;
    float tmp6;
    float phi_in;
    float phi_out;
    float tmp7;
    float tmp8;
    float tmp9;
    float3 tmp10;
    float tmp13;
    float3 tmp14;
    float3 tmp15;
    float phi_in17;
    float phi_out18;
    float tmp23;
    float tmp25;
    float phi_in26;
    float phi_out27;
    float tmp28;
    float tmp29;
    float tmp30;
    float tmp31;
    float tmp32;
    float tmp33;
    float tmp34;
    float tmp35;
    float tmp37;
    float tmp38;
    float tmp39;
    float3 tmp40;
    float tmp43;
    float tmp44;
    float tmp45;
    float tmp46;
    float tmp47;
    float tmp48;
    float3 tmp49;
    float3 tmp50;
    float phi_in52;
    float phi_out53;
    float3 tmp54;
    float tmp57;
    float tmp59;
    float phi_in60;
    float phi_out61;
    float phi_in62;
    float phi_out63;
    float phi_in64;
    float phi_out65;
    float tmp66;
    float tmp67;
    float tmp68;
    float tmp70;
    float tmp71;
    float3 tmp72;
    float3 tmp74;
    float tmp76;
    float phi_in77;
    float phi_out78;
    float tmp80;
    float tmp82;
    float tmp83;
    float tmp84;
    float tmp85;
    tmp3 = p_11.text_results[25].x;
    tmp4 = p_11.text_results[25].y;
    tmp5 = p_11.text_results[25].z;
    tmp6 = min(max(p_11.text_results[13].y, 0.0f), 1.0f);
    phi_in = 0.0f;
    if (tmp6 > 0.0f) {
        tmp7 = p_22.x;
        tmp8 = p_22.y;
        tmp9 = p_22.z;
        tmp10 = mdl_read_argblock_as_bool(p_11.arg_block_offset) ? p_11.text_results[14].xyz : float3(1.0f, 1.0f, 1.0f);
        float3 tmp11 = p_11.geom_normal;
        float3 tmp12 = p_00.k1 * tmp11;
        tmp13 = asfloat((asint(tmp12.x + tmp12.y + tmp12.z) & -2147483648) | 1065353216);
        tmp14 = p_00.k2;
        tmp15 = float3(tmp11.x * tmp13, tmp11.y * tmp13, tmp11.z * tmp13);
        float3 tmp16 = tmp15 * tmp14;
        phi_in17 = 0.0f;
        if (tmp16.x + tmp16.y + tmp16.z < 0.0f) {
            float3 tmp19 = float3(tmp3 * tmp13, tmp4 * tmp13, tmp5 * tmp13);
            float3 tmp20 = tmp19 * tmp15;
            float3 tmp21 = tmp19 * tmp14;
            float tmp22 = max(-(tmp21.x + tmp21.y + tmp21.z), 0.0f);
            tmp23 = tmp22 / (max(tmp20.x + tmp20.y + tmp20.z, 0.0f) * 1.570796371f + 1.570796371f);
            bool tmp24 = tmp22 > 0.0f;
            p_00.bsdf_diffuse.x = p_00.bsdf_diffuse.x + tmp7 * tmp6 * (tmp24 ? tmp23 * min(max(tmp10.x, 0.0f), 1.0f) : 0.0f);
            p_00.bsdf_diffuse.y = p_00.bsdf_diffuse.y + tmp8 * tmp6 * (tmp24 ? tmp23 * min(max(tmp10.y, 0.0f), 1.0f) : 0.0f);
            p_00.bsdf_diffuse.z = p_00.bsdf_diffuse.z + tmp9 * tmp6 * (tmp24 ? tmp23 * min(max(tmp10.z, 0.0f), 1.0f) : 0.0f);
            phi_in17 = tmp23;
        }
        phi_out18 = phi_in17;
        p_00.pdf = phi_out18;
        tmp25 = phi_out18 * tmp6;
        phi_in = tmp25;
    }
    phi_out = phi_in;
    phi_in26 = phi_out;
    if (tmp6 < 1.0f) {
        tmp28 = 1.0f - tmp6;
        tmp29 = p_22.x * tmp28;
        tmp30 = p_22.y * tmp28;
        tmp31 = p_22.z * tmp28;
        tmp32 = min(max(p_11.text_results[24].x, 0.0f), 1.0f);
        tmp33 = min(max(p_11.text_results[24].y, 0.0f), 1.0f);
        tmp34 = min(max(p_11.text_results[24].z, 0.0f), 1.0f);
        tmp35 = max(p_11.text_results[20].w, 0.0f);
        float3 tmp36 = p_11.geom_normal;
        tmp37 = p_00.k1.x;
        tmp38 = p_00.k1.y;
        tmp39 = p_00.k1.z;
        tmp40 = float3(tmp37, tmp38, tmp39);
        float3 tmp41 = tmp40 * tmp36;
        float tmp42 = asfloat((asint(tmp41.x + tmp41.y + tmp41.z) & -2147483648) | 1065353216);
        tmp43 = tmp3 * tmp42;
        tmp44 = tmp4 * tmp42;
        tmp45 = tmp5 * tmp42;
        tmp46 = p_00.k2.x;
        tmp47 = p_00.k2.y;
        tmp48 = p_00.k2.z;
        tmp49 = float3(tmp46, tmp47, tmp48);
        tmp50 = float3(tmp36.x * tmp42, tmp36.y * tmp42, tmp36.z * tmp42);
        float3 tmp51 = tmp50 * tmp49;
        phi_in52 = 0.0f;
        if (tmp51.x + tmp51.y + tmp51.z > 0.0f) {
            tmp54 = float3(tmp43, tmp44, tmp45);
            float3 tmp55 = tmp50 * tmp54;
            float3 tmp56 = tmp54 * tmp49;
            tmp57 = tmp56.x + tmp56.y + tmp56.z;
            float tmp58 = max(tmp57, 0.0f);
            tmp59 = tmp58 / (max(tmp55.x + tmp55.y + tmp55.z, 0.0f) * 1.570796371f + 1.570796371f);
            phi_in60 = 0.0f;
            phi_in62 = 0.0f;
            phi_in64 = 0.0f;
            if (tmp58 > 0.0f) {
                tmp66 = tmp59 * tmp32;
                tmp67 = tmp59 * tmp33;
                tmp68 = tmp59 * tmp34;
                phi_in60 = tmp66;
                phi_in62 = tmp67;
                phi_in64 = tmp68;
                if (tmp35 > 0.0f) {
                    float3 tmp69 = tmp54 * tmp40;
                    tmp70 = tmp69.x + tmp69.y + tmp69.z;
                    tmp71 = tmp35 * tmp35;
                    tmp72 = float3(tmp46 - tmp57 * tmp43, tmp47 - tmp57 * tmp44, tmp48 - tmp57 * tmp45);
                    float3 tmp73 = tmp72 * tmp72;
                    tmp74 = float3(tmp37 - tmp70 * tmp43, tmp38 - tmp70 * tmp44, tmp39 - tmp70 * tmp45);
                    float3 tmp75 = tmp74 * tmp74;
                    tmp76 = (tmp75.x + tmp75.y + tmp75.z) * (tmp73.x + tmp73.y + tmp73.z);
                    phi_in77 = 0.0f;
                    if (!(tmp76 == 0.0f)) {
                        float3 tmp79 = tmp74 * tmp72;
                        tmp80 = tmp71 * 0.4499999881f * max(sqrt((1.0f - tmp70 * tmp70) * (1.0f - tmp57 * tmp57) / tmp76) * (tmp79.x + tmp79.y + tmp79.z) / max(tmp57, tmp70), 0.0f) / (tmp71 + 0.09000000358f);
                        phi_in77 = tmp80;
                    }
                    phi_out78 = phi_in77;
                    float tmp81 = phi_out78 + (1.0f - tmp71 / (tmp71 * 2.0f + 0.6600000262f));
                    tmp82 = tmp81 * tmp66;
                    tmp83 = tmp81 * tmp67;
                    tmp84 = tmp81 * tmp68;
                    phi_in60 = tmp82;
                    phi_in62 = tmp83;
                    phi_in64 = tmp84;
                }
            }
            phi_out61 = phi_in60;
            phi_out63 = phi_in62;
            phi_out65 = phi_in64;
            p_00.bsdf_diffuse.x = p_00.bsdf_diffuse.x + tmp29 * phi_out61;
            p_00.bsdf_diffuse.y = p_00.bsdf_diffuse.y + tmp30 * phi_out63;
            p_00.bsdf_diffuse.z = p_00.bsdf_diffuse.z + tmp31 * phi_out65;
            phi_in52 = tmp59;
        }
        phi_out53 = phi_in52;
        tmp85 = phi_out53 * tmp28 + phi_out;
        phi_in26 = tmp85;
    }
    phi_out27 = phi_in26;
    p_00.pdf = phi_out27;
    return;
}

public void mdl_surface_scattering_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float tmp0;
    float tmp1;
    float tmp2;
    float tmp3;
    float tmp4;
    float tmp5;
    float22 tmp6;
    float22 tmp7;
    float3 tmp8;
    float3 tmp9;
    float tmp10;
    float tmp11;
    float tmp12;
    float tmp14;
    float tmp15;
    float tmp16;
    float3 tmp17;
    float tmp19;
    float tmp20;
    float tmp21;
    float tmp22;
    float3 tmp23;
    float tmp25;
    float tmp26;
    float tmp27;
    float tmp28;
    float tmp29;
    float3 tmp30;
    float tmp32;
    float tmp33;
    float tmp35;
    bool tmp36;
    float phi_in;
    float phi_out;
    float phi_in37;
    float phi_out38;
    float phi_in39;
    float phi_out40;
    float tmp42;
    float tmp43;
    float tmp44;
    float tmp55;
    float tmp56;
    float tmp59;
    float tmp60;
    float tmp61;
    float tmp62;
    float tmp63;
    float tmp64;
    float3 tmp66;
    float3 tmp67;
    float3 tmp68;
    float tmp70;
    float phi_in71;
    float phi_out72;
    float3 phi_in73;
    float3 phi_out74;
    float phi_in75;
    float phi_out76;
    float3 tmp78;
    float3 tmp79;
    float tmp80;
    float phi_in81;
    float phi_out82;
    float3 phi_in83;
    float3 phi_out84;
    float3 tmp89;
    float tmp91;
    float tmp109;
    float3 tmp111;
    float tmp112;
    float tmp113;
    float tmp114;
    float tmp115;
    float tmp116;
    float tmp117;
    float tmp118;
    float tmp119;
    float tmp120;
    float tmp122;
    float tmp123;
    float tmp124;
    float tmp125;
    float tmp126;
    float tmp127;
    float tmp128;
    float phi_in129;
    float phi_out130;
    float tmp131;
    float tmp132;
    float tmp133;
    float tmp134;
    float tmp135;
    float tmp136;
    float tmp137;
    float tmp138;
    float tmp139;
    float tmp140;
    float tmp141;
    float tmp142;
    float tmp143;
    float tmp144;
    float tmp145;
    float tmp146;
    float3 tmp147;
    float3 tmp148;
    float phi_in149;
    float phi_out150;
    float phi_in151;
    float phi_out152;
    float phi_in153;
    float phi_out154;
    float tmp156;
    float tmp157;
    float tmp158;
    float tmp164;
    float3 phi_in165;
    float3 phi_out166;
    float tmp167;
    float tmp168;
    float tmp169;
    float tmp170;
    float phi_in171;
    float phi_out172;
    float phi_in173;
    float phi_out174;
    float phi_in175;
    float phi_out176;
    float tmp177;
    float tmp178;
    float tmp179;
    float tmp180;
    float phi_in181;
    float phi_out182;
    float phi_in183;
    float phi_out184;
    float phi_in185;
    float phi_out186;
    float phi_in187;
    float phi_out188;
    float phi_in189;
    float phi_out190;
    int phi_in191;
    int phi_out192;
    float phi_in193;
    float phi_out194;
    float phi_in195;
    float phi_out196;
    float phi_in197;
    float phi_out198;
    float tmp200;
    float phi_in201;
    float phi_out202;
    float phi_in203;
    float phi_out204;
    float phi_in205;
    float phi_out206;
    int phi_in207;
    int phi_out208;
    float phi_in209;
    float phi_out210;
    float phi_in211;
    float phi_out212;
    float phi_in213;
    float phi_out214;
    float tmp216;
    float tmp217;
    float tmp218;
    float tmp219;
    int tmp220;
    float rmemload160;
    float rmemload164;
    float phi_in221;
    float phi_out222;
    float phi_in223;
    float phi_out224;
    float phi_in225;
    float phi_out226;
    float phi_in227;
    float phi_out228;
    float rmemload159;
    float rmemload163;
    float rmemload158;
    float rmemload162;
    float tmp230;
    float tmp231;
    float phi_in232;
    float phi_out233;
    float phi_in234;
    float phi_out235;
    float phi_in236;
    float phi_out237;
    float phi_in238;
    float phi_out239;
    float phi_in240;
    float phi_out241;
    float phi_in242;
    float phi_out243;
    float phi_in244;
    float phi_out245;
    float phi_in246;
    float phi_out247;
    int phi_in248;
    int phi_out249;
    float phi_in250;
    float phi_out251;
    float phi_in252;
    float phi_out253;
    float phi_in254;
    float phi_out255;
    int phi_in256;
    int phi_out257;
    float phi_in258;
    float phi_out259;
    float phi_in260;
    float phi_out261;
    float phi_in262;
    float phi_out263;
    float phi_in264;
    float phi_out265;
    float phi_in266;
    float phi_out267;
    float tmp270;
    float tmp275;
    float tmp276;
    float tmp283;
    float tmp284;
    float tmp286;
    float tmp287;
    float tmp289;
    float tmp290;
    float tmp292;
    float tmp294;
    float tmp295;
    float tmp297;
    float tmp298;
    float phi_in299;
    float phi_out300;
    float tmp301;
    float tmp302;
    float phi_in303;
    float phi_out304;
    float tmp305;
    float tmp306;
    float tmp307;
    float tmp308;
    float tmp309;
    float tmp318;
    float tmp321;
    float tmp322;
    float tmp323;
    float tmp324;
    float tmp325;
    float tmp326;
    float phi_in327;
    float phi_out328;
    float phi_in329;
    float phi_out330;
    float phi_in331;
    float phi_out332;
    int phi_in333;
    int phi_out334;
    float phi_in335;
    float phi_out336;
    float phi_in337;
    float phi_out338;
    float phi_in339;
    float phi_out340;
    float phi_in341;
    float phi_out342;
    float phi_in343;
    float phi_out344;
    float phi_in345;
    float phi_out346;
    float phi_in347;
    float phi_out348;
    float phi_in349;
    float phi_out350;
    int phi_in351;
    int phi_out352;
    float phi_in353;
    float phi_out354;
    float phi_in355;
    float phi_out356;
    float phi_in357;
    float phi_out358;
    float phi_in359;
    float phi_out360;
    float phi_in361;
    float phi_out362;
    float tmp370;
    float tmp371;
    float tmp372;
    float tmp373;
    int tmp374;
    float rmemload148;
    float rmemload152;
    float phi_in375;
    float phi_out376;
    float phi_in377;
    float phi_out378;
    float phi_in379;
    float phi_out380;
    float phi_in381;
    float phi_out382;
    float rmemload147;
    float rmemload151;
    float rmemload;
    float rmemload150;
    bool tmp383;
    float phi_in384;
    float phi_out385;
    float phi_in386;
    float phi_out387;
    float phi_in388;
    float phi_out389;
    float phi_in390;
    float phi_out391;
    float phi_in392;
    float phi_out393;
    float phi_in394;
    float phi_out395;
    float phi_in396;
    float phi_out397;
    float3 tmp398;
    float3 tmp440;
    float tmp441;
    float tmp442;
    float tmp443;
    float phi_in444;
    float phi_out445;
    float tmp446;
    float tmp447;
    float tmp448;
    float tmp449;
    float tmp450;
    float tmp451;
    float tmp452;
    float3 tmp454;
    float3 tmp455;
    float3 tmp456;
    float tmp458;
    float phi_in459;
    float phi_out460;
    float3 phi_in461;
    float3 phi_out462;
    float phi_in463;
    float phi_out464;
    float3 tmp466;
    float3 tmp467;
    float tmp469;
    float tmp470;
    float tmp471;
    float tmp472;
    float phi_in473;
    float phi_out474;
    float3 phi_in475;
    float3 phi_out476;
    float3 tmp481;
    float tmp483;
    float tmp501;
    float3 tmp503;
    float tmp504;
    float tmp505;
    float tmp506;
    float tmp507;
    float tmp508;
    float tmp509;
    float phi_in510;
    float phi_out511;
    float tmp512;
    float tmp513;
    float tmp514;
    float tmp515;
    float tmp516;
    float phi_in517;
    float phi_out518;
    float tmp519;
    float tmp520;
    float tmp521;
    float tmp522;
    float phi_in523;
    float phi_out524;
    float tmp525;
    float tmp526;
    float tmp527;
    int tmp528;
    bool tmp529;
    bool phi_in530;
    bool phi_out531;
    float tmp536;
    float tmp537;
    float tmp538;
    float3 tmp539;
    float tmp541;
    float tmp543;
    float tmp544;
    float tmp545;
    float tmp547;
    float tmp548;
    float tmp549;
    float tmp550;
    bool tmp551;
    float3 tmp553;
    float3 tmp554;
    float3 tmp555;
    float tmp557;
    float phi_in558;
    float phi_out559;
    float3 phi_in560;
    float3 phi_out561;
    float phi_in562;
    float phi_out563;
    float3 tmp565;
    float3 tmp566;
    float tmp567;
    float phi_in568;
    float phi_out569;
    float tmp570;
    float tmp571;
    float tmp572;
    float tmp573;
    float tmp574;
    float phi_in575;
    float phi_out576;
    float phi_in577;
    float phi_out578;
    float phi_in579;
    float phi_out580;
    float tmp581;
    float tmp582;
    float tmp583;
    float tmp586;
    float tmp588;
    float tmp589;
    float tmp590;
    float tmp592;
    float tmp593;
    float tmp594;
    float phi_in595;
    float phi_out596;
    float phi_in597;
    float phi_out598;
    float phi_in599;
    float phi_out600;
    float tmp601;
    float tmp602;
    float tmp603;
    float tmp604;
    float tmp605;
    float tmp606;
    float tmp607;
    float tmp608;
    float tmp609;
    float3 tmp611;
    float tmp613;
    float tmp615;
    float tmp618;
    float phi_in619;
    float phi_out620;
    float3 phi_in621;
    float3 phi_out622;
    float tmp624;
    float tmp625;
    float tmp626;
    float tmp627;
    float tmp635;
    float tmp642;
    float tmp645;
    float tmp647;
    float tmp648;
    float tmp649;
    float phi_in650;
    float phi_out651;
    float phi_in652;
    float phi_out653;
    float phi_in654;
    float phi_out655;
    float tmp656;
    float3 tmp659;
    float tmp660;
    float phi_in661;
    float phi_out662;
    float phi_in663;
    float phi_out664;
    float tmp665;
    float tmp666;
    float tmp667;
    float phi_in668;
    float phi_out669;
    float tmp671;
    float tmp672;
    float tmp673;
    float tmp674;
    float tmp675;
    float tmp676;
    float tmp677;
    float phi_in678;
    float phi_out679;
    float tmp680;
    float tmp681;
    float tmp682;
    float tmp683;
    float3 tmp684;
    float tmp686;
    float phi_in687;
    float phi_out688;
    float tmp689;
    float tmp690;
    float tmp691;
    bool phi_in692;
    bool phi_out693;
    float3 tmp694;
    float tmp697;
    float3 tmp698;
    float3 tmp699;
    float phi_in701;
    float phi_out702;
    float tmp707;
    float tmp709;
    float phi_in710;
    float phi_out711;
    float tmp712;
    float tmp713;
    float tmp714;
    float tmp715;
    bool tmp716;
    bool phi_in717;
    bool phi_out718;
    float3 tmp719;
    float tmp720;
    float tmp721;
    float tmp723;
    float tmp724;
    float tmp725;
    float3 tmp726;
    float tmp729;
    float tmp730;
    float tmp731;
    float tmp732;
    float tmp733;
    float tmp734;
    float3 tmp736;
    float3 tmp737;
    float3 tmp738;
    float3 tmp739;
    float tmp741;
    float phi_in742;
    float phi_out743;
    float3 phi_in744;
    float3 phi_out745;
    float3 tmp747;
    float3 tmp748;
    float tmp749;
    float phi_in750;
    float phi_out751;
    float tmp752;
    float tmp753;
    float tmp754;
    float tmp755;
    float tmp756;
    float phi_in757;
    float phi_out758;
    float phi_in759;
    float phi_out760;
    float phi_in761;
    float phi_out762;
    float tmp763;
    float tmp764;
    float tmp765;
    float tmp768;
    float tmp770;
    float tmp771;
    float tmp772;
    float tmp773;
    float tmp774;
    float3 tmp775;
    float tmp777;
    float tmp778;
    bool tmp780;
    float tmp782;
    float tmp783;
    float tmp784;
    float phi_in785;
    float phi_out786;
    float phi_in787;
    float phi_out788;
    float phi_in789;
    float phi_out790;
    float tmp791;
    float tmp792;
    float tmp793;
    float tmp794;
    float tmp795;
    float tmp796;
    float tmp797;
    float tmp798;
    float tmp799;
    float3 tmp801;
    float tmp803;
    float tmp805;
    float tmp808;
    int phi_in809;
    int phi_out810;
    float phi_in811;
    float phi_out812;
    float phi_in813;
    float phi_out814;
    float3 phi_in815;
    float3 phi_out816;
    float tmp825;
    float tmp832;
    float tmp835;
    float tmp837;
    float tmp838;
    float phi_in839;
    float phi_out840;
    float phi_in841;
    float phi_out842;
    float phi_in843;
    float phi_out844;
    float tmp845;
    float tmp846;
    float3 tmp848;
    float tmp849;
    float tmp850;
    float tmp851;
    float phi_in852;
    float phi_out853;
    float tmp854;
    float tmp855;
    float tmp856;
    float tmp857;
    float tmp861;
    float tmp862;
    float tmp863;
    float tmp865;
    float tmp866;
    float tmp867;
    float3 tmp868;
    float tmp871;
    float tmp872;
    float tmp873;
    float3 tmp874;
    float tmp876;
    float tmp877;
    float tmp878;
    float tmp879;
    float tmp880;
    float3 tmp881;
    float tmp883;
    float tmp884;
    float tmp886;
    bool tmp887;
    float phi_in888;
    float phi_out889;
    float phi_in890;
    float phi_out891;
    float phi_in892;
    float phi_out893;
    float tmp895;
    float tmp896;
    float tmp897;
    float tmp904;
    float tmp905;
    float tmp906;
    float tmp908;
    float tmp909;
    float tmp910;
    float tmp911;
    float tmp912;
    float tmp913;
    float tmp915;
    float tmp916;
    float tmp917;
    float tmp918;
    bool tmp919;
    float3 tmp921;
    float3 tmp922;
    float3 tmp923;
    float tmp925;
    float phi_in926;
    float phi_out927;
    float3 phi_in928;
    float3 phi_out929;
    float phi_in930;
    float phi_out931;
    float3 tmp933;
    float3 tmp934;
    float tmp935;
    float phi_in936;
    float phi_out937;
    float3 phi_in938;
    float3 phi_out939;
    float3 tmp944;
    float tmp946;
    float tmp964;
    float3 tmp966;
    float tmp967;
    float tmp968;
    float tmp969;
    float tmp970;
    float tmp971;
    float tmp972;
    float tmp973;
    float tmp974;
    float tmp976;
    float tmp977;
    float tmp978;
    float tmp979;
    float phi_in980;
    float phi_out981;
    float tmp982;
    float tmp983;
    float tmp984;
    float tmp985;
    float tmp986;
    float tmp987;
    float tmp989;
    float tmp990;
    bool tmp991;
    int tmp992;
    float phi_in996;
    float phi_out997;
    float phi_in998;
    float phi_out999;
    float3 phi_in1000;
    float3 phi_out1001;
    float tmp1002;
    float tmp1009;
    float tmp1011;
    float tmp1013;
    float3 tmp1018;
    float tmp1019;
    float phi_in1020;
    float phi_out1021;
    float tmp1026;
    float tmp1027;
    float phi_in1028;
    float phi_out1029;
    float tmp1032;
    float tmp1033;
    float tmp1034;
    tmp10 = min(max(state.text_results[21].z, 0.0f), 1.0f);
    tmp11 = min(max(1.0f - state.text_results[16].w, 0.0f), 1.0f);
    tmp12 = min(max(state.text_results[17].x, 0.0f), 1.0f);
    float3 tmp13 = state.geom_normal;
    tmp14 = sret_ptr.k1.x;
    tmp15 = sret_ptr.k1.y;
    tmp16 = sret_ptr.k1.z;
    tmp17 = float3(tmp14, tmp15, tmp16);
    float3 tmp18 = tmp17 * tmp13;
    tmp19 = asfloat((asint(tmp18.x + tmp18.y + tmp18.z) & -2147483648) | 1065353216);
    tmp20 = state.text_results[25].w * tmp19;
    tmp21 = state.text_results[26].x * tmp19;
    tmp22 = state.text_results[26].y * tmp19;
    tmp23 = float3(tmp20, tmp21, tmp22);
    float3 tmp24 = tmp23 * tmp17;
    tmp25 = tmp24.x + tmp24.y + tmp24.z;
    tmp26 = min(max(tmp25, 0.0f), 1.0f);
    tmp27 = sret_ptr.k2.x;
    tmp28 = sret_ptr.k2.y;
    tmp29 = sret_ptr.k2.z;
    tmp30 = float3(tmp27, tmp28, tmp29);
    float3 tmp31 = tmp23 * tmp30;
    tmp32 = tmp31.x + tmp31.y + tmp31.z;
    tmp33 = abs(tmp32);
    float3 tmp34 = float3(tmp13.x * tmp19, tmp13.y * tmp19, tmp13.z * tmp19) * tmp30;
    tmp35 = tmp34.x + tmp34.y + tmp34.z;
    tmp36 = tmp35 < 0.0f;
    phi_in = tmp29;
    phi_in37 = tmp27;
    phi_in39 = tmp28;
    if (tmp36) {
        float tmp41 = tmp33 * 2.0f;
        tmp42 = tmp41 * tmp20 + tmp27;
        tmp43 = tmp41 * tmp21 + tmp28;
        tmp44 = tmp41 * tmp22 + tmp29;
        phi_in = tmp44;
        phi_in37 = tmp42;
        phi_in39 = tmp43;
    }
    phi_out = phi_in;
    phi_out38 = phi_in37;
    phi_out40 = phi_in39;
    float tmp45 = phi_out40 + tmp15;
    float tmp46 = phi_out38 + tmp14;
    float tmp47 = phi_out + tmp16;
    float3 tmp48 = float3(sqrt(tmp46 * tmp46 + tmp47 * tmp47 + tmp45 * tmp45), 0.0f, 0.0f);
    float3 tmp49 = float3(tmp46, tmp45, tmp47) * tmp17 / tmp48.xxx;
    float tmp50 = 1.0f - min(max(abs(tmp49.x + tmp49.y + tmp49.z), 0.0f), 1.0f);
    float tmp51 = tmp50 * tmp50;
    float tmp52 = tmp11 - tmp10;
    float tmp53 = 1.0f - min(max(tmp26, 0.0f), 1.0f);
    float tmp54 = tmp53 * tmp53;
    tmp55 = tmp54 * tmp54 * (tmp53 * tmp52) + tmp10;
    tmp56 = min(max(tmp33, 0.0f), 1.0f);
    float tmp57 = 1.0f - tmp56;
    float tmp58 = tmp57 * tmp57;
    tmp59 = tmp58 * tmp58 * (tmp57 * tmp52) + tmp10;
    tmp60 = (tmp51 * tmp51 * (tmp50 * tmp52) + tmp10) * tmp12;
    tmp61 = state.text_results[21].y;
    tmp62 = state.text_results[21].x;
    tmp63 = max(tmp62, 1.000000012e-07f);
    tmp64 = max(tmp61, 1.000000012e-07f);
    float3 tmp65 = float3(state.text_results[22].z, state.text_results[22].w, state.text_results[23].x);
    tmp66 = tmp23.zxy;
    tmp67 = tmp23.yzx;
    tmp68 = tmp65.yzx * tmp66 - tmp65.zxy * tmp67;
    float3 tmp69 = tmp68 * tmp68;
    tmp70 = tmp69.x + tmp69.y + tmp69.z;
    if (tmp70 < 9.999999939e-09f) {
        sret_ptr.pdf = 0.0f;
        phi_in71 = 0.0f;
        phi_in73 = float3(0.0f, 0.0f, 0.0f);
        phi_in75 = 0.0f;
    } else {
        float tmp77 = 1.0f / sqrt(tmp70);
        tmp78 = float3(tmp77 * tmp68.x, tmp77 * tmp68.y, tmp77 * tmp68.z);
        tmp79 = tmp78.zxy * tmp67 - tmp78.yzx * tmp66;
        if (sret_ptr.ior1.x == -1.0f) {
            sret_ptr.ior1.x = state.text_results[28].x;
            sret_ptr.ior1.y = state.text_results[28].y;
            sret_ptr.ior1.z = state.text_results[28].z;
        }
        if (sret_ptr.ior2.x == -1.0f) {
            sret_ptr.ior2.x = state.text_results[28].x;
            sret_ptr.ior2.y = state.text_results[28].y;
            sret_ptr.ior2.z = state.text_results[28].z;
        }
        tmp80 = abs(tmp25);
        phi_in81 = 0.0f;
        phi_in83 = float3(0.0f, 0.0f, 0.0f);
        if (!tmp36) {
            float tmp85 = tmp27 + tmp14;
            float tmp86 = tmp28 + tmp15;
            float tmp87 = tmp29 + tmp16;
            float3 tmp88 = float3(sqrt(tmp86 * tmp86 + tmp85 * tmp85 + tmp87 * tmp87), 0.0f, 0.0f);
            tmp89 = float3(tmp85, tmp86, tmp87) / tmp88.xxx;
            float3 tmp90 = tmp23 * tmp89;
            tmp91 = tmp90.x + tmp90.y + tmp90.z;
            float3 tmp92 = tmp89 * tmp17;
            float3 tmp93 = tmp89 * tmp30;
            phi_in81 = 0.0f;
            phi_in83 = float3(0.0f, 0.0f, 0.0f);
            if (!(tmp93.x + tmp93.y + tmp93.z < 0.0f || (tmp92.x + tmp92.y + tmp92.z < 0.0f || tmp91 < 0.0f))) {
                float3 tmp94 = tmp79 * tmp89;
                float3 tmp95 = tmp78 * tmp89;
                float tmp96 = 1.0f / tmp63;
                float tmp97 = 1.0f / tmp64;
                float tmp98 = (tmp94.x + tmp94.y + tmp94.z) * tmp96;
                float tmp99 = (tmp95.x + tmp95.y + tmp95.z) * tmp97;
                float tmp100 = tmp99 * tmp99 + tmp91 * tmp91 + tmp98 * tmp98;
                float3 tmp101 = tmp79 * tmp17;
                float3 tmp102 = tmp78 * tmp17;
                float3 tmp103 = tmp79 * tmp30;
                float3 tmp104 = tmp78 * tmp30;
                float tmp105 = (tmp101.x + tmp101.y + tmp101.z) * tmp63;
                float tmp106 = (tmp102.x + tmp102.y + tmp102.z) * tmp64;
                float tmp107 = (tmp103.x + tmp103.y + tmp103.z) * tmp63;
                float tmp108 = (tmp104.x + tmp104.y + tmp104.z) * tmp64;
                tmp109 = tmp91 * 0.1591549367f * tmp97 * tmp96 / (tmp100 * tmp100 * (tmp80 * tmp91) * (sqrt((tmp105 * tmp105 + tmp106 * tmp106) / (tmp25 * tmp25) + 1.0f) + 1.0f));
                float tmp110 = tmp109 * 2.0f / (sqrt((tmp107 * tmp107 + tmp108 * tmp108) / (tmp32 * tmp32) + 1.0f) + 1.0f);
                tmp111 = float3(tmp110, tmp110, tmp110);
                phi_in81 = tmp109;
                phi_in83 = tmp111;
            }
        }
        phi_out82 = phi_in81;
        phi_out84 = phi_in83;
        tmp112 = min(max(sqrt(tmp62 * tmp61), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
        tmp113 = tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp80, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp112, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
        tmp114 = tmp113 * phi_out82;
        sret_ptr.pdf = tmp114;
        phi_in71 = tmp114;
        phi_in73 = phi_out84;
        phi_in75 = 0.0f;
        if (!(tmp35 < 0.0f)) {
            tmp115 = 0.3183098733f - tmp113 * 0.3183098733f + tmp114;
            sret_ptr.pdf = tmp115;
            tmp116 = tmp33 * 0.9200000167f * (1.0f - tmp113) * (1.0f - tex_lookup_float3_3d(1, float3(min(max(acos(tmp56) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp112, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x) / tex_lookup_float3_3d(1, float3(1.0f, tmp112, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
            phi_in71 = tmp115;
            phi_in73 = phi_out84;
            phi_in75 = tmp116;
        }
    }
    phi_out72 = phi_in71;
    phi_out74 = phi_in73;
    phi_out76 = phi_in75;
    tmp117 = phi_out76 * tmp60;
    tmp118 = phi_out74.x * tmp60;
    tmp119 = phi_out74.y * tmp60;
    tmp120 = phi_out74.z * tmp60;
    sret_ptr.bsdf_diffuse.x = tmp117;
    sret_ptr.bsdf_diffuse.y = tmp117;
    sret_ptr.bsdf_diffuse.z = tmp117;
    sret_ptr.bsdf_glossy.x = tmp118;
    sret_ptr.bsdf_glossy.y = tmp119;
    sret_ptr.bsdf_glossy.z = tmp120;
    float tmp121 = 1.0f - max(tmp55, tmp59) * tmp12;
    tmp122 = min(max(state.text_results[18].x, 0.0f), 1.0f) * tmp121;
    tmp123 = min(max(state.text_results[18].y, 0.0f), 1.0f) * tmp121;
    tmp124 = min(max(state.text_results[18].z, 0.0f), 1.0f) * tmp121;
    tmp125 = state.text_results[25].x;
    tmp126 = state.text_results[25].y;
    tmp127 = state.text_results[25].z;
    tmp128 = min(max(state.text_results[20].y, 0.0f), 1.0f);
    phi_in129 = 0.0f;
    if (tmp128 > 0.0f) {
        tmp131 = tmp128 * tmp122;
        tmp132 = tmp128 * tmp123;
        tmp133 = tmp128 * tmp124;
        tmp134 = state.text_results[30].y;
        tmp135 = state.text_results[30].z;
        tmp136 = state.text_results[30].w;
        tmp137 = mdl_read_argblock_as_bool(state.arg_block_offset + 236) ? state.text_results[19].w : 0.0f;
        tmp138 = state.text_results[29].z;
        tmp139 = state.text_results[29].w;
        tmp140 = state.text_results[30].x;
        tmp3 = tmp138;
        tmp4 = tmp139;
        tmp5 = tmp140;
        tmp141 = state.text_results[28].w;
        tmp142 = state.text_results[29].x;
        tmp143 = state.text_results[29].y;
        tmp0 = tmp141;
        tmp1 = tmp142;
        tmp2 = tmp143;
        tmp144 = tmp125 * tmp19;
        tmp145 = tmp126 * tmp19;
        tmp146 = tmp127 * tmp19;
        tmp147 = float3(tmp144, tmp145, tmp146);
        tmp148 = tmp147 * tmp30;
        phi_in149 = tmp29;
        phi_in151 = tmp27;
        phi_in153 = tmp28;
        if (tmp36) {
            float tmp155 = abs(tmp148.x + tmp148.y + tmp148.z) * 2.0f;
            tmp156 = tmp155 * tmp144 + tmp27;
            tmp157 = tmp155 * tmp145 + tmp28;
            tmp158 = tmp155 * tmp146 + tmp29;
            phi_in149 = tmp158;
            phi_in151 = tmp156;
            phi_in153 = tmp157;
        }
        phi_out150 = phi_in149;
        phi_out152 = phi_in151;
        phi_out154 = phi_in153;
        float tmp159 = phi_out154 + tmp15;
        float tmp160 = phi_out152 + tmp14;
        float tmp161 = phi_out150 + tmp16;
        float3 tmp162 = float3(sqrt(tmp160 * tmp160 + tmp161 * tmp161 + tmp159 * tmp159), 0.0f, 0.0f);
        float3 tmp163 = float3(tmp160, tmp159, tmp161) * tmp17 / tmp162.xxx;
        tmp164 = tmp163.x + tmp163.y + tmp163.z;
        phi_in165 = float3(0.0f, 0.0f, 0.0f);
        if (!(tmp164 < 0.0f)) {
            tmp167 = sret_ptr.ior1.x;
            if (tmp167 == -1.0f) {
                tmp168 = state.text_results[28].x;
                tmp169 = state.text_results[28].y;
                tmp170 = state.text_results[28].z;
                sret_ptr.ior1.x = tmp168;
                sret_ptr.ior1.y = tmp169;
                sret_ptr.ior1.z = tmp170;
                phi_in171 = tmp170;
                phi_in173 = tmp169;
                phi_in175 = tmp168;
            } else {
                tmp177 = sret_ptr.ior1.y;
                tmp178 = sret_ptr.ior1.z;
                phi_in171 = tmp178;
                phi_in173 = tmp177;
                phi_in175 = tmp167;
            }
            phi_out172 = phi_in171;
            phi_out174 = phi_in173;
            phi_out176 = phi_in175;
            if (tmp137 > 0.0f) {
                tmp179 = max(1.0f - tmp164 * tmp164, 0.0f);
                tmp180 = tmp137 * 12.56637096f;
                phi_in181 = tmp136;
                phi_in183 = tmp143;
                phi_in185 = tmp140;
                phi_in187 = phi_out172;
                phi_in189 = 409.375f;
                phi_in191 = 0;
                phi_in193 = 0.0f;
                phi_in195 = 0.0f;
                phi_in197 = 0.0f;
                do {
                    phi_out182 = phi_in181;
                    phi_out184 = phi_in183;
                    phi_out186 = phi_in185;
                    phi_out188 = phi_in187;
                    phi_out190 = phi_in189;
                    phi_out192 = phi_in191;
                    phi_out194 = phi_in193;
                    phi_out196 = phi_in195;
                    phi_out198 = phi_in197;
                    float tmp199 = phi_out188 / phi_out182;
                    tmp200 = tmp199 * tmp199 * tmp179;
                    if (tmp200 > 1.0f) {
                        phi_in201 = phi_out182;
                        phi_in203 = phi_out188;
                        phi_in205 = phi_out190;
                        phi_in207 = phi_out192;
                        phi_in209 = phi_out194;
                        phi_in211 = phi_out196;
                        phi_in213 = phi_out198;
                        do {
                            phi_out202 = phi_in201;
                            phi_out204 = phi_in203;
                            phi_out206 = phi_in205;
                            phi_out208 = phi_in207;
                            phi_out210 = phi_in209;
                            phi_out212 = phi_in211;
                            phi_out214 = phi_in213;
                            int tmp215 = phi_out208;
                            tmp216 = glob_cnst91[tmp215].x + phi_out214;
                            tmp217 = glob_cnst91[tmp215].y + phi_out212;
                            tmp218 = glob_cnst91[tmp215].z + phi_out210;
                            tmp219 = phi_out206 + 18.75f;
                            tmp220 = phi_out208 + 1;
                            rmemload160 = tmp3;
                            rmemload164 = tmp0;
                            phi_in221 = rmemload164;
                            phi_in223 = rmemload160;
                            phi_in225 = phi_out176;
                            phi_in227 = tmp134;
                            if (!(uint(phi_out208) > 8u)) {
                                rmemload159 = tmp5;
                                rmemload163 = tmp2;
                                phi_in221 = rmemload163;
                                phi_in223 = rmemload159;
                                phi_in225 = phi_out172;
                                phi_in227 = tmp136;
                                if (uint(phi_out208) > 3u) {
                                    rmemload158 = tmp4;
                                    rmemload162 = tmp1;
                                    phi_in221 = rmemload162;
                                    phi_in223 = rmemload158;
                                    phi_in225 = phi_out174;
                                    phi_in227 = tmp135;
                                }
                            }
                            phi_out222 = phi_in221;
                            phi_out224 = phi_in223;
                            phi_out226 = phi_in225;
                            phi_out228 = phi_in227;
                            bool tmp229 = phi_out226 != phi_out204 || phi_out228 != phi_out202;
                            tmp230 = tmp229 ? phi_out226 : phi_out204;
                            tmp231 = tmp229 ? phi_out228 : phi_out202;
                            phi_in232 = phi_out222;
                            phi_in234 = phi_out224;
                            phi_in236 = tmp230;
                            phi_in238 = tmp231;
                            phi_in240 = tmp216;
                            phi_in242 = tmp217;
                            phi_in244 = tmp218;
                            phi_in246 = tmp219;
                            phi_in248 = tmp220;
                            phi_in201 = tmp231;
                            phi_in203 = tmp230;
                            phi_in205 = tmp219;
                            phi_in207 = tmp220;
                            phi_in209 = tmp218;
                            phi_in211 = tmp217;
                            phi_in213 = tmp216;
                            if (uint(phi_out208) > 14u || tmp229)
                                break;
                        } while (true);
                        phi_out233 = phi_in232;
                        phi_out235 = phi_in234;
                        phi_out237 = phi_in236;
                        phi_out239 = phi_in238;
                        phi_out241 = phi_in240;
                        phi_out243 = phi_in242;
                        phi_out245 = phi_in244;
                        phi_out247 = phi_in246;
                        phi_out249 = phi_in248;
                        phi_in250 = phi_out241;
                        phi_in252 = phi_out243;
                        phi_in254 = phi_out245;
                        phi_in256 = phi_out249;
                        phi_in258 = phi_out247;
                        phi_in260 = phi_out237;
                        phi_in262 = phi_out235;
                        phi_in264 = phi_out233;
                        phi_in266 = phi_out239;
                    } else {
                        float tmp268 = sqrt(max(1.0f - tmp200, 0.0f));
                        float tmp269 = phi_out188 * tmp164;
                        tmp270 = tmp268 * phi_out182;
                        float tmp271 = (tmp269 - tmp270) / (tmp270 + tmp269);
                        float tmp272 = tmp268 * phi_out188;
                        float tmp273 = phi_out182 * tmp164;
                        float tmp274 = (tmp273 - tmp272) / (tmp272 + tmp273);
                        tmp275 = tmp271 * tmp271;
                        tmp276 = tmp274 * tmp274;
                        float tmp277 = phi_out186 * phi_out186;
                        float tmp278 = phi_out184 * phi_out184;
                        float tmp279 = phi_out182 * phi_out182;
                        float tmp280 = tmp278 - tmp277;
                        float tmp281 = (tmp280 - tmp200 * tmp279) * 0.5f;
                        float tmp282 = sqrt(max(tmp281 * tmp281 + tmp277 * tmp278, 0.0f));
                        tmp283 = tmp282 - tmp281;
                        tmp284 = sqrt(max(tmp282 + tmp281, 0.0f));
                        float tmp285 = sqrt(max(tmp283, 0.0f));
                        tmp286 = tmp268 * tmp280;
                        tmp287 = tmp284 * phi_out182;
                        float tmp288 = phi_out184 * 2.0f * phi_out186;
                        tmp289 = tmp268 * tmp288;
                        tmp290 = tmp285 * phi_out182;
                        float tmp291 = tmp270 * 2.0f;
                        tmp292 = tmp285 * tmp291;
                        float tmp293 = tmp282 * 2.0f;
                        tmp294 = tmp293 - tmp270 * tmp270;
                        tmp295 = (tmp284 * tmp288 - tmp285 * tmp280) * tmp291;
                        float tmp296 = tmp268 * (tmp277 + tmp278);
                        tmp297 = tmp296 * tmp296 - tmp293 * tmp279;
                        tmp298 = tmp292 * tmp292 + tmp294 * tmp294;
                        phi_in299 = 0.0f;
                        if (tmp298 > 0.0f) {
                            tmp301 = 1.0f / sqrt(tmp298);
                            phi_in299 = tmp301;
                        }
                        phi_out300 = phi_in299;
                        tmp302 = tmp295 * tmp295 + tmp297 * tmp297;
                        phi_in303 = 0.0f;
                        if (tmp302 > 0.0f) {
                            tmp305 = 1.0f / sqrt(tmp302);
                            phi_in303 = tmp305;
                        }
                        phi_out304 = phi_in303;
                        tmp306 = phi_out300 * tmp294;
                        tmp307 = phi_out304 * tmp297;
                        tmp308 = phi_out300 * tmp292;
                        tmp309 = phi_out304 * tmp295;
                        float tmp310 = tmp270 - tmp284;
                        float tmp311 = tmp284 + tmp270;
                        float tmp312 = (tmp310 * tmp310 + tmp283) / (tmp311 * tmp311 + tmp283);
                        float tmp313 = tmp286 - tmp287;
                        float tmp314 = tmp289 - tmp290;
                        float tmp315 = tmp287 + tmp286;
                        float tmp316 = tmp290 + tmp289;
                        float tmp317 = (tmp314 * tmp314 + tmp313 * tmp313) / (tmp316 * tmp316 + tmp315 * tmp315);
                        tmp318 = tmp180 * tmp270;
                        float tmp319 = max(tmp312 * tmp275, 0.0f);
                        float tmp320 = max(tmp317 * tmp276, 0.0f);
                        tmp321 = sqrt(tmp319) * 2.0f;
                        tmp322 = tmp312 + tmp275;
                        tmp323 = tmp319 + 1.0f;
                        tmp324 = sqrt(tmp320) * 2.0f;
                        tmp325 = tmp317 + tmp276;
                        tmp326 = tmp320 + 1.0f;
                        phi_in327 = phi_out198;
                        phi_in329 = phi_out196;
                        phi_in331 = phi_out194;
                        phi_in333 = phi_out192;
                        phi_in335 = phi_out190;
                        phi_in337 = phi_out188;
                        phi_in339 = phi_out186;
                        phi_in341 = phi_out184;
                        phi_in343 = phi_out182;
                        do {
                            phi_out328 = phi_in327;
                            phi_out330 = phi_in329;
                            phi_out332 = phi_in331;
                            phi_out334 = phi_in333;
                            phi_out336 = phi_in335;
                            phi_out338 = phi_in337;
                            phi_out340 = phi_in339;
                            phi_out342 = phi_in341;
                            phi_out344 = phi_in343;
                            phi_in345 = phi_out328;
                            phi_in347 = phi_out330;
                            phi_in349 = phi_out332;
                            phi_in351 = 16;
                            phi_in353 = phi_out336;
                            phi_in355 = phi_out338;
                            phi_in357 = phi_out340;
                            phi_in359 = phi_out342;
                            phi_in361 = phi_out344;
                            if (phi_out334 == 16)
                                break;
                            else {
                                float tmp363 = tmp318 / phi_out336;
                                float tmp364 = sin(tmp363);
                                float tmp365 = cos(tmp363);
                                float tmp366 = tmp321 * (tmp306 * tmp365 - tmp308 * tmp364);
                                float tmp367 = tmp324 * (tmp307 * tmp365 - tmp309 * tmp364);
                                int tmp368 = phi_out334;
                                float tmp369 = ((tmp322 + tmp366) / (tmp323 + tmp366) + (tmp325 + tmp367) / (tmp326 + tmp367)) * 0.5f;
                                tmp370 = glob_cnst91[tmp368].x * tmp369 + phi_out328;
                                tmp371 = glob_cnst91[tmp368].y * tmp369 + phi_out330;
                                tmp372 = glob_cnst91[tmp368].z * tmp369 + phi_out332;
                                tmp373 = phi_out336 + 18.75f;
                                tmp374 = phi_out334 + 1;
                                rmemload148 = tmp0;
                                rmemload152 = tmp3;
                                phi_in375 = rmemload152;
                                phi_in377 = rmemload148;
                                phi_in379 = tmp134;
                                phi_in381 = phi_out176;
                                if (!(uint(phi_out334) > 8u)) {
                                    rmemload147 = tmp2;
                                    rmemload151 = tmp5;
                                    phi_in375 = rmemload151;
                                    phi_in377 = rmemload147;
                                    phi_in379 = tmp136;
                                    phi_in381 = phi_out172;
                                    if (uint(phi_out334) > 3u) {
                                        rmemload = tmp1;
                                        rmemload150 = tmp4;
                                        phi_in375 = rmemload150;
                                        phi_in377 = rmemload;
                                        phi_in379 = tmp135;
                                        phi_in381 = phi_out174;
                                    }
                                }
                                phi_out376 = phi_in375;
                                phi_out378 = phi_in377;
                                phi_out380 = phi_in379;
                                phi_out382 = phi_in381;
                                tmp383 = phi_out382 != phi_out338 || (phi_out376 != phi_out340 || (phi_out380 != phi_out344 || phi_out378 != phi_out342));
                                phi_in384 = phi_out382;
                                phi_in386 = phi_out376;
                                phi_in388 = phi_out378;
                                phi_in390 = phi_out380;
                                if (!tmp383) {
                                    phi_in384 = phi_out338;
                                    phi_in386 = phi_out340;
                                    phi_in388 = phi_out342;
                                    phi_in390 = phi_out344;
                                }
                                phi_out385 = phi_in384;
                                phi_out387 = phi_in386;
                                phi_out389 = phi_in388;
                                phi_out391 = phi_in390;
                                phi_in345 = tmp370;
                                phi_in347 = tmp371;
                                phi_in349 = tmp372;
                                phi_in351 = tmp374;
                                phi_in353 = tmp373;
                                phi_in355 = phi_out385;
                                phi_in357 = phi_out387;
                                phi_in359 = phi_out389;
                                phi_in361 = phi_out391;
                                phi_in327 = tmp370;
                                phi_in329 = tmp371;
                                phi_in331 = tmp372;
                                phi_in333 = tmp374;
                                phi_in335 = tmp373;
                                phi_in337 = phi_out385;
                                phi_in339 = phi_out387;
                                phi_in341 = phi_out389;
                                phi_in343 = phi_out391;
                                if (tmp383)
                                    break;
                            }
                        } while (true);
                        phi_out346 = phi_in345;
                        phi_out348 = phi_in347;
                        phi_out350 = phi_in349;
                        phi_out352 = phi_in351;
                        phi_out354 = phi_in353;
                        phi_out356 = phi_in355;
                        phi_out358 = phi_in357;
                        phi_out360 = phi_in359;
                        phi_out362 = phi_in361;
                        phi_in250 = phi_out346;
                        phi_in252 = phi_out348;
                        phi_in254 = phi_out350;
                        phi_in256 = phi_out352;
                        phi_in258 = phi_out354;
                        phi_in260 = phi_out356;
                        phi_in262 = phi_out358;
                        phi_in264 = phi_out360;
                        phi_in266 = phi_out362;
                    }
                    phi_out251 = phi_in250;
                    phi_out253 = phi_in252;
                    phi_out255 = phi_in254;
                    phi_out257 = phi_in256;
                    phi_out259 = phi_in258;
                    phi_out261 = phi_in260;
                    phi_out263 = phi_in262;
                    phi_out265 = phi_in264;
                    phi_out267 = phi_in266;
                    phi_in181 = phi_out267;
                    phi_in183 = phi_out265;
                    phi_in185 = phi_out263;
                    phi_in187 = phi_out261;
                    phi_in189 = phi_out259;
                    phi_in191 = phi_out257;
                    phi_in193 = phi_out255;
                    phi_in195 = phi_out253;
                    phi_in197 = phi_out251;
                    phi_in392 = phi_out251;
                    phi_in394 = phi_out253;
                    phi_in396 = phi_out255;
                    if (!(uint(phi_out257) < 16u))
                        break;
                } while (true);
                phi_out393 = phi_in392;
                phi_out395 = phi_in394;
                phi_out397 = phi_in396;
                tmp398 = float3(min(max(phi_out395 * -0.2216216922f + phi_out393 * 0.467204839f + phi_out397 * -0.07188431919f, 0.0f), 1.0f), min(max(phi_out395 * 0.3432191908f + phi_out393 * -0.1772817373f + phi_out397 * 0.007593344897f, 0.0f), 1.0f), min(max(phi_out395 * -0.0389967896f + phi_out393 * 0.01064765267f + phi_out397 * 0.2020568848f, 0.0f), 1.0f));
                phi_in165 = tmp398;
            } else {
                float tmp399 = 1.0f / phi_out176;
                float tmp400 = 1.0f / phi_out174;
                float tmp401 = 1.0f / phi_out172;
                float tmp402 = tmp399 * tmp141;
                float tmp403 = tmp400 * tmp142;
                float tmp404 = tmp401 * tmp143;
                float tmp405 = tmp399 * tmp138;
                float tmp406 = tmp400 * tmp139;
                float tmp407 = tmp401 * tmp140;
                float tmp408 = tmp164 * tmp164;
                float tmp409 = 1.0f - tmp408;
                float tmp410 = -tmp409;
                float tmp411 = tmp402 * tmp402;
                float tmp412 = tmp405 * tmp405;
                float tmp413 = tmp410 - tmp412 + tmp411;
                float tmp414 = sqrt(tmp413 * tmp413 + tmp411 * 4.0f * tmp412);
                float tmp415 = tmp414 + tmp408;
                float tmp416 = tmp164 * 2.0f;
                float tmp417 = sqrt(max((tmp414 + tmp413) * 0.5f, 0.0f)) * tmp416;
                float tmp418 = (tmp415 - tmp417) / (tmp417 + tmp415);
                float tmp419 = tmp409 * tmp409;
                float tmp420 = tmp414 * tmp408 + tmp419;
                float tmp421 = tmp417 * tmp409;
                float tmp422 = tmp403 * tmp403;
                float tmp423 = tmp406 * tmp406;
                float tmp424 = tmp410 - tmp423 + tmp422;
                float tmp425 = sqrt(tmp424 * tmp424 + tmp422 * 4.0f * tmp423);
                float tmp426 = tmp425 + tmp408;
                float tmp427 = sqrt(max((tmp425 + tmp424) * 0.5f, 0.0f)) * tmp416;
                float tmp428 = (tmp426 - tmp427) / (tmp427 + tmp426);
                float tmp429 = tmp425 * tmp408 + tmp419;
                float tmp430 = tmp427 * tmp409;
                float tmp431 = tmp404 * tmp404;
                float tmp432 = tmp407 * tmp407;
                float tmp433 = tmp410 - tmp432 + tmp431;
                float tmp434 = sqrt(tmp433 * tmp433 + tmp431 * 4.0f * tmp432);
                float tmp435 = tmp434 + tmp408;
                float tmp436 = sqrt(max((tmp434 + tmp433) * 0.5f, 0.0f)) * tmp416;
                float tmp437 = (tmp435 - tmp436) / (tmp436 + tmp435);
                float tmp438 = tmp434 * tmp408 + tmp419;
                float tmp439 = tmp436 * tmp409;
                tmp440 = float3(min(max((tmp418 * (tmp420 - tmp421) / (tmp421 + tmp420) + tmp418) * 0.5f, 0.0f), 1.0f), min(max((tmp428 * (tmp429 - tmp430) / (tmp430 + tmp429) + tmp428) * 0.5f, 0.0f), 1.0f), min(max((tmp437 * (tmp438 - tmp439) / (tmp439 + tmp438) + tmp437) * 0.5f, 0.0f), 1.0f));
                phi_in165 = tmp440;
            }
        }
        phi_out166 = phi_in165;
        tmp441 = phi_out166.x;
        tmp442 = phi_out166.y;
        tmp443 = phi_out166.z;
        if (tmp443 == 0.0f && (tmp441 == 0.0f && tmp442 == 0.0f)) {
            sret_ptr.pdf = 0.0f;
            phi_in444 = 0.0f;
        } else {
            tmp446 = tmp131 * tmp441;
            tmp447 = tmp132 * tmp442;
            tmp448 = tmp133 * tmp443;
            tmp449 = state.text_results[17].w;
            tmp450 = state.text_results[17].z;
            tmp451 = max(tmp450, 1.000000012e-07f);
            tmp452 = max(tmp449, 1.000000012e-07f);
            float3 tmp453 = state.text_results[23].yzw;
            tmp454 = tmp147.zxy;
            tmp455 = tmp147.yzx;
            tmp456 = tmp453.yzx * tmp454 - tmp453.zxy * tmp455;
            float3 tmp457 = tmp456 * tmp456;
            tmp458 = tmp457.x + tmp457.y + tmp457.z;
            if (tmp458 < 9.999999939e-09f) {
                sret_ptr.pdf = 0.0f;
                phi_in459 = 0.0f;
                phi_in461 = float3(0.0f, 0.0f, 0.0f);
                phi_in463 = 0.0f;
            } else {
                float tmp465 = 1.0f / sqrt(tmp458);
                tmp466 = float3(tmp465 * tmp456.x, tmp465 * tmp456.y, tmp465 * tmp456.z);
                tmp467 = tmp466.zxy * tmp455 - tmp466.yzx * tmp454;
                if (sret_ptr.ior1.x == -1.0f) {
                    sret_ptr.ior1.x = state.text_results[28].x;
                    sret_ptr.ior1.y = state.text_results[28].y;
                    sret_ptr.ior1.z = state.text_results[28].z;
                }
                if (sret_ptr.ior2.x == -1.0f) {
                    sret_ptr.ior2.x = state.text_results[28].x;
                    sret_ptr.ior2.y = state.text_results[28].y;
                    sret_ptr.ior2.z = state.text_results[28].z;
                }
                float3 tmp468 = tmp147 * tmp17;
                tmp469 = tmp468.x + tmp468.y + tmp468.z;
                tmp470 = abs(tmp469);
                tmp471 = tmp148.x + tmp148.y + tmp148.z;
                tmp472 = abs(tmp471);
                phi_in473 = 0.0f;
                phi_in475 = float3(0.0f, 0.0f, 0.0f);
                if (!tmp36) {
                    float tmp477 = tmp27 + tmp14;
                    float tmp478 = tmp28 + tmp15;
                    float tmp479 = tmp29 + tmp16;
                    float3 tmp480 = float3(sqrt(tmp478 * tmp478 + tmp477 * tmp477 + tmp479 * tmp479), 0.0f, 0.0f);
                    tmp481 = float3(tmp477, tmp478, tmp479) / tmp480.xxx;
                    float3 tmp482 = tmp147 * tmp481;
                    tmp483 = tmp482.x + tmp482.y + tmp482.z;
                    float3 tmp484 = tmp481 * tmp17;
                    float3 tmp485 = tmp481 * tmp30;
                    phi_in473 = 0.0f;
                    phi_in475 = float3(0.0f, 0.0f, 0.0f);
                    if (!(tmp485.x + tmp485.y + tmp485.z < 0.0f || (tmp484.x + tmp484.y + tmp484.z < 0.0f || tmp483 < 0.0f))) {
                        float3 tmp486 = tmp467 * tmp481;
                        float3 tmp487 = tmp466 * tmp481;
                        float tmp488 = 1.0f / tmp451;
                        float tmp489 = 1.0f / tmp452;
                        float tmp490 = (tmp486.x + tmp486.y + tmp486.z) * tmp488;
                        float tmp491 = (tmp487.x + tmp487.y + tmp487.z) * tmp489;
                        float tmp492 = tmp491 * tmp491 + tmp483 * tmp483 + tmp490 * tmp490;
                        float3 tmp493 = tmp467 * tmp17;
                        float3 tmp494 = tmp466 * tmp17;
                        float3 tmp495 = tmp467 * tmp30;
                        float3 tmp496 = tmp466 * tmp30;
                        float tmp497 = (tmp493.x + tmp493.y + tmp493.z) * tmp451;
                        float tmp498 = (tmp494.x + tmp494.y + tmp494.z) * tmp452;
                        float tmp499 = (tmp495.x + tmp495.y + tmp495.z) * tmp451;
                        float tmp500 = (tmp496.x + tmp496.y + tmp496.z) * tmp452;
                        tmp501 = tmp483 * 0.1591549367f * tmp489 * tmp488 / (tmp492 * tmp492 * (tmp470 * tmp483) * (sqrt((tmp497 * tmp497 + tmp498 * tmp498) / (tmp469 * tmp469) + 1.0f) + 1.0f));
                        float tmp502 = tmp501 * 2.0f / (sqrt((tmp499 * tmp499 + tmp500 * tmp500) / (tmp471 * tmp471) + 1.0f) + 1.0f);
                        tmp503 = float3(tmp502, tmp502, tmp502);
                        phi_in473 = tmp501;
                        phi_in475 = tmp503;
                    }
                }
                phi_out474 = phi_in473;
                phi_out476 = phi_in475;
                tmp504 = min(max(sqrt(tmp450 * tmp449), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
                tmp505 = tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp470, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp504, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                tmp506 = tmp505 * phi_out474;
                sret_ptr.pdf = tmp506;
                phi_in459 = tmp506;
                phi_in461 = phi_out476;
                phi_in463 = 0.0f;
                if (!(tmp35 < 0.0f)) {
                    tmp507 = (1.0f - tmp505) * tmp472 * (1.0f - tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp472, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp504, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x) / tex_lookup_float3_3d(1, float3(1.0f, tmp504, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                    tmp508 = 0.3183098733f - tmp505 * 0.3183098733f + tmp506;
                    sret_ptr.pdf = tmp508;
                    phi_in459 = tmp508;
                    phi_in461 = phi_out476;
                    phi_in463 = tmp507;
                }
            }
            phi_out460 = phi_in459;
            phi_out462 = phi_in461;
            phi_out464 = phi_in463;
            sret_ptr.bsdf_diffuse.x = phi_out464 * tmp446 + tmp117;
            sret_ptr.bsdf_diffuse.y = phi_out464 * tmp447 + tmp117;
            sret_ptr.bsdf_diffuse.z = phi_out464 * tmp448 + tmp117;
            sret_ptr.bsdf_glossy.x = phi_out462.x * tmp446 + tmp118;
            sret_ptr.bsdf_glossy.y = phi_out462.y * tmp447 + tmp119;
            sret_ptr.bsdf_glossy.z = phi_out462.z * tmp448 + tmp120;
            phi_in444 = phi_out460;
        }
        phi_out445 = phi_in444;
        tmp509 = phi_out445 * tmp128;
        phi_in129 = tmp509;
    }
    phi_out130 = phi_in129;
    phi_in510 = phi_out130;
    if (tmp128 < 1.0f) {
        tmp512 = 1.0f - tmp128;
        tmp513 = tmp512 * tmp122;
        tmp514 = tmp512 * tmp123;
        tmp515 = tmp512 * tmp124;
        tmp516 = min(max(state.text_results[12].x, 0.0f), 1.0f);
        phi_in517 = 0.0f;
        if (tmp516 > 0.0f) {
            tmp519 = tmp516 * tmp513;
            tmp520 = tmp516 * tmp514;
            tmp521 = tmp516 * tmp515;
            tmp522 = min(max(state.text_results[20].x, 0.0f), 1.0f);
            phi_in523 = 0.0f;
            if (tmp522 > 0.0f) {
                tmp525 = tmp522 * tmp519;
                tmp526 = tmp522 * tmp520;
                tmp527 = tmp522 * tmp521;
                tmp528 = state.arg_block_offset;
                tmp529 = mdl_read_argblock_as_bool(tmp528);
                phi_in530 = true;
                if (!tmp529) {
                    phi_in530 = true;
                    if (!(state.text_results[12].y == 0.0f))
                        phi_in530 = false;
                }
                phi_out531 = phi_in530;
                float3 tmp532 = phi_out531 ? float3(state.text_results[12].z, state.text_results[12].w, state.text_results[13].x) : float3(1.0f, 1.0f, 1.0f);
                float tmp533 = state.text_results[26].z;
                float tmp534 = state.text_results[26].w;
                float tmp535 = state.text_results[27].x;
                tmp536 = tmp125 * tmp19;
                tmp537 = tmp126 * tmp19;
                tmp538 = tmp127 * tmp19;
                tmp539 = float3(tmp536, tmp537, tmp538);
                float3 tmp540 = tmp539 * tmp30;
                tmp541 = tmp540.x + tmp540.y + tmp540.z;
                bool tmp542 = tmp541 < 0.0f;
                tmp543 = tmp525 * min(max(tmp542 ? tmp532.x : tmp533, 0.0f), 1.0f);
                tmp544 = tmp526 * min(max(tmp542 ? tmp532.y : tmp534, 0.0f), 1.0f);
                tmp545 = tmp527 * min(max(tmp542 ? tmp532.z : tmp535, 0.0f), 1.0f);
                float3 tmp546 = float3(tmp533, tmp534, tmp535) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                tmp547 = state.text_results[17].w;
                tmp548 = state.text_results[17].z;
                tmp549 = max(tmp548, 1.000000012e-07f);
                tmp550 = max(tmp547, 1.000000012e-07f);
                tmp7.m_0 = (state.text_results[30].z + state.text_results[30].y + state.text_results[30].w) * 0.3333333433f;
                tmp7.m_1 = mdl_read_argblock_as_bool(tmp528 + 236) ? state.text_results[19].w : 0.0f;
                tmp551 = tmp546.z > 0.0f || (tmp546.x > 0.0f || tmp546.y > 0.0f);
                float3 tmp552 = state.text_results[23].yzw;
                tmp553 = tmp539.zxy;
                tmp554 = tmp539.yzx;
                tmp555 = tmp552.yzx * tmp553 - tmp552.zxy * tmp554;
                float3 tmp556 = tmp555 * tmp555;
                tmp557 = tmp556.x + tmp556.y + tmp556.z;
                phi_in558 = 0.0f;
                phi_in560 = float3(0.0f, 0.0f, 0.0f);
                phi_in562 = 0.0f;
                if (!(tmp557 < 9.999999939e-09f)) {
                    float tmp564 = 1.0f / sqrt(tmp557);
                    tmp565 = float3(tmp564 * tmp555.x, tmp564 * tmp555.y, tmp564 * tmp555.z);
                    tmp566 = tmp565.zxy * tmp554 - tmp565.yzx * tmp553;
                    tmp567 = sret_ptr.ior1.x;
                    phi_in568 = tmp567;
                    if (tmp567 == -1.0f) {
                        tmp570 = state.text_results[28].x;
                        sret_ptr.ior1.x = tmp570;
                        sret_ptr.ior1.y = state.text_results[28].y;
                        sret_ptr.ior1.z = state.text_results[28].z;
                        phi_in568 = tmp570;
                    }
                    phi_out569 = phi_in568;
                    tmp571 = sret_ptr.ior2.x;
                    if (tmp571 == -1.0f) {
                        tmp572 = state.text_results[28].x;
                        tmp573 = state.text_results[28].y;
                        tmp574 = state.text_results[28].z;
                        sret_ptr.ior2.x = tmp572;
                        sret_ptr.ior2.y = tmp573;
                        sret_ptr.ior2.z = tmp574;
                        phi_in575 = tmp574;
                        phi_in577 = tmp573;
                        phi_in579 = tmp572;
                    } else {
                        tmp581 = sret_ptr.ior2.y;
                        tmp582 = sret_ptr.ior2.z;
                        phi_in575 = tmp582;
                        phi_in577 = tmp581;
                        phi_in579 = tmp571;
                    }
                    phi_out576 = phi_in575;
                    phi_out578 = phi_in577;
                    phi_out580 = phi_in579;
                    tmp583 = (sret_ptr.ior1.y + phi_out569 + sret_ptr.ior1.z) * 0.3333333433f;
                    float tmp584 = (phi_out578 + phi_out576 + phi_out580) * 0.3333333433f;
                    float tmp585 = tmp584 - tmp583;
                    tmp586 = abs(tmp585) < 9.999999747e-05f ? tmp583 + asfloat((asint(tmp585) & -2147483648) | 953267991) : tmp584;
                    tmp6.m_0 = tmp583;
                    tmp6.m_1 = tmp586;
                    float3 tmp587 = tmp539 * tmp17;
                    tmp588 = tmp587.x + tmp587.y + tmp587.z;
                    tmp589 = abs(tmp588);
                    tmp590 = abs(tmp541);
                    if (tmp36) {
                        if (tmp529) {
                            float tmp591 = tmp590 * 2.0f;
                            tmp592 = tmp27 + tmp14 + tmp591 * tmp536;
                            tmp593 = tmp28 + tmp15 + tmp591 * tmp537;
                            tmp594 = tmp29 + tmp16 + tmp591 * tmp538;
                            phi_in595 = tmp592;
                            phi_in597 = tmp593;
                            phi_in599 = tmp594;
                        } else {
                            tmp601 = tmp586 * tmp27 + tmp583 * tmp14;
                            tmp602 = tmp586 * tmp28 + tmp583 * tmp15;
                            tmp603 = tmp586 * tmp29 + tmp583 * tmp16;
                            phi_in595 = tmp601;
                            phi_in597 = tmp602;
                            phi_in599 = tmp603;
                            if (tmp586 > tmp583) {
                                tmp604 = -tmp603;
                                tmp605 = -tmp602;
                                tmp606 = -tmp601;
                                phi_in595 = tmp606;
                                phi_in597 = tmp605;
                                phi_in599 = tmp604;
                            }
                        }
                    }
                    else {
                        tmp607 = tmp27 + tmp14;
                        tmp608 = tmp28 + tmp15;
                        tmp609 = tmp29 + tmp16;
                        phi_in595 = tmp607;
                        phi_in597 = tmp608;
                        phi_in599 = tmp609;
                    }
                    phi_out596 = phi_in595;
                    phi_out598 = phi_in597;
                    phi_out600 = phi_in599;
                    float3 tmp610 = float3(sqrt(phi_out598 * phi_out598 + phi_out596 * phi_out596 + phi_out600 * phi_out600), 0.0f, 0.0f);
                    tmp611 = float3(phi_out596, phi_out598, phi_out600) / tmp610.xxx;
                    float3 tmp612 = tmp611 * tmp539;
                    tmp613 = tmp612.x + tmp612.y + tmp612.z;
                    float3 tmp614 = tmp611 * tmp17;
                    tmp615 = tmp614.x + tmp614.y + tmp614.z;
                    float3 tmp616 = tmp611 * tmp30;
                    float tmp617 = tmp616.x + tmp616.y + tmp616.z;
                    tmp618 = tmp36 ? -tmp617 : tmp617;
                    phi_in619 = 0.0f;
                    phi_in621 = float3(0.0f, 0.0f, 0.0f);
                    if (!(tmp613 < 0.0f || tmp615 < 0.0f || tmp618 < 0.0f)) {
                        structtype1 tmp623 = _ZNK23Fresnel_function_coated4evalERK6float2f(tmp7, tmp6, tmp615);
                        tmp624 = tmp623.m_0;
                        tmp625 = tmp623.m_1;
                        tmp626 = tmp623.m_2;
                        tmp627 = tmp623.m_3;
                        float3 tmp628 = tmp611 * tmp566;
                        float3 tmp629 = tmp611 * tmp565;
                        float tmp630 = 1.0f / tmp549;
                        float tmp631 = 1.0f / tmp550;
                        float tmp632 = (tmp628.x + tmp628.y + tmp628.z) * tmp630;
                        float tmp633 = (tmp629.x + tmp629.y + tmp629.z) * tmp631;
                        float tmp634 = tmp632 * tmp632 + tmp613 * tmp613 + tmp633 * tmp633;
                        tmp635 = tmp631 * 0.3183098733f * tmp630 * tmp613 / (tmp634 * tmp634);
                        float3 tmp636 = tmp566 * tmp17;
                        float3 tmp637 = tmp565 * tmp17;
                        float3 tmp638 = tmp566 * tmp30;
                        float3 tmp639 = tmp565 * tmp30;
                        float tmp640 = (tmp636.x + tmp636.y + tmp636.z) * tmp549;
                        float tmp641 = (tmp637.x + tmp637.y + tmp637.z) * tmp550;
                        tmp642 = sqrt((tmp640 * tmp640 + tmp641 * tmp641) / (tmp588 * tmp588) + 1.0f) + 1.0f;
                        float tmp643 = (tmp638.x + tmp638.y + tmp638.z) * tmp549;
                        float tmp644 = (tmp639.x + tmp639.y + tmp639.z) * tmp550;
                        tmp645 = sqrt((tmp643 * tmp643 + tmp644 * tmp644) / (tmp541 * tmp541) + 1.0f) + 1.0f;
                        if (tmp36 && tmp529 != true) {
                            float tmp646 = tmp615 * tmp583 - tmp618 * tmp586;
                            tmp647 = tmp618 * tmp615;
                            tmp648 = tmp646 * tmp646 * (tmp613 * tmp589);
                            tmp649 = tmp635 * 2.0f;
                            phi_in650 = tmp649;
                            phi_in652 = tmp647;
                            phi_in654 = tmp648;
                        } else {
                            tmp656 = tmp613 * tmp589;
                            phi_in650 = 0.5f;
                            phi_in652 = tmp635;
                            phi_in654 = tmp656;
                        }
                        phi_out651 = phi_in650;
                        phi_out653 = phi_in652;
                        phi_out655 = phi_in654;
                        float tmp657 = phi_out653 * phi_out651 / (phi_out655 * tmp642);
                        float tmp658 = tmp657 * 2.0f / tmp645;
                        tmp659 = float3(tmp658 * (tmp36 ? 1.0f - tmp624 : tmp624), tmp658 * (tmp36 ? 1.0f - tmp625 : tmp625), tmp658 * (tmp36 ? 1.0f - tmp626 : tmp626));
                        tmp660 = tmp657 * (tmp36 ? 1.0f - tmp627 : tmp627);
                        phi_in619 = tmp660;
                        phi_in621 = tmp659;
                    }
                    phi_out620 = phi_in619;
                    phi_out622 = phi_in621;
                    phi_in661 = phi_out620;
                    phi_in663 = 0.0f;
                    if (tmp551) {
                        tmp665 = tmp529 ? -1.0f : tmp583 / tmp586;
                        tmp666 = min(max(acos(min(max(tmp589, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f);
                        tmp667 = min(max(sqrt(tmp548 * tmp547), 0.0f), 1.0f);
                        phi_in668 = 0.0f;
                        if (!(tmp665 < 0.0f)) {
                            bool tmp670 = tmp665 < 1.0f;
                            tmp671 = (min(max((tmp670 ? 1.0f / tmp665 : tmp665) * 0.5333333611f + -0.6000000238f, 0.0f), 1.0f) * 15.0f + (tmp670 ? 1.5f : 17.5f)) * 0.03030303121f;
                            phi_in668 = tmp671;
                        }
                        phi_out669 = phi_in668;
                        tmp672 = tmp667 * 0.984375f + 0.0078125f;
                        tmp673 = tex_lookup_float3_3d(1, float3(tmp666 * 0.9692307711f + 0.007692307699f, tmp672, phi_out669), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                        tmp674 = tmp673 * phi_out620;
                        phi_in661 = tmp674;
                        phi_in663 = 0.0f;
                        if (!(tmp35 < 0.0f)) {
                            tmp675 = (1.0f - tmp673) * tmp590 * (1.0f - tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp590, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp672, phi_out669), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x) / tex_lookup_float3_3d(1, float3(1.0f, tmp672, phi_out669), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                            tmp676 = 0.3183098733f - tmp673 * 0.3183098733f + tmp674;
                            sret_ptr.pdf = tmp676;
                            phi_in661 = tmp676;
                            phi_in663 = tmp675;
                        }
                    }
                    phi_out662 = phi_in661;
                    phi_out664 = phi_in663;
                    phi_in558 = phi_out662;
                    phi_in560 = phi_out622;
                    phi_in562 = phi_out664;
                }
                phi_out559 = phi_in558;
                phi_out561 = phi_in560;
                phi_out563 = phi_in562;
                sret_ptr.bsdf_diffuse.x = min(max(tmp546.x, 0.0f), 1.0f) * tmp543 * phi_out563 + sret_ptr.bsdf_diffuse.x;
                sret_ptr.bsdf_diffuse.y = sret_ptr.bsdf_diffuse.y + min(max(tmp546.y, 0.0f), 1.0f) * tmp544 * phi_out563;
                sret_ptr.bsdf_diffuse.z = sret_ptr.bsdf_diffuse.z + min(max(tmp546.z, 0.0f), 1.0f) * tmp545 * phi_out563;
                sret_ptr.bsdf_glossy.x = sret_ptr.bsdf_glossy.x + phi_out561.x * tmp543;
                sret_ptr.bsdf_glossy.y = sret_ptr.bsdf_glossy.y + phi_out561.y * tmp544;
                sret_ptr.bsdf_glossy.z = sret_ptr.bsdf_glossy.z + phi_out561.z * tmp545;
                tmp677 = phi_out559 * tmp522;
                phi_in523 = tmp677;
            }
            phi_out524 = phi_in523;
            phi_in678 = phi_out524;
            if (tmp522 < 1.0f) {
                tmp680 = 1.0f - tmp522;
                tmp681 = tmp680 * tmp519;
                tmp682 = tmp680 * tmp520;
                tmp683 = tmp680 * tmp521;
                tmp684 = state.normal;
                float tmp685 = state.text_results[17].y;
                tmp686 = min(max(tmp685 > 0.349999994f ? tmp685 + -0.349999994f : 0.0f, 0.0f), 1.0f);
                phi_in687 = 0.0f;
                if (tmp686 > 0.0f) {
                    tmp689 = tmp686 * tmp681;
                    tmp690 = tmp686 * tmp682;
                    tmp691 = tmp686 * tmp683;
                    phi_in692 = true;
                    if (!mdl_read_argblock_as_bool(state.arg_block_offset)) {
                        phi_in692 = true;
                        if (!(state.text_results[12].y == 0.0f))
                            phi_in692 = false;
                    }
                    phi_out693 = phi_in692;
                    tmp694 = phi_out693 ? float3(state.text_results[12].z, state.text_results[12].w, state.text_results[13].x) : float3(1.0f, 1.0f, 1.0f);
                    float3 tmp695 = state.geom_normal;
                    float3 tmp696 = sret_ptr.k1 * tmp695;
                    tmp697 = asfloat((asint(tmp696.x + tmp696.y + tmp696.z) & -2147483648) | 1065353216);
                    tmp698 = sret_ptr.k2;
                    tmp699 = float3(tmp695.x * tmp697, tmp695.y * tmp697, tmp695.z * tmp697);
                    float3 tmp700 = tmp699 * tmp698;
                    phi_in701 = 0.0f;
                    if (tmp700.x + tmp700.y + tmp700.z < 0.0f) {
                        float3 tmp703 = float3(tmp684.x * tmp697, tmp684.y * tmp697, tmp684.z * tmp697);
                        float3 tmp704 = tmp703 * tmp699;
                        float3 tmp705 = tmp703 * tmp698;
                        float tmp706 = max(-(tmp705.x + tmp705.y + tmp705.z), 0.0f);
                        tmp707 = tmp706 / (max(tmp704.x + tmp704.y + tmp704.z, 0.0f) * 1.570796371f + 1.570796371f);
                        bool tmp708 = tmp706 > 0.0f;
                        sret_ptr.bsdf_diffuse.x = sret_ptr.bsdf_diffuse.x + tmp689 * (tmp708 ? tmp707 * min(max(tmp694.x, 0.0f), 1.0f) : 0.0f);
                        sret_ptr.bsdf_diffuse.y = sret_ptr.bsdf_diffuse.y + tmp690 * (tmp708 ? tmp707 * min(max(tmp694.y, 0.0f), 1.0f) : 0.0f);
                        sret_ptr.bsdf_diffuse.z = sret_ptr.bsdf_diffuse.z + tmp691 * (tmp708 ? tmp707 * min(max(tmp694.z, 0.0f), 1.0f) : 0.0f);
                        phi_in701 = tmp707;
                    }
                    phi_out702 = phi_in701;
                    sret_ptr.pdf = phi_out702;
                    tmp709 = phi_out702 * tmp686;
                    phi_in687 = tmp709;
                }
                phi_out688 = phi_in687;
                phi_in710 = phi_out688;
                if (tmp686 < 1.0f) {
                    tmp712 = 1.0f - tmp686;
                    tmp713 = state.text_results[23].y;
                    tmp714 = state.text_results[23].z;
                    tmp715 = state.text_results[23].w;
                    tmp716 = mdl_read_argblock_as_bool(state.arg_block_offset);
                    phi_in717 = true;
                    if (!tmp716) {
                        phi_in717 = true;
                        if (!(state.text_results[12].y == 0.0f))
                            phi_in717 = false;
                    }
                    phi_out718 = phi_in717;
                    tmp719 = phi_out718 ? float3(state.text_results[12].z, state.text_results[12].w, state.text_results[13].x) : float3(1.0f, 1.0f, 1.0f);
                    tmp720 = max(state.text_results[17].z, 1.000000012e-07f);
                    tmp721 = max(state.text_results[17].w, 1.000000012e-07f);
                    float3 tmp722 = state.geom_normal;
                    tmp723 = sret_ptr.k1.x;
                    tmp724 = sret_ptr.k1.y;
                    tmp725 = sret_ptr.k1.z;
                    tmp726 = float3(tmp723, tmp724, tmp725);
                    float3 tmp727 = tmp726 * tmp722;
                    float tmp728 = asfloat((asint(tmp727.x + tmp727.y + tmp727.z) & -2147483648) | 1065353216);
                    tmp729 = tmp722.x * tmp728;
                    tmp730 = tmp722.y * tmp728;
                    tmp731 = tmp722.z * tmp728;
                    tmp732 = tmp125 * tmp728;
                    tmp733 = tmp126 * tmp728;
                    tmp734 = tmp127 * tmp728;
                    float3 tmp735 = float3(tmp713, tmp714, tmp715);
                    tmp736 = float3(tmp732, tmp733, tmp734);
                    tmp737 = tmp736.zxy;
                    tmp738 = tmp736.yzx;
                    tmp739 = tmp737 * tmp735.yzx - tmp738 * tmp735.zxy;
                    float3 tmp740 = tmp739 * tmp739;
                    tmp741 = tmp740.x + tmp740.y + tmp740.z;
                    phi_in742 = 0.0f;
                    phi_in744 = float3(0.0f, 0.0f, 0.0f);
                    if (!(tmp741 < 9.999999939e-09f)) {
                        float tmp746 = 1.0f / sqrt(tmp741);
                        tmp747 = float3(tmp746 * tmp739.x, tmp746 * tmp739.y, tmp746 * tmp739.z);
                        tmp748 = tmp747.zxy * tmp738 - tmp747.yzx * tmp737;
                        tmp749 = sret_ptr.ior1.x;
                        phi_in750 = tmp749;
                        if (tmp749 == -1.0f) {
                            tmp752 = state.text_results[28].x;
                            sret_ptr.ior1.x = tmp752;
                            sret_ptr.ior1.y = state.text_results[28].y;
                            sret_ptr.ior1.z = state.text_results[28].z;
                            phi_in750 = tmp752;
                        }
                        phi_out751 = phi_in750;
                        tmp753 = sret_ptr.ior2.x;
                        if (tmp753 == -1.0f) {
                            tmp754 = state.text_results[28].x;
                            tmp755 = state.text_results[28].y;
                            tmp756 = state.text_results[28].z;
                            sret_ptr.ior2.x = tmp754;
                            sret_ptr.ior2.y = tmp755;
                            sret_ptr.ior2.z = tmp756;
                            phi_in757 = tmp756;
                            phi_in759 = tmp755;
                            phi_in761 = tmp754;
                        } else {
                            tmp763 = sret_ptr.ior2.y;
                            tmp764 = sret_ptr.ior2.z;
                            phi_in757 = tmp764;
                            phi_in759 = tmp763;
                            phi_in761 = tmp753;
                        }
                        phi_out758 = phi_in757;
                        phi_out760 = phi_in759;
                        phi_out762 = phi_in761;
                        tmp765 = (sret_ptr.ior1.y + phi_out751 + sret_ptr.ior1.z) * 0.3333333433f;
                        float tmp766 = (phi_out760 + phi_out758 + phi_out762) * 0.3333333433f;
                        float tmp767 = tmp766 - tmp765;
                        tmp768 = abs(tmp767) < 9.999999747e-05f ? tmp765 + asfloat((asint(tmp767) & -2147483648) | 953267991) : tmp766;
                        float3 tmp769 = tmp736 * tmp726;
                        tmp770 = tmp769.x + tmp769.y + tmp769.z;
                        tmp771 = abs(tmp770);
                        tmp772 = sret_ptr.k2.x;
                        tmp773 = sret_ptr.k2.y;
                        tmp774 = sret_ptr.k2.z;
                        tmp775 = float3(tmp772, tmp773, tmp774);
                        float3 tmp776 = tmp775 * tmp736;
                        tmp777 = tmp776.x + tmp776.y + tmp776.z;
                        tmp778 = abs(tmp777);
                        float3 tmp779 = tmp775 * float3(tmp729, tmp730, tmp731);
                        tmp780 = tmp779.x + tmp779.y + tmp779.z < 0.0f;
                        if (tmp780) {
                            if (tmp716) {
                                float tmp781 = tmp778 * 2.0f;
                                tmp782 = tmp772 + tmp723 + tmp781 * tmp732;
                                tmp783 = tmp773 + tmp724 + tmp781 * tmp733;
                                tmp784 = tmp774 + tmp725 + tmp781 * tmp734;
                                phi_in785 = tmp782;
                                phi_in787 = tmp783;
                                phi_in789 = tmp784;
                            } else {
                                tmp791 = tmp768 * tmp772 + tmp765 * tmp723;
                                tmp792 = tmp768 * tmp773 + tmp765 * tmp724;
                                tmp793 = tmp768 * tmp774 + tmp765 * tmp725;
                                phi_in785 = tmp791;
                                phi_in787 = tmp792;
                                phi_in789 = tmp793;
                                if (tmp768 > tmp765) {
                                    tmp794 = -tmp793;
                                    tmp795 = -tmp792;
                                    tmp796 = -tmp791;
                                    phi_in785 = tmp796;
                                    phi_in787 = tmp795;
                                    phi_in789 = tmp794;
                                }
                            }
                        }
                        else {
                            tmp797 = tmp772 + tmp723;
                            tmp798 = tmp773 + tmp724;
                            tmp799 = tmp774 + tmp725;
                            phi_in785 = tmp797;
                            phi_in787 = tmp798;
                            phi_in789 = tmp799;
                        }
                        phi_out786 = phi_in785;
                        phi_out788 = phi_in787;
                        phi_out790 = phi_in789;
                        float3 tmp800 = float3(sqrt(phi_out788 * phi_out788 + phi_out786 * phi_out786 + phi_out790 * phi_out790), 0.0f, 0.0f);
                        tmp801 = float3(phi_out786, phi_out788, phi_out790) / tmp800.xxx;
                        float3 tmp802 = tmp801 * tmp736;
                        tmp803 = tmp802.x + tmp802.y + tmp802.z;
                        float3 tmp804 = tmp801 * tmp726;
                        tmp805 = tmp804.x + tmp804.y + tmp804.z;
                        float3 tmp806 = tmp801 * tmp775;
                        float tmp807 = tmp806.x + tmp806.y + tmp806.z;
                        tmp808 = tmp780 ? -tmp807 : tmp807;
                        phi_in742 = 0.0f;
                        phi_in744 = float3(0.0f, 0.0f, 0.0f);
                        if (!(tmp803 < 0.0f || tmp805 < 0.0f || tmp808 < 0.0f)) {
                            phi_in809 = 0;
                            phi_in811 = 0.0f;
                            phi_in813 = 0.0f;
                            phi_in815 = float3(0.0f, 0.0f, 0.0f);
                            if (!tmp780) {
                                float tmp817 = tmp765 / tmp768;
                                phi_in809 = 0;
                                phi_in811 = 1.0f;
                                phi_in813 = 0.0f;
                                phi_in815 = float3(0.0f, 0.0f, 0.0f);
                                if (!(tmp817 * tmp817 * (1.0f - tmp805 * tmp805) > 1.0f)) {
                                    phi_in809 = 1;
                                    phi_in811 = 0.0f;
                                    phi_in813 = 0.0f;
                                    phi_in815 = float3(0.0f, 0.0f, 0.0f);
                                }
                            }
                            phi_out810 = phi_in809;
                            phi_out812 = phi_in811;
                            phi_out814 = phi_in813;
                            phi_out816 = phi_in815;
                            phi_in742 = phi_out814;
                            phi_in744 = phi_out816;
                            if (phi_out810 == 0) {
                                float3 tmp818 = tmp801 * tmp748;
                                float3 tmp819 = tmp801 * tmp747;
                                float tmp820 = 1.0f / tmp720;
                                float tmp821 = 1.0f / tmp721;
                                float tmp822 = (tmp818.x + tmp818.y + tmp818.z) * tmp820;
                                float tmp823 = (tmp819.x + tmp819.y + tmp819.z) * tmp821;
                                float tmp824 = tmp822 * tmp822 + tmp803 * tmp803 + tmp823 * tmp823;
                                tmp825 = tmp821 * 0.3183098733f * tmp820 * tmp803 / (tmp824 * tmp824);
                                float3 tmp826 = tmp748 * tmp726;
                                float3 tmp827 = tmp747 * tmp726;
                                float3 tmp828 = tmp775 * tmp748;
                                float3 tmp829 = tmp775 * tmp747;
                                float tmp830 = (tmp826.x + tmp826.y + tmp826.z) * tmp720;
                                float tmp831 = (tmp827.x + tmp827.y + tmp827.z) * tmp721;
                                tmp832 = sqrt((tmp830 * tmp830 + tmp831 * tmp831) / (tmp770 * tmp770) + 1.0f) + 1.0f;
                                float tmp833 = (tmp828.x + tmp828.y + tmp828.z) * tmp720;
                                float tmp834 = (tmp829.x + tmp829.y + tmp829.z) * tmp721;
                                tmp835 = sqrt((tmp834 * tmp834 + tmp833 * tmp833) / (tmp777 * tmp777) + 1.0f) + 1.0f;
                                if (tmp780 && tmp716 != true) {
                                    float tmp836 = tmp805 * tmp765 - tmp808 * tmp768;
                                    tmp837 = tmp808 * tmp805;
                                    tmp838 = tmp836 * tmp836 * (tmp803 * tmp771);
                                    phi_in839 = tmp825;
                                    phi_in841 = tmp837;
                                    phi_in843 = tmp838;
                                } else {
                                    tmp845 = tmp803 * tmp771;
                                    phi_in839 = 0.25f;
                                    phi_in841 = tmp825;
                                    phi_in843 = tmp845;
                                }
                                phi_out840 = phi_in839;
                                phi_out842 = phi_in841;
                                phi_out844 = phi_in843;
                                tmp846 = (tmp780 ? 1.0f - phi_out812 : phi_out812) * 2.0f * phi_out840 * phi_out842 / (phi_out844 * tmp832);
                                float tmp847 = tmp846 * 2.0f / tmp835;
                                tmp848 = float3(tmp847, tmp847, tmp847);
                                phi_in742 = tmp846;
                                phi_in744 = tmp848;
                            }
                        }
                    }
                    phi_out743 = phi_in742;
                    phi_out745 = phi_in744;
                    sret_ptr.bsdf_glossy.x = tmp712 * tmp681 * min(max(tmp719.x, 0.0f), 1.0f) * phi_out745.x + sret_ptr.bsdf_glossy.x;
                    sret_ptr.bsdf_glossy.y = sret_ptr.bsdf_glossy.y + tmp712 * tmp682 * min(max(tmp719.y, 0.0f), 1.0f) * phi_out745.y;
                    sret_ptr.bsdf_glossy.z = sret_ptr.bsdf_glossy.z + tmp712 * tmp683 * min(max(tmp719.z, 0.0f), 1.0f) * phi_out745.z;
                    tmp849 = phi_out743 * tmp712 + phi_out688;
                    phi_in710 = tmp849;
                }
                phi_out711 = phi_in710;
                tmp850 = phi_out711 * tmp680 + phi_out524;
                phi_in678 = tmp850;
            }
            phi_out679 = phi_in678;
            tmp851 = phi_out679 * tmp516;
            phi_in517 = tmp851;
        }
        phi_out518 = phi_in517;
        phi_in852 = phi_out518;
        if (tmp516 < 1.0f) {
            tmp854 = 1.0f - tmp516;
            tmp855 = tmp854 * tmp513;
            tmp856 = tmp854 * tmp514;
            tmp857 = tmp854 * tmp515;
            float tmp858 = state.text_results[17].y;
            float tmp859 = state.text_results[24].w;
            float tmp860 = (tmp859 + -1.0f) / (tmp859 + 1.0f);
            tmp861 = min(max(tmp860 * tmp860, 0.0f), 1.0f);
            tmp862 = min(max(1.0f - tmp858 * tmp858, 0.0f), 1.0f);
            tmp863 = min(max(state.text_results[20].x, 0.0f), 1.0f);
            float3 tmp864 = state.geom_normal;
            tmp865 = sret_ptr.k1.x;
            tmp866 = sret_ptr.k1.y;
            tmp867 = sret_ptr.k1.z;
            tmp868 = float3(tmp865, tmp866, tmp867);
            float3 tmp869 = tmp868 * tmp864;
            float tmp870 = asfloat((asint(tmp869.x + tmp869.y + tmp869.z) & -2147483648) | 1065353216);
            tmp871 = tmp125 * tmp870;
            tmp872 = tmp126 * tmp870;
            tmp873 = tmp127 * tmp870;
            tmp874 = float3(tmp871, tmp872, tmp873);
            float3 tmp875 = tmp874 * tmp868;
            tmp876 = tmp875.x + tmp875.y + tmp875.z;
            tmp877 = min(max(tmp876, 0.0f), 1.0f);
            tmp878 = sret_ptr.k2.x;
            tmp879 = sret_ptr.k2.y;
            tmp880 = sret_ptr.k2.z;
            tmp881 = float3(tmp878, tmp879, tmp880);
            float3 tmp882 = tmp874 * tmp881;
            tmp883 = tmp882.x + tmp882.y + tmp882.z;
            tmp884 = abs(tmp883);
            float3 tmp885 = float3(tmp864.x * tmp870, tmp864.y * tmp870, tmp864.z * tmp870) * tmp881;
            tmp886 = tmp885.x + tmp885.y + tmp885.z;
            tmp887 = tmp886 < 0.0f;
            phi_in888 = tmp880;
            phi_in890 = tmp878;
            phi_in892 = tmp879;
            if (tmp887) {
                float tmp894 = tmp884 * 2.0f;
                tmp895 = tmp894 * tmp871 + tmp878;
                tmp896 = tmp894 * tmp872 + tmp879;
                tmp897 = tmp894 * tmp873 + tmp880;
                phi_in888 = tmp897;
                phi_in890 = tmp895;
                phi_in892 = tmp896;
            }
            phi_out889 = phi_in888;
            phi_out891 = phi_in890;
            phi_out893 = phi_in892;
            float tmp898 = phi_out893 + tmp866;
            float tmp899 = phi_out891 + tmp865;
            float tmp900 = phi_out889 + tmp867;
            float3 tmp901 = float3(sqrt(tmp899 * tmp899 + tmp900 * tmp900 + tmp898 * tmp898), 0.0f, 0.0f);
            float3 tmp902 = float3(tmp899, tmp898, tmp900) * tmp868 / tmp901.xxx;
            float tmp903 = tmp862 - tmp861;
            tmp904 = pow(1.0f - min(max(tmp877, 0.0f), 1.0f), 4.0f) * tmp903 + tmp861;
            tmp905 = min(max(tmp884, 0.0f), 1.0f);
            tmp906 = pow(1.0f - tmp905, 4.0f) * tmp903 + tmp861;
            float tmp907 = (pow(1.0f - min(max(abs(tmp902.x + tmp902.y + tmp902.z), 0.0f), 1.0f), 4.0f) * tmp903 + tmp861) * tmp863;
            tmp908 = tmp907 * tmp855;
            tmp909 = tmp907 * tmp856;
            tmp910 = tmp907 * tmp857;
            tmp911 = state.text_results[26].z;
            tmp912 = state.text_results[26].w;
            tmp913 = state.text_results[27].x;
            float3 tmp914 = float3(tmp911, tmp912, tmp913) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
            tmp915 = state.text_results[17].w;
            tmp916 = state.text_results[17].z;
            tmp917 = max(tmp916, 1.000000012e-07f);
            tmp918 = max(tmp915, 1.000000012e-07f);
            tmp919 = tmp914.z > 0.0f || (tmp914.x > 0.0f || tmp914.y > 0.0f);
            float3 tmp920 = state.text_results[23].yzw;
            tmp921 = tmp874.zxy;
            tmp922 = tmp874.yzx;
            tmp923 = tmp920.yzx * tmp921 - tmp920.zxy * tmp922;
            float3 tmp924 = tmp923 * tmp923;
            tmp925 = tmp924.x + tmp924.y + tmp924.z;
            if (tmp925 < 9.999999939e-09f) {
                sret_ptr.pdf = 0.0f;
                phi_in926 = 0.0f;
                phi_in928 = float3(0.0f, 0.0f, 0.0f);
                phi_in930 = 0.0f;
            } else {
                float tmp932 = 1.0f / sqrt(tmp925);
                tmp933 = float3(tmp932 * tmp923.x, tmp932 * tmp923.y, tmp932 * tmp923.z);
                tmp934 = tmp933.zxy * tmp922 - tmp933.yzx * tmp921;
                if (sret_ptr.ior1.x == -1.0f) {
                    sret_ptr.ior1.x = state.text_results[28].x;
                    sret_ptr.ior1.y = state.text_results[28].y;
                    sret_ptr.ior1.z = state.text_results[28].z;
                }
                if (sret_ptr.ior2.x == -1.0f) {
                    sret_ptr.ior2.x = state.text_results[28].x;
                    sret_ptr.ior2.y = state.text_results[28].y;
                    sret_ptr.ior2.z = state.text_results[28].z;
                }
                tmp935 = abs(tmp876);
                if (tmp887) {
                    sret_ptr.pdf = 0.0f;
                    phi_in936 = 0.0f;
                    phi_in938 = float3(0.0f, 0.0f, 0.0f);
                } else {
                    float tmp940 = tmp878 + tmp865;
                    float tmp941 = tmp879 + tmp866;
                    float tmp942 = tmp880 + tmp867;
                    float3 tmp943 = float3(sqrt(tmp941 * tmp941 + tmp940 * tmp940 + tmp942 * tmp942), 0.0f, 0.0f);
                    tmp944 = float3(tmp940, tmp941, tmp942) / tmp943.xxx;
                    float3 tmp945 = tmp874 * tmp944;
                    tmp946 = tmp945.x + tmp945.y + tmp945.z;
                    float3 tmp947 = tmp944 * tmp868;
                    float3 tmp948 = tmp944 * tmp881;
                    if (tmp948.x + tmp948.y + tmp948.z < 0.0f || (tmp947.x + tmp947.y + tmp947.z < 0.0f || tmp946 < 0.0f)) {
                        sret_ptr.pdf = 0.0f;
                        phi_in936 = 0.0f;
                        phi_in938 = float3(0.0f, 0.0f, 0.0f);
                    } else {
                        float3 tmp949 = tmp934 * tmp944;
                        float3 tmp950 = tmp933 * tmp944;
                        float tmp951 = 1.0f / tmp917;
                        float tmp952 = 1.0f / tmp918;
                        float tmp953 = (tmp949.x + tmp949.y + tmp949.z) * tmp951;
                        float tmp954 = (tmp950.x + tmp950.y + tmp950.z) * tmp952;
                        float tmp955 = tmp954 * tmp954 + tmp946 * tmp946 + tmp953 * tmp953;
                        float3 tmp956 = tmp934 * tmp868;
                        float3 tmp957 = tmp933 * tmp868;
                        float3 tmp958 = tmp934 * tmp881;
                        float3 tmp959 = tmp933 * tmp881;
                        float tmp960 = (tmp956.x + tmp956.y + tmp956.z) * tmp917;
                        float tmp961 = (tmp957.x + tmp957.y + tmp957.z) * tmp918;
                        float tmp962 = (tmp958.x + tmp958.y + tmp958.z) * tmp917;
                        float tmp963 = (tmp959.x + tmp959.y + tmp959.z) * tmp918;
                        tmp964 = tmp946 * 0.1591549367f * tmp952 * tmp951 / (tmp955 * tmp955 * (tmp935 * tmp946) * (sqrt((tmp960 * tmp960 + tmp961 * tmp961) / (tmp876 * tmp876) + 1.0f) + 1.0f));
                        float tmp965 = tmp964 * 2.0f / (sqrt((tmp962 * tmp962 + tmp963 * tmp963) / (tmp883 * tmp883) + 1.0f) + 1.0f);
                        tmp966 = float3(tmp965, tmp965, tmp965);
                        sret_ptr.pdf = tmp964;
                        phi_in936 = tmp964;
                        phi_in938 = tmp966;
                    }
                }
                phi_out937 = phi_in936;
                phi_out939 = phi_in938;
                phi_in926 = phi_out937;
                phi_in928 = phi_out939;
                phi_in930 = 0.0f;
                if (tmp919) {
                    tmp967 = min(max(sqrt(tmp916 * tmp915), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
                    tmp968 = tex_lookup_float3_3d(1, float3(min(max(acos(min(max(tmp935, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp967, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                    tmp969 = tmp968 * phi_out937;
                    sret_ptr.pdf = tmp969;
                    phi_in926 = tmp969;
                    phi_in928 = phi_out939;
                    phi_in930 = 0.0f;
                    if (!(tmp886 < 0.0f)) {
                        tmp970 = (1.0f - tmp968) * tmp884 * (1.0f - tex_lookup_float3_3d(1, float3(min(max(acos(tmp905) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp967, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x) / tex_lookup_float3_3d(1, float3(1.0f, tmp967, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                        tmp971 = 0.3183098733f - tmp968 * 0.3183098733f + tmp969;
                        sret_ptr.pdf = tmp971;
                        phi_in926 = tmp971;
                        phi_in928 = phi_out939;
                        phi_in930 = tmp970;
                    }
                }
            }
            phi_out927 = phi_in926;
            phi_out929 = phi_in928;
            phi_out931 = phi_in930;
            sret_ptr.bsdf_diffuse.x = min(max(tmp914.x, 0.0f), 1.0f) * tmp908 * phi_out931 + sret_ptr.bsdf_diffuse.x;
            sret_ptr.bsdf_diffuse.y = sret_ptr.bsdf_diffuse.y + min(max(tmp914.y, 0.0f), 1.0f) * tmp909 * phi_out931;
            sret_ptr.bsdf_diffuse.z = sret_ptr.bsdf_diffuse.z + min(max(tmp914.z, 0.0f), 1.0f) * tmp910 * phi_out931;
            tmp972 = sret_ptr.bsdf_glossy.x + min(max(tmp911, 0.0f), 1.0f) * tmp908 * phi_out929.x;
            sret_ptr.bsdf_glossy.x = tmp972;
            tmp973 = sret_ptr.bsdf_glossy.y + min(max(tmp912, 0.0f), 1.0f) * tmp909 * phi_out929.y;
            sret_ptr.bsdf_glossy.y = tmp973;
            tmp974 = sret_ptr.bsdf_glossy.z + min(max(tmp913, 0.0f), 1.0f) * tmp910 * phi_out929.z;
            sret_ptr.bsdf_glossy.z = tmp974;
            float tmp975 = 1.0f - max(tmp904, tmp906) * tmp863;
            tmp976 = tmp975 * tmp855;
            tmp977 = tmp975 * tmp856;
            tmp978 = tmp975 * tmp857;
            tmp979 = min(max(state.text_results[20].z, 0.0f), 1.0f);
            phi_in980 = 0.0f;
            if (tmp979 > 0.0f) {
                tmp982 = tmp979 * tmp976;
                tmp983 = tmp979 * tmp977;
                tmp984 = tmp979 * tmp978;
                tmp985 = state.text_results[27].y;
                tmp986 = state.text_results[27].z;
                tmp987 = state.text_results[27].w;
                float3 tmp988 = float3(tmp985, tmp986, tmp987) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
                tmp989 = state.text_results[19].z;
                tmp990 = 1.0f / max(tmp989, 1.000000012e-07f);
                tmp991 = tmp988.z > 0.0f || (tmp988.x > 0.0f || tmp988.y > 0.0f);
                tmp992 = tmp991 ? 2 : 0;
                float3 tmp993 = state.tangent_u[0];
                float3 tmp994 = tmp993.yzx * tmp921 - tmp993.zxy * tmp922;
                float3 tmp995 = tmp994 * tmp994;
                if (tmp995.x + tmp995.y + tmp995.z < 9.999999939e-09f) {
                    sret_ptr.pdf = 0.0f;
                    phi_in996 = 0.0f;
                    phi_in998 = 0.0f;
                    phi_in1000 = float3(0.0f, 0.0f, 0.0f);
                } else {
                    if (sret_ptr.ior1.x == -1.0f) {
                        sret_ptr.ior1.x = state.text_results[28].x;
                        sret_ptr.ior1.y = state.text_results[28].y;
                        sret_ptr.ior1.z = state.text_results[28].z;
                    }
                    if (sret_ptr.ior2.x == -1.0f) {
                        sret_ptr.ior2.x = state.text_results[28].x;
                        sret_ptr.ior2.y = state.text_results[28].y;
                        sret_ptr.ior2.z = state.text_results[28].z;
                    }
                    tmp1002 = abs(tmp876);
                    if (tmp887) {
                        sret_ptr.pdf = 0.0f;
                        phi_in996 = 0.0f;
                        phi_in998 = tmp1002;
                        phi_in1000 = float3(0.0f, 0.0f, 0.0f);
                    } else {
                        float tmp1003 = tmp878 + tmp865;
                        float tmp1004 = tmp879 + tmp866;
                        float tmp1005 = tmp880 + tmp867;
                        float3 tmp1006 = float3(sqrt(tmp1004 * tmp1004 + tmp1003 * tmp1003 + tmp1005 * tmp1005), 0.0f, 0.0f);
                        float3 tmp1007 = float3(tmp1003, tmp1004, tmp1005) / tmp1006.xxx;
                        float3 tmp1008 = tmp874 * tmp1007;
                        tmp1009 = tmp1008.x + tmp1008.y + tmp1008.z;
                        float3 tmp1010 = tmp1007 * tmp868;
                        tmp1011 = tmp1010.x + tmp1010.y + tmp1010.z;
                        float3 tmp1012 = tmp1007 * tmp881;
                        tmp1013 = tmp1012.x + tmp1012.y + tmp1012.z;
                        if (tmp1013 < 0.0f || (tmp1011 < 0.0f || tmp1009 < 0.0f)) {
                            sret_ptr.pdf = 0.0f;
                            phi_in996 = 0.0f;
                            phi_in998 = tmp1002;
                            phi_in1000 = float3(0.0f, 0.0f, 0.0f);
                        } else {
                            float tmp1014 = tmp1009 * 2.0f;
                            float tmp1015 = min(tmp1014 * tmp1002 / tmp1011, 1.0f);
                            float tmp1016 = (tmp990 * 0.03978873417f + 0.07957746834f) * tmp1009 * pow(sqrt(max(1.0f - tmp1009 * tmp1009, 0.0f)), tmp990) / (tmp1002 * tmp1009);
                            float tmp1017 = tmp1016 * min(tmp1015, min(tmp1014 * tmp884 / tmp1013, 1.0f));
                            tmp1018 = float3(tmp1017, tmp1017, tmp1017);
                            tmp1019 = tmp1016 * tmp1015;
                            sret_ptr.pdf = tmp1019;
                            phi_in996 = tmp1019;
                            phi_in998 = tmp1002;
                            phi_in1000 = tmp1018;
                        }
                    }
                }
                phi_out997 = phi_in996;
                phi_out999 = phi_in998;
                phi_out1001 = phi_in1000;
                sret_ptr.bsdf_glossy.x = min(max(tmp985, 0.0f), 1.0f) * tmp982 * phi_out1001.x + tmp972;
                sret_ptr.bsdf_glossy.y = min(max(tmp986, 0.0f), 1.0f) * tmp983 * phi_out1001.y + tmp973;
                sret_ptr.bsdf_glossy.z = min(max(tmp987, 0.0f), 1.0f) * tmp984 * phi_out1001.z + tmp974;
                phi_in1020 = phi_out997;
                if (tmp991) {
                    float tmp1022 = min(max(abs(tmp989), 0.0f), 1.0f) * 0.984375f + 0.0078125f;
                    float tmp1023 = tex_lookup_float3_3d(tmp992, float3(min(max(acos(min(max(phi_out999, 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp1022, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
                    float tmp1024 = 1.0f - max(tmp1023, tex_lookup_float3_3d(tmp992, float3(min(max(acos(tmp905) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, tmp1022, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x);
                    tmp8.x = min(max(tmp988.x, 0.0f), 1.0f) * tmp982 * tmp1024;
                    tmp8.y = min(max(tmp988.y, 0.0f), 1.0f) * tmp983 * tmp1024;
                    tmp8.z = min(max(tmp988.z, 0.0f), 1.0f) * tmp984 * tmp1024;
                    gen_weighted_layer_evaluate(sret_ptr, state, tmp8);
                    float tmp1025 = sret_ptr.pdf;
                    tmp1026 = (phi_out997 - tmp1025) * tmp1023 + tmp1025;
                    sret_ptr.pdf = tmp1026;
                    phi_in1020 = tmp1026;
                }
                phi_out1021 = phi_in1020;
                tmp1027 = phi_out1021 * tmp979;
                phi_in980 = tmp1027;
            }
            phi_out981 = phi_in980;
            phi_in1028 = phi_out981;
            if (tmp979 < 1.0f) {
                float tmp1030 = 1.0f - tmp979;
                tmp9.x = tmp1030 * tmp976;
                tmp9.y = tmp1030 * tmp977;
                tmp9.z = tmp1030 * tmp978;
                gen_weighted_layer_evaluate(sret_ptr, state, tmp9);
                float tmp1031 = sret_ptr.pdf;
                tmp1032 = tmp1031 * tmp1030 + phi_out981;
                phi_in1028 = tmp1032;
            }
            phi_out1029 = phi_in1028;
            tmp1033 = (tmp904 * tmp863 * (phi_out927 - phi_out1029) + phi_out1029) * tmp854 + phi_out518;
            phi_in852 = tmp1033;
        }
        phi_out853 = phi_in852;
        tmp1034 = phi_out853 * tmp512 + phi_out130;
        phi_in510 = tmp1034;
    }
    phi_out511 = phi_in510;
    sret_ptr.pdf = tmp55 * tmp12 * (phi_out72 - phi_out511) + phi_out511;
    return;
}

void mdl_surface_scattering_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    gen_custom_curve_layer_pdf(sret_ptr, state);
    return;
}

void mdl_surface_scattering_auxiliary(inout Bsdf_auxiliary_data sret_ptr, in Shading_state_material state)
{
    float tmp0;
    float tmp1;
    float tmp2;
    float tmp3;
    float tmp4;
    float tmp5;
    float3 tmp6;
    float3 tmp11;
    float tmp13;
    float tmp14;
    float tmp15;
    float tmp16;
    float tmp21;
    float3 tmp23;
    float3 tmp24;
    float tmp28;
    float tmp29;
    float tmp30;
    float3 tmp34;
    float tmp35;
    float tmp36;
    float tmp37;
    float tmp38;
    float tmp39;
    float tmp40;
    float tmp41;
    int tmp42;
    float tmp43;
    float tmp44;
    float tmp45;
    float tmp46;
    float tmp47;
    float tmp48;
    float tmp49;
    float tmp50;
    float tmp51;
    float tmp52;
    float3 tmp53;
    float tmp55;
    float tmp56;
    float tmp57;
    float tmp58;
    float tmp59;
    float phi_in;
    float phi_out;
    float phi_in60;
    float phi_out61;
    float phi_in62;
    float phi_out63;
    float tmp64;
    float tmp65;
    float tmp66;
    float tmp67;
    float phi_in68;
    float phi_out69;
    float phi_in70;
    float phi_out71;
    float phi_in72;
    float phi_out73;
    float phi_in74;
    float phi_out75;
    float phi_in76;
    float phi_out77;
    int phi_in78;
    int phi_out79;
    float phi_in80;
    float phi_out81;
    float phi_in82;
    float phi_out83;
    float phi_in84;
    float phi_out85;
    float tmp87;
    float phi_in88;
    float phi_out89;
    float phi_in90;
    float phi_out91;
    float phi_in92;
    float phi_out93;
    int phi_in94;
    int phi_out95;
    float phi_in96;
    float phi_out97;
    float phi_in98;
    float phi_out99;
    float phi_in100;
    float phi_out101;
    float tmp103;
    float tmp104;
    float tmp105;
    float tmp106;
    int tmp107;
    float rmemload183;
    float rmemload187;
    float phi_in108;
    float phi_out109;
    float phi_in110;
    float phi_out111;
    float phi_in112;
    float phi_out113;
    float phi_in114;
    float phi_out115;
    float rmemload182;
    float rmemload186;
    float rmemload181;
    float rmemload185;
    float tmp117;
    float tmp118;
    float phi_in119;
    float phi_out120;
    float phi_in121;
    float phi_out122;
    float phi_in123;
    float phi_out124;
    float phi_in125;
    float phi_out126;
    float phi_in127;
    float phi_out128;
    float phi_in129;
    float phi_out130;
    float phi_in131;
    float phi_out132;
    float phi_in133;
    float phi_out134;
    int phi_in135;
    int phi_out136;
    float phi_in137;
    float phi_out138;
    float phi_in139;
    float phi_out140;
    float phi_in141;
    float phi_out142;
    int phi_in143;
    int phi_out144;
    float phi_in145;
    float phi_out146;
    float phi_in147;
    float phi_out148;
    float phi_in149;
    float phi_out150;
    float phi_in151;
    float phi_out152;
    float phi_in153;
    float phi_out154;
    float tmp157;
    float tmp162;
    float tmp163;
    float tmp170;
    float tmp171;
    float tmp173;
    float tmp174;
    float tmp176;
    float tmp177;
    float tmp179;
    float tmp181;
    float tmp182;
    float tmp184;
    float tmp185;
    float phi_in186;
    float phi_out187;
    float tmp188;
    float tmp189;
    float phi_in190;
    float phi_out191;
    float tmp192;
    float tmp193;
    float tmp194;
    float tmp195;
    float tmp196;
    float tmp205;
    float tmp208;
    float tmp209;
    float tmp210;
    float tmp211;
    float tmp212;
    float tmp213;
    float phi_in214;
    float phi_out215;
    float phi_in216;
    float phi_out217;
    float phi_in218;
    float phi_out219;
    int phi_in220;
    int phi_out221;
    float phi_in222;
    float phi_out223;
    float phi_in224;
    float phi_out225;
    float phi_in226;
    float phi_out227;
    float phi_in228;
    float phi_out229;
    float phi_in230;
    float phi_out231;
    float phi_in232;
    float phi_out233;
    float phi_in234;
    float phi_out235;
    float phi_in236;
    float phi_out237;
    int phi_in238;
    int phi_out239;
    float phi_in240;
    float phi_out241;
    float phi_in242;
    float phi_out243;
    float phi_in244;
    float phi_out245;
    float phi_in246;
    float phi_out247;
    float phi_in248;
    float phi_out249;
    float tmp257;
    float tmp258;
    float tmp259;
    float tmp260;
    int tmp261;
    float rmemload171;
    float rmemload175;
    float phi_in262;
    float phi_out263;
    float phi_in264;
    float phi_out265;
    float phi_in266;
    float phi_out267;
    float phi_in268;
    float phi_out269;
    float rmemload170;
    float rmemload174;
    float rmemload;
    float rmemload173;
    bool tmp270;
    float phi_in271;
    float phi_out272;
    float phi_in273;
    float phi_out274;
    float phi_in275;
    float phi_out276;
    float phi_in277;
    float phi_out278;
    float phi_in279;
    float phi_out280;
    float phi_in281;
    float phi_out282;
    float phi_in283;
    float phi_out284;
    float tmp285;
    float tmp286;
    float tmp287;
    float phi_in288;
    float phi_out289;
    float phi_in290;
    float phi_out291;
    float phi_in292;
    float phi_out293;
    float tmp317;
    float tmp327;
    float tmp337;
    float tmp338;
    float tmp339;
    float tmp340;
    float tmp345;
    float tmp346;
    float tmp347;
    float tmp348;
    float tmp349;
    float tmp350;
    float tmp351;
    float tmp352;
    float tmp353;
    float tmp354;
    float tmp355;
    bool tmp356;
    bool phi_in357;
    bool phi_out358;
    float3 tmp359;
    float tmp361;
    float tmp362;
    float tmp363;
    float tmp364;
    float tmp365;
    float tmp366;
    bool tmp369;
    float tmp372;
    float tmp373;
    float tmp374;
    float tmp375;
    float tmp376;
    float tmp377;
    float tmp378;
    float tmp379;
    float3 phi_in380;
    float3 phi_out381;
    float tmp382;
    float tmp383;
    float tmp384;
    float tmp390;
    float tmp391;
    float tmp392;
    float3 phi_in393;
    float3 phi_out394;
    float tmp397;
    float tmp398;
    float tmp399;
    float tmp403;
    float tmp404;
    float tmp405;
    float tmp406;
    float tmp409;
    float tmp410;
    float tmp411;
    float tmp412;
    float tmp413;
    float tmp414;
    float tmp415;
    float tmp416;
    float tmp417;
    float tmp418;
    float3 tmp420;
    float phi_in421;
    float phi_out422;
    float tmp423;
    float tmp424;
    float tmp427;
    float tmp428;
    float tmp429;
    float tmp430;
    float tmp431;
    float tmp432;
    float tmp433;
    float3 tmp434;
    float tmp437;
    float tmp438;
    float tmp439;
    float tmp440;
    float tmp441;
    float tmp442;
    float phi_in443;
    float phi_out444;
    float phi_in445;
    float phi_out446;
    float phi_in447;
    float phi_out448;
    float tmp450;
    float tmp451;
    float tmp452;
    float tmp454;
    tmp6 = state.normal;
    sret_ptr.albedo_diffuse = float3(0.0f, 0.0f, 0.0f);
    sret_ptr.albedo_glossy = float3(0.0f, 0.0f, 0.0f);
    sret_ptr.normal = float3(0.0f, 0.0f, 0.0f);
    sret_ptr.roughness = float3(0.0f, 0.0f, 0.0f);
    float tmp7 = min(max(state.text_results[21].z, 0.0f), 1.0f);
    float tmp8 = sret_ptr.k1.x;
    float tmp9 = sret_ptr.k1.y;
    float tmp10 = sret_ptr.k1.z;
    tmp11 = float3(tmp8, tmp9, tmp10);
    float3 tmp12 = tmp11 * state.geom_normal;
    tmp13 = asfloat((asint(tmp12.x + tmp12.y + tmp12.z) & -2147483648) | 1065353216);
    tmp14 = state.text_results[25].w * tmp13;
    tmp15 = state.text_results[26].x * tmp13;
    tmp16 = state.text_results[26].y * tmp13;
    float3 tmp17 = float3(tmp14, tmp15, tmp16);
    float3 tmp18 = tmp17 * tmp11;
    float tmp19 = 1.0f - min(max(min(max(tmp18.x + tmp18.y + tmp18.z, 0.0f), 1.0f), 0.0f), 1.0f);
    float tmp20 = tmp19 * tmp19;
    tmp21 = (tmp20 * tmp20 * (tmp19 * (min(max(1.0f - state.text_results[16].w, 0.0f), 1.0f) - tmp7)) + tmp7) * min(max(state.text_results[17].x, 0.0f), 1.0f);
    float3 tmp22 = state.tangent_u[0];
    tmp23 = tmp22.yzx;
    tmp24 = tmp22.zxy;
    float3 tmp25 = tmp17.zxy * tmp23 - tmp17.yzx * tmp24;
    float3 tmp26 = tmp25 * tmp25;
    if (!(tmp26.x + tmp26.y + tmp26.z < 9.999999939e-09f)) {
        sret_ptr.albedo_glossy.x = tmp21;
        sret_ptr.albedo_glossy.y = tmp21;
        sret_ptr.albedo_glossy.z = tmp21;
        sret_ptr.normal.x = tmp21 * tmp14;
        sret_ptr.normal.y = tmp21 * tmp15;
        sret_ptr.normal.z = tmp21 * tmp16;
        sret_ptr.roughness.x = state.text_results[21].x * tmp21;
        sret_ptr.roughness.y = state.text_results[21].y * tmp21;
        sret_ptr.roughness.z = tmp21;
    }
    float tmp27 = 1.0f - tmp21;
    tmp28 = min(max(state.text_results[18].x, 0.0f), 1.0f) * tmp27;
    tmp29 = min(max(state.text_results[18].y, 0.0f), 1.0f) * tmp27;
    tmp30 = min(max(state.text_results[18].z, 0.0f), 1.0f) * tmp27;
    float tmp31 = state.text_results[25].x;
    float tmp32 = state.text_results[25].y;
    float tmp33 = state.text_results[25].z;
    tmp34 = float3(tmp31, tmp32, tmp33);
    tmp35 = min(max(state.text_results[20].y, 0.0f), 1.0f);
    tmp36 = tmp35 * tmp28;
    tmp37 = tmp35 * tmp29;
    tmp38 = tmp35 * tmp30;
    tmp39 = state.text_results[30].y;
    tmp40 = state.text_results[30].z;
    tmp41 = state.text_results[30].w;
    tmp42 = state.arg_block_offset;
    tmp43 = mdl_read_argblock_as_bool(tmp42 + 236) ? state.text_results[19].w : 0.0f;
    tmp44 = state.text_results[29].z;
    tmp45 = state.text_results[29].w;
    tmp46 = state.text_results[30].x;
    tmp3 = tmp44;
    tmp4 = tmp45;
    tmp5 = tmp46;
    tmp47 = state.text_results[28].w;
    tmp48 = state.text_results[29].x;
    tmp49 = state.text_results[29].y;
    tmp0 = tmp47;
    tmp1 = tmp48;
    tmp2 = tmp49;
    tmp50 = tmp31 * tmp13;
    tmp51 = tmp32 * tmp13;
    tmp52 = tmp33 * tmp13;
    tmp53 = float3(tmp50, tmp51, tmp52);
    float3 tmp54 = tmp53 * tmp11;
    tmp55 = tmp54.x + tmp54.y + tmp54.z;
    if (!(tmp55 < 0.0f)) {
        tmp56 = sret_ptr.ior1.x;
        if (tmp56 == -1.0f) {
            tmp57 = state.text_results[28].x;
            tmp58 = state.text_results[28].y;
            tmp59 = state.text_results[28].z;
            sret_ptr.ior1.x = tmp57;
            sret_ptr.ior1.y = tmp58;
            sret_ptr.ior1.z = tmp59;
            phi_in = tmp59;
            phi_in60 = tmp58;
            phi_in62 = tmp57;
        } else {
            tmp64 = sret_ptr.ior1.y;
            tmp65 = sret_ptr.ior1.z;
            phi_in = tmp65;
            phi_in60 = tmp64;
            phi_in62 = tmp56;
        }
        phi_out = phi_in;
        phi_out61 = phi_in60;
        phi_out63 = phi_in62;
        if (tmp43 > 0.0f) {
            tmp66 = max(1.0f - tmp55 * tmp55, 0.0f);
            tmp67 = tmp43 * 12.56637096f;
            phi_in68 = tmp41;
            phi_in70 = tmp49;
            phi_in72 = tmp46;
            phi_in74 = phi_out;
            phi_in76 = 409.375f;
            phi_in78 = 0;
            phi_in80 = 0.0f;
            phi_in82 = 0.0f;
            phi_in84 = 0.0f;
            do {
                phi_out69 = phi_in68;
                phi_out71 = phi_in70;
                phi_out73 = phi_in72;
                phi_out75 = phi_in74;
                phi_out77 = phi_in76;
                phi_out79 = phi_in78;
                phi_out81 = phi_in80;
                phi_out83 = phi_in82;
                phi_out85 = phi_in84;
                float tmp86 = phi_out75 / phi_out69;
                tmp87 = tmp86 * tmp86 * tmp66;
                if (tmp87 > 1.0f) {
                    phi_in88 = phi_out69;
                    phi_in90 = phi_out75;
                    phi_in92 = phi_out77;
                    phi_in94 = phi_out79;
                    phi_in96 = phi_out81;
                    phi_in98 = phi_out83;
                    phi_in100 = phi_out85;
                    do {
                        phi_out89 = phi_in88;
                        phi_out91 = phi_in90;
                        phi_out93 = phi_in92;
                        phi_out95 = phi_in94;
                        phi_out97 = phi_in96;
                        phi_out99 = phi_in98;
                        phi_out101 = phi_in100;
                        int tmp102 = phi_out95;
                        tmp103 = glob_cnst91[tmp102].x + phi_out101;
                        tmp104 = glob_cnst91[tmp102].y + phi_out99;
                        tmp105 = glob_cnst91[tmp102].z + phi_out97;
                        tmp106 = phi_out93 + 18.75f;
                        tmp107 = phi_out95 + 1;
                        rmemload183 = tmp3;
                        rmemload187 = tmp0;
                        phi_in108 = rmemload187;
                        phi_in110 = rmemload183;
                        phi_in112 = phi_out63;
                        phi_in114 = tmp39;
                        if (!(uint(phi_out95) > 8u)) {
                            rmemload182 = tmp5;
                            rmemload186 = tmp2;
                            phi_in108 = rmemload186;
                            phi_in110 = rmemload182;
                            phi_in112 = phi_out;
                            phi_in114 = tmp41;
                            if (uint(phi_out95) > 3u) {
                                rmemload181 = tmp4;
                                rmemload185 = tmp1;
                                phi_in108 = rmemload185;
                                phi_in110 = rmemload181;
                                phi_in112 = phi_out61;
                                phi_in114 = tmp40;
                            }
                        }
                        phi_out109 = phi_in108;
                        phi_out111 = phi_in110;
                        phi_out113 = phi_in112;
                        phi_out115 = phi_in114;
                        bool tmp116 = phi_out113 != phi_out91 || phi_out115 != phi_out89;
                        tmp117 = tmp116 ? phi_out113 : phi_out91;
                        tmp118 = tmp116 ? phi_out115 : phi_out89;
                        phi_in119 = phi_out109;
                        phi_in121 = phi_out111;
                        phi_in123 = tmp117;
                        phi_in125 = tmp118;
                        phi_in127 = tmp103;
                        phi_in129 = tmp104;
                        phi_in131 = tmp105;
                        phi_in133 = tmp106;
                        phi_in135 = tmp107;
                        phi_in88 = tmp118;
                        phi_in90 = tmp117;
                        phi_in92 = tmp106;
                        phi_in94 = tmp107;
                        phi_in96 = tmp105;
                        phi_in98 = tmp104;
                        phi_in100 = tmp103;
                        if (uint(phi_out95) > 14u || tmp116)
                            break;
                    } while (true);
                    phi_out120 = phi_in119;
                    phi_out122 = phi_in121;
                    phi_out124 = phi_in123;
                    phi_out126 = phi_in125;
                    phi_out128 = phi_in127;
                    phi_out130 = phi_in129;
                    phi_out132 = phi_in131;
                    phi_out134 = phi_in133;
                    phi_out136 = phi_in135;
                    phi_in137 = phi_out128;
                    phi_in139 = phi_out130;
                    phi_in141 = phi_out132;
                    phi_in143 = phi_out136;
                    phi_in145 = phi_out134;
                    phi_in147 = phi_out124;
                    phi_in149 = phi_out122;
                    phi_in151 = phi_out120;
                    phi_in153 = phi_out126;
                } else {
                    float tmp155 = sqrt(max(1.0f - tmp87, 0.0f));
                    float tmp156 = phi_out75 * tmp55;
                    tmp157 = tmp155 * phi_out69;
                    float tmp158 = (tmp156 - tmp157) / (tmp157 + tmp156);
                    float tmp159 = tmp155 * phi_out75;
                    float tmp160 = phi_out69 * tmp55;
                    float tmp161 = (tmp160 - tmp159) / (tmp159 + tmp160);
                    tmp162 = tmp158 * tmp158;
                    tmp163 = tmp161 * tmp161;
                    float tmp164 = phi_out73 * phi_out73;
                    float tmp165 = phi_out71 * phi_out71;
                    float tmp166 = phi_out69 * phi_out69;
                    float tmp167 = tmp165 - tmp164;
                    float tmp168 = (tmp167 - tmp87 * tmp166) * 0.5f;
                    float tmp169 = sqrt(max(tmp168 * tmp168 + tmp164 * tmp165, 0.0f));
                    tmp170 = tmp169 - tmp168;
                    tmp171 = sqrt(max(tmp169 + tmp168, 0.0f));
                    float tmp172 = sqrt(max(tmp170, 0.0f));
                    tmp173 = tmp155 * tmp167;
                    tmp174 = tmp171 * phi_out69;
                    float tmp175 = phi_out71 * 2.0f * phi_out73;
                    tmp176 = tmp155 * tmp175;
                    tmp177 = tmp172 * phi_out69;
                    float tmp178 = tmp157 * 2.0f;
                    tmp179 = tmp172 * tmp178;
                    float tmp180 = tmp169 * 2.0f;
                    tmp181 = tmp180 - tmp157 * tmp157;
                    tmp182 = (tmp171 * tmp175 - tmp172 * tmp167) * tmp178;
                    float tmp183 = tmp155 * (tmp164 + tmp165);
                    tmp184 = tmp183 * tmp183 - tmp180 * tmp166;
                    tmp185 = tmp179 * tmp179 + tmp181 * tmp181;
                    phi_in186 = 0.0f;
                    if (tmp185 > 0.0f) {
                        tmp188 = 1.0f / sqrt(tmp185);
                        phi_in186 = tmp188;
                    }
                    phi_out187 = phi_in186;
                    tmp189 = tmp182 * tmp182 + tmp184 * tmp184;
                    phi_in190 = 0.0f;
                    if (tmp189 > 0.0f) {
                        tmp192 = 1.0f / sqrt(tmp189);
                        phi_in190 = tmp192;
                    }
                    phi_out191 = phi_in190;
                    tmp193 = phi_out187 * tmp181;
                    tmp194 = phi_out191 * tmp184;
                    tmp195 = phi_out187 * tmp179;
                    tmp196 = phi_out191 * tmp182;
                    float tmp197 = tmp157 - tmp171;
                    float tmp198 = tmp171 + tmp157;
                    float tmp199 = (tmp197 * tmp197 + tmp170) / (tmp198 * tmp198 + tmp170);
                    float tmp200 = tmp173 - tmp174;
                    float tmp201 = tmp176 - tmp177;
                    float tmp202 = tmp174 + tmp173;
                    float tmp203 = tmp177 + tmp176;
                    float tmp204 = (tmp201 * tmp201 + tmp200 * tmp200) / (tmp203 * tmp203 + tmp202 * tmp202);
                    tmp205 = tmp67 * tmp157;
                    float tmp206 = max(tmp199 * tmp162, 0.0f);
                    float tmp207 = max(tmp204 * tmp163, 0.0f);
                    tmp208 = sqrt(tmp206) * 2.0f;
                    tmp209 = tmp199 + tmp162;
                    tmp210 = tmp206 + 1.0f;
                    tmp211 = sqrt(tmp207) * 2.0f;
                    tmp212 = tmp204 + tmp163;
                    tmp213 = tmp207 + 1.0f;
                    phi_in214 = phi_out85;
                    phi_in216 = phi_out83;
                    phi_in218 = phi_out81;
                    phi_in220 = phi_out79;
                    phi_in222 = phi_out77;
                    phi_in224 = phi_out75;
                    phi_in226 = phi_out73;
                    phi_in228 = phi_out71;
                    phi_in230 = phi_out69;
                    do {
                        phi_out215 = phi_in214;
                        phi_out217 = phi_in216;
                        phi_out219 = phi_in218;
                        phi_out221 = phi_in220;
                        phi_out223 = phi_in222;
                        phi_out225 = phi_in224;
                        phi_out227 = phi_in226;
                        phi_out229 = phi_in228;
                        phi_out231 = phi_in230;
                        phi_in232 = phi_out215;
                        phi_in234 = phi_out217;
                        phi_in236 = phi_out219;
                        phi_in238 = 16;
                        phi_in240 = phi_out223;
                        phi_in242 = phi_out225;
                        phi_in244 = phi_out227;
                        phi_in246 = phi_out229;
                        phi_in248 = phi_out231;
                        if (phi_out221 == 16)
                            break;
                        else {
                            float tmp250 = tmp205 / phi_out223;
                            float tmp251 = sin(tmp250);
                            float tmp252 = cos(tmp250);
                            float tmp253 = tmp208 * (tmp193 * tmp252 - tmp195 * tmp251);
                            float tmp254 = tmp211 * (tmp194 * tmp252 - tmp196 * tmp251);
                            int tmp255 = phi_out221;
                            float tmp256 = ((tmp209 + tmp253) / (tmp210 + tmp253) + (tmp212 + tmp254) / (tmp213 + tmp254)) * 0.5f;
                            tmp257 = glob_cnst91[tmp255].x * tmp256 + phi_out215;
                            tmp258 = glob_cnst91[tmp255].y * tmp256 + phi_out217;
                            tmp259 = glob_cnst91[tmp255].z * tmp256 + phi_out219;
                            tmp260 = phi_out223 + 18.75f;
                            tmp261 = phi_out221 + 1;
                            rmemload171 = tmp0;
                            rmemload175 = tmp3;
                            phi_in262 = rmemload175;
                            phi_in264 = rmemload171;
                            phi_in266 = tmp39;
                            phi_in268 = phi_out63;
                            if (!(uint(phi_out221) > 8u)) {
                                rmemload170 = tmp2;
                                rmemload174 = tmp5;
                                phi_in262 = rmemload174;
                                phi_in264 = rmemload170;
                                phi_in266 = tmp41;
                                phi_in268 = phi_out;
                                if (uint(phi_out221) > 3u) {
                                    rmemload = tmp1;
                                    rmemload173 = tmp4;
                                    phi_in262 = rmemload173;
                                    phi_in264 = rmemload;
                                    phi_in266 = tmp40;
                                    phi_in268 = phi_out61;
                                }
                            }
                            phi_out263 = phi_in262;
                            phi_out265 = phi_in264;
                            phi_out267 = phi_in266;
                            phi_out269 = phi_in268;
                            tmp270 = phi_out269 != phi_out225 || (phi_out263 != phi_out227 || (phi_out267 != phi_out231 || phi_out265 != phi_out229));
                            phi_in271 = phi_out269;
                            phi_in273 = phi_out263;
                            phi_in275 = phi_out265;
                            phi_in277 = phi_out267;
                            if (!tmp270) {
                                phi_in271 = phi_out225;
                                phi_in273 = phi_out227;
                                phi_in275 = phi_out229;
                                phi_in277 = phi_out231;
                            }
                            phi_out272 = phi_in271;
                            phi_out274 = phi_in273;
                            phi_out276 = phi_in275;
                            phi_out278 = phi_in277;
                            phi_in232 = tmp257;
                            phi_in234 = tmp258;
                            phi_in236 = tmp259;
                            phi_in238 = tmp261;
                            phi_in240 = tmp260;
                            phi_in242 = phi_out272;
                            phi_in244 = phi_out274;
                            phi_in246 = phi_out276;
                            phi_in248 = phi_out278;
                            phi_in214 = tmp257;
                            phi_in216 = tmp258;
                            phi_in218 = tmp259;
                            phi_in220 = tmp261;
                            phi_in222 = tmp260;
                            phi_in224 = phi_out272;
                            phi_in226 = phi_out274;
                            phi_in228 = phi_out276;
                            phi_in230 = phi_out278;
                            if (tmp270)
                                break;
                        }
                    } while (true);
                    phi_out233 = phi_in232;
                    phi_out235 = phi_in234;
                    phi_out237 = phi_in236;
                    phi_out239 = phi_in238;
                    phi_out241 = phi_in240;
                    phi_out243 = phi_in242;
                    phi_out245 = phi_in244;
                    phi_out247 = phi_in246;
                    phi_out249 = phi_in248;
                    phi_in137 = phi_out233;
                    phi_in139 = phi_out235;
                    phi_in141 = phi_out237;
                    phi_in143 = phi_out239;
                    phi_in145 = phi_out241;
                    phi_in147 = phi_out243;
                    phi_in149 = phi_out245;
                    phi_in151 = phi_out247;
                    phi_in153 = phi_out249;
                }
                phi_out138 = phi_in137;
                phi_out140 = phi_in139;
                phi_out142 = phi_in141;
                phi_out144 = phi_in143;
                phi_out146 = phi_in145;
                phi_out148 = phi_in147;
                phi_out150 = phi_in149;
                phi_out152 = phi_in151;
                phi_out154 = phi_in153;
                phi_in68 = phi_out154;
                phi_in70 = phi_out152;
                phi_in72 = phi_out150;
                phi_in74 = phi_out148;
                phi_in76 = phi_out146;
                phi_in78 = phi_out144;
                phi_in80 = phi_out142;
                phi_in82 = phi_out140;
                phi_in84 = phi_out138;
                phi_in279 = phi_out138;
                phi_in281 = phi_out140;
                phi_in283 = phi_out142;
                if (!(uint(phi_out144) < 16u))
                    break;
            } while (true);
            phi_out280 = phi_in279;
            phi_out282 = phi_in281;
            phi_out284 = phi_in283;
            tmp285 = phi_out282 * -0.0389967896f + phi_out280 * 0.01064765267f + phi_out284 * 0.2020568848f;
            tmp286 = min(max(phi_out282 * -0.2216216922f + phi_out280 * 0.467204839f + phi_out284 * -0.07188431919f, 0.0f), 1.0f);
            tmp287 = min(max(phi_out282 * 0.3432191908f + phi_out280 * -0.1772817373f + phi_out284 * 0.007593344897f, 0.0f), 1.0f);
            phi_in288 = tmp285;
            phi_in290 = tmp286;
            phi_in292 = tmp287;
        } else {
            float tmp294 = 1.0f / phi_out63;
            float tmp295 = 1.0f / phi_out61;
            float tmp296 = 1.0f / phi_out;
            float tmp297 = tmp294 * tmp47;
            float tmp298 = tmp295 * tmp48;
            float tmp299 = tmp296 * tmp49;
            float tmp300 = tmp294 * tmp44;
            float tmp301 = tmp295 * tmp45;
            float tmp302 = tmp296 * tmp46;
            float tmp303 = tmp55 * tmp55;
            float tmp304 = 1.0f - tmp303;
            float tmp305 = -tmp304;
            float tmp306 = tmp297 * tmp297;
            float tmp307 = tmp300 * tmp300;
            float tmp308 = tmp305 - tmp307 + tmp306;
            float tmp309 = sqrt(tmp308 * tmp308 + tmp306 * 4.0f * tmp307);
            float tmp310 = tmp309 + tmp303;
            float tmp311 = tmp55 * 2.0f;
            float tmp312 = sqrt(max((tmp309 + tmp308) * 0.5f, 0.0f)) * tmp311;
            float tmp313 = (tmp310 - tmp312) / (tmp312 + tmp310);
            float tmp314 = tmp304 * tmp304;
            float tmp315 = tmp309 * tmp303 + tmp314;
            float tmp316 = tmp312 * tmp304;
            tmp317 = min(max((tmp313 * (tmp315 - tmp316) / (tmp316 + tmp315) + tmp313) * 0.5f, 0.0f), 1.0f);
            float tmp318 = tmp298 * tmp298;
            float tmp319 = tmp301 * tmp301;
            float tmp320 = tmp305 - tmp319 + tmp318;
            float tmp321 = sqrt(tmp320 * tmp320 + tmp318 * 4.0f * tmp319);
            float tmp322 = tmp321 + tmp303;
            float tmp323 = sqrt(max((tmp321 + tmp320) * 0.5f, 0.0f)) * tmp311;
            float tmp324 = (tmp322 - tmp323) / (tmp323 + tmp322);
            float tmp325 = tmp321 * tmp303 + tmp314;
            float tmp326 = tmp323 * tmp304;
            tmp327 = min(max((tmp324 * (tmp325 - tmp326) / (tmp326 + tmp325) + tmp324) * 0.5f, 0.0f), 1.0f);
            float tmp328 = tmp299 * tmp299;
            float tmp329 = tmp302 * tmp302;
            float tmp330 = tmp305 - tmp329 + tmp328;
            float tmp331 = sqrt(tmp330 * tmp330 + tmp328 * 4.0f * tmp329);
            float tmp332 = tmp331 + tmp303;
            float tmp333 = sqrt(max((tmp331 + tmp330) * 0.5f, 0.0f)) * tmp311;
            float tmp334 = (tmp332 - tmp333) / (tmp333 + tmp332);
            float tmp335 = tmp331 * tmp303 + tmp314;
            float tmp336 = tmp333 * tmp304;
            tmp337 = (tmp334 * (tmp335 - tmp336) / (tmp336 + tmp335) + tmp334) * 0.5f;
            phi_in288 = tmp337;
            phi_in290 = tmp317;
            phi_in292 = tmp327;
        }
        phi_out289 = phi_in288;
        phi_out291 = phi_in290;
        phi_out293 = phi_in292;
        tmp338 = tmp36 * phi_out291;
        tmp339 = tmp37 * phi_out293;
        tmp340 = tmp38 * min(max(phi_out289, 0.0f), 1.0f);
        float3 tmp341 = tmp53.zxy * tmp23 - tmp53.yzx * tmp24;
        float3 tmp342 = tmp341 * tmp341;
        if (!(tmp342.x + tmp342.y + tmp342.z < 9.999999939e-09f)) {
            float tmp343 = tmp339 * 0.7151600122f + tmp338 * 0.2126709968f + tmp340 * 0.07216899842f;
            sret_ptr.albedo_glossy.x = sret_ptr.albedo_glossy.x + tmp338;
            sret_ptr.albedo_glossy.y = sret_ptr.albedo_glossy.y + tmp339;
            sret_ptr.albedo_glossy.z = sret_ptr.albedo_glossy.z + tmp340;
            sret_ptr.normal.x = sret_ptr.normal.x + tmp343 * tmp50;
            sret_ptr.normal.y = sret_ptr.normal.y + tmp343 * tmp51;
            sret_ptr.normal.z = sret_ptr.normal.z + tmp343 * tmp52;
            sret_ptr.roughness.x = sret_ptr.roughness.x + state.text_results[17].z * tmp343;
            sret_ptr.roughness.y = sret_ptr.roughness.y + state.text_results[17].w * tmp343;
            sret_ptr.roughness.z = sret_ptr.roughness.z + tmp343;
        }
    }
    float tmp344 = 1.0f - tmp35;
    tmp345 = tmp344 * tmp28;
    tmp346 = tmp344 * tmp29;
    tmp347 = tmp344 * tmp30;
    tmp348 = min(max(state.text_results[12].x, 0.0f), 1.0f);
    tmp349 = tmp348 * tmp345;
    tmp350 = tmp348 * tmp346;
    tmp351 = tmp348 * tmp347;
    tmp352 = min(max(state.text_results[20].x, 0.0f), 1.0f);
    tmp353 = tmp352 * tmp349;
    tmp354 = tmp352 * tmp350;
    tmp355 = tmp352 * tmp351;
    tmp356 = mdl_read_argblock_as_bool(tmp42);
    phi_in357 = true;
    if (!tmp356) {
        phi_in357 = true;
        if (!(state.text_results[12].y == 0.0f))
            phi_in357 = false;
    }
    phi_out358 = phi_in357;
    tmp359 = float3(state.text_results[12].z, state.text_results[12].w, state.text_results[13].x);
    float3 tmp360 = phi_out358 ? tmp359 : float3(1.0f, 1.0f, 1.0f);
    tmp361 = state.text_results[26].z;
    tmp362 = state.text_results[26].w;
    tmp363 = state.text_results[27].x;
    tmp364 = tmp353 * min(max((tmp360.x + tmp361) * 0.5f, 0.0f), 1.0f);
    tmp365 = tmp354 * min(max((tmp360.y + tmp362) * 0.5f, 0.0f), 1.0f);
    tmp366 = tmp355 * min(max((tmp360.z + tmp363) * 0.5f, 0.0f), 1.0f);
    float3 tmp367 = tmp53.zxy * tmp23 - tmp53.yzx * tmp24;
    float3 tmp368 = tmp367 * tmp367;
    tmp369 = tmp368.x + tmp368.y + tmp368.z < 9.999999939e-09f;
    if (!tmp369) {
        float tmp370 = tmp365 * 0.7151600122f + tmp364 * 0.2126709968f + tmp366 * 0.07216899842f;
        sret_ptr.albedo_glossy.x = sret_ptr.albedo_glossy.x + tmp364;
        sret_ptr.albedo_glossy.y = sret_ptr.albedo_glossy.y + tmp365;
        sret_ptr.albedo_glossy.z = sret_ptr.albedo_glossy.z + tmp366;
        sret_ptr.normal.x = sret_ptr.normal.x + tmp370 * tmp50;
        sret_ptr.normal.y = sret_ptr.normal.y + tmp370 * tmp51;
        sret_ptr.normal.z = sret_ptr.normal.z + tmp370 * tmp52;
        sret_ptr.roughness.x = sret_ptr.roughness.x + state.text_results[17].z * tmp370;
        sret_ptr.roughness.y = sret_ptr.roughness.y + state.text_results[17].w * tmp370;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp370;
    }
    float tmp371 = 1.0f - tmp352;
    tmp372 = tmp371 * tmp349;
    tmp373 = tmp371 * tmp350;
    tmp374 = tmp371 * tmp351;
    tmp375 = state.text_results[17].y;
    tmp376 = min(max(tmp375 > 0.349999994f ? tmp375 + -0.349999994f : 0.0f, 0.0f), 1.0f);
    tmp377 = tmp376 * tmp372;
    tmp378 = tmp376 * tmp373;
    tmp379 = tmp376 * tmp374;
    phi_in380 = tmp359;
    if (!tmp356) {
        phi_in380 = tmp359;
        if (!(state.text_results[12].y == 0.0f))
            phi_in380 = float3(1.0f, 1.0f, 1.0f);
    }
    phi_out381 = phi_in380;
    tmp382 = tmp6.x * tmp13;
    tmp383 = tmp6.y * tmp13;
    tmp384 = tmp6.z * tmp13;
    float3 tmp385 = float3(tmp382, tmp383, tmp384);
    float3 tmp386 = tmp385.zxy * tmp23 - tmp385.yzx * tmp24;
    float3 tmp387 = tmp386 * tmp386;
    if (!(tmp387.x + tmp387.y + tmp387.z < 9.999999939e-09f)) {
        float tmp388 = tmp378 * 0.7151600122f + tmp377 * 0.2126709968f + tmp379 * 0.07216899842f;
        sret_ptr.albedo_diffuse.x = min(max(phi_out381.x, 0.0f), 1.0f) * tmp377;
        sret_ptr.albedo_diffuse.y = min(max(phi_out381.y, 0.0f), 1.0f) * tmp378;
        sret_ptr.albedo_diffuse.z = min(max(phi_out381.z, 0.0f), 1.0f) * tmp379;
        sret_ptr.normal.x = sret_ptr.normal.x + tmp388 * tmp382;
        sret_ptr.normal.y = sret_ptr.normal.y + tmp388 * tmp383;
        sret_ptr.normal.z = sret_ptr.normal.z + tmp388 * tmp384;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp388;
    }
    float tmp389 = 1.0f - tmp376;
    tmp390 = tmp389 * tmp372;
    tmp391 = tmp389 * tmp373;
    tmp392 = tmp389 * tmp374;
    phi_in393 = tmp359;
    if (!tmp356) {
        phi_in393 = tmp359;
        if (!(state.text_results[12].y == 0.0f))
            phi_in393 = float3(1.0f, 1.0f, 1.0f);
    }
    phi_out394 = phi_in393;
    if (!tmp369) {
        float tmp395 = tmp391 * 0.7151600122f + tmp390 * 0.2126709968f + tmp392 * 0.07216899842f;
        sret_ptr.albedo_glossy.x = sret_ptr.albedo_glossy.x + min(max(phi_out394.x, 0.0f), 1.0f) * tmp390;
        sret_ptr.albedo_glossy.y = sret_ptr.albedo_glossy.y + min(max(phi_out394.y, 0.0f), 1.0f) * tmp391;
        sret_ptr.albedo_glossy.z = sret_ptr.albedo_glossy.z + min(max(phi_out394.z, 0.0f), 1.0f) * tmp392;
        sret_ptr.normal.x = sret_ptr.normal.x + tmp395 * tmp50;
        sret_ptr.normal.y = sret_ptr.normal.y + tmp395 * tmp51;
        sret_ptr.normal.z = sret_ptr.normal.z + tmp395 * tmp52;
        sret_ptr.roughness.x = sret_ptr.roughness.x + state.text_results[17].z * tmp395;
        sret_ptr.roughness.y = sret_ptr.roughness.y + state.text_results[17].w * tmp395;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp395;
    }
    float tmp396 = 1.0f - tmp348;
    tmp397 = tmp396 * tmp345;
    tmp398 = tmp396 * tmp346;
    tmp399 = tmp396 * tmp347;
    float tmp400 = state.text_results[24].w;
    float tmp401 = (tmp400 + -1.0f) / (tmp400 + 1.0f);
    float tmp402 = min(max(tmp401 * tmp401, 0.0f), 1.0f);
    tmp403 = ((min(max(1.0f - tmp375 * tmp375, 0.0f), 1.0f) - tmp402) * pow(1.0f - min(max(min(max(tmp55, 0.0f), 1.0f), 0.0f), 1.0f), 4.0f) + tmp402) * tmp352;
    tmp404 = tmp403 * tmp397;
    tmp405 = tmp403 * tmp398;
    tmp406 = tmp403 * tmp399;
    if (!tmp369) {
        float tmp407 = tmp405 * 0.7151600122f + tmp404 * 0.2126709968f + tmp406 * 0.07216899842f;
        sret_ptr.albedo_glossy.x = sret_ptr.albedo_glossy.x + tmp404 * min(max(tmp361, 0.0f), 1.0f);
        sret_ptr.albedo_glossy.y = sret_ptr.albedo_glossy.y + tmp405 * min(max(tmp362, 0.0f), 1.0f);
        sret_ptr.albedo_glossy.z = sret_ptr.albedo_glossy.z + tmp406 * min(max(tmp363, 0.0f), 1.0f);
        sret_ptr.normal.x = sret_ptr.normal.x + tmp407 * tmp50;
        sret_ptr.normal.y = sret_ptr.normal.y + tmp407 * tmp51;
        sret_ptr.normal.z = sret_ptr.normal.z + tmp407 * tmp52;
        sret_ptr.roughness.x = sret_ptr.roughness.x + state.text_results[17].z * tmp407;
        sret_ptr.roughness.y = sret_ptr.roughness.y + state.text_results[17].w * tmp407;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp407;
    }
    float tmp408 = 1.0f - tmp403;
    tmp409 = tmp408 * tmp397;
    tmp410 = tmp408 * tmp398;
    tmp411 = tmp408 * tmp399;
    tmp412 = min(max(state.text_results[20].z, 0.0f), 1.0f);
    tmp413 = tmp412 * tmp409;
    tmp414 = tmp412 * tmp410;
    tmp415 = tmp412 * tmp411;
    tmp416 = state.text_results[27].y;
    tmp417 = state.text_results[27].z;
    tmp418 = state.text_results[27].w;
    float3 tmp419 = float3(tmp416, tmp417, tmp418) * float3(0.9200000167f, 0.9200000167f, 0.9200000167f);
    tmp420 = tmp34 * tmp11;
    phi_in421 = 1.0f;
    if (tmp419.z > 0.0f || (tmp419.x > 0.0f || tmp419.y > 0.0f)) {
        tmp423 = tex_lookup_float3_3d(2, float3(min(max(acos(min(max(min(max(tmp420.x + tmp420.y + tmp420.z, 0.0f), 1.0f), 0.0f), 1.0f)) * 0.6366197467f, 0.0f), 1.0f) * 0.9692307711f + 0.007692307699f, min(max(abs(state.text_results[19].z), 0.0f), 1.0f) * 0.984375f + 0.0078125f, 0.0f), 0, 0, 0, float2(0.0f, 1.0f), float2(0.0f, 1.0f), float2(0.0f, 1.0f), 0.0f).x;
        phi_in421 = tmp423;
    }
    phi_out422 = phi_in421;
    tmp424 = 1.0f - phi_out422;
    if (!tmp369) {
        float tmp425 = tmp414 * 0.7151600122f + tmp413 * 0.2126709968f + tmp415 * 0.07216899842f;
        sret_ptr.albedo_diffuse.x = sret_ptr.albedo_diffuse.x + min(max(tmp424 * tmp419.x, 0.0f), 1.0f) * tmp413;
        sret_ptr.albedo_diffuse.y = sret_ptr.albedo_diffuse.y + min(max(tmp424 * tmp419.y, 0.0f), 1.0f) * tmp414;
        sret_ptr.albedo_diffuse.z = sret_ptr.albedo_diffuse.z + min(max(tmp424 * tmp419.z, 0.0f), 1.0f) * tmp415;
        sret_ptr.albedo_glossy.x = sret_ptr.albedo_glossy.x + min(max(phi_out422 * tmp416, 0.0f), 1.0f) * tmp413;
        sret_ptr.albedo_glossy.y = sret_ptr.albedo_glossy.y + min(max(phi_out422 * tmp417, 0.0f), 1.0f) * tmp414;
        sret_ptr.albedo_glossy.z = sret_ptr.albedo_glossy.z + min(max(phi_out422 * tmp418, 0.0f), 1.0f) * tmp415;
        sret_ptr.normal.x = sret_ptr.normal.x + tmp425 * tmp50;
        sret_ptr.normal.y = sret_ptr.normal.y + tmp425 * tmp51;
        sret_ptr.normal.z = sret_ptr.normal.z + tmp425 * tmp52;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp425;
    }
    float tmp426 = 1.0f - tmp412;
    tmp427 = tmp426 * tmp409;
    tmp428 = tmp426 * tmp410;
    tmp429 = tmp426 * tmp411;
    tmp430 = min(max(state.text_results[13].y, 0.0f), 1.0f);
    tmp431 = tmp430 * tmp427;
    tmp432 = tmp430 * tmp428;
    tmp433 = tmp430 * tmp429;
    tmp434 = tmp356 ? state.text_results[14].xyz : float3(1.0f, 1.0f, 1.0f);
    if (!tmp369) {
        float tmp435 = tmp432 * 0.7151600122f + tmp431 * 0.2126709968f + tmp433 * 0.07216899842f;
        sret_ptr.albedo_diffuse.x = sret_ptr.albedo_diffuse.x + min(max(tmp434.x, 0.0f), 1.0f) * tmp431;
        sret_ptr.albedo_diffuse.y = sret_ptr.albedo_diffuse.y + min(max(tmp434.y, 0.0f), 1.0f) * tmp432;
        sret_ptr.albedo_diffuse.z = sret_ptr.albedo_diffuse.z + min(max(tmp434.z, 0.0f), 1.0f) * tmp433;
        sret_ptr.normal.x = sret_ptr.normal.x + tmp435 * tmp50;
        sret_ptr.normal.y = sret_ptr.normal.y + tmp435 * tmp51;
        sret_ptr.normal.z = sret_ptr.normal.z + tmp435 * tmp52;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp435;
    }
    float tmp436 = 1.0f - tmp430;
    tmp437 = tmp436 * tmp427;
    tmp438 = tmp436 * tmp428;
    tmp439 = tmp436 * tmp429;
    if (tmp369) {
        tmp440 = sret_ptr.normal.x;
        tmp441 = sret_ptr.normal.y;
        tmp442 = sret_ptr.normal.z;
        phi_in443 = tmp442;
        phi_in445 = tmp441;
        phi_in447 = tmp440;
    } else {
        float tmp449 = tmp438 * 0.7151600122f + tmp437 * 0.2126709968f + tmp439 * 0.07216899842f;
        sret_ptr.albedo_diffuse.x = min(max(state.text_results[24].x, 0.0f), 1.0f) * tmp437 + sret_ptr.albedo_diffuse.x;
        sret_ptr.albedo_diffuse.y = sret_ptr.albedo_diffuse.y + min(max(state.text_results[24].y, 0.0f), 1.0f) * tmp438;
        sret_ptr.albedo_diffuse.z = sret_ptr.albedo_diffuse.z + min(max(state.text_results[24].z, 0.0f), 1.0f) * tmp439;
        tmp450 = sret_ptr.normal.x + tmp449 * tmp50;
        sret_ptr.normal.x = tmp450;
        tmp451 = sret_ptr.normal.y + tmp449 * tmp51;
        sret_ptr.normal.y = tmp451;
        tmp452 = sret_ptr.normal.z + tmp449 * tmp52;
        sret_ptr.normal.z = tmp452;
        sret_ptr.roughness.z = sret_ptr.roughness.z + tmp449;
        phi_in443 = tmp452;
        phi_in445 = tmp451;
        phi_in447 = tmp450;
    }
    phi_out444 = phi_in443;
    phi_out446 = phi_in445;
    phi_out448 = phi_in447;
    if (phi_out448 != 0.0f || (phi_out446 != 0.0f || phi_out444 != 0.0f)) {
        float3 tmp453 = float3(sqrt(phi_out446 * phi_out446 + phi_out448 * phi_out448 + phi_out444 * phi_out444), 0.0f, 0.0f);
        sret_ptr.normal = float3(phi_out448, phi_out446, phi_out444) / tmp453.xxx;
    }
    tmp454 = sret_ptr.roughness.z;
    if (!(tmp454 == 0.0f)) {
        sret_ptr.roughness.x = sret_ptr.roughness.x / tmp454;
        sret_ptr.roughness.y = sret_ptr.roughness.y / tmp454;
    }
    return;
}

void mdl_surface_emission_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0;
    float tmp1;
    float tmp2;
    float tmp3;
    float3 tmp4;
    float tmp5;
    float tmp6;
    float3 phi_in;
    float3 phi_out;
    float tmp9;
    float tmp10;
    float tmp11;
    float tmp12;
    float tmp13;
    float phi_in14;
    float phi_out15;
    float phi_in16;
    float phi_out17;
    float phi_in18;
    float phi_out19;
    float tmp20;
    float tmp21;
    float3 tmp22;
    float tmp23;
    float3 tmp25;
    float3 tmp26;
    float tmp29;
    bool tmp30;
    float phi_in31;
    float phi_out32;
    float phi_in33;
    float phi_out34;
    float phi_in35;
    float phi_out36;
    float phi_in37;
    float phi_out38;
    float phi_in39;
    float phi_out40;
    float phi_in41;
    float phi_out42;
    float tmp44;
    float tmp45;
    float tmp46;
    float phi_in49;
    float phi_out50;
    float phi_in51;
    float phi_out52;
    float phi_in53;
    float phi_out54;
    float phi_in55;
    float phi_out56;
    float3 tmp63;
    int phi_in64;
    int phi_out65;
    float phi_in66;
    float phi_out67;
    float phi_in68;
    float phi_out69;
    float phi_in70;
    float phi_out71;
    float phi_in72;
    float phi_out73;
    float tmp76;
    float phi_in77;
    float phi_out78;
    float phi_in79;
    float phi_out80;
    float phi_in81;
    float phi_out82;
    tmp0 = state.normal;
    tmp1 = max(1.0f - state.text_results[16].w, 0.0f);
    tmp2 = state.text_results[17].x;
    tmp3 = state.text_results[21].z;
    tmp4 = state.geom_normal;
    tmp5 = sret_ptr.xi.x;
    tmp6 = sret_ptr.xi.y;
    phi_in = float3(0.0f, 1.0f, 0.0f);
    if (!(tmp5 == 0.0f && tmp6 == 0.0f)) {
        float tmp7 = tmp5 * 2.0f;
        float tmp8 = tmp6 * 2.0f;
        tmp9 = tmp7 < 1.0f ? tmp7 : tmp7 + -2.0f;
        tmp10 = tmp8 < 1.0f ? tmp8 : tmp8 + -2.0f;
        tmp11 = tmp9 * tmp9;
        tmp12 = tmp10 * tmp10;
        if (tmp11 > tmp12) {
            tmp13 = tmp10 * -0.7853981853f / tmp9;
            phi_in14 = tmp9;
            phi_in16 = tmp13;
            phi_in18 = tmp11;
        } else {
            tmp20 = tmp9 * 0.7853981853f / tmp10 + -1.570796371f;
            phi_in14 = tmp10;
            phi_in16 = tmp20;
            phi_in18 = tmp12;
        }
        phi_out15 = phi_in14;
        phi_out17 = phi_in16;
        phi_out19 = phi_in18;
        tmp21 = 1.0f - phi_out19;
        phi_in = float3(0.0f, 1.0f, 0.0f);
        if (tmp21 > 0.0f) {
            tmp22 = float3(sin(phi_out17) * phi_out15, sqrt(tmp21), cos(phi_out17) * phi_out15);
            phi_in = tmp22;
        }
    }
    phi_out = phi_in;
    tmp23 = phi_out.y;
    float3 tmp24 = state.tangent_u[0];
    tmp25 = tmp0.zxy;
    tmp26 = tmp0.yzx;
    float3 tmp27 = tmp24.yzx * tmp25 - tmp24.zxy * tmp26;
    float3 tmp28 = tmp27 * tmp27;
    tmp29 = tmp28.x + tmp28.y + tmp28.z;
    tmp30 = tmp29 < 9.999999939e-09f;
    phi_in31 = tmp27.x;
    phi_in33 = tmp27.y;
    phi_in35 = tmp27.z;
    phi_in37 = 0.0f;
    phi_in39 = 0.0f;
    phi_in41 = 0.0f;
    if (!tmp30) {
        float tmp43 = 1.0f / sqrt(tmp29);
        tmp44 = tmp43 * tmp27.x;
        tmp45 = tmp43 * tmp27.y;
        tmp46 = tmp43 * tmp27.z;
        float3 tmp47 = float3(tmp44, tmp45, tmp46);
        float3 tmp48 = tmp47.zxy * tmp26 - tmp47.yzx * tmp25;
        phi_in31 = tmp44;
        phi_in33 = tmp45;
        phi_in35 = tmp46;
        phi_in37 = tmp48.x;
        phi_in39 = tmp48.y;
        phi_in41 = tmp48.z;
    }
    phi_out32 = phi_in31;
    phi_out34 = phi_in33;
    phi_out36 = phi_in35;
    phi_out38 = phi_in37;
    phi_out40 = phi_in39;
    phi_out42 = phi_in41;
    if (tmp30) {
        sret_ptr.k1 = float3(0.0f, 0.0f, 0.0f);
        sret_ptr.pdf = 0.0f;
        sret_ptr.edf_over_pdf = float3(0.0f, 0.0f, 0.0f);
        sret_ptr.event_type = 0;
        phi_in49 = 0.0f;
        phi_in51 = 0.0f;
        phi_in53 = 0.0f;
        phi_in55 = 0.0f;
    } else {
        float tmp57 = phi_out.z;
        float tmp58 = phi_out.x;
        float tmp59 = phi_out32 * tmp57 + tmp23 * tmp0.x + phi_out38 * tmp58;
        float tmp60 = phi_out34 * tmp57 + tmp23 * tmp0.y + phi_out40 * tmp58;
        float tmp61 = phi_out36 * tmp57 + tmp23 * tmp0.z + phi_out42 * tmp58;
        float3 tmp62 = float3(sqrt(tmp60 * tmp60 + tmp59 * tmp59 + tmp61 * tmp61), 0.0f, 0.0f);
        tmp63 = float3(tmp59, tmp60, tmp61) / tmp62.xxx;
        sret_ptr.k1.x = tmp63.x;
        sret_ptr.k1.y = tmp63.y;
        sret_ptr.k1.z = tmp63.z;
        phi_in64 = 0;
        phi_in66 = 0.0f;
        phi_in68 = 0.0f;
        phi_in70 = 0.0f;
        phi_in72 = 0.0f;
        if (tmp23 > 0.0f) {
            float3 tmp74 = tmp63 * tmp4;
            phi_in64 = 0;
            phi_in66 = 0.0f;
            phi_in68 = 0.0f;
            phi_in70 = 0.0f;
            phi_in72 = 0.0f;
            if (tmp74.x + tmp74.y + tmp74.z > 0.0f) {
                sret_ptr.event_type = 1;
                sret_ptr.handle = 0;
                phi_in64 = 1;
                phi_in66 = 1.0f;
                phi_in68 = tmp63.z;
                phi_in70 = tmp63.y;
                phi_in72 = tmp63.x;
            }
        }
        phi_out65 = phi_in64;
        phi_out67 = phi_in66;
        phi_out69 = phi_in68;
        phi_out71 = phi_in70;
        phi_out73 = phi_in72;
        phi_in49 = phi_out67;
        phi_in51 = phi_out69;
        phi_in53 = phi_out71;
        phi_in55 = phi_out73;
        if (phi_out65 == 0) {
            sret_ptr.k1 = float3(0.0f, 0.0f, 0.0f);
            sret_ptr.pdf = 0.0f;
            sret_ptr.edf_over_pdf = float3(0.0f, 0.0f, 0.0f);
            sret_ptr.event_type = 0;
            phi_in49 = 0.0f;
            phi_in51 = 0.0f;
            phi_in53 = 0.0f;
            phi_in55 = 0.0f;
        }
    }
    phi_out50 = phi_in49;
    phi_out52 = phi_in51;
    phi_out54 = phi_in53;
    phi_out56 = phi_in55;
    float3 tmp75 = float3(phi_out56, phi_out54, phi_out52) * tmp0;
    tmp76 = tmp75.x + tmp75.y + tmp75.z;
    if (tmp76 < 0.0f) {
        sret_ptr.k1 = float3(0.0f, 0.0f, 0.0f);
        sret_ptr.pdf = 0.0f;
        sret_ptr.edf_over_pdf = float3(0.0f, 0.0f, 0.0f);
        sret_ptr.event_type = 0;
        phi_in77 = 0.0f;
        phi_in79 = 0.0f;
        phi_in81 = 0.0f;
    } else {
        float tmp83 = min(max(1.0f - tmp3 * tmp2, 0.0f), 1.0f);
        float tmp84 = (pow(1.0f - min(max(tmp76, 0.0f), 1.0f), 4.0f) * (min(max(1.0f - tmp1 * tmp2, 0.0f), 1.0f) - tmp83) + tmp83) * phi_out50;
        sret_ptr.edf_over_pdf.x = tmp84;
        sret_ptr.edf_over_pdf.y = tmp84;
        sret_ptr.edf_over_pdf.z = tmp84;
        phi_in77 = phi_out52;
        phi_in79 = phi_out54;
        phi_in81 = phi_out56;
    }
    phi_out78 = phi_in77;
    phi_out80 = phi_in79;
    phi_out82 = phi_in81;
    float3 tmp85 = float3(phi_out82, phi_out80, phi_out78);
    float3 tmp86 = tmp85 * tmp4;
    float tmp87 = asfloat((asint(tmp86.x + tmp86.y + tmp86.z) & -2147483648) | 1065353216);
    float3 tmp88 = float3(tmp0.x * tmp87, tmp0.y * tmp87, tmp0.z * tmp87) * tmp85;
    sret_ptr.pdf = max(tmp88.x + tmp88.y + tmp88.z, 0.0f) * 0.3183098733f;
    return;
}

void mdl_surface_emission_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float tmp1;
    float3 tmp2;
    float tmp4;
    float3 phi_in;
    float3 phi_out;
    float3 tmp7;
    float tmp8;
    float tmp9;
    float tmp10;
    float phi_in11;
    float phi_out12;
    float tmp17;
    float3 tmp0 = state.normal;
    sret_ptr.edf.x = 0.0f;
    sret_ptr.edf.y = 0.0f;
    sret_ptr.edf.z = 0.0f;
    tmp1 = state.text_results[17].x;
    tmp2 = sret_ptr.k1;
    float3 tmp3 = tmp2 * tmp0;
    tmp4 = tmp3.x + tmp3.y + tmp3.z;
    phi_in = float3(0.0f, 0.0f, 0.0f);
    if (!(tmp4 < 0.0f)) {
        float tmp5 = min(max(1.0f - state.text_results[21].z * tmp1, 0.0f), 1.0f);
        float tmp6 = (min(max(1.0f - max(1.0f - state.text_results[16].w, 0.0f) * tmp1, 0.0f), 1.0f) - tmp5) * pow(1.0f - min(max(tmp4, 0.0f), 1.0f), 4.0f) + tmp5;
        tmp7 = float3(tmp6, tmp6, tmp6);
        phi_in = tmp7;
    }
    phi_out = phi_in;
    tmp8 = phi_out.x;
    tmp9 = phi_out.y;
    tmp10 = phi_out.z;
    phi_in11 = 0.0f;
    if (!(tmp10 == 0.0f && (tmp8 == 0.0f && tmp9 == 0.0f))) {
        float3 tmp13 = state.geom_normal * tmp2;
        float tmp14 = asfloat((asint(tmp13.x + tmp13.y + tmp13.z) & -2147483648) | 1065353216);
        float3 tmp15 = float3(tmp0.x * tmp14, tmp0.y * tmp14, tmp0.z * tmp14) * tmp2;
        float tmp16 = max(tmp15.x + tmp15.y + tmp15.z, 0.0f);
        sret_ptr.cos = tmp16;
        tmp17 = tmp16 * 0.3183098733f;
        sret_ptr.edf.x = tmp8 * 0.3183098733f;
        sret_ptr.edf.y = tmp9 * 0.3183098733f;
        sret_ptr.edf.z = tmp10 * 0.3183098733f;
        phi_in11 = tmp17;
    }
    phi_out12 = phi_in11;
    sret_ptr.pdf = phi_out12;
    return;
}

void mdl_surface_emission_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = sret_ptr.k1;
    float3 tmp2 = tmp1 * state.geom_normal;
    float tmp3 = asfloat((asint(tmp2.x + tmp2.y + tmp2.z) & -2147483648) | 1065353216);
    float3 tmp4 = float3(tmp0.x * tmp3, tmp0.y * tmp3, tmp0.z * tmp3) * tmp1;
    sret_ptr.pdf = max(tmp4.x + tmp4.y + tmp4.z, 0.0f) * 0.3183098733f;
    return;
}

void mdl_surface_emission_auxiliary(in Edf_auxiliary_data sret_ptr, in Shading_state_material state)
{
    return;
}

float3 mdl_surface_emission_intensity(in Shading_state_material state)
{
    return float3(state.text_results[18].w, state.text_results[19].x, state.text_results[19].y);
}

float3 mdl_volume_absorption_coefficient(in Shading_state_material state)
{
    return state.text_results[16].xyz;
}

public bool mdl_thin_walled(in Shading_state_material state)
{
    return mdl_read_argblock_as_bool(state.arg_block_offset);
}

public float mdl_standalone_geometry_cutout_opacity(in Shading_state_material state)
{
    int tmp0;
    float tmp1;
    int tmp2;
    structtype0 tmp16;
    float3 newret;
    int tmp17;
    float newret2;
    float phi_in;
    float phi_out;
    float tmp18;
    float tmp19;
    tmp0 = state.arg_block_offset;
    tmp1 = mdl_read_argblock_as_float(tmp0 + 820);
    tmp2 = mdl_read_argblock_as_int(tmp0 + 824);
    int tmp3 = mdl_read_argblock_as_int(tmp0 + 72);
    int tmp4 = mdl_read_argblock_as_int(tmp0 + 76);
    Derived_float tmp5 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 80), 0.0f, 0.0f);
    Derived_float tmp6 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 84), 0.0f, 0.0f);
    Derived_float tmp7 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 88), 0.0f, 0.0f);
    Derived_float tmp8 = constr_Derived_float(mdl_read_argblock_as_float(tmp0 + 92), 0.0f, 0.0f);
    bool tmp9 = mdl_read_argblock_as_bool(tmp0 + 96);
    bool tmp10 = mdl_read_argblock_as_bool(tmp0 + 97);
    bool tmp11 = mdl_read_argblock_as_bool(tmp0 + 98);
    int tmp12 = mdl_read_argblock_as_int(tmp0 + 100);
    Derived_float3 tmp13 = constr_Derived_float3(float3(mdl_read_argblock_as_float(tmp0 + 112), mdl_read_argblock_as_float(tmp0 + 116), mdl_read_argblock_as_float(tmp0 + 120)), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    Derived_float3 tmp14 = constr_Derived_float3(float3(mdl_read_argblock_as_float(tmp0 + 128), mdl_read_argblock_as_float(tmp0 + 132), mdl_read_argblock_as_float(tmp0 + 136)), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    Derived_float3 tmp15 = constr_Derived_float3(float3(mdl_read_argblock_as_float(tmp0 + 144), mdl_read_argblock_as_float(tmp0 + 148), mdl_read_argblock_as_float(tmp0 + 152)), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    tmp16 = _ZN11OmniSurface9OmniImage16texture_lookup_2EU7uniformu10texture_2dU7uniformbu5colorU7uniformbu6float2U7uniformN11OmniSurface9OmniImage9wrap_modeEU7uniformN11OmniSurface9OmniImage9wrap_modeEN4base23texture_coordinate_infoE(tmp2, mdl_read_argblock_as_bool(tmp0 + 16), float3(mdl_read_argblock_as_float(tmp0 + 32), mdl_read_argblock_as_float(tmp0 + 36), mdl_read_argblock_as_float(tmp0 + 40)), mdl_read_argblock_as_bool(tmp0 + 48), constr_Derived_float2(float2(mdl_read_argblock_as_float(tmp0 + 56), mdl_read_argblock_as_float(tmp0 + 60)), float2(0.0f, 0.0f), float2(0.0f, 0.0f)), mdl_read_argblock_as_int(tmp0 + 64), mdl_read_argblock_as_int(tmp0 + 68), _ZN11OmniSurface9OmniImage28compute_texture_coordinate_2EU7uniformN4base25texture_coordinate_systemEU7uniformiU7uniformfU7uniformfU7uniformfU7uniformfU7uniformbU7uniformbU7uniformbU7uniformN11OmniSurface9OmniImage15projection_modeEU7uniformu6float3U7uniformu6float3U7uniformu6float3(state, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15));
    newret = tmp16.m_0;
    tmp17 = mdl_read_argblock_as_int(tmp0 + 828);
    if (tmp17 == 0) {
        newret2 = tmp16.m_1;
        phi_in = newret2;
    } else if (tmp17 == 1)
        phi_in = newret.x;
    else if (tmp17 == 2)
        phi_in = newret.y;
    else if (tmp17 == 3)
        phi_in = newret.z;
    else if (tmp17 == 4)
        phi_in = 1.0f;
    else if (tmp17 == 7) {
        tmp18 = (newret.x + newret.y + newret.z) * 0.3333333433f;
        phi_in = tmp18;
    } else if (tmp17 == 6) {
        tmp19 = newret.x * 0.2126709968f + newret.y * 0.7151600122f + newret.z * 0.07216899842f;
        phi_in = tmp19;
    } else
        phi_in = 0.0f;
    phi_out = phi_in;
    float tmp20 = tex_texture_isvalid(tmp2) ? phi_out : mdl_read_argblock_as_float(tmp0 + 832);
    return mdl_read_argblock_as_bool(tmp0 + 816) ? tmp1 == 0.0f ? tmp20 : tmp1 <= tmp20 ? 1.0f : 0.0f : 1.0f;
}