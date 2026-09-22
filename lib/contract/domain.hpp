// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Domínio de atributo: o termo de aceite escrito UMA VEZ, consumido por três
// portas.
//
// A IDEIA CENTRAL
//
// A diretriz de implementação do projeto pede contrato explícito por atributo,
// sem condicional de validação redundante no código otimizado. O que unifica
// isso é não duplicar o predicado:
//
//     struct sample_count_domain : closed_range<int, 3, 1'000'000> { ... };
//
// e então o MESMO `accepts()` serve às três portas:
//
//     sample_count::of<25>()      porta T  -> static_assert, erro de COMPILAÇÃO
//     sample_count::trusted(v)    porta R  -> `assert` em DEBUG, `[[assume]]` em release
//     sample_count::parse(v)      porta E  -> std::expected, SEMPRE verificado
//
// Escrever o termo de aceite três vezes seria a forma óbvia e errada: as três
// cópias divergem, e a que divergir em silêncio será a que valida a entrada
// externa.
//
// POR QUE A PORTA E NUNCA É COMPILADA FORA
//
// Um número malformado numa variável de ambiente, num arquivo de configuração
// ou na linha de comando não é bug de programador: é dado de ambiente. Se a
// validação de entrada externa desaparecesse em release, o programa passaria a
// confiar em conteúdo que ninguém controla -- isso é furo de correção, não
// otimização.
//
// `parse()` devolve `std::expected` e não lança: exceção tem custo
// imprevisível ao desenrolar a pilha, e código de retorno numérico é fácil de
// ignorar. `std::expected` com `[[nodiscard]]` dá os dois -- sem custo de
// exceção, e o compilador reclama se o erro for ignorado. É a mesma escolha,
// pela mesma razão, que a alternativa em C++23 do projeto irmão já fez.
//
// O GANHO QUE FECHA A DIRETRIZ
//
// Se o domínio é `constexpr`, **o teste do domínio é `static_assert`**: custo
// zero em runtime, e a violação quebra o BUILD em vez de quebrar a suíte. Ver
// os asserts ao fim deste arquivo e em measurement_domains.hpp.

#ifndef PERF_CONTRACT_DOMAIN_HPP
#define PERF_CONTRACT_DOMAIN_HPP

#include "lib/contract/contract.hpp"

#include <bit>
#include <concepts>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

namespace perf::contract
{

// O que faz de um tipo um domínio.
//
// `name` é exigido, e não é enfeite: é o que torna útil a mensagem de erro da
// porta E. Um `std::unexpected` que diz "valor 2 fora de faixa" obriga quem lê o
// log a descobrir de qual parâmetro se fala; um que diz "contagem de amostras"
// não.
template <typename D>
concept domain = requires {
    typename D::value_type;
    { D::accepts(typename D::value_type{}) } -> std::convertible_to<bool>;
    { D::name } -> std::convertible_to<std::string_view>;
};

// Falha da porta E: valor externo recusado.
//
// Carrega o valor ofensor E o nome do domínio. Não carrega os limites porque
// não todo domínio tem limites -- um conjunto enumerado não tem faixa --, e um
// campo que só faz sentido em metade dos casos convida a mensagem errada.
template <typename T>
struct domain_error
{
    T offending{};
    std::string_view domain_name{};

    [[nodiscard]] std::string describe() const
    {
        return std::format("value {} rejected by domain '{}'", offending, domain_name);
    }
};

// --- Domínios genéricos ---------------------------------------------------

// Faixa fechada [Lo, Hi]. O caso mais comum.
template <std::integral T, T Lo, T Hi>
struct closed_range
{
    using value_type = T;

    // Porta T aplicada ao PRÓPRIO domínio: uma faixa invertida é erro de quem
    // declarou o domínio, e é decidível em compilação. Não há razão para que
    // isso seja verificável só em runtime.
    static_assert(Lo <= Hi, "domínio invertido: Lo > Hi");

    static constexpr T lowest = Lo;
    static constexpr T highest = Hi;

    PERF_PURE_PREDICATE static constexpr bool accepts(T v) noexcept { return v >= Lo && v <= Hi; }
};

// Conjunto finito de valores aceitos. Ex.: um modo de operação com três
// alternativas nomeadas.
//
// `accepts` é uma expressão de dobra sobre o pacote -- **sem laço e sem
// tabela**. Isso não é a tese de compile-time em ação, é ganho estrutural: o
// compilador vê N comparações com constantes e as resolve como quiser. A tese
// só entra quando N cresce o suficiente para que uma tabela seja hipótese
// plausível, e aí é experimento, não estilo (padrão do projeto, seção 14).
template <std::integral T, T... Vs>
struct enumerated
{
    using value_type = T;

    static_assert(sizeof...(Vs) > 0, "domínio enumerado vazio não aceita nada");

    static constexpr std::size_t size = sizeof...(Vs);

    PERF_PURE_PREDICATE static constexpr bool accepts(T v) noexcept { return ((v == Vs) || ...); }
};

// Potência de dois dentro de uma faixa. Ex.: alinhamento em bytes.
//
// Usa `std::has_single_bit` de `<bit>`, que é instrução dedicada na maioria dos
// alvos -- em vez de laço de deslocamento ou de tabela dos valores válidos.
// Ganho estrutural, também não é a tese.
template <std::unsigned_integral T, T Lo, T Hi>
struct power_of_two_in
{
    using value_type = T;

    static_assert(Lo <= Hi, "domínio invertido: Lo > Hi");
    static_assert(std::has_single_bit(Lo), "limite inferior não é potência de dois");
    static_assert(std::has_single_bit(Hi), "limite superior não é potência de dois");

    static constexpr T lowest = Lo;
    static constexpr T highest = Hi;

    PERF_PURE_PREDICATE static constexpr bool accepts(T v) noexcept
    {
        return v >= Lo && v <= Hi && std::has_single_bit(v);
    }
};

// --- O valor verificado, e as três portas ---------------------------------

template <domain D>
class checked
{
public:
    using value_type = typename D::value_type;
    using domain_type = D;

    // ------------------------------------------------------------------
    // PORTA T -- valor conhecido em compilação.
    //
    // O termo de aceite vira erro de COMPILAÇÃO. Custo em release: zero, e não
    // "quase zero" -- não há código.
    //
    // É `static_assert` e não uma cláusula `requires` de propósito: as duas
    // rejeitam o valor, mas a `requires` produz "nenhuma sobrecarga
    // corresponde", que manda quem lê procurar o que não existe, enquanto o
    // `static_assert` diz qual regra foi violada. Diagnóstico é interface.
    // ------------------------------------------------------------------
    template <value_type V>
    [[nodiscard]] static constexpr checked of() noexcept
    {
        static_assert(D::accepts(V), "value outside the domain's acceptance terms");
        return checked{V};
    }

    // ------------------------------------------------------------------
    // PORTA R -- valor interno, cuja violação é bug de programador.
    //
    // Em DEBUG o predicado é verificado e a violação relatada com o texto do
    // predicado e o local. Em RELEASE ele é entregue ao otimizador e não é
    // avaliado.
    //
    // O nome é `trusted` e não `from` porque a assinatura precisa dizer o que
    // ela assume. `from(v)` pareceria uma conversão segura; `trusted(v)` avisa
    // que a segurança é responsabilidade de quem chama.
    // ------------------------------------------------------------------
    [[nodiscard]] static constexpr checked trusted(value_type v) noexcept
    {
        PERF_EXPECTS(D::accepts(v));
        return checked{v};
    }

    // ------------------------------------------------------------------
    // PORTA E -- valor externo. SEMPRE verificado, em toda configuração.
    //
    // `[[nodiscard]]` não é decoração: um `parse()` cujo resultado é descartado
    // é precisamente a validação que não aconteceu.
    // ------------------------------------------------------------------
    [[nodiscard]] static constexpr std::expected<checked, domain_error<value_type>> parse(
        value_type v) noexcept
    {
        if (!D::accepts(v))
        {
            return std::unexpected(domain_error<value_type>{v, D::name});
        }
        return checked{v};
    }

    [[nodiscard]] constexpr value_type get() const noexcept { return value_; }

    // Comparação entre valores do MESMO domínio. Não há conversão implícita
    // para `value_type`: se houvesse, `checked<nice>` e `checked<rt_priority>`
    // voltariam a se comparar entre si através de `int`, que é a classe de erro
    // que este tipo existe para eliminar.
    [[nodiscard]] friend constexpr bool operator==(checked, checked) noexcept = default;
    [[nodiscard]] friend constexpr auto operator<=>(checked, checked) noexcept = default;

private:
    explicit constexpr checked(value_type v) noexcept : value_{v} {}

    value_type value_;
};

// --- Testes em tempo de compilação ---------------------------------------

namespace
{

struct test_range : closed_range<int, -20, 19>
{
    static constexpr std::string_view name = "faixa de teste";
};

struct test_set : enumerated<unsigned, 0U, 1U, 2U>
{
    static constexpr std::string_view name = "conjunto de teste";
};

struct test_pow2 : power_of_two_in<unsigned long, 4096UL, 1073741824UL>
{
    static constexpr std::string_view name = "potencia de dois de teste";
};

static_assert(domain<test_range>);
static_assert(domain<test_set>);
static_assert(domain<test_pow2>);

// Faixa fechada: os dois limites DENTRO, e o primeiro valor fora de cada lado.
static_assert(test_range::accepts(-20));
static_assert(test_range::accepts(19));
static_assert(test_range::accepts(0));
static_assert(!test_range::accepts(-21));
static_assert(!test_range::accepts(20));

// Conjunto enumerado: dentro, fora, e o vizinho imediato do maior.
static_assert(test_set::accepts(0U));
static_assert(test_set::accepts(2U));
static_assert(!test_set::accepts(3U));
static_assert(test_set::size == 3);

// Potência de dois: limites, potência interna, e os dois modos de recusa --
// fora da faixa, e dentro da faixa mas não sendo potência de dois. O segundo é
// o que uma faixa simples aceitaria por engano.
static_assert(test_pow2::accepts(4096UL));
static_assert(test_pow2::accepts(2097152UL));
static_assert(test_pow2::accepts(1073741824UL));
static_assert(!test_pow2::accepts(2048UL));
static_assert(!test_pow2::accepts(4097UL));
static_assert(!test_pow2::accepts(3145728UL)); // 3 MiB: na faixa, não é potência de dois

// PORTA T em constante: o valor atravessa e o tipo preserva o domínio.
static_assert(checked<test_range>::of<0>().get() == 0);
static_assert(checked<test_range>::of<-20>().get() == -20);

// PORTA R em avaliação constante. Isto prova que `trusted()` continua
// `constexpr` COM o contrato ligado -- se `check()` não fosse `constexpr`, este
// assert não compilaria em build de debug, e o contrato teria custado a
// avaliação em compilação.
static_assert(checked<test_range>::trusted(5).get() == 5);

// PORTA E: aceita, recusa, e a recusa carrega valor e nome do domínio.
static_assert(checked<test_range>::parse(19).has_value());
static_assert(checked<test_range>::parse(19)->get() == 19);
static_assert(!checked<test_range>::parse(20).has_value());
static_assert(checked<test_range>::parse(20).error().offending == 20);
static_assert(checked<test_range>::parse(20).error().domain_name == "faixa de teste");

// Comparação dentro do domínio.
static_assert(checked<test_range>::of<1>() == checked<test_range>::trusted(1));
static_assert(checked<test_range>::of<1>() < checked<test_range>::of<2>());

} // namespace

} // namespace perf::contract

#endif // PERF_CONTRACT_DOMAIN_HPP
