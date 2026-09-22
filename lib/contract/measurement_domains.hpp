// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Os primeiros domínios reais do projeto: os da própria régua de medição.
//
// POR QUE ESTES, E NÃO OUTROS
//
// A biblioteca de contratos precisa de domínios concretos para que as três
// portas sejam mais que uma ideia. A escolha de QUAIS domínios é uma decisão de
// escopo, e esta é deliberada: os primeiros domínios são os da régua de
// apuração deste próprio projeto.
//
// Três razões:
//
//   1. **São conceituais.** Contagem de amostras, rodadas por amostra e
//      percentil pertencem ao método de medição, que é o objeto deste projeto.
//      Nenhum deles depende de sistema operacional, de distribuição ou de
//      versão de kernel;
//
//   2. **São autocontidos.** A biblioteca de contratos não passa a depender de
//      nada fora do repositório, e o material não precisa ensinar um assunto
//      novo antes de ensinar o contrato;
//
//   3. **Os termos de aceite já existiam, em prosa.** `statistics.hpp`
//      documentava "menos de 3 amostras não permite quartis úteis" e "abaixo de
//      1000 rodadas o relógio domina" -- e implementava os dois como número
//      solto no meio de uma chamada. Transformá-los em domínio é o exemplo mais
//      honesto possível da diretriz: o termo de aceite deixa de ser comentário
//      e passa a ser verificado.
//
// O QUE DELIBERADAMENTE NÃO ESTÁ AQUI
//
// Domínios de parâmetro de ajuste do sistema operacional -- `cpu.weight`,
// `nice`, `swappiness`, `overcommit_memory` e afins. Eles são o material da
// **etapa final** do projeto, quando houver um plano de integração com o estudo
// de otimização de baixo nível; trazê-los para a fundação misturaria aquele
// eixo com o conceitual, que é o que se está construindo agora.
//
// Ver a Etapa 6 do ROADMAP.md.

#ifndef PERF_CONTRACT_MEASUREMENT_DOMAINS_HPP
#define PERF_CONTRACT_MEASUREMENT_DOMAINS_HPP

#include "lib/contract/domain.hpp"
#include "lib/measurement/statistics.hpp"

#include <cstddef>
#include <string_view>
#include <type_traits> // std::is_same_v, nos testes ao fim do arquivo

namespace perf::contract::measurement
{

// --- Contagem de amostras -------------------------------------------------
//
// O piso é 3 porque quartis exigem pelo menos três amostras para dizer algo.
// O teto é generoso e existe só para que um valor absurdo vindo de variável de
// ambiente seja recusado em vez de alocar por horas.
struct sample_count_domain
    : closed_range<int, perf::measurement::min_samples, 1'000'000>
{
    static constexpr std::string_view name = "sample count";
};

// --- Rodadas por amostra --------------------------------------------------
//
// O piso é 1000 porque abaixo disso o custo de ler o relógio domina o custo da
// operação medida, e o número publicado passa a medir o relógio.
struct rounds_per_sample_domain
    : closed_range<int, perf::measurement::min_rounds, 1'000'000'000>
{
    static constexpr std::string_view name = "rounds per sample";
};

// --- Percentil ------------------------------------------------------------
//
// Domínio SOB MEDIDA, e não `closed_range`, porque `closed_range` só aceita
// tipo integral -- um percentil é fração. Está aqui de propósito: mostra que o
// conceito `domain` é aberto, e que um domínio novo precisa apenas de
// `value_type`, `accepts()` e `name`.
struct percentile_domain
{
    using value_type = double;

    static constexpr value_type lowest = 0.0;
    static constexpr value_type highest = 1.0;

    // Intervalo FECHADO nas duas pontas: `percentile(v, 0.0)` é o mínimo e
    // `percentile(v, 1.0)` é o máximo, e as duas chamadas são legítimas.
    PERF_PURE_PREDICATE static constexpr bool accepts(value_type p) noexcept
    {
        return p >= lowest && p <= highest;
    }

    static constexpr std::string_view name = "percentil";
};

// --- Alinhamento ----------------------------------------------------------
//
// Potência de dois, entre 16 e 4096 bytes. Entra aqui porque layout e
// alinhamento são objeto deste projeto -- e porque é o domínio que exercita
// `power_of_two_in`, cujo modo de recusa interessante é o valor que está DENTRO
// da faixa e não é potência de dois. Uma faixa simples o aceitaria.
struct alignment_domain : power_of_two_in<std::size_t, 16, 4096>
{
    static constexpr std::string_view name = "alinhamento em bytes";
};

// --- Os tipos que o resto do projeto usa ----------------------------------
//
// Cada um é um tipo DISTINTO, sem conversão implícita para o inteiro
// subjacente. `sample_count` e `rounds_per_sample` são os dois `int`, e trocar
// um pelo outro numa chamada é o erro que o tipo existe para impedir -- o
// compilador, sozinho, aceitaria a troca em silêncio.
using sample_count = checked<sample_count_domain>;
using rounds_per_sample = checked<rounds_per_sample_domain>;
using percentile = checked<percentile_domain>;
using alignment = checked<alignment_domain>;

// --- Testes em tempo de compilação ---------------------------------------

namespace
{

// A CORRESPONDÊNCIA COM A RÉGUA, que é a razão de este arquivo existir.
//
// Se alguém mudar o piso em `statistics.hpp` e não mudar o domínio, ou o
// contrário, o BUILD quebra. Sem isto, os dois números divergiriam em silêncio
// e o domínio passaria a recusar uma coleta que a régua aceita.
static_assert(sample_count_domain::lowest == perf::measurement::min_samples);
static_assert(rounds_per_sample_domain::lowest == perf::measurement::min_rounds);

// O padrão de amostras da régua precisa CABER no domínio. Um padrão fora do
// próprio domínio seria uma régua que se recusa a si mesma.
static_assert(sample_count_domain::accepts(perf::measurement::default_samples_fixed));

// Contagem de amostras: piso, acima do piso, e os dois modos de recusa.
static_assert(sample_count_domain::accepts(3));
static_assert(sample_count_domain::accepts(25));
static_assert(!sample_count_domain::accepts(2));
static_assert(!sample_count_domain::accepts(0));
static_assert(!sample_count_domain::accepts(-1));

// Rodadas por amostra: o 999 é o caso que importa, porque é o valor que alguém
// escreveria por arredondamento.
static_assert(rounds_per_sample_domain::accepts(1000));
static_assert(rounds_per_sample_domain::accepts(200'000));
static_assert(!rounds_per_sample_domain::accepts(999));

// Percentil: as duas pontas DENTRO -- é a decisão do intervalo fechado, e um
// "conserto" para intervalo aberto derruba estes dois asserts.
static_assert(percentile_domain::accepts(0.0));
static_assert(percentile_domain::accepts(0.5));
static_assert(percentile_domain::accepts(1.0));
static_assert(!percentile_domain::accepts(-0.01));
static_assert(!percentile_domain::accepts(1.01));

// Os percentis que a régua de fato publica precisam estar no domínio.
static_assert(percentile_domain::accepts(0.25));
static_assert(percentile_domain::accepts(0.75));
static_assert(percentile_domain::accepts(0.99));

// Alinhamento: limites, potência interna, e o valor DENTRO da faixa que não é
// potência de dois -- o modo de recusa que distingue este domínio de uma faixa.
static_assert(alignment_domain::accepts(16));
static_assert(alignment_domain::accepts(64));
static_assert(alignment_domain::accepts(4096));
static_assert(!alignment_domain::accepts(8));
static_assert(!alignment_domain::accepts(8192));
static_assert(!alignment_domain::accepts(100));

// --- As três portas, nos domínios reais ----------------------------------

// PORTA T: constante válida atravessa; constante inválida não compilaria.
static_assert(sample_count::of<25>().get() == 25);
static_assert(alignment::of<64>().get() == 64);
static_assert(percentile::of<0.5>().get() == 0.5);

// PORTA R: `trusted` continua avaliável em compilação COM o contrato ligado.
static_assert(rounds_per_sample::trusted(200'000).get() == 200'000);

// PORTA E: aceita, recusa, e a recusa identifica o domínio pelo nome -- que é o
// que torna o erro legível quando o valor vem de variável de ambiente.
static_assert(sample_count::parse(25).has_value());
static_assert(!sample_count::parse(2).has_value());
static_assert(sample_count::parse(2).error().offending == 2);
static_assert(sample_count::parse(2).error().domain_name == "sample count");

// A SEPARAÇÃO DE TIPOS: `sample_count` e `rounds_per_sample` compartilham o
// tipo subjacente e têm faixas que se sobrepõem. Sem tipos distintos, o
// compilador aceitaria trocar um pelo outro em silêncio.
static_assert(!std::is_same_v<sample_count, rounds_per_sample>);
static_assert(std::is_same_v<sample_count::value_type, rounds_per_sample::value_type>);

} // namespace

} // namespace perf::contract::measurement

#endif // PERF_CONTRACT_MEASUREMENT_DOMAINS_HPP
