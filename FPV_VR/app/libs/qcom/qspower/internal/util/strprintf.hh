#pragma once

#include <string>

#include <qspower/internal/util/macros.hh>

namespace qspower
{
    namespace internal
    {
        QSPOWER_GCC_ATTRIBUTE((format(printf, 1, 2)))
        std::string strprintf(const char*, ...);

    }; // namespace internal
};     // namespace qspower
