/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include <libasync/Observable.h>
#include <libio/Path.h>
#include <libsystem/process/Process.h>
#include <libutils/Vector.h>

namespace FilePicker
{

struct Navigation :
    public Async::Observable<Navigation>,
    public RefCounted<Navigation>
{
private:
    Vector<IO::Path> _backward{};
    IO::Path _current{};
    Vector<IO::Path> _foreward{};

public:
    enum Direction
    {
        NONE,
        BACKWARD,
        FOREWARD
    };

    const IO::Path &current() const { return _current; }

    Navigation();

    bool can_go_up();

    void go_up();

    bool can_go_backward();

    void go_backward();

    bool can_go_forward();

    void go_forward();

    void go_home();

    void go_home_dont_record_history();

    void refresh();

    void navigate(String directory);

    void navigate(IO::Path path);

    void navigate(IO::Path path, Direction record_history);

    void clear_foreward();
};

} 