// Based on
// https://github.com/alexozer/glsl-arb-prec/blob/master/arb-precision.glsl

#define CELL_TYPE highp int

// integers per arbitrary-precision number
#define VALS 3  //{{VALS}}; // ints per value


#define LEN_PROD ((VALS - 1) * 2 + 1) 
#define CLIP (LEN_PROD - VALS)

/* power of 10 one larger than maximum value per int
   A value of 10000 seems to work the best
   */
#define LIMIT 10000000 //(1 << 30) // {{LIMIT}};

const highp float limitFlt = float(LIMIT);

CELL_TYPE result[VALS];

#define intZero(x, len)                                                        \
    for (lowp int i = 0; i < len; i++) {                                       \
        x[i] = 0;                                                              \
    }
#define intAssign(x, y)                                                        \
    for (lowp int i = 0; i < VALS; i++) {                                      \
        x[i] = y[i];                                                           \
    }
#define intNegate(x)                                                           \
    for (lowp int i = 0; i < VALS; i++) {                                      \
        x[i] = -x[i];                                                          \
    }

bool intSignp(CELL_TYPE[VALS] a) { return (a[VALS - 1] >= 0); }

CELL_TYPE _keepVal, _carry;

void _roundOff(CELL_TYPE x) {
    _carry = x / LIMIT;
    _keepVal = x - _carry * LIMIT;
}

void intAdd(CELL_TYPE[VALS] a, CELL_TYPE[VALS] b) {
    bool s1 = intSignp(a), s2 = intSignp(b);

    _carry = 0;

    for (lowp int i = 0; i < VALS - 1; i++) {
        _roundOff(a[i] + b[i] + _carry);

        if (_keepVal < 0) {
            _keepVal += LIMIT;
            _carry--;
        }

        result[i] = _keepVal;
    }
    _roundOff(a[VALS - 1] + b[VALS - 1] + _carry);
    result[VALS - 1] = _keepVal;

    if (s1 != s2 && !intSignp(result)) {
        intNegate(result);

        _carry = 0;

        for (lowp int i = 0; i < VALS; i++) {
            _roundOff(result[i] + _carry);

            if (_keepVal < 0) {
                _keepVal += LIMIT;
                _carry--;
            }

            result[i] = _keepVal;
        }

        intNegate(result);
    }
}

void intMul(CELL_TYPE[VALS] a, CELL_TYPE[VALS] b) {
    bool tointNegate = false;

    if (!intSignp(a)) {
        intNegate(a);
        tointNegate = !tointNegate;
    }
    if (!intSignp(b)) {
        intNegate(b);
        tointNegate = !tointNegate;
    }

    CELL_TYPE prod[LEN_PROD];
    intZero(prod, LEN_PROD);

    for (lowp int i = 0; i < VALS; i++) {
        for (lowp int j = 0; j < VALS; j++) {
            prod[i + j] += a[i] * b[j];
        }
    }

    _carry = 0;
    for (lowp int i = 0; i < CLIP; i++) {
        _roundOff(prod[i] + _carry);
        prod[i] = _keepVal;
    }

    if (prod[CLIP - 1] >= LIMIT / 2) {
        _carry++;
    }

    for (lowp int i = CLIP; i < LEN_PROD; i++) {
        _roundOff(prod[i] + _carry);
        prod[i] = _keepVal;
    }

    for (lowp int i = 0; i < LEN_PROD - CLIP; i++) {
        result[i] = prod[i + CLIP];
    }

    if (tointNegate) {
        intNegate(result);
    }
}

void loadFloat(highp float f) {
    for (lowp int i = VALS - 1; i >= 0; i--) {
        CELL_TYPE fCurr = int(f);
        result[i] = fCurr;
        f -= float(fCurr);
        f *= limitFlt;
    }
}