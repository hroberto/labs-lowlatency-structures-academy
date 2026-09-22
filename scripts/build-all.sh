#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Constrói a matriz de configurações da norma (seção 16).
#
# Uso:
#   ./scripts/build-all.sh              # todas as configuracoes
#   ./scripts/build-all.sh debug        # so uma
set -eu

RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
. "$RAIZ/scripts/lib-configuracoes.sh"
cd "$RAIZ"

alvo=${1:-todas}
construidas=0

for cfg in "${CONFIGURACOES[@]}"; do
    nome=$(cfg_nome "$cfg")
    [ "$alvo" = "todas" ] || [ "$alvo" = "$nome" ] || continue

    dir="build-$nome"
    cc=$(cfg_cc "$cfg")
    printf '\n=== %s (%s) ===\n' "$dir" "$(cfg_prova "$cfg")"

    if [ -f "$dir/build.ninja" ]; then
        # `--wipe` e não `--reconfigure`: trocar de compilador exige árvore
        # nova, e `--reconfigure` mantém o compilador da primeira vez sem
        # dizer nada.
        printf -- '-- ja configurado, reconstruindo\n'
    else
        # shellcheck disable=SC2086
        env ${cc:+CXX=$cc} meson setup "$dir" $(cfg_args "$cfg")
    fi
    meson compile -C "$dir"
    construidas=$((construidas + 1))
done

if [ "$construidas" -eq 0 ]; then
    printf 'Nenhuma configuracao com o nome "%s". Disponiveis:\n' "$alvo" >&2
    for cfg in "${CONFIGURACOES[@]}"; do printf '  %s\n' "$(cfg_nome "$cfg")" >&2; done
    exit 1
fi
printf '\n%d configuracao(oes) construida(s). Agora: ./scripts/test-all.sh\n' "$construidas"
