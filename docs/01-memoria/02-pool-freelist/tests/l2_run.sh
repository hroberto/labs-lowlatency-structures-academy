#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# L2 — a varredura de rotatividade como o leitor a executa.
set -u

BIN=${1:?uso: l2_run.sh <caminho do binario>}
falhas=0
erro() { printf 'FALHA: %s\n' "$*" >&2; falhas=$((falhas + 1)); }
ok()   { printf '  ok: %s\n' "$*"; }

[ -x "$BIN" ] || { erro "binario nao executavel: $BIN"; exit 1; }
export HARNESS_TAIL_SAMPLES=300

if saida=$("$BIN" 2>&1); then ok "o programa sai com zero"
else erro "saiu com codigo diferente de zero:"; printf '%s\n' "$saida" | sed 's/^/    /' >&2; fi

for vivos in 64 512 4096; do
    n=$(printf '%s\n' "$saida" | awk -v v="$vivos" '$2 == v {n++} END {print n+0}')
    [ "$n" -eq 2 ] || erro "conjunto vivo $vivos aparece em $n linha(s); esperadas 2"
done
ok "os dois bracos aparecem nos tres tamanhos de conjunto vivo"

# O aquecimento simetrico e declarado NA SAIDA, que e o que a campanha arquiva.
case "$saida" in
    *"aquecimento"*"iguais nos dois bracos"*) ok "a saida declara o aquecimento simetrico" ;;
    *) erro "a saida nao declara o aquecimento simetrico" ;;
esac

# Os passos por amostra sao o que pÕe a medicao acima do piso do instrumento
# (topico 08.01). Se esse numero sumir da saida, a campanha arquiva uma medicao
# cujo regime ninguem consegue reconstruir.
case "$saida" in
    *"passos por amostra"*) ok "a saida declara os passos por amostra" ;;
    *) erro "a saida nao declara os passos por amostra" ;;
esac

if csv=$("$BIN" --csv 2>&1); then
    [ "$(printf '%s\n' "$csv" | head -1)" = "arm,metric,value,unit" ] \
        && ok "o CSV comeca com o cabecalho esperado" || erro "cabecalho do CSV inesperado"
    tortas=$(printf '%s\n' "$csv" | awk -F, 'NF != 4 {n++} END {print n+0}')
    [ "$tortas" -eq 0 ] && ok "todas as linhas do CSV tem quatro campos" \
        || erro "$tortas linha(s) com numero de campos diferente de 4"
    for campo in "pool_V64,p50" "std::allocator_V64,p50" "pool_V4096,p99" \
                 "parameters,steps_per_sample" "parameters,samples"; do
        case "$csv" in *"$campo"*) ;; *) erro "o CSV nao traz '$campo'" ;; esac
    done
    ok "o CSV traz os campos que a tabela e o metadata citam"

    # p50 por passo zerado significa laco removido pelo otimizador, e nao
    # rotatividade gratuita.
    zerados=$(printf '%s\n' "$csv" | awk -F, '$2 == "p50_per_step" && $3 <= 0 {n++} END {print n+0}')
    [ "$zerados" -eq 0 ] && ok "nenhum p50 por passo zerado (laco nao foi removido)" \
        || erro "$zerados ponto(s) com p50 por passo <= 0"

    # O POOL TEM DE GANHAR, e isto e afirmacao do topico, nao torcida: se o
    # braco custom perder do baseline, ou a medicao esta errada ou a conclusao
    # publicada deixou de valer. Nos dois casos, vermelho.
    perdeu=$(printf '%s\n' "$csv" | awk -F, '
        /^pool_V[0-9]+,p50,/      {split($1,a,"_"); pool[a[2]] = $3}
        /^std::allocator_V[0-9]+,p50,/ {split($1,a,"_"); std[a[2]] = $3}
        END {for (v in pool) if (pool[v] >= std[v]) n++; print n+0}')
    [ "$perdeu" -eq 0 ] && ok "o pool e mais rapido que o baseline em todos os pontos" \
        || erro "$perdeu ponto(s) em que o pool NAO ganhou -- a conclusao publicada nao vale"
else
    erro "o modo --csv saiu com codigo diferente de zero"
fi

if [ "$falhas" -eq 0 ]; then printf 'l2 pool: todos os casos passaram\n'; exit 0; fi
printf 'l2 pool: %d caso(s) falharam\n' "$falhas" >&2
exit 1
