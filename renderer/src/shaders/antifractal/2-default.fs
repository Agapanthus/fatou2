#version 300 es
// <![CDATA[

#define prec highp

in prec vec2 TexCoords;
out prec vec4 fragColor;

uniform prec vec2 zoom;
uniform prec vec2 pos;

#define product(a, b) vec2(a.x *b.x - a.y * b.y, a.x * b.y + a.y * b.x)
#define divide(a, b)                                                           \
  vec2(((a.x * b.x + a.y * b.y) / (b.x * b.x + b.y * b.y)),                    \
       ((a.y * b.x - a.x * b.y) / (b.x * b.x + b.y * b.y)))
#define magnitudeA(a) sqrt(2.0 * a.x * a.x)
#define msizeA(a)                                                              \
  2.0 * a.x *a.x // Using them will result in quite interesting results!
#define magnitude(a) sqrt(a.x *a.x + a.y * a.y)
#define msize(a) (a.x * a.x + a.y * a.y)

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

prec vec2 ds_square(prec float ds) {
  return ds_mult(vec2(0., ds), vec2(0., ds));

  /*prec vec2 dsc;
  prec float c11, c21, c2, e, t1, t2;
  prec float a1, a2, b1, b2, cona, conb, split = 8193.;

  cona = ds * split;
  conb = ds * split;
  a1 = cona - (cona - ds);
  b1 = conb - (conb - ds);
  a2 = ds - a1;
  b2 = ds - b1;

  c11 = ds * ds;
  c21 = a2 * b2 + (a2 * b1 + (a1 * b2 + (a1 * b1 - c11)));

  c2 = ds * 0. + 0. * ds;

  t1 = c11 + c2;
  e = t1 - c11;
  t2 = 0. * 0. + ((c2 - e) + (c11 - (t1 - e))) + c21;

  dsc.x = t1 + t2;
  dsc.y = t2 - (dsc.x - t1);

  return dsc;*/
}

prec vec2 ds_sub(prec vec2 dsa, prec vec2 dsb) {
  return ds_add(dsa, ds_mult(ds(-1.0), dsb));
}

void main() {
  prec vec2 z;
  prec vec2 tc = vec2(TexCoords[0], TexCoords[1]);
  z.x = zoom.x * (tc.x - 0.5) + pos.x;
  z.y = zoom.y * (tc.y - 0.5) + pos.y;

  prec float MEC = 1.0;
  prec float MUC = 0.0;
  prec float F1 = 1.0;
  prec float F2 = 2.0;

  prec float x1 = z.x;
  prec float x2 = z.y;

  /*
  prec vec2 a1;
  a1 = ds_square(x1);
  prec vec2 a2;
  a2 = ds_square(x2);

  prec float A = F1 * a1.x - a2.x;
  */

  int iter = 1;

  prec float x3;
  prec float A;
  int x1t = int(x1);
  for (int i = 0; i < iter; i++) {
    A = float(int(F1) * int(x1) * int(x1) - int(x2) * int(x2));
    x1t = x1t * int(x2) * int(F2);
    // x1 = mod(x1 * x2 * F2, 2147483648.0) ;
    x3 = float(x1t ^ int(A));
  }

  // prec float x4 = 0.0; // TODO
  // prec float res = MUC * x4 + MEC * x3;

  prec float res = x3 / 255.0;

  fragColor = vec4(vec3(clamp(res, 0.0, 1.0)), 1.0);
  // if (a1.y == 0.)
  //  fragColor = vec4(1., 0,0,1.);
}
// ]]>