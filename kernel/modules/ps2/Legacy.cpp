/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

// includes
#include "ps2/Legacy.h"

Iteration legacy_scan(IterFunc<LegacyAddress> callback)
{
    if (callback(LEGACY_MOUSE) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_KEYBOARD) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_ATA0) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_ATA1) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_ATA2) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_ATA3) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_COM1) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_COM2) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_COM3) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    if (callback(LEGACY_COM4) == Iteration::STOP)
    {
        return Iteration::STOP;
    }

    return Iteration::CONTINUE;
}