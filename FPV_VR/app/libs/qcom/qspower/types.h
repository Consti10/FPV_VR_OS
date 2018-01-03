#ifndef QSPOWER_LIBPOWER_TYPES_H
#define QSPOWER_LIBPOWER_TYPES_H

typedef unsigned int qspower_freq_t;
typedef unsigned int qspower_freq_percent_t;

/** @addtogroup libpower_c_doc
    @{ */
typedef unsigned long long qspower_milliseconds_t;
typedef float              qspower_goal_t;
typedef float              qspower_tolerance_t;

/**
 *  Data structure for capturing the current goal performance since
 *  the last <code>qspower_set_goal()</code>.
 *
 *  Consists of the following fields
 *  - regulation_steps : the number of completed regulation steps since the
 *                    last <code>qspower_set_goal()</code>.
 *                    Note: each <code>qspower_regulate()</code> call marks the
 *                    start of a new regulation step and the completion of
 *                    the prior regulation step.
 *  - goal_fraction : the fraction of regulation steps where the desired
 *                    goal was achieved within the specified tolerance.
 *  - mean_squared_error : the mean squared error in achieving the desired goal
 *                    over the regulation steps.
 *  - normalized_mserror : the mean-squared error normalized by the tolerance value.
 *
 *  @sa qspower_get_goal_performance()
 */
typedef struct
{
    unsigned long long _regulation_steps;
    double             _goal_fraction;
    double             _average_error;
    double             _mean_squared_error;
    double             _normalized_mserror;
} qspower_goal_performance_t;

/** @} */ /* end_addtogroup libpower_c_doc */

#endif // QSPOWER_LIBPOWER_TYPES_H
