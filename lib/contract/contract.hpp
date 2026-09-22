// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Contrato de runtime: verificado em DEBUG, entregue ao otimizador em RELEASE.
//
// ESTE É O MECANISMO DA PORTA R
//
// A norma do projeto (docs/padrao-do-projeto.md, seção 13) define três portas
// de contrato, e este arquivo implementa exatamente uma delas:
//
//   porta T  valor constante          -> `static_assert`, em domain.hpp
//   porta R  valor interno confiável  -> ESTE ARQUIVO
//   porta E  valor externo            -> `std::expected`, em domain.hpp
//
// A porta R vale para o valor cuja violação é **bug de programador**. Não vale
// para dado que vem de `/sys`, `/proc`, linha de comando ou arquivo: aquilo não
// é bug, é ambiente, e a verificação dele nunca é compilada fora.
//
// POR QUE `[[assume]]` E NÃO OS CONTRACTS DE C++26
//
// Não é preferência de estilo. Foi MEDIDO em 16/09/2026 que o GCC 15.2.0 desta
// máquina rejeita `int f(int x) pre(x>0)` mesmo com `-std=c++2c -fcontracts`,
// com `expected initializer before 'pre'`. `[[assume]]` é de C++23 (P1774) e
// compila em GCC 15.2.0 e Clang 21.1.8. Ver docs/referencias.md#p2900.
//
// A RELAÇÃO COM `assert`, QUE É A ORIGEM DESTE DESENHO
//
// `PERF_EXPECTS` é **`assert` mais `[[assume]]`**. A semântica de ativação é a
// do `assert` e vem do mesmo gatilho: `NDEBUG` definido desliga a verificação.
// Ver [CPP-ASSERT] em docs/referencias.md.
//
// A pergunta natural é por que não usar `assert` direto. A resposta é medida.
//
//   MEDIDO em 16/09/2026, GCC 15.2.0, `-O2`, com o mesmo caminho morto que
//   ferramental/qualidade/verificar-suposicao.py usa como sonda:
//
//     configuração        assert sozinho              assert + [[assume]]
//     -----------------   -------------------------   --------------------
//     sem NDEBUG          verificado; morto sai       verificado; morto sai
//     com NDEBUG          NAO verificado; morto FICA  nao verificado; morto sai
//
// A linha de baixo é o ponto. Com `NDEBUG`, `assert` expande para `((void) 0)`:
// não sobra nada, nem a verificação nem a INFORMAÇÃO. A verificação redundante
// a jusante volta a existir no binário de release -- exatamente a condicional
// que a segunda diretriz do projeto quer eliminar.
//
// E o caso de cima é uma armadilha de leitura: em debug o caminho morto É
// eliminado, porque o próprio ramo do `assert` informa a faixa ao otimizador. Um
// experimento que só olhasse o build de debug concluiria que `assert` basta.
//
// Daí as duas metades: `assert` dá a verificação em debug, `[[assume]]` dá a
// informação em release. Nenhuma das duas sozinha atende às duas diretrizes.
//
// POR QUE NÃO DELEGAR AO `assert` DA BIBLIOTECA NO RAMO DE DEBUG
//
// Três razões, e a primeira é dura:
//
//   1. `assert` é macro de UM argumento, e isto foi conferido:
//      `assert(std::is_same_v<int, int>)` não compila -- `macro 'assert' passed
//      2 arguments, but takes just 1`. Num projeto cheio de template, um
//      predicado com vírgula dentro de `<>` aparece no primeiro domínio
//      genérico. `PERF_EXPECTS` é variádico e não tem esse limite;
//   2. `assert` não distingue pré-condição de pós-condição na mensagem, e as
//      duas têm culpados diferentes: a pré é de quem chamou, a pós é de quem
//      implementou;
//   3. a mensagem deste projeto precisa dizer mais do que "asserção falhou" --
//      ela explica a confusão de porta, que é o erro conceitual mais provável
//      aqui: dado externo verificado pela porta R em vez da E.
//
// O que se herda do `assert` é o que importa: o gatilho `NDEBUG`, e o
// comportamento em avaliação constante. Um `assert` falso em `constexpr` já
// quebra o build, porque alcança `__assert_fail`, que não é `constexpr`
// (conferido). `detail::violated()` abaixo é não-`constexpr` pela mesma razão e
// com o mesmo efeito.
//
// POR QUE MACRO, E POR QUE ISSO ESTÁ CERTO AQUI
//
// Uma função não serve, por três razões independentes:
//
//   1. `[[assume]]` precisa do predicado NO LOCAL DE CHAMADA e não avaliado.
//      Passar o predicado a uma função o avaliaria -- que é exatamente o custo
//      que se quer eliminar em release;
//   2. o TEXTO do predicado (`#expr`) só existe na expansão do pré-processador;
//   3. `std::source_location::current()` como argumento default captura o local
//      da chamada, mas o texto não.
//
// E a macro é a **costura de migração**: quando os Contracts de C++26 chegarem
// ao toolchain, o corpo destas duas macros muda e **nenhum ponto de chamada é
// tocado**. É por isso que `.clang-tidy` desliga `cppcoreguidelines-macro-usage`
// com a razão escrita no próprio arquivo.
//
// ================================ ADVERTÊNCIA ================================
//
// **Predicado falso em `[[assume]]` é comportamento indefinido.** O otimizador
// tem licença para eliminar caminhos inteiros do código a partir dele. Um
// contrato errado NÃO vira um assert que falha: vira uma release silenciosamente
// miscompilada, e o sintoma aparece longe da causa.
//
// É por isso que a matriz de build tem TRÊS configurações e não duas
// (docs/padrao-do-projeto.md, seção 16). A que importa é a terceira:
//
//   meson setup build-release-checked -Dbuildtype=release -Db_ndebug=false
//
// Ela roda com `-O3` E com os contratos verificados, e é a única que prova que
// o que se assume é verdade no código otimizado. Sem ela, o projeto assume sem
// nunca verificar o que assumiu.
//
// O PREDICADO PRECISA SER LIVRE DE EFEITO COLATERAL
//
// Em release ele não é avaliado; em debug ele é. Um predicado com efeito
// colateral faz as duas configurações executarem programas diferentes, e o
// defeito só aparece na que ninguém depura.
//
// E não é só disciplina de estilo: o compilador pode DESCARTAR a suposição
// inteira quando não consegue provar que o predicado é puro. Daí
// `PERF_PURE_PREDICATE`, abaixo.
//
// ================ O DEFEITO QUE `PERF_PURE_PREDICATE` EVITA ================
//
// MEDIDO em 16/09/2026, com o predicado sendo a chamada `D::accepts(v)` -- que
// é a forma que o desenho de domínios deste projeto usa em toda parte:
//
//   compilador   configuração              verificação redundante a jusante
//   ----------   -----------------------   --------------------------------
//   GCC 15.2     sem assume                sobrevive
//   GCC 15.2     assume, sem gnu::const    ELIMINADA
//   GCC 15.2     assume + gnu::const       ELIMINADA
//   Clang 21.1   sem assume                sobrevive
//   Clang 21.1   assume, sem gnu::const    **SOBREVIVE**
//   Clang 21.1   assume + gnu::const       ELIMINADA
//
// A linha em negrito é o problema: o Clang trata a chamada de função como
// possivelmente com efeito colateral e **ignora a suposição**, avisando por
// `-Wassume` -- um aviso, não um erro. Sem o atributo, a porta R em Clang é um
// no-op que PARECE entregar a pré-condição ao otimizador. É "verde
// indistinguível do legítimo" na forma mais direta possível.
//
// Três medidas, porque uma só não basta:
//
//   1. `PERF_PURE_PREDICATE` em todo `accepts()` de domínio (domain.hpp);
//   2. `-Werror=assume` no Clang, registrado no meson.build. O GCC **não tem**
//      `-Wassume` -- conferido: `cc1plus: error: '-Werror=assume': no option
//      '-Wassume'` --, e também não precisa, porque honra a suposição sem o
//      atributo;
//   3. um teste de CÓDIGO GERADO na suíte, que compila uma sonda e confere no
//      assembly que a verificação redundante desapareceu. É o único dos três
//      que pega a regressão em qualquer compilador, inclusive num futuro que
//      mude de opinião. Ver ferramental/qualidade/verificar-suposicao.py.

#ifndef PERF_CONTRACT_CONTRACT_HPP
#define PERF_CONTRACT_CONTRACT_HPP

#include <cstdlib>
#include <print>
#include <source_location>
#include <string_view>

namespace perf::contract
{

namespace detail
{

// NÃO é `constexpr`, e a ausência é o mecanismo.
//
// Em avaliação constante, alcançar uma função não-`constexpr` é erro de
// compilação -- então um contrato violado com valores conhecidos em compilação
// quebra o BUILD, sem precisar de `if consteval` nem de um segundo caminho.
// Em runtime, aborta.
//
// `cold` e `noinline` tiram o tratamento do caminho quente. Isto não é
// especulação: no projeto irmão, um `abort()` embutido numa função inline de
// medição degradou o laço chamador em 17% no lote 32 e 28% no lote 128, em dez
// execuções por ponto. O custo não era o do teste -- era o da otimização
// perdida no corpo que o hospedava.
[[noreturn]] [[gnu::cold, gnu::noinline]] inline void violated(
    std::string_view predicate, std::string_view kind, const std::source_location &where)
{
    std::print(stderr,
               "\nCONTRATO VIOLADO ({})\n"
               "  predicado : {}\n"
               "  em        : {}:{}\n"
               "  função    : {}\n\n"
               "Isto é bug de programador, não condição de ambiente: a porta R\n"
               "do contrato só protege valor interno. Se este valor veio de\n"
               "/sys, /proc, arquivo ou linha de comando, ele está na porta\n"
               "errada -- use a porta E (parse/std::expected).\n",
               kind, predicate, where.file_name(), where.line(), where.function_name());
    std::abort();
}

} // namespace detail

// Verificação de contrato em modo DEBUG.
//
// `constexpr` de propósito: mantém `constexpr` toda função que a use, de modo
// que um domínio permanece avaliável em compilação mesmo com o contrato ligado.
// É o que permite que `checked::trusted()` seja usável em `static_assert`.
constexpr void check(bool ok, std::string_view predicate, std::string_view kind,
                     const std::source_location &where = std::source_location::current())
{
    if (!ok)
    {
        detail::violated(predicate, kind, where);
    }
}

} // namespace perf::contract

// Marca o predicado como puro: sem efeito colateral e sem ler estado global,
// dependendo apenas dos argumentos. É verdade para todo `accepts()` de domínio
// deste projeto, que é aritmética sobre o próprio valor.
//
// Sem isto, o Clang descarta a suposição. Ver a tabela medida acima.
#if __has_cpp_attribute(gnu::const)
#define PERF_PURE_PREDICATE [[gnu::const]]
#else
#define PERF_PURE_PREDICATE
#endif

// As macros usam `...` e `__VA_ARGS__`, não um parâmetro nomeado.
//
// POR QUE: o pré-processador protege vírgulas dentro de PARÊNTESES, mas não
// dentro de sinais de menor/maior. Com um parâmetro único,
// `PERF_EXPECTS(std::is_same_v<A, B>)` seria lido como dois argumentos e não
// compilaria -- num projeto cheio de template, isso apareceria no primeiro
// domínio genérico.
#ifdef NDEBUG

// Release: o predicado NÃO é avaliado. Ele é entregue ao otimizador, que pode
// usá-lo para eliminar verificações redundantes a jusante e propagar constantes.
#define PERF_EXPECTS(...) [[assume(__VA_ARGS__)]]
#define PERF_ENSURES(...) [[assume(__VA_ARGS__)]]

#else

// Debug: mesma semântica de ativação do `assert` -- `NDEBUG` ausente, logo
// verifica -- com o diagnóstico do projeto. Ver a comparação medida no topo.
#define PERF_EXPECTS(...)                                                                          \
    ::perf::contract::check(static_cast<bool>(__VA_ARGS__), #__VA_ARGS__,                          \
                            "pré-condição", ::std::source_location::current())

#define PERF_ENSURES(...)                                                                          \
    ::perf::contract::check(static_cast<bool>(__VA_ARGS__), #__VA_ARGS__,                          \
                            "pós-condição", ::std::source_location::current())

#endif

#endif // PERF_CONTRACT_CONTRACT_HPP
