// A simple shader ported from https://www.shadertoy.com/view/XdlSDs.
// Author: dynamite
// License: CC BY-NC-SA 3.0 https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en

import playground;

[playground::TIME]
uniform float time;

[playground::FRAME_ID]
uniform float frame;

typealias vec2 = float2;
typealias vec3 = float3;
typealias vec4 = float4;

float4 imageMain(uint2 dispatchThreadID, int2 screenSize)
{
    vec2 p = (dispatchThreadID.xy * 2.0f - screenSize.xy) / (float)screenSize.y;
    float tau = float.getPi() * 2.0;
    float a = atan2(p.x,p.y);
    float r = length(p)*0.75;
    vec2 uv = vec2(a / tau, r);
    float t = frame / 60.f;

	//get the color
	float xCol = (uv.x - t/3) * 3.0;
	xCol = fmod(abs(xCol), 3.0f);
	vec3 horColour = vec3(0.25, 0.25, 0.25);
	
	if (xCol < 1.0)
    {
		horColour.r += 1.0 - xCol;
		horColour.g += xCol;
	}
	else if (xCol < 2.0)
    {
		xCol -= 1.0;
		horColour.g += 1.0 - xCol;
		horColour.b += xCol;
	}
	else
    {
		xCol -= 2.0;
		horColour.b += 1.0 - xCol;
		horColour.r += xCol;
	}

	// draw color beam
	uv = (2.0 * uv) - 1.0;
	float beamWidth = (0.7 + 
        0.5 * cos(uv.x * 10.0 * tau * 0.15 * clamp(floor(5.0 + 10.0 * cos(t)), 0.0, 10.0)))
        * abs(1.0 / (30.0 * uv.y));
	vec3 horBeam = vec3(beamWidth);
	return vec4(((horBeam) * horColour), 1.0);
}