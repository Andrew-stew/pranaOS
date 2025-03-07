/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

// includes
#include <base/IntrusiveList.h>
#include <base/OwnPtr.h>
#include <base/SinglyLinkedList.h>
#include <base/Vector.h>

namespace Base {

template<typename T, int segment_size = 1000>
class Queue {
public:
    Queue() = default;

    ~Queue()
    {
        clear();
    }

    size_t size() const { return m_size; }
    bool is_empty() const { return m_size == 0; }

    template<typename U = T>
    void enqueue(U&& value)
    {
        if (m_segments.is_empty() || m_segments.last()->data.size() >= segment_size) {
            auto segment = new QueueSegment;
            m_segments.append(*segment);
        }
        m_segments.last()->data.append(forward<U>(value));
        ++m_size;
    }

    T dequeue()
    {
        VERIFY(!is_empty());
        auto value = move(m_segments.first()->data[m_index_into_first++]);
        if (m_index_into_first == segment_size) {
            delete m_segments.take_first();
            m_index_into_first = 0;
        }
        --m_size;
        if (m_size == 0 && !m_segments.is_empty()) {
            m_index_into_first = 0;
            m_segments.last()->data.clear_with_capacity();
        }
        return value;
    }

    const T& head() const
    {
        VERIFY(!is_empty());
        return m_segments.first()->data[m_index_into_first];
    }

    void clear()
    {
        while (auto* segment = m_segments.take_first())
            delete segment;
        m_index_into_first = 0;
        m_size = 0;
    }

private:
    struct QueueSegment {
        Vector<T, segment_size> data;
        IntrusiveListNode<QueueSegment> node;
    };

    IntrusiveList<QueueSegment, RawPtr<QueueSegment>, &QueueSegment::node> m_segments;
    size_t m_index_into_first { 0 };
    size_t m_size { 0 };
};

}

using Base::Queue;