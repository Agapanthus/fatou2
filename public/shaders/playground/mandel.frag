#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


layout(binding = 1) uniform UniformBufferObject2 {
	vec2 pos;
    float zoom;
	int maxIter;
} ubo;

float magnitudeSquaredFast(vec2 z) {
	return z.x*z.x + z.y*z.y;
}

vec2 imAdd(vec2 x, vec2 y){
	return x + y;
}

vec2 imSquare(vec2 z){
	return vec2(
		z.x*z.x-z.y*z.y,
		2*z.x*z.y
	);
}

vec4 makeColors(float v) {
	float contrast = 3.0;
	float shift = 1.0;
	float phase = 0.0;
	float igamma = .7;

    v = contrast*v + phase;
    return pow(sin(vec4(v, v + 1. * shift, v + 2. * shift, 1.0)) * 0.5 + 0.5, vec4(igamma));
}

void main() {
	float cx = 4.0;
	float smoothing = 1.0;

	////////////

	vec2 z = fragTexCoord * ubo.zoom + ubo.pos;
	vec2 p = vec2(0.);
	float cx2 = (cx*cx);

//	outColor = vec4( sin(vec3(iter)) * .5 + .5, 1.0);  
	int i = maxIter;
	for (int j=0; j <= maxIter; j++) {
		p = imAdd(imSquare(p), z);
		// p.x *= (1.+ c.y / float(j+1) / p.y / 50.);
		if(magnitudeSquaredFast(p) > cx2) {
			i = j;
			break;
		}
	}
	
    // Smoothing
    float log_zn = log(magnitudeSquaredFast(p)) * 0.5;
    float nu = log(log_zn * 1.44269504088896) * 1.44269504088896 * smoothing;

    float v = (float(i + 1) - nu) * 0.02; // + zx.x * 10.0;
    outColor = (i >= (maxIter - 1))
                    ? vec4(0.0)
                    : makeColors(v);

	// outColor = vec4(1.0);
}

