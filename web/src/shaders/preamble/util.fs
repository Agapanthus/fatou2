#include "types.fs"

#define getCoords() imExpand(vec2(TexCoords[0], TexCoords[1]) - vec2(.5))

// vec4(contrast,phase,shift,igamma) = (0.0, 1.0, 0.4, 1.0)
RGB makeColors(Real32 v, vec4 params) {
    // based on https://krazydad.com/tutorials/makecolors.php
    v = params.x*v + params.y;
    return pow(sin(vec3(v, v + 1. * params.z, v + 2. * params.z)) * 0.5 + 0.5, vec3(params.w));
}

#define INDEX(A) {{INDEX}}