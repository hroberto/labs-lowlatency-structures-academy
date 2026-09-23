// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// A LEI DO TÓPICO 02.01 — mesmo dado lógico, dois layouts.
//
// O QUE A SPEC TEM DE GARANTIR, E É O MAIS IMPORTANTE DESTE TÓPICO
//
// Os dois braços guardam **o mesmo dado**. Se um deles guardar menos, ou em
// outro tipo, a comparação mede a diferença de dado e publica com o nome de
// layout -- e esse é o modo de falha clássico de um benchmark de SoA contra AoS.
//
// A invariante 1 existe para isso: escrever o mesmo conteúdo nos dois e exigir
// que as duas travessias devolvam **exatamente** a mesma soma. Não é
// verificação de aritmética; é verificação de que a comparação é legítima.
//
// AS DUAS OPERAÇÕES, E POR QUE SÃO DUAS
//
//   `sum_field(k)`      percorre todos os registros lendo UM campo.
//   `touch_record(i)`   escreve em TODOS os campos de um registro.
//
// A primeira favorece SoA: lê 8 bytes por registro, e num layout por campo isso
// significa oito registros por linha de cache. A segunda favorece AoS: escreve
// 64 bytes num registro, que num layout por registro é uma linha só.
//
// Medir só a primeira publicaria metade da verdade. A *Regra de decisão* deste
// tópico depende de **qual das duas operações domina o seu caminho quente**, e
// por isso as duas são medidas e as duas entram na tabela.

#ifndef ACADEMY_LAYOUT_SOA_SPEC_HPP
#define ACADEMY_LAYOUT_SOA_SPEC_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace academy::layout
{

// Oito campos de 8 bytes: um registro ocupa exatamente uma linha de cache.
//
// O número não é arbitrário. Com oito campos, uma travessia de um campo lê 1/8
// de cada linha no AoS e 8/8 no SoA -- a razão entre os dois layouts fica
// legível na tabela sem precisar de conta. Com três campos, o efeito existiria e
// ficaria embaçado por padding.
inline constexpr std::size_t kCampos = 8;
inline constexpr std::size_t kBytesPorRegistro = kCampos * sizeof(std::uint64_t);

static_assert(kBytesPorRegistro == 64, "o registro deve ocupar uma linha de cache");

struct arm_traits
{
    // O layout guarda os campos de um registro juntos?
    bool record_contiguous;
    std::string_view name;
};

template <typename S>
concept record_store = requires(S s, std::size_t i, std::size_t k, std::uint64_t v) {
    { s.sum_field(k) } -> std::same_as<std::uint64_t>;
    { s.touch_record(i, v) } -> std::same_as<void>;
    { s.set(i, k, v) } -> std::same_as<void>;
    { s.get(i, k) } -> std::same_as<std::uint64_t>;
    { s.size() } -> std::same_as<std::size_t>;
    { S::traits } -> std::convertible_to<arm_traits>;
};

// AS QUATRO INVARIANTES.
enum class invariant
{
    same_data,         // 1. os dois layouts guardam o mesmo dado lógico
    sum_matches,       // 2. sum_field concorda com a soma feita por get()
    touch_writes_all,  // 3. touch_record escreve em todos os campos
    independent_fields // 4. escrever um campo não altera os outros
};

} // namespace academy::layout

#endif // ACADEMY_LAYOUT_SOA_SPEC_HPP
