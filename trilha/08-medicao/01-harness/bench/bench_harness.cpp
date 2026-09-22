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
//   1. custo de ler o relógio     - o piso de qualquer medição temporal aqui;
//   2. custo de registrar amostra - o que o coletor de cauda cobra por amostra;
//   3. piso por operação          - o intervalo medido entre duas leituras
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
//   POR OPERAÇÃO - um par de leituras por operação. A cauda aparece, e o piso
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
double custo_do_relogio(int rodadas) noexcept
{
    const std::uint64_t t0 = now_ns();
    for (int i = 0; i < rodadas; ++i)
    {
        const std::uint64_t agora = now_ns();
        sink(agora);
    }
    const std::uint64_t t1 = now_ns();
    return static_cast<double>(t1 - t0) / static_cast<double>(rodadas);
}

// --- braço 2: custo de registrar uma amostra ------------------------------
//
// O `sink` DEPOIS do laço não é simetria com o braço 1: é correção de um
// defeito que a primeira campanha arquivou.
//
// Em `-O2` este braço media 0,359 ns. Em `release` (`-O3`) passou a medir
// 0,000 -- e 0,000 não é "rápido", é "não aconteceu": ninguém observava o
// estado do coletor depois, então o laço inteiro era removido. A régua trata
// mediana zero como `below_resolution`, que é estado legítimo para operação
// mais rápida que o relógio, e por isso a tabela saiu plausível.
//
// Observar `coletor.size()` ao fim torna o efeito do laço visível ao
// compilador, sem custo dentro dele.
double custo_do_registro(tail_collector &coletor, int rodadas) noexcept
{
    const std::uint64_t t0 = now_ns();
    for (int i = 0; i < rodadas; ++i)
    {
        coletor.record(static_cast<std::uint64_t>(i));
    }
    const std::uint64_t t1 = now_ns();
    sink(coletor.size());
    return static_cast<double>(t1 - t0) / static_cast<double>(rodadas);
}

// --- braço 3: piso por operação -------------------------------------------
//
// Mede o intervalo entre duas leituras consecutivas, com nada no meio. Cada
// intervalo é UMA amostra da cauda: é assim que um tópico de estrutura vai
// medir, e portanto é assim que o piso tem de ser medido.
tail_statistics piso_por_operacao(std::size_t amostras)
{
    tail_collector coletor{amostras};
    for (std::size_t i = 0; i < amostras; ++i)
    {
        const std::uint64_t a = now_ns();
        const std::uint64_t b = now_ns();
        coletor.record(b - a);
    }
    return coletor.summarize();
}

void imprimir_cauda(std::string_view rotulo, const tail_statistics &t)
{
    std::println("{:<28} {:>10} {:>10.1f} {:>10.1f} {:>10.1f} {:>12} {:>8}", rotulo, t.samples,
                 t.p50, t.p99, t.p999, t.maximum, t.discarded);
}

// O maior percentil que a contagem sustenta é DADO DA TABELA, não nota de pé de
// página: sem ele, um p99,9 calculado de amostra insuficiente sai idêntico a um
// legítimo.
void imprimir_cabecalho_cauda()
{
    std::println("\n{:<28} {:>10} {:>10} {:>10} {:>10} {:>12} {:>8}", "por operacao (ns)", "n",
                 "p50", "p99", "p99,9", "max", "descart.");
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
int conferir(const statistics &relogio, const statistics &registro, const tail_statistics &piso,
             int amostras, std::size_t capacidade)
{
    if (!is_valid(relogio, amostras) || !is_valid(registro, amostras))
    {
        std::println("FALHA: coleta invalida -- relogio [{}], registro [{}].", badge(relogio),
                     badge(registro));
        std::println("       mediana zero aqui significa laco removido pelo otimizador,");
        std::println("       nao operacao mais rapida que o relogio. Nada disto e publicavel.");
        return 1;
    }
    if (piso.samples != capacidade || piso.discarded != 0)
    {
        std::println("FALHA: a cauda coletou {} de {} amostras, {} descartadas.", piso.samples,
                     capacidade, piso.discarded);
        return 1;
    }
    return 0;
}

int csv(const statistics &relogio, const statistics &registro, const tail_statistics &piso,
        int amostras, int rodadas)
{
    std::println("braco,metrica,valor,unidade");
    std::println("clock_read,median,{:.4f},ns", relogio.median);
    std::println("clock_read,minimum,{:.4f},ns", relogio.minimum);
    std::println("clock_read,p99,{:.4f},ns", relogio.p99);
    std::println("clock_read,disp,{:.2f},percent", relogio.disp);
    std::println("record_sample,median,{:.4f},ns", registro.median);
    std::println("record_sample,minimum,{:.4f},ns", registro.minimum);
    std::println("record_sample,p99,{:.4f},ns", registro.p99);
    std::println("record_sample,disp,{:.2f},percent", registro.disp);
    std::println("per_op_floor,p50,{:.1f},ns", piso.p50);
    std::println("per_op_floor,p99,{:.1f},ns", piso.p99);
    std::println("per_op_floor,p999,{:.1f},ns", piso.p999);
    std::println("per_op_floor,maximum,{},ns", piso.maximum);
    std::println("per_op_floor,samples,{},count", piso.samples);
    std::println("per_op_floor,discarded,{},count", piso.discarded);
    std::println("per_op_floor,highest_supported,{:.4f},percentile", piso.highest_supported);
    std::println("parametros,amostras,{},count", amostras);
    std::println("parametros,rodadas,{},count", rodadas);
    std::println("parametros,clock_period,{:.4f},ns", clock_period_ns());
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    bool modo_csv = false;
    bool controle = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg{argv[i]};
        modo_csv = modo_csv || arg == "--csv";
        controle = controle || arg == "--braco-de-controle";
    }

    const int amostras = samples();
    const int rodadas = rounds(200'000);

    // Capacidade da cauda: o mínimo que sustenta p99,9 pela convenção de
    // `tail.hpp`, salvo se o ambiente pedir mais. Não é número redondo
    // escolhido por gosto -- é o termo de aceite do percentil que a tabela
    // publica.
    std::size_t capacidade = min_samples_for(0.999);
    if (const char *env = std::getenv("CPP_ACADEMY_CAUDA"))
    {
        const long pedido = std::strtol(env, nullptr, 10);
        if (pedido > 0)
        {
            capacidade = static_cast<std::size_t>(pedido);
        }
    }

    // O coletor do braço 2 é criado FORA da medição, e com capacidade para
    // todas as rodadas: se ele enchesse no meio, `record` passaria a contar
    // descarte em vez de gravar, e o braço mediria o ramo barato.
    tail_collector coletor_registro{static_cast<std::size_t>(rodadas)};

    // BRAÇO DE CONTROLE: simula o braço que o otimizador removeu.
    //
    // Ele existe porque o portão de validade deste programa ficou uma campanha
    // inteira sem disparar no modo `--csv`, e nada ficou vermelho. Uma
    // verificação que nunca dispara é indistinguível de uma ausente, então o
    // caminho de reprovação passou a ser exercitável de fora -- e o teste L2 o
    // exercita nos DOIS modos, porque o defeito era exatamente a diferença
    // entre eles.
    const statistics relogio =
        controle ? collect([] { return 0.0; }, amostras)
                 : collect([&] { return custo_do_relogio(rodadas); }, amostras);
    const statistics registro =
        collect([&] { return custo_do_registro(coletor_registro, rodadas); }, amostras);
    const tail_statistics piso = piso_por_operacao(capacidade);

    // O PORTÃO DE VALIDADE VEM ANTES DE QUALQUER SAÍDA, e a ordem foi corrigida
    // depois de uma campanha arquivada com um braço inválido.
    //
    // A primeira versão conferia a validade no fim, DEPOIS do `return` do modo
    // `--csv`. O modo texto reprovava; o modo `--csv` -- que é justamente o que
    // a campanha arquiva -- saía zero. O resultado foi um `metadata.json`
    // versionado, com procedência completa, descrevendo uma medição que não
    // mediu: exatamente o defeito que este tópico existe para expor.
    if (const int problema = conferir(relogio, registro, piso, amostras, capacidade))
    {
        return problema;
    }

    if (modo_csv)
    {
        return csv(relogio, registro, piso, amostras, rodadas);
    }

    std::println("Harness medindo a si mesmo");
    std::println("  resolucao do steady_clock ... {:.1f} ns", clock_period_ns());
    std::println("  amostras x rodadas ......... {} x {}", amostras, rodadas);
    std::println("  capacidade da cauda ........ {} (minimo para p99,9: {})", capacidade,
                 min_samples_for(0.999));

    print_header();
    print_row("ler o relogio", relogio);
    print_row("registrar amostra", registro);

    imprimir_cabecalho_cauda();
    imprimir_cauda("piso do par de leituras", piso);

    std::println("\nO que estes numeros autorizam:");
    std::println("  medicao POR OPERACAO nao distingue custo abaixo de ~{:.0f} ns (p50 do piso).",
                 piso.p50);
    std::println("  abaixo disso, a tabela descreve o instrumento -- o caminho e medir em LOTE.");
    if (piso.highest_supported < 0.999)
    {
        std::println("  ATENCAO: {} amostras nao sustentam p99,9 (minimo {}). O p99,9 acima",
                     piso.samples, min_samples_for(0.999));
        std::println("  esta calculado, mas NAO e publicavel -- ver tail.hpp.");
    }

    return 0;
}
