#include "fence.fs"
#include "types128.fs"

// Exports: add, sub, mul, square, expand, lt

// Based on Andrew Thall's paper: http://andrewthall.org/papers/df64_qf128.pdf
// ...and Yozo Hida et al's paper: https://www.davidhbailey.com/dhbpapers/quad-double.pdf
// These numbers use 128 bits but have only 96 bits of precision!

Real32 _quickTwoSum(Real32 x, Real32 y, Real32 z) {
    // TODO
    return 0.;
}

// This renormalization is a vriant of Priest's renormalization
// The input is a five-term expansion with limited overlaping bits, with a0
// being the most significant component
Real128 _renormalize128(Real32 a0, Real32 a1, Real32 a2, Real32 a3, Real32 a4) {
    Real32 t0, t1, t2, t3, t4;
    Real32 s0, s1, s2, s3;
    s0 = s1 = s2 = s3 = 0.;

    Real32 s;
    s = _quickTwoSum(a3, a4, t4);
    s = _quickTwoSum(a2, s, t3);
    s = _quickTwoSum(a1, s, t2);
    t0 = _quickTwoSum(a0, s, t1);
    s0 = _quickTwoSum(t0, t1, s1);

    if (s1 != 0.) {
        s1 = _quickTwoSum(s1, t2, s2);
        if (s2 != 0.) {
            s2 = _quickTwoSum(s1, t2, s2);
            if (s3 != 0.) {
                s3 += t4;
            } else {
                s2 += t4;
            }
        } else {
            s1 = _quickTwoSum(s1, t3, s2);
            if (s2 != 0.) {
                s2 = _quickTwoSum(s2, t4, s3);
            } else {
                s1 = _quickTwoSum(s1, t4, s2);
            }
        }
    } else {
        s0 = _quickTwoSum(s0, t2, s1);
        if (s1 != 0.) {
            s1 = _quickTwoSum(s1, t3, s2);
            if (s2 != 0.) {
                s2 = _quickTwoSum(s2, t4, s3);
            } else {
                s1 = _quickTwoSum(s1, t4, s2);
            }
        } else {
            s0 = _quickTwoSum(s0, t3, s1);
            if (s1 != 0.) {
                s1 = _quickTwoSum(s1, t4, s2);
            } else {
                s0 = _quickTwoSum(s0, t4, s1);
            }
        }
    }

    return MkReal128(s0, s1, s2, s3);
}