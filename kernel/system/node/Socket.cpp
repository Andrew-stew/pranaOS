/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

// includes
#include <assert.h>
#include "system/node/Connection.h"
#include "system/node/Socket.h"

FsSocket::FsSocket() : FsNode(J_FILE_TYPE_SOCKET)
{
}

ResultOr<RefPtr<FsNode>> FsSocket::connect()
{
    auto connection = make<FsConnection>();
    _pending.push_back(connection);
    return {connection};
}

bool FsSocket::can_accept()
{
    return _pending.any();
}

ResultOr<RefPtr<FsNode>> FsSocket::accept()
{
    assert(_pending.any());

    auto connection = _pending.pop_back();
    connection->accepted();
    return connection;
}