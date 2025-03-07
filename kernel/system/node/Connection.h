/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include <libutils/RingBuffer.h>
#include "system/node/Node.h"

struct FsConnection : public FsNode
{
private:
    static constexpr int BUFFER_SIZE = 4096;

    bool _accepted = false;

    RingBuffer<char> _data_to_server{BUFFER_SIZE};
    RingBuffer<char> _data_to_client{BUFFER_SIZE};

public:
    FsConnection();

    void accepted() override;

    bool is_accepted() override;

    bool can_read(FsHandle &handle) override;

    bool can_write(FsHandle &handle) override;

    ResultOr<size_t> read(FsHandle &handle, void *buffer, size_t size) override;

    ResultOr<size_t> write(FsHandle &handle, const void *buffer, size_t size) override;
};