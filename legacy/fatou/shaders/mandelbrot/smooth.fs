#include "arith-complex.fs"
#include "util.fs"

in highp vec2 TexCoords;
out highp vec4 fragColor;

uniform highp vec4 c;
uniform highp vec4 c2;
uniform Complex zoom, pos;

void main() {
    Complex z = getComplexStart(zoom, pos, TexCoords);

    Complex p = MkComplex(0.);

    Real32 cx2 = c.x * c.x;

    Index i = MkIndex(c.w);
    for (Index j = MkIndex(0); j <= INDEX(MkIndex(c.w)); j++) {
        p = imAdd(imSquare(p), z);
        //p.x += c.y / float(j+1) / 100.;
        //p.y += c.y / float(j+1) / 100.;
        p.x *= (1.+ c.y / float(j+1) / p.y / 50.);
        i = j;

        // approximate magnitude.
        if (magnitudeSquaredFast(p) > cx2)
            break;
    }

    // Smoothing
    Real32 log_zn = log(magnitudeSquaredFast(p)) * 0.5;
    Real32 nu = log(log_zn * 1.44269504088896) * 1.44269504088896 * c.z;

    Real32 v = (MkReal32(i + 1) - nu) * 0.02; // + zx.x * 10.0;
    fragColor.xyz = (i >= (MkIndex(c.w) - 1))
                    ? RGB(0.0)
                    : makeColors(v, c2);
    fragColor.w = 1.0;
}