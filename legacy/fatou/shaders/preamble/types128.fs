#include "types.fs"

#define Complex Complex64
#define Real Real64
#define MkComplex MkComplex64
#define toReal32(VAR) ((VAR).x + (VAR).y)
#define re(VAR) ((VAR).xy)
#define im(VAR) ((VAR).zw)
