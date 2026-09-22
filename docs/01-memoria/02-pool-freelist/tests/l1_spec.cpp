// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// L1 — as cinco invariantes da spec do 01.02, nos dois braços.
//
// A DIFERENÇA PARA O L1 DO 01.01 ESTÁ NA PRIMEIRA INVARIANTE
//
// Lá, "cada `acquire` devolve ponteiro distinto" valia para sempre: o lote não
// libera nada até o fim. Aqui, um slot liberado **deve** voltar a ser servido --
// é a invariante 4 --, então dois ponteiros iguais em momentos diferentes são
// corretos, e dois ponteiros iguais ao mesmo tempo são defeito.
//
// A invariante 1 passou a ser "distintos ENQUANTO VIVOS", e o teste precisa
// acompanhar o conjunto vivo em vez de acumular tudo. Uma cópia do teste do
// 01.01 passaria aqui e não verificaria nada do que interessa.

#include "docs/01-memoria/02-pool-freelist/custom/pool_arm.hpp"
#include "docs/01-memoria/02-pool-freelist/spec.hpp"
#include "docs/01-memoria/02-pool-freelist/std/heap_churn_arm.hpp"

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
class SpecChurn : public ::testing::Test
{
};

using Bracos = ::testing::Types<heap_churn_arm, pool_arm>;
TYPED_TEST_SUITE(SpecChurn, Bracos);

// --- invariante 1 ---------------------------------------------------------
TYPED_TEST(SpecChurn, dois_objetos_vivos_nunca_sao_o_mesmo_endereco)
{
    auto a = criar<TypeParam>();
    std::set<node *> vivos;
    std::vector<node *> lista;

    // Regime: metade da capacidade viva, e rotatividade em cima disso.
    for (std::size_t i = 0; i < kCapacidade / 2; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        ASSERT_TRUE(vivos.insert(p).second) << "endereco repetido entre vivos, posicao " << i;
        lista.push_back(p);
    }
    for (std::size_t passo = 0; passo < kCapacidade * 4; ++passo)
    {
        // Libera um do meio -- e não o último -- para que a ordem de liberação
        // não seja a de aquisição. É o padrão que a arena não atende.
        const std::size_t alvo = (passo * 7) % lista.size();
        node *velho = lista[alvo];
        vivos.erase(velho);
        a.release(velho);

        node *novo = a.acquire();
        ASSERT_NE(novo, nullptr) << "esgotou em regime, passo " << passo;
        ASSERT_TRUE(vivos.insert(novo).second)
            << "endereco repetido entre vivos, passo " << passo;
        lista[alvo] = novo;
    }
    for (node *p : lista)
    {
        a.release(p);
    }
    EXPECT_EQ(a.outstanding(), 0U);
}

// --- invariante 2 ---------------------------------------------------------
TYPED_TEST(SpecChurn, o_que_se_escreve_no_objeto_volta_a_ser_lido)
{
    auto a = criar<TypeParam>();
    std::vector<node *> lista;
    for (std::size_t i = 0; i < 32; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        p->id = 0x5A5A'0000ULL + i;
        lista.push_back(p);
    }
    for (std::size_t i = 0; i < lista.size(); ++i)
    {
        EXPECT_EQ(lista[i]->id, 0x5A5A'0000ULL + i);
    }
    for (node *p : lista)
    {
        a.release(p);
    }
}

// --- invariante 3 ---------------------------------------------------------
TYPED_TEST(SpecChurn, o_ponteiro_respeita_o_alinhamento_do_no)
{
    auto a = criar<TypeParam>();
    std::vector<node *> lista;
    for (std::size_t i = 0; i < 32; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % alignof(node), 0U);
        lista.push_back(p);
    }
    for (node *p : lista)
    {
        a.release(p);
    }
}

// --- invariante 4: a que o 01.01 não tinha --------------------------------
TYPED_TEST(SpecChurn, slot_liberado_volta_a_ser_servido)
{
    auto a = criar<TypeParam>();
    std::vector<node *> lista;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        lista.push_back(p);
    }
    // Com tudo vivo, um braço com teto não tem o que servir.
    if (TypeParam::traits.bounded)
    {
        EXPECT_EQ(a.acquire(), nullptr);
    }
    a.release(lista.back());
    lista.pop_back();

    node *reaproveitado = a.acquire();
    ASSERT_NE(reaproveitado, nullptr) << "liberou um e nao serviu o proximo";
    lista.push_back(reaproveitado);
    EXPECT_EQ(a.outstanding(), kCapacidade);

    for (node *p : lista)
    {
        a.release(p);
    }
}

// --- invariante 5: SÓ para quem declara teto ------------------------------
TYPED_TEST(SpecChurn, teto_acabado_devolve_nullptr)
{
    if (!TypeParam::traits.bounded)
    {
        GTEST_SKIP() << TypeParam::traits.name
                     << " nao declara teto: quando a memoria acaba ele LANCA. A invariante "
                        "5 nao se aplica, e isto e o mesmo achado do topico 01.01.";
    }
    auto a = criar<TypeParam>();
    std::vector<node *> lista;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        lista.push_back(a.acquire());
    }
    EXPECT_EQ(a.acquire(), nullptr);
    EXPECT_EQ(a.acquire(), nullptr) << "esgotamento nao pode ser estado transitorio";
    for (node *p : lista)
    {
        a.release(p);
    }
    // E depois de devolver tudo, ele volta a servir a capacidade inteira.
    EXPECT_NE(a.acquire(), nullptr) << "nao se recuperou do esgotamento";
}
