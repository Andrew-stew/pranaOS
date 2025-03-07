
/*
 * Copyright (c) 2021, evilbat831
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/


#pragma once

// includes
#include "unix/UNIX.h"

struct UNIXDevice : public Device
{
private:
public:
    UNIXDevice(DeviceAddress address, DeviceClass klass)
        : Device(address, klass)
    {
    }

    ~UNIXDevice()
    {
    }
};

template <typename UNIXDeviceType>
struct UNIXDeviceMatcher : public DeviceMatcher
{
private:
    UNIXAddress _address;

public:
    UNIXDeviceMatcher(const char *name, UNIXAddress address)
        : DeviceMatcher(BUS_UNIX, name),
          _address(address)
    {
    }

    bool match(DeviceAddress address)
    {
        return _address == address.unix();
    }

    RefPtr<Device> instance(DeviceAddress address)
    {
        return make<UNIXDeviceType>(address);
    }
};
