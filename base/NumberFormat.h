/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include <base/String.h>

namespace Base {

static String number_string_with_one_decimal(u64 number, u64 unit, const char* suffix)
{
    int decimal = (number % unit) * 10 / unit;
    return String::formatted("{}.{} {}", number / unit, decimal, suffix);
}

static inline String human_readable_size(u64 size)
{
    if (size < 1 * KiB)
        return String::formatted("{} B", size);
    if (size < 1 * MiB)
        return number_string_with_one_decimal(size, KiB, "KiB");
    if (size < 1 * GiB)
        return number_string_with_one_decimal(size, MiB, "MiB");
    if (size < 1 * TiB)
        return number_string_with_one_decimal(size, GiB, "GiB");
    if (size < 1 * PiB)
        return number_string_with_one_decimal(size, TiB, "TiB");
    if (size < 1 * EiB)
        return number_string_with_one_decimal(size, PiB, "PiB");
    return number_string_with_one_decimal(size, EiB, "EiB");
}

static inline String human_readable_size_long(u64 size)
{
    if (size < 1 * KiB)
        return String::formatted("{} bytes", size);
    else
        return String::formatted("{} ({} bytes)", human_readable_size(size), size);
}

}

using Base::human_readable_size;
using Base::human_readable_size_long;