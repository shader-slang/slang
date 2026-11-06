// The ocean shader from ShaderToys by afl_ext.
// https://www.shadertoy.com/view/MdXyzX
// afl_ext 2017-2024
// MIT License

// This shader is modified to use Slang's automatic differentiation feature
// to compute the normal of the wave by differentiating the `getwaves` function
// with regard to `position`. See `normal` function for how autodiff is used.

import playground;

import ocean_utils;

float4 imageMain(uint2 dispatchThreadID, int2 screenSize)
{
  float2 size = float2(screenSize.x, screenSize.y);

  // get the ray
  float3 ray = getRay(dispatchThreadID.xy, size);
  if(ray.y >= 0.0) {
    // if ray.y is positive, render the sky
    float3 C = getAtmosphere(ray) + getSun(ray);
    float4 color = float4(aces_tonemap(C * 2.0),1.0);
    return color;
  }

  // now ray.y must be negative, water must be hit
  // define water planes
  float3 waterPlaneHigh = float3(0.0, 0.0, 0.0);
  float3 waterPlaneLow = float3(0.0, -WATER_DEPTH, 0.0);

  // define ray origin, moving around
  float3 origin = float3(time * 2.0, CAMERA_HEIGHT, 1);

  // calculate intersections and reconstruct positions
  float highPlaneHit = intersectPlane(origin, ray, waterPlaneHigh, float3(0.0, 1.0, 0.0));
  float lowPlaneHit = intersectPlane(origin, ray, waterPlaneLow, float3(0.0, 1.0, 0.0));
  float3 highHitPos = origin + ray * highPlaneHit;
  float3 lowHitPos = origin + ray * lowPlaneHit;

  // raymatch water and reconstruct the hit pos
  float dist = raymarchwater(origin, highHitPos, lowHitPos, WATER_DEPTH);
  float3 waterHitPos = origin + ray * dist;

  // calculate normal at the hit position
  float3 N = normal(waterHitPos.xz, 0.01, WATER_DEPTH);

  // smooth the normal with distance to avoid disturbing high frequency noise
  float interpFactor = 0.8 * min(1.0, sqrt(dist*0.01) * 1.1);
  N = lerp(N, float3(0.0, 1.0, 0.0), interpFactor);

  // calculate fresnel coefficient
  float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));

  // reflect the ray and make sure it bounces up
  float3 R = normalize(reflect(ray, N));
  R.y = abs(R.y);

  // calculate the reflection and approximate subsurface scattering
  float3 reflection = getAtmosphere(R) + getSun(R);
  float3 scattering = float3(0.0293, 0.0698, 0.1717) * 0.1 * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);

  // return the combined result
  float3 C = fresnel * reflection + scattering;
  float4 color = float4(aces_tonemap(C * 2.0), 1.0);
  return color;
}