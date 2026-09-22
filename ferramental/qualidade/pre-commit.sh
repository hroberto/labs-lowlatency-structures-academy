#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Portão de consistência do projeto: gancho de commit e etapa de CI.
#
# POR QUE ELE RODA NOS DOIS LUGARES
#
# Um gancho local não roda em pull request de terceiro nem em PR do Dependabot,
# por construção. E uma etapa de CI não impede que um commit ruim exista -- ela
# só o reprova depois. As duas coisas cobrem lacunas diferentes, e o projeto a
# montante registrou o caso em que este passo foi perdido num `reset --hard`
# **enquanto o README continuava afirmando que a CI o executava**: retratação
# falsa é pior que a lacuna original, porque desliga o alarme.
#
# MODOS
#
#   --rapido   sintaxe, verificadores de documentação, segredos, pin da ação.
#              É o que roda na CI antes das etapas de build nomeadas: sem ele,
#              a suíte rodaria duas vezes e uma falha ficaria sem endereço --
#              o vermelho apareceria aqui, antes do `meson setup`, sem dizer
#              qual suíte quebrou.
#   (sem flag) o mesmo, mais a suíte inteira nas configurações construídas.
set -u

RAIZ="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$RAIZ"
RAPIDO=0
[ "${1:-}" = "--rapido" ] && RAPIDO=1

falhas=0
ok()    { printf '  ok     - %s\n' "$1"; }
falha() { printf '  FALHA  - %s\n' "$1"; falhas=$((falhas + 1)); }
aviso() { printf '  aviso  - %s\n' "$1"; }

# --- 1. sintaxe -----------------------------------------------------------
# `bash -n` é barato e pega o erro que mais dói: um script que só falha quando
# alguém o executa, semanas depois, no meio de uma campanha de medição.
echo "Sintaxe:"
erros_sh=0
while IFS= read -r s; do
    bash -n "$s" 2>/dev/null || { falha "bash -n: $s"; erros_sh=$((erros_sh + 1)); }
done < <(find scripts ferramental -name '*.sh' -not -path '*/build*' 2>/dev/null)
[ "$erros_sh" -eq 0 ] && ok "todos os scripts shell passam em bash -n"

if command -v python3 >/dev/null 2>&1; then
    if python3 -m compileall -q ferramental scripts >/dev/null 2>&1; then
        ok "todos os .py compilam"
    else
        falha "python3 -m compileall acusou erro de sintaxe"
        python3 -m compileall -q ferramental scripts 2>&1 | sed 's/^/           /' | head -10
    fi
    # O __pycache__ que o compileall cria é ruído versionado se escapar.
    find ferramental scripts -name '__pycache__' -type d -exec rm -rf {} + 2>/dev/null
else
    aviso "sem python3: os verificadores e o compileall serao PULADOS"
fi

# --- 2. verificadores de documentação -------------------------------------
#
# `suposicao` NÃO ESTÁ NA LISTA RÁPIDA, e a separação nasceu de uma CI vermelha.
#
# Ele é o único verificador que COMPILA uma sonda: confere no assembly que a
# pré-condição da porta R chega ao otimizador. Os outros cinco leem Markdown.
# O job de consistência da CI roda antes do build e não instala compilador de
# propósito -- e o `g++` padrão da imagem é o 13, que não tem `<print>`. O
# verificador então se recusou a aprovar, corretamente: "a sonda NAO COMPILOU
# -- a porta R nao pode ser verificada".
#
# Quem estava errado era a lista, não ele. A cobertura não se perde: o job de
# build roda `test-all.sh`, e a suíte registra `verificar-suposicao` com os
# compiladores instalados.
VERIFICADORES_DOC="links ancoras aritmetica retratacoes paridade autodescricao promessa medicao"
VERIFICADORES_COMPILADOR="suposicao"
if [ "$RAPIDO" -eq 1 ]; then
    VERIFICADORES="$VERIFICADORES_DOC"
else
    VERIFICADORES="$VERIFICADORES_DOC $VERIFICADORES_COMPILADOR"
fi

echo "Documentacao:"
if command -v python3 >/dev/null 2>&1; then
    for v in $VERIFICADORES; do
        script="ferramental/qualidade/verificar-$v.py"
        [ -f "$script" ] || { falha "$script nao existe"; continue; }
        if saida=$(python3 "$script" . 2>&1); then
            ok "verificar-$v"
        else
            falha "verificar-$v acusou:"
            printf '%s\n' "$saida" | sed 's/^/           /'
        fi
    done
    # O autoteste de cada verificador roda AQUI também, e não só na suíte: um
    # verificador que passou a aceitar tudo continua imprimindo "0 quebrados".
    for v in $VERIFICADORES; do
        python3 "ferramental/qualidade/verificar-$v.py" --autoteste >/dev/null 2>&1 \
            || falha "autoteste de verificar-$v falhou"
    done
    ok "autotestes dos verificadores executados: $VERIFICADORES"
    if [ "$RAPIDO" -eq 1 ]; then
        # DITO, e não omitido: o que não rodou não pode passar por verificado.
        aviso "verificar-suposicao PULADO no modo rapido (exige compilador C++23); ele roda na suite, no job de build"
    fi
fi

# --- 3. segredos ----------------------------------------------------------
# Escopo declarado: chave privada e token com prefixo reconhecível nos arquivos
# RASTREADOS. Não é varredura de histórico nem de entropia -- dizer o escopo
# evita que este passo seja lido como garantia que ele não dá.
echo "Segredos (nos arquivos rastreados, por padrao reconhecivel):"
if git rev-parse --git-dir >/dev/null 2>&1; then
    padrao='BEGIN [A-Z ]*PRIVATE KEY|ghp_[A-Za-z0-9]{36}|github_pat_[A-Za-z0-9_]{50,}|AKIA[0-9A-Z]{16}|xox[baprs]-[A-Za-z0-9-]{10,}'
    if achados=$(git grep -nIE "$padrao" -- . 2>/dev/null); then
        falha "padrao de segredo encontrado:"
        printf '%s\n' "$achados" | sed 's/^/           /'
    else
        ok "nenhum padrao de chave privada ou token reconhecido"
    fi
else
    aviso "fora de repositorio git: varredura de segredos pulada"
fi

# --- 4. o pin da ação de CI ainda é o topo do major declarado? ------------
#
# A ação é fixada por SHA, e não por tag, porque `v5` é ponteiro móvel e quem
# controla o repositório da ação pode reapontá-lo. O preço é que SHA fixo
# envelhece em silêncio -- e foi assim que, no projeto a montante, um PR do
# Dependabot trocou o SHA sem trocar o comentário que declarava a versão.
echo "Pin da acao de CI:"
WF=".github/workflows/ci.yml"
if [ ! -f "$WF" ]; then
    aviso "$WF nao existe"
elif ! command -v gh >/dev/null 2>&1; then
    aviso "sem gh: nao da para resolver a tag declarada de volta para SHA"
else
    while IFS= read -r linha; do
        acao=$(printf '%s' "$linha" | sed -n 's|.*uses: \([^@]*\)@\([0-9a-f]\{40\}\).*|\1|p')
        sha=$(printf '%s' "$linha" | sed -n 's|.*uses: [^@]*@\([0-9a-f]\{40\}\).*|\1|p')
        [ -n "$acao" ] || continue
        major=$(grep -A3 -F "$sha" "$WF" | sed -n 's|.*gh api repos/[^/]*/[^/]*/git/ref/tags/\(v[0-9]*\).*|\1|p' | head -1)
        [ -n "$major" ] || major=$(grep -B8 -F "$sha" "$WF" | sed -n 's|.*[Cc]orresponde a \(v[0-9]*\).*|\1|p' | head -1)
        if [ -z "$major" ]; then
            aviso "$acao@$sha sem major declarado no comentario -- nao da para conferir"
            continue
        fi
        topo=$(gh api "repos/$acao/git/ref/tags/$major" --jq '.object.sha' 2>/dev/null || echo "")
        if [ -z "$topo" ]; then
            aviso "$acao: nao foi possivel consultar $major (rede ou permissao)"
        elif [ "$topo" = "$sha" ]; then
            ok "$acao@$sha e o topo de $major"
        else
            aviso "$acao: o SHA fixado nao e mais o topo de $major (topo: $topo). Atualizar exige trocar TAMBEM o comentario que declara a versao"
        fi
    done < <(grep -E 'uses: .*@[0-9a-f]{40}' "$WF")
fi

# --- 5. suíte (só no modo completo) ---------------------------------------
if [ "$RAPIDO" -eq 0 ]; then
    echo "Suite:"
    if ls -d build-* >/dev/null 2>&1; then
        ./scripts/test-all.sh >/dev/null 2>&1 && ok "test-all.sh passou nas configuracoes construidas" \
            || { falha "test-all.sh falhou"; ./scripts/test-all.sh 2>&1 | tail -12 | sed 's/^/           /'; }
    else
        aviso "nenhuma configuracao construida: rode ./scripts/build-all.sh"
    fi
fi

echo
if [ "$falhas" -eq 0 ]; then
    printf 'pre-commit: tudo consistente.\n'
    exit 0
fi
printf 'pre-commit: %d falha(s). O commit nao deve seguir assim.\n' "$falhas" >&2
exit 1
