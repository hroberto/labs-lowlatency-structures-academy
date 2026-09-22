#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Autoteste de scripts/check-env.sh e da matriz de configurações.
#
# O CASO QUE IMPORTA É O BRAÇO DE CONTROLE
#
# Um diagnóstico de ambiente que diz "ok" numa máquina boa não prova nada: ele
# diria "ok" também se tivesse parado de verificar. Então o teste ESCONDE o
# Meson do `PATH` e exige que o script acuse a falta e saia com código 1. Sem
# esse braço, um `check-env.sh` que passasse a imprimir "ok" incondicionalmente
# continuaria verde na suíte.
#
# O segundo caso guarda a razão de `lib-configuracoes.sh` existir: se
# `build-all.sh` e `test-all.sh` voltarem a ter a lista de configurações
# duplicada, elas divergem e a suíte passa a reportar verde sobre um conjunto
# menor do que o construído.
set -u

RAIZ="$(cd "$(dirname "$0")/../.." && pwd)"
CHECK="$RAIZ/scripts/check-env.sh"
falhas=0
erro() { printf 'FALHA: %s\n' "$*" >&2; falhas=$((falhas + 1)); }
ok()   { printf '  ok: %s\n' "$*"; }

[ -x "$CHECK" ] || { erro "check-env.sh nao e executavel"; exit 1; }

# --- 1. caminho felizinho -------------------------------------------------
if saida=$("$CHECK" 2>&1); then
    ok "check-env.sh sai com zero neste ambiente"
else
    erro "check-env.sh saiu com codigo diferente de zero aqui:"
    printf '%s\n' "$saida" | sed 's/^/    /' >&2
fi
for secao in "Build:" "Compiladores:" "Suporte a C++23" "Ferramental de qualidade:" "Medicao:" "Topologia"; do
    case "$saida" in
        *"$secao"*) ;;
        *) erro "check-env.sh nao imprime a secao '$secao'" ;;
    esac
done
ok "as seis secoes do diagnostico estao presentes"

# O diagnóstico tem de dizer o que fazer depois. Um relatório que termina sem
# próximo passo faz o leitor adivinhar.
case "$saida" in
    *"build-all.sh"*) ok "o diagnostico aponta o proximo passo" ;;
    *) erro "check-env.sh nao aponta o proximo passo" ;;
esac

# --- 2. braço de controle: sem meson no PATH ------------------------------
# `PATH` reduzido a um diretório vazio mais o mínimo: o script tem de ACUSAR.
jaula=$(mktemp -d) || { erro "mktemp falhou"; exit 1; }
trap 'rm -rf "$jaula"' EXIT
# O que o próprio teste precisa continua disponível; o que se esconde é o meson.
for bin in bash sh env printf cat awk sed grep sort head tr mktemp rm command uname nproc dirname cut basename g++ clang++ python3 git cc; do
    caminho=$(command -v "$bin" 2>/dev/null) && ln -sf "$caminho" "$jaula/$bin"
done
if saida_jaula=$(PATH="$jaula" "$CHECK" 2>&1); then
    erro "com meson ausente, check-env.sh saiu com ZERO -- a deteccao nao dispara"
    printf '%s\n' "$saida_jaula" | sed 's/^/    /' >&2
else
    case "$saida_jaula" in
        *"FALTA"*meson*|*meson*"FALTA"*|*"FALTA  - meson"*)
            ok "com meson ausente, o script acusa FALTA e sai com codigo 1" ;;
        *)
            # Saiu com 1, mas sem nomear o que falta: melhor que verde, e ainda
            # assim insuficiente para quem lê.
            case "$saida_jaula" in
                *"INCOMPLETO"*) ok "com meson ausente, o script sai com 1 e declara ambiente INCOMPLETO" ;;
                *) erro "saiu com 1 mas sem dizer o que falta:"; printf '%s\n' "$saida_jaula" | sed 's/^/    /' >&2 ;;
            esac ;;
    esac
fi

# --- 3. a matriz de configuracoes vive num lugar so -----------------------
LIB="$RAIZ/scripts/lib-configuracoes.sh"
[ -r "$LIB" ] || erro "lib-configuracoes.sh nao existe"
if [ -r "$LIB" ]; then
    # shellcheck source=../lib-configuracoes.sh
    . "$LIB"
    n=${#CONFIGURACOES[@]}
    [ "$n" -ge 3 ] && ok "a matriz tem $n configuracao(oes)" \
        || erro "a matriz tem $n configuracao(oes); a norma exige as tres da secao 16"
    for obrigatoria in debug release release-checked; do
        achou=0
        for cfg in "${CONFIGURACOES[@]}"; do
            [ "${cfg%%|*}" = "$obrigatoria" ] && achou=1
        done
        [ "$achou" -eq 1 ] || erro "a configuracao obrigatoria '$obrigatoria' nao esta na matriz"
    done
    ok "as tres configuracoes obrigatorias estao na matriz"

    # `release` tem de pedir b_ndebug=true EXPLICITAMENTE: medido com Meson
    # 1.10.1, `-Dbuildtype=release` sozinho deixa b_ndebug=false, e release e
    # release-checked passariam a ser a mesma configuracao.
    for cfg in "${CONFIGURACOES[@]}"; do
        if [ "${cfg%%|*}" = "release" ]; then
            case "$cfg" in
                *"-Db_ndebug=true"*) ok "release pede b_ndebug=true explicitamente" ;;
                *) erro "release sem -Db_ndebug=true: seria identica a release-checked" ;;
            esac
        fi
    done
fi

# Nenhum dos dois scripts pode ter a lista propria: e a razao de a lib existir.
for s in build-all.sh test-all.sh; do
    if grep -q 'lib-configuracoes.sh' "$RAIZ/scripts/$s"; then
        if grep -qE '^\s*CONFIGURACOES=\(' "$RAIZ/scripts/$s"; then
            erro "$s define a propria lista de configuracoes, duplicando a lib"
        fi
    else
        erro "$s nao carrega lib-configuracoes.sh"
    fi
done
ok "build-all.sh e test-all.sh compartilham a mesma matriz"

if [ "$falhas" -eq 0 ]; then
    printf 'check-env.sh: todos os casos passaram\n'
    exit 0
fi
printf 'check-env.sh: %d caso(s) falharam\n' "$falhas" >&2
exit 1
