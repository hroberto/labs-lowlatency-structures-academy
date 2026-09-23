// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `custom` — estrutura de arrays.
//
// Um vetor por campo. O registro deixa de existir como objeto contíguo e passa
// a ser uma linha imaginária que atravessa oito vetores.
//
// O QUE SE PAGA POR ISSO, E ESTÁ NA TABELA
//
//   - `touch_record` toca oito vetores, e portanto até oito linhas de cache,
//     onde o AoS tocava uma;
//   - o registro não é mais um objeto: não se pode passar `&r` para nada, nem
//     copiar um registro com uma atribuição.
//
// O segundo item não aparece em benchmark nenhum e é o que decide a maioria dos
// casos reais. A *Regra de decisão* diz isso com as palavras que couberem.

#ifndef ACADEMY_LAYOUT_SOA_ARM_HPP
#define ACADEMY_LAYOUT_SOA_ARM_HPP

#include "docs/02-layout/01-soa-vs-aos/spec.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace academy::layout
{

class soa_arm
{
  public:
    static constexpr arm_traits traits{
        .record_contiguous = false,
        .name = "SoA",
    };

    explicit soa_arm(std::size_t n) : n_(n)
    {
        for (auto &coluna : colunas_)
        {
            coluna.assign(n, 0);
        }
    }

    // Percorre UM vetor inteiro. Cada linha de cache traz oito valores úteis.
    [[nodiscard]] std::uint64_t sum_field(std::size_t k) const noexcept
    {
        std::uint64_t soma = 0;
        for (std::uint64_t v : colunas_[k])
        {
            soma += v;
        }
        return soma;
    }

    // Aqui o SoA paga: oito vetores, oito posições distantes umas das outras.
    void touch_record(std::size_t i, std::uint64_t v) noexcept
    {
        for (std::size_t k = 0; k < kCampos; ++k)
        {
            colunas_[k][i] = v + k;
        }
    }

    void set(std::size_t i, std::size_t k, std::uint64_t v) noexcept { colunas_[k][i] = v; }
    [[nodiscard]] std::uint64_t get(std::size_t i, std::size_t k) const noexcept
    {
        return colunas_[k][i];
    }
    [[nodiscard]] std::size_t size() const noexcept { return n_; }

  private:
    std::array<std::vector<std::uint64_t>, kCampos> colunas_{};
    std::size_t n_ = 0;
};

static_assert(record_store<soa_arm>);

} // namespace academy::layout

#endif // ACADEMY_LAYOUT_SOA_ARM_HPP
