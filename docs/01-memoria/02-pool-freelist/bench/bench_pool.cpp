// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Rotatividade em regime: os dois braços, mesma carga, mesmo harness.
//
// A CARGA: CONJUNTO VIVO CONSTANTE, ORDEM DE LIBERAÇÃO EMBARALHADA
//
// Um conjunto de V objetos vivos. Cada passo libera um deles -- escolhido por
// um passo coprimo com V, e não o último -- e adquire outro. O conjunto vivo
// não cresce nem encolhe; o que gira é quem está nele.
//
// A ORDEM EMBARALHADA NÃO É ENFEITE
//
// Liberar sempre o último transformaria o padrão numa pilha, e uma pilha é o
// melhor caso dos dois braços: o alocador global devolveria o bloco que acabou
// de liberar, e o pool devolveria o slot que acabou de empilhar. A tabela
// publicaria o melhor caso com o nome de "rotatividade".
//
// O passo coprimo com V percorre o conjunto inteiro antes de repetir, e é
// determinístico -- duas execuções fazem exatamente a mesma sequência, que é o
// que permite comparar execuções.
//
// A VARIÁVEL DA VARREDURA É O TAMANHO DO CONJUNTO VIVO
//
// É ele que decide se os blocos vivos cabem no cache, e é aí que os dois braços
// se separam: o pool tem os slots contíguos, o alocador global não promete
// nada sobre onde eles ficam.

#include "docs/01-memoria/02-pool-freelist/custom/pool_arm.hpp"
#include "docs/01-memoria/02-pool-freelist/spec.hpp"
#include "docs/01-memoria/02-pool-freelist/std/heap_churn_arm.hpp"
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

// Tamanhos do conjunto vivo. 64 nós são 4 KB e cabem no L1; 4096 são 256 KB e
// passam do L2 desta máquina -- a varredura atravessa a hierarquia de propósito.
constexpr std::size_t kVivos[] = {64, 512, 4096};
constexpr std::size_t kMaiorVivo = 4096;

// Passos de rotatividade por amostra. O custo de um passo é da ordem de 50 ns,
// e o piso do instrumento é 20 ns (topico 08.01): 64 passos por amostra põem a
// medicao duas ordens de grandeza acima do piso.
constexpr std::size_t kPassosPorAmostra = 64;
constexpr int kAquecimento = 64;

// Coprimo com todos os tamanhos da varredura, que são potências de dois.
constexpr std::size_t kSalto = 7;

template <typename T>
[[gnu::always_inline]] inline void sink(const T &v) noexcept
{
    __asm__ __volatile__("" : : "r,m"(v) : "memory");
}

template <typename A>
[[gnu::noinline]] std::uint64_t rodar(A &alocador, std::vector<node *> &vivos, std::size_t passos,
                                      std::size_t &cursor, std::uint64_t semente)
{
    std::uint64_t soma = 0;
    for (std::size_t i = 0; i < passos; ++i)
    {
        cursor = (cursor + kSalto) % vivos.size();
        alocador.release(vivos[cursor]);
        node *novo = alocador.acquire();
        if (novo == nullptr) [[unlikely]]
        {
            return 0;
        }
        novo->id = semente + i;
        soma += novo->id;
        vivos[cursor] = novo;
    }
    return soma;
}

template <typename A>
tail_statistics medir(std::size_t conjunto, std::size_t amostras)
{
    // A capacidade do pool é o conjunto vivo MAIS UM: o passo libera antes de
    // adquirir, então nunca há mais de `conjunto` vivos ao mesmo tempo. Dar
    // folga esconderia o caso em que a lista de livres fica vazia, que é
    // justamente o regime deste tópico.
    A alocador{conjunto};
    std::vector<node *> vivos;
    vivos.reserve(kMaiorVivo);
    for (std::size_t i = 0; i < conjunto; ++i)
    {
        node *p = alocador.acquire();
        if (p == nullptr)
        {
            return {};
        }
        p->id = i;
        vivos.push_back(p);
    }

    std::size_t cursor = 0;
    for (int i = 0; i < kAquecimento; ++i)
    {
        sink(rodar(alocador, vivos, kPassosPorAmostra, cursor, static_cast<std::uint64_t>(i)));
    }

    tail_collector coletor{amostras};
    for (std::size_t i = 0; i < amostras; ++i)
    {
        const std::uint64_t t0 = now_ns();
        const std::uint64_t soma = rodar(alocador, vivos, kPassosPorAmostra, cursor, i);
        const std::uint64_t t1 = now_ns();
        sink(soma);
        coletor.record(t1 - t0);
    }
    const tail_statistics t = coletor.summarize();
    for (node *p : vivos)
    {
        alocador.release(p);
    }
    return t;
}

void linha_csv(std::string_view braco, std::size_t vivos, const tail_statistics &t)
{
    const auto pref = std::string{braco} + "_V" + std::to_string(vivos);
    const double por_passo = t.p50 / static_cast<double>(kPassosPorAmostra);
    std::println("{},p50,{:.1f},ns", pref, t.p50);
    std::println("{},p99,{:.1f},ns", pref, t.p99);
    std::println("{},p999,{:.1f},ns", pref, t.p999);
    std::println("{},maximum,{},ns", pref, t.maximum);
    std::println("{},p50_per_step,{:.3f},ns", pref, por_passo);
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
        std::println("Pool com lista de livres contra o alocador global");
        std::println("  amostras por ponto ......... {}", amostras);
        std::println("  passos por amostra ......... {}", kPassosPorAmostra);
        std::println("  aquecimento ................ {} rodadas (iguais nos dois bracos)",
                     kAquecimento);
        std::println("");
        std::println("  {:<14} {:>7} {:>10} {:>10} {:>10} {:>12} {:>12}", "braco", "vivos", "p50",
                     "p99", "p99.9", "max", "p50/passo");
        std::println("  {}", dashes(82));
    }

    int problemas = 0;
    for (std::size_t conjunto : kVivos)
    {
        const tail_statistics h = medir<heap_churn_arm>(conjunto, amostras);
        const tail_statistics p = medir<pool_arm>(conjunto, amostras);

        for (const auto &[nome, t] : {std::pair{heap_churn_arm::traits.name, h},
                                      std::pair{pool_arm::traits.name, p}})
        {
            if (t.samples != amostras || t.discarded != 0 || t.p50 <= 0.0)
            {
                std::println("FAILED: arm {} at live set {} collected {} of {} samples", nome,
                             conjunto, t.samples, amostras);
                ++problemas;
            }
            if (csv)
            {
                linha_csv(nome, conjunto, t);
            }
            else
            {
                std::println("  {:<14} {:>7} {:>10.1f} {:>10.1f} {:>10.1f} {:>12} {:>12.2f}", nome,
                             conjunto, t.p50, t.p99, t.p999, t.maximum,
                             t.p50 / static_cast<double>(kPassosPorAmostra));
            }
        }
    }

    if (csv)
    {
        std::println("parameters,samples,{},count", amostras);
        std::println("parameters,steps_per_sample,{},count", kPassosPorAmostra);
        std::println("parameters,warmup_rounds,{},count", kAquecimento);
        std::println("parameters,clock_period,{:.4f},ns", clock_period_ns());
    }
    return problemas == 0 ? 0 : 1;
}
