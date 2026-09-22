// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `std` deste tópico.
//
// É o mesmo `std::allocator` do 01.01, com a interface que esta spec exige --
// e nada mais. Reescrevê-lo daria a chance de escrevê-lo diferente, e dois
// baselines diferentes no mesmo módulo tornariam os dois tópicos
// incomparáveis.

#ifndef ACADEMY_MEMORY_HEAP_CHURN_ARM_HPP
#define ACADEMY_MEMORY_HEAP_CHURN_ARM_HPP

#include "docs/01-memoria/02-pool-freelist/spec.hpp"

#include <memory>

namespace academy::memory
{

class heap_churn_arm
{
  public:
    static constexpr arm_traits traits{
        .frees_individually = true,
        .bounded = false,
        .name = "std::allocator",
    };

    explicit heap_churn_arm(std::size_t /*capacity_hint*/ = 0) noexcept {}

    [[nodiscard]] node *acquire()
    {
        node *p = alloc_.allocate(1);
        std::construct_at(p);
        ++vivos_;
        return p;
    }

    void release(node *p) noexcept
    {
        std::destroy_at(p);
        alloc_.deallocate(p, 1);
        --vivos_;
    }

    [[nodiscard]] std::size_t outstanding() const noexcept { return vivos_; }

  private:
    std::allocator<node> alloc_{};
    std::size_t vivos_ = 0;
};

static_assert(churn_allocator<heap_churn_arm>);

} // namespace academy::memory

#endif // ACADEMY_MEMORY_HEAP_CHURN_ARM_HPP
