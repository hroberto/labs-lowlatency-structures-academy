#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Campanha de medição: roda o experimento N vezes e arquiva o resultado.
#
# O QUE "ARQUIVAR" SIGNIFICA AQUI
#
# A seção 3 da norma declara que o histórico de campanha É VERSIONADO, e por
# quê: um p99,9 cuja campanha não está na árvore não é auditável depois. O ativo
# do projeto é a procedência do número.
#
# Uma campanha promovida a histórico traz, num diretório com a data no nome:
#
#   r0.csv ... rN.csv   a saída de CADA repetição, crua
#   ambiente.json       a coleta de ambiente, gerada
#   ambiente.md         a mesma coleta, em tabela para colar no documento
#   metadata.json       parâmetros, build, commit, variação entre execuções
#   tabela.md           a tabela publicável
#
# POR QUE RELEASE, E POR QUE ELE É EXIGIDO E NÃO ESCOLHIDO
#
# A seção 16 da norma diz que `release` é a configuração que se publica e se
# mede. Medir em `debug` e publicar produz número que descreve o build de
# desenvolvimento -- e a diferença não aparece em lugar nenhum da tabela. Então
# este script RECUSA outra configuração em vez de aceitar e anotar.
#
# Uso:
#   ./scripts/arquivar-medicao.sh <nome-da-campanha> [repeticoes]
set -u

RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
cd "$RAIZ"

CAMPANHA=${1:?uso: arquivar-medicao.sh <nome-da-campanha> [repeticoes]}
REPETICOES=${2:-3}
DIR_BUILD="build-release"
DESTINO="trilha/08-medicao/01-harness/bench/medicoes/historico/$(date +%Y-%m-%d)-$CAMPANHA"

command -v python3 >/dev/null 2>&1 || {
    echo "ERRO: arquivar medicao exige python3 (compoe o metadata e a tabela)" >&2
    exit 1
}

# REPETIÇÃO ÚNICA NÃO É CAMPANHA, e o script recusa em vez de aceitar em
# silêncio: a variação entre execuções é o campo que separa a cauda da estrutura
# da cauda da máquina, e com uma execução ela não existe.
if [ "$REPETICOES" -lt 2 ]; then
    echo "ERRO: $REPETICOES repeticao(oes). A variacao entre execucoes exige ao menos 2." >&2
    exit 1
fi

if [ ! -f "$DIR_BUILD/build.ninja" ]; then
    echo "ERRO: $DIR_BUILD nao existe. Rode: ./scripts/build-all.sh release" >&2
    exit 1
fi

# Confere na configuração REAL do Meson, e não no nome do diretório: um
# `build-release` configurado como debug passaria por qualquer verificação que
# olhasse só o nome.
CFG=$(python3 -c "
import json, sys
o = {x['name']: x['value'] for x in json.load(open(sys.argv[1]))}
print(o.get('buildtype'), o.get('b_ndebug'))" "$DIR_BUILD/meson-info/intro-buildoptions.json")
if [ "$CFG" != "release true" ]; then
    echo "ERRO: $DIR_BUILD esta como '$CFG', e a norma publica em 'release true'." >&2
    exit 1
fi

meson compile -C "$DIR_BUILD" >/dev/null || { echo "ERRO: a compilacao falhou" >&2; exit 1; }
BIN="$DIR_BUILD/trilha/08-medicao/01-harness/bench-harness"
[ -x "$BIN" ] || { echo "ERRO: binario nao encontrado: $BIN" >&2; exit 1; }

mkdir -p "$DESTINO"
printf 'Campanha: %s\n  destino: %s\n  repeticoes: %s\n\n' "$CAMPANHA" "$DESTINO" "$REPETICOES"

# Parâmetros da campanha: publicáveis, e portanto NÃO os tetos que a suíte usa.
# Ver o comentário do meson.build sobre HARNESS_SAMPLES na suíte -- lá o
# objetivo é verificar que o programa executa.
export HARNESS_SAMPLES=${HARNESS_SAMPLES:-25}
export HARNESS_ROUNDS=${HARNESS_ROUNDS:-200000}
export HARNESS_TAIL_SAMPLES=${HARNESS_TAIL_SAMPLES:-10000}

INICIO=$(date +%s)
for i in $(seq 0 $((REPETICOES - 1))); do
    printf '  execucao r%s ... ' "$i"
    if "$BIN" --csv > "$DESTINO/r$i.csv" 2>"$DESTINO/r$i.erro"; then
        printf 'ok (%s linhas)\n' "$(wc -l < "$DESTINO/r$i.csv")"
        rm -f "$DESTINO/r$i.erro"
    else
        printf 'FALHOU\n'
        cat "$DESTINO/r$i.erro" >&2
        echo "ERRO: a execucao r$i falhou; a campanha nao e arquivavel." >&2
        exit 1
    fi
done
FIM=$(date +%s)

./scripts/ambiente.sh --json > "$DESTINO/ambiente.json"
./scripts/ambiente.sh --markdown > "$DESTINO/ambiente.md"

MEDICAO_CONFIGURACAO="$DIR_BUILD ($CFG)" \
MEDICAO_COMPILADOR="$(python3 -c "
import json, sys
d = json.load(open(sys.argv[1]))['host']['cpp']
print(d['id'], d['version'])" "$DIR_BUILD/meson-info/intro-compilers.json")" \
MEDICAO_FLAGS="$(python3 -c "
import json, sys
o = {x['name']: x['value'] for x in json.load(open(sys.argv[1]))}
print('cpp_std=' + str(o.get('cpp_std')), 'warning_level=' + str(o.get('warning_level')),
      'buildtype=' + str(o.get('buildtype')), 'b_ndebug=' + str(o.get('b_ndebug')))" \
  "$DIR_BUILD/meson-info/intro-buildoptions.json")" \
MEDICAO_DURACAO="$((FIM - INICIO))" \
python3 ./scripts/compor-medicao.py "$DESTINO" "$DESTINO/ambiente.json" "$DESTINO"/r*.csv || exit 1

printf '\nA campanha esta arquivada e É VERSIONADA (norma, secao 3).\n'
printf 'Nenhum numero vai para o README sem apontar para este diretorio.\n'
