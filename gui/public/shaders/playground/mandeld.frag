#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


layout(binding = 1) uniform UniformBufferObject2 {
	dvec2 pos;
    double zoom;
	int maxIter;
	float iGamma;
	float play;
	float shift;
	float contrast;
	float phase;
	float radius;
	float smoothing;
} ubo;

float magnitudeSquaredFast(dvec2 z) {
	return float(z.x*z.x + z.y*z.y);
}

dvec2 imAdd(dvec2 x, dvec2 y){
	return x + y;
}

dvec2 imSquare(dvec2 z){
	return dvec2(
		z.x*z.x-z.y*z.y,
		2*z.x*z.y
	);
}

vec4 makeColors(float v) {
	float contrast = ubo.contrast;
	float shift =ubo.shift;
	float phase = ubo.phase;
	float igamma = ubo.iGamma;

    v = contrast*v + shift;
    return pow(sin(vec4(v, v + 1. * phase, v + 2. * phase, 1.0)) * 0.5 + 0.5, vec4(igamma));
}

void main() {
	float radius = ubo.radius;
	float smoothing = ubo.smoothing;
	int maxIter = ubo.maxIter;

	////////////

	dvec2 z = dvec2(fragTexCoord) * ubo.zoom + ubo.pos;
	dvec2 p = dvec2(0.);
	float radius2 = (radius*radius);

//	outColor = vec4( sin(vec3(iter)) * .5 + .5, 1.0);  
	int i = maxIter;
	if( ubo.play == 0) {
		for (int j=0; j <= maxIter; j++) {
			p = imAdd(imSquare(p), z);
			if(magnitudeSquaredFast(p) > radius2) {
				i = j;
				break;
			}
		}
	}else {
		for (int j=0; j <= maxIter; j++) {
			p = imAdd(imSquare(p), z);
			p.x += p.x * ubo.play / float(j+1) / p.y / 50.;
			if(magnitudeSquaredFast(p) > radius2) {
				i = j;
				break;
			}
		}
	}
	
    // Smoothing
    float log_zn = log(magnitudeSquaredFast(p)) * 0.5;
    float nu = log(log_zn * 1.44269504088896) * 1.44269504088896 * smoothing;

    float v = (float(i + 1) - nu) * 0.02; // + zx.x * 10.0;
    outColor = (i >= (maxIter - 1))
                    ? vec4(0.0)
                    : makeColors(v);
}

