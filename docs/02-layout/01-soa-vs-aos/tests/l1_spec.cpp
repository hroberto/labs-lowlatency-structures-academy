// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// L1 — as quatro invariantes da spec, nos dois layouts.
//
// A INVARIANTE 1 É A QUE PROTEGE A COMPARAÇÃO, E NÃO O CÓDIGO
//
// As outras três verificam que cada braço funciona. A primeira verifica que os
// **dois guardam o mesmo dado** -- e é ela que impede o modo de falha clássico
// de um benchmark de layout: um braço guardando menos, e a tabela publicando a
// diferença de dado com o nome de layout.
//
// Ela é a única que não roda por tipo: roda sobre o PAR, porque é uma afirmação
// sobre a relação entre os dois.

#include "docs/02-layout/01-soa-vs-aos/custom/soa_arm.hpp"
#include "docs/02-layout/01-soa-vs-aos/spec.hpp"
#include "docs/02-layout/01-soa-vs-aos/std/aos_arm.hpp"

#include <gtest/gtest.h>

#include <cstdint>

using namespace academy::layout;

namespace
{
constexpr std::size_t kN = 1024;

// Conteúdo determinístico: o mesmo nos dois braços, e reproduzível.
constexpr std::uint64_t valor(std::size_t i, std::size_t k)
{
    return (static_cast<std::uint64_t>(i) * 131U) ^ (static_cast<std::uint64_t>(k) * 17U);
}

template <typename S>
S preenchido(std::size_t n)
{
    S s{n};
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t k = 0; k < kCampos; ++k)
        {
            s.set(i, k, valor(i, k));
        }
    }
    return s;
}
} // namespace

// --- invariante 1: sobre o PAR, e não sobre um tipo ----------------------
TEST(SpecLayoutPar, os_dois_layouts_guardam_o_mesmo_dado)
{
    auto aos = preenchido<aos_arm>(kN);
    auto soa = preenchido<soa_arm>(kN);

    ASSERT_EQ(aos.size(), soa.size());
    for (std::size_t i = 0; i < kN; ++i)
    {
        for (std::size_t k = 0; k < kCampos; ++k)
        {
            ASSERT_EQ(aos.get(i, k), soa.get(i, k)) << "divergem no registro " << i
                                                    << ", campo " << k;
        }
    }
    // E a operação medida devolve o mesmo nos dois. É esta linha que torna a
    // comparação do benchmark legítima.
    for (std::size_t k = 0; k < kCampos; ++k)
    {
        EXPECT_EQ(aos.sum_field(k), soa.sum_field(k)) << "sum_field divergiu no campo " << k;
    }
}

template <typename S>
class SpecLayout : public ::testing::Test
{
};

using Bracos = ::testing::Types<aos_arm, soa_arm>;
TYPED_TEST_SUITE(SpecLayout, Bracos);

// --- invariante 2 ---------------------------------------------------------
TYPED_TEST(SpecLayout, sum_field_concorda_com_a_soma_feita_por_get)
{
    auto s = preenchido<TypeParam>(kN);
    for (std::size_t k = 0; k < kCampos; ++k)
    {
        std::uint64_t esperada = 0;
        for (std::size_t i = 0; i < kN; ++i)
        {
            esperada += s.get(i, k);
        }
        EXPECT_EQ(s.sum_field(k), esperada) << "campo " << k;
    }
}

// --- invariante 3 ---------------------------------------------------------
TYPED_TEST(SpecLayout, touch_record_escreve_em_todos_os_campos)
{
    auto s = preenchido<TypeParam>(kN);
    constexpr std::size_t alvo = 77;
    s.touch_record(alvo, 1000);
    for (std::size_t k = 0; k < kCampos; ++k)
    {
        EXPECT_EQ(s.get(alvo, k), 1000U + k) << "campo " << k << " nao foi escrito";
    }
}

// --- invariante 4 ---------------------------------------------------------
TYPED_TEST(SpecLayout, escrever_um_campo_nao_altera_os_outros)
{
    auto s = preenchido<TypeParam>(kN);
    constexpr std::size_t alvo = 123;
    s.set(alvo, 3, 0xDEAD'BEEF);
    for (std::size_t k = 0; k < kCampos; ++k)
    {
        if (k == 3)
        {
            EXPECT_EQ(s.get(alvo, k), 0xDEAD'BEEFU);
        }
        else
        {
            EXPECT_EQ(s.get(alvo, k), valor(alvo, k)) << "campo " << k << " foi contaminado";
        }
    }
    // E o registro vizinho não foi tocado -- que no AoS é a linha seguinte e no
    // SoA é a posição seguinte de um vetor.
    for (std::size_t k = 0; k < kCampos; ++k)
    {
        EXPECT_EQ(s.get(alvo + 1, k), valor(alvo + 1, k)) << "vizinho contaminado no campo " << k;
    }
}
