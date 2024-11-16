// Author: @patriciogv
// Title: CellularNoise

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

float voronoi(vec2 st, float turb) {
    
    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);

    float m_dist = 1.;  // minimun distance

    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(x),float(y));

            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);

			// Animate the point
            point = 0.5 + 0.5*sin(u_time*turb + 6.2831*point);

			// Vector between the pixel and the point
            vec2 diff = neighbor + point - f_st;

            // Distance to the point
            float dist = length(diff);

            // Keep the closer distance
            m_dist = min(m_dist, dist);
        }
    }

    return m_dist;
}

void main() {
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    st.x *= u_resolution.x/u_resolution.y;
   
     // seed + time
    float seed = u_time*0.03;
    st.x += seed;
    
    // Scale everything
	st *= 1.;
    
    vec3 color = vec3(0.303,0.691,0.915);


    float cloud = -(voronoi(st * 4., .1) * 1.0
        + voronoi(st * 8. + seed*1., .3) * -0.5
        + voronoi(st * 11. - seed*3., .1) * 0.4
        + voronoi(st * 16. - seed*4., -.45) * -0.1
        + voronoi(st * 32. + seed*5., .9) * 0.1
        + voronoi(st * 60. + seed*9., -2.) * -0.05
       );
    cloud = cloud *1.0 + 0.8;
        
    if(cloud > 0.) {
        gl_FragColor = vec4(color*(1.-cloud) + vec3(cloud) , 1.0);
    } else {
    	gl_FragColor = vec4(color,1.0);
    }

    //gl_FragColor = vec4(vec3(cloud),1.0); 
}
