#define prec highp

in prec vec2 TexCoords;
out prec vec4 fragColor;

const int iter = 1000;

uniform prec vec4 c;
uniform prec vec2 zoom, pos;

prec vec2 ds(prec float a) {
    prec vec2 z;
    z.x = a;
    z.y = 0.0;
    return z;
}

prec vec2 ds_add(prec vec2 dsa, prec vec2 dsb) {
    prec vec2 dsc;
    prec float t1, t2, e;

    t1 = dsa.x + dsb.x;
    e = t1 - dsa.x;
    t2 = ((dsb.x - e) + (dsa.x - (t1 - e))) + dsa.y + dsb.y;

    dsc.x = t1 + t2;
    dsc.y = t2 - (dsc.x - t1);
    return dsc;
}

prec vec2 ds_mult(prec vec2 dsa, prec vec2 dsb) {
    prec vec2 dsc;
    prec float c11, c21, c2, e, t1, t2;
    prec float a1, a2, b1, b2, cona, conb, split = 8193.;

    cona = dsa.x * split;
    conb = dsb.x * split;
    a1 = cona - (cona - dsa.x);
    b1 = conb - (conb - dsb.x);
    a2 = dsa.x - a1;
    b2 = dsb.x - b1;

    c11 = dsa.x * dsb.x;
    c21 = a2 * b2 + (a2 * b1 + (a1 * b2 + (a1 * b1 - c11)));

    c2 = dsa.x * dsb.y + dsa.y * dsb.x;

    t1 = c11 + c2;
    e = t1 - c11;
    t2 = dsa.y * dsb.y + ((c2 - e) + (c11 - (t1 - e))) + c21;

    dsc.x = t1 + t2;
    dsc.y = t2 - (dsc.x - t1);

    return dsc;
}

prec vec2 ds_square(prec vec2 dsa) { return ds_mult(dsa, dsa); }


#define product(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define divide(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))
#define magnitudeA(a) sqrt(2.0*a.x*a.x)
#define msizeA(a) 2.0*a.x*a.x // Using them will result in quite interesting results!
#define magnitude(a) sqrt(a.x*a.x+a.y*a.y)
#define msize(a) (a.x*a.x+a.y*a.y)

void main() {
    prec vec2 tc = vec2(TexCoords[0], TexCoords[1]);
    prec vec2 zx = ds_add(ds_mult(ds(zoom.x), ds(tc.x - 0.5)), ds(pos.x));
    prec vec2 zy = ds_add(ds_mult(ds(zoom.y), ds(tc.y - 0.5)), ds(pos.y));

    int i = 0;


    prec vec2 px = zx;
    prec vec2 py = zy;
    for (int i_i = 0; i_i < iter; i_i++) {
        i = i_i;

        // square p - z // TODO: Reuse the squares!
        px = ds_add(ds_square(px) - ds_square(py), zx);
        py = ds_add(ds_mult(px, py) + ds_mult(py, px), zy);

        // try to calculate magnitude. TODO: we don't do the real thing here,
        // because we don't have a square root yet
        if (sqrt(px.x*px.x + py.x*py.x) > c.x)
            break;
        // if (magnitude(p) > c.x)
        //    break;
    }
        


    prec float v = (float(i + 1)) * c.y * 0.02;
    fragColor =
        (i >= (iter - 1))
            ? vec4(0.0)
            : sin(vec4(v, v + 1.0, v + 2.0, 0.0)) * 0.4 +
                  0.5; // https://krazydad.com/tutorials/makecolors.php
    fragColor.w = 1.0;
}