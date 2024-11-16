#include "types32.fs"

// Exports: add, sub, mul, square, expand, lt

#define add(A,B) ((A)+(B))
#define sub(A,B) ((A)-(B))
#define mul(A,B) ((A)*(B))
#define lt(A,B) ((A)<(B))
#define expand(A) A

Real32 square(Real32 x) {
    return x*x;
}

Complex imExpand(Complex32 hi, Complex32 lo) {
    return MkComplex(hi.x, hi.y);
}