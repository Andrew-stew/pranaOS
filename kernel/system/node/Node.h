/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/


#pragma once

// includes
#include <libabi/Filesystem.h>
#include <libabi/IOCall.h>
#include <libabi/Result.h>
#include <libutils/Lock.h>
#include <libutils/RefPtr.h>
#include <libutils/ResultOr.h>
#include <libutils/String.h>

struct FsNode;
struct FsHandle;

struct FsNode : public RefCounted<FsNode>
{
private:
    Lock _lock{"fsnode"};
    JFileType _type;

    unsigned int _readers = 0;
    unsigned int _writers = 0;
    unsigned int _clients = 0;
    unsigned int _server = 0;

public:
    JFileType type() { return _type; }

    unsigned int readers() { return _readers; }

    unsigned int writers() { return _writers; }

    unsigned int clients() { return _clients; }

    unsigned int server() { return _server; }

    FsNode(JFileType type);

    virtual ~FsNode()
    {
    }

    void ref_handle(FsHandle &handle);

    void deref_handle(FsHandle &handle);

    virtual JResult open(FsHandle &) { return SUCCESS; }

    virtual void close(FsHandle &) {}

    virtual size_t size() { return 0; }

    virtual JResult call(FsHandle &handle, IOCall request, void *args)
    {
        UNUSED(handle);
        UNUSED(request);
        UNUSED(args);

        return ERR_INAPPROPRIATE_CALL_FOR_DEVICE;
    }

    virtual bool can_read(FsHandle &) { return true; }

    virtual bool can_write(FsHandle &) { return true; }

    virtual ResultOr<size_t> read(FsHandle &handle, void *buffer, size_t size)
    {
        UNUSED(handle);
        UNUSED(buffer);
        UNUSED(size);

        return ERR_NOT_READABLE;
    }

    virtual ResultOr<size_t> write(FsHandle &handle, const void *buffer, size_t size)
    {
        UNUSED(handle);
        UNUSED(buffer);
        UNUSED(size);

        return ERR_NOT_WRITABLE;
    }

    virtual RefPtr<FsNode> find(String name)
    {
        UNUSED(name);

        return nullptr;
    }

    virtual JResult link(String name, RefPtr<FsNode> child)
    {
        UNUSED(name);
        UNUSED(child);

        return ERR_OPERATION_NOT_SUPPORTED;
    }

    virtual JResult unlink(String name)
    {
        UNUSED(name);

        return ERR_OPERATION_NOT_SUPPORTED;
    }

    virtual void accepted() {}


    virtual bool is_accepted() { return false; }

    virtual ResultOr<RefPtr<FsNode>> connect() { return ERR_SOCKET_OPERATION_ON_NON_SOCKET; }

    virtual bool can_accept() { return false; }

    virtual ResultOr<RefPtr<FsNode>> accept() { return ERR_SOCKET_OPERATION_ON_NON_SOCKET; }

    bool is_acquire();

    void acquire(int who_acquire);

    void release(int who_release);
};
