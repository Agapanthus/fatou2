#version 100
// <![CDATA[

#define MAX_POLY 100
#define prec highp

uniform sampler2D screenTexture;
// in prec vec2 TexCoords;
varying prec vec2 TexCoords;

const int iter = 100;
// uniform int iter;
uniform prec vec2 c, zoom, pos;

uniform prec float coe[MAX_POLY];
// uniform int coec;
const int coec = 45;

#define product(a, b) vec2(a.x *b.x - a.y * b.y, a.x * b.y + a.y * b.x)
#define divide(a, b)                                                           \
    vec2(((a.x * b.x + a.y * b.y) / (b.x * b.x + b.y * b.y)),                  \
         ((a.y * b.x - a.x * b.y) / (b.x * b.x + b.y * b.y)))
#define magnitudeA(a) sqrt(2.0 * a.x * a.x)
#define msizeA(a)                                                              \
    2.0 * a.x *a.x // Using them will result in quite interesting results!
#define magnitude(a) sqrt(a.x *a.x + a.y * a.y)
#define msize(a) (a.x * a.x + a.y * a.y)

void main() {
    prec vec2 z;
    prec vec2 tc = vec2(TexCoords[0], TexCoords[1]);
    z.x = zoom.x * (tc.x - 0.5) + pos.x;
    z.y = zoom.y * (tc.y - 0.5) + pos.y;

    int i;

    prec float nu = 0.0;
    for (int i_i = 0; i_i < iter; i_i++) {
        i = i_i;
        prec vec2 f = vec2(coe[0], 0.0);
        prec vec2 ff = vec2(0.0);

        prec vec2 ex = vec2(1.0, 0.0);
        for (int ii = 1; ii <= coec; ii++) {
            if (coe[ii] != 0.0)
                ff += (coe[ii] * float(ii)) * ex;
            ex = product(ex, z);
            if (coe[ii] != 0.0)
                f += coe[ii] * ex;
        }
        prec vec2 dif = divide(f, ff);
        if (msize(dif) < c.x) {
            nu = log(log(float(msize(dif))) / log(float(c.x))) *
                 log(float(coec));
            break;
        }
        z -= dif;
    }
    prec float vali = (float(i + 1) - nu) * float(c.y);
    if (vali > float(iter))
        vali = float(iter);

    prec vec2 nz = normalize(z) * 0.4 + 0.5;
    prec vec4 v4 = vec4(0.0);
    // Geht in IE11 nicht
    // gl_FragColor = ( (i >= (iter - 1)) ? v4 : (1.0 - vali /
    // float(iter))*texture2D(screenTexture, nz) );

    gl_FragColor = texture2D(screenTexture, nz) * (1.0 - vali / float(iter));
    gl_FragColor.w = 1.0;
}
// ]]>