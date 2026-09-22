// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `custom` — a arena.
//
// A troca é inteira, e vale enunciá-la antes do código: **a arena não devolve
// objeto individual, e em troca aloca somando um índice.** Tudo o que ela ganha
// vem de ter desistido da liberação individual; tudo o que ela custa vem de ter
// um teto.
//
// A REGRA DA SEÇÃO 34 FOI CUMPRIDA NESTA ORDEM
//
// Este arquivo só existe porque o braço `std` já está medido e publicado. Um
// complemento proposto contra baseline não medido é opinião com código.
//
// O QUE ELA NÃO É
//
// Não é um alocador de uso geral. Ela serve ao padrão que o cenário do livro de
// ofertas tem: um lote de objetos nasce, é usado, e morre inteiro. Fora desse
// padrão, a liberação individual que ela não tem deixa de ser detalhe e passa a
// ser impedimento -- e é isso que a *Regra de decisão* do documento diz, com
// número.

#ifndef ACADEMY_MEMORY_ARENA_ARM_HPP
#define ACADEMY_MEMORY_ARENA_ARM_HPP

#include "docs/01-memoria/01-alocador-arena/spec.hpp"

#include <cstddef>
#include <memory>
#include <new>
#include <span>
#include <vector>

namespace academy::memory
{

class arena_arm
{
  public:
    static constexpr arm_traits traits{
        .frees_individually = false,
        .bounded = true,
        .name = "arena",
    };

    // A capacidade é reservada UMA vez, no construtor, e fora de qualquer
    // medição. Uma arena que crescesse sob demanda chamaria o alocador global
    // no meio do lote -- e o braço passaria a medir, de vez em quando, aquilo
    // de que ele existe para fugir.
    explicit arena_arm(std::size_t capacity) : buffer_(capacity), capacity_(capacity) {}

    // O CAMINHO QUENTE INTEIRO: um teste e uma soma.
    //
    // O ramo de esgotamento é `[[unlikely]]` porque, num lote dimensionado, ele
    // não acontece -- mas ele EXISTE, e é a invariante 6. Uma arena sem esse
    // teste é `arena_sem_teto`, que mora em `tests/` e que a suíte exige que
    // falhe: verificação que nunca dispara é indistinguível de ausente.
    [[nodiscard]] node *acquire() noexcept
    {
        if (used_ >= capacity_) [[unlikely]]
        {
            return nullptr;
        }
        return &buffer_[used_++];
    }

    // `release_all` ignora os ponteiros de propósito: a arena devolve tudo
    // zerando um índice, e é daí que vem a diferença que o tópico mede.
    void release_all(std::span<node *const> /*held*/) noexcept { used_ = 0; }

    [[nodiscard]] std::size_t outstanding() const noexcept { return used_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

  private:
    std::vector<node> buffer_;
    std::size_t capacity_ = 0;
    std::size_t used_ = 0;
};

static_assert(batch_allocator<arena_arm>);

} // namespace academy::memory

#endif // ACADEMY_MEMORY_ARENA_ARM_HPP
