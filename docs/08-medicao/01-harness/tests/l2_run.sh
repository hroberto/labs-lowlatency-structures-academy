#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# L2: roda o benchmark COMO O LEITOR O RODARIA.
#
# POR QUE L2 EXISTE SEPARADO DE L1
#
# L1 testa `tail.hpp` como biblioteca, e passaria mesmo se o programa do tópico
# não compilasse, não imprimisse tabela, ou saísse zero sobre coleta inválida.
# L2 executa o binário e confere o que o leitor vê: a tabela, o aviso de
# percentil sem sustentação, e o código de saída.
#
# O BRAÇO QUE IMPORTA É O DA CAUDA CURTA
#
# Com poucas amostras, o p99,9 continua sendo calculado -- e é isso que o torna
# perigoso: o número sai bonito. O programa TEM de avisar que ele não é
# publicável, e este teste exige o aviso. Sem esse braço, a guarda de percentil
# poderia deixar de existir sem que nada ficasse vermelho.
set -u

BIN=${1:?uso: l2_run.sh <caminho do binario>}
falhas=0
erro() { printf 'FALHA: %s\n' "$*" >&2; falhas=$((falhas + 1)); }
ok()   { printf '  ok: %s\n' "$*"; }

[ -x "$BIN" ] || { erro "binario nao executavel: $BIN"; exit 1; }

# Parâmetros baixos: aqui se verifica que o programa EXECUTA e o que ele diz,
# não se coleta estatística que ninguém vai ler. Medição publicável não define
# estas variáveis -- ver a campanha em scripts/arquivar-medicao.sh.
export HARNESS_SAMPLES=3
export HARNESS_ROUNDS=20000

# --- 1. cauda suficiente: sem aviso, saida zero ---------------------------
if saida=$(HARNESS_TAIL_SAMPLES=10000 "$BIN" 2>&1); then
    ok "o programa sai com zero quando a coleta e valida"
else
    erro "saiu com codigo diferente de zero:"; printf '%s\n' "$saida" | sed 's/^/    /' >&2
fi
for linha in "reading the clock" "recording a sample" "floor of the read pair"; do
    case "$saida" in
        *"$linha"*) ;;
        *) erro "a saida nao traz o braco '$linha'" ;;
    esac
done
ok "os tres bracos aparecem na tabela"

case "$saida" in
    *"NOT publishable"*) erro "avisou percentil sem sustentacao com 10 000 amostras" ;;
    *) ok "com 10 000 amostras nao ha aviso de p99,9 sem sustentacao" ;;
esac

# O programa tem de dizer o que os numeros AUTORIZAM, e nao so imprimi-los: e a
# secao "o que esta medicao nao mostra" da norma, dentro do proprio programa.
case "$saida" in
    *"license"*"BATCH"*) ok "a saida diz o que os numeros autorizam e aponta a medicao em lote" ;;
    *) erro "a saida nao diz o que os numeros autorizam" ;;
esac

# --- 2. braco de controle: cauda curta TEM de avisar ----------------------
if saida_curta=$(HARNESS_TAIL_SAMPLES=500 "$BIN" 2>&1); then
    case "$saida_curta" in
        *"NOT publishable"*)
            ok "com 500 amostras o programa avisa que o p99,9 nao e publicavel" ;;
        *)
            erro "com 500 amostras NAO houve aviso -- a guarda de percentil nao dispara"
            printf '%s\n' "$saida_curta" | sed 's/^/    /' >&2 ;;
    esac
else
    erro "o programa falhou com cauda de 500 amostras; deveria avisar, nao falhar"
fi

# --- 3. o portao de validade dispara nos DOIS modos ----------------------
#
# ESTE E O CASO QUE JUSTIFICA O ARQUIVO.
#
# A primeira versao do programa conferia a validade DEPOIS do `return` do modo
# `--csv`. O modo texto reprovava, e o `--csv` -- que e justamente o que a
# campanha arquiva -- saia zero. Uma campanha inteira foi versionada, com
# metadata e procedencia completa, descrevendo um braco que nao mediu.
#
# Entao o teste exige reprovacao nos dois modos, e nao em um.
for modo in "" "--csv"; do
    rotulo=${modo:-texto}
    if HARNESS_TAIL_SAMPLES=2000 "$BIN" --control-arm $modo >/dev/null 2>&1; then
        erro "no modo $rotulo, o braco de controle (mediana zero) saiu com ZERO -- o portao nao dispara"
    else
        ok "no modo $rotulo, o braco de controle e reprovado"
    fi
done

# E a mensagem tem de dizer a CAUSA: mediana zero aqui e laco removido pelo
# otimizador, nao operacao mais rapida que o relogio. Sem isso, quem le a falha
# procura no lugar errado.
msg=$(HARNESS_TAIL_SAMPLES=2000 "$BIN" --control-arm 2>&1 || true)
case "$msg" in
    *"removed by the optimizer"*) ok "a falha explica que mediana zero e laco removido" ;;
    *) erro "a mensagem de falha nao explica a causa da mediana zero" ;;
esac

# --- 4. modo CSV: e o que a campanha arquiva -----------------------------
if csv=$(HARNESS_TAIL_SAMPLES=2000 "$BIN" --csv 2>&1); then
    cabecalho=$(printf '%s\n' "$csv" | head -1)
    [ "$cabecalho" = "arm,metric,value,unit" ] \
        && ok "o CSV comeca com o cabecalho esperado" \
        || erro "cabecalho do CSV inesperado: '$cabecalho'"

    # Toda linha tem quatro campos. Um CSV com linha torta quebra o arquivamento
    # semanas depois, longe da causa.
    tortas=$(printf '%s\n' "$csv" | awk -F, 'NF != 4 {n++} END {print n+0}')
    [ "$tortas" -eq 0 ] && ok "todas as linhas do CSV tem quatro campos" \
        || erro "$tortas linha(s) do CSV com numero de campos diferente de 4"

    # Os campos que o metadata precisa citar: se um deles sair do CSV, a
    # medicao arquivada deixa de ser auditavel e nada mais acusa.
    for campo in per_op_floor,p50 per_op_floor,p999 per_op_floor,highest_supported \
                 per_op_floor,discarded parameters,samples parameters,clock_period; do
        case "$csv" in
            *"$campo"*) ;;
            *) erro "o CSV nao traz '$campo', que o metadata cita" ;;
        esac
    done
    ok "o CSV traz os campos que o metadata cita"
else
    erro "o modo --csv saiu com codigo diferente de zero"
fi

if [ "$falhas" -eq 0 ]; then
    printf 'l2 harness: todos os casos passaram\n'
    exit 0
fi
printf 'l2 harness: %d caso(s) falharam\n' "$falhas" >&2
exit 1
