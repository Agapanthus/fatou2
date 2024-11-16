#include "arith-float{{precision}}.fs"

Complex imMul(Complex a, Complex b) {
    return MkComplex(sub(mul(re(a), re(b)), mul(im(a), im(b))),
                          add(mul(re(a), im(b)), mul(im(a), re(b))));
}

Complex imAdd(Complex a, Complex b) {
    return MkComplex(add(re(a), re(b)), add(im(a), im(b)));
}

Real magnitudeSquared(Complex a) { return add(square(re(a)), square(im(a))); }

Real32 magnitudeSquaredFast(Complex a) {
    Real32 real = toReal32(re(a));
    Real32 imaginary = toReal32(im(a));
    return (real * real + imaginary * imaginary);
}

Complex imExpand(Complex32 hi) {
    return imExpand(hi, MkComplex32(0.));
}

Complex imMul(Complex a, Real b) {
    return MkComplex(mul(re(a), b), mul(im(a), b));
}

Complex imSquare(Complex a) {
    // this is a bit inexact, but faster
    return MkComplex(sub(square(re(a)), square(im(a))),
                          2. * mul(re(a), im(a)));
}

Complex getComplexStart(Complex zoom, Complex pos, Complex32 TexCoords) {
    Complex tc = imExpand(vec2(TexCoords[0], TexCoords[1]) - vec2(.5));

    return imAdd(MkComplex(mul(re(tc), re(zoom)), mul(im(tc), im(zoom))),
                 pos);
}