// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O BRAÇO `std` — array de estruturas.
//
// É o layout que a linguagem convida a escrever: `struct` com os campos, e um
// `vector` dela. Nada aqui é ingênuo de propósito -- este É o código que se
// escreve quando não se está pensando em cache, e é justamente por isso que ele
// é o baseline.

#ifndef ACADEMY_LAYOUT_AOS_ARM_HPP
#define ACADEMY_LAYOUT_AOS_ARM_HPP

#include "docs/02-layout/01-soa-vs-aos/spec.hpp"

#include <cstdint>
#include <vector>

namespace academy::layout
{

class aos_arm
{
  public:
    static constexpr arm_traits traits{
        .record_contiguous = true,
        .name = "AoS",
    };

    struct alignas(64) registro
    {
        std::uint64_t campo[kCampos] = {};
    };

    explicit aos_arm(std::size_t n) : dados_(n) {}

    // Percorre todos os registros lendo UM campo. No AoS, cada registro é uma
    // linha de cache, e desta linha usamos 8 dos 64 bytes.
    [[nodiscard]] std::uint64_t sum_field(std::size_t k) const noexcept
    {
        std::uint64_t soma = 0;
        for (const registro &r : dados_)
        {
            soma += r.campo[k];
        }
        return soma;
    }

    // Escreve em todos os campos de um registro. No AoS isso é uma linha só.
    void touch_record(std::size_t i, std::uint64_t v) noexcept
    {
        for (std::size_t k = 0; k < kCampos; ++k)
        {
            dados_[i].campo[k] = v + k;
        }
    }

    void set(std::size_t i, std::size_t k, std::uint64_t v) noexcept { dados_[i].campo[k] = v; }
    [[nodiscard]] std::uint64_t get(std::size_t i, std::size_t k) const noexcept
    {
        return dados_[i].campo[k];
    }
    [[nodiscard]] std::size_t size() const noexcept { return dados_.size(); }

  private:
    std::vector<registro> dados_;
};

static_assert(record_store<aos_arm>);

} // namespace academy::layout

#endif // ACADEMY_LAYOUT_AOS_ARM_HPP
