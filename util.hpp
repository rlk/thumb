#ifndef UTIL_HPP
#define UTIL_HPP

#include <math.h>

//-----------------------------------------------------------------------------

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(n, a, b) MIN(MAX(n, a), b)
#endif

//-----------------------------------------------------------------------------

float cosi(int);
float sini(int);

//-----------------------------------------------------------------------------

#endif
