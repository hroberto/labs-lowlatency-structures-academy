// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// L1 — as sete invariantes do 01.01, agora sobre TRÊS braços.
//
// POR QUE OS TRÊS, E NÃO SÓ O NOVO
//
// A spec é a mesma do 01.01, e a lista de tipos é o lugar onde isso deixa de
// ser afirmação e vira verificação: se a `pmr_arm` cumprisse uma lei
// ligeiramente diferente, este arquivo não compilaria.
//
// Rodar os três também mede o que interessa ao tópico: a `pmr_arm` pula
// exatamente os mesmos casos que a `arena_arm` -- invariante 7, liberação
// individual --, e isso é o resultado, não o teste. A biblioteca padrão oferece
// o mesmo mecanismo, com as mesmas renúncias.

#include "docs/01-memoria/01-alocador-arena/custom/arena_arm.hpp"
#include "docs/01-memoria/01-alocador-arena/std/heap_arm.hpp"
#include "docs/01-memoria/03-pmr-o-meio-termo/custom/pmr_arm.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <vector>

using namespace academy::memory;

namespace
{
constexpr std::size_t kCapacidade = 256;

template <typename A>
A criar()
{
    return A(kCapacidade);
}
} // namespace

template <typename A>
class SpecPmr : public ::testing::Test
{
};

using Bracos = ::testing::Types<heap_arm, arena_arm, pmr_arm>;
TYPED_TEST_SUITE(SpecPmr, Bracos);

TYPED_TEST(SpecPmr, cada_acquire_devolve_ponteiro_distinto)
{
    auto a = criar<TypeParam>();
    std::set<node *> vistos;
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr) << "esgotou antes da capacidade declarada, posicao " << i;
        EXPECT_TRUE(vistos.insert(p).second) << "ponteiro repetido na posicao " << i;
        guardados.push_back(p);
    }
    a.release_all(guardados);
}

TYPED_TEST(SpecPmr, o_que_se_escreve_no_objeto_volta_a_ser_lido)
{
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < 16; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        p->id = 0xC3C3'0000ULL + i;
        guardados.push_back(p);
    }
    for (std::size_t i = 0; i < guardados.size(); ++i)
    {
        EXPECT_EQ(guardados[i]->id, 0xC3C3'0000ULL + i);
    }
    a.release_all(guardados);
}

TYPED_TEST(SpecPmr, o_ponteiro_respeita_o_alinhamento_do_no)
{
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < 16; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % alignof(node), 0U)
            << "objeto " << i << " atravessa linha de cache";
        guardados.push_back(p);
    }
    a.release_all(guardados);
}

TYPED_TEST(SpecPmr, release_all_zera_o_que_esta_em_uso)
{
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < 32; ++i)
    {
        guardados.push_back(a.acquire());
    }
    EXPECT_EQ(a.outstanding(), 32U);
    a.release_all(guardados);
    EXPECT_EQ(a.outstanding(), 0U);
}

TYPED_TEST(SpecPmr, depois_de_release_all_o_acquire_volta_a_servir)
{
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        guardados.push_back(a.acquire());
    }
    a.release_all(guardados);
    guardados.clear();
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr) << "nao serviu na segunda rodada, posicao " << i;
        guardados.push_back(p);
    }
    a.release_all(guardados);
}

TYPED_TEST(SpecPmr, teto_acabado_devolve_nullptr)
{
    if (!TypeParam::traits.bounded)
    {
        GTEST_SKIP() << TypeParam::traits.name
                     << " nao declara teto: quando a memoria acaba ele LANCA.";
    }
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        guardados.push_back(a.acquire());
    }
    EXPECT_EQ(a.acquire(), nullptr) << "serviu alem da capacidade declarada";
    EXPECT_EQ(a.acquire(), nullptr) << "esgotamento nao pode ser estado transitorio";
    a.release_all(guardados);
}

TYPED_TEST(SpecPmr, libera_objeto_individual)
{
    if (!TypeParam::traits.frees_individually)
    {
        GTEST_SKIP() << TypeParam::traits.name
                     << " nao libera objeto individual. No caso da pmr_arm isto e o "
                        "RESULTADO do topico: a biblioteca padrao oferece o mesmo "
                        "mecanismo da arena, com as mesmas renuncias.";
    }
    auto a = criar<TypeParam>();
    node *p = a.acquire();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(a.outstanding(), 1U);
    if constexpr (requires(TypeParam &x, node *q) { x.release(q); })
    {
        a.release(p);
        EXPECT_EQ(a.outstanding(), 0U);
    }
}
