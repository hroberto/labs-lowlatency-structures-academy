// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Leitura do relógio monotônico, em um lugar só.
//
// ORIGEM
//
// Portado de `docs/01-fundamentos/medicoes/clock_ns.h` do projeto irmão
// labs-dpdk-academy. Lá o cabeçalho existe porque `now_ns()` estava definida
// dez vezes, em três variantes de tipo de retorno, e **nenhuma das dez conferia
// o retorno de `clock_gettime()`** -- quando ela falha, a `struct timespec`
// fica não inicializada e a função devolve lixo de pilha, que vira intervalo
// medido, entra na estatística e sai publicado como tempo.
//
// O QUE MUDA NA PORTA PARA C++23, E POR QUÊ
//
// O cabeçalho de origem carrega uma lição MEDIDA que não transfere para cá, e
// ela não pode ser descartada em silêncio: lá, o `abort()` embutido na função
// inline degradava a otimização do laço chamador -- `custo-anel.c` ficou 17%
// mais lento no lote 32 e 28% no lote 128, em dez execuções por ponto. A
// correção foi marcar o caminho de erro com `cold` e `noinline`.
//
// Aqui esse caminho **não existe**. `std::chrono::steady_clock::now()` não tem
// canal de erro: a especificação não lhe dá um, e na libstdc++ sobre Linux ela
// resolve para o mesmo `clock_gettime(CLOCK_MONOTONIC)` sem expor o retorno.
// Não há verificação a fazer, logo não há tratamento a esfriar, logo o defeito
// de origem -- lixo de pilha publicado como tempo -- é impossível de escrever.
//
// Isto é um ganho real do porte, e é o tipo de ganho que este projeto estuda:
// não "C++ é mais rápido", mas **a fronteira de tipo eliminou uma classe de
// defeito em vez de exigir disciplina para evitá-la**. O custo da eliminação é
// perder o controle explícito sobre qual relógio se lê; ver a nota abaixo.
//
// QUAL RELÓGIO, E POR QUE NÃO `system_clock` NEM `high_resolution_clock`
//
//   steady_clock          - monotônico por garantia do padrão. É este.
//   system_clock          - pode andar para trás (NTP, ajuste manual). Um
//                           intervalo medido com ele pode sair negativo.
//   high_resolution_clock - apelido de um dos dois, e QUAL deles é decisão de
//                           implementação. Um relógio cuja identidade depende
//                           da biblioteca não serve para medição publicável.
//
// O nome é atraente e a semântica é indefinida; por isso não aparece aqui.

#ifndef PERF_MEASUREMENT_CLOCK_HPP
#define PERF_MEASUREMENT_CLOCK_HPP

#include <chrono>
#include <cstdint>

namespace perf::measurement
{

// DUAS ASSINATURAS, DE PROPÓSITO -- a mesma decisão do cabeçalho de origem.
//
// `now_ns()` devolve inteiro: é a leitura exata, sem perda.
// `now_ns_d()` devolve `double`, para os laços que calculam médias e dividem,
// evitando conversão espalhada no ponto de uso.
//
// As duas leem o mesmo relógio pela mesma chamada; a diferença é só o tipo na
// fronteira. Ter as duas é deliberado: foi a AUSÊNCIA de uma decisão como esta
// que produziu as três variantes divergentes no projeto de origem.
[[nodiscard]] inline std::uint64_t now_ns() noexcept
{
    const auto t = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t).count());
}

[[nodiscard]] inline double now_ns_d() noexcept
{
    return static_cast<double>(now_ns());
}

// A resolução do relógio é um DADO DA MEDIÇÃO, não trivialidade: ela decide
// qual é o menor custo publicável. Um resultado cuja mediana é da ordem da
// resolução não mede a operação, mede o relógio -- e `statistics.hpp` trata
// esse caso como estado próprio (`below_resolution`), não como zero.
//
// `period` é constante de compilação, então isto é `consteval`-amigável e entra
// em `static_assert` sem custo de runtime.
[[nodiscard]] constexpr double clock_period_ns() noexcept
{
    using period = std::chrono::steady_clock::period;
    return 1e9 * static_cast<double>(period::num) / static_cast<double>(period::den);
}

// Termo de aceite do próprio relógio. Um relógio com período maior que 1 us
// não sustenta microbenchmark, e isso é decidível em compilação -- portanto é
// `static_assert`, não verificação de runtime. É a primeira aplicação da regra
// de contratos deste projeto: o que é constante vira erro de compilação.
static_assert(clock_period_ns() > 0.0, "relogio sem periodo positivo");
static_assert(clock_period_ns() <= 1000.0,
              "steady_clock com resolucao pior que 1 us: microbenchmark nao e publicavel");

} // namespace perf::measurement

#endif // PERF_MEASUREMENT_CLOCK_HPP
