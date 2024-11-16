#include "types.fs"
#include "typesUInt.fs"

UInt64 u64Add(UInt64 a, UInt64 b) {
    UInt64 res;

    res.lo = a.lo + b.lo;
    res.hi = a.hi + b.hi;

    if (res.lo < a.lo || res.lo < b.lo) {
        res.hi++;
    }

    return res;
}

UInt64 u64Sub(UInt64 a, UInt64 b) {
    UInt64 res;

    res.lo = a.lo - b.lo;
    res.hi = a.hi - b.hi;

    if (res.lo > a.lo || res.lo > b.lo) {
        res.hi--;
    }

    return res;
}

// (k*ah + al) * (k*bh + bl) = k*k*ah*bh + k*(ah*bl + al*bh) + al*bl
UInt64 u64Mul32(UInt32 a, UInt32 b) {
    UInt64 res;

    UInt16 ah = a >> 16;
    UInt16 al = a & 0xFFFFU;
    UInt16 bh = b >> 16;
    UInt16 bl = b & 0xFFFFU;

    res.lo = al * bl;
    res.hi = ah * bh;
    UInt32 rm1 = ah * bl;
    UInt32 rm2 = al * bh;

    UInt32 rmlc = (rm1 & 0xFFFFU) + (rm2 & 0xFFFFU);
    res.lo += rmlc << 16;

    UInt32 rmh = (rm1 >> 16) + (rm2 >> 16);
    res.hi += rmh;

    // carry
    if ((rmlc & 0xFFFF0000U) != 0U)
        res.hi++;

    return res;
}

UInt64 u64Square32(UInt32 a) {
    UInt64 res;

    UInt16 ah = a >> 16;
    UInt16 al = a & 0xFFFFU;

    res.lo = al * al;
    res.hi = ah * ah;
    UInt32 rm = ah * al;

    UInt32 rmlc = 2U * (rm & 0xFFFFU);
    res.lo += rmlc << 16;

    UInt32 rmh = rm >> 15;
    res.hi += rmh;

    // carry
    if ((rmlc & 0xFFFF0000U) != 0U)
        res.hi++;

    return res;
}

// (k*ah + al) * (k*bh + bl) = k*k*ah*bh + k*(ah*bl + al*bh) + al*bl
// but we will discard most, so we just have:
// lower(k*(ah*bl + al*bh)) + al*bl
UInt64 u64Mul(UInt64 a, UInt64 b) {
    UInt64 res;

    UInt32 ah = a.hi;
    UInt32 al = a.lo;
    UInt32 bh = b.hi;
    UInt32 bl = b.lo;

    res = u64Mul32(al, bl);

    // discard high
    UInt64 rm1 = u64Mul32(ah, bl);
    UInt32 rm1l = rm1.lo;

    // discard high
    UInt64 rm2 = u64Mul32(al, bh);
    UInt32 rm2l = rm2.lo;

    // discard carry
    UInt32 rml = rm1l + rm2l;
    res.hi += rml;

    return res;
}

UInt64 u64Square(UInt64 a) {
    UInt64 res;

    UInt32 ah = a.hi;
    UInt32 al = a.lo;
    res = u64Square32(al);

    // discard high
    UInt64 rm = u64Mul32(ah, al);

    // discard carry
    res.hi += 2U * rm.lo;

    return res;
}

UInt64 u64Expand(UInt32 hi, UInt32 lo) {
    UInt64 res;
    res.hi = hi;
    res.lo = lo;
    return res;
}

UInt64 u64Expand(UInt32 x) { return u64Expand(0U, x); }

UInt64 u64ExpandHi(Real32 x) {
    UInt64 res;
    res.hi = MkUInt32(abs(x));
    res.lo = MkUInt32(MkReal32(0xffffffffU) * fract(abs(x)));
    if (x < 0.) {
        res = u64Sub(u64Expand(0U), res);
    }

    return res;
}

UInt64 u64Expand(Real32 x) {
    UInt64 res;
    res.hi = MkUInt32(abs(x) / MkReal32(0xffffffffU));
    res.lo = MkUInt32(abs(x));
    if (x < 0.) {
        res = u64Sub(u64Expand(0U), res);
    }

    return res;
}
