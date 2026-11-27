// The ocean shader from ShaderToys by afl_ext.
// https://www.shadertoy.com/view/MdXyzX
// afl_ext 2017-2024
// MIT License

// This shader is modified to use Slang's automatic differentiation feature
// to compute the normal of the wave by differentiating the `getwaves` function
// with regard to `position`. See `normal` function for how autodiff is used.

import playground;

[playground::TIME]
uniform float time;

[playground::MOUSE_POSITION]
uniform float4 mousePosition;

static const float DRAG_MULT = 0.38; // changes how much waves pull on the water
static const float WATER_DEPTH = 1.0; // how deep is the water
static const float CAMERA_HEIGHT = 1.5; // how high the camera should be
static const int ITERATIONS_RAYMARCH = 12; // waves iterations of raymarching
static const int ITERATIONS_NORMAL = 37; // waves iterations when calculating normals

// Calculates wave value and its derivative, 
// for the wave direction, position in space, wave frequency and time
[Differentiable]
[PreferCheckpoint]
float2 wavedx(float2 position, float2 direction, float frequency, float timeshift) {
  float x = dot(direction, position) * frequency + timeshift;
  float wave = exp(sin(x) - 1.0);
  float dx = wave * cos(x);
  return float2(wave, -dx);
}

// Calculates waves by summing octaves of various waves with various parameters.
// This function is a generic function that can be specialized by different number of iterations.
[Differentiable]
float getwaves<int iterations>(float2 position) {
  float wavePhaseShift = length(position) * 0.1; // this is to avoid every octave having exactly the same phase everywhere
  float iter = 0.0; // this will help generating well distributed wave directions
  float frequency = 1.0; // frequency of the wave, this will change every iteration
  float timeMultiplier = 5.0; // time multiplier for the wave, this will change every iteration
  float weight = 1.0;// weight in final sum for the wave, this will change every iteration
  float sumOfValues = 0.0; // will store final sum of values
  float sumOfWeights = 0.0; // will store final sum of weights
  [ForceUnroll]
  for(int i=0; i < iterations; i++) {
    // generate some wave direction that looks kind of random
    float2 p = float2(sin(iter), cos(iter));
    
    // calculate wave data
    float2 res = wavedx(position, p, frequency, time * timeMultiplier + wavePhaseShift);

    // shift position around according to wave drag and derivative of the wave
    position += p * res.y * weight * DRAG_MULT;

    // add the results to sums
    sumOfValues += res.x * weight;
    sumOfWeights += weight;

    // modify next octave ;
    weight = lerp(weight, 0.0, 0.2);
    frequency *= 1.18;
    timeMultiplier *= 1.07;

    // add some kind of random value to make next wave look random too
    iter += 1232.399963;
  }
  // calculate and return
  return sumOfValues / sumOfWeights;
}

// Raymarches the ray from top water layer boundary to low water layer boundary
float raymarchwater(float3 camera, float3 start, float3 end, float depth) {
  float3 pos = start;
  float3 dir = normalize(end - start);
  for(int i=0; i < 64; i++) {
    // the height is from 0 to -depth
    float height = getwaves<ITERATIONS_RAYMARCH>(pos.xz) * depth - depth;
    // if the waves height almost nearly matches the ray height, assume its a hit and return the hit distance
    if(height + 0.01 > pos.y) {
      return distance(pos, camera);
    }
    // iterate forwards according to the height mismatch
    pos += dir * (pos.y - height);
  }
  // if hit was not registered, just assume hit the top layer, 
  // this makes the raymarching faster and looks better at higher distances
  return distance(start, camera);
}

// Calculate normal at point by using autodiff to get the derivative of `getwave` function
// with regard to `position`.
float3 normal(float2 pos, float e, float depth) {
   DifferentialPair<float2> diffPos = diffPair(pos);
   bwd_diff(getwaves<ITERATIONS_NORMAL>)(diffPos, 1.0);
   return normalize(float3(-diffPos.d.x, 1.0, -diffPos.d.y));
}

// Helper function generating a rotation matrix around the axis by the angle
float3x3 createRotationMatrixAxisAngle(float3 axis, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  return float3x3(
    oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
    oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
  );
}

// Helper function that generates camera ray based on UV and mouse
float3 getRay(float2 fragCoord, float2 resolution) {
  float2 uv = ((fragCoord.xy / resolution.xy) * 2.0 - 1.0) * float2(resolution.x / resolution.y, 1.0);
  // for fisheye, uncomment following line and comment the next one
  //float3 proj = normalize(float3(uv.x, uv.y, 1.0) + float3(uv.x, uv.y, -1.0) * pow(length(uv), 2.0) * 0.05);
  float3 proj = normalize(float3(uv.x, uv.y, 1.5));
  if (resolution.x < 40.0 || resolution.y < 40.0) {
    return proj;
  }
  var normalizedMouse = mousePosition.xy / float2(resolution);
  float3x3 mat = mul(createRotationMatrixAxisAngle(float3(0.0, -1.0, 0.0), 3.0 * ((1.0-normalizedMouse.x + 0.5) * 2.0 - 1.0)) ,
                     createRotationMatrixAxisAngle(float3(1.0, 0.0, 0.0), 0.5 + 1.5 * (((normalizedMouse.y == 0.0 ? 0.35 : normalizedMouse.y) * 1.0) * 2.0 - 1.0)));
  return mul(mat, proj);
}

// Ray-Plane intersection checker
float intersectPlane(float3 origin, float3 direction, float3 point, float3 normal) {
  return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0);
}

// Some very barebones but fast atmosphere approximation
float3 extra_cheap_atmosphere(float3 raydir, float3 sundir) {
  sundir.y = max(sundir.y, -0.07);
  float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
  float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
  float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
  float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
  float mymie = sundt * special_trick * 0.2;
  float3 suncolor = lerp(float3(1.0), max(float3(0.0), float3(1.0) - float3(5.5, 13.0, 22.4) / 22.4), special_trick2);
  float3 bluesky= float3(5.5, 13.0, 22.4) / 22.4 * suncolor;
  float3 bluesky2 = max(float3(0.0), bluesky - float3(5.5, 13.0, 22.4) * 0.002 * (special_trick + -6.0 * sundir.y * sundir.y));
  bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
  return bluesky2 * (1.0 + 1.0 * pow(1.0 - raydir.y, 3.0));
}

// Calculate where the sun should be, it will be moving around the sky
float3 getSunDirection() {
  return normalize(float3(sin(time), 1.0, cos(time)));
}

// Get atmosphere color for given direction
float3 getAtmosphere(float3 dir) {
  return extra_cheap_atmosphere(dir, getSunDirection()) * 0.5;
}

// Get sun color for given direction
float getSun(float3 dir) {
  return pow(max(0.0, dot(dir, getSunDirection())), 720.0) * 210.0;
}

// Great tonemapping function from my other shader: https://www.shadertoy.com/view/XsGfWV
float3 aces_tonemap(float3 color) {
  float3x3 m1 = float3x3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
  );

  float3x3 m2 = float3x3(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
  );

  float3 v = mul(color, m1);
  float3 a = v * (v + 0.0245786) - 0.000090537;
  float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
  return pow(clamp(mul((a / b), m2), 0.0, 1.0), float3(1.0 / 2.2));
}

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