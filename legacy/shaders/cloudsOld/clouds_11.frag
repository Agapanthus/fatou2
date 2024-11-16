#ifdef GL_ES
precision mediump float;
#endif

///////////////////////////
///
/// TODO: Wolken mit FBN sind besser!
/// https://www.shadertoy.com/view/4dSBDt
/// und was ist raymarching? https://www.shadertoy.com/view/XsVGz3
/// https://www.shadertoy.com/view/XsfXW8
/// https://www.shadertoy.com/view/XlsXDB
/// Godrays? https://www.shadertoy.com/view/MstBWs
/// Siehe auch https://www.shadertoy.com/view/ltlSWB und https://www.shadertoy.com/view/XscXRs
/// Godrays, Wasser, Wolken: https://www.shadertoy.com/view/4sXGRM
///
///////////////////////////

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

// Some useful functions
vec3 mod289(vec3 x) {return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) {return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) {return mod289(((x * 34.0) + 1.0) * x); }

// TODO: PIXI only!
#define PIXI
#ifdef PIXI

uniform sampler2D uSampler;
vec4 bluenoise(vec2 fc)
{
    fc = floor(fc) + 0.5;
    // PIXI wants to be smart and scales the texture when the screen is small.................
    return texture2D(
        uSampler, vec2(
            mod(fc.x / 1024.0 / min(1.0, u_resolution.x / 1024.0), 1.0),
            mod(fc.y / 1024.0 / min(1.0, u_resolution.y / 1024.0), 1.0)
        )
    );
}

#else

// use nearest filter + repeat!
uniform sampler2D u_texture_0;
vec4 bluenoise(vec2 fc)
{
    vec2 iChannel0Resolution = vec2(1024.0);
    // I use floor+0.5 to allow linear texture filtering (because, I always need nearest filter!)
    return texture2D(u_texture_0, (floor(fc) + 0.5) / iChannel0Resolution);
}

#endif

//
// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
float snoise(vec2 v) {
    
    // Precompute values for skewed triangular grid
    const vec4 C = vec4(0.211324865405187,
        // (3.0-sqrt(3.0))/6.0
        0.366025403784439,
        // 0.5*(sqrt(3.0)-1.0)
        - 0.577350269189626,
        // -1.0 + 2.0 * C.x
    0.024390243902439);
    // 1.0 / 41.0
    
    // First corner (x0)
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v-i + dot(i, C.xx);
    
    // Other two corners (x1, x2)
    vec2 i1 = vec2(0.0);
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 x1 = x0.xy + C.xx - i1;
    vec2 x2 = x0.xy + C.zz;
    
    // Do some permutations to avoid
    // truncation effects in permutation
    i = mod289(i);
    vec3 p = permute(
        permute(i.y + vec3(0.0, i1.y, 1.0))
        + i.x + vec3(0.0, i1.x, 1.0)
    );
    
    vec3 m = max(0.5 - vec3(
            dot(x0, x0),
            dot(x1, x1),
            dot(x2, x2)
        ), 0.0
    );
    
    m = m*m;
    m = m*m;
    
    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)
    
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x-ox;
    
    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h*h);
    
    // Compute final noise value at P
    vec3 g = vec3(0.0);
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * vec2(x1.x, x2.x) + h.yz * vec2(x1.y, x2.y);
    return 130.0 * dot(m, g);
}

vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float voronoiSmoothAsy(vec2 st, float turb, float asy, float smoothness) {
    
    float ellipse = 1.0;
    
    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);
    
    float m_dist = 0.0;
    
    for(int y =- 1; y <= 1; y ++ ) {
        for(int x =- 1; x <= 1; x ++ ) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(x), float(y));
            
            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);
            
            // Animate the point
            point = 0.5 + 0.5 * sin(u_time * turb + 6.2831 * point);
            
            // Vector between the pixel and the point
            vec2 diff = neighbor + point - f_st;
            
            diff.x *= ellipse;
            
            //diff.x=weibull(1.,diff.x,1.);
            if (diff.y > 0.0) {
                diff.y *= asy;
            }
            //2./(1. + exp2(-diff.x)) - 1.;
            
            // Distance to the point
            float dist = length(diff);
            
            // Keep the smooth distance
            m_dist += exp(-smoothness * dist);
        }
    }
    
    return - (1.0 / smoothness) * log(m_dist);
}
float sigmoid(float x) {
    return 2.0 / (1.0 + exp2(-x)) - 1.0;
}

float voronoiAsy(vec2 st, float turb, float asy) {
    
    float ellipse = 1.0;
    
    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);
    
    float m_dist = 1.0; // minimun distance
    
    for(int y =- 1; y <= 1; y ++ ) {
        for(int x =- 1; x <= 1; x ++ ) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(x), float(y));
            
            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);
            
            // Animate the point
            point = 0.5 + 0.5 * sin(u_time * turb + 6.2831 * point);
            
            // Vector between the pixel and the point
            vec2 diff = neighbor + point - f_st;
            
            diff.x *= ellipse;
            
            //diff.x=weibull(1.,diff.x,1.);
            if (diff.y > 0.0) {
                diff.y *= asy;
            }
            //2./(1. + exp2(-diff.x)) - 1.;
            
            // Distance to the point
            float dist = length(diff);
            
            // Keep the closer distance
            m_dist = min(m_dist, dist);
        }
    }
    
    return m_dist;
}
float voronoiAsyFake(vec2 st, float turb, float asy) {
    
    float ellipse = 1.0;
    
    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);
    
    float m_dist = 1.0; // minimun distance
    int y = 0;
    int x = 0;
    // Neighbor place in the grid
    vec2 neighbor = vec2(float(x), float(y));
    
    // Random position from current + neighbor place in the grid
    vec2 point = random2(i_st + neighbor);
    
    // Animate the point
    point = 0.5 + 0.5 * sin(u_time * turb + 6.2831 * point);
    
    // Vector between the pixel and the point
    vec2 diff = neighbor + point - f_st;
    
    diff.x *= ellipse;
    
    //diff.x=weibull(1.,diff.x,1.);
    if (diff.y > 0.0) {
        diff.y *= asy;
    }
    //2./(1. + exp2(-diff.x)) - 1.;
    
    // Distance to the point
    float dist = length(diff);
    
    // Keep the closer distance
    m_dist = min(m_dist, dist);
    
    return m_dist;
}

float voronoiAsyLumo(vec2 st, float turb, float asy, float smoothness, vec2 lightDir) {
    
    float ellipse = 1.0;
    
    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);
    
    float m_lumo = 0.0; // minimun luminosity
    float m_dist = 0.0;
    vec2 m_v = vec2(0.0);

    
    for(int y =- 1; y <= 1; y ++ ) {
        for(int x =- 1; x <= 1; x ++ ) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(x), float(y));
            
            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);
            
            // Animate the point
            point = 0.5 + 0.5 * sin(u_time * turb + 6.2831 * point);
            
            // Vector between the pixel and the point
            vec2 diff = neighbor + point - f_st;
            
            diff.x *= ellipse;
            
            if (diff.y > 0.0) {
                diff.y *= asy;
            }
            
            float lumo = max(dot(diff, lightDir), 0.0);
            
            float l_dist = exp(-smoothness * length(diff));
            m_dist += l_dist;
            
            // Keep the smooth distance
            m_lumo += lumo * l_dist; //exp(-smoothness*lumo);
        }
    }
    
    //float totalLum = -(1./smoothness)*log(m_dist);
    
    return m_lumo / m_dist * 1.0; // -(1./smoothness)*log(m_lumo);
    //return m_v * 200.0;
}

float pow2(float x) {
    return x * x;
}
float pow3(float x) {
    return x * x*x;
}
float ln(float x) {
    return log(x);
}
// Bubble of height 1 and length 1
float footBubble(float x) {
    
    x = (0.9 + (pow2(4.0 * (x * 10.0 + 3.0)) * ln(x * 10.0 + 1.0)) / pow3(x * 10.0 + 3.0) - (x * 10.0 + 3.0) * 0.3) / 2.95;
    
    return x;
}

float fineStuff(vec2 st, vec2 seedX) {
    return (
        // Feine Strukturen
        snoise(st * 32.0 + seedX * 0.2) * 0.03
        + snoise(st * 64.0 + seedX * 0.3) * 0.015
        + voronoiAsy(st * 32.0 + seedX * 0.1, 0.1, 1.0) * 0.12
        + voronoiAsy(st * 60.0 + seedX * 0.4, - 0.2, 1.0) *- 0.04
    );
}

float shadowAt(vec2 st, float seed, vec2 lightDir, float fact2, float fact3) {
    
    float shadowAsy = 1.0;
    vec2 seedX = vec2(seed, 0.0);
    float shadowSmooth = 12.0;
    float lumi = voronoiAsyLumo(st * vec2(4.0, 4.0) - seedX * 0.0, 0.005, shadowAsy, shadowSmooth, lightDir);
    float tlumi = lumi;
    
    lumi = voronoiAsyLumo(st * 9.0 - seedX * 0.2, 0.3, 1.0, shadowSmooth, lightDir) * 0.6;
    tlumi = tlumi + fact2 * 0.5 * lumi;
    
    lumi = voronoiAsyLumo(st * 16.0 - seedX * 0.3 - seed * 1.0, - 0.1, 1.0, shadowSmooth, lightDir) * 0.6;
    tlumi = tlumi + fact3 * 0.25 * lumi;
    
    return tlumi;
}

const float PI = 3.141592653589793238462643383279502884197169;
const vec3 UP = vec3(0.0, 1.0, 0.0);

vec3 totalRayleigh(vec3 lambda, float refractiveIndex, float depolarizationFactor, float numMolecules)
{
    return (8.0 * pow(PI, 3.0) * pow(pow(refractiveIndex, 2.0) - 1.0, 2.0) * (6.0 + 3.0 * depolarizationFactor)) / (3.0 * numMolecules * pow(lambda, vec3(4.0)) * (6.0 - 7.0 * depolarizationFactor));
}

vec3 totalMie(vec3 lambda, vec3 K, float T, float mieV)
{
    float c = 0.2 * T* 0.00000000000000001; // 10e-18;
    return.434 * c*PI * pow((2.0 * PI) / lambda, vec3(mieV - 2.0)) * K;
}

float rayleighPhase(float cosTheta)
{
    return (3.0 / (16.0 * PI)) * (1.0 + pow(cosTheta, 2.0));
}

float henyeyGreensteinPhase(float cosTheta, float g)
{
    return (1.0 / (4.0 * PI)) * ((1.0 - pow(g, 2.0)) / pow(1.0 - 2.0 * g*cosTheta + pow(g, 2.0), 1.5));
}

float sunIntensity(float zenithAngleCos, float sunIntensityFactor, float sunIntensityFalloffSteepness)
{
    float cutoffAngle = PI / 1.95; // Earth shadow hack
    return sunIntensityFactor * max(0.0, 1.0 - exp(-((cutoffAngle - acos(zenithAngleCos)) / sunIntensityFalloffSteepness)));
}

// Whitescale tonemapping calculation, see http://filmicgames.com/archives/75
// Also see http://blenderartists.org/forum/showthread.php?321110-Shaders-and-Skybox-madness
const float A = 0.15; // Shoulder strength
const float B = 0.50; // Linear strength
const float C = 0.10; // Linear angle
const float D = 0.20; // Toe strength
const float E = 0.02; // Toe numerator
const float F = 0.30; // Toe denominator
vec3 Uncharted2Tonemap(vec3 W)
{
    return ((W * (A * W+C * B) + D*E) / (W * (A * W+B) + D*F)) - E / F;
}

//note: works for structured patterns too [0;1)
float remap_noise_tri_erp(const float v)
{
    float r2 = 0.5 * v;
    float f1 = sqrt(r2);
    float f2 = 1.0 - sqrt(r2 - 0.25);
    return (v < 0.5) ? f1 : f2;
}

vec3 skyColor() {
    
    float whitePart = 0.3;
    vec3 color = mix(
        vec3(0.6784, 0.902, 0.9608),
        mix(
            vec3(0.3373, 0.7412, 0.9725),
            vec3(0.1294, 0.4745, 0.6745),
            gl_FragCoord.y / u_resolution.y * (1.0 - whitePart) + whitePart
        ),
        min(1.0, (1.0 / whitePart) * gl_FragCoord.y / u_resolution.y)
    );
    return color;
}

#define ARR_LEN 21

#define FILL_PARAMETERS(/**first parameters*/N, turbidity, rayleigh, mieCoefficient, mieDirectionalG, luminance,/**second parameters*/ refractiveIndex, numMolecules, depolarizationFactor, rayleighZenithLength, mieV, mieZenithLength, sunIntensityFactor, sunIntensityFalloffSteepness, sunAngularDiameterDegrees, tonemapWeighting,/**primaries*/ p1, p2, p3, /**mie factors*/ m1, m2, m3)parameters[N * ARR_LEN + 0] = turbidity; parameters[N * ARR_LEN + 1] = rayleigh; parameters[N * ARR_LEN + 2] = mieCoefficient; parameters[N * ARR_LEN + 3] = mieDirectionalG; parameters[N * ARR_LEN + 4] = luminance; parameters[N * ARR_LEN + 5] = refractiveIndex; parameters[N * ARR_LEN + 6] = numMolecules; parameters[N * ARR_LEN + 7] = depolarizationFactor; parameters[N * ARR_LEN + 8] = rayleighZenithLength; parameters[N * ARR_LEN + 9] = mieV; parameters[N * ARR_LEN + 10] = mieZenithLength; parameters[N * ARR_LEN + 11] = sunIntensityFactor; parameters[N * ARR_LEN + 12] = sunIntensityFalloffSteepness; parameters[N * ARR_LEN + 13] = sunAngularDiameterDegrees; parameters[N * ARR_LEN + 14] = tonemapWeighting; parameters[N * ARR_LEN + 15] = p1; parameters[N * ARR_LEN + 16] = p2; parameters[N * ARR_LEN + 17] = p3; parameters[N * ARR_LEN + 18] = m1; parameters[N * ARR_LEN + 19] = m2; parameters[N * ARR_LEN + 20] = m3;
#define APPLY_PARAMETERS(N)turbidity = parameters[N * ARR_LEN + 0]; rayleigh = parameters[N * ARR_LEN + 1]; mieCoefficient = parameters[N * ARR_LEN + 2]; mieDirectionalG = parameters[N * ARR_LEN + 3]; luminance = parameters[N * ARR_LEN + 4]; refractiveIndex = parameters[N * ARR_LEN + 5]; numMolecules = parameters[N * ARR_LEN + 6]; depolarizationFactor = parameters[N * ARR_LEN + 7]; rayleighZenithLength = parameters[N * ARR_LEN + 8]; mieV = parameters[N * ARR_LEN + 9]; mieZenithLength = parameters[N * ARR_LEN + 10]; sunIntensityFactor = parameters[N * ARR_LEN + 11]; sunIntensityFalloffSteepness = parameters[N * ARR_LEN + 12]; sunAngularDiameterDegrees = parameters[N * ARR_LEN + 13]; tonemapWeighting = parameters[N * ARR_LEN + 14]; primaries = vec3(parameters[N * ARR_LEN + 15], parameters[N * ARR_LEN + 16], parameters[N * ARR_LEN + 17]); mieKCoefficient = vec3(parameters[N * ARR_LEN + 18], parameters[N * ARR_LEN + 19], parameters[N * ARR_LEN + 20]);


#define DRAG_MULT 0.048
//#define ITERATIONS_RAYMARCH 13
//#define ITERATIONS_NORMAL 48
#define ITERATIONS_RAYMARCH 8
#define ITERATIONS_NORMAL 25

#define Mouse vec2(0.634,0.581) //(u_mouse.xy / u_resolution.xy)
#define Resolution (u_resolution.xy)
#define Time (u_time*0.6)


vec2 wavedx(vec2 position, vec2 direction, float speed, float frequency, float timeshift) {
    float x = dot(direction, position) * frequency + timeshift * speed;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

float getwaves(vec2 position){
	float iter = 0.0;
    float phase = 6.0;
    float speed = 2.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<ITERATIONS_NORMAL;i++){
        vec2 p = vec2(sin(iter), cos(iter));
        vec2 res = wavedx(position, p, speed, phase, Time);
        position += normalize(p) * res.y * weight * DRAG_MULT;
        w += res.x * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.18;
        speed *= 1.07;
    }
    return w / ws;
}


float getwavesLow(vec2 position){
	float iter = 0.0;
    float phase = 6.0;
    float speed = 2.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<3;i++){
        vec2 p = vec2(sin(iter), cos(iter));
        vec2 res = wavedx(position, p, speed, phase, Time);
        position += normalize(p) * res.y * weight * DRAG_MULT;
        w += res.x * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.18;
        speed *= 1.07;
    }
    return w / ws;
}

float getwavesRaymarch(vec2 position){
	float iter = 0.0;
    float phase = 6.0;
    float speed = 2.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<ITERATIONS_RAYMARCH;i++){
        vec2 p = vec2(sin(iter), cos(iter));
        vec2 res = wavedx(position, p, speed, phase, Time);
        position += normalize(p) * res.y * weight * DRAG_MULT;
        w += res.x * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.18;
        speed *= 1.07;
    }
    return w / ws;
}

float raymarchwater(vec3 camera, vec3 start, vec3 end, float depth){
    vec3 pos = start;
    float h = 0.0;
    float hupper = depth;
    float hlower = 0.0;
    vec2 zer = vec2(0.0);
    vec3 dir = normalize(end - start);
    for(int i=0;i<318;i++){
        h = getwavesRaymarch(pos.xz * 0.1) * depth - depth;
        if(h + 0.01 > pos.y) {
            return distance(pos, camera);
        }
        pos += dir * (pos.y - h);
    }
    return -1.0;
}

float raymarchwaterLow(vec3 camera, vec3 start, vec3 end, float depth){
    vec3 pos = start;
    float h = 0.0;
    float hupper = depth;
    float hlower = 0.0;
    vec2 zer = vec2(0.0);
    vec3 dir = normalize(end - start);
    for(int i=0;i<318;i++){
        h = getwavesLow(pos.xz * 0.1) * depth - depth;
        if(h + 0.01 > pos.y) {
            return distance(pos, camera);
        }
        pos += dir * (pos.y - h);
    }
    return -1.0;
}

float H = 0.0;
vec3 normal(vec2 pos, float e, float depth){
    vec2 ex = vec2(e, 0);
    H = getwaves(pos.xy * 0.1) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    return normalize(cross(normalize(a-vec3(pos.x - e, getwaves(pos.xy * 0.1 - ex.xy * 0.1) * depth, pos.y)), 
                           normalize(a-vec3(pos.x, getwaves(pos.xy * 0.1 + ex.yx * 0.1) * depth, pos.y + e))));
}
vec3 normalLow(vec2 pos, float e, float depth){
    vec2 ex = vec2(e, 0);
    H = getwavesLow(pos.xy * 0.1) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    return normalize(cross(normalize(a-vec3(pos.x - e, getwavesLow(pos.xy * 0.1 - ex.xy * 0.1) * depth, pos.y)), 
                           normalize(a-vec3(pos.x, getwavesLow(pos.xy * 0.1 + ex.yx * 0.1) * depth, pos.y + e))));
}
mat3 rotmat(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return mat3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s, 
	oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s, 
	oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

vec3 getRay(vec2 uv){
    uv = (uv * 2.0 - 1.0) * vec2(Resolution.x / Resolution.y, 1.0);
	vec3 proj = normalize(vec3(uv.x, uv.y, 1.0) + vec3(uv.x, uv.y, -1.0) * pow(length(uv), 2.0) * 0.05);	
    if(Resolution.x < 400.0) return proj;
	vec3 ray = rotmat(vec3(0.0, -1.0, 0.0), 3.0 * (Mouse.x * 2.0 - 1.0)) * rotmat(vec3(1.0, 0.0, 0.0), 1.5 * (Mouse.y *0.9* 2.0 - 1.0)) * proj;
    return ray;
}

float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal)
{ 
    return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); 
}

float sun(vec3 ray, float sunHeight, float strengthen){
 	vec3 sd = normalize(vec3(1.0- ray.y*strengthen, sunHeight + ray.x*strengthen,1.0-ray.y*strengthen));   
    return pow(max(0.0, dot(ray, sd)), 528.0) * 110.0;
}

vec3 aces_tonemap(vec3 color){	
	mat3 m1 = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
	);
	vec3 v = m1 * color;    
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / 2.2));	
}


void main() {
    
    float distance = 40000.0;
    
    float parameters[6 * ARR_LEN + 0];
    
    float phaseDistance = -0.4;
    float phase = gl_FragCoord.x / u_resolution.x + u_mouse.x / u_resolution.x * abs(phaseDistance) * 5.0 * 3.6 - 2.5 / 0.2 * abs(phaseDistance);
    

    float hqWater = 1.0;

    //
    FILL_PARAMETERS(
        0, 1.25, 1.0, 0.00335, 0.787, 1.0,
        1.000317, 2.542e25, 0.067, 615.0, 4.012, 500.0, 1111.0, 0.98, 0.012, 9.5,
        0.00000068, 0.00000055, 0.00000045,
        0.686, 0.678, 0.666
    )
    
    // Red sunset
    FILL_PARAMETERS(
        1, 4.7, 2.28, 0.005, 0.82, 1.0,
        1.00029, 2.542e25, 0.02, 8400.0, 3.936, 34000.0, 1000.0, 1.5, 0.032, 9.5,
        0.00000068, 0.00000055, 0.00000045,
        0.686, 0.678, 0.666
    )
    
    // Alien day
    FILL_PARAMETERS(
        2, 12.575, 5.75, 0.0074, 0.468, 1.0,
        1.000128, 2.542e25, 0.137, 3795.0, 4.007, 7100.0, 1024.0, 1.3, 0.006, 9.5,
        0.00000068, 0.00000055, 0.00000045,
        0.686, 0.678, 0.666
    )
    
    // Blue dawn
    FILL_PARAMETERS(
        3, 2.5, 2.295, 0.011475, 0.814, 1.0,
        1.000262, 2.542e25, 0.095, 540.0, 3.979, 1000.0, 1151.0, 1.22, 0.012, 9.5,
        0.00000068, 0.00000055, 0.00000045,
        0.686, 0.678, 0.666
    )
    
    // Blood sky
    FILL_PARAMETERS(
        4, 4.75, 6.77, 0.0191, 0.793, 1.0,
        1.000633, 2.542e25, 0.01, 1425.0, 4.042, 1600.0, 2069.0, 2.26, 0.01487, 9.5,
        0.0000007929, 0.0000003766, 0.0000003172,
        0.686, 0.678, 0.666
    )

    // Blood sky special
    FILL_PARAMETERS(
        5, 14.75, 3.77, 0.0391, 0.83, 1.0,
        1.000533, 2.542e25, 0.1, 1425.0, 4.042, 800.0, 2069.0, 2.9, 0.01487, 9.5,
        0.0000007929, 0.0000003766, 0.0000003172,
        0.686, 0.678, 0.666
    )
    
    vec3 cameraPos;
    float depolarizationFactor;
    float luminance;
    float mieCoefficient;
    float mieDirectionalG;
    vec3 mieKCoefficient;
    float mieV;
    float mieZenithLength;
    float numMolecules;
    vec3 primaries;
    float rayleigh;
    float rayleighZenithLength;
    float refractiveIndex;
    float sunAngularDiameterDegrees;
    float sunIntensityFactor;
    float sunIntensityFalloffSteepness;
    vec3 sunPosition;
    float tonemapWeighting;
    float turbidity;
    float inclination;
    float azimuth;
    
    cameraPos = vec3(150000.0 + u_resolution.x * 110.0, - 4000.0, 400000.0);
    
    vec2 altSt = gl_FragCoord.xy / u_resolution.y; // / u_resolution.xy;

    vec2 st = gl_FragCoord.xy / u_resolution.xy;
    st.x *= u_resolution.x / u_resolution.y;
    vec2 cloudSt = st;

    if(/*ray.y >= -0.01*/ st.y < 0.185 && phase < phaseDistance * 3.0){

       altSt.y = pow( (altSt.y - 0.2)*2.0, 0.4);
       cloudSt.y = pow( (cloudSt.y - 0.2)*2.0, 0.4);
    }

    float skyScale = 200000.0;
    vec2 skyTranslate = vec2(0.5, - 0.2);
    vec3 vWorldPosition = vec3((altSt + skyTranslate) * skyScale, distance);
    
    turbidity = 2.5;
    rayleigh = 2.295;
    mieCoefficient = 0.011475;
    mieDirectionalG = 0.814;
    luminance = 1.0;
    
    int mode = 0;

	float waterdepth = 2.1;
    float elongationFactor = 1.0;
    float daytime = 0.0; //  u_mouse.y / u_resolution.y; //- cos(u_time*0.01)*0.2 +0.38;
    if (u_mouse.y / u_resolution.y > 0.6) {
        daytime = 0.5;
        elongationFactor = 0.0;
    } else  if (u_mouse.y / u_resolution.y > 0.3) {
        daytime = 0.22;
        waterdepth = 1.4;     
        elongationFactor = 1.4;   
    } else {
        daytime = 0.25144;
        mode = 1;
        waterdepth = 0.3;
        elongationFactor = 1.1;
    }

    inclination = 0.4987 - daytime * 0.5 + 0.11 ;
    azimuth = 0.2268;
    
    float theta = PI * (inclination - 0.5);
    float phi = 2.0 * PI * (azimuth - 0.5);
    sunPosition = vec3(
        distance * cos(phi),
        distance * sin(phi) * sin(theta),
        distance * sin(phi) * cos(theta)
    );
    
    // Refractive index of air
    refractiveIndex = 1.000262;
    
    // Number of molecules per unit volume for air at 288.15K and 1013mb (sea level -45 celsius)
    numMolecules = 2.542e25;
    
    // Depolarization factor for air wavelength of primaries
    depolarizationFactor = 0.095;
    primaries = vec3(0.00000068, 0.00000055, 0.00000045);
    
    // Mie, K coefficient for the primaries
    mieKCoefficient = vec3(0.686, 0.678, 0.666);
    mieV = 3.979;
    
    // Optical length at zenith for molecules
    rayleighZenithLength = 540.0;
    mieZenithLength = 1000.0;
    
    // Sun intensity factors
    sunIntensityFactor = 1151.0;
    sunIntensityFalloffSteepness = 1.22;
    
    // Visual size of sun
    sunAngularDiameterDegrees = 0.00639;
    
    // W factor in tonemap calculation
    tonemapWeighting = 9.50;
    
    //float programmMix = max(0.0, min(1.0, 15.0 * (daytime - 0.18)));
    float programmMix = max(0.0, min(1.0, 15.0 * (daytime - 0.185)));
    
    for(int i = 0; i < ARR_LEN; i ++ ) {
        parameters[0 * ARR_LEN + i] = mix(parameters[1 * ARR_LEN + i], parameters[3 * ARR_LEN + i], programmMix);
    }
    APPLY_PARAMETERS(0)
    if(mode == 1) {

        APPLY_PARAMETERS(5)
        programmMix = 0.992;
    }
    
    //////////////////////////
    
    if (phase > phaseDistance * 3.0) {
        mieCoefficient = 0.0;
    }
    
    // Rayleigh coefficient
    float sunfade = 1.0 - clamp(1.0 - exp((sunPosition.y / 450000.0)), 0.0, 1.0);
    float rayleighCoefficient = rayleigh - (1.0 * (1.0 - sunfade));
    vec3 betaR = totalRayleigh(primaries, refractiveIndex, depolarizationFactor, numMolecules) * rayleighCoefficient;
    
    // Mie coefficient
    vec3 betaM = totalMie(primaries, mieKCoefficient, turbidity, mieV) * mieCoefficient;
    
    // Optical length, cutoff angle at 90 to avoid singularity
    float zenithAngle = acos(max(0.0, dot(UP, normalize(vWorldPosition - cameraPos))));
    float denom = cos(zenithAngle) + 0.15 * pow(93.885 - ((zenithAngle * 180.0) / PI), - 1.253);
    float sR = rayleighZenithLength / denom;
    float sM = mieZenithLength / denom;
    
    // Combined extinction factor
    vec3 Fex = exp(-(betaR * sR + betaM * sM));
    
    // In-scattering
    vec3 sunDirection = normalize(sunPosition);
    float cosTheta = dot(normalize(vWorldPosition - cameraPos), sunDirection);
    vec3 betaRTheta = betaR * rayleighPhase(cosTheta * 0.5 + 0.5);
    vec3 betaMTheta = betaM * henyeyGreensteinPhase(cosTheta, mieDirectionalG);
    
    if (phase > phaseDistance * 2.0) {
        betaMTheta *= 0.0;
    }
    float sunE = sunIntensity(dot(sunDirection, UP), sunIntensityFactor, sunIntensityFalloffSteepness);
    vec3 Lin = pow(sunE * ((betaRTheta + betaMTheta) / (betaR + betaM)) * (1.0 - Fex), vec3(1.5));
    Lin *= mix(vec3(1.0), pow(sunE * ((betaRTheta + betaMTheta) / (betaR + betaM)) * Fex, vec3(0.5)), clamp(pow(1.0 - dot(UP, sunDirection), 5.0), 0.0, 1.0));
    
    // Composition + solar disc
    float sunAngularDiameterCos = cos(sunAngularDiameterDegrees);
    float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta);
    if (altSt.y < 0.2 && phase < phaseDistance * 7.0) { // Eric: Hide sun under horizon
        sundisk *= max(0.0, 1.0 - pow(abs(altSt.y - 0.21) * 40.0, 0.1));
    }
    
    if (phase > phaseDistance * 6.0) {
        sundisk *= 0.0;
    }
    vec3 L0 = vec3(0.1) * Fex;
    L0 += sunE * 19000.0 * Fex * sundisk;
    vec3 texColor = Lin + L0;
    texColor *= 0.04;
    texColor += vec3(0.0, 0.001, 0.0025) * 0.3;
    
    ////// Sonnenkoordinaten
    
    vec3 sunProjection = normalize(sunPosition + cameraPos);
    // TODO: Diese Zeile stimmt so nicht. Eigentlich haette ich gerne die Koordinaten der Sonne wie sie auf dem Bildschirm erscheint in sunXY
    sunProjection = 20000.0 + 101.0 * sunProjection / sunProjection.z * distance - cameraPos;
    vec2 sunXY = sunProjection.xy / skyScale - skyTranslate;
    sunXY.x = 0.5;
    
    //////////////////////////////////////////
    //////////////////////////////////////////

    
    // seed + time
    float seed = u_time * 0.01;
    cloudSt.x += seed;
    vec2 seedX = vec2(seed, 0.0);
    
    //vec3 color=skyColor();
    
    float cloudMinHeight = 0.29;
    float cloudLayerStrength = 1.0;
    float cloudDensity = footBubble((cloudSt.y - cloudMinHeight) / cloudLayerStrength);
    
    float cloud = 0.0;
    float shadowDark = 1.4;
    
    vec2 lightVector = sunXY - gl_FragCoord.xy / u_resolution.xy;
    
    float lightAngle =- seed * 0.0;
    vec2 lightDir = (
        normalize(lightVector)
    );
    //vec2(cos(lightAngle),sin(lightAngle));
    
    float tlumi = 0.0;
    if (phase < phaseDistance * 0.0) {
        tlumi = shadowAt(cloudSt, seed, lightDir, 1.0, 1.0);
        tlumi *= shadowDark;
    } else if (phase < phaseDistance * -1.0) {
        tlumi = shadowAt(cloudSt, seed, lightDir, 1.0, 0.0);
        tlumi *= shadowDark;
    } else if (phase < phaseDistance * -2.0) {
        tlumi = shadowAt(cloudSt, seed, lightDir, 0.0, 00.0);
        tlumi *= shadowDark;
    }
    
    float details = fineStuff(cloudSt, seedX);
    if (phase < phaseDistance * 1.0) {
        tlumi = min(tlumi, 0.5 + 1.0 * details);
    }
    
    // if the sun is behind the clouds and the lightVector is very short, the clouds should appear as shadow only, as light comes from behind!
    
    if (phase < phaseDistance * 8.0) {
        tlumi = mix(1.0, tlumi, max(0.0, min(1.0, 0.5 * exp(2.0 * length(lightVector)))));
    }
    
    float asy = 6.0;
    if (phase > phaseDistance * -5.0) {
        asy = 1.0;
    }

    if (phase > phaseDistance * -4.0) {
        cloudDensity = 1.0;
    }

    cloud =-
    // Richtige Wolken
    (voronoiAsy(cloudSt * vec2(4.0, 4.0) - seedX * 0.0, 0.005, asy) * 1.3 + 0.01 - 0.3 * cloudDensity);
    
    if (phase > phaseDistance * -6.0) {
        cloud =- (voronoiAsyFake(cloudSt * vec2(4.0, 4.0) - seedX * 0.0, 0.005, asy) * 1.3 + 0.01 - 0.3 * cloudDensity);
    }
    
    if (phase < phaseDistance * 2.0) {
        
        cloud -=
        // Schäfchen, die sich langsam ineinander umformen
        voronoiSmoothAsy(cloudSt * 9.0 - seedX * 0.2, 0.3, 1.0, 16.00) * 0.6;
        
        if (phase < phaseDistance * 4.0) {
            cloud -= voronoiAsy(cloudSt * 16.0 - seedX * 0.3 - seed * 1.0, - 0.1, 1.0) * 0.3;
            
            if (phase < phaseDistance * 5.0) {
                cloud -= details;
            }
        }
        
    }
    cloud = cloud * 0.9 + 0.3 + 0.5;
    
    if (phase < phaseDistance * -4.0) {
        cloud = cloud - 0.5 + cloudDensity * 0.8;
    }
        
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    
    // Tonemapping
    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(tonemapWeighting));
    vec3 curr = Uncharted2Tonemap((log2(2.0 / pow(luminance, 4.0))) * texColor);
    vec3 color = curr * whiteScale;
    vec3 retColor = pow(color, vec3(1.0 / (1.2 + (1.2 * sunfade))));
    
    if (phase > phaseDistance * -3.0) {
        retColor = skyColor();
    }
    
    vec3 lightColor = vec3(1.0);
    
    if (phase < phaseDistance * 6.0) {
        lightColor = mix(texColor*0.6, vec3(1.0), pow(programmMix,5.0));
    }
    if (phase < phaseDistance * 7.0) {
          if(st.y > 0.185) {
            //lightColor = whiteScale * Uncharted2Tonemap((log2(2.0 / pow(luminance / tlumi *0.00001, 4.0))) * lightColor);
            lightColor = pow(lightColor, vec3(1.0 / (1.2 + (1.2 * sunfade))));

            
          }

    }
    vec3 shadowColor = vec3(0.3);

    float cloudEnhance = 1.3;
    if(phase > phaseDistance * 3.0) {
        cloudEnhance = 1.0;
    }

    if(st.y > 0.185 || phase > phaseDistance * 7.0) {
        //lightColor = whiteScale * Uncharted2Tonemap((log2(2.0 / pow(luminance / tlumi *0.00001, 4.0))) * lightColor);
        lightColor = pow(lightColor, vec3(1.0 / (1.2 + (1.2 * sunfade))));

        

        // Add clouds
        retColor = mix(retColor, mix(lightColor, shadowColor, pow(tlumi*cloudEnhance, 1.)), max(0.0, cloud));
        

    } else {
        retColor = mix(texColor, mix(lightColor*0.2, shadowColor, pow(tlumi*cloudEnhance, 1.)), max(0.0, cloud*0.7));
        
        // Optional: Wolken werfen Schatten auf Wasser
        //curr = retColor;
    }

    /////////////////////////////////////////////////7
    //////////////////////////////////////////////////


    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////

    vec3 ray = vec3(0.0);
    if (phase < phaseDistance * 4.0) {
        vec2 uv = gl_FragCoord.xy / u_resolution.xy*2.0- vec2(0.55,-0.09);
        
        vec3 wfloor = vec3(0.0, -waterdepth, 0.0);
        vec3 wceil = vec3(0.0, 0.0, 0.0);
        vec3 orig = vec3(0.0, 4., 0.0);
        ray = getRay(uv);
        float hihit = intersectPlane(orig, ray, wceil, vec3(0.0, 1.0, 0.0));
        if( /*ray.y< -0.01*/ st.y < 0.185){
            float lohit = intersectPlane(orig, ray, wfloor, vec3(0.0, 1.0, 0.0));
            vec3 hipos = orig + ray * hihit;
            vec3 lopos = orig + ray * lohit;
            float dist =raymarchwaterLow(orig, hipos, lopos, waterdepth); 

            if (phase < phaseDistance * 9.0 && hqWater > 0.0) {
                dist = raymarchwater(orig, hipos, lopos, waterdepth);
            }
            vec3 pos = orig + ray * dist;

            vec3 N = normalLow(pos.xz, 0.001, waterdepth);
             
            if (phase < phaseDistance * 9.0 && hqWater > 0.0) { //&& abs(gl_FragCoord.x - u_resolution.x/2.0)< 200.0) {
                N = normal(pos.xz, 0.001, waterdepth);
            }
            vec2 velocity = N.xz * (1.0 - N.y);
            N = mix(vec3(0.0, 1.0, 0.0), N, 1.0 / (dist * dist * 0.01 + 1.0));
            vec3 R = reflect(ray, N);
            float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));
            
            vec3 skyColorAbove = vec3(0.4,0.4,0.8); // TODO:
            if (phase < phaseDistance * 5.0) {
                skyColorAbove = curr;
            }
            float sunHeight = (0.5 - inclination)*5.0;
            float elongate = -2.5 * (gl_FragCoord.y / u_resolution.y - 0.2) * elongationFactor;
            float sunReflectionInt = 1.0;
            if (phase > phaseDistance * 6.0) {
                sunReflectionInt = 0.0;
            }
            vec3 C = fresnel * skyColorAbove * 2.0 + sunReflectionInt * fresnel * sun(R, sunHeight, elongate) * lightColor;
            //tonemapping
            C = aces_tonemap(C);

            retColor = C;
        }
    }
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    
    // Dithering (See http://loopit.dk/banding_in_games.pdf and https://www.shadertoy.com/view/MslGR8)
    
    if (phase < phaseDistance * 9.0) {
        vec2 r_seed = gl_FragCoord.xy;
        r_seed += 1337.0 * fract(u_time);
        vec3 bn = bluenoise(r_seed).rgb;
        
        #define CHROMATIC
        #ifdef CHROMATIC
        vec3 bn_tri = vec3(
            remap_noise_tri_erp(bn.x),
            remap_noise_tri_erp(bn.y),
            remap_noise_tri_erp(bn.z)
        );
        #else
        float bn_tri = remap_noise_tri_erp(bn.r);
        #endif
        
        // TODO: Why do I need more noise (5.0-times as much) to hide the bands?
        retColor = retColor + 5.0 * (vec3(bn_tri) * 2.0 - 0.5) / 255.0;
    }
    

    ////////////////////////////////////
    /////////////////////////////////////

    

    gl_FragColor = vec4(retColor, 1.0);
    
}