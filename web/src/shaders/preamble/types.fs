#define Int8 lowp int
#define Int16 mediump int
#define Int32 highp int
struct Int64 {
    Int32 hi;
    Int32 lo;
};

#define MkInt8 int
#define MkInt16 int
#define MkInt32 int

#define Index Int32
#define MkIndex int

#define Real8 lowp float
#define Real16 mediump float
#define Real32 highp float
#define Real64 highp vec2
#define Real128 highp vec4

#define MkReal8 float
#define MkReal16 float
#define MkReal32 float
#define MkReal64 vec2
#define MkReal128 vec4

#define Complex8 lowp vec2
#define Complex16 mediump vec2
#define Complex32 highp vec2
#define Complex64 highp vec4
struct Complex128 {
    Real128 r;
    Real128 i;
};

#define MkComplex8 vec2
#define MkComplex16 vec2
#define MkComplex32 vec2
#define MkComplex64 vec4
Complex128 MkComplex128(Real128 r, Real128 i) {
    Complex128 res;
    res.r = r;
    res.i = i;
    return res;
}

#define RGB vec3
#define RGBA vec4