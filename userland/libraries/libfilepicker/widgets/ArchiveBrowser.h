/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include <libfilepicker/model/ArchiveListing.h>
#include <libfilepicker/model/Navigation.h>
#include <libwidget/Views.h>
#include <libfile/Archive.h>

namespace FilePicker
{

struct ArchiveBrowser : public Widget::Table
{
private:
    RefPtr<Navigation> _navigation;
    RefPtr<Archive> _archive;
    RefPtr<ArchiveListing> _listing;
    OwnPtr<Async::Observer<Navigation>> _navigation_observer;

public:
    ArchiveBrowser(RefPtr<Navigation> navigation, RefPtr<Archive> archive)
        : Table(), _navigation(navigation), _archive(archive)
    {
        _listing = make<ArchiveListing>(navigation, archive);
        model(_listing);

        flags(Element::FILL);

        empty_message("This archive is empty.");

        _navigation_observer = navigation->observe([this](auto &) {
            select(-1);
            scroll_to_top();
        });

        on(Widget::Event::ACTION, [this](auto) {
            if (selected() >= 0)
            {
                if (_listing->info(selected()).type == HJ_FILE_TYPE_DIRECTORY)
                {
                    _navigation->navigate(_listing->info(selected()).name);
                }
                else
                {
                }
            }
        });
    }
};

}  