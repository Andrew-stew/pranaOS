/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include <libutils/Prelude.h>

struct MemoryObject
{
    int id;
    MemoryRange _range;

    int refcount;

    auto range() { return _range; }
};

void memory_object_initialize();

MemoryObject *memory_object_create(size_t size);

void memory_object_destroy(MemoryObject *memory_object);

MemoryObject *memory_object_ref(MemoryObject *memory_object);

void memory_object_deref(MemoryObject *memory_object);

MemoryObject *memory_object_by_id(int id);