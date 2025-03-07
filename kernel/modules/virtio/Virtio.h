/*
 * Copyright (c) 2021, nuke123-sudo
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/


#pragma once

// includes
#include "system/devices/DeviceAddress.h"

// virtio status codes
#define VIRTIO_STATUS_ACKNOWLEDGE (1)
#define VIRTIO_STATUS_DRIVER (2)
#define VIRTIO_STATUS_DRIVER_OK (4)
#define VIRTIO_STATUS_FAILED (128)
#define VIRTIO_STATUS_FEATURES_OK (8)
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET (64)

// register device features
#define VIRTIO_REGISTER_DEVICE_FEATURES (0x00)
#define VIRTIO_REGISTER_GUEST_FEATURES (0x04)
#define VIRTIO_REGISTER_QUEUE_ADDRESS (0x08)
#define VIRTIO_REGISTER_QUEUE_SIZE (0x0C)
#define VIRTIO_REGISTER_QUEUE_SELECT (0x0E)
#define VIRTIO_REGISTER_QUEUE_NOTIFY (0x10)
#define VIRTIO_REGISTER_DEVICE_STATUS (0x12)
#define VIRTIO_REGISTER_ISR_STATUS (0x13)
