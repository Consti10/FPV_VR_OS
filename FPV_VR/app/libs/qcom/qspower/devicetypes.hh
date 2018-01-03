/** @file devicetypes.hh */
#pragma once

#include <initializer_list>
#include <string>
#include <qspower/devices.h>
#include <qspower/internal/util/macros.hh>

namespace qspower
{
    class device_set;

    namespace internal
    {
        qspower_device_set_t get_raw_device_set_t(device_set const& d);
    }; // namespace internal

    /** @addtogroup support_power_doc
        @{ */

    /**
     *  @brief The system devices that can be configured via the Power Optimization SDK.
     *
     *  The system devices that can be configured via the Power Optimization SDK.
     */
    enum device_type
    {
        cpu_big    = QSPOWER_DEVICE_TYPE_CPU_BIG,
        cpu_little = QSPOWER_DEVICE_TYPE_CPU_LITTLE,
        cpu        = QSPOWER_DEVICE_TYPE_CPU_BIG | QSPOWER_DEVICE_TYPE_CPU_LITTLE,
        gpu        = QSPOWER_DEVICE_TYPE_GPU,
    };

    /**
     *  @brief Converts <code>device_type</code> to string.
     *
     *  Converts <code>device_type</code> to string.
     *
     *  @param d device_type (e.g., cpu, cpu_big, cpu_little, or gpu).
     *  @return std::string (e.g., "cpu", "cpu_big", "cpu_little", or "gpu").
     */

    std::string to_string(device_type d);

    /**
     *  @brief Captures a set of device types.
     *
     *  Captures a set of device types.
     *
     *  Supports addition and removal of <code>device_type</code>s from the set.
     *  Supports set union and set subtraction with another device_set object.
     */
    class device_set
    {
    private:
        qspower_device_set_t _device_set_mask;

    public:
        /**
          *  @brief Default constructor produces empty set.
          *
          *  Default constructor produces empty set.
          */
        device_set();

         /**
          *  @brief Constructor taking qspower_device_set_t
          *
          */
        device_set(qspower_device_set_t devices);

        /**
          *  @brief Constructor with initialization.
          *
          *  Constructor with initialization.
          *
          *  @param device_list nn initializer list of <code>device_type</code> elements.
          *
          *  Example:\n
          *  <code>
          *    qspower::device_set ds{qspower::cpu, qspower::gpu};
          *  </code>
          */
        device_set(std::initializer_list<device_type> device_list);

        /**
        *  @brief Checks if device set has any devices or its empty
        *
        *  Checks if device set has any devices or its empty.
        *
        *  @return
        *  <code>true</code> if device_set has no devices,\n
        *  <code>false</code> otherwise.
        */
        bool empty() const;

        /**
        *  @brief Query if CPU is part of the device_set.
        *
        *  Query if CPU is part of the device_set.
        *
        *  @return
        *  <code>true</code> if CPU present,\n
        *  <code>false</code> otherwise.
        */
        bool on_cpu() const;

        /**
        *  @brief Query if CPU big core is part of the device_set.
        *
        *  Query if CPU big core is part of the device_set.
        *
        *  @return
        *  <code>true</code> if CPU big core present,\n
        *  <code>false</code> otherwise.
        */
        bool on_cpu_big() const;

        /**
        *  @brief Query if CPU LITTLE core is part of the device_set.
        *
        *  Query if CPU LITTLE core is part of the device_set.
        *
        *  @return
        *  <code>true</code> if CPU LITTLE core present,\n
        *  <code>false</code> otherwise.
        */
        bool on_cpu_little() const;

        /**
        *  @brief Query if GPU is part of the device_set.
        *
        *  Query if GPU is part of the device_set.
        *
        *  @return
        *  <code>true</code> if GPU present,\n
        *  <code>false</code> otherwise.
        */
        bool on_gpu() const;

        /**
        *  @brief Add a <code>device_type</code> to the <code>device_set</code>.
        *
        *  Add a <code>device_type</code> to the <code>device_set</code>.
        *
        *  @param d <code>device_type</code> to add (no effect if already present).
        *
        *  @return Reference to the updated <code>device_set</code>.
        */
        device_set& add(device_type d);

        /**
        *  @brief Set union with another <code>device_set</code>.
        *
        *  Set union with another <code>device_set</code>.
        *
        *  @param other Another <code>device_set</code>.
        *
        *  @return Reference to the updated <code>device_set</code>.
        *
        *  Example:\n
        *  <code>
        *    qspower::device_set a{qspower::cpu};\n
        *    qspower::device_set b{qspower::gpu};\n
        *    a.add(b);\n
        *    assert(true == a.on_cpu());\n
        *    assert(true == a.on_gpu());\n
        *  </code>
        */
        device_set& add(device_set const& other);

        /**
        *   @brief Remove a device_type from the device_set.
        *
        *  Remove a device_type from the device_set.
        *
        *  @param d device_type to remove (no effect if not present).
        *
        *  @return Reference to the updated device_set.
        */
        device_set& remove(device_type d);

        /**
        *  @brief Set substraction with another device_set.
        *
        *  Set substraction with another device_set.
        *
        *  @param other Another device_set.
        *
        *  @return Reference to the updated device_set.
        *
        *  Example:\n
        *  <code>
        *    qspower::device_set a{qspower::cpu, qspower::gpu};\n
        *    qspower::device_set b{qspower::gpu};\n
        *    a.remove(b);\n
        *    assert(true == a.on_cpu());\n
        *    assert(false == a.on_gpu());\n
        *  </code>
        */
        device_set& remove(device_set const& other);

        /**
        *  @brief Negate the device_set.
        *
        *  Negate the device_set.
        *
        *  @return Reference to the updated device_set.
        *
        *  Example:\n
        *  <code>
        *    qspower::device_set a{qspower::cpu, qspower::gpu};\n
        *    a.negate();\n
        *    assert(false == a.on_cpu());\n
        *    assert(false == a.on_gpu());\n
        *  </code>
        */
        device_set& negate();

        /**
        *  @brief Convert the device_set to a string representation.
        *
        *  Convert the device_set to a string representation.
        *
        *  @return std::string representation of the devices present.
        *
        *  Example:\n
        *  <code>
        *    qspower::device_set a{qspower::cpu, qspower::gpu};\n
        *    assert(a.to_string() == "cpu gpu ");\n
        *  </code>
        */
        std::string to_string() const;

        /**
        *  @brief Compare device_sets and return true or false
        *
        */
        bool operator==(device_set & rhs) const;

        /**
        * Copy constructor.
        */
        QSPOWER_DEFAULT_METHOD(device_set(device_set const&));

        /**
        * Copy assignment.
        */
        QSPOWER_DEFAULT_METHOD(device_set& operator=(device_set const&));

        /**
        * Move constructor.
        */
        QSPOWER_DEFAULT_METHOD(device_set(device_set&&));

        /**
        * Move assignment.
        */
        QSPOWER_DEFAULT_METHOD(device_set& operator=(device_set&&));

        friend qspower_device_set_t internal::get_raw_device_set_t(device_set const& d);
    };

    /** @} */ /* end_addtogroup support_power_doc */

}; // namespace qspower
