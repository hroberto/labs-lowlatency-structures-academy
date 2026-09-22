// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Os dois braços, no mesmo harness, sob a mesma carga.
//
// A CARGA: UM LOTE NASCE, É USADO, E MORRE INTEIRO
//
// É o padrão do cenário que atravessa a trilha -- um lote de mensagens chega,
// vira objetos, é processado, e some. Não é o padrão de um `std::map` que vive
// anos; é o de um caminho quente que recicla.
//
// Cada amostra é UM LOTE COMPLETO: adquirir L nós, tocar cada um, devolver
// todos. O relógio é lido duas vezes por lote, e não duas vezes por objeto --
// e a razão está medida no tópico 08.01: o piso por operação desta máquina é de
// 20 ns, e uma alocação custa da ordem de 25 ns. Medir objeto a objeto
// publicaria uma tabela em que metade do número é o relógio.
//
// VARREDURA, E NÃO PONTO ÚNICO (norma, seção 11)
//
// O tamanho do lote é a variável: em lote pequeno o custo por objeto é o que
// manda; em lote grande, a pressão sobre o alocador global aparece. Um ponto
// único responderia "a arena é mais rápida" sem dizer onde, que é a pergunta do
// módulo.
//
// AQUECIMENTO SIMÉTRICO, E DECLARADO
//
// Os dois braços rodam o mesmo número de lotes de aquecimento antes de
// qualquer coleta. Sem isso, a primeira alocação de cada um paga a primeira
// falta de página do seu buffer, e a tabela publicaria a diferença de
// aquecimento com o nome da estrutura.

#include "docs/01-memoria/01-alocador-arena/custom/arena_arm.hpp"
#include "docs/01-memoria/01-alocador-arena/spec.hpp"
#include "docs/01-memoria/01-alocador-arena/std/heap_arm.hpp"
#include "lib/measurement/clock.hpp"
#include "lib/measurement/statistics.hpp"
#include "lib/measurement/tail.hpp"

#include <cstdint>
#include <cstdlib>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using namespace academy::memory;
using namespace perf::measurement;

// Os tamanhos de lote da varredura. Potências de dois separadas por 8x: quatro
// pontos cobrem três ordens de grandeza sem produzir tabela que ninguém lê.
constexpr std::size_t kLotes[] = {8, 64, 512, 4096};
constexpr std::size_t kMaiorLote = 4096;

// Aquecimento: lotes descartados antes de qualquer coleta, iguais nos dois
// braços. O número é declarado aqui, e aparece no CSV.
constexpr int kAquecimento = 64;

template <typename T>
[[gnu::always_inline]] inline void sink(const T &v) noexcept
{
    __asm__ __volatile__("" : : "r,m"(v) : "memory");
}

// Um lote: adquirir, tocar, devolver. O `tocar` não é enfeite -- sem ele, a
// memória devolvida pela arena nunca é escrita, e a comparação deixaria de
// pagar a falta de página que o braço `std` paga.
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
            // Esgotamento no meio do lote: é a pergunta de falha do módulo, e
            // aqui ela é ERRO DE MEDIÇÃO, não caminho a medir. O programa
            // reprova em vez de publicar um lote incompleto como se fosse um
            // lote.
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
tail_statistics medir(A &alocador, std::size_t tamanho, std::size_t amostras)
{
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
    const double por_no = t.p50 / static_cast<double>(lote);
    std::println("{},p50,{:.1f},ns", pref, t.p50);
    std::println("{},p99,{:.1f},ns", pref, t.p99);
    std::println("{},p999,{:.1f},ns", pref, t.p999);
    std::println("{},maximum,{},ns", pref, t.maximum);
    std::println("{},p50_per_node,{:.3f},ns", pref, por_no);
    std::println("{},samples,{},count", pref, t.samples);
    std::println("{},discarded,{},count", pref, t.discarded);
    std::println("{},highest_supported,{:.4f},percentile", pref, t.highest_supported);
}

void linha_texto(std::string_view braco, std::size_t lote, const tail_statistics &t)
{
    std::println("  {:<14} {:>6} {:>10.1f} {:>10.1f} {:>10.1f} {:>12} {:>11.2f}", braco, lote,
                 t.p50, t.p99, t.p999, t.maximum, t.p50 / static_cast<double>(lote));
}

} // namespace

int main(int argc, char **argv)
{
    const bool csv = argc > 1 && std::string_view{argv[1]} == "--csv";

    // O número de amostras vem do mínimo que sustenta p99,9, pela convenção de
    // `tail.hpp`. Não é redondo por gosto: é termo de aceite do percentil que a
    // tabela publica.
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
        std::println("Alocador de arena contra o alocador global");
        std::println("  amostras por ponto ......... {}", amostras);
        std::println("  lotes de aquecimento ....... {} (iguais nos dois bracos)", kAquecimento);
        std::println("  resolucao do relogio ....... {:.1f} ns", clock_period_ns());
        std::println("");
        std::println("  {:<14} {:>6} {:>10} {:>10} {:>10} {:>12} {:>11}", "braco", "lote", "p50",
                     "p99", "p99.9", "max", "p50/no");
        std::println("  {}", dashes(80));
    }

    int problemas = 0;
    for (std::size_t lote : kLotes)
    {
        heap_arm heap;
        arena_arm arena{kMaiorLote};

        const tail_statistics h = medir(heap, lote, amostras);
        const tail_statistics a = medir(arena, lote, amostras);

        for (const auto &[nome, t] : {std::pair{heap_arm::traits.name, h},
                                      std::pair{arena_arm::traits.name, a}})
        {
            if (t.samples != amostras || t.discarded != 0 || t.p50 <= 0.0)
            {
                std::println("FAILED: arm {} at batch {} collected {} of {} samples, {} discarded",
                             nome, lote, t.samples, amostras, t.discarded);
                ++problemas;
            }
            if (csv)
            {
                linha_csv(nome, lote, t);
            }
            else
            {
                linha_texto(nome, lote, t);
            }
        }
    }

    if (csv)
    {
        std::println("parameters,samples,{},count", amostras);
        std::println("parameters,warmup_batches,{},count", kAquecimento);
        std::println("parameters,node_bytes,{},bytes", sizeof(node));
        std::println("parameters,clock_period,{:.4f},ns", clock_period_ns());
    }
    return problemas == 0 ? 0 : 1;
}
