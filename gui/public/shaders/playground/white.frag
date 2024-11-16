#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0);

    // To keep the unused variable
    if(fragTexCoord.x < -1000.0){
        outColor = vec4(0.0);
    }
}
