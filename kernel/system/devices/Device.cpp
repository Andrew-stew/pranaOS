/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

// includes

#include "system/devices/Device.h"
#include "system/devices/Devices.h"

Device::Device(DeviceAddress address, DeviceClass klass)
    : _address(address),
      _klass(klass),
      _name(device_claim_name(klass))
{
}