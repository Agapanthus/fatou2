#include "types.fs"

// This is a _fence. The optimizer can't relate things before and after.
Real32 _fence(Real32 a, Real32 b, Real32 ab) {
    // return mix(a, ab, b != 0. ? 1. : 0.);

    if (b == 0.)
        return a;
    else
        return ab;   
}

Complex32 _fence(Complex32 a, Complex32 b, Complex32 ab) {
    // if (b.x == 0.) return a; return ab;
    return mix(a, ab, b != vec2(0.) ? 1. : 0.);
    // if (b == vec2(0.)) return a; return ab;
    // return ab;
}

Real32 mul_fence(Real32 a, Real32 b) { return _fence(0., b, a * b); }

Real32 add_fence(Real32 a, Real32 b) { return _fence(a, b, a + b); }

Real32 sub_fence(Real32 a, Real32 b) { return _fence(a, b, a - b); }

Complex32 add2_fence(Complex32 a, Complex32 b) { return _fence(a, b, a + b); }

Complex32 sub2_fence(Complex32 a, Complex32 b) { return _fence(a, b, a - b); }
