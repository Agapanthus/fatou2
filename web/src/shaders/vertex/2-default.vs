#version 300 es
// <![CDATA[
#define prec highp

out prec vec2 TexCoords;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

void main() {
  TexCoords = aTexCoords;
  gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}

// ]]>