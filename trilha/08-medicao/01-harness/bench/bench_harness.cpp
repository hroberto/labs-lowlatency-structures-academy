// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// O harness medindo a si mesmo -- que é a pergunta deste módulo.
//
// A PERGUNTA
//
//   Como o harness evita medir a si mesmo?
//
// Não evita. Ele mede o próprio custo primeiro, publica esse número, e só então
// pode dizer o que de fato pertence à operação sob teste. O que se evita é
// medir a si mesmo SEM SABER.
//
// OS TRÊS BRAÇOS, E O QUE CADA UM RESPONDE
//
//   1. custo de ler o relógio     - o tail_floor de qualquer medição temporal aqui;
//   2. custo de registrar amostra - o que o collector de cauda cobra por amostra;
//   3. tail_floor por operação          - o intervalo medido entre duas leituras
//                                   consecutivas do relógio, COM NADA no meio.
//
// O braço 3 é o que fecha o argumento. Ele é a menor latência que este harness
// consegue observar por operação: se a operação sob teste custa menos que isso,
// a tabela publicada descreve o instrumento, não a estrutura -- e a saída de um
// tópico nessa situação não é um número menor, é trocar para medição em LOTE,
// onde o custo do relógio se amortiza sobre milhares de operações.
//
// POR QUE OS DOIS MODOS EXISTEM, E NÃO UM SÓ
//
//   LOTE      - um par de leituras para N operações. O custo do relógio se
//               dilui, mas a distribuição se perde: não há p99 de uma média.
//   POR OPERAÇÃO - um par de leituras por operação. A cauda aparece, e o tail_floor
//               do instrumento entra em cada amostra.
//
// A norma publica percentis altos (seção 28), então a trilha precisa do segundo
// modo. Este tópico existe para que o preço dele esteja medido e publicado
// ANTES de qualquer tópico de estrutura usá-lo.

#include "lib/measurement/clock.hpp"
#include "lib/measurement/statistics.hpp"
#include "lib/measurement/tail.hpp"

#include <cstdint>
#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <vector>

namespace
{

using namespace perf::measurement;

// BARREIRA CONTRA O OTIMIZADOR, e sem ela este programa mede zero.
//
// Um laço cujo resultado ninguém usa é removido inteiro, e a medição sai
// "abaixo da resolução do relógio" -- que é um estado legítimo da régua, e
// aqui seria mentira: a operação não é rápida, ela não aconteceu. A barreira
// diz ao compilador que o valor foi consumido de forma que ele não pode
// enxergar, sem emitir instrução alguma.
template <typename T>
[[gnu::always_inline]] inline void sink(const T &v) noexcept
{
    __asm__ __volatile__("" : : "r,m"(v) : "memory");
}

// --- braço 1: custo de ler o relógio --------------------------------------
double clock_read_cost(int round_count) noexcept
{
    const std::uint64_t t0 = now_ns();
    for (int i = 0; i < round_count; ++i)
    {
        const std::uint64_t now = now_ns();
        sink(now);
    }
    const std::uint64_t t1 = now_ns();
    return static_cast<double>(t1 - t0) / static_cast<double>(round_count);
}

// --- braço 2: custo de registrar uma amostra ------------------------------
//
// O `sink` DEPOIS do laço não é simetria com o braço 1: é correção de um
// defeito que a primeira campanha arquivou.
//
// Em `-O2` este braço media 0,359 ns. Em `release` (`-O3`) passou a medir
// 0,000 -- e 0,000 não é "rápido", é "não aconteceu": ninguém observava o
// estado do collector depois, então o laço inteiro era removido. A régua trata
// mediana zero como `below_resolution`, que é estado legítimo para operação
// mais rápida que o relógio, e por isso a tabela saiu plausível.
//
// Observar `collector.size()` ao fim torna o efeito do laço visível ao
// compilador, sem custo dentro dele.
double record_sample_cost(tail_collector &collector, int round_count) noexcept
{
    const std::uint64_t t0 = now_ns();
    for (int i = 0; i < round_count; ++i)
    {
        collector.record(static_cast<std::uint64_t>(i));
    }
    const std::uint64_t t1 = now_ns();
    sink(collector.size());
    return static_cast<double>(t1 - t0) / static_cast<double>(round_count);
}

// --- braço 3: tail_floor por operação -------------------------------------------
//
// Mede o intervalo entre duas leituras consecutivas, com nada no meio. Cada
// intervalo é UMA amostra da cauda: é assim que um tópico de estrutura vai
// medir, e portanto é assim que o tail_floor tem de ser medido.
tail_statistics measure_per_op_floor(std::size_t sample_count)
{
    tail_collector collector{sample_count};
    for (std::size_t i = 0; i < sample_count; ++i)
    {
        const std::uint64_t a = now_ns();
        const std::uint64_t b = now_ns();
        collector.record(b - a);
    }
    return collector.summarize();
}

void print_tail_row(std::string_view label, const tail_statistics &t)
{
    std::println("{:<28} {:>10} {:>10.1f} {:>10.1f} {:>10.1f} {:>12} {:>8}", label, t.samples,
                 t.p50, t.p99, t.p999, t.maximum, t.discarded);
}

// O maior percentil que a contagem sustenta é DADO DA TABELA, não nota de pé de
// página: sem ele, um p99,9 calculado de amostra insuficiente sai idêntico a um
// legítimo.
void print_tail_header()
{
    std::println("\n{:<28} {:>10} {:>10} {:>10} {:>10} {:>12} {:>8}", "per operation (ns)", "n",
                 "p50", "p99", "p99.9", "max", "discarded");
    std::println("{}", dashes(92));
}

// O código de saída é o que a suíte lê, e ele não pode dizer OK sobre coleta
// inválida: foi exatamente assim que, no projeto de origem, tabelas com valor
// sentinela negativo saíram publicadas com saída zero.
//
// `below_resolution` reprova aqui de propósito. É estado legítimo da régua para
// operação mais rápida que o relógio, mas neste programa ele significa que o
// laço medido foi removido pelo otimizador -- e um braço removido não é um
// braço rápido.
int check_collection(const statistics &clock_arm, const statistics &record_arm, const tail_statistics &tail_floor,
             int sample_count, std::size_t capacity_requested)
{
    if (!is_valid(clock_arm, sample_count) || !is_valid(record_arm, sample_count))
    {
        std::println("FAILED: invalid collection -- clock [{}], record [{}].", badge(clock_arm),
                     badge(record_arm));
        std::println("        a zero median here means the loop was removed by the optimizer,");
        std::println("        not an operation faster than the clock. None of this is publishable.");
        return 1;
    }
    if (tail_floor.samples != capacity_requested || tail_floor.discarded != 0)
    {
        std::println("FAILED: the tail collected {} of {} samples, {} discarded.",
                     tail_floor.samples, capacity_requested, tail_floor.discarded);
        return 1;
    }
    return 0;
}

int csv(const statistics &clock_arm, const statistics &record_arm, const tail_statistics &tail_floor,
        int sample_count, int round_count)
{
    std::println("arm,metric,value,unit");
    std::println("clock_read,median,{:.4f},ns", clock_arm.median);
    std::println("clock_read,minimum,{:.4f},ns", clock_arm.minimum);
    std::println("clock_read,p99,{:.4f},ns", clock_arm.p99);
    std::println("clock_read,disp,{:.2f},percent", clock_arm.disp);
    std::println("record_sample,median,{:.4f},ns", record_arm.median);
    std::println("record_sample,minimum,{:.4f},ns", record_arm.minimum);
    std::println("record_sample,p99,{:.4f},ns", record_arm.p99);
    std::println("record_sample,disp,{:.2f},percent", record_arm.disp);
    std::println("per_op_floor,p50,{:.1f},ns", tail_floor.p50);
    std::println("per_op_floor,p99,{:.1f},ns", tail_floor.p99);
    std::println("per_op_floor,p999,{:.1f},ns", tail_floor.p999);
    std::println("per_op_floor,maximum,{},ns", tail_floor.maximum);
    std::println("per_op_floor,samples,{},count", tail_floor.samples);
    std::println("per_op_floor,discarded,{},count", tail_floor.discarded);
    std::println("per_op_floor,highest_supported,{:.4f},percentile", tail_floor.highest_supported);
    std::println("parameters,samples,{},count", sample_count);
    std::println("parameters,rounds,{},count", round_count);
    std::println("parameters,clock_period,{:.4f},ns", clock_period_ns());
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    bool csv_mode = false;
    bool control_arm = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg{argv[i]};
        csv_mode = csv_mode || arg == "--csv";
        control_arm = control_arm || arg == "--control-arm";
    }

    const int sample_count = samples();
    const int round_count = rounds(200'000);

    // Capacidade da cauda: o mínimo que sustenta p99,9 pela convenção de
    // `tail.hpp`, salvo se o ambiente pedir mais. Não é número redondo
    // escolhido por gosto -- é o termo de aceite do percentil que a tabela
    // publica.
    std::size_t capacity_requested = min_samples_for(0.999);
    if (const char *env = std::getenv("HARNESS_TAIL_SAMPLES"))
    {
        const long requested = std::strtol(env, nullptr, 10);
        if (requested > 0)
        {
            capacity_requested = static_cast<std::size_t>(requested);
        }
    }

    // O collector do braço 2 é criado FORA da medição, e com capacity_requested para
    // todas as round_count: se ele enchesse no meio, `record` passaria a contar
    // descarte em vez de gravar, e o braço mediria o ramo barato.
    tail_collector record_collector{static_cast<std::size_t>(round_count)};

    // BRAÇO DE CONTROLE: simula o braço que o otimizador removeu.
    //
    // Ele existe porque o portão de validade deste programa ficou uma campanha
    // inteira sem disparar no modo `--csv`, e nada ficou vermelho. Uma
    // verificação que nunca dispara é indistinguível de uma ausente, então o
    // caminho de reprovação passou a ser exercitável de fora -- e o teste L2 o
    // exercita nos DOIS modos, porque o defeito era exatamente a diferença
    // entre eles.
    const statistics clock_arm =
        control_arm ? collect([] { return 0.0; }, sample_count)
                 : collect([&] { return clock_read_cost(round_count); }, sample_count);
    const statistics record_arm =
        collect([&] { return record_sample_cost(record_collector, round_count); }, sample_count);
    const tail_statistics tail_floor = measure_per_op_floor(capacity_requested);

    // O PORTÃO DE VALIDADE VEM ANTES DE QUALQUER SAÍDA, e a ordem foi corrigida
    // depois de uma campanha arquivada com um braço inválido.
    //
    // A primeira versão conferia a validade no fim, DEPOIS do `return` do modo
    // `--csv`. O modo texto reprovava; o modo `--csv` -- que é justamente o que
    // a campanha arquiva -- saía zero. O resultado foi um `metadata.json`
    // versionado, com procedência completa, descrevendo uma medição que não
    // mediu: exatamente o defeito que este tópico existe para expor.
    if (const int problem = check_collection(clock_arm, record_arm, tail_floor, sample_count, capacity_requested))
    {
        return problem;
    }

    if (csv_mode)
    {
        return csv(clock_arm, record_arm, tail_floor, sample_count, round_count);
    }

    std::println("The harness measuring itself");
    std::println("  steady_clock resolution .... {:.1f} ns", clock_period_ns());
    std::println("  samples x rounds ........... {} x {}", sample_count, round_count);
    std::println("  tail capacity .............. {} (minimum for p99.9: {})", capacity_requested,
                 min_samples_for(0.999));

    print_header();
    print_row("reading the clock", clock_arm);
    print_row("recording a sample", record_arm);

    print_tail_header();
    print_tail_row("floor of the read pair", tail_floor);

    std::println("\nWhat these numbers license:");
    std::println("  PER-OPERATION measurement cannot separate a cost below ~{:.0f} ns "
                 "(the floor's p50).",
                 tail_floor.p50);
    std::println("  below that, the table describes the instrument -- the way out is BATCH "
                 "measurement.");
    if (tail_floor.highest_supported < 0.999)
    {
        std::println("  WARNING: {} samples do not support p99.9 (minimum {}). The p99.9 above",
                     tail_floor.samples, min_samples_for(0.999));
        std::println("  is computed, but is NOT publishable -- see tail.hpp.");
    }

    return 0;
}
