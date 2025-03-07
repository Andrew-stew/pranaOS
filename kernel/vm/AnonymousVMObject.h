/*
 * Copyright (c) 2021, Krisna Pranav
 *
 * SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

#include <kernel/PhysicalAddress.h>
#include <kernel/VM/AllocationStrategy.h>
#include <kernel/VM/MemoryManager.h>
#include <kernel/VM/PageFaultResponse.h>
#include <kernel/VM/PurgeablePageRanges.h>
#include <kernel/VM/VMObject.h>

namespace Kernel {

class AnonymousVMObject final : public VMObject {
    friend class PurgeablePageRanges;

public:
    virtual ~AnonymousVMObject() override;

    static RefPtr<AnonymousVMObject> try_create_with_size(size_t, AllocationStrategy);
    static RefPtr<AnonymousVMObject> try_create_for_physical_range(PhysicalAddress paddr, size_t size);
    static RefPtr<AnonymousVMObject> try_create_with_physical_pages(Span<NonnullRefPtr<PhysicalPage>>);
    virtual RefPtr<VMObject> try_clone() override;

    [[nodiscard]] NonnullRefPtr<PhysicalPage> allocate_committed_page(Badge<Region>, size_t);
    PageFaultResponse handle_cow_fault(size_t, VirtualAddress);
    size_t cow_pages() const;
    bool should_cow(size_t page_index, bool) const;
    void set_should_cow(size_t page_index, bool);

    void register_purgeable_page_ranges(PurgeablePageRanges&);
    void unregister_purgeable_page_ranges(PurgeablePageRanges&);

    int purge();

    bool is_any_volatile() const;

    template<IteratorFunction<VolatilePageRange const&> F>
    IterationDecision for_each_volatile_range(F f) const
    {
        VERIFY(m_lock.is_locked());
        for (auto* purgeable_range : m_purgeable_ranges) {
            ScopedSpinLock purgeable_lock(purgeable_range->m_volatile_ranges_lock);
            for (auto& r1 : purgeable_range->volatile_ranges().ranges()) {
                VolatilePageRange range(r1);
                for (auto* purgeable_range2 : m_purgeable_ranges) {
                    if (purgeable_range2 == purgeable_range)
                        continue;
                    ScopedSpinLock purgeable2_lock(purgeable_range2->m_volatile_ranges_lock);
                    if (purgeable_range2->is_empty()) {
                        return IterationDecision::Continue;
                    }
                    for (auto const& r2 : purgeable_range2->volatile_ranges().ranges()) {
                        range = range.intersected(r2);
                        if (range.is_empty())
                            break;
                    }
                    if (range.is_empty())
                        break;
                }
                if (range.is_empty())
                    continue;
                IterationDecision decision = f(range);
                if (decision != IterationDecision::Continue)
                    return decision;
            }
        }
        return IterationDecision::Continue;
    }

    template<IteratorFunction<VolatilePageRange const&> F>
    IterationDecision for_each_nonvolatile_range(F f) const
    {
        size_t base = 0;
        for_each_volatile_range([&](VolatilePageRange const& volatile_range) {
            if (volatile_range.base == base)
                return IterationDecision::Continue;
            IterationDecision decision = f(VolatilePageRange { base, volatile_range.base - base });
            if (decision != IterationDecision::Continue)
                return decision;
            base = volatile_range.base + volatile_range.count;
            return IterationDecision::Continue;
        });
        if (base < page_count())
            return f(VolatilePageRange { base, page_count() - base });
        return IterationDecision::Continue;
    }

    template<VoidFunction<VolatilePageRange const&> F>
    IterationDecision for_each_volatile_range(F f) const
    {
        return for_each_volatile_range([&](auto& range) {
            f(range);
            return IterationDecision::Continue;
        });
    }

    template<VoidFunction<VolatilePageRange const&> F>
    IterationDecision for_each_nonvolatile_range(F f) const
    {
        return for_each_nonvolatile_range([&](auto range) {
            f(move(range));
            return IterationDecision::Continue;
        });
    }

private:
    explicit AnonymousVMObject(size_t, AllocationStrategy);
    explicit AnonymousVMObject(PhysicalAddress, size_t);
    explicit AnonymousVMObject(Span<NonnullRefPtr<PhysicalPage>>);
    explicit AnonymousVMObject(AnonymousVMObject const&);

    virtual StringView class_name() const override { return "AnonymousVMObject"sv; }

    void update_volatile_cache();
    void set_was_purged(VolatilePageRange const&);
    size_t remove_lazy_commit_pages(VolatilePageRange const&);
    void range_made_volatile(VolatilePageRange const&);
    void range_made_nonvolatile(VolatilePageRange const&);
    size_t count_needed_commit_pages_for_nonvolatile_range(VolatilePageRange const&);
    size_t mark_committed_pages_for_nonvolatile_range(VolatilePageRange const&, size_t);
    bool is_nonvolatile(size_t page_index);

    AnonymousVMObject& operator=(AnonymousVMObject const&) = delete;
    AnonymousVMObject& operator=(AnonymousVMObject&&) = delete;
    AnonymousVMObject(AnonymousVMObject&&) = delete;

    virtual bool is_anonymous() const override { return true; }

    Bitmap& ensure_cow_map();
    void ensure_or_reset_cow_map();

    VolatilePageRanges m_volatile_ranges_cache;
    bool m_volatile_ranges_cache_dirty { true };
    Vector<PurgeablePageRanges*> m_purgeable_ranges;
    size_t m_unused_committed_pages { 0 };

    Bitmap m_cow_map;

    RefPtr<CommittedCowPages> m_shared_committed_cow_pages;
};

}