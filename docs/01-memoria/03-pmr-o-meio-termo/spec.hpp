// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// A LEI DO TÓPICO 01.03 — e a pergunta que ela responde é diferente.
//
// O 01.01 e o 01.02 compararam `std::allocator` com algo escrito à mão. Este
// compara `std::allocator` com **outra coisa da biblioteca padrão**:
// `std::pmr::monotonic_buffer_resource`, que é a arena que o C++17 já traz.
//
// POR QUE ISSO IMPORTA, E POR QUE É O TERCEIRO E NÃO O PRIMEIRO
//
// A tese do projeto é medir onde a biblioteca padrão para. Nos dois tópicos
// anteriores ela parou, e o complemento artesanal ganhou 27,9x e 3,0x. Aqui a
// pergunta é outra: **a própria biblioteca padrão já oferece o mecanismo?**
//
// Se a resposta for sim, os dois tópicos anteriores continuam certos e ganham
// uma ressalva cara: eles mediram o custo de `std::allocator`, não o custo do
// que a linguagem oferece. A ordem é esta de propósito -- o meio-termo só se
// avalia depois de conhecidos os dois extremos.
//
// A SPEC É A DO 01.01, E NÃO UMA NOVA
//
// O padrão medido é o mesmo: lote que nasce e morre inteiro. Trocar a spec
// tornaria os números incomparáveis com os do 01.01, e a comparação com eles é
// a razão deste tópico existir.

#ifndef ACADEMY_MEMORY_PMR_SPEC_HPP
#define ACADEMY_MEMORY_PMR_SPEC_HPP

#include "docs/01-memoria/01-alocador-arena/spec.hpp"

namespace academy::memory
{

// Nada novo a declarar: a lei é a `batch_allocator` do 01.01, com as mesmas
// sete invariantes. Este arquivo existe para dizer isso em um lugar, em vez de
// o leitor ter de deduzir do `#include`.
//
// As duas invariantes condicionais se comportam como na arena: o
// `monotonic_buffer_resource` não libera objeto individual (invariante 7) e,
// configurado sobre um buffer fixo com `null_memory_resource` de reserva,
// declara teto (invariante 6).

} // namespace academy::memory

#endif // ACADEMY_MEMORY_PMR_SPEC_HPP
