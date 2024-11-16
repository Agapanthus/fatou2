// Based on https://github.com/Tw1ddle/Sky-Shader/blob/master/src/shaders/glsl/sky.fragment

// Based on "A Practical Analytic Model for Daylight" aka The Preetham Model, the de facto standard analytic skydome model
// http://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf
// Original implementation by Simon Wallner: http://www.simonwallner.at/projects/atmospheric-scattering
// Improved by Martin Upitis: http://blenderartists.org/forum/showthread.php?245954-preethams-sky-impementation-HDR
// Three.js integration by zz85: http://twitter.com/blurspline / https://github.com/zz85 / http://threejs.org/examples/webgl_shaders_sky.html
// Additional uniforms, refactoring and integrated with editable sky example: https://twitter.com/Sam_Twidale / https://github.com/Tw1ddle/Sky-Particles-Shader

// Modified by Eric Skaliks
// -> Removed vertex shader
// -> Added dithering
// -> Added preset interpolation

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

// use nearest filter + repeat!
uniform sampler2D u_texture_0;

/*
varying vec3 vWorldPosition;

uniform vec3 cameraPos;
uniform float depolarizationFactor;
uniform float luminance;
uniform float mieCoefficient;
uniform float mieDirectionalG;
uniform vec3 mieKCoefficient;
uniform float mieV;
uniform float mieZenithLength;
uniform float numMolecules;
uniform vec3 primaries;
uniform float rayleigh;
uniform float rayleighZenithLength;
uniform float refractiveIndex;
uniform float sunAngularDiameterDegrees;
uniform float sunIntensityFactor;
uniform float sunIntensityFalloffSteepness;
uniform vec3 sunPosition;
uniform float tonemapWeighting;
uniform float turbidity;
*/




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

vec4 bluenoise(vec2 fc)
{
    vec2 iChannel0Resolution = vec2(1024.0);
    // I use floor+0.5 to allow linear texture filtering (because, I always need nearest filter!)
    return texture2D(u_texture_0, (floor(fc) + 0.5) / iChannel0Resolution);
}

//note: works for structured patterns too [0;1)
float remap_noise_tri_erp(const float v)
{
    float r2 = 0.5 * v;
    float f1 = sqrt(r2);
    float f2 = 1.0 - sqrt(r2 - 0.25);
    return (v < 0.5) ? f1 : f2;
}



#define ARR_LEN 21

#define FILL_PARAMETERS(/**first parameters*/N, turbidity, rayleigh, mieCoefficient, mieDirectionalG, luminance,/**second parameters*/ refractiveIndex, numMolecules, depolarizationFactor, rayleighZenithLength, mieV, mieZenithLength, sunIntensityFactor, sunIntensityFalloffSteepness, sunAngularDiameterDegrees, tonemapWeighting,/**primaries*/ p1, p2, p3, /**mie factors*/ m1, m2, m3)parameters[N * ARR_LEN + 0] = turbidity; parameters[N * ARR_LEN + 1] = rayleigh; parameters[N * ARR_LEN + 2] = mieCoefficient; parameters[N * ARR_LEN + 3] = mieDirectionalG; parameters[N * ARR_LEN + 4] = luminance; parameters[N * ARR_LEN + 5] = refractiveIndex; parameters[N * ARR_LEN + 6] = numMolecules; parameters[N * ARR_LEN + 7] = depolarizationFactor; parameters[N * ARR_LEN + 8] = rayleighZenithLength; parameters[N * ARR_LEN + 9] = mieV; parameters[N * ARR_LEN + 10] = mieZenithLength; parameters[N * ARR_LEN + 11] = sunIntensityFactor; parameters[N * ARR_LEN + 12] = sunIntensityFalloffSteepness; parameters[N * ARR_LEN + 13] = sunAngularDiameterDegrees; parameters[N * ARR_LEN + 14] = tonemapWeighting; parameters[N * ARR_LEN + 15] = p1; parameters[N * ARR_LEN + 16] = p2; parameters[N * ARR_LEN + 17] = p3; parameters[N * ARR_LEN + 18] = m1; parameters[N * ARR_LEN + 19] = m2; parameters[N * ARR_LEN + 20] = m3;
#define APPLY_PARAMETERS(N) turbidity = parameters[N * ARR_LEN + 0]; rayleigh = parameters[N * ARR_LEN + 1]; mieCoefficient = parameters[N * ARR_LEN + 2]; mieDirectionalG = parameters[N * ARR_LEN + 3]; luminance = parameters[N * ARR_LEN + 4]; refractiveIndex = parameters[N * ARR_LEN + 5]; numMolecules = parameters[N * ARR_LEN + 6]; depolarizationFactor = parameters[N * ARR_LEN + 7]; rayleighZenithLength = parameters[N * ARR_LEN + 8]; mieV = parameters[N * ARR_LEN + 9]; mieZenithLength = parameters[N * ARR_LEN + 10]; sunIntensityFactor = parameters[N * ARR_LEN + 11]; sunIntensityFalloffSteepness = parameters[N * ARR_LEN + 12]; sunAngularDiameterDegrees = parameters[N * ARR_LEN + 13]; tonemapWeighting = parameters[N * ARR_LEN + 14]; primaries = vec3(parameters[N * ARR_LEN + 15],parameters[N * ARR_LEN + 16],parameters[N * ARR_LEN + 17]); mieKCoefficient = vec3(parameters[N * ARR_LEN + 18], parameters[N * ARR_LEN + 19], parameters[N * ARR_LEN + 20]); 

void main()
{
    float distance = 400000.0;
    
    float parameters[5 * ARR_LEN + 0];
    
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
        1.00029, 2.542e25, 0.02, 8400.0, 3.936, 34000.0, 1000.0, 1.5, 0.04, 9.5,
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
    
    cameraPos = vec3(1500000.0 + u_resolution.x * 1100.0, - 40000.0, 4000000.0);
    
    vec2 st = gl_FragCoord.xy / u_resolution.y; // / u_resolution.xy;
    vec3 vWorldPosition = vec3((st + vec2(0.5, - 0.2)) * 2000000.0, distance);
    
    turbidity = 2.5;
    rayleigh = 2.295;
    mieCoefficient = 0.011475;
    mieDirectionalG = 0.814;
    luminance = 1.0;
    
    inclination = 0.4987 - u_mouse.y / u_resolution.y * 0.5 + 0.11 ;
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


    for(int i = 0; i<ARR_LEN; i++) {
        parameters[0*ARR_LEN+i] = mix(parameters[1*ARR_LEN+i], parameters[3*ARR_LEN+i], max(0.0, min(1.0,15.0*(u_mouse.y/u_resolution.y-0.18) ) ));
    }
    APPLY_PARAMETERS(0)
    
    //////////////////////////
    
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
    float sunE = sunIntensity(dot(sunDirection, UP), sunIntensityFactor, sunIntensityFalloffSteepness);
    vec3 Lin = pow(sunE * ((betaRTheta + betaMTheta) / (betaR + betaM)) * (1.0 - Fex), vec3(1.5));
    Lin *= mix(vec3(1.0), pow(sunE * ((betaRTheta + betaMTheta) / (betaR + betaM)) * Fex, vec3(0.5)), clamp(pow(1.0 - dot(UP, sunDirection), 5.0), 0.0, 1.0));
    
    // Composition + solar disc
    float sunAngularDiameterCos = cos(sunAngularDiameterDegrees);
    float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta);
    if(st.y < 0.2) {  // Eric: Hide sun under horizon
        sundisk *= max(0.0, 1.0 - pow( abs(st.y - 0.21)*40.0, 0.1));
    }
    vec3 L0 = vec3(0.1) * Fex;
    L0 += sunE * 19000.0 * Fex * sundisk;
    vec3 texColor = Lin + L0;
    texColor *= 0.04;
    texColor += vec3(0.0, 0.001, 0.0025) * 0.3;
    
    // Tonemapping
    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(tonemapWeighting));
    vec3 curr = Uncharted2Tonemap((log2(2.0 / pow(luminance, 4.0))) * texColor);
    vec3 color = curr * whiteScale;
    vec3 retColor = pow(color, vec3(1.0 / (1.2 + (1.2 * sunfade))));
    
    // Dithering (See http://loopit.dk/banding_in_games.pdf and https://www.shadertoy.com/view/MslGR8)
    vec2 seed = gl_FragCoord.xy;
    seed += 1337.0 * fract(u_time);
    vec3 bn = bluenoise(seed).rgb;
    
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
    gl_FragColor = vec4(retColor + 5.0 * (vec3(bn_tri) * 2.0 - 0.5) / 255.0, 1.0);
    
    //gl_FragColor=vec4(retColor,1.);
}