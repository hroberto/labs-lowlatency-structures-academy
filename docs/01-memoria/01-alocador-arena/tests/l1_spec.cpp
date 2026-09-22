// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// L1 — as sete invariantes da spec, rodando sobre os DOIS braços.
//
// POR QUE GOOGLETEST ENTRA AQUI, E NÃO ANTES
//
// Os programas de sanidade de `lib/tests/` são um teste do Meson com onze casos
// dentro: uma regressão em um deles reporta "1 de N". Isso incomodava; não
// pagava um subprojeto.
//
// Este arquivo é outra coisa. Ele roda **o mesmo conjunto de invariantes sobre
// dois tipos diferentes**, e é para isso que `TYPED_TEST_SUITE` existe. Escrever
// à mão significaria um laço sobre `std::variant`, ou duas cópias do arquivo --
// e duas cópias divergem. Com `protocol: 'gtest'`, o Meson conta cada caso de
// cada tipo, e a suíte passa a medir o que verifica.
//
// NOME DE CASO EM PORTUGUÊS
//
// A norma manda identificador em inglês e prosa em português. Nome de caso de
// teste é **prosa**: ele é a frase que aparece quando o caso falha, e quem a lê
// é quem escreve o material.
//
// A INVARIANTE QUE UM BRAÇO NÃO HONRA É CASO PULADO COM MOTIVO
//
// `GTEST_SKIP()` com a razão escrita, e não `#if` nem caso ausente. A diferença
// importa: um caso ausente some da contagem, e some em silêncio. Um caso pulado
// aparece no relatório dizendo por que não rodou, que é exatamente o que a
// divergência entre os braços é -- um achado do tópico.

#include "docs/01-memoria/01-alocador-arena/custom/arena_arm.hpp"
#include "docs/01-memoria/01-alocador-arena/spec.hpp"
#include "docs/01-memoria/01-alocador-arena/std/heap_arm.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <vector>

using namespace academy::memory;

namespace
{
constexpr std::size_t kCapacidade = 256;

// Fábrica: a arena precisa de capacidade no construtor, o heap ignora.
template <typename A>
A criar()
{
    return A(kCapacidade);
}
} // namespace

template <typename A>
class Spec : public ::testing::Test
{
};

using Bracos = ::testing::Types<heap_arm, arena_arm>;
TYPED_TEST_SUITE(Spec, Bracos);

// --- invariante 1 ---------------------------------------------------------
TYPED_TEST(Spec, cada_acquire_devolve_ponteiro_distinto)
{
    auto a = criar<TypeParam>();
    std::set<node *> vistos;
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr) << "esgotou antes da capacidade declarada";
        EXPECT_TRUE(vistos.insert(p).second) << "ponteiro repetido na posicao " << i;
        guardados.push_back(p);
    }
    a.release_all(guardados);
}

// --- invariante 2 ---------------------------------------------------------
TYPED_TEST(Spec, o_que_se_escreve_no_objeto_volta_a_ser_lido)
{
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < 16; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr);
        p->id = 0xA5A5'0000ULL + i;
        p->payload[6] = i;
        guardados.push_back(p);
    }
    for (std::size_t i = 0; i < guardados.size(); ++i)
    {
        EXPECT_EQ(guardados[i]->id, 0xA5A5'0000ULL + i);
        EXPECT_EQ(guardados[i]->payload[6], i);
    }
    a.release_all(guardados);
}

// --- invariante 3 ---------------------------------------------------------
TYPED_TEST(Spec, o_ponteiro_respeita_o_alinhamento_do_no)
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

// --- invariante 4 ---------------------------------------------------------
TYPED_TEST(Spec, release_all_zera_o_que_esta_em_uso)
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

// --- invariante 5 ---------------------------------------------------------
TYPED_TEST(Spec, depois_de_release_all_o_acquire_volta_a_servir)
{
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        guardados.push_back(a.acquire());
    }
    a.release_all(guardados);
    guardados.clear();

    // A rodada seguinte tem de caber inteira. Numa arena que não zerasse o
    // índice, esta é a primeira que falharia -- e falharia no `nullptr`.
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        node *p = a.acquire();
        ASSERT_NE(p, nullptr) << "nao serviu na segunda rodada, posicao " << i;
        guardados.push_back(p);
    }
    a.release_all(guardados);
}

// --- invariante 6: SÓ para quem declara teto ------------------------------
TYPED_TEST(Spec, teto_acabado_devolve_nullptr)
{
    if (!TypeParam::traits.bounded)
    {
        GTEST_SKIP() << TypeParam::traits.name
                     << " nao declara teto: quando a memoria acaba ele LANCA, em vez de "
                        "devolver nullptr. A invariante 6 nao se aplica, e isto e um "
                        "achado do topico, nao uma lacuna do teste.";
    }
    auto a = criar<TypeParam>();
    std::vector<node *> guardados;
    for (std::size_t i = 0; i < kCapacidade; ++i)
    {
        guardados.push_back(a.acquire());
    }
    EXPECT_EQ(a.acquire(), nullptr) << "serviu alem da capacidade declarada";
    // E continua devolvendo nullptr: o esgotamento não é estado transitório.
    EXPECT_EQ(a.acquire(), nullptr);
    a.release_all(guardados);
}

// --- invariante 7: SÓ para quem libera individual -------------------------
TYPED_TEST(Spec, libera_objeto_individual)
{
    if (!TypeParam::traits.frees_individually)
    {
        GTEST_SKIP() << TypeParam::traits.name
                     << " nao libera objeto individual: e a troca que ele faz para alocar "
                        "somando um indice. A invariante 7 nao se aplica, e a *Regra de "
                        "decisao* do topico e onde essa troca vira numero.";
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
