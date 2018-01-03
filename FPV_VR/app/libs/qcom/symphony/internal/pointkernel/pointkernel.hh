// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include<string>

#include "pk_util.hh"

namespace symphony{

namespace internal{

namespace pointkernel{

template<typename RT, typename... Args>
class pointkernel;
};

template <typename ReturnType, typename ...Args, typename ArgTuple, size_t Dims, size_t... Indices>
void pfor_each_run_helper(const symphony::range<Dims>& r,
                pointkernel::pointkernel<ReturnType,Args...>& pk,
                const symphony::pattern::tuner& t,
                symphony::internal::integer_list_gen<Indices...>,
                ArgTuple& atpl);

namespace testing{

class pk_accessor;
};

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

inline static constexpr size_t get_num_dsp_threads(){
  return 1;
}
#endif

namespace pointkernel{

template<typename ReturnType, typename ...Args>
class pointkernel{

  using cpu_kernel_type = typename cpu_kernel_generator<ReturnType, typename make_cpu_kernel_args<Args...>::type>::type;
  using cpu_kernel_signature_type = typename cpu_kernel_signature_generator<ReturnType,
                                      typename make_cpu_kernel_args<Args...>::type>::type;

#if defined(SYMPHONY_HAVE_OPENCL)
  using gpu_kernel_args =typename replace_pointer_size_pair_with_bufferptr<Args...>::type;

  using gpu_kernel_type = typename gpu_kernel_generator<gpu_kernel_args>::type;

  using gpu_output_buffer_type = typename get_output_buffer_type<gpu_kernel_args>::type;
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
  using hexagon_kernel_args = typename make_hexagon_kernel_args<Args...>::type;

  using hexagon_kernel_signature = typename hexagon_kernel_signature_generator<hexagon_kernel_args>::type;

  using hexagon_kernel_type = typename hexagon_kernel_generator<hexagon_kernel_args>::type;

  using hexagon_output_buffer_type = typename get_hexagon_output_buffer_type<Args...>::type;
#endif

public:
  pointkernel():
    _cpu_kernel()
#if defined(SYMPHONY_HAVE_OPENCL)
    , _gpu_kernel_name()
    , _gpu_kernel_string()
#endif
#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
    ,_hexagon_kernel()
#endif
  {}

  pointkernel(cpu_kernel_signature_type fn
#if defined(SYMPHONY_HAVE_OPENCL)
              , std::string kernel_name
              , std::string kernel_string
#endif
#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
              , hexagon_kernel_signature dsp_fn
#endif
  ):
    _cpu_kernel(symphony::create_cpu_kernel<>(fn))
#if defined(SYMPHONY_HAVE_OPENCL)
    ,_gpu_kernel(kernel_string, kernel_name

#ifdef __aarch64__
                  ,
                  "-Dcbrtf=cbrt "
                  "-Dcosf=cos "
                  "-Dcoshf=cosh "
                  "-Dcospif=cospi "
                  "-Derfcf=erfc "
                  "-Derff=erf "
                  "-Dexpf=exp "
                  "-Dexp2f=exp2 "
                  "-Dexpm1f=expm1 "
                  "-Dfabsf=fabs "
                  "-Dfdimf=fdim "
                  "-Dfloorf=floor "
                  "-Dfmaf=fma "
                  "-Dfmaxf=fmax "
                  "-Dfminf=fmin "
                  "-Dfmodf=fmod "
                  "-Dfractf=fract "
                  "-Dfrexpf=frexp "
                  "-Dhypotf=hypot "
                  "-Dldexpf=ldexp "
                  "-Dlgammaf=lgamma "
                  "-Dlogf=logf "
                  "-Dlog2f=log2 "
                  "-Dlog10f=log10 "
                  "-Dlog1pf=log1p "
                  "-Dlogbf=logb "
                  "-Dmadf=mad "
                  "-Dmaxmagf=maxmag "
                  "-Dminmagf=minmag "
                  "-Dmodff=modf "
                  "-Dnextafterf=nextafter "
                  "-Dpowf=pow "
                  "-Dpownf=pown "
                  "-Dpowrf=powr "
                  "-Dremainderf=remainder "
                  "-Dremquof=remquo "
                  "-Drintf=rint "
                  "-Drootnf=rootn "
                  "-Droundf=round "
                  "-Dsqrtf=sqrt "
                  "-Drsqrtf=rsqrt "
                  "-Dsinf=sin "
                  "-Dsincosf=sincos "
                  "-Dsinhf=sinh "
                  "-Dsinpif=sinpi "
                  "-Dtanf=tan "
                  "-Dtanhf=tanh "
                  "-Dtanpif=tanpi "
                  "-Dtgammaf=tgamma "
                  "-Dtruncf=trunc "
#endif
                  )
    ,_gpu_kernel_name(kernel_name)
    ,_gpu_kernel_string(kernel_string)
    ,_gpu_local_buffer()
#endif
#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
    ,_hexagon_kernel(symphony::create_hexagon_kernel<>(dsp_fn))
    ,_hexagon_local_buffer_vec(get_num_dsp_threads())
#endif
    {}

#if defined(SYMPHONY_HAVE_OPENCL)
  gpu_output_buffer_type get_gpu_local_buffer(){return _gpu_local_buffer;}

  void set_gpu_local_buffer(gpu_output_buffer_type buf){ _gpu_local_buffer = buf;}
#endif

  SYMPHONY_DEFAULT_METHOD(pointkernel(pointkernel const&));
  SYMPHONY_DEFAULT_METHOD(pointkernel(pointkernel&&));
  SYMPHONY_DEFAULT_METHOD(pointkernel& operator=(pointkernel const &));
  SYMPHONY_DEFAULT_METHOD(pointkernel& operator=(pointkernel&&));

  friend class ::symphony::internal::testing::pk_accessor;

  template <typename RT, typename ...PKArgs, typename ArgTuple, size_t Dims, size_t... Indices>
  friend void symphony::internal::pfor_each_run_helper(
                         symphony::pattern::pfor<
                           symphony::internal::pointkernel::pointkernel<RT,PKArgs...>,
                           ArgTuple>* const p,
                         const symphony::range<Dims>& r,
                         symphony::internal::pointkernel::pointkernel<RT,PKArgs...>& pk,
                         symphony::pattern::tuner& t,
                         symphony::internal::integer_list_gen<Indices...>,
                         ArgTuple& atpl);

private:
  cpu_kernel_type _cpu_kernel;
#if defined(SYMPHONY_HAVE_OPENCL)
  gpu_kernel_type _gpu_kernel;
  std::string _gpu_kernel_name;
  std::string _gpu_kernel_string;
  gpu_output_buffer_type _gpu_local_buffer;

#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
  hexagon_kernel_type _hexagon_kernel;
  std::vector<hexagon_output_buffer_type> _hexagon_local_buffer_vec;
#endif
};

};

};

};
