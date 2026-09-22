#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# A matriz de configurações da norma (seção 16), num lugar só.
#
# POR QUE ISTO É UMA BIBLIOTECA, E NÃO UMA LISTA EM CADA SCRIPT
#
# `build-all.sh` e `test-all.sh` precisam concordar sobre quais configurações
# existem. Se a lista estiver duplicada, o dia em que uma delas ganhar um perfil
# de sanitizer e a outra não, a suíte passa a reportar verde sobre um conjunto
# menor do que o que foi construído -- e o verde parece legítimo.
#
# O `-Db_ndebug` EXPLÍCITO nas três linhas não é redundância, e isto foi medido
# com Meson 1.10.1: `-Dbuildtype=release` SOZINHO deixa `b_ndebug=false` e não
# emite `-DNDEBUG`. Sem a flag, `release` e `release-checked` seriam a MESMA
# configuração, e a única que prova o contrato no código otimizado ficaria
# silenciosamente anulada.

# Cada entrada: nome|argumentos de setup|o que ela prova
CONFIGURACOES=(
  "debug|-Dbuildtype=debug -Db_ndebug=false|os contratos valem no codigo nao otimizado"
  "release|-Dbuildtype=release -Db_ndebug=true|e o que se publica e se mede"
  "release-checked|-Dbuildtype=release -Db_ndebug=false|os contratos valem no codigo OTIMIZADO"
)

# Clang entra como configuração própria, porque a norma mede nos dois
# compiladores e registra onde divergirem. Ela existe só se o clang++ existir:
# uma configuração que não pode ser construída não entra na lista, em vez de
# entrar e falhar.
if command -v clang++ >/dev/null 2>&1; then
  CONFIGURACOES+=("clang-release-checked|-Dbuildtype=release -Db_ndebug=false|o mesmo contrato no outro compilador")
fi

cfg_nome()  { printf '%s' "${1%%|*}"; }
cfg_args()  { local r="${1#*|}"; printf '%s' "${r%%|*}"; }
cfg_prova() { printf '%s' "${1##*|}"; }
cfg_cc()    { case "${1%%|*}" in clang-*) printf 'clang++' ;; *) printf '' ;; esac; }
