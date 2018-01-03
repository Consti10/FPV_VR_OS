#ifndef QSPOWER_LIBPOWER_DEVICES_H
#define QSPOWER_LIBPOWER_DEVICES_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup libpower_c_doc
    @{ */
/**
 *  qspower_device_type_t indicates a specific device on which a Power Optimization SDK
 *  request to be applied.
 */

typedef enum
{
    // bitmask, one device a bit
    QSPOWER_DEVICE_NONE            = 0x0,
    QSPOWER_DEVICE_TYPE_CPU_BIG    = 0x01,
    QSPOWER_DEVICE_TYPE_CPU_LITTLE = 0x02,
    QSPOWER_DEVICE_TYPE_GPU        = 0x04,
    QSPOWER_DEVICE_TYPE_ALL        = 0x0F
} qspower_device_type_t;

/**
 * qspower_device_set_t aggregates all the devices on which a Power Optimization SDK
 * request to be applied.
 *
 * - For example:
 *   <code> qspower_device_set_t devices = QSPOWER_DEVICE_TYPE_CPU_BIG |
 *   QSPOWER_DEVICE_TYPE_CPU_LITTLE</code>
 */
typedef unsigned int qspower_device_set_t;

/** @} */ /* end_addtogroup libpower_c_doc */

#ifdef __cplusplus
}
#endif

#endif /* QSPOWER_LIBPOWER_DEVICES_H */
