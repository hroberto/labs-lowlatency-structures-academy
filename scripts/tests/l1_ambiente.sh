#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Autoteste de scripts/ambiente.sh.
#
# POR QUE ELE EXISTE, E O QUE ELE TESTA DE VERDADE
#
# Um script de ambiente que devolve campo vazio EM SILÊNCIO é o defeito que a
# seção 29 da norma manda evitar, e é o defeito que o script de origem cometeu
# (parsing do texto traduzido do `lscpu`). Um autoteste que só conferisse
# "rodou e saiu com zero" não pegaria nada disso: o script sai com zero
# justamente quando devolve campo vazio.
#
# Os três casos abaixo nasceram de defeitos reais desta porta:
#
#   1. o resumo de CPUs por domínio de L3 perdia o PRIMEIRO grupo, porque em
#      awk uma variável não inicializada compara numericamente com `$1`. A
#      saída ficava plausível -- um CCD, com o número certo de CPUs dentro --,
#      e por isso o caso tem BRAÇO SINTÉTICO: numa máquina de um único CCD, um
#      teste que olhasse só a saída real passaria com o bug presente;
#   2. o modo `--json` lia variáveis de ambiente que o script não exportava, e
#      devolvia JSON válido com todos os campos vazios;
#   3. valor de mitigação ativa contém ';' e '=', e topologia contém tabulação:
#      qualquer um deles quebra a citação de um JSON montado à mão.
set -u

RAIZ="$(cd "$(dirname "$0")/../.." && pwd)"
AMBIENTE="$RAIZ/scripts/ambiente.sh"
falhas=0

erro() { printf 'FALHA: %s\n' "$*" >&2; falhas=$((falhas + 1)); }
ok()   { printf '  ok: %s\n' "$*"; }

[ -x "$AMBIENTE" ] || { erro "$AMBIENTE nao e executavel"; exit 1; }

# --- 1. braço sintético do agrupamento de L3 ------------------------------
# Dois domínios na entrada têm de virar dois grupos na saída. É o braço de
# controle: ele dispara mesmo numa máquina sem CCD múltiplo.
esperado='L3#0=[0,4] L3#1=[1,5]'
obtido=$(printf '0\t0\n0\t4\n1\t1\n1\t5\n' | "$AMBIENTE" --agrupar-l3)
if [ "$obtido" = "$esperado" ]; then
    ok "agrupamento de L3 preserva os dois dominios"
else
    erro "agrupamento de L3: esperado '$esperado', obtido '$obtido'"
fi

# Entrada de um domínio só: não deve inventar grupo nem perder o único.
obtido=$(printf '3\t0\n3\t1\n' | "$AMBIENTE" --agrupar-l3)
[ "$obtido" = 'L3#3=[0,1]' ] || erro "agrupamento de um dominio: obtido '$obtido'"
[ "$obtido" = 'L3#3=[0,1]' ] && ok "agrupamento de um dominio unico"

# Entrada vazia: saída vazia, e sem erro.
obtido=$(printf '' | "$AMBIENTE" --agrupar-l3)
[ -z "$obtido" ] || erro "agrupamento de entrada vazia devolveu '$obtido'"
[ -z "$obtido" ] && ok "agrupamento de entrada vazia nao inventa grupo"

# --- 2. modo texto --------------------------------------------------------
texto=$("$AMBIENTE") || erro "modo texto saiu com codigo diferente de zero"
for campo in "modelo" "dominios de L3" "CPUs por dominio" "TSC invariante" \
             "perf_event_paranoid" "builds na arvore" "Topologia por CPU logica"; do
    case "$texto" in
        *"$campo"*) ;;
        *) erro "modo texto nao traz o campo '$campo'" ;;
    esac
done
ok "modo texto traz os campos que a norma exige (secoes 28 e 29)"

# Uma linha de topologia por CPU lógica: se o laço do sysfs quebrar, some.
linhas_topo=$(printf '%s\n' "$texto" | grep -c '^    cpu[0-9]')
if [ "$linhas_topo" -eq "$(nproc)" ]; then
    ok "topologia tem uma linha por CPU logica ($linhas_topo)"
else
    erro "topologia tem $linhas_topo linhas para $(nproc) CPUs logicas"
fi

# --- 3. modo markdown -----------------------------------------------------
md=$("$AMBIENTE" --markdown) || erro "modo markdown saiu com codigo diferente de zero"
case "$md" in
    "| Item | Valor |"*) ok "markdown comeca com o cabecalho da tabela" ;;
    *) erro "markdown nao comeca com '| Item | Valor |'" ;;
esac

# CÉLULA VAZIA É O DEFEITO CENTRAL: `| CPU |  |` passaria por qualquer teste
# que só contasse linhas.
vazias=$(printf '%s\n' "$md" | grep -c '^|[^|]*|[[:space:]]*|$' || true)
if [ "$vazias" -eq 0 ]; then
    ok "nenhuma celula vazia na tabela markdown"
else
    erro "$vazias celula(s) vazia(s) no markdown:"
    printf '%s\n' "$md" | grep -n '^|[^|]*|[[:space:]]*|$' >&2
fi

# --- 4. modo json ---------------------------------------------------------
json=$("$AMBIENTE" --json) || erro "modo json saiu com codigo diferente de zero"

# O JSON vai para python3 por ARQUIVO, e o caminho por argumento. Nao por
# `pipe`: o heredoc do proprio programa python ocupa o stdin, e um
# `printf | python3 - <<'PY'` entrega ao interprete o TEXTO DO PROGRAMA como
# entrada, nao o JSON. A primeira versao deste teste falhava exatamente assim,
# acusando "JSON invalido" com o JSON intacto -- teste que acusa a coisa errada
# custa mais caro que teste ausente.
TMP_JSON=$(mktemp) || { erro "mktemp falhou"; exit 1; }
trap 'rm -f "$TMP_JSON"' EXIT
printf '%s' "$json" > "$TMP_JSON"

if ! command -v python3 >/dev/null 2>&1; then
    printf '  PULADO: sem python3 para validar o JSON\n'
else
    python3 - "$TMP_JSON" <<'PY' || falhas=$((falhas + 1))
import json, sys
try:
    d = json.load(open(sys.argv[1], encoding="utf-8"))
except Exception as e:
    sys.exit(f"FALHA: --json nao produziu JSON valido: {e}")

if not isinstance(d, dict) or len(d) < 40:
    sys.exit(f"FALHA: --json trouxe {len(d)} chaves; esperadas 40 ou mais")

# O campo que o script declara como nao coletado tem de estar declarado, e nao
# apenas vazio: a norma manda documentar variavel desconhecida COMO
# desconhecida.
nao_coletados = d.get("campos_nao_coletados")
if nao_coletados is None:
    sys.exit("FALHA: --json nao traz a lista campos_nao_coletados")
vazios = [k for k, v in d.items()
          if k not in ("campos_nao_coletados", "topologia") and v in ("", None)]
if sorted(vazios) != sorted(nao_coletados):
    sys.exit(f"FALHA: campos vazios {sorted(vazios)} divergem dos declarados "
             f"{sorted(nao_coletados)} -- vazio em silencio e o defeito que este "
             f"script existe para nao cometer")

topo = d.get("topologia") or []
if len(topo) < 1:
    sys.exit("FALHA: --json nao traz topologia por CPU logica")
faltando = [c for c in ("cpu", "core_id", "l3_id", "smt_siblings")
            if c not in topo[0]]
if faltando:
    sys.exit(f"FALHA: entrada de topologia sem os campos {faltando}")

# A tabulacao da topologia e o ';'/'=' das mitigacoes atravessaram intactos?
if d.get("mitigacoes_ativas") and "nao expostas" not in d["mitigacoes_ativas"]:
    if "=" not in d["mitigacoes_ativas"]:
        sys.exit("FALHA: mitigacoes_ativas perdeu o '=' no caminho para o JSON")

print(f"  ok: json com {len(d)} chaves, {len(topo)} CPUs na topologia, "
      f"{len(nao_coletados)} campo(s) declarado(s) nao coletado(s)")
PY
fi

# --- 5. o resumo real bate com a topologia real ---------------------------
# Invariante: tantos grupos no resumo quantos domínios distintos de L3 houver.
if command -v python3 >/dev/null 2>&1; then
    python3 - "$TMP_JSON" <<'PY' || falhas=$((falhas + 1))
import json, sys
d = json.load(open(sys.argv[1], encoding="utf-8"))
ids = {e["l3_id"] for e in d.get("topologia", []) if e.get("l3_id") not in ("", "-", None)}
resumo = d.get("l3_cpus_por_dominio") or ""
grupos = resumo.count("L3#")
if not ids:
    print("  PULADO: sysfs nao expoe id de L3 nesta maquina")
elif grupos != len(ids):
    sys.exit(f"FALHA: {len(ids)} dominio(s) de L3 na topologia, {grupos} no resumo")
else:
    print(f"  ok: resumo de L3 cobre os {grupos} dominio(s) da topologia")
PY
fi

if [ "$falhas" -eq 0 ]; then
    printf 'ambiente.sh: todos os casos passaram\n'
    exit 0
fi
printf 'ambiente.sh: %d caso(s) falharam\n' "$falhas" >&2
exit 1
