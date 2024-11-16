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

float voronoiSmoothAsy(vec2 st,float turb,float asy,float smoothness){
    
    float ellipse=1.;
    
    // Tile the space
    vec2 i_st=floor(st);
    vec2 f_st=fract(st);
    
    float m_dist=0.;
    
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
            float dist=length(diff);
            
            // Keep the smooth distance
            m_dist+=exp(-smoothness*dist);
        }
    }
    
    return-(1./smoothness)*log(m_dist);
}
float sigmoid(float x){
    return 2./(1.+exp2(-x))-1.;
}

float voronoiAsyLumo(vec2 st,float turb,float asy,float smoothness,vec2 lightDir){
    
    float ellipse=1.;
    
    // Tile the space
    vec2 i_st=floor(st);
    vec2 f_st=fract(st);
    
    float m_lumo=0.;// minimun luminosity
    float m_dist = 0.;
    vec2 m_v=vec2(0.);
    
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
            
            if(diff.y>.0){
                diff.y*=asy;
            }
            
            float lumo=max(dot(diff,lightDir),0.);
            
            float l_dist = exp(-smoothness*length(diff));
            m_dist+=l_dist;

            // Keep the smooth distance
            m_lumo+=lumo * l_dist; //exp(-smoothness*lumo);
        }
    }
        
        
    //float totalLum = -(1./smoothness)*log(m_dist);

    
    return m_lumo / m_dist * 1.0; // -(1./smoothness)*log(m_lumo);
    //return m_v * 200.0;
}

vec3 skyColor(){
    
    float whitePart=.3;
    vec3 color=mix(
        vec3(.6784,.902,.9608),
        mix(
            vec3(.3373,.7412,.9725),
            vec3(.1294,.4745,.6745),
            gl_FragCoord.y/u_resolution.y*(1.-whitePart)+whitePart
        ),
        min(1.,(1./whitePart)*gl_FragCoord.y/u_resolution.y)
    );
    return color;
}

float pow2(float x){
    return x*x;
}
float pow3(float x){
    return x*x*x;
}
float ln(float x){
    return log(x);
}
// Bubble of height 1 and length 1
float footBubble(float x){
    
    x=(.9+(pow2(4.*(x*10.+3.))*ln(x*10.+1.))/pow3(x*10.+3.)-(x*10.+3.)*.3)/2.95;
    
    return x;
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
    
    vec3 color=skyColor();
    
    float cloudMinHeight=.2;
    float cloudLayerStrength=1.;
    float cloudDensity=footBubble((gl_FragCoord.y/u_resolution.y-cloudMinHeight)/cloudLayerStrength);
    
    float cloud=0.;
    float lightAngle=-seed*9.;
    float shadowAsy = 1.;
    float shadowSmooth = 12.0;
    float shadowDark = 1.4;
    vec2 lightDir=vec2(cos(lightAngle),sin(lightAngle));
    
    float lumi=voronoiAsyLumo(st*vec2(4.,4.)-seedX*0.,.0,shadowAsy,shadowSmooth,lightDir);
    //float lumi=max(dot(dist,lightDir),0.);
    float tlumi=lumi;
    
    lumi=voronoiAsyLumo(st*9.-seedX*.2,.4,1., shadowSmooth,lightDir)*.6;
    tlumi=tlumi+.5*lumi;
    
    lumi=voronoiAsyLumo(st*16.-seedX*.3-seed*1.,-.2,1., shadowSmooth,lightDir)*.6;
    tlumi=tlumi+.25*lumi;
    tlumi *= shadowDark;


    if( sqrt(pow2(gl_FragCoord.x/u_resolution.x- 0.05) + pow2(gl_FragCoord.y/u_resolution.y -0.05)) < 0.025){
        float lumo=max(dot(gl_FragCoord.xy/u_resolution.xy-vec2(0.05),lightDir),0.);
        gl_FragColor = vec4(mix(vec3(0.7), vec3(1.0), (lumo*50.)),1.0);

    } else if(gl_FragCoord.x/u_resolution.x * (1.-gl_FragCoord.y/u_resolution.y)>.45){
        cloud=1.+tlumi;
        
        if(cloud>1.){
            gl_FragColor=vec4(vec3(2.-cloud)+vec3(cloud-1.)*.7,1.);
        }else if(cloud>0.){
            gl_FragColor=vec4(color*(1.-cloud)+vec3(cloud),1.);
        }else{
            gl_FragColor=vec4(color,1.);
        }
    }else{
        float smoothness=42.;
        cloud=-(
            // Richtige Wolken
            // (snoise(st * vec2(4.,2.)) + wolkigkeit)*.5
            (voronoiSmoothAsy(st*vec2(4.,4.)-seedX*0.,.0,6.,smoothness)*1.3+.01-.3*cloudDensity)
            
            // Schäfchen, die sich langsam ineinander umformen
            +voronoiSmoothAsy(st*9.-seedX*.2,.4,1.,smoothness)*.6
            +voronoiSmoothAsy(st*16.-seedX*.3-seed*1.,-.2,1.,smoothness)*.3
            
            // Feine Strukturen
            +snoise(st*32.+seedX*.2)*.03
            +snoise(st*64.+seedX*.3)*.015
            +voronoiSmoothAsy(st*32.+seedX*.1,.1,1.,smoothness)*.12
            +voronoiSmoothAsy(st*60.+seedX*.4,-.2,1.,smoothness)*-.04
        );
        cloud=cloud*.9+.3+cloudDensity*.8;
        
        if(cloud>0.){
            gl_FragColor=vec4(mix(color,mix(vec3(1.),vec3(.3),tlumi),cloud),1.);
        }else{
            gl_FragColor=vec4(color,1.);
        }
        
    }
}
