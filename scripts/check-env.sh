#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Verifica o ambiente necessário para a trilha e informa o que falta.
#
# O QUE ESTE PROJETO EXIGE, E O QUE ELE NÃO EXIGE
#
# A norma (seção 16) declara **nenhuma dependência de sistema além do
# compilador, do Meson e do Ninja**. Isso é escolha de projeto, não sorte: o
# objeto de estudo é a linguagem, e qualquer máquina Linux x86-64 com um
# compilador de C++23 roda tudo. Não há biblioteca de sistema, não há driver,
# não há privilégio.
#
# Três coisas aparecem como `aviso`, e não como `FALTA`, porque a trilha roda
# sem elas -- mas cada aviso diz o que DEIXA de ser possível, e isso é
# deliberado: um aviso que não diz o que se perde é ruído que o leitor aprende a
# ignorar.
#
# Uso: ./scripts/check-env.sh
set -u

ok()    { printf '  ok     - %s\n' "$1"; }
falta() { printf '  FALTA  - %s\n' "$1"; faltas=$((faltas + 1)); }
aviso() { printf '  aviso  - %s\n' "$1"; avisos=$((avisos + 1)); }
info()  { printf '  info   - %s\n' "$1"; }
faltas=0
avisos=0
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"

echo "Build:"
for t in meson ninja; do
    if command -v "$t" >/dev/null 2>&1; then ok "$t ($("$t" --version 2>/dev/null | head -1))"
    else falta "$t (a norma exige Meson >= 1.1 e Ninja)"; fi
done
if command -v meson >/dev/null 2>&1; then
    mv=$(meson --version 2>/dev/null)
    # Comparação por ordenação de versão, e não por texto: "1.10" < "1.9" em
    # comparação lexicográfica, e este projeto roda em 1.10.
    if [ "$(printf '%s\n1.1\n' "$mv" | sort -V | head -1)" = "1.1" ]; then
        ok "Meson $mv atende ao minimo de 1.1"
    else
        falta "Meson $mv e anterior a 1.1"
    fi
fi

echo "Compiladores:"
achou_cxx=0
for t in g++ clang++; do
    if command -v "$t" >/dev/null 2>&1; then
        ok "$t ($("$t" --version 2>/dev/null | head -1))"
        achou_cxx=1
    else
        aviso "$t ausente -- a norma mede nos DOIS, e onde divergirem o documento registra"
    fi
done
[ "$achou_cxx" -eq 1 ] || falta "nenhum compilador de C++ encontrado"

echo "Suporte a C++23 (o que a norma usa, nao o que o compilador anuncia):"
# Sonda com os recursos que o projeto realmente usa. A versão do compilador não
# responde a pergunta: GCC 13 anuncia `-std=c++23` e não tem `<print>`.
sonda=$(mktemp -d) || { echo "mktemp falhou" >&2; exit 1; }
trap 'rm -rf "$sonda"' EXIT
cat > "$sonda/sonda.cpp" <<'CPP'
#include <expected>
#include <print>
#include <span>
#include <new>
#include <cstdint>

// `hardware_destructive_interference_size` e o modulo 02 inteiro (false
// sharing): sem ele, o padding vira numero magico escrito a mao.
static_assert(std::hardware_destructive_interference_size >= 32);

std::expected<int, const char*> f(bool b) {
    if (b) return 42;
    return std::unexpected("nao");
}

int main() {
    std::println("c++23 ok: {}", f(true).value());
}
CPP
for cc in g++ clang++; do
    command -v "$cc" >/dev/null 2>&1 || continue
    if "$cc" -std=c++23 "$sonda/sonda.cpp" -o "$sonda/sonda" 2>"$sonda/erro.txt"; then
        ok "$cc compila <expected>, <print>, <span> e hardware_destructive_interference_size"
    else
        falta "$cc nao compila os recursos de C++23 que a norma usa (veja: $("$cc" --version | head -1))"
        sed 's/^/           /' "$sonda/erro.txt" | head -5
    fi
done

echo "Ferramental de qualidade:"
if command -v python3 >/dev/null 2>&1; then
    ok "python3 ($(python3 --version 2>&1))"
else
    # Não é FALTA porque a suíte PULA os verificadores com código 77 em vez de
    # sumir com eles. Mas o que se perde é o portão da norma, e isso se diz.
    aviso "python3 ausente -- os 5 verificadores de documentacao serao PULADOS, e a norma deixa de ter portao"
fi
command -v git >/dev/null 2>&1 && ok "git ($(git --version))" \
    || aviso "git ausente -- o metadata da medicao nao registra o commit que a produziu"

echo "Medicao:"
# PMU: é o que decide se a seção "Hardware counters" da norma (seção 25) pode
# existir num módulo. Sem isto, o módulo de perf e cache misses não mede.
paranoid=$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo "")
if command -v perf >/dev/null 2>&1; then
    ok "perf ($(perf --version 2>&1 | head -1))"
else
    aviso "perf ausente -- o modulo de contadores de hardware nao roda (o resto roda)"
fi
# O 2 E SUFICIENTE PARA ESTE PROJETO, e dizer isso importa.
#
# O que a trilha mede sao os benchmarks DESTE repositorio, em espaco de usuario
# e no proprio processo -- que e exatamente o que o 2 permite. Escopo de CPU
# (`perf stat -a`) e perfilagem de kernel ficam de fora, e pela divisao de
# trabalho da secao 30 da norma eles sao assunto do DPDK Academy e do EX442,
# nao daqui.
#
# Por isso o 2 sai como `ok` e nao como aviso: um aviso que diz "funciona tudo
# o que voce precisa" e ruido, e ruido ensina o leitor a ignorar os avisos que
# importam.
case "${paranoid:-vazio}" in
    -1|0) ok "perf_event_paranoid=$paranoid: contadores de CPU e perfilagem de kernel disponiveis" ;;
    1)    ok "perf_event_paranoid=1: contador do proprio processo, inclusive modo kernel; sem escopo de CPU" ;;
    2)    ok "perf_event_paranoid=2: contador do proprio processo em espaco de usuario -- suficiente para a trilha" ;;
    vazio) aviso "perf_event_paranoid nao exposto por este kernel" ;;
    *)    aviso "perf_event_paranoid=$paranoid BLOQUEIA contador de hardware (o 3+ e extensao do Ubuntu). Para a trilha basta: sudo sysctl -w kernel.perf_event_paranoid=2" ;;
esac

# Governor e boost decidem se duas execuções são comparáveis -- e é a coleta de
# `ambiente.sh` que registra isso em cada medição publicada.
gov=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "")
case "$gov" in
    performance) ok "governor=performance" ;;
    "")          info "governor nao exposto" ;;
    *)           aviso "governor=$gov: a frequencia varia entre execucoes, e a cauda do percentil sente. O ambiente.sh registra isso em cada medicao" ;;
esac
[ "$(cat /sys/devices/system/cpu/smt/active 2>/dev/null || echo 0)" = "1" ] && \
    info "SMT ativo: o par de nucleos escolhido e variavel do experimento (norma, secao 28)"

echo "Topologia (o que a norma exige registrar, secao 29):"
n_l3=$(cat /sys/devices/system/cpu/cpu*/cache/index3/id 2>/dev/null | sort -u | wc -l)
if [ "${n_l3:-0}" -gt 1 ]; then
    ok "$n_l3 dominios de L3: ha par de nucleos no mesmo dominio e par atravessando"
elif [ "${n_l3:-0}" -eq 1 ]; then
    info "1 dominio de L3: os experimentos de CCD cruzado nao sao reproduziveis aqui"
else
    aviso "sysfs nao expoe id de L3 -- o ambiente nao registra CCD nesta maquina"
fi

echo
if [ "$faltas" -gt 0 ]; then
    printf 'Ambiente INCOMPLETO: %d item(ns) obrigatorio(s) faltando, %d aviso(s).\n' "$faltas" "$avisos"
    printf 'A trilha nao compila sem eles.\n'
    exit 1
fi
if [ "$avisos" -gt 0 ]; then
    printf 'Ambiente suficiente para compilar e rodar, com %d aviso(s) acima.\n' "$avisos"
    printf 'Cada aviso diz o que deixa de ser possivel; nenhum impede a trilha.\n'
else
    printf 'Ambiente completo.\n'
fi
printf 'Proximo passo: ./scripts/build-all.sh && ./scripts/test-all.sh\n'
printf 'Para o registro do ambiente: ./scripts/ambiente.sh\n'
exit 0
