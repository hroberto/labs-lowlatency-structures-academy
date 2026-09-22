// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `std` — o que a linguagem entrega, sem ajuda.
//
// Ele é o baseline, e é escrito para ser **justo**, não para perder. Duas
// escolhas seguem dessa regra:
//
//   1. usa `std::allocator<node>`, e não `new` direto. É a forma que a
//      biblioteca padrão oferece para quem quer trocar de alocador depois, e é
//      o que um código de produção escreveria;
//   2. NÃO guarda a lista de ponteiros. Quem guarda é o chamador, igual para os
//      dois braços -- ver o comentário da spec sobre simetria.
//
// O que ele não faz é esconder o que custa: cada `acquire()` vai ao alocador
// global, e cada `release_all()` devolve um por um. É exatamente isso que o
// tópico quer medir.

#ifndef ACADEMY_MEMORY_HEAP_ARM_HPP
#define ACADEMY_MEMORY_HEAP_ARM_HPP

#include "docs/01-memoria/01-alocador-arena/spec.hpp"

#include <memory>

namespace academy::memory
{

class heap_arm
{
  public:
    // `bounded` é falso e isso é uma AFIRMAÇÃO, não uma omissão: o alocador
    // global não tem teto declarado, e quando a memória acaba ele lança em vez
    // de devolver `nullptr`. O teste da invariante 6 é pulado com esse motivo.
    static constexpr arm_traits traits{
        .frees_individually = true,
        .bounded = false,
        .name = "std::allocator",
    };

    explicit heap_arm(std::size_t /*capacity_hint*/ = 0) noexcept {}

    [[nodiscard]] node *acquire()
    {
        node *p = alloc_.allocate(1);
        std::construct_at(p);
        ++outstanding_;
        return p;
    }

    void release_all(std::span<node *const> held) noexcept
    {
        for (node *p : held)
        {
            std::destroy_at(p);
            alloc_.deallocate(p, 1);
        }
        outstanding_ = 0;
    }

    // A invariante 7, que só este braço honra.
    void release(node *p) noexcept
    {
        std::destroy_at(p);
        alloc_.deallocate(p, 1);
        --outstanding_;
    }

    [[nodiscard]] std::size_t outstanding() const noexcept { return outstanding_; }

  private:
    std::allocator<node> alloc_{};
    std::size_t outstanding_ = 0;
};

static_assert(batch_allocator<heap_arm>);

} // namespace academy::memory

#endif // ACADEMY_MEMORY_HEAP_ARM_HPP
