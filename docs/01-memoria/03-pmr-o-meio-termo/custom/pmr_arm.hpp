// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `custom` QUE NÃO É ARTESANAL: `std::pmr`.
//
// `monotonic_buffer_resource` é a arena da biblioteca padrão. Ele aloca somando
// um ponteiro dentro de um buffer que se dá a ele, e `release()` devolve tudo
// de uma vez -- exatamente o desenho do tópico 01.01.
//
// A RESERVA É `null_memory_resource`, E ISSO É A INVARIANTE 6
//
// Por padrão, quando o buffer acaba, o `monotonic_buffer_resource` pede mais ao
// recurso de reserva -- que é o alocador global. Isso é conveniente e destrói o
// teto: no meio de um lote, sem aviso, a alocação volta a custar o que custava.
//
// `null_memory_resource` como reserva transforma o esgotamento em
// `std::bad_alloc`, que este braço converte em `nullptr` para cumprir a spec.
// É a diferença entre "arena com teto" e "arena que às vezes vira heap", e é
// uma decisão de configuração que quase não aparece nos exemplos de `pmr`.

#ifndef ACADEMY_MEMORY_PMR_ARM_HPP
#define ACADEMY_MEMORY_PMR_ARM_HPP

#include "docs/01-memoria/03-pmr-o-meio-termo/spec.hpp"

#include <cstddef>
#include <memory_resource>
#include <new>
#include <span>
#include <vector>

namespace academy::memory
{

class pmr_arm
{
  public:
    static constexpr arm_traits traits{
        .frees_individually = false,
        .bounded = true,
        .name = "std::pmr",
    };

    // O BUFFER É UM `vector<node>`, E NÃO UM `vector<byte>` COM FOLGA.
    //
    // A primeira versão reservava `capacity * sizeof(node) + alignof(node)`
    // bytes, "para o alinhamento". Só que `alignof(node)` é 64 e `sizeof(node)`
    // é 64: a folga era **exatamente um nó**, e o braço servia `capacity + 1`.
    //
    // O teste da invariante 6 pegou -- "serviu alem da capacidade declarada" --,
    // e é literalmente para isso que ela existe. Um teto que não é o teto
    // declarado é pior que teto nenhum: o dimensionamento do lote passa a ser
    // feito contra um número que o código não respeita.
    //
    // `vector<node>` resolve na origem: o `node` é `alignas(64)`, então o
    // armazenamento já vem alinhado, e o tamanho em bytes é exato.
    explicit pmr_arm(std::size_t capacity)
        : buffer_(capacity),
          recurso_(buffer_.data(), capacity * sizeof(node), std::pmr::null_memory_resource()),
          capacity_(capacity)
    {
    }

    [[nodiscard]] node *acquire() noexcept
    {
        try
        {
            void *p = recurso_.allocate(sizeof(node), alignof(node));
            ++used_;
            return static_cast<node *>(p);
        }
        catch (const std::bad_alloc &)
        {
            // O `catch` está no caminho quente e não custa nada enquanto não
            // dispara: o modelo de exceção do GCC e do Clang em x86-64 é
            // zero-cost. Quando dispara, o lote já acabou -- e é isso que a
            // invariante 6 diz.
            return nullptr;
        }
    }

    void release_all(std::span<node *const> /*held*/) noexcept
    {
        recurso_.release();
        used_ = 0;
    }

    [[nodiscard]] std::size_t outstanding() const noexcept { return used_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

  private:
    std::vector<node> buffer_;
    std::pmr::monotonic_buffer_resource recurso_;
    std::size_t capacity_ = 0;
    std::size_t used_ = 0;
};

static_assert(batch_allocator<pmr_arm>);

} // namespace academy::memory

#endif // ACADEMY_MEMORY_PMR_ARM_HPP
