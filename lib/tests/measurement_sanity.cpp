// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Sanidade da régua de apuração.
//
// POR QUE ESTE PROGRAMA EXISTE
//
// Ele não mede nada interessante de propósito -- o corpo cronometrado é um
// incremento sobre `volatile`, escolhido por ser barato e determinístico. O que
// ele verifica é a RÉGUA, não o objeto:
//
//   1. compilar este alvo avalia todos os `static_assert` de
//      lib/measurement/. Os testes de `percentile`, `decimals`, `dashes` e do
//      período do relógio são asserções de COMPILAÇÃO, e uma regressão neles
//      quebra o build antes de quebrar a suíte. É a diretriz de contratos do
//      projeto aplicada à própria régua;
//
//   2. a coleta executa e produz resultado PUBLICÁVEL. Sair com zero quando a
//      coleta é inválida seria o defeito exato que o selo "?" e o estado
//      `collection_state` existem para impedir: tabela impressa, nada medido, e
//      a suíte dizendo OK para quem não lê a tabela.
//
// O AQUECIMENTO NÃO É OPCIONAL AQUI
//
// A primeira passagem é descartada, e isso é norma do projeto, não zelo local:
// a seção 11 do padrão exige aquecimento simétrico e declarado em toda
// comparação. Um programa de referência que não aquece ensina, por exemplo, a
// não aquecer.

#include "lib/measurement/clock.hpp"
#include "lib/measurement/cpu_pause.hpp"
#include "lib/measurement/statistics.hpp"

#include <cstdint>
#include <print>

int main()
{
    using namespace perf::measurement;

    const int n = samples();
    const int r = rounds(200'000);

    auto medir = [r] {
        const double inicio = now_ns_d();
        volatile std::uint64_t acumulador = 0;
        for (int i = 0; i < r; ++i)
        {
            acumulador = acumulador + 1;
        }
        const double fim = now_ns_d();
        return (fim - inicio) / static_cast<double>(r);
    };

    // A dica de espera precisa existir no binário para provar que compila no
    // alvo -- arm64 está no escopo declarado, e o defeito que originou
    // cpu_pause.hpp no projeto irmão era exatamente um builtin de x86 sem
    // guarda de arquitetura.
    cpu_pause();

    (void) medir(); // aquecimento, descartado

    const auto e = collect(medir, n);

    std::print("  periodo do relogio: {:.3f} ns\n", clock_period_ns());
    std::print("  amostras: {}   rodadas por amostra: {}\n\n", n, r);
    print_header();
    print_row("incremento volatile (ns/op)", e);
    std::print("\n");

    switch (state_of(e, n))
    {
    case collection_state::valid:
        return 0;
    case collection_state::below_resolution:
        std::print(stderr,
                   "  COLETA ABAIXO DA RESOLUCAO DO RELOGIO: sem resultado publicavel.\n"
                   "  A operacao medida e mais rapida que {:.3f} ns; aumente as rodadas\n"
                   "  por amostra em vez de publicar zero como custo.\n",
                   clock_period_ns());
        return 1;
    case collection_state::invalid:
        std::print(stderr, "  COLETA INVALIDA: sem resultado publicavel.\n");
        return 1;
    }
    return 1;
}
