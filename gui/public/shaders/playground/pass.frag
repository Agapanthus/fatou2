#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, texture(texSampler, fragTexCoord).a);
    outColor = texture(texSampler, fragTexCoord).zyxw;
}