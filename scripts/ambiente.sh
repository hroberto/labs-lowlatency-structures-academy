#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Registro do ambiente de medição.
#
# POR QUE ESTE SCRIPT EXISTE
#
# A norma (docs/padrao-do-projeto.md, seção 29) proíbe descrever o ambiente em
# prosa, e a razão é a experiência do projeto a montante: a descrição diverge
# entre arquivos sem que ninguém perceba, envelhece em silêncio quando o kernel
# ou o compilador mudam, e não diz a quem reproduz o que comparar. Então o
# ambiente não é escrito: é gerado.
#
# ATENÇÃO AO PARSING -- ISTO NÃO É PARA SER REESCRITO
#
# Nada aqui depende do TEXTO do `lscpu`. A saída dele é traduzida, e nesta
# máquina sai em português: `lscpu` imprime "Nome do modelo", não "Model name".
# Um parsing por rótulo em inglês devolve campo vazio EM SILÊNCIO, que é o pior
# modo de falha possível num script cuja razão de existir é reprodutibilidade.
# As formas usadas abaixo -- `lscpu -p=` e o sysfs -- são independentes de
# idioma. A lição é herdada do DPDK Academy, onde o defeito aconteceu.
#
# O QUE ESTE PROJETO ACRESCENTOU AO SCRIPT DE ORIGEM
#
#   1. CCD por núcleo e par SMT. A máquina de referência tem dois domínios de
#      L3, e um par de núcleos no mesmo CCD mede coisa diferente de um par em
#      CCDs distintos -- para a MESMA estrutura. "Núcleo 3" não identifica o
#      experimento (norma, seção 28).
#   2. disponibilidade do PMU: `perf_event_paranoid` decide se o módulo de
#      medição pode ler contador, e a norma passou a exigir o registro.
#   3. invariância do TSC, que é premissa do relógio de `lib/measurement/`.
#   4. quais configurações de build existem na árvore, porque a norma exige
#      saber qual das três produziu o número (seção 28).
#   5. saída `--json`, para o `metadata.json` que acompanha cada medição.
#
# Uso:
#   ./scripts/ambiente.sh                    # texto legível
#   ./scripts/ambiente.sh --markdown         # tabela para colar em documento
#   ./scripts/ambiente.sh --json             # para o metadata.json da medição
#   sudo ./scripts/ambiente.sh --cachear-memoria
set -u

MODO=${1:-texto}
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"

v() { # v <comando...> -> primeira linha da saída, ou "-"
    local saida
    saida=$("$@" 2>/dev/null | head -1)
    printf '%s' "${saida:--}"
}

# --- coleta: processador --------------------------------------------------
CPU_MODELO=$(awk -F': ' '/^model name/{print $2; exit}' /proc/cpuinfo)
[ -z "$CPU_MODELO" ] && CPU_MODELO="-"
CPU_LOGICAS=$(nproc)
lscpu_col() { lscpu -p="$1" 2>/dev/null | grep -v '^#' | sort -u | wc -l; }
CPU_FISICOS=$(lscpu -p=CORE,SOCKET 2>/dev/null | grep -v '^#' | sort -u | wc -l)
CPU_SOQUETES=$(lscpu_col SOCKET)
NUMA_NOS=$(lscpu_col NODE)
CPU_SMT=$([ "${CPU_FISICOS:-0}" -gt 0 ] && echo $((CPU_LOGICAS / CPU_FISICOS)) || echo "-")
CPU_SMT_ATIVO=$(cat /sys/devices/system/cpu/smt/active 2>/dev/null || echo "-")

# L3 em Ryzen tem uma instância por CCD: as duas informações importam.
CPU_L3_TAM=$(cat /sys/devices/system/cpu/cpu0/cache/index3/size 2>/dev/null || echo "-")
CPU_L3_N=$(cat /sys/devices/system/cpu/cpu*/cache/index3/id 2>/dev/null | sort -u | wc -l)
CPU_L3="$CPU_L3_TAM x $CPU_L3_N instancia(s)"
CPU_DOMINIOS=$(cat /sys/devices/system/cpu/cpu*/cache/index3/shared_cpu_list 2>/dev/null | sort -u | paste -sd' ' -)
[ -z "$CPU_DOMINIOS" ] && CPU_DOMINIOS="-"
CPU_TSC=$(grep -o 'constant_tsc\|nonstop_tsc\|tsc_known_freq\|tsc_reliable' /proc/cpuinfo | sort -u | paste -sd' ' -)
[ -z "$CPU_TSC" ] && CPU_TSC="(nenhum sinalizador de TSC exposto)"
# Invariância é o que o relógio da régua de apuração pressupõe. Declarada, não
# suposta: a pergunta "o que acontece quando o TSC não é invariante" não é
# reproduzível numa máquina onde ele é (norma, seção 29).
case "$CPU_TSC" in
    *constant_tsc*nonstop_tsc*|*nonstop_tsc*constant_tsc*) TSC_INVARIANTE="sim (constant_tsc e nonstop_tsc)" ;;
    *) TSC_INVARIANTE="NAO -- rdtsc calibrado nao vale nesta maquina" ;;
esac

# --- coleta: topologia por núcleo lógico ---------------------------------
# UMA LINHA POR CPU LÓGICA, com o CCD (domínio de L3) e o par SMT. É esta
# tabela que permite dizer, depois, que o benchmark rodou "no CCD 0" ou
# "atravessando CCDs" -- e o par de núcleos é variável do experimento.
topologia() { # -> "cpu<TAB>core<TAB>l3<TAB>siblings"
    local c n
    for c in /sys/devices/system/cpu/cpu[0-9]*; do
        n=${c##*/cpu}
        printf '%s\t%s\t%s\t%s\n' \
            "$n" \
            "$(cat "$c/topology/core_id" 2>/dev/null || echo -)" \
            "$(cat "$c/cache/index3/id" 2>/dev/null || echo -)" \
            "$(cat "$c/topology/thread_siblings_list" 2>/dev/null || echo -)"
    done | sort -n -k1
}
TOPOLOGIA=$(topologia)
# Resumo: quais CPUs lógicas em cada domínio de L3, para caber numa tabela.
#
# Sem `asorti`: ela é do gawk, e o awk desta distribuição é o mawk 1.3.4, onde
# a chamada falha. A primeira versão tinha um fallback -- e o fallback rodou,
# em silêncio, produzindo "0=0 0=1 0=2 ..." em vez de "L3#0=[0-5,12-17]". Um
# resumo errado que parece um resumo é pior do que resumo nenhum, então o
# agrupamento agora é feito por `sort`, que existe em qualquer lugar.
#
# O agrupamento é FUNÇÃO, e tem modo próprio (`--agrupar-l3`), porque o
# autoteste precisa exercitá-lo com entrada sintética de dois domínios. Numa
# máquina de um único CCD, um teste que só olhasse a saída real passaria com o
# bug da armadilha 2 presente -- verificação que nunca dispara.
agrupar_l3() { # stdin: "l3_id<TAB>cpu" -> "L3#0=[...] L3#1=[...]"
    sort -t"$(printf '\t')" -k1,1n -k2,2n \
    | awk -F'\t' '
        # DUAS ARMADILHAS AQUI, as duas ja tendo custado o primeiro CCD da saida.
        #
        # 1. atribuicao dentro do printf: passar `sep = " "` como quarto
        #    argumento de um printf de tres especificadores fazia o grupo
        #    anterior sumir no mawk. Truque de uma linha que perde um dado nao
        #    economiza nada.
        # 2. `atual` nao inicializada compara NUMERICAMENTE com $1. Em awk uma
        #    variavel nao inicializada vale "" e 0 ao mesmo tempo, e $1 vindo da
        #    entrada como "0" e strnum: `$1 != atual` dava 0 != 0, FALSO, e o
        #    dominio L3#0 -- doze CPUs -- nunca abria grupo. A saida ficava
        #    plausivel: um unico CCD, com o numero certo de CPUs dentro.
        #
        # O sentinela `iniciado` resolve as duas: nenhuma comparacao depende de
        # valor nao inicializado.
        function despejar() { if (iniciado) { printf "%sL3#%s=[%s]", sep, atual, lista; sep = " " } }
        !iniciado             { atual = $1; lista = $2; iniciado = 1; next }
        $1 != atual           { despejar(); atual = $1; lista = $2; next }
                              { lista = lista "," $2 }
        END                   { despejar() }'
}

if [ "$MODO" = "--agrupar-l3" ]; then agrupar_l3; exit 0; fi

CCD_RESUMO=$(printf '%s\n' "$TOPOLOGIA" | awk -F'\t' '{print $3"\t"$1}' | agrupar_l3)
[ -z "$CCD_RESUMO" ] && CCD_RESUMO="(topologia de L3 nao exposta pelo sysfs)"

# --- coleta: frequência --------------------------------------------------
GOVERNOR=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "-")
TURBO_INTEL=$(cat /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null || echo "-")
TURBO_AMD=$(cat /sys/devices/system/cpu/cpufreq/boost 2>/dev/null || echo "-")

# --- coleta: PMU ---------------------------------------------------------
# `perf_event_paranoid` decide o que o módulo de medição consegue ler:
#   -1 tudo    0 acesso a CPU sem rastreamento    1 sem acesso a nível de CPU
#    2 só o próprio processo, sem contador de kernel    3+ nada
PMU_PARANOID=$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo "-")
case "$PMU_PARANOID" in
    -1|0) PMU_ESTADO="contadores de CPU disponiveis" ;;
    1)    PMU_ESTADO="disponivel por processo; sem escopo de CPU" ;;
    2)    PMU_ESTADO="so o proprio processo, sem eventos de kernel" ;;
    -)    PMU_ESTADO="nao exposto" ;;
    *)    PMU_ESTADO="BLOQUEADO -- contador de hardware indisponivel" ;;
esac
PERF_BIN=$(v perf --version)

# --- coleta: memória e sistema -------------------------------------------
MEM_TOTAL=$(awk '/^MemTotal:/{printf "%.1f GiB", $2/1048576}' /proc/meminfo)
MEM_DISPO=$(awk '/^MemAvailable:/{printf "%.1f GiB", $2/1048576}' /proc/meminfo)
HUGE_TOTAL=$(awk '/^HugePages_Total:/{print $2}' /proc/meminfo)
HUGE_LIVRES=$(awk '/^HugePages_Free:/{print $2}' /proc/meminfo)
HUGE_TAM=$(awk '/^Hugepagesize:/{print $2" "$3}' /proc/meminfo)
KERNEL=$(uname -r)
ARQ=$(uname -m)
DISTRO=$(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME")
[ -z "$DISTRO" ] && DISTRO="-"
CMDLINE=$(tr ' ' '\n' < /proc/cmdline 2>/dev/null | grep -E 'isolcpus|nohz|mitigations|hugepages|processor\.max_cstate|intel_idle|amd_pstate' | tr '\n' ' ')
[ -z "$CMDLINE" ] && CMDLINE="(nenhum parametro relevante)"

# Mitigações mudam o custo de syscall em ordem de grandeza. O formato abaixo
# corta no PRIMEIRO ':' e não trunca: no script de origem, truncar descartava o
# fim da lista em ordem alfabética -- spectre_*, srso --, que são as caras.
mitig_pares() { grep -H . /sys/devices/system/cpu/vulnerabilities/* 2>/dev/null | sed 's|.*/vulnerabilities/||; s|:|\t|'; }
MITIG_ATIVAS=$(mitig_pares | awk -F'\t' '$2 != "Not affected" {printf "%s%s=%s", sep, $1, $2; sep="; "}')
MITIG_INATIVAS=$(mitig_pares | awk -F'\t' '$2 == "Not affected" {printf "%s%s", sep, $1; sep=", "}')
[ -z "$MITIG_ATIVAS" ] && [ -z "$MITIG_INATIVAS" ] && MITIG_ATIVAS="(nao expostas)"
[ -z "$MITIG_ATIVAS" ] && MITIG_ATIVAS="(nenhuma ativa)"
[ -z "$MITIG_INATIVAS" ] && MITIG_INATIVAS="(nenhuma)"

PLACA="$(v cat /sys/class/dmi/id/board_vendor) $(v cat /sys/class/dmi/id/board_name)"
BIOS="$(v cat /sys/class/dmi/id/bios_version) de $(v cat /sys/class/dmi/id/bios_date)"

# --- velocidade da memória: root, cache, ou diz que não leu ---------------
#
# Herdado por causa de um defeito caro no projeto a montante: a memória foi
# trocada, a latência caiu 12%, e todos os números publicados até então
# descreviam outra configuração de hardware -- porque o script "gerado" omitia
# justamente a variável que mais move os resultados. Quando não há privilégio,
# o campo DIZ que não foi lido: um "-" discreto seria indistinguível de "não há
# o que reportar".
if [ "$MODO" = "--cachear-memoria" ]; then
    D=$(dmidecode -t memory 2>/dev/null) || { echo "precisa de root: sudo $0 --cachear-memoria" >&2; exit 1; }
    V=$(printf '%s' "$D" | awk -F': ' '/Configured Memory Speed/ && $2 !~ /Unknown/ {print $2; exit}')
    TP=$(printf '%s' "$D" | awk -F': ' '/^[[:space:]]*Type:/ && $2 !~ /Unknown|Other/ {print $2; exit}')
    PE=$(printf '%s' "$D" | grep -c "^[[:space:]]*Size: [0-9]")
    SL=$(printf '%s' "$D" | grep -c "^[[:space:]]*Size:")
    { echo "CACHE_DATA='$(date +%Y-%m-%d)'"
      echo "CACHE_VEL='${V:-nao reportado}${TP:+ ($TP)}'"
      echo "CACHE_CANAIS='$PE pente(s) em $SL slot(s)'"
    } > "$RAIZ/.ambiente-memoria"
    chmod 644 "$RAIZ/.ambiente-memoria"
    [ -n "${SUDO_USER:-}" ] && chown "$SUDO_USER" "$RAIZ/.ambiente-memoria" 2>/dev/null
    echo "gravado em $RAIZ/.ambiente-memoria:"; cat "$RAIZ/.ambiente-memoria"
    exit 0
fi

if MEM_DMI=$(sudo -n dmidecode -t memory 2>/dev/null); then
    MEM_VEL=$(printf '%s' "$MEM_DMI" | awk -F': ' '/Configured Memory Speed/ && $2 !~ /Unknown/ {print $2; exit}')
    MEM_TIPO=$(printf '%s' "$MEM_DMI" | awk -F': ' '/^[[:space:]]*Type:/ && $2 !~ /Unknown|Other/ {print $2; exit}')
    MEM_VEL="${MEM_VEL:-nao reportado pelo firmware}${MEM_TIPO:+ ($MEM_TIPO)}"
    MEM_PENTES=$(printf '%s' "$MEM_DMI" | grep -c "^[[:space:]]*Size: [0-9]")
    MEM_SLOTS=$(printf '%s' "$MEM_DMI" | grep -c "^[[:space:]]*Size:")
    MEM_CANAIS="$MEM_PENTES pente(s) em $MEM_SLOTS slot(s)"
elif [ -r "$RAIZ/.ambiente-memoria" ]; then
    # O cache SEMPRE se apresenta com a data em que foi feito: quem lê decide se
    # ela ainda vale. Um cache mudo seria pior que o campo ausente.
    . "$RAIZ/.ambiente-memoria"
    MEM_VEL="$CACHE_VEL (lido em $CACHE_DATA)"
    MEM_CANAIS="$CACHE_CANAIS (lido em $CACHE_DATA)"
else
    MEM_VEL="NAO LIDO -- rode: sudo ./scripts/ambiente.sh --cachear-memoria"
    MEM_CANAIS="NAO LIDO -- idem"
fi

# --- ferramental ---------------------------------------------------------
GCC=$(v gcc --version)
CLANG=$(v clang++ --version)
MESON=$(v meson --version)
NINJA=$(v ninja --version)
GIT_COMMIT=$(cd "$RAIZ" && git rev-parse --short HEAD 2>/dev/null || echo "-")
GIT_SUJO=$(cd "$RAIZ" && [ -n "$(git status --porcelain 2>/dev/null)" ] && echo "sim" || echo "nao")

# Quais das três configurações da norma (seção 16) existem na árvore. A
# medição diz qual produziu o número; aqui se registra o que está configurado.
configs() {
    local d nome bt nd
    for d in "$RAIZ"/build*/; do
        [ -f "$d/meson-info/intro-buildoptions.json" ] || continue
        nome=$(basename "$d")
        bt=$(python3 -c "
import json,sys
o={x['name']:x['value'] for x in json.load(open(sys.argv[1]))}
print(o.get('buildtype','?'), 'b_ndebug='+str(o.get('b_ndebug','?')))" "$d/meson-info/intro-buildoptions.json" 2>/dev/null)
        printf '%s%s (%s)' "${sep:-}" "$nome" "${bt:-?}"; sep="; "
    done
}
BUILDS=$(configs)
[ -z "$BUILDS" ] && BUILDS="(nenhuma configurada)"

DATA=$(date -Is)

# --- saída ---------------------------------------------------------------
if [ "$MODO" = "--markdown" ]; then
    cat <<EOF
| Item | Valor |
|---|---|
| CPU | $CPU_MODELO |
| Núcleos / threads | $CPU_FISICOS físicos, $CPU_LOGICAS lógicos ($CPU_SMT por núcleo, smt/active=$CPU_SMT_ATIVO) |
| Soquetes / nós NUMA | $CPU_SOQUETES / $NUMA_NOS |
| L3 | $CPU_L3 |
| Domínios de L3 (CCD) | \`$CPU_DOMINIOS\` |
| CPUs lógicas por domínio | \`$CCD_RESUMO\` |
| Sinalizadores de TSC | $CPU_TSC |
| TSC invariante | $TSC_INVARIANTE |
| Governor / turbo | $GOVERNOR / intel_no_turbo=$TURBO_INTEL amd_boost=$TURBO_AMD |
| PMU | perf_event_paranoid=$PMU_PARANOID — $PMU_ESTADO |
| Memória | $MEM_TOTAL (disponível: $MEM_DISPO) |
| Velocidade da memória | $MEM_VEL |
| Pentes / canais | $MEM_CANAIS |
| Hugepages | $HUGE_TOTAL de $HUGE_TAM ($HUGE_LIVRES livres) |
| Placa | $PLACA |
| BIOS | $BIOS |
| Kernel / arquitetura | $KERNEL / $ARQ |
| Distribuição | $DISTRO |
| Linha de comando do kernel | \`$CMDLINE\` |
| Mitigações ativas | $MITIG_ATIVAS |
| Mitigações não aplicáveis | $MITIG_INATIVAS |
| GCC | $GCC |
| Clang | $CLANG |
| Meson / Ninja | $MESON / $NINJA |
| perf | $PERF_BIN |
| Configurações de build na árvore | $BUILDS |
| Commit / árvore suja | $GIT_COMMIT / $GIT_SUJO |
| Coletado em | $DATA |
EOF
    exit 0
fi

if [ "$MODO" = "--json" ]; then
    # Montado por python3 para que a citação seja correta por construção, e não
    # por cuidado ao escrever heredoc: um valor de mitigação contém ';' e '='.
    #
    # Os valores atravessam por AMBIENTE, e não por argumento nem por
    # interpolação no heredoc: mitigação ativa traz ';' e '=', a topologia traz
    # tabulação e quebra de linha, e qualquer um dos dois quebraria a citação de
    # um heredoc montado à mão -- em silêncio, produzindo JSON inválido ou, pior,
    # JSON válido com campo truncado.
    export AMB_CPU_MODELO="$CPU_MODELO" AMB_CPU_FISICOS="$CPU_FISICOS"
    export AMB_CPU_LOGICOS="$CPU_LOGICAS" AMB_CPU_SMT="$CPU_SMT"
    export AMB_CPU_SMT_ATIVO="$CPU_SMT_ATIVO" AMB_CPU_SOQUETES="$CPU_SOQUETES"
    export AMB_NUMA_NOS="$NUMA_NOS" AMB_L3="$CPU_L3"
    export AMB_L3_DOMINIOS="$CPU_DOMINIOS" AMB_L3_CPUS_POR_DOMINIO="$CCD_RESUMO"
    export AMB_TSC_SINALIZADORES="$CPU_TSC" AMB_TSC_INVARIANTE="$TSC_INVARIANTE"
    export AMB_GOVERNOR="$GOVERNOR" AMB_TURBO_INTEL="$TURBO_INTEL"
    export AMB_TURBO_AMD="$TURBO_AMD" AMB_PMU_PARANOID="$PMU_PARANOID"
    export AMB_PMU_ESTADO="$PMU_ESTADO" AMB_MEM_TOTAL="$MEM_TOTAL"
    export AMB_MEM_DISPONIVEL="$MEM_DISPO" AMB_MEM_VELOCIDADE="$MEM_VEL"
    export AMB_MEM_CANAIS="$MEM_CANAIS" AMB_HUGEPAGES_TOTAL="$HUGE_TOTAL"
    export AMB_HUGEPAGES_LIVRES="$HUGE_LIVRES" AMB_HUGEPAGES_TAMANHO="$HUGE_TAM"
    export AMB_PLACA="$PLACA" AMB_BIOS="$BIOS" AMB_KERNEL="$KERNEL"
    export AMB_ARQUITETURA="$ARQ" AMB_DISTRIBUICAO="$DISTRO"
    export AMB_KERNEL_CMDLINE="$CMDLINE" AMB_MITIGACOES_ATIVAS="$MITIG_ATIVAS"
    export AMB_MITIGACOES_NAO_APLICAVEIS="$MITIG_INATIVAS"
    export AMB_GCC="$GCC" AMB_CLANG="$CLANG" AMB_MESON="$MESON"
    export AMB_NINJA="$NINJA" AMB_PERF="$PERF_BIN"
    export AMB_BUILDS_NA_ARVORE="$BUILDS" AMB_COMMIT="$GIT_COMMIT"
    export AMB_ARVORE_SUJA="$GIT_SUJO" AMB_COLETADO_EM="$DATA"
    export AMB_TOPOLOGIA="$TOPOLOGIA"
    python3 - <<'PY'
import json, os, sys
campos = [
    "cpu_modelo", "cpu_fisicos", "cpu_logicos", "cpu_smt", "cpu_smt_ativo",
    "cpu_soquetes", "numa_nos", "l3", "l3_dominios", "l3_cpus_por_dominio",
    "tsc_sinalizadores", "tsc_invariante", "governor", "turbo_intel",
    "turbo_amd", "pmu_paranoid", "pmu_estado", "mem_total", "mem_disponivel",
    "mem_velocidade", "mem_canais", "hugepages_total", "hugepages_livres",
    "hugepages_tamanho", "placa", "bios", "kernel", "arquitetura",
    "distribuicao", "kernel_cmdline", "mitigacoes_ativas",
    "mitigacoes_nao_aplicaveis", "gcc", "clang", "meson", "ninja", "perf",
    "builds_na_arvore", "commit", "arvore_suja", "coletado_em",
]
faltando = [c for c in campos if not os.environ.get("AMB_" + c.upper())]
# Campo vazio no metadata.json é o defeito que este script existe para não
# cometer: a norma manda documentar variável desconhecida COMO desconhecida
# (seção 28), e um "" silencioso é indistinguível de "não havia o que
# reportar". Então o JSON sai com a lista do que não foi coletado.
d = {c: os.environ.get("AMB_" + c.upper(), "") or None for c in campos}
d["campos_nao_coletados"] = faltando
d["topologia"] = [
    dict(zip(("cpu", "core_id", "l3_id", "smt_siblings"), l.split("\t")))
    for l in os.environ.get("AMB_TOPOLOGIA", "").splitlines() if l.strip()
]
json.dump(d, sys.stdout, indent=2, ensure_ascii=False)
print()
PY
    exit 0
fi

cat <<EOF

== Ambiente de medição ==

  Processador
    modelo ................. $CPU_MODELO
    nucleos/threads ........ $CPU_FISICOS fisicos, $CPU_LOGICAS logicos ($CPU_SMT por nucleo)
    SMT ativo .............. $CPU_SMT_ATIVO
    soquetes / nos NUMA .... $CPU_SOQUETES / $NUMA_NOS
    L3 ..................... $CPU_L3
    dominios de L3 (CCD) ... $CPU_DOMINIOS
    CPUs por dominio ....... $CCD_RESUMO
    sinalizadores de TSC ... $CPU_TSC
    TSC invariante ......... $TSC_INVARIANTE

  Estado de frequencia (decide se a medicao e comparavel entre execucoes)
    governor ............... $GOVERNOR
    turbo .................. intel_no_turbo=$TURBO_INTEL  amd_boost=$TURBO_AMD

  Contadores de hardware
    perf_event_paranoid .... $PMU_PARANOID ($PMU_ESTADO)
    perf ................... $PERF_BIN

  Memoria
    total .................. $MEM_TOTAL
    disponivel ............. $MEM_DISPO
    velocidade ............. $MEM_VEL
    pentes / canais ........ $MEM_CANAIS
    hugepages .............. $HUGE_TOTAL de $HUGE_TAM ($HUGE_LIVRES livres)

  Placa e firmware
    placa .................. $PLACA
    BIOS ................... $BIOS

  Sistema
    kernel / arquitetura ... $KERNEL / $ARQ
    distribuicao ........... $DISTRO
    linha de comando ....... $CMDLINE
    mitigacoes ativas ...... $MITIG_ATIVAS
    nao aplicaveis ......... $MITIG_INATIVAS

  Ferramental
    gcc .................... $GCC
    clang .................. $CLANG
    meson / ninja .......... $MESON / $NINJA
    builds na arvore ....... $BUILDS
    commit / arvore suja ... $GIT_COMMIT / $GIT_SUJO

  Topologia por CPU logica (cpu, core, L3/CCD, par SMT)
$(printf '%s\n' "$TOPOLOGIA" | awk -F'\t' '{printf "    cpu%-3s core %-3s L3#%-2s par %s\n", $1, $2, $3, $4}')

  Para colar num documento: ./scripts/ambiente.sh --markdown
  Para o metadata.json:     ./scripts/ambiente.sh --json

EOF
