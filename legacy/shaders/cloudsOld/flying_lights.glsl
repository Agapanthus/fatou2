#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

// Some useful functions
vec3 mod289(vec3 x){return x-floor(x*(1./289.))*289.;}
vec2 mod289(vec2 x){return x-floor(x*(1./289.))*289.;}
vec3 permute(vec3 x){return mod289(((x*34.)+1.)*x);}

//
// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
float snoise(vec2 v){
    
    // Precompute values for skewed triangular grid
    const vec4 C=vec4(.211324865405187,
        // (3.0-sqrt(3.0))/6.0
        .366025403784439,
        // 0.5*(sqrt(3.0)-1.0)
        -.577350269189626,
        // -1.0 + 2.0 * C.x
    .024390243902439);
    // 1.0 / 41.0
    
    // First corner (x0)
    vec2 i=floor(v+dot(v,C.yy));
    vec2 x0=v-i+dot(i,C.xx);
    
    // Other two corners (x1, x2)
    vec2 i1=vec2(0.);
    i1=(x0.x>x0.y)?vec2(1.,0.):vec2(0.,1.);
    vec2 x1=x0.xy+C.xx-i1;
    vec2 x2=x0.xy+C.zz;
    
    // Do some permutations to avoid
    // truncation effects in permutation
    i=mod289(i);
    vec3 p=permute(
        permute(i.y+vec3(0.,i1.y,1.))
        +i.x+vec3(0.,i1.x,1.)
    );
    
    vec3 m=max(.5-vec3(
            dot(x0,x0),
            dot(x1,x1),
            dot(x2,x2)
        ),0.
    );
    
    m=m*m;
    m=m*m;
    
    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)
    
    vec3 x=2.*fract(p*C.www)-1.;
    vec3 h=abs(x)-.5;
    vec3 ox=floor(x+.5);
    vec3 a0=x-ox;
    
    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m*=1.79284291400159-.85373472095314*(a0*a0+h*h);
    
    // Compute final noise value at P
    vec3 g=vec3(0.);
    g.x=a0.x*x0.x+h.x*x0.y;
    g.yz=a0.yz*vec2(x1.x,x2.x)+h.yz*vec2(x1.y,x2.y);
    return 130.*dot(m,g);
}

vec2 random2(vec2 p){
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

float voronoi(vec2 st,float turb){
    
    // Tile the space
    vec2 i_st=floor(st);
    vec2 f_st=fract(st);
    
    float m_dist=1.;// minimun distance
    
    for(int y=-1;y<=1;y++){
        for(int x=-1;x<=1;x++){
            // Neighbor place in the grid
            vec2 neighbor=vec2(float(x),float(y));
            
            // Random position from current + neighbor place in the grid
            vec2 point=random2(i_st+neighbor);
            
            // Animate the point
            point=.5+.5*sin(u_time*turb+6.2831*point);
            
            // Vector between the pixel and the point
            vec2 diff=neighbor+point-f_st;
            
            // Distance to the point
            float dist=length(diff);
            
            // Keep the closer distance
            m_dist=min(m_dist,dist);
        }
    }
    
    return m_dist;
}

float sigmoid(float x){
    return 2./(1.+exp2(-x))-1.;
}

float weibull(float k,float x,float l){
    if(x>=.0)
    return l*k*pow(l*x,k-1.)*exp(-pow(l*x,k));
    else
    return l*k*pow(-l*x,k-1.)*exp(-pow(-l*x,k));
}

float voronoiAsy(vec2 st,float turb,float asy){
    
    float ellipse=1.;
    
    // Tile the space
    vec2 i_st=floor(st);
    vec2 f_st=fract(st);
    
    float m_dist=1.;// minimun distance
    
    for(int y=-1;y<=1;y++){
        for(int x=-1;x<=1;x++){
            // Neighbor place in the grid
            vec2 neighbor=vec2(float(x),float(y));
            
            // Random position from current + neighbor place in the grid
            vec2 point=random2(i_st+neighbor);
            
            // Animate the point
            point=.5+.5*sin(u_time*turb+6.2831*point);
            
            // Vector between the pixel and the point
            vec2 diff=neighbor+point-f_st;
            
            diff.x*=ellipse;
            
            //diff.x=weibull(1.,diff.x,1.);
            if(diff.y>.0){
                diff.y*=asy;
            }
            //2./(1. + exp2(-diff.x)) - 1.;
            
            // Distance to the point
            float dist=sqrt(diff.x*diff.x+diff.y*diff.y);
            
            // Keep the closer distance
            m_dist=min(m_dist,dist);
        }
    }
    
    return m_dist;
}

vec3 skyColor() {

    float whitePart=.3;
    vec3 color=mix(
        vec3(0.4118, 0.4431, 0.7255),
        mix(
            vec3(.1294,.4745,.6745),
            vec3(0.0627, 0.2588, 0.3725),
            gl_FragCoord.y/u_resolution.y*(1.-whitePart)+whitePart
        ),
        min(1.,(1./whitePart)*gl_FragCoord.y/u_resolution.y)
    );
    return color;
}

void main(){
    vec2 st=gl_FragCoord.xy/u_resolution.xy;
    st.x*=u_resolution.x/u_resolution.y;
    
    // seed + time
    float seed=u_time*.03;
    st.x+=seed;
    vec2 seedX=vec2(seed,0);
    
    float wolkigkeit=.0;// (u_mouse.y/u_resolution.y)*2. - 1.;
    
    // Scale everything
    st*=1.;
    
    vec3 color = skyColor();


    float cloud = -(
        // Flying lights
        + voronoi(st * 11. - seed*3., .1) * 0.4 / voronoi(st * 16. - seed*4., -1.) * -0.1
       );
    cloud = cloud * .6 + 0.;
        
    if(cloud > 0.) {
        gl_FragColor = vec4(color*(1.-cloud) + vec3(cloud) , 1.0);
    } else {
    	gl_FragColor = vec4(color,1.0);
    }
}
