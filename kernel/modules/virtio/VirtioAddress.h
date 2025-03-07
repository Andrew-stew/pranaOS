/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

enum VirtioAddress
{
    VIRTIO_DEVICE_NETWORK = 1,
    VIRTIO_DEVICE_BLOCK = 2,
    VIRTIO_DEVICE_CONSOLE = 3,
    VIRTIO_DEVICE_ENTROPY = 4,
    VIRTIO_DEVICE_GRAPHICS = 16,
};