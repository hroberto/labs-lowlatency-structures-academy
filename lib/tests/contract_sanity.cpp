// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Sanidade das três portas de contrato.
//
// O QUE ESTE PROGRAMA VERIFICA, E O QUE JÁ FOI VERIFICADO ANTES DELE
//
// A maior parte do contrato é testada em COMPILAÇÃO: os `static_assert` de
// domain.hpp, measurement_domains.hpp e budget.hpp já rodaram quando este
// arquivo compilou, e uma regressão neles quebra o build antes de a suíte existir. É a
// diretriz do projeto em ação -- onde o domínio é `constexpr`, o teste do
// domínio é `static_assert`.
//
// Sobram três coisas que compilação não alcança:
//
//   1. a porta E funcionando sobre valor que só existe em RUNTIME -- aqui o
//      caso é o número vindo de variável de ambiente, e é o único dos três que
//      não pode ser compilado fora;
//   2. a mensagem de erro da porta E sendo de fato legível;
//   3. a porta R ABORTANDO quando violada. Um contrato que não dispara é
//      indistinguível de um contrato ausente, e nenhum `static_assert` prova
//      que o caminho de falha existe.
//
// O TERCEIRO CASO SÓ FAZ SENTIDO COM O CONTRATO LIGADO
//
// Com `NDEBUG`, `PERF_EXPECTS` vira `[[assume]]`: violate o predicado é
// comportamento INDEFINIDO, não abort. Um teste que esperasse falha ali estaria
// esperando um comportamento que o padrão não promete. Por isso `--violate`
// PULA com código 77 em release, em vez de passar ou de falhar: o que não foi
// verificado não pode ser reportado como verificado.

#include "lib/contract/budget.hpp"
#include "lib/contract/domain.hpp"
#include "lib/contract/measurement_domains.hpp"

#include <cstdint>
#include <cstdlib>
#include <print>
#include <string>
#include <string_view>

namespace
{

using namespace perf::contract::measurement;

// Simula a fronteira externa: um valor que o compilador não pode conhecer.
// `volatile` impede que ele seja propagado como constante e que o caso vire,
// sem aviso, um teste de compilação disfarçado de teste de runtime.
template <typename T>
T from_env(T simulado)
{
    volatile T opaco = simulado;
    return opaco;
}

int gate_e()
{
    int failures = 0;

    // Valor válido vindo "de fora".
    const auto accepted = sample_count::parse(from_env(25));
    if (!accepted.has_value())
    {
        std::print(stderr, "  FAILED: gate E rejected 25, which is inside the domain\n");
        ++failures;
    }
    else if (accepted->get() != 25)
    {
        std::print(stderr, "  FAILED: gate E altered the accepted value\n");
        ++failures;
    }

    // Valor inválido vindo "de fora": tem de ser recusado em TODA configuração,
    // inclusive em release. É a regra que a norma chama de "a porta E nunca é
    // compilada fora".
    //
    // O 2 é o caso realista: `HARNESS_SAMPLES=2` é o que alguém escreveria
    // para acelerar a CI, e duas amostras não sustentam quartil nenhum.
    const auto rejected = sample_count::parse(from_env(2));
    if (rejected.has_value())
    {
        std::print(stderr, "  FAILED: gate E accepted 2 samples, outside the domain."
                           " A validacao externa foi compilada fora?\n");
        ++failures;
    }
    else
    {
        const auto text = rejected.error().describe();
        // A mensagem precisa identificar o PARAMETRO, nao so a faixa: num log
        // de diagnostico, "valor fora de faixa" obriga quem le a descobrir de
        // qual dos seis parametros se fala.
        if (text.find("sample count") == std::string::npos)
        {
            std::print(stderr, "  FAILED: the gate E message does not name the parameter: {}\n", text);
            ++failures;
        }
        else
        {
            std::print("  gate E: 2 rejected -- {}\n", text);
        }
    }

    // Outro domínio: o erro precisa mudar de nome junto. Um `domain_name`
    // fixo passaria no caso acima e falharia aqui.
    const auto rounds_value = rounds_per_sample::parse(from_env(999));
    if (rounds_value.has_value() || rounds_value.error().domain_name != "rounds per sample")
    {
        std::print(stderr, "  FAILED: gate E for rounds did not identify the domain\n");
        ++failures;
    }

    return failures;
}

int gates_t_and_r()
{
    int failures = 0;

    // Porta T: já validada em compilação; aqui só se confirma que o valor
    // atravessa intacto até o runtime.
    if (sample_count::of<25>().get() != 25 || alignment::of<64>().get() != 64)
    {
        std::print(stderr, "  FAILED: gate T altered the value\n");
        ++failures;
    }

    // Porta R com valor válido: não deve disparar em nenhuma configuração.
    if (rounds_per_sample::trusted(from_env(200'000)).get() != 200'000)
    {
        std::print(stderr, "  FAILED: gate R altered the value\n");
        ++failures;
    }

    // Orçamento declarado.
    using feed = perf::contract::workload_contract<5'000'000>;
    if (feed::budget_ns != 200.0 || !feed::fits(200.0) || feed::fits(200.1))
    {
        std::print(stderr, "  FAILED: the declared budget does not check out\n");
        ++failures;
    }
    std::print("  budget: {} events/s -> {:.1f} ns per event\n", feed::events_per_second,
               feed::budget_ns);

    return failures;
}

} // namespace

int main(int argc, char **argv)
{
    const bool violate = argc > 1 && std::string_view{argv[1]} == "--violate";

    if (violate)
    {
#ifdef NDEBUG
        std::print("  SKIPPED - a contract violation in release is UNDEFINED\n"
                   "  behaviour, not abort: `PERF_EXPECTS` became `[[assume]]`.\n"
                   "  This case did NOT pass: it does not run in this configuration.\n"
                   "  It runs in build-debug and in build-release-checked.\n");
        return 77;
#else
        std::print("  firing a deliberate gate R violation"
                   " (2 samples, below the floor of 3)\n");
        // Esperado: mensagem de contrato violado em stderr, e abort.
        const auto v = sample_count::trusted(from_env(2));
        std::print(stderr, "  FAILED: the violation did NOT abort; value returned: {}\n", v.get());
        return 1;
#endif
    }

    std::print("  sanity of the three contract gates\n\n");
    const int failures = gates_t_and_r() + gate_e();
    std::print("\n  {} failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
