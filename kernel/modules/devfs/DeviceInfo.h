/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include "system/node/Node.h"

struct FsDeviceInfo : public FsNode
{
private:
public:
    FsDeviceInfo();

    JResult open(FsHandle &handle) override;

    void close(FsHandle &handle) override;

    ResultOr<size_t> read(FsHandle &handle, void *buffer, size_t size) override;
};

void device_info_initialize();