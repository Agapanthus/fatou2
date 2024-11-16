#version 300 es
// <![CDATA[

#define prec highp

in prec vec2 TexCoords;
out prec vec4 fragColor;

uniform prec vec2 zoom;
uniform prec vec2 pos;
uniform prec vec2 mods;
uniform prec vec2 divs;

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

  prec float A = F1 * x1 * x1 - x2 * x2;
  x1 = x1 * x2 * F2;
  prec float x3 =
      float(int(mod(int(x1), int(mods.x))) ^ int(mod(int(A), int(mods.y)))) /
      divs.x;

  // prec float x4 = 0.0; // TODO
  // prec float res = MUC * x4 + MEC * x3;

  prec float res = x3 / 255.0;

  fragColor = vec4(vec3(clamp(res, 0.0, 1.0)), 1.0);
}
// ]]>