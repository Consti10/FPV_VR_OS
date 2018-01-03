/** @file power.hh */
#pragma once

#include <chrono>
#include <string>
#include <qspower/devicetypes.hh>
#include <qspower/power.h>
#include <qspower/types.h>

#include <qspower/internal/util/strprintf.hh>

namespace qspower
{
    using frequency_type    = ::qspower_freq_t;
    using freq_percent_type = qspower_freq_percent_t;
    using goal_type = qspower_goal_t;
    using tolerance_type = qspower_tolerance_t;

    namespace mode
    {
        /** @addtogroup static_power_doc
        @{ */

        class efficient_t
        {
        };

        class normal_t
        {
        };

        class perf_burst_t
        {
        };

        class saver_t
        {
        };

        /**
            * Enable all available cores and reduce the maximum frequency. Useful for
            * parallel applications with work bursts or for long-running applications with
            * thermal constraints.
            */
        const efficient_t efficient{};

        /**
            * System default power settings. When the programmer requests this mode,
            * it cancels previously requested modes, and the device returns to its
            * previous state.
            */
        const normal_t normal{};

        /**
            * Enable all cores at the maximum frequency for a limited amount of time
            * in the order of seconds. Actual time will depend on the device, the rest of
            * running applications, and the environment. Useful for punctual high demands
            * of work preceded and followed by long idle periods.
            */
        const perf_burst_t perf_burst{};

        /**
            * Enable part of the cores in those systems with support and with maximum
            * mid-demand applications.
            */
        const saver_t saver{};

        /**
            * Window power mode class.
            * It contains two data members, the minimum and maximum frequency percentages.
            */
        class window
        {
        public:
            explicit window(freq_percent_type min = 0, freq_percent_type max = 100);

            /**
                * Copy constructor.
                */
            explicit window(window const& other);

            /**
                * Move constructor.
                */
            explicit window(window&& other);

            /**
                * Copy assignment operator.
                */
            window& operator=(window const& other);

            // deleted methods
            QSPOWER_DELETE_METHOD(window& operator=(window&& other));

            /**
                * Returns the minimum frequency percentage for the window.
                *
                * @return freq_percent_type _min
                */
            freq_percent_type get_min() const { return _min; }
            /**
                * Returns the maximum frequency percentage for the window.
                *
                * @return freq_percent_type _max
                */
            freq_percent_type get_max() const { return _max; }

            bool is_set() const { return (_min != 0 || _max != 100); }

        private:
            /* Minimum frequency percentage.
                * Minimum frequency is determined by _min percent of the
                * max number of available frequencies the device can support.
                */
            freq_percent_type _min;

            /* Maximum frequency percentage.
                * Maximum frequency is determined by _max percent of the
                * max number of available frequencies the device can support.
                */
            freq_percent_type _max;

        }; // end of class window

        /** @} */ /* end_addtogroup static_power_doc */
    };            // namespace mode

    /** @addtogroup support_power_doc
    @{ */

    /**
    Check if the current system supports the Qualcomm® Snapdragon™ Power Optimization SDK.

    This function can only be called after Qualcomm® Snapdragon™ Power Optimization SDK has been initialized.

    @return
    <code>true</code> The system supports the Qualcomm® Snapdragon™ Power Optimization SDK.\n
    <code>false</code> The system does not support the Qualcomm® Snapdragon™ Power Optimization SDK.
    */
    bool is_supported();

    /**
    Return the current version number of Qualcomm® Snapdragon™ Power Optimization SDK.

    This function will return the current version number of Qualcomm® Snapdragon™ Power Optimization SDK.

        @return
        <code>const char*</code> Current Version number of Qualcomm® Snapdragon™ Power Optimization SDK.
        */
    const char* get_version();

    /**
    Initializes the power infrastructure.

    @return
    <code>true</code> if Power API is successfully initialized
    <code>false</code> if Power library initialize fails.
    */
    bool init();

    /**
    Terminates the power infrastructure.

    \note This should be called when Qualcomm® Snapdragon™ Power Optimization SDK is no longer needed,
    before the application ends.
    */
    void terminate();

    /** @} */ /* end_addtogroup support_power_doc */

    /** @addtogroup static_power_doc
        @{ */
    /**
        Set the system in the window performance/power mode. This  will enable all cores
        with user-provided minimum and maximum frequency percents for a given time.

        @param[in] win Frequency window at which to operate cores.
        @param[in] duration Time in milliseconds that the mode should remain enabled.
        If duration is lower than 0, the function returns immediately.
        If duration is 0, the window power mode remains active until:
        - the application finishes, or until
        - another qspower_request_call, or
        - a call to qspower_set_goal()
        @param[in] devices  The set of devices to apply the power mode

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.

    */
    device_set request_mode(mode::window const&              win,
                            std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                            device_set const&                devices  = { qspower::device_type::cpu });

    /**
        Set the system in the window performance/power mode. This  will enable all cores
        with user-provided minimum and maximum frequency percents lasting until request of
        another power mode or the end of the program execution.

        @param[in] win Frequency window at which to operate cores.
        @param[in] devices  The set of devices to apply the power mode.

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.

    */
    device_set request_mode(mode::window const& win, device_set const& devices);

    /**
        Set the system in the efficient performance/power mode. Efficient mode will enable
        all available cores and reduce the maximum frequency.

        @param[in] duration Time in milliseconds that the mode should remain enabled.
        If duration is lower than 0, the function returns immediately.
        If duration is 0, the efficient power mode remains active until:
        - the application finishes, or until
        - another qspower_request_call, or
        - a call to qspower_set_goal()
        @param[in] devices  The set of devices to apply the power mode

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.

        @par Example: Setting the efficent mode.
        @includelineno snippets/power_static.cc

        \note Depending on the load and the system status, requests can be ignored.
    */
    device_set request_mode(mode::efficient_t const&,
                            std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                            device_set const&                devices  = { qspower::device_type::cpu });

    /**
        Set the system in the efficient performance/power mode. Efficient mode will enable
        all available cores and reduce the maximum frequency, lasting until request of
        another power mode or the end of the program execution.

        @param[in] devices  The set of devices to apply the power mode

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.
    */
    device_set request_mode(mode::efficient_t const&, device_set const& devices);

    /**
        Return the system to the normal performance/power modes. In normal mode, Power Optimization SDK will not
        send a request to the system, and the existing power and temperature managers
        will control the number of cores and their frequency.

        @param[in] devices  The set of devices to apply the power mode.

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.
    */
    device_set request_mode(mode::normal_t const&, device_set const& devices = { qspower::device_type::cpu });

    /**
        Set the system in the saver performance/power mode. Saver mode will enable half
        of the total cores with maximum frequency limited to the median of the available
        frequencies.

        @param[in] duration Time in milliseconds that the mode should remain enabled.
        If duration is lower than 0, the function returns immediately.
        If duration is 0, the saver power mode remains active until:
        - the application finishes, or until
        - another qspower_request_call, or
        - a call to qspower_set_goal()

        @param[in] devices  The set of devices to apply the power mode.

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.

        \note Depending on the load and the system status, requests can be ignored.
    */
    device_set request_mode(mode::saver_t const&,
                            std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                            device_set const&                devices  = { qspower::device_type::cpu });

    /**
        Set the system in the saver performance/power mode. Saver mode will enable half
        of the total cores with maximum frequency limited to the median of the available
        frequencies, lasting until request of another power mode or the end of the program execution.

        @param[in] devices  The set of devices to apply the power mode.

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.
    */
    device_set request_mode(mode::saver_t const&, device_set const& devices);

    /**
        Set the system in the perf_burst predefined performance/power modes. Perf_Burst
        mode will enable all cores at the maximum frequency for a limited amount of time in
        the order of seconds. Actual time will depend on the device, the rest of running
        applications, and the environment.

        @param[in] duration Time in milliseconds that the mode should remain enabled.
        perf_burst mode has a limited maximum duration depending on the device
        and environment. If duration is lower than 0, the function returns
        immediately. If duration is 0, the perf_burst power mode remains active until:
        - the application finishes, or until
        - another qspower_request_call, or
        - a call to qspower_set_goal()
        @param[in] devices  The set of devices to apply the power mode.

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.

        \note Depending on the load and the system status, requests can be ignored.
    */
    device_set request_mode(mode::perf_burst_t const&,
                            std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                            device_set const&                devices  = { qspower::device_type::cpu });

    /**
        Set the system in the perf_burst predefined performance/power modes. Perf_Burst
        mode will enable all cores at the maximum frequency for the maximum limited amount of time.
        Actual time will depend on the device, the rest of running applications, and the environment.

        @param[in] devices  The set of devices to apply the power mode.

        @return
        <code>device_set</code> Set of devices on which the request was
        successfully performed.
    */
    device_set request_mode(mode::perf_burst_t const&, device_set const& devices);

    /** @} */ /* end_addtogroup static_power_doc */

    /** @addtogroup dynamic_power_doc
        @{ */
    /**
        Start the automatic performance/power regulation mode.

        Once the regulation process starts, subsequent calls to this function
        are silently ignored.

        @param[in] desired User-defined performance metric used as a target for the
        runtime system. Usually, the parameter desired corresponds to a throughput
        metric such as frames, bits, or packets-per-second. Any other metric can also
        be used.  Both the metric used for the parameter desired and the parameter
        regulate must be in the same units of measurement. For better results,
        use metrics whose values range between 10 and 100.
        @param[in] tolerance Percentage of allowed errors between the desired and
        the measured values. If the error is less than the tolerance, the system
        remains in the same state. For example, if the quotient between measured
        and desired is 0.95 and tolerance is 0.1, the state will not change.
        When the tolerance is met, the system will remain in the current state.
        If tolerance is 0.0, the runtime tries to adjust the system at every step.
        @param[in] devices  The set of devices to apply the power mode.

        \note Currently only cpu_big device is supported and setting goal on all other
        devices will be silently ignored.

    */
    bool set_goal(goal_type desired, tolerance_type tolerance = 0.0, device_set const& devices = { qspower::device_type::cpu_big });

    /**
        Terminate the regulation process and return the system to normal mode.
    */
    bool clear_goal();

    /**
        Regulate the system to achieve the desired performance level. If the
        measured parameter is different from the desired value set by set_goal(),
        the runtime will try to update the system status to reach it.

        @param[in] measured Measured performance observed in the last iteration by the
        program. The program must periodically recompute this value and pass the new
        value to the runtime system, even if the goal has been reached. For example,
        if the desired goal is 30 frames per second, the program will obtain the
        current fps and pass the value to measured.
    */
    bool regulate(goal_type measured);

    /**
        *  Data structure for capturing the current goal performance since
        *  the last <code>set_goal()</code>.
        *
        *  Consists of the following fields
        *  - regulation_steps : the number of completed regulation steps since the
        *                    last <code>set_goal()</code>.
        *                    Note: each <code>regulate()</code> call marks the
        *                    start of a new regulation step and the completion of
        *                    the prior regulation step.
        *  - goal_fraction : the fraction of regulation steps where the desired
        *                    goal was achieved within the specified tolerance.
        *  - mean_squared_error : the mean squared error in achieving the desired goal
        *                    over the regulation steps.
        *  - normalized_mserror : the mean-squared error normalized by the tolerance value.
        *
        *  @sa qspower::get_goal_performance()
        */
    class goal_performance
    {
    public:
        goal_performance() :
            _regulation_steps(0),
            _goal_fraction(0.0),
            _average_error(0.0),
            _mean_squared_error(0.0),
            _normalized_mserror(0.0)
        {
        }

        goal_performance(size_t arg_regulation_steps,
                            double arg_goal_fraction,
                            double arg_average_error,
                            double arg_mean_squared_error,
                            double arg_normalized_mserror) :
            _regulation_steps(arg_regulation_steps),
            _goal_fraction(arg_goal_fraction),
            _average_error(arg_average_error),
            _mean_squared_error(arg_mean_squared_error),
            _normalized_mserror(arg_normalized_mserror)
        {
        }

        inline size_t regulation_steps() const
        {
            return _regulation_steps;
        }

        inline double goal_fraction() const
        {
            return _goal_fraction;
        }

        inline double average_error() const
        {
            return _average_error;
        }

        inline double mean_squared_error() const
        {
            return _mean_squared_error;
        }

        inline double normalized_mserror() const
        {
            return _normalized_mserror;
        }

        std::string to_string() const
        {
            auto s = qspower::internal::strprintf("steps=%zu goal fr=%f err=%f mse=%f nmse=%f",
                                                  _regulation_steps,
                                                  _goal_fraction,
                                                  _average_error,
                                                  _mean_squared_error,
                                                  _normalized_mserror);
            return s;
        }

    private:
        size_t _regulation_steps;
        double _goal_fraction;
        double _average_error;
        double _mean_squared_error;
        double _normalized_mserror;
    };

    /**
        *  Gets the performance achieved by the dynamic regulation API
        *  since the last <code>set_goal()</code> call.
        *
        *  @sa qspower::goal_performance
        */
    goal_performance get_goal_performance();

    /** @} */ /* end_addtogroup dynamic_power_doc */

    // beta feature: dynamic regulation on both multiple devices (GPU/CPU)
    namespace beta
    {
        /** @cond PRIVATE */
        class tuner
        {
        public:
            qspower::device_set devices;
        };
        /** @endcond */

        bool set_goal(const float, const float, const tuner&);
        bool clear_goal();
        bool regulate(const float);
    }; // namespace beta

}; // namespace qspower
