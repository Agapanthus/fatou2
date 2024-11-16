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

vec2 random2(vec2 p){
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
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

// TODO: Smoothness ist stufig wenn !=8.0
float smoothVoronoi(vec2 x,float smoothness)
{
    vec2 p=floor(x);
    vec2 f=fract(x);
    
    float res=0.;
    for(int j=-1;j<=1;j++)
    for(int i=-1;i<=1;i++)
    {
        vec2 b=vec2(float(i),float(j));
        vec2 r=vec2(b)-f+random2(p+b);
        float d=dot(r,r);
        
        res+=1./pow(d,smoothness);
    }
    return pow(1./res,1./2./smoothness);
}

// Smoothness sollte >= 12.0 sein, damit stufen vermieden werden!
float smoothVoronoi2(vec2 x,float smoothness)
{
    vec2 p=floor(x);
    vec2 f=fract(x);
    
    float res=0.;
    for(int j=-1;j<=1;j++)
    for(int i=-1;i<=1;i++)
    {
        vec2 b=vec2(float(i),float(j));
        vec2 r=vec2(b)-f+random2(p+b);
        float d=length(r);
        
        res+=exp(-smoothness*d);
    }
    return-(1./smoothness)*log(res);
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

void main(){
    vec2 st=gl_FragCoord.xy/u_resolution.xy;
    st.x*=u_resolution.x/u_resolution.y;
    
    // seed + time
    float seed=u_time*.03;
    st.x+=seed;
    vec2 seedX=vec2(seed,0);
    
    if(st.y<.5){
        gl_FragColor=vec4(vec3(voronoiAsy(st*5.+seedX*.4,-.2,1.)),1.);
    }else{
        gl_FragColor=vec4(vec3(voronoiSmoothAsy(st*5.+seedX*.4,-.2,1.,12.)),1.);
    }
}
