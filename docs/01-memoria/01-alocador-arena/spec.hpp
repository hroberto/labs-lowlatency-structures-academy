// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// A LEI DO TÓPICO 01.01 — o contrato que `std/` e `custom/` cumprem.
//
// POR QUE `spec.hpp` E NÃO `contract.hpp`
//
// Neste repositório **contrato** já significa outra coisa: o mecanismo das três
// portas de `lib/contract/` (norma, seção 13). A interface comum de um par
// `std` e `custom` é a **spec** do tópico. Ver a seção 34 da norma.
//
// O QUE A SPEC DECLARA, E O QUE ELA NÃO IMPÕE
//
// Ela declara **sete invariantes**. Cinco valem para qualquer implementação; as
// outras duas valem **só para quem as honra**, e cada braço diz em `traits`
// quais são as suas.
//
// Isso não é frouxidão: é o assunto do tópico. `std::allocator` libera objeto
// individual e não tem teto; a arena não libera individual e tem teto. Exigir
// as duas coisas dos dois produziria uma spec que só a `std` cumpre — e a
// comparação viraria "a arena é uma std::allocator pior".
//
// > Uma spec que os dois braços cumprem sem ressalva costuma ser sinal de que
// > ela foi escrita fraca o suficiente para caber nos dois.
//
// A divergência fica em CÓDIGO, e não em prosa: o teste L1 é parametrizado pela
// spec e consulta os `traits`. Uma invariante que um braço não honra vira caso
// **pulado com motivo**, nunca caso ausente.

#ifndef ACADEMY_MEMORY_ARENA_SPEC_HPP
#define ACADEMY_MEMORY_ARENA_SPEC_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace academy::memory
{

// O objeto sob alocação: uma linha de cache exata.
//
// POR QUE 64 BYTES, E POR QUE ISSO É DECISÃO E NÃO DETALHE
//
// Um objeto menor que a linha faz dois objetos vizinhos compartilharem linha, e
// o tópico passaria a medir também falso compartilhamento -- que é assunto do
// módulo 02, não deste. Um objeto muito maior diluiria o custo de alocar no
// custo de tocar. 64 bytes isola o que este tópico quer medir.
//
// O `alignas` não é enfeite: sem ele, a arena poderia devolver ponteiro que
// atravessa linha, e o `static_assert` abaixo é o que impede isso de passar.
struct alignas(64) node
{
    std::uint64_t id = 0;
    std::uint64_t payload[7] = {};
};

static_assert(sizeof(node) == 64, "o no deve ocupar exatamente uma linha de cache");
static_assert(alignof(node) == 64, "o no deve estar alinhado a linha de cache");

// O que cada braço declara sobre si.
//
// São as duas invariantes condicionais da spec, e mais o nome que aparece na
// tabela publicada. Um braço novo que esqueça de declarar não compila: o
// concept abaixo exige os três campos.
struct arm_traits
{
    // Devolve um objeto por vez, sem devolver o resto?
    bool frees_individually;
    // Tem teto declarado, e devolve `nullptr` quando ele acaba?
    bool bounded;
    std::string_view name;
};

// A SPEC.
//
// `release_all` recebe os ponteiros que o CHAMADOR guardou, e não os guarda
// sozinho -- e isso é escolha de simetria, não de conveniência.
//
// A `std::allocator` precisa saber quais ponteiros devolver; a arena não
// precisa de nenhum. Se cada braço guardasse os seus, o braço `std` pagaria
// também o custo de manter a lista, e a tabela publicaria como "custo de
// alocar" algo que é custo de escriturar. Com a lista do lado de fora, os dois
// braços pagam a mesma escrituração e a diferença medida é a que o tópico
// afirma medir (norma, seção 11).
template <typename A>
concept batch_allocator = requires(A a, std::span<node *const> devolvidos) {
    { a.acquire() } -> std::same_as<node *>;
    { a.release_all(devolvidos) } -> std::same_as<void>;
    { a.outstanding() } -> std::same_as<std::size_t>;
    { A::traits } -> std::convertible_to<arm_traits>;
};

// AS SETE INVARIANTES, nomeadas para que o teste L1 e o documento usem as
// mesmas palavras. Um caso de teste que não corresponda a uma delas é caso sem
// lei -- e uma lei sem caso é promessa.
enum class invariant
{
    distinct_pointers,   // 1. cada acquire devolve ponteiro distinto
    usable_memory,       // 2. o que se escreve no objeto volta a ser lido
    aligned,             // 3. o ponteiro respeita alignof(node)
    release_all_clears,  // 4. release_all zera outstanding()
    reusable_after_all,  // 5. depois de release_all, acquire volta a servir
    exhaustion_returns_null,  // 6. SÓ para bounded: teto acabado devolve nullptr
    individual_release,       // 7. SÓ para frees_individually
};

} // namespace academy::memory

#endif // ACADEMY_MEMORY_ARENA_SPEC_HPP
