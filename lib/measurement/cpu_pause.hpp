// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Dica de espera ocupada, portátil entre x86-64 e arm64.
//
// ORIGEM
//
// Portado de `docs/01-fundamentos/medicoes/cpu_pause.h` do projeto irmão
// labs-dpdk-academy. Lá o cabeçalho nasceu de uma divergência entre promessa e
// código: o README declarava "Linux x86_64 ou arm64" e quatro programas de
// medição chamavam `__builtin_ia32_pause()` em doze pontos sem guarda de
// arquitetura -- em arm64 eles não compilam, porque o builtin é exclusivo de
// alvo x86 no GCC e no Clang.
//
// O mesmo requisito vale aqui: arm64 está no escopo declarado deste projeto,
// portanto nenhum programa de medição chama o builtin de x86 diretamente.
//
// O QUE CADA INSTRUÇÃO FAZ
//
//   x86-64  `pause`: avisa o processador de que este laço é espera, reduzindo o
//           consumo e a penalidade de saída do laço por especulação.
//   arm64   `isb`: barreira de sincronização de instruções. Não é equivalente
//           exato de `pause` -- arm64 tem `wfe`, que exige um evento para
//           acordar e não serve a uma espera por variável comum --, mas é a
//           escolha que o DPDK faz em `rte_pause()` para esta arquitetura.
//
// Em arquitetura desconhecida vira nada: o laço continua correto, só não recebe
// a dica. **Correção nunca depende desta chamada.**
//
// POR QUE FUNÇÃO E NÃO MACRO, AO CONTRÁRIO DO ORIGINAL
//
// O original é macro porque C não tem outra forma de emitir asm inline sem
// arriscar que o compilador não inline a função. Aqui uma função
// `[[gnu::always_inline]]` dá a mesma garantia com tipo, escopo de namespace e
// visibilidade para o depurador. A exceção documentada em .clang-tidy para
// macros vale para PERF_EXPECTS, que precisa do local de chamada -- esta não
// precisa, então não é macro.

#ifndef PERF_MEASUREMENT_CPU_PAUSE_HPP
#define PERF_MEASUREMENT_CPU_PAUSE_HPP

namespace perf::measurement
{

[[gnu::always_inline]] inline void cpu_pause() noexcept
{
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#elif defined(__aarch64__)
    __asm__ __volatile__("isb" ::: "memory");
#else
    // Arquitetura sem dica conhecida: a espera continua correta sem ela.
#endif
}

} // namespace perf::measurement

#endif // PERF_MEASUREMENT_CPU_PAUSE_HPP
