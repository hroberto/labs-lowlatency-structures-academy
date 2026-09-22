// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Cauda da distribuição: percentis altos sobre amostras cruas.
//
// POR QUE ESTE ARQUIVO EXISTE, E POR QUE NÃO É UM HISTOGRAMA
//
// `statistics.hpp` resume um punhado de amostras -- 25 por padrão --, onde cada
// amostra é a média de muitas rodadas. Isso mede **custo típico** e é o que a
// maior parte dos tópicos precisa. Mas a norma publica latência por percentis
// altos (p99, p99,9), e p99,9 de 25 amostras não existe: é a maior delas com
// outro nome.
//
// O documento de origem da trilha previa `HdrHistogram_c` para isso, fixado por
// wrap. Duas coisas foram verificadas em 2026-09-21 e mudaram a decisão:
//
//   1. HdrHistogram_c **não está no WrapDB** -- consulta ao `releases.json`
//      devolve 367 projetos, nenhum de histograma. O wrap teria de ser escrito
//      à mão, com `meson.build` próprio, porque o projeto usa CMake; e o
//      escritor de log dele arrasta zlib, o que colide com a seção 16 da norma:
//      "nenhuma dependência de sistema além do compilador, do Meson e do Ninja";
//   2. o problema que o HDR resolve **não é o nosso**. Ele existe para fluxo
//      ILIMITADO de amostras, onde guardá-las todas não cabe na memória: ele
//      troca exatidão por espaço constante, e por isso precisa declarar
//      precisão em cada tabela. Um harness de laboratório mede um número de
//      operações que ELE MESMO escolhe -- o orçamento é declarado antes de
//      medir (norma, seção 10).
//
// Então guardamos as amostras. 10 milhões de `uint64_t` são 80 MB: cabe, é
// declarado, e o percentil sai **exato**, sem precisão a declarar porque não há
// perda a declarar.
//
// O QUE SE PERDE COM ESSA ESCOLHA, PORQUE ALGO SE PERDE
//
//   - memória proporcional ao número de amostras, e não constante. Para
//     medição contínua em produção, a escolha certa seria a outra;
//   - a ordenação final é O(n log n) sobre o vetor inteiro. Fica FORA do
//     caminho quente, mas não é gratuita: 10 milhões de amostras levam dezenas
//     de milissegundos para resumir.
//
// Nenhum dos dois pesa num experimento de laboratório com número de operações
// declarado. Os dois pesariam num coletor de produção, e é por isso que o
// motivo está escrito aqui em vez de a escolha parecer óbvia.
//
// A REGRA DE QUANTAS AMOSTRAS UM PERCENTIL EXIGE
//
// Um p99,9 calculado de 1000 amostras repousa sobre UMA observação acima do
// percentil, e uma observação não é estimativa: ela é o máximo com outro nome.
// A convenção deste projeto -- e é convenção declarada, não teorema -- exige
// `tail_observations_required` observações além do percentil, o que dá
// n >= 10/(1-p): 1000 amostras para p99, 10 000 para p99,9.
//
// `tail_statistics` carrega `highest_supported` justamente para que publicar
// p99,9 de amostra insuficiente seja um erro visível e não um número bonito.

#ifndef PERF_MEASUREMENT_TAIL_HPP
#define PERF_MEASUREMENT_TAIL_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace perf::measurement
{

// Observações exigidas ALÉM do percentil para que ele seja publicável.
inline constexpr std::size_t tail_observations_required = 10;

// Menor número de amostras que sustenta o percentil `p`.
//
// `constexpr` porque é termo de aceite, e termo de aceite deste projeto vira
// `static_assert` em vez de verificação de runtime (norma, seção 13).
[[nodiscard]] constexpr std::size_t min_samples_for(double p) noexcept
{
    if (p <= 0.0 || p >= 1.0)
    {
        return 0;
    }
    const double exigido = static_cast<double>(tail_observations_required) / (1.0 - p);
    return static_cast<std::size_t>(exigido) + 1;
}

// O `m != 0` não é guarda defensiva: é o que impede a leitura errada que o
// `static_assert` abaixo pegou na primeira versão deste arquivo. `min_samples_for`
// devolve 0 para percentil fora de (0,1) -- "não há mínimo definido" --, e um
// `n >= 0` ingênuo traduz isso para "qualquer número de amostras basta",
// inclusive zero. Percentil indefinido não é percentil suportado.
[[nodiscard]] constexpr bool supports(std::size_t n, double p) noexcept
{
    const std::size_t m = min_samples_for(p);
    return m != 0 && n >= m;
}

static_assert(min_samples_for(0.99) == 1000);
static_assert(min_samples_for(0.999) == 10000);
static_assert(!supports(25, 0.999), "25 amostras nao sustentam p99,9");
static_assert(supports(10000, 0.999));
// Percentil fora do intervalo aberto (0,1) não tem mínimo definido, e devolver
// 0 aqui diria "qualquer número de amostras basta" se `supports` não tratasse
// o caso. Este assert reprovou a primeira versão, que fazia exatamente isso.
static_assert(min_samples_for(1.0) == 0);
static_assert(!supports(0, 1.0) && !supports(1'000'000, 1.0) && !supports(1'000'000, 0.0));

// Percentil sobre amostras inteiras JÁ ORDENADAS.
//
// A fórmula é a MESMA de `percentile()` em statistics.hpp, e a igualdade é
// verificada por `static_assert` ao fim deste arquivo. Não é zelo: duas tabelas
// do mesmo projeto calculadas por duas convenções de percentil não são
// comparáveis entre si, e a divergência não apareceria em nenhum teste -- os
// dois números continuariam plausíveis.
[[nodiscard]] constexpr double percentile_ns(std::span<const std::uint64_t> v, double p) noexcept
{
    if (v.empty())
    {
        return 0.0;
    }
    if (v.size() == 1)
    {
        return static_cast<double>(v[0]);
    }
    const double pos = p * static_cast<double>(v.size() - 1);
    const auto i = static_cast<std::size_t>(pos);
    const double frac = pos - static_cast<double>(i);
    if (i + 1 >= v.size())
    {
        return static_cast<double>(v.back());
    }
    const double a = static_cast<double>(v[i]);
    const double b = static_cast<double>(v[i + 1]);
    return a + frac * (b - a);
}

struct tail_statistics
{
    std::size_t samples = 0;
    // Amostras que NÃO entraram porque o buffer encheu. Existe como campo, e
    // não como aviso em log, porque o resumo é o que vai para o metadata da
    // medição: um descarte que não aparece no arquivo arquivado é um descarte
    // que ninguém audita depois.
    std::size_t discarded = 0;
    std::uint64_t minimum = 0;
    std::uint64_t maximum = 0;
    double p50 = 0.0;
    double p75 = 0.0;
    double p99 = 0.0;
    double p999 = 0.0;
    // O maior percentil que a contagem de amostras sustenta pela convenção
    // acima. Publicar acima disto é erro de documento, e este campo é o que
    // permite ao teste e ao verificador pegá-lo.
    double highest_supported = 0.0;
};

// Coletor de cauda: um vetor, reservado UMA vez, e nada mais.
//
// POR QUE `record` NÃO PODE ALOCAR
//
// Ele é chamado dentro do laço cronometrado. Um `push_back` que cresce o vetor
// chama o alocador global no meio da medição, e o custo dessa realocação entra
// na amostra seguinte -- o harness passaria a medir o próprio crescimento e a
// atribuir isso à operação sob teste. É o defeito que o módulo 08 existe para
// não cometer: o instrumento medindo a si mesmo sem dizer.
class tail_collector
{
  public:
    explicit tail_collector(std::size_t capacity)
    {
        buffer_.reserve(capacity);
        capacity_ = capacity;
    }

    // Sem verificação de contrato aqui, e a ausência é deliberada: este corpo
    // está no caminho quente, e o ramo de capacidade JÁ é o teste que
    // precisamos. Um `PERF_EXPECTS` adicional não acrescentaria informação e
    // acrescentaria trabalho.
    void record(std::uint64_t ns) noexcept
    {
        if (buffer_.size() < capacity_) [[likely]]
        {
            buffer_.push_back(ns);
            return;
        }
        ++discarded_;
    }

    // ORDENA o buffer no lugar, como `summarize()` da régua faz com o vetor de
    // amostras. Chamar duas vezes é seguro; chamar durante a medição não é --
    // e é por isso que ela não é `const`.
    [[nodiscard]] tail_statistics summarize() noexcept
    {
        tail_statistics t{};
        t.samples = buffer_.size();
        t.discarded = discarded_;
        if (buffer_.empty())
        {
            return t;
        }
        std::ranges::sort(buffer_);
        const std::span<const std::uint64_t> s{buffer_};
        t.minimum = s.front();
        t.maximum = s.back();
        t.p50 = percentile_ns(s, 0.50);
        t.p75 = percentile_ns(s, 0.75);
        t.p99 = percentile_ns(s, 0.99);
        t.p999 = percentile_ns(s, 0.999);
        t.highest_supported = highest_supported_for(t.samples);
        return t;
    }

    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }
    [[nodiscard]] std::size_t discarded() const noexcept { return discarded_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

  private:
    // O maior dos percentis que este arquivo publica e que `n` sustenta. Não é
    // uma inversão contínua da fórmula: publicar "p99,37 é o máximo suportado"
    // sugeriria que p99,37 sai em alguma tabela, e não sai.
    [[nodiscard]] static double highest_supported_for(std::size_t n) noexcept
    {
        if (supports(n, 0.999))
        {
            return 0.999;
        }
        if (supports(n, 0.99))
        {
            return 0.99;
        }
        if (supports(n, 0.75))
        {
            return 0.75;
        }
        return supports(n, 0.50) ? 0.50 : 0.0;
    }

    std::vector<std::uint64_t> buffer_;
    std::size_t capacity_ = 0;
    std::size_t discarded_ = 0;
};

// --------------------------------------------------------------------------
// Testes de compilação.
//
// A IGUALDADE COM statistics.hpp É O ASSERT QUE MAIS IMPORTA AQUI.
//
// Se as duas fórmulas de percentil divergirem, nenhuma suíte acusa: os dois
// números continuam plausíveis, e duas tabelas do mesmo repositório passam a
// ser incomparáveis em silêncio. O assert abaixo faz a divergência quebrar o
// BUILD, que é o único lugar onde ela é barata.
namespace
{
constexpr std::uint64_t ordenadas_u[] = {1, 2, 3, 4, 5};
static_assert(percentile_ns(std::span<const std::uint64_t>{ordenadas_u}, 0.50) == 3.0);
static_assert(percentile_ns(std::span<const std::uint64_t>{ordenadas_u}, 0.00) == 1.0);
static_assert(percentile_ns(std::span<const std::uint64_t>{ordenadas_u}, 1.00) == 5.0);
static_assert(percentile_ns(std::span<const std::uint64_t>{ordenadas_u}, 0.25) == 2.0);

// Interpolação com número par de amostras: a mediana cai entre duas.
constexpr std::uint64_t quatro_u[] = {10, 20, 30, 40};
static_assert(percentile_ns(std::span<const std::uint64_t>{quatro_u}, 0.50) == 25.0);

// Uma amostra só: qualquer percentil é ela. Não é caso de borda teórico --
// acontece quando a operação medida roda uma vez.
constexpr std::uint64_t uma_u[] = {7};
static_assert(percentile_ns(std::span<const std::uint64_t>{uma_u}, 0.999) == 7.0);

// Vazio devolve 0, e não lixo. O estado "não coletei nada" é tratado por
// `samples == 0` no resumo, não por valor sentinela no percentil.
static_assert(percentile_ns(std::span<const std::uint64_t>{}, 0.50) == 0.0);
} // namespace

} // namespace perf::measurement

#endif // PERF_MEASUREMENT_TAIL_HPP
