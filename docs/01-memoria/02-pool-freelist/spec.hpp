// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// A LEI DO TÓPICO 01.02 — o que a arena não podia cumprir.
//
// POR QUE ESTA SPEC É OUTRA, E NÃO A DO 01.01
//
// O tópico anterior mediu o par sob um padrão em que **o lote morre inteiro**,
// e ali a liberação individual era invariante CONDICIONAL: a arena não a honra,
// e o caso dela era pulado com motivo.
//
// Aqui ela é **obrigatória para os dois braços**, e a mudança de status é o
// assunto do tópico. A pergunta deixa de ser "quanto custa alocar" e passa a
// ser **quanto custa manter a liberação individual** -- que é o preço que a
// arena não paga porque ela não oferece.
//
// Uma spec única para os três tópicos do módulo teria de ser fraca o bastante
// para caber na arena, e aí o pool pareceria uma arena mais lenta. Cada tópico
// tem a sua lei, e a diferença entre as leis é o que o módulo ensina.
//
// O PADRÃO DE USO TAMBÉM É OUTRO
//
// Lote que nasce e morre inteiro é o 01.01. Aqui o padrão é **rotatividade em
// regime**: um conjunto de objetos vivos de tamanho aproximadamente constante,
// em que cada passo libera um e adquire outro -- e a ordem de liberação **não**
// é a de aquisição. É o padrão de uma tabela de registros que entra e sai, e é
// justamente o que a arena não atende.

#ifndef ACADEMY_MEMORY_POOL_SPEC_HPP
#define ACADEMY_MEMORY_POOL_SPEC_HPP

#include "docs/01-memoria/01-alocador-arena/spec.hpp"

#include <concepts>
#include <cstddef>

namespace academy::memory
{

// O `node` e o `arm_traits` vêm do tópico 01.01, e reusá-los é decisão:
// medir os dois tópicos sobre objetos de tamanhos diferentes tornaria os
// números incomparáveis, e a tabela consolidada de regras de decisão perderia o
// eixo que liga os três tópicos do módulo.

// A SPEC DO 01.02.
//
// Note o que mudou em relação à do 01.01: `release(node*)` entrou na interface
// exigida, e não mais em `if constexpr`. Um braço que não a ofereça não compila
// -- e é assim que a divergência entre tópicos vira erro de build em vez de
// virar nota de rodapé.
template <typename A>
concept churn_allocator = requires(A a, node *p) {
    { a.acquire() } -> std::same_as<node *>;
    { a.release(p) } -> std::same_as<void>;
    { a.outstanding() } -> std::same_as<std::size_t>;
    { A::traits } -> std::convertible_to<arm_traits>;
};

// AS CINCO INVARIANTES DESTE TÓPICO.
//
// As três primeiras são as do 01.01 que sobrevivem ao padrão novo. As duas
// últimas são o que a rotatividade exige e o lote não exigia.
enum class churn_invariant
{
    distinct_while_live,  // 1. dois objetos vivos nunca são o mesmo endereço
    usable_memory,        // 2. o que se escreve volta a ser lido
    aligned,              // 3. o ponteiro respeita alignof(node)
    release_then_reuse,   // 4. slot liberado volta a ser servido
    exhaustion_returns_null,  // 5. SÓ para bounded
};

} // namespace academy::memory

#endif // ACADEMY_MEMORY_POOL_SPEC_HPP
