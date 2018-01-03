#ifndef QSPOWER_POWER_H
#define QSPOWER_POWER_H

#include <stdbool.h>
#include <qspower/devices.h>
#include <qspower/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup libpower_c_doc
    @{ */

/**
 * Initializes the power infrastructure.
 *
 * @return
 * <code>true</code> if Power API is successfully initialized;
 * <code>false</code> if Power library initialize fails.
 */
bool qspower_init();

/**
 * Terminates the power infrastructure.
 *
 * \note This should be called when Qualcomm® Snapdragon™ Power Optimization SDK is no longer needed,
 * before the application ends.
 */
void qspower_terminate();

/**
 * Check if the current system supports the Qualcomm® Snapdragon™ Power Optimization SDK.
 *
 * This function can only be called after Qualcomm® Snapdragon™ Power Optimization SDK has been initialized.
 *
 * @return
 * <code>true</code> The system supports the Qualcomm® Snapdragon™ Power Optimization SDK.\n
 * <code>false</code> The system does not support the Qualcomm® Snapdragon™ Power Optimization SDK.
 */
bool qspower_is_supported();

/**
  *  Return the current version number of Qualcomm® Snapdragon™ Power Optimization SDK.
  *
  *  This function will return the current version number of Qualcomm® Snapdragon™ Power Optimization SDK.
  *
  *  @return
  *  <code>char*</code> Current Version number of Qualcomm® Snapdragon™ Power Optimization SDK.
  */
char* qspower_get_version();

/**
 * Set the system in the efficient performance/power mode. Efficient mode will enable
 * all available cores and reduce the maximum frequency.
 * This power mode is useful for parallel applications with work bursts or for
 * long-running applications with thermal constraints.
 *
 * The programmer can specify the duration of the mode in milliseconds or can pass
 * a 0 in the duration parameter.
 *
 * @param[in] duration Time (in milliseconds) that the mode should remain enabled.
 * If duration is 0, the efficient power mode remains active until:
 *  - the application finishes, or until
 *  - another qspower_request_call, or
 *  - a call to qspower_set_goal()
 *
 * @param[in] device_set Aggregates all the devices on which user intends to apply
 * the power mode. Possible device choices can be found from
 * <code>qspower_device_type_t</code>.
 * - For example: to request efficient mode for both big and LITTLE cores, simply call
 * <code>qspower_request_efficient_mode(0, QSPOWER_DEVICE_TYPE_CPU_BIG |
 * QSPOWER_DEVICE_TYPE_CPU_LITTLE)</code>;
 *
 * \note Depending on the load and the system status, requests can be ignored.
 * For example, in case of thermal throttling.
 *
 * @return
 * <code>qspower_device_set_t</code> Set of devices on which the request was
 * successfully performed.
 */
qspower_device_set_t qspower_request_efficient_mode(qspower_milliseconds_t duration, qspower_device_set_t device_set);

/**
 *  Return the system to the normal performance/power modes. In normal
 *  mode, Power Optimization SDK will not send a request to the system, and the original
 *  power and temperature managers will control the number of cores
 *  and their frequency.
 *
 * @param[in] device_set Aggregates all the devices on which user intends to apply
 * the power mode. Possible device choices can be found from
 * <code>qspower_device_type_t</code>.
 * - For example: to request normal mode for both big and LITTLE cores, simply call
 * <code>qspower_request_normal_mode(0, QSPOWER_DEVICE_TYPE_CPU_BIG |
 * QSPOWER_DEVICE_TYPE_CPU_LITTLE)</code>;
 *
 * @return
 * <code>qspower_device_set_t</code> Set of devices on which the request was
 * successfully performed.
 */
qspower_device_set_t qspower_request_normal_mode(qspower_device_set_t device_set);

/**
 * Set the system in the perf_burst predefined performance/power mode. Perf_Burst
 * mode will enable all cores at the maximum frequency for a limited amount of time in
 * the order of seconds. Actual time will depend on the device, the rest of running
 * applications, and the environment.
 *
 * @param[in] duration Time (in milliseconds) that the mode should remain enabled.
 * perf_burst mode has a limited maximum duration depending on the device
 * and environment. If duration is lower than 0, the function returns
 * immediately. If duration is 0, the perf_burst power mode remains active until:
 *      - the application finishes, or until
 *      - another qspower_request_call, or
 *      - a call to qspower_set_goal()
 *
 * @param[in] device_set Aggregates all the devices on which user intends to apply
 * the power mode. Possible device choices can be found from
 * <code>qspower_device_type_t</code>.
 * - For example: to request perf_burst mode for both big and LITTLE cores, simply call
 * <code>qspower_request_perf_burst_mode(0, QSPOWER_DEVICE_TYPE_CPU_BIG |
 * QSPOWER_DEVICE_TYPE_CPU_LITTLE)</code>;
 *
 *  \note Depending on the load and the system status, requests can be ignored.
 *
 * @return
 * <code>qspower_device_set_t</code> Set of devices on which the request was
 * successfully performed.
 */
qspower_device_set_t qspower_request_perf_burst_mode(qspower_milliseconds_t duration, qspower_device_set_t device_set);

/**
 *  Set the system in the saver performance/power mode. Saver mode will enable half
 *  of the total cores with maximum frequency limited to the median of the available
 *  frequencies.
 *
 *   @param[in] duration Time (in milliseconds) that the mode should remain enabled.
 *  If duration is lower than 0, the function returns immediately.
 *  If duration is 0, the saver power mode remains active until:
 *      - the application finishes, or until
 *      - another qspower_request_call, or
 *      - a call to qspower_set_goal()
 *
 *   @param[in] device_set Aggregates all the devices on which user intends to apply
 *   the power mode. Possible device choices can be found from
 *   <code>qspower_device_type_t</code>.
 *   - For example: to request saver mode for both big and LITTLE cores,
 *     simply call <code>qspower_request_saver_mode(0,
 *     QSPOWER_DEVICE_TYPE_CPU_BIG | QSPOWER_DEVICE_TYPE_CPU_LITTLE)</code>;
 *
 *   \note Depending on the load and the system status, requests can be ignored.
 *
 *   @return
 *   <code>qspower_device_set_t</code> Set of devices on which the request was
 *   successfully performed.
 */

qspower_device_set_t qspower_request_saver_mode(qspower_milliseconds_t duration, qspower_device_set_t device_set);

/**
 *  Set the system in the window performance/power mode. This  will enable all cores
 *  with user-provided minimum and maximum frequency percents.
 *
 *   @param[in] min_freq_percent Minimal frequency on which to operate cores. Minimum frequency
                is determined by min_freq_percent of the max number of available frequencies the
                device can support.
 *   @param[in] max_freq_percent Maximum frequency window on which to operate cores. Maximum frequency
                is determined by max_freq_percent of the max number of available frequencies the
                device can support.
 *   @param[in] duration Time (in milliseconds) that the mode should remain enabled.
 *      If duration is lower than 0, the function returns immediately.
 *      If duration is 0, the saver power mode remains active until:
 *          - the application finishes, or until
 *          - another qspower_request_call, or
 *          - a call to qspower_set_goal()
 *
 *   @param[in] device_set Aggregates all the devices on which user intends to apply
 *   the power mode. Possible device choices can be found from
 *   <code>qspower_device_type_t</code>.
 *   - For example: to request saver mode for both big and LITTLE cores,
 *     simply call <code>qspower_request_saver_mode(0,
 *     QSPOWER_DEVICE_TYPE_CPU_BIG | QSPOWER_DEVICE_TYPE_CPU_LITTLE)</code>;
 *
 *   @return
 *   <code>qspower_device_set_t</code> Set of devices on which the request was
 *   successfully performed.
*/
qspower_device_set_t qspower_request_window_mode(qspower_freq_percent_t min_freq_percent,
                                                 qspower_freq_percent_t max_freq_percent,
                                                 qspower_milliseconds_t duration,
                                                 qspower_device_set_t   device_set);

/**
 *  Start the automatic performance/power regulation mode.

 *  Once the regulation process starts, subsequent calls to this function
 *  are silently ignored.

 *  @param[in] desired User-defined performance metric used as a target for the
 *  runtime system. Usually, the parameter desired corresponds to a throughput
 *  metric such as frames, bits, or packets-per-second. Any other metric can also
 *  be used.  Both the metric used for the parameter desired and the parameter
 *  regulate must be in the same units of measurement. For better results,
 *  use metrics whose values range between 10 and 100.

 *  @param[in] tolerance Percentage of allowed errors between the desired and
 *  the measured values. If the error is less than the tolerance, the system
 *  remains in the same state. For example, if the quotient between measured
 *  and desired is 0.95 and tolerance is 0.1, the state will not change.

 *  When the tolerance is met, the system will remain in the current state.
 *  If tolerance is 0.0, the runtime tries to adjust the system at every step.

 *  @param[in] devices Aggregates all the devices on which user intends to apply
 *  the power mode. Possible device choices can be found from
 *  <code>qspower_device_type_t</code>.

 *  \note Currently dynamic power control only support tunning on cpu big cores. Setting
 *  power goals on any other devices will be silently ignored.

  @sa qspower_clear_goal(), qspower_regulate()
*/
bool qspower_set_goal(qspower_goal_t desired, qspower_tolerance_t tolerance, qspower_device_set_t devices);

/**
 *  Terminate the regulation process and return the system to normal mode.
 *
 *  @sa qspower_set_goal(), qspower_regulate()
*/
bool qspower_clear_goal();

/**
 *  Regulate the system to achieve the desired performance level. If the
 *  measured parameter is different from the desired value set by
 *  qspower_set_goal(), the runtime will try to update the system status
 *  to reach it.

 *  @param[in] measured Measured performance observed in the last iteration by the
 *  program. The program must periodically recompute this value and pass the new
 *  value to the runtime system, even if the goal has been reached. For example,
 *  if the desired goal is 30 frames per second, the program will obtain the
 *  current fps and pass the value to measured.

 *  @sa qspower_set_goal(), qspower_clear_goal()
*/
bool qspower_regulate(qspower_goal_t measured);

/**
 *  Gets the performance achieved by the dynamic regulation API
 *  since the last <code>qspower_set_goal()</code> call.
 *
 *  @sa qspower_goal_performance_t
 */
qspower_goal_performance_t qspower_get_goal_performance();

/** @} */ /* end_addtogroup libpower_c_doc */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // QSPOWER_POWER_H
