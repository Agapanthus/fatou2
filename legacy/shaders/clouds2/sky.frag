precision highp float;

varying vec2 vTextureCoord;

//uniform vec3 uSunPos;

#pragma glslify: atmosphere = require(./atmosphere.frag)

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;


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

void main() {
    float daytime = 0.25144;

    float distance = 40000.0;

    float inclination = 0.4987 - daytime * 0.5 + 0.11 ;
    float azimuth = 0.2268;
    
    float theta = PI * (inclination - 0.5);
    float phi = 2.0 * PI * (azimuth - 0.5);
    vec3 sunPos = vec3(
        distance * cos(phi),
        distance * sin(phi) * sin(theta),
        distance * sin(phi) * cos(theta)
    );


    vec3 color = atmosphere(
        vec3(vTextureCoord,0.),           // normalized ray direction
        vec3(0,6372e3,0),               // ray origin
        sunPos,                        // position of the sun
        22.0,                           // intensity of the sun
        6371e3,                         // radius of the planet in meters
        6471e3,                         // radius of the atmosphere in meters
        vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient
        21e-6,                          // Mie scattering coefficient
        8e3,                            // Rayleigh scale height
        1.2e3,                          // Mie scale height
        0.758                           // Mie preferred scattering direction
    );

    // Apply exposure.
    color = 1.0 - exp(-1.0 * color);

    gl_FragColor = vec4(color, 1);
}
