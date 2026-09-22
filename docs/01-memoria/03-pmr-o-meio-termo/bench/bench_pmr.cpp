// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Três braços, mesma carga do 01.01, mesmo harness.
//
// A CARGA É IDÊNTICA À DO TÓPICO 01.01, E ISSO É A DECISÃO PRINCIPAL
//
// Mesmos tamanhos de lote, mesmo aquecimento, mesmo objeto, mesma forma de
// amostra. Sem isso, a pergunta deste tópico -- "a biblioteca padrão já
// oferece o mecanismo?" -- não teria com o que ser respondida: comparar a
// `pmr` com uma arena medida sob outra carga responderia outra coisa.
//
// O braço `std::allocator` aparece de novo, e não por simetria: ele é a âncora
// que permite dizer se os números deste tópico e os do 01.01 vieram da mesma
// máquina no mesmo estado. Se ele divergir muito do que o 01.01 publicou, é a
// máquina que mudou, não a `pmr` que é rápida.

#include "docs/01-memoria/01-alocador-arena/custom/arena_arm.hpp"
#include "docs/01-memoria/01-alocador-arena/std/heap_arm.hpp"
#include "docs/01-memoria/03-pmr-o-meio-termo/custom/pmr_arm.hpp"
#include "lib/measurement/clock.hpp"
#include "lib/measurement/statistics.hpp"
#include "lib/measurement/tail.hpp"

#include <cstdint>
#include <cstdlib>
#include <print>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using namespace academy::memory;
using namespace perf::measurement;

constexpr std::size_t kLotes[] = {8, 64, 512, 4096};
constexpr std::size_t kMaiorLote = 4096;
constexpr int kAquecimento = 64;

template <typename T>
[[gnu::always_inline]] inline void sink(const T &v) noexcept
{
    __asm__ __volatile__("" : : "r,m"(v) : "memory");
}

template <typename A>
[[gnu::noinline]] std::uint64_t um_lote(A &alocador, std::vector<node *> &guardados,
                                        std::size_t tamanho, std::uint64_t semente)
{
    guardados.clear();
    for (std::size_t i = 0; i < tamanho; ++i)
    {
        node *p = alocador.acquire();
        if (p == nullptr) [[unlikely]]
        {
            return 0;
        }
        p->id = semente + i;
        guardados.push_back(p);
    }
    std::uint64_t soma = 0;
    for (node *p : guardados)
    {
        soma += p->id;
    }
    alocador.release_all(guardados);
    return soma;
}

template <typename A>
tail_statistics medir(std::size_t tamanho, std::size_t amostras)
{
    A alocador{kMaiorLote};
    std::vector<node *> guardados;
    guardados.reserve(kMaiorLote);
    tail_collector coletor{amostras};

    for (int i = 0; i < kAquecimento; ++i)
    {
        sink(um_lote(alocador, guardados, tamanho, static_cast<std::uint64_t>(i)));
    }
    for (std::size_t i = 0; i < amostras; ++i)
    {
        const std::uint64_t t0 = now_ns();
        const std::uint64_t soma = um_lote(alocador, guardados, tamanho, i);
        const std::uint64_t t1 = now_ns();
        sink(soma);
        coletor.record(t1 - t0);
    }
    return coletor.summarize();
}

void linha_csv(std::string_view braco, std::size_t lote, const tail_statistics &t)
{
    const auto pref = std::string{braco} + "_L" + std::to_string(lote);
    std::println("{},p50,{:.1f},ns", pref, t.p50);
    std::println("{},p99,{:.1f},ns", pref, t.p99);
    std::println("{},p999,{:.1f},ns", pref, t.p999);
    std::println("{},maximum,{},ns", pref, t.maximum);
    std::println("{},p50_per_node,{:.3f},ns", pref, t.p50 / static_cast<double>(lote));
    std::println("{},samples,{},count", pref, t.samples);
    std::println("{},discarded,{},count", pref, t.discarded);
    std::println("{},highest_supported,{:.4f},percentile", pref, t.highest_supported);
}

} // namespace

int main(int argc, char **argv)
{
    const bool csv = argc > 1 && std::string_view{argv[1]} == "--csv";

    std::size_t amostras = min_samples_for(0.999);
    if (const char *env = std::getenv("HARNESS_TAIL_SAMPLES"))
    {
        const long pedido = std::strtol(env, nullptr, 10);
        if (pedido > 0)
        {
            amostras = static_cast<std::size_t>(pedido);
        }
    }

    if (csv)
    {
        std::println("arm,metric,value,unit");
    }
    else
    {
        std::println("std::pmr contra a arena artesanal e o alocador global");
        std::println("  amostras por ponto ......... {}", amostras);
        std::println("  aquecimento ................ {} lotes (iguais nos tres bracos)",
                     kAquecimento);
        std::println("  carga ...................... a mesma do topico 01.01");
        std::println("");
        std::println("  {:<16} {:>6} {:>10} {:>10} {:>10} {:>12} {:>11}", "braco", "lote", "p50",
                     "p99", "p99.9", "max", "p50/no");
        std::println("  {}", dashes(82));
    }

    int problemas = 0;
    for (std::size_t lote : kLotes)
    {
        const tail_statistics h = medir<heap_arm>(lote, amostras);
        const tail_statistics a = medir<arena_arm>(lote, amostras);
        const tail_statistics p = medir<pmr_arm>(lote, amostras);

        for (const auto &[nome, t] : {std::pair{heap_arm::traits.name, h},
                                      std::pair{arena_arm::traits.name, a},
                                      std::pair{pmr_arm::traits.name, p}})
        {
            if (t.samples != amostras || t.discarded != 0 || t.p50 <= 0.0)
            {
                std::println("FAILED: arm {} at batch {} collected {} of {} samples", nome, lote,
                             t.samples, amostras);
                ++problemas;
            }
            if (csv)
            {
                linha_csv(nome, lote, t);
            }
            else
            {
                std::println("  {:<16} {:>6} {:>10.1f} {:>10.1f} {:>10.1f} {:>12} {:>11.2f}", nome,
                             lote, t.p50, t.p99, t.p999, t.maximum,
                             t.p50 / static_cast<double>(lote));
            }
        }
    }

    if (csv)
    {
        std::println("parameters,samples,{},count", amostras);
        std::println("parameters,warmup_batches,{},count", kAquecimento);
        std::println("parameters,clock_period,{:.4f},ns", clock_period_ns());
    }
    return problemas == 0 ? 0 : 1;
}
