// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Orçamento de carga declarado em tempo de compilação.
//
// POR QUE ISTO EXISTE
//
// Otimização sem critério de parada não termina. O projeto irmão resolve isso
// abrindo pela pergunta "quanto tempo existe por unidade de trabalho", com o
// número derivado de norma -- quadro IEEE 802.3 mais intervalo entre quadros,
// na taxa do enlace. É o que transforma "deixar rápido" em "caber no
// orçamento".
//
// Aqui não há taxa de enlace, então o orçamento é DECLARADO. E, sendo
// declarado, não há razão para ele ser descoberto em runtime: vira constante de
// compilação, e a pergunta "este resultado cabe?" passa a ser respondível por
// `static_assert` quando o número é conhecido.
//
// Norma correspondente: docs/padrao-do-projeto.md, seção 10. **Todo módulo com
// experimento declara o orçamento antes de medir.** Um resultado só é "bom" ou
// "ruim" contra um orçamento; sem ele, só é diferente.
//
// O QUE ISTO NÃO É
//
// Não é um limite que o programa impõe a si mesmo em runtime. É a régua contra
// a qual um resultado medido é julgado, e o julgamento acontece no documento,
// não no processo. A aplicação disso a um programa que se RECUSA a publicar
// latência quando o ambiente não honra o orçamento é a Etapa 6 do ROADMAP, e
// depende de ler o ambiente -- que é porta E, não porta T.

#ifndef PERF_CONTRACT_BUDGET_HPP
#define PERF_CONTRACT_BUDGET_HPP

#include "lib/contract/contract.hpp"

#include <cstdint>
// `std::is_same_v`, nos testes ao fim do arquivo. Vinha por inclusão
// transitiva de `<print>` e compilava -- o que é exatamente o tipo de
// dependência que quebra quando outra biblioteca padrão reorganiza os
// cabeçalhos internos dela.
#include <type_traits>

namespace perf::contract
{

// Orçamento derivado de uma taxa de eventos declarada.
//
// `EventsPerSecond` é parâmetro de template e não argumento de construtor de
// propósito: é o que permite que `fits()` seja avaliado em compilação sempre
// que o valor medido também for constante, e é o que faz de dois orçamentos
// diferentes DOIS TIPOS diferentes -- um `workload_contract<5'000'000>` não se
// confunde com um `workload_contract<100'000>` em nenhuma assinatura.
template <std::uint64_t EventsPerSecond>
struct workload_contract
{
    // Carga nula não define orçamento: dividiria por zero e produziria um
    // orçamento infinito, contra o qual qualquer resultado "cabe". Um orçamento
    // que aceita tudo é pior que nenhum, porque parece um critério.
    static_assert(EventsPerSecond > 0, "carga nula nao define orcamento");

    static constexpr std::uint64_t events_per_second = EventsPerSecond;

    // Nanossegundos disponíveis por evento.
    static constexpr double budget_ns = 1e9 / static_cast<double>(EventsPerSecond);

    // O valor medido cabe no orçamento?
    //
    // `<=` e não `<`: gastar exatamente o orçamento é caber nele. A escolha é
    // arbitrária no limite e precisa estar escrita, porque um leitor que suponha
    // `<` interpretará "no orçamento" de forma diferente da do código.
    PERF_PURE_PREDICATE static constexpr bool fits(double measured_ns) noexcept
    {
        return measured_ns <= budget_ns;
    }

    // Quanto do orçamento o valor medido consome, em percentual.
    //
    // Existe separado de `fits()` porque a informação útil raramente é binária:
    // "cabe" e "cabe com 3% de folga" levam a decisões de arquitetura
    // diferentes, e é a segunda que entra na tabela de resultados.
    PERF_PURE_PREDICATE static constexpr double usage_percent(double measured_ns) noexcept
    {
        return 100.0 * measured_ns / budget_ns;
    }
};

// --- Testes em tempo de compilação ---------------------------------------

namespace
{

using feed_5m = workload_contract<5'000'000>;

static_assert(feed_5m::budget_ns == 200.0);
static_assert(feed_5m::fits(199.0));
static_assert(feed_5m::fits(200.0)); // o limite CABE -- a decisão do `<=`
static_assert(!feed_5m::fits(200.1));
static_assert(feed_5m::usage_percent(100.0) == 50.0);
static_assert(feed_5m::usage_percent(200.0) == 100.0);

// Uma taxa dez vezes menor dá um orçamento dez vezes maior. Trivial, e é o tipo
// de relação que uma inversão de sinal na fórmula quebraria em silêncio.
using feed_500k = workload_contract<500'000>;
static_assert(feed_500k::budget_ns == 2000.0);
static_assert(feed_500k::fits(200.0));

// Orçamentos diferentes são TIPOS diferentes: é o que impede julgar um
// resultado contra a régua de outra carga.
static_assert(!std::is_same_v<feed_5m, feed_500k>);

} // namespace

} // namespace perf::contract

#endif // PERF_CONTRACT_BUDGET_HPP
