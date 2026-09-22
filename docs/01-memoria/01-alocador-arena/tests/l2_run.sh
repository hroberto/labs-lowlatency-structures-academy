#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# L2 — o benchmark como o leitor o executa.
#
# O BRAÇO QUE IMPORTA É O DA VARREDURA COMPLETA
#
# O L1 confere as invariantes da spec; ele passaria mesmo se o programa
# publicasse um único tamanho de lote. A norma (seção 11) exige varredura, e a
# regra de decisão do tópico repousa nela: sem os quatro pontos, "acima de 64"
# não tem de onde sair.
set -u

BIN=${1:?uso: l2_run.sh <caminho do binario>}
falhas=0
erro() { printf 'FALHA: %s\n' "$*" >&2; falhas=$((falhas + 1)); }
ok()   { printf '  ok: %s\n' "$*"; }

[ -x "$BIN" ] || { erro "binario nao executavel: $BIN"; exit 1; }

# Amostras baixas: aqui se verifica que o programa EXECUTA e o que ele diz.
export HARNESS_TAIL_SAMPLES=300

if saida=$("$BIN" 2>&1); then
    ok "o programa sai com zero"
else
    erro "saiu com codigo diferente de zero:"; printf '%s\n' "$saida" | sed 's/^/    /' >&2
fi

# --- 1. os dois bracos, nos quatro pontos da varredura --------------------
for lote in 8 64 512 4096; do
    n=$(printf '%s\n' "$saida" | awk -v l="$lote" '$2 == l {n++} END {print n+0}')
    [ "$n" -eq 2 ] || erro "lote $lote aparece em $n linha(s); esperadas 2 (um por braco)"
done
ok "os dois bracos aparecem nos quatro tamanhos de lote"

for braco in "std::allocator" "arena"; do
    case "$saida" in
        *"$braco"*) ;;
        *) erro "a saida nao traz o braco '$braco'" ;;
    esac
done
ok "os dois bracos estao nomeados na tabela"

# O aquecimento e declarado NA SAIDA, e nao so no fonte: a secao 11 da norma
# exige que ele apareca no documento, e a saida e o que a campanha arquiva.
case "$saida" in
    *"aquecimento"*"iguais nos dois bracos"*) ok "a saida declara o aquecimento simetrico" ;;
    *) erro "a saida nao declara o aquecimento simetrico" ;;
esac

# --- 2. braco de controle: a arena esgotada reprova -----------------------
#
# A pergunta de falha do modulo e o que acontece quando a arena esgota no meio
# de um lote. No benchmark isso e ERRO DE MEDICAO, e o programa tem de
# reprovar em vez de publicar lote incompleto. Sem este caso, a guarda poderia
# sumir do programa sem nada ficar vermelho.
#
# O caminho: pedir um lote maior que a capacidade da arena. Como a capacidade e
# o maior lote da varredura, isso se faz reduzindo a varredura -- e ela e
# constante de compilacao, de proposito. Entao o que este caso confere e a
# GUARDA: que o programa saiba reprovar, exercitada pelo L1 (invariante 6).
if grep -q 'FAILED' <<< "$saida"; then
    erro "a execucao normal reportou FAILED"
else
    ok "a execucao normal nao reporta falha de coleta"
fi

# --- 3. modo CSV: e o que a campanha arquiva ------------------------------
if csv=$("$BIN" --csv 2>&1); then
    [ "$(printf '%s\n' "$csv" | head -1)" = "arm,metric,value,unit" ] \
        && ok "o CSV comeca com o cabecalho esperado" \
        || erro "cabecalho do CSV inesperado"

    tortas=$(printf '%s\n' "$csv" | awk -F, 'NF != 4 {n++} END {print n+0}')
    [ "$tortas" -eq 0 ] && ok "todas as linhas do CSV tem quatro campos" \
        || erro "$tortas linha(s) do CSV com numero de campos diferente de 4"

    # Os campos que a tabela publicavel cita. Se um sair do CSV, o
    # compor-medicao.py falha -- mas falha depois, na campanha, longe daqui.
    for campo in "arena_L64,p50" "std::allocator_L64,p50" "arena_L512,p99" \
                 "parameters,samples" "parameters,warmup_batches" "parameters,node_bytes"; do
        case "$csv" in
            *"$campo"*) ;;
            *) erro "o CSV nao traz '$campo', que a tabela ou o metadata citam" ;;
        esac
    done
    ok "o CSV traz os campos que a tabela e o metadata citam"

    # p50 por no: e a grandeza que a analise usa, e ela nao pode sair negativa
    # nem zero -- zero aqui significaria lote sem custo, que e laco removido.
    zerados=$(printf '%s\n' "$csv" | awk -F, '$2 == "p50_per_node" && $3 <= 0 {n++} END {print n+0}')
    [ "$zerados" -eq 0 ] && ok "nenhum p50 por no zerado (laco nao foi removido)" \
        || erro "$zerados ponto(s) com p50 por no <= 0"
else
    erro "o modo --csv saiu com codigo diferente de zero"
fi

if [ "$falhas" -eq 0 ]; then
    printf 'l2 arena: todos os casos passaram\n'
    exit 0
fi
printf 'l2 arena: %d caso(s) falharam\n' "$falhas" >&2
exit 1
