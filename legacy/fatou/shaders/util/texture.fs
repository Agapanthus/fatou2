// <![CDATA[
#define prec highp
varying prec vec2 TexCoords;
uniform sampler2D screenTexture;
void main() {
  gl_FragColor = texture2D(screenTexture, vec2(TexCoords[0], TexCoords[1]));
}
// ]]>