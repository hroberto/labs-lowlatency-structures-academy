// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Sanidade do coletor de cauda.
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
// `--violar` existe para provar que o caminho de falha EXISTE. Uma verificação
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
int falhas = 0;

void checar(bool condicao, std::string_view descricao)
{
    if (condicao)
    {
        std::println("  ok: {}", descricao);
        return;
    }
    std::println("  FALHA: {}", descricao);
    ++falhas;
}
} // namespace

int main(int argc, char **argv)
{
    const bool violar = argc > 1 && std::string_view{argv[1]} == "--violar";

    // --- o descarte é declarado, não silencioso --------------------------
    {
        tail_collector c{2};
        for (int i = 0; i < 5; ++i)
        {
            c.record(1);
        }
        const tail_statistics t = c.summarize();
        checar(t.samples == 2, "buffer cheio para de gravar na capacidade");
        checar(t.discarded == 3, "as 3 amostras que nao couberam aparecem em discarded");
        checar(c.capacity() == 2, "a capacidade e a pedida, nao a que o vetor cresceu");
    }

    // --- percentis exatos, sem precisão a declarar -----------------------
    {
        tail_collector c{10};
        for (std::uint64_t v : {9ULL, 1ULL, 8ULL, 2ULL, 7ULL, 3ULL, 6ULL, 4ULL, 5ULL, 10ULL})
        {
            c.record(v);
        }
        const tail_statistics t = c.summarize();
        checar(t.minimum == 1 && t.maximum == 10, "minimo e maximo saem da amostra ordenada");
        checar(t.p50 == 5.5, "p50 de 1..10 e 5,5 -- exato, e nao aproximado por bucket");
        checar(t.samples == 10 && t.discarded == 0, "dez amostras, nenhum descarte");
    }

    // --- o teto de percentil publicável ----------------------------------
    {
        tail_collector pequeno{100};
        for (int i = 0; i < 100; ++i)
        {
            pequeno.record(static_cast<std::uint64_t>(i));
        }
        const tail_statistics t = pequeno.summarize();
        checar(t.highest_supported < 0.99,
               "100 amostras nao sustentam p99 (a convencao exige 1000)");
        checar(t.p999 > 0.0,
               "o p99,9 e CALCULADO mesmo sem sustentacao -- e o teto que o reprova, "
               "nao a ausencia do numero");
    }
    {
        tail_collector grande{min_samples_for(0.999)};
        for (std::size_t i = 0; i < min_samples_for(0.999); ++i)
        {
            grande.record(static_cast<std::uint64_t>(i));
        }
        checar(grande.summarize().highest_supported == 0.999,
               "10 000 amostras sustentam p99,9");
    }

    // --- coletor vazio: estado, não lixo ---------------------------------
    {
        tail_collector vazio{4};
        const tail_statistics t = vazio.summarize();
        checar(t.samples == 0 && t.p50 == 0.0 && t.maximum == 0,
               "coletor vazio devolve zeros e samples 0, nao valor sentinela");
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
        checar(a == b, "percentile() e percentile_ns() dao o mesmo valor para a mesma amostra");
    }

    if (violar)
    {
        // A violação é pedir publicação de um percentil que a amostra não
        // sustenta. O programa MORRE, e o meson espera que ele morra.
        tail_collector c{10};
        c.record(1);
        const tail_statistics t = c.summarize();
        PERF_EXPECTS(t.highest_supported >= 0.999);
        std::println("  FALHA: o contrato nao disparou com {} amostra(s)", t.samples);
        return 1;
    }

    if (falhas == 0)
    {
        std::println("cauda: todas as assercoes passaram");
        return 0;
    }
    std::println("cauda: {} assercao(oes) falharam", falhas);
    return 1;
}
