#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# L2 — os três braços como o leitor os executa.
#
# O CASO QUE IMPORTA É A ORDEM DOS TRÊS
#
# A conclusão publicada deste tópico é que a `pmr` fica ENTRE o baseline e a
# arena artesanal. Se essa ordem se inverter -- por mudança de compilador, de
# biblioteca ou de máquina --, o documento passa a afirmar o contrário do que o
# programa mede. Este teste é onde isso fica vermelho.
set -u

BIN=${1:?uso: l2_run.sh <caminho do binario>}
falhas=0
erro() { printf 'FALHA: %s\n' "$*" >&2; falhas=$((falhas + 1)); }
ok()   { printf '  ok: %s\n' "$*"; }

[ -x "$BIN" ] || { erro "binario nao executavel: $BIN"; exit 1; }
export HARNESS_TAIL_SAMPLES=300

if saida=$("$BIN" 2>&1); then ok "o programa sai com zero"
else erro "saiu com codigo diferente de zero:"; printf '%s\n' "$saida" | sed 's/^/    /' >&2; fi

for lote in 8 64 512 4096; do
    n=$(printf '%s\n' "$saida" | awk -v l="$lote" '$2 == l {n++} END {print n+0}')
    [ "$n" -eq 3 ] || erro "lote $lote aparece em $n linha(s); esperadas 3 (um por braco)"
done
ok "os tres bracos aparecem nos quatro tamanhos de lote"

# A carga tem de ser declarada como a MESMA do 01.01: e o que torna os dois
# conjuntos de numeros comparaveis, e a comparacao entre eles e o topico.
case "$saida" in
    *"a mesma do topico 01.01"*) ok "a saida declara que a carga e a do 01.01" ;;
    *) erro "a saida nao declara a carga herdada do 01.01" ;;
esac

if csv=$("$BIN" --csv 2>&1); then
    [ "$(printf '%s\n' "$csv" | head -1)" = "arm,metric,value,unit" ] \
        && ok "o CSV comeca com o cabecalho esperado" || erro "cabecalho do CSV inesperado"

    for campo in "std::pmr_L512,p50" "arena_L512,p50" "std::allocator_L512,p50"; do
        case "$csv" in *"$campo"*) ;; *) erro "o CSV nao traz '$campo'" ;; esac
    done
    ok "o CSV traz os tres bracos no ponto que a tabela publica"

    # A ORDEM, que e a conclusao: arena <= pmr < std::allocator, a partir do
    # lote 64. O lote 8 fica de fora porque os tres estao no piso do
    # instrumento, e comparar ali seria comparar o relogio.
    fora=$(printf '%s\n' "$csv" | awk -F, '
        /^(arena|std::pmr|std::allocator)_L[0-9]+,p50,/ {
            # `+ 0` FORCA NUMERO. Sem ele o mawk compara como TEXTO depois de
            # o valor passar por um array, e "591.0" > "14758.0" porque '5' > '1'.
            # O teste acusava a ordem invertida com os numeros certos na mao.
            split($1, a, "_L"); v[a[1] "," a[2]] = $3 + 0
        }
        END {
            split("64 512 4096", lotes, " ")
            for (i in lotes) {
                l = lotes[i]
                if (!(("arena," l) in v)) continue
                if (v["arena," l] > v["std::pmr," l]) { print "arena perdeu da pmr no lote " l; n++ }
                if (v["std::pmr," l] >= v["std::allocator," l]) { print "pmr nao ganhou do baseline no lote " l; n++ }
            }
            exit (n > 0)
        }')
    if [ -z "$fora" ]; then
        ok "a ordem publicada se sustenta: arena <= pmr < std::allocator"
    else
        erro "a ordem publicada NAO se sustenta:"; printf '%s\n' "$fora" | sed 's/^/    /' >&2
    fi
else
    erro "o modo --csv saiu com codigo diferente de zero"
fi

if [ "$falhas" -eq 0 ]; then printf 'l2 pmr: todos os casos passaram\n'; exit 0; fi
printf 'l2 pmr: %d caso(s) falharam\n' "$falhas" >&2
exit 1
