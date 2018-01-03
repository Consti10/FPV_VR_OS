// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#ifndef SYMPHONY_MATH_H
#define SYMPHONY_MATH_H

#if !defined(_MSC_VER)

#include <math.h>

#if __hexagon__
#define SYMPHONY_M_PI 3.14159265358979323846
#else

#define SYMPHONY_M_PI M_PI
#endif

#define SYMPHONY_M_PI_F 3.14159274101257f

static inline
double
acospi(double x)
{
  return acos(x) / SYMPHONY_M_PI;
}

static inline
double
asinpi(double x)
{
  return asin(x) / SYMPHONY_M_PI;
}

static inline
double
atanpi(double x)
{
  return atan(x) / SYMPHONY_M_PI;
}

static inline
double
atan2pi(double x, double y)
{
  return atan2(x, y) / SYMPHONY_M_PI;
}

#ifndef __hexagon__
static inline
float
cbrt(float x)
{
  return cbrtf(x);
}

static inline
float
ceil(float x)
{
  return ceilf(x);
}

static inline
float
copysign(float x, float y)
{
  return copysignf(x, y);
}

static inline
float
cos(float x)
{
  return cosf(x);
}

static inline
float
cosh(float x)
{
  return coshf(x);
}
#endif

static inline
float
#ifdef __hexagon__
cospif(float x)
#else
cospi(float x)
#endif
{
  return cos(x * SYMPHONY_M_PI_F);
}

static inline
double
cospi(double x)
{
  return cos(x * SYMPHONY_M_PI);
}

#ifndef __hexagon__
static inline
float
erfc(float x)
{
  return erfcf(x);
}

static inline
float
erf(float x)
{
  return erff(x);
}

static inline
float
exp(float x)
{
  return expf(x);
}

static inline
float
exp2(float x)
{
  return exp2f(x);
}
#endif

static inline
float
exp10(float x)
{

  return exp(x * 2.3025851f);
}

#ifndef __hexagon__
static inline
float
expm1(float x)
{
  return expm1f(x);
}

static inline
float
fabs(float x)
{
  return fabsf(x);
}

static inline
float
fdim(float x, float y)
{
  return fdimf(x, y);
}

static inline
float
floor(float x)
{
  return floorf(x);
}

static inline
float
fma(float x, float y, float z)
{
  return fmaf(x, y, z);
}

static inline
float
fmax(float x, float y)
{
  return fmaxf(x, y);
}

static inline
float
fmin(float x, float y)
{
  return fminf(x, y);
}

static inline
float
fmod(float x, float y)
{
  return fmodf(x, y);
}
#endif

static inline
float
#ifdef __hexagon__
fractf(float x, float *iptr)
#else
fract(float x, float *iptr)
#endif
{
  *iptr = floor(x);
  return fmin(x - *iptr, 0x1.fffffep-1f);
}

static inline
double
fract(double x, double *iptr)
{
  *iptr = floor(x);
  return fmin(x - *iptr, 0x1.fffffep-1);
}

#ifndef __hexagon__
static inline
float
frexp(float x, int *exp)
{
  return frexpf(x, exp);
}

static inline
float
hypot(float x, float y)
{
  return hypotf(x, y);
}

static inline
float
ldexp(float x, int n)
{
  return ldexpf(x, n);
}

static inline
float
lgamma(float x)
{
  return lgammaf(x);
}
#endif

#if 0

static inline
float
lgamma_r(float x, int* signp)
{
  return lgammaf_r(x, signp);
}
#endif

#ifndef __hexagon__
static inline
float
log(float x)
{
  return logf(x);
}

static inline
float
log2(float x)
{
  return log2f(x);
}

static inline
float
log10(float x)
{
  return log10f(x);
}
#endif

#ifndef __hexagon__
static inline
float
log1p(float x)
{
  return log1pf(x);
}

static inline
float
logb(float x)
{
  return logbf(x);
}
#endif

static inline
float
madf(float x, float y, float z)
{
  return x * y + z;
}

static inline
double
mad(double x, double y, double z)
{
  return x * y + z;
}

static inline
float
maxmagf(float x, float y)
{
  return fmax(fabs(x), fabs(y));
}

static inline
double
maxmag(double x, double y)
{
  return fmax(fabs(x), fabs(y));
}

static inline
float
minmagf(float x, float y)
{
  return fmin(fabs(x), fabs(y));
}

static inline
double
minmag(double x, double y)
{
  return fmin(fabs(x), fabs(y));
}

#ifndef __hexagon__
static inline
float
modf(float x, float *iptr)
{
  return modff(x, iptr);
}
#endif

#ifndef __hexagon__
static inline
float
nan(unsigned int nancode)
{
  return reinterpret_cast<const float&>(nancode |= 0x7F800000U);
}

static inline
double
nan(unsigned long nancode)
{

  return reinterpret_cast<const double&>(nancode |= 0x7F800000U);
}

static inline
float
nextafter(float x, float y)
{
  return nextafterf(x, y);
}

static inline
float
pow(float x, float y)
{
  return powf(x, y);
}
#endif

static inline
float
pownf(float x, int n)
{
  float res = 1.0;
  char neg = (n < 0) ? 1 : 0;
  if (neg != 0) n = -n;
  while (n > 0) {
    res *= x * x;
    if ((n & 1) == 1) res *= x;
    n >>= 1;
  }
  return (neg != 0) ? res : (1 / res);
}

static inline
double
pown(double x, int n)
{
  double res = 1.0;
  char neg = (n < 0) ? 1 : 0;
  if (neg != 0) n = -n;
  while (n > 0) {
    res *= x * x;
    if ((n & 1) == 1) res *= x;
    n >>= 1;
  }
  return (neg != 0) ? res : (1 / res);
}

static inline
float
powrf(float x, float y)
{
  return powf(x, y);
}

static inline
double
powr(double x, double y)
{
  return pow(x, y);
}

#ifndef __hexagon__
static inline
float
remainder(float x, float y)
{
  return remainderf(x, y);
}

static inline
float
remquo(float x, float y, int *quo)
{
  return remquof(x, y, quo);
}

static inline
float
rint(float x)
{
  return rintf(x);
}
#endif

static inline
float
rootnf(float x, int y)
{
  return pow(x, 1.0f / y);
}

static inline
double
rootn(double x, int y)
{
  return pow(x, 1.0 / y);
}

#ifndef __hexagon__
static inline
float
round(float x)
{
  return roundf(x);
}

static inline
float
sqrt(float x)
{
  return sqrtf(x);
}
#endif

static inline
float
rsqrtf(float x)
{
  return 1.0f / sqrt(x);
}

static inline
double
rsqrt(double x)
{
  return 1.0f / sqrt(x);
}

#ifndef __hexagon__
static inline
float
sin(float x)
{
  return sinf(x);
}
#endif

static inline
float
sincosf(float x, float *cosval)
{
#ifdef _GNU_SOURCE
  float sinval;
  sincosf(x, &sinval, cosval);
  return sinval;
#else
  *cosval = cos(x);
  return sin(x);
#endif
}

static inline
double
sincos(double x, double *cosval)
{
#ifdef _GNU_SOURCE
  double sinval;
  sincos(x, &sinval, cosval);
  return sinval;
#else
  *cosval = cos(x);
  return sin(x);
#endif
}

#ifndef __hexagon__
static inline
float
sinh(float x)
{
  return sinhf(x);
}
#endif

static inline
float
sinpif(float x)
{
  return sin(x * SYMPHONY_M_PI_F);
}

static inline
double
sinpi(double x)
{
  return sin(x * SYMPHONY_M_PI);
}

#ifndef __hexagon__
static inline
float
tan(float x)
{
  return tanf(x);
}

static inline
float
tanh(float x)
{
  return tanhf(x);
}
#endif

static inline
float
tanpif(float x)
{
  return tan(x * SYMPHONY_M_PI_F);
}

static inline
double
tanpi(double x)
{
  return tan(x * SYMPHONY_M_PI);
}

#ifndef __hexagon__
static inline
float
tgamma(float x)
{
  return tgammaf(x);
}

static inline
float
trunc(float x)
{
  return truncf(x);
}
#endif

#if !defined(__hexagon__) && !defined(__OPENCL_VERSION__)

template <typename...>
struct symphony_is_one_of {
  static constexpr bool value = false;
};

template <typename T, typename F, typename...R>
struct symphony_is_one_of <T, F, R...> {
  static constexpr bool value = std::is_same<typename std::remove_cv<T>::type, F>::value || symphony_is_one_of<T, R...>::value;
};

template <typename T>
struct is_opencl_gentypef {
  static constexpr bool value =
    symphony_is_one_of<T, symphony::float2, symphony::float3, symphony::float4, symphony::float8, symphony::float16>::value;
};

template <typename T>
struct is_opencl_gentyped {
  static constexpr bool value =
    symphony_is_one_of<T, symphony::double2, symphony::double3, symphony::double4, symphony::double8, symphony::double16>::value;
};

template <typename T>
struct is_opencl_gentype {
  static constexpr bool value = is_opencl_gentypef<T>::value || is_opencl_gentyped<T>::value;
};

#define SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(func)\
template<typename T>\
static inline \
T \
func(const T& x)\
{\
  static_assert(is_opencl_gentype<T>::value, "expected float/double variants");\
  T v;\
  for (size_t i = 0; i < T::N; i++) {\
    v.s[i] = func(x.s[i]);\
  }\
  return v;\
}

#define SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(func)\
template<typename T>\
static inline \
T \
func(const T& x, const T& y)\
{\
  static_assert(is_opencl_gentype<T>::value, "expected float/double variants");\
  T v;\
  for (size_t i = 0; i < T::N; i++) {\
    v.s[i] = func(x.s[i], y.s[i]);\
  }\
  return v;\
}

#define SYMPHONY_MATH_VECTOR_FROM_SCALAR_3(func)\
template<typename T>\
static inline \
T \
func(const T& x, const T& y, const T& z)\
{\
  static_assert(is_opencl_gentype<T>::value, "expected float/double variants");\
  T v;\
  for (size_t i = 0; i < T::N; i++) {\
    v.s[i] = func(x.s[i], y.s[i], z.s[i]);\
  }\
  return v;\
}

#define SYMPHONY_MATH_VECTOR_FROM_SCALAR_2_P(func)\
template<typename T>\
static inline \
T \
func(const T& x, T* y)\
{\
  static_assert(is_opencl_gentype<T>::value, "expected float/double variants");\
  T v;\
  for (size_t i = 0; i < T::N; i++) {\
    v.s[i] = func(x.s[i], y->s[i]);\
  }\
  return v;\
}

SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(acos)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(acosh)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(acospi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(asin)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(asinh)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(asinpi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(atan)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(atan2)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(atanh)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(atanpi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(atan2pi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(cbrt)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(cbrtf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(ceil)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(ceilf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(copysign)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(copysignf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(cos)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(cosf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(cospi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(cospif)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(erfc)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(erfcf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(erf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(erff)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(exp)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(exp2)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(exp10)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(expm1)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(fabs)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(fdim)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(floor)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_3(fma)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(fmax)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(fmin)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(fmod)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2_P(fract)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(hypot)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(lgamma)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(log)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(log2)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(log10)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(log1p)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(logb)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_3(mad)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(maxmag)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(minmag)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2_P(modf)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(nextafter)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(pow)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(powr)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2(remainder)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(rint)

SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(round)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(rsqrt)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(sin)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_2_P(sincos)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(sinh)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(sinpi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(sinpif)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(sqrt)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(sqrtf)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(tan)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(tanh)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(tanpi)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(tgamma)
SYMPHONY_MATH_VECTOR_FROM_SCALAR_1(trunc)

#endif

#endif

#endif
