#include "types64.fs"
#include "fence.fs"

// Exports: add, sub, mul, square, expand, lt

// TODO: For div, sqrt, log, sin, cos and quad: see
// http://andrewthall.org/papers/df64_qf128.pdf

///////////////////////////////

/*
// Split the p-bit Float a into two non-overlapping values a = hi + lo:
// hi with (p - s) bits
// lo with (s - 1) bits
// where p/2 <= s <= p-1 is the splitting point
Real64 splitDekker(Real32 a, lowp int s) {
    Real32 c = float((1 << s) + 1) * a;
    Real32 big = c - a;
    Real32 hi = c - big;
    Real32 lo = a - hi;
    return vec2(hi, lo);
}

const Real64 SPLITTER = (1 << 29) + 1;

// Andrew Thall
Real64 splitThall(Real64 a) {
    Real64 t = a * SPLITTER;
    Real64 hi = t - Real64(t - a);
    Real64 lo = a - hi;
    return vec2(float(hi), float(lo));
}*/

// Andrew Thall, Dekker
// (Fast) Dekker-Sum
// REQUIRES: |a| >= |b|
// This is a non-overlapping non-adjacent expansion x + y = a + b
Real64 _quickTwoSum(Real32 a, Real32 b) {
    // We need frc to prevent optimizations destroying our function!
    Real32 s = add_fence(a, b);
    Real32 v = sub_fence(s, a);
    Real32 e = b - v;
    return vec2(s, e);
}

// Andrew Thall, Knuth, modified by Eric Skaliks
// Knuth-Sum
// This is a non-overlapping non-adjacent expansion x + y = a + b
Real64 _twoSum(Real32 a, Real32 b) {
    // We need frc to prevent optimizations destroying our function!
    Real32 s = add_fence(a, b);
    Real32 v = sub_fence(s, a);
    Real32 e = (a - (s - v)) + (b - v);
    return vec2(s, e);
}

// Andrew Thall, modified by Eric Skaliks
Complex64 _twoSum2(Complex32 a, Complex32 b) {
    // TODO: use the "precise" qualifier instead of fences
    Real64 s = add2_fence(a, b);
    Real64 v = sub2_fence(s, a);
    Real64 e = (a - (s - v)) + (b - v);
    Complex64 res;
    res.xz = s;
    res.yw = e;
    return res;
}

// Andrew Thall
Real64 add(Real64 a, Real64 b) {
    Complex64 st;
    st = _twoSum2(a, b);
    st.y = st.y + st.z;
    st.xy = _quickTwoSum(st.x, st.y);
    st.y = st.y + st.w;
    st.xy = _quickTwoSum(st.x, st.y);
    return st.xy;
}

Real64 sub(Real64 a, Real64 b) { return add(a, -b); }

// Andrew Thall, modified by Eric Skaliks
Real64 _split(Real32 a) {
    const Real32 SPLIT = 4097.; // (1 << 12) + 1
    Real32 t = a * SPLIT;
    Real32 hi = t - sub_fence(t, a);
    Real32 lo = a - hi;
    return vec2(hi, lo);
}

// Andrew Thall, modified by Eric Skaliks
Complex64 _split2(Complex32 a) {
    const Real32 SPLIT = 4097.; // (1 << 12) + 1
    Complex32 t = a * SPLIT;
    Complex32 hi = t - sub2_fence(t, a);
    Complex32 lo = a - hi;
    Complex64 res;
    res.xz = hi;
    res.yw = lo;
    return res;
}

// Andrew Thall, modified by Eric Skaliks
Real64 _twoProd(Real32 a, Real32 b) {
    Real32 p = mul_fence(a, b);
    Complex64 s = _split2(vec2(a, b));
    Real32 err = ((s.x * s.z - p) + s.x * s.w + s.y * s.z) + s.y * s.w;
    return vec2(p, err);
}

/*
// Andrew Thall
Real64 twoProdFMA(Real32 a, Real32 b) {
    Real32 x = a * b;
    Real32 y = fma(a, b, x);
    return vec2(x, y);
}*/

// Andrew Thall, modified by Eric Skaliks
Real64 mul(Real64 a, Real64 b) {
    Real64 p = _twoProd(a.x, b.x);
    p.y = p.y + a.x * b.y + a.y * b.x;
    return _quickTwoSum(p.x, p.y);
}

// Eric Skaliks
Real64 _twoProdSquare(Real32 a) {
    Real32 p = mul_fence(a, a);
    Real64 s = _split(a);
    Real32 err = ((s.x * s.x - p) + 2. * s.x * s.y) + s.y * s.y;
    return vec2(p, err);
}

// Eric Skaliks
Real64 square(Real64 a) {
    Real64 p = _twoProdSquare(a.x);
    p.y = p.y + 2. * a.x * a.y;
    return _quickTwoSum(p.x, p.y);
}

bool lt(Real64 a, Real64 b) { return (a.x < b.x || (a.x == b.x && a.y < b.y)); }

Real64 expand(Real32 a) { return vec2(a, 0.0); }

Complex imExpand(Complex32 hi, Complex32 lo) {
    return MkComplex(vec2(hi.x, lo.x), vec2(hi.y, lo.y));
}