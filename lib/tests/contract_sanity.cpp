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
// Com `NDEBUG`, `PERF_EXPECTS` vira `[[assume]]`: violar o predicado é
// comportamento INDEFINIDO, não abort. Um teste que esperasse falha ali estaria
// esperando um comportamento que o padrão não promete. Por isso `--violar`
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
T do_ambiente(T simulado)
{
    volatile T opaco = simulado;
    return opaco;
}

int porta_e()
{
    int falhas = 0;

    // Valor válido vindo "de fora".
    const auto bom = sample_count::parse(do_ambiente(25));
    if (!bom.has_value())
    {
        std::print(stderr, "  FALHA: porta E recusou 25, que esta no dominio\n");
        ++falhas;
    }
    else if (bom->get() != 25)
    {
        std::print(stderr, "  FALHA: porta E alterou o valor aceito\n");
        ++falhas;
    }

    // Valor inválido vindo "de fora": tem de ser recusado em TODA configuração,
    // inclusive em release. É a regra que a norma chama de "a porta E nunca é
    // compilada fora".
    //
    // O 2 é o caso realista: `CPP_ACADEMY_AMOSTRAS=2` é o que alguém escreveria
    // para acelerar a CI, e duas amostras não sustentam quartil nenhum.
    const auto ruim = sample_count::parse(do_ambiente(2));
    if (ruim.has_value())
    {
        std::print(stderr, "  FALHA: porta E aceitou 2 amostras, fora do dominio."
                           " A validacao externa foi compilada fora?\n");
        ++falhas;
    }
    else
    {
        const auto texto = ruim.error().describe();
        // A mensagem precisa identificar o PARAMETRO, nao so a faixa: num log
        // de diagnostico, "valor fora de faixa" obriga quem le a descobrir de
        // qual dos seis parametros se fala.
        if (texto.find("contagem de amostras") == std::string::npos)
        {
            std::print(stderr, "  FALHA: mensagem da porta E nao nomeia o parametro: {}\n", texto);
            ++falhas;
        }
        else
        {
            std::print("  porta E: 2 recusado -- {}\n", texto);
        }
    }

    // Outro domínio: o erro precisa mudar de nome junto. Um `domain_name`
    // fixo passaria no caso acima e falharia aqui.
    const auto rodadas = rounds_per_sample::parse(do_ambiente(999));
    if (rodadas.has_value() || rodadas.error().domain_name != "rodadas por amostra")
    {
        std::print(stderr, "  FALHA: porta E de rodadas nao identificou o dominio\n");
        ++falhas;
    }

    return falhas;
}

int portas_t_e_r()
{
    int falhas = 0;

    // Porta T: já validada em compilação; aqui só se confirma que o valor
    // atravessa intacto até o runtime.
    if (sample_count::of<25>().get() != 25 || alignment::of<64>().get() != 64)
    {
        std::print(stderr, "  FALHA: porta T alterou o valor\n");
        ++falhas;
    }

    // Porta R com valor válido: não deve disparar em nenhuma configuração.
    if (rounds_per_sample::trusted(do_ambiente(200'000)).get() != 200'000)
    {
        std::print(stderr, "  FALHA: porta R alterou o valor\n");
        ++falhas;
    }

    // Orçamento declarado.
    using feed = perf::contract::workload_contract<5'000'000>;
    if (feed::budget_ns != 200.0 || !feed::fits(200.0) || feed::fits(200.1))
    {
        std::print(stderr, "  FALHA: orcamento declarado nao confere\n");
        ++falhas;
    }
    std::print("  orcamento: {} eventos/s -> {:.1f} ns por evento\n", feed::events_per_second,
               feed::budget_ns);

    return falhas;
}

} // namespace

int main(int argc, char **argv)
{
    const bool violar = argc > 1 && std::string_view{argv[1]} == "--violar";

    if (violar)
    {
#ifdef NDEBUG
        std::print("  PULADO - violacao de contrato em release e comportamento\n"
                   "  INDEFINIDO, nao abort: `PERF_EXPECTS` virou `[[assume]]`.\n"
                   "  Este caso NAO passou: ele nao roda nesta configuracao.\n"
                   "  Ele roda em build-debug e em build-release-checked.\n");
        return 77;
#else
        std::print("  disparando violacao deliberada da porta R"
                   " (2 amostras, abaixo do piso de 3)\n");
        // Esperado: mensagem de contrato violado em stderr, e abort.
        const auto v = sample_count::trusted(do_ambiente(2));
        std::print(stderr, "  FALHA: a violacao NAO abortou; valor devolvido: {}\n", v.get());
        return 1;
#endif
    }

    std::print("  sanidade das tres portas de contrato\n\n");
    const int falhas = portas_t_e_r() + porta_e();
    std::print("\n  {} falha(s)\n", falhas);
    return falhas == 0 ? 0 : 1;
}
