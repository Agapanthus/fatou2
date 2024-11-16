#include "arith-int64.fs"
#include "arith-complex.fs"

in highp vec2 TexCoords;
out highp vec4 fragColor;

uniform Complex zoom, pos;

uniform highp vec4 c;

void main() {
    Complex32 tc = vec2(TexCoords[0], TexCoords[1]);

    // TODO: This coordinate calculation is suboptimal. When zooming out far,
    // there is only noise and artifacts. The original didn't have this problem.
    // See the assembly.
    UInt64 tx = u64Add(u64Mul(u64Expand(zoom.x), u64ExpandHi(tc.x - 0.5)),
                       u64ExpandHi(pos.x));
    UInt64 ty = u64Add(u64Mul(u64Expand(zoom.y), u64ExpandHi(tc.y - 0.5)),
                       u64ExpandHi(pos.y / (zoom.y / zoom.x)));

    UInt64 A = u64Sub(u64Square(tx), u64Square(ty));
    UInt64 B = u64Mul(u64Mul(tx, ty), u64Expand(c.w));

    Real32 res = 0.0;
    if (((B.hi ^ A.hi) & (1U << 31)) == 0U) // ^ A.hi
        res = 1.0;

    Real32 contrast = c.x;
    // prec float res =        x3 * contrast + (x3 / 2.0 / float(1 << 31) + 0.5)
    // * (1. - contrast);

    fragColor = vec4(vec3(res + pos.x * 0.000001 + c.x * 0.000001), 1.0);

    /*if (u64Mul(u64Expand(676008330U, 2776703425U),
                u64Expand(4207851578U, 1486413076U)) ==
        u64Expand(4195030453U, 3402776596U)) {
        fragColor.x = 1.0;
    }*/
    /*if (u64Sub(u64Expand(4015442496U, 1747804152U),
               u64Expand(4015442496U, 1747804153U)) ==
        u64Expand(4294967295U, 4294967295U)) {
        fragColor.x = 1.0;
    }*/
}
