#include "types.fs"

#define Complex Complex32
#define Real Real32
#define MkComplex MkComplex32
#define toReal32(VAR) (VAR)
#define re(VAR) ((VAR).x)
#define im(VAR) ((VAR).y)
