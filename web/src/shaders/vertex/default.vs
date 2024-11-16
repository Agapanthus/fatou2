#version 100
// <![CDATA[
#define prec highp

varying prec vec2 TexCoords;

attribute prec vec2 aPos;
attribute prec vec2 aTexCoords;

void main() {
  TexCoords = aTexCoords;
  gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}

// ]]>