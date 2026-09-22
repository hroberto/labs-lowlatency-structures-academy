// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `custom` — pool com lista de livres.
//
// A IDEIA EM UMA FRASE: o slot livre guarda o índice do próximo slot livre.
//
// A lista de livres não custa memória extra, e essa é a parte elegante: o
// objeto morto não precisa dos seus 64 bytes para nada, então o primeiro deles
// vira o elo. Um `union` explícito seria mais claro para quem lê, e mais frágil
// para quem otimiza -- aqui o elo mora num vetor paralelo de índices, que é
// mais um acesso e uma ordem de grandeza menos surpresa.
//
// POR QUE ÍNDICE, E NÃO PONTEIRO
//
// Índice de 32 bits ocupa metade de um ponteiro, e o pool tem teto declarado --
// então o índice basta. Num pool de 4096 slots, a lista inteira cabe em 16 KB e
// fica no L1; a mesma lista em ponteiros ocuparia 32 KB e começaria a disputar.
//
// É o tipo de troca que este módulo existe para medir, e ela está na tabela.

#ifndef ACADEMY_MEMORY_POOL_ARM_HPP
#define ACADEMY_MEMORY_POOL_ARM_HPP

#include "docs/01-memoria/02-pool-freelist/spec.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace academy::memory
{

class pool_arm
{
  public:
    static constexpr arm_traits traits{
        .frees_individually = true,
        .bounded = true,
        .name = "pool",
    };

    static constexpr std::uint32_t kNulo = std::numeric_limits<std::uint32_t>::max();

    explicit pool_arm(std::size_t capacity)
        : buffer_(capacity), proximo_(capacity), capacity_(capacity)
    {
        // A lista nasce encadeada em ordem, do slot 0 ao último. Poderia nascer
        // vazia e crescer sob demanda -- e aí o primeiro `acquire` de cada slot
        // pagaria a primeira falta de página DENTRO da medição, que é o defeito
        // que o aquecimento do benchmark existe para evitar.
        for (std::size_t i = 0; i < capacity; ++i)
        {
            proximo_[i] = (i + 1 < capacity) ? static_cast<std::uint32_t>(i + 1) : kNulo;
        }
        cabeca_ = capacity > 0 ? 0U : kNulo;
    }

    // O CAMINHO QUENTE: um teste, uma leitura de índice, uma escrita.
    [[nodiscard]] node *acquire() noexcept
    {
        if (cabeca_ == kNulo) [[unlikely]]
        {
            return nullptr;
        }
        const std::uint32_t slot = cabeca_;
        cabeca_ = proximo_[slot];
        ++vivos_;
        return &buffer_[slot];
    }

    // Devolver é empilhar na cabeça. O slot devolvido é o PRÓXIMO a sair, e
    // isso não é detalhe: ele acabou de ser tocado, então está quente. Uma
    // lista FIFO devolveria o slot mais frio, e a diferença aparece no p50.
    void release(node *p) noexcept
    {
        const auto slot = static_cast<std::uint32_t>(p - buffer_.data());
        proximo_[slot] = cabeca_;
        cabeca_ = slot;
        --vivos_;
    }

    [[nodiscard]] std::size_t outstanding() const noexcept { return vivos_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

  private:
    std::vector<node> buffer_;
    std::vector<std::uint32_t> proximo_;
    std::size_t capacity_ = 0;
    std::size_t vivos_ = 0;
    std::uint32_t cabeca_ = kNulo;
};

static_assert(churn_allocator<pool_arm>);

} // namespace academy::memory

#endif // ACADEMY_MEMORY_POOL_ARM_HPP
