#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Roda a suíte em cada configuração da norma e resume o resultado.
#
# O RESUMO DIZ QUANTOS FORAM PULADOS, E ISSO É O PONTO
#
# Em `release`, a violação da porta R devolve 77 e o Meson reporta PULADO --
# violar `[[assume]]` é comportamento indefinido, não abort. Um resumo que só
# dissesse "passou" esconderia que um caso não foi exercido, e "não verificado"
# reportado como "verificado" é o defeito que este projeto mais persegue.
#
# Uso:
#   ./scripts/test-all.sh               # todas as configuracoes
#   ./scripts/test-all.sh debug         # so uma
#   ./scripts/test-all.sh -- --suite l1+docs   # argumentos para o meson test
set -u

RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
. "$RAIZ/scripts/lib-configuracoes.sh"
cd "$RAIZ"

alvo="todas"
if [ $# -gt 0 ] && [ "$1" != "--" ]; then alvo="$1"; shift; fi
[ "${1:-}" = "--" ] && shift

falhou=0
resumo=""
# A quebra de linha vai por `$'\n'` explicito: substituicao de comando remove o
# \n final, e a primeira versao imprimia o resumo das quatro configuracoes numa
# unica linha corrida.
NL=$'\n'

for cfg in "${CONFIGURACOES[@]}"; do
    nome=$(cfg_nome "$cfg")
    [ "$alvo" = "todas" ] || [ "$alvo" = "$nome" ] || continue
    dir="build-$nome"

    if [ ! -f "$dir/build.ninja" ]; then
        # Configuração não construída é PULADA com aviso, nunca omitida do
        # resumo: uma configuração ausente do relatório é indistinguível de uma
        # que passou.
        resumo="$resumo$(printf '  %-24s NAO CONSTRUIDA -- rode ./scripts/build-all.sh %s' "$nome" "$nome")$NL"
        falhou=1
        continue
    fi

    printf '\n=== %s ===\n' "$dir"
    saida=$(meson test -C "$dir" "$@" 2>&1)
    codigo=$?
    printf '%s\n' "$saida" | grep -E '^\s*[0-9]+/[0-9]+' || true

    linha=$(printf '%s\n' "$saida" | awk '
        /^Ok:/            {ok = $2}
        /^Expected Fail:/ {ef = $3}
        /^Fail:/          {f = $2}
        /^Unexpected Pass:/ {up = $3}
        /^Skipped:/       {sk = $2}
        /^Timeout:/       {to = $2}
        END {printf "%s ok, %s falha esperada, %s falha, %s pulado", ok+0, ef+0, f+0, sk+0
             if (up+0 > 0) printf ", %s PASSOU SEM DEVER", up+0
             if (to+0 > 0) printf ", %s TIMEOUT", to+0}')
    if [ "$codigo" -eq 0 ]; then
        resumo="$resumo$(printf '  %-24s OK      %s' "$nome" "$linha")$NL"
    else
        resumo="$resumo$(printf '  %-24s FALHOU  %s' "$nome" "$linha")$NL"
        printf '%s\n' "$saida" | sed -n '/^Summary of Failures/,$p'
        falhou=1
    fi
done

printf '\n== Resumo ==\n%s' "$resumo"
if [ -z "$resumo" ]; then
    printf '  nenhuma configuracao com o nome "%s"\n' "$alvo" >&2
    exit 1
fi
[ "$falhou" -eq 0 ] && printf 'Todas as configuracoes passaram.\n' && exit 0
printf 'Ha configuracao com falha acima.\n' >&2
exit 1
