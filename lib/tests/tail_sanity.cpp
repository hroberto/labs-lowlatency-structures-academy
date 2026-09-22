// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Sanidade do collector de cauda.
//
// ESTE ALVO TEM DUAS FUNÇÕES, E A PRIMEIRA NÃO SE VÊ
//
// Compilar este arquivo avalia todos os `static_assert` de `tail.hpp` --
// inclusive o que exige que a fórmula de percentil de lá coincida com a de
// `statistics.hpp`. Uma divergência entre as duas não apareceria em teste de
// runtime nenhum: os dois números continuariam plausíveis, e duas tabelas do
// mesmo repositório passariam a ser incomparáveis em silêncio.
//
// A segunda é verificar em runtime o que `static_assert` não alcança: o
// descarte por buffer cheio, e o teto de percentil publicável.
//
// `--violate` existe para provar que o caminho de falha EXISTE. Uma verificação
// que nunca dispara é indistinguível de uma ausente, e o `should_fail` no
// meson.build é o que a exercita.

#include "lib/contract/contract.hpp"
#include "lib/measurement/statistics.hpp"
#include "lib/measurement/tail.hpp"

#include <cstdint>
#include <print>
#include <string_view>
#include <vector>

using namespace perf::measurement;

namespace
{
int failures = 0;

void check_case(bool condition, std::string_view description)
{
    if (condition)
    {
        std::println("  ok: {}", description);
        return;
    }
    std::println("  FAILED: {}", description);
    ++failures;
}
} // namespace

int main(int argc, char **argv)
{
    const bool violate = argc > 1 && std::string_view{argv[1]} == "--violate";

    // --- o descarte é declarado, não silencioso --------------------------
    {
        tail_collector c{2};
        for (int i = 0; i < 5; ++i)
        {
            c.record(1);
        }
        const tail_statistics t = c.summarize();
        check_case(t.samples == 2, "a full buffer stops recording at the capacity");
        check_case(t.discarded == 3, "the 3 samples that did not fit show up in discarded");
        check_case(c.capacity() == 2, "the capacity is the one requested, not the one the vector grew to");
    }

    // --- percentis exatos, sem precisão a declarar -----------------------
    {
        tail_collector c{10};
        for (std::uint64_t v : {9ULL, 1ULL, 8ULL, 2ULL, 7ULL, 3ULL, 6ULL, 4ULL, 5ULL, 10ULL})
        {
            c.record(v);
        }
        const tail_statistics t = c.summarize();
        check_case(t.minimum == 1 && t.maximum == 10, "minimum and maximum come from the sorted sample");
        check_case(t.p50 == 5.5, "p50 of 1..10 is 5.5 -- exact, not approximated by a bucket");
        check_case(t.samples == 10 && t.discarded == 0, "ten samples, no discards");
    }

    // --- o teto de percentil publicável ----------------------------------
    {
        tail_collector small{100};
        for (int i = 0; i < 100; ++i)
        {
            small.record(static_cast<std::uint64_t>(i));
        }
        const tail_statistics t = small.summarize();
        check_case(t.highest_supported < 0.99,
               "100 samples do not support p99 (the convention requires 1000)");
        check_case(t.p999 > 0.0,
               "p99.9 is COMPUTED even without support -- what rejects it is the ceiling, "
               "not the absence of the number");
    }
    {
        tail_collector large{min_samples_for(0.999)};
        for (std::size_t i = 0; i < min_samples_for(0.999); ++i)
        {
            large.record(static_cast<std::uint64_t>(i));
        }
        check_case(large.summarize().highest_supported == 0.999,
               "10,000 samples support p99.9");
    }

    // --- collector empty: estado, não lixo ---------------------------------
    {
        tail_collector empty{4};
        const tail_statistics t = empty.summarize();
        check_case(t.samples == 0 && t.p50 == 0.0 && t.maximum == 0,
               "an empty collector returns zeros and samples 0, not a sentinel value");
    }

    // --- as duas fórmulas de percentil concordam em runtime também -------
    //
    // O `static_assert` de tail.hpp cobre o caso constexpr. Este cobre o
    // caminho de dados real, com o mesmo conjunto passando pelas duas funções.
    {
        std::vector<double> d{10.0, 20.0, 30.0, 40.0};
        std::vector<std::uint64_t> u{10, 20, 30, 40};
        const double a = percentile(std::span<const double>{d}, 0.75);
        const double b = percentile_ns(std::span<const std::uint64_t>{u}, 0.75);
        check_case(a == b, "percentile() and percentile_ns() give the same value for the same sample");
    }

    if (violate)
    {
        // A violação é pedir publicação de um percentil que a amostra não
        // sustenta. O programa MORRE, e o meson espera que ele morra.
        tail_collector c{10};
        c.record(1);
        const tail_statistics t = c.summarize();
        PERF_EXPECTS(t.highest_supported >= 0.999);
        std::println("  FAILED: the contract did not fire with {} sample(s)", t.samples);
        return 1;
    }

    if (failures == 0)
    {
        std::println("tail: every assertion passed");
        return 0;
    }
    std::println("tail: {} assertion(s) failed", failures);
    return 1;
}
