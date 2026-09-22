#!/usr/bin/env python3
"""Confere que o material só cita programas que a árvore de fato entrega.

POR QUE ISTO EXISTE

Este projeto não promete que o leitor ENTENDA — promete que ele consiga REFAZER.
A forma de toda promessa executável é a mesma: "rode `X` e observe `Y`". Clareza
não tem proxy sintático, mas **"o programa citado existe e entra na
compilação" é decidível por caminho**.

E é uma promessa que o projeto de origem quebrou duas vezes, pelo mesmo
mecanismo — um `git filter-branch` faz checkout ao terminar e descarta o que não
estava commitado. Nos dois casos o arquivo ficou no disco e fora da compilação,
enquanto a documentação seguia citando o que existia no texto e não no build. O
portão de "build limpo, zero avisos" não o enxergava: fonte que ninguém
referencia não entra na compilação, e o portão anunciava que tinha conferido.

AS TRÊS REGRAS

  1. TODO PROGRAMA CITADO EXISTE. Um token entre crases com cara de programa
     deste projeto — minúsculas com hífen, ou terminado em `.sh`/`.py` — precisa
     ser alvo de `executable()` em algum `meson.build`, arquivo da árvore, ou
     ferramenta externa DECLARADA em `EXTERNAS`.
  2. TODA FONTE ENTRA NA COMPILAÇÃO. Todo `.cpp` sob `docs/` e `lib/` precisa
     ser referenciado por algum `meson.build`. Fonte órfã não é compilada, e
     portanto não é conferida por nada — nem pelo compilador, nem pela suíte.
  3. TODO COMANDO `./caminho` DE BLOCO DE CÓDIGO APONTA PARA ARQUIVO. Um bloco
     ```bash com `./scripts/foo.sh` é a forma mais literal da promessa: o leitor
     vai copiar e colar.

O CUSTO DESTA VERIFICAÇÃO, DECLARADO

A lista `EXTERNAS` precisa de manutenção. Não há como distinguir por sintaxe o
nome de um pacote (`ninja-build`, `python3-pip`) do nome de um programa deste
projeto (`arquivar-medicao.sh`): os dois são minúsculas com hífen. Quem citar
uma ferramenta externa nova precisa declará-la aqui, e o erro diz exatamente
isso.

O QUE ELE DELIBERADAMENTE NÃO PEGA

- se o programa citado FAZ o que o texto diz que ele faz. Isso é a regra do
  número com programa (seção 8 da norma), e quem a sustenta é a campanha
  arquivada;
- cabeçalho `.hpp` órfão. A régua e a biblioteca de contratos são header-only de
  propósito, e um cabeçalho entra na compilação por inclusão, não por
  `meson.build` — exigir referência direta acusaria o desenho correto.
"""
import os
import re
import sys

IGNORAR = {".git", "build", "builddir", "subprojects", "__pycache__", "temp"}
ORIGEM_PRESERVADA = "origem"

# Ferramentas externas citadas pelo material. Cada uma é uma DECLARAÇÃO: quem
# acrescenta uma, assume que ela é de fora.
EXTERNAS = {
    "meson", "ninja", "ninja-build", "python3", "python3-pip", "git", "gh",
    "perf", "clang", "clang++", "gcc", "g++", "sudo", "sysctl", "dmidecode",
    "lscpu", "setcap", "apt-get", "update-alternatives", "install",
    "actions/checkout", "clang-format", "clang-tidy", "cppcheck",
    "google-benchmark", "gtest", "googletest", "hdr_histogram",
    "perf_event_open", "perf_event_paranoid", "kernel.perf_event_paranoid",
    "clock_gettime", "filter-branch", "libstdc++", "ubuntu-24.04",
    "ubuntu-26.04", "ubuntu-latest", "ubuntu-22.04", "wrapdb",
    # Nomes de PERFIL DE BUILD e de configuracao, nao de programa.
    "release-checked", "sanitize-address", "sanitize-thread",
    "debugoptimized", "github-actions", "cppcoreguidelines-macro-usage",
    "build-release", "build-debug", "build-release-checked",
}

# Programas do projeto a montante que este repositório cita como DÍVIDA
# DECLARADA — eles existem lá e não aqui, e o ROADMAP diz isso.
#
# Quando um deles for portado, some daqui: o arquivo passa a existir e a regra 1
# o confere de verdade. Manter na lista depois de portado mascararia a
# verificação, e é por isso que a lista é curta e nominal.
NAO_PORTADOS = {
    "verificar-medicao.py", "verificar-citacao.py", "verificar-evidencia.py",
    "verificar-tabelas.py", "verificar-blocos.py", "verificar-identidade.py",
    "verificar-forca-epistemica.py", "verificar-ponteiros.py",
    "auditar-fontes.py", "inventariar-dados.py", "rastros-da-suite.py",
    "variacao-entre-execucoes.py", "comparar-hardware.py",
    "pular-sem-gtest.sh", "graficos-memoria.py",
}

# Tem cara de programa deste projeto?
#
# HÍFEN, E NÃO SUBLINHADO. A primeira versão aceitava os dois como separador e
# acusou `unordered_map` e `condition_variable` -- identificadores de C++, que
# o material cita o tempo todo. Programa deste projeto usa hífen
# (`arquivar-medicao.sh`); sublinhado é estilo de identificador.
PROGRAMA = re.compile(r"^(?:[a-z0-9]+(?:-[a-z0-9]+)+(?:\.sh|\.py)?|[a-z0-9-]+\.(?:sh|py))$")

# Linha de tabela que DECLARA que o item ainda não existe.
#
# Os índices de módulo listam os tópicos previstos com o rótulo da seção 5 da
# norma: `| \`01-map-de-precos\` — ... | **não iniciado** |`. Acusar esses
# nomes seria exigir que o índice só citasse o que já existe -- ou seja, que
# não houvesse índice. O rótulo é a declaração, e ela basta.
NAO_INICIADO = re.compile(r"^\|.*\|\s*\*\*(?:não iniciado|nao iniciado|not started)\*\*\s*\|",
                          re.I)

# Item de ROADMAP em aberto: `- [ ] portar \`verificar-medicao.py\``.
#
# Um checkbox aberto é a forma que este projeto usa para dizer "isto não existe
# ainda". Acusar o que ele cita seria exigir que o ROADMAP só falasse do que já
# está feito -- ou seja, que não houvesse ROADMAP.
PENDENCIA = re.compile(r"^-\s*\[ \]")
CRASE = re.compile(r"`([^`\n]+)`")
COMANDO = re.compile(r"^\s*(?:\$\s*)?(\./[\w./-]+)", re.M)


def documentos(raiz):
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        if ORIGEM_PRESERVADA in os.path.normpath(base).split(os.sep):
            continue
        for nome in sorted(arquivos):
            if nome.endswith(".md"):
                yield os.path.join(base, nome)


def blocos_bash(texto):
    """Só o conteúdo dos blocos ```bash / ```sh / ```console."""
    return re.findall(r"^```(?:bash|sh|console)\n(.*?)^```", texto, re.S | re.M)


def meson_texto(raiz):
    partes = []
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        if "meson.build" in arquivos:
            partes.append(open(os.path.join(base, "meson.build"), encoding="utf-8").read())
    return "\n".join(partes)


def arquivos_da_arvore(raiz):
    """Arquivos E DIRETÓRIOS.

    Só arquivos era bug: o índice do módulo 08 cita `01-harness`, que existe --
    como diretório. A regra acusava o índice de prometer o que ele entrega.
    """
    fora = set()
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        for nome in list(dirs) + list(arquivos):
            fora.add(nome)
            fora.add(os.path.relpath(os.path.join(base, nome), raiz))
    return fora


def verificar(raiz="."):
    if not os.path.isdir(raiz):
        print(f"ERRO: raiz inexistente: {raiz}")
        return 1

    problemas = 0
    meson = meson_texto(raiz)
    arquivos = arquivos_da_arvore(raiz)
    # Alvos de executable('nome', ...)
    alvos = set(re.findall(r"executable\(\s*'([^']+)'", meson))
    citados = 0

    # --- regra 1 ----------------------------------------------------------
    for caminho in documentos(raiz):
        texto = open(caminho, encoding="utf-8").read()
        pendente = False
        for linha in texto.split("\n"):
            crua = linha.strip()
            # Item de ROADMAP continua em linhas seguintes, indentadas.
            if PENDENCIA.match(crua):
                pendente = True
            elif crua.startswith("- [") or crua.startswith("#") or not crua:
                pendente = False
            if pendente or NAO_INICIADO.match(crua):
                continue
            for token in CRASE.findall(linha):
                t = token.strip()
                if not PROGRAMA.match(t) or t in EXTERNAS or t in NAO_PORTADOS:
                    continue
                citados += 1
                if t in alvos or t in arquivos or any(a.endswith("/" + t) for a in arquivos):
                    continue
                print(f"  {caminho}: cita `{t}`, que nao e alvo do meson, nem arquivo "
                      f"da arvore, nem esta declarado em EXTERNAS")
                problemas += 1

    # --- regra 2 ----------------------------------------------------------
    for sub in ("docs", "lib"):
        base_dir = os.path.join(raiz, sub)
        if not os.path.isdir(base_dir):
            continue
        for base, dirs, arqs in os.walk(base_dir):
            dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
            for nome in sorted(arqs):
                if not nome.endswith(".cpp"):
                    continue
                rel = os.path.relpath(os.path.join(base, nome), raiz)
                citados += 1
                if nome not in meson and rel not in meson:
                    print(f"  {rel}: fonte orfa -- nenhum meson.build a referencia, "
                          f"logo ela nao e compilada nem conferida")
                    problemas += 1

    # --- regra 3 ----------------------------------------------------------
    for caminho in documentos(raiz):
        texto = open(caminho, encoding="utf-8").read()
        for bloco in blocos_bash(texto):
            for cmd in COMANDO.findall(bloco):
                alvo = cmd[2:]
                citados += 1
                if not os.path.exists(os.path.join(raiz, alvo)):
                    print(f"  {caminho}: bloco de comando manda rodar `{cmd}`, "
                          f"que nao existe na arvore")
                    problemas += 1

    print(f"\n  {citados} promessa(s) conferida(s), {problemas} quebrada(s)")
    return problemas


# --------------------------------------------------------------------------
def autoteste():
    import io as _io
    import tempfile
    from contextlib import redirect_stdout

    falhas = 0

    def caso(n, descricao, arquivos, deve_acusar):
        nonlocal falhas
        with tempfile.TemporaryDirectory() as raiz:
            for nome, conteudo in arquivos.items():
                caminho = os.path.join(raiz, nome)
                os.makedirs(os.path.dirname(caminho), exist_ok=True)
                open(caminho, "w", encoding="utf-8").write(conteudo)
            buf = _io.StringIO()
            with redirect_stdout(buf):
                rc = verificar(raiz)
            if (rc > 0) != deve_acusar:
                print(f"  AUTOTESTE {n} FALHOU ({descricao}): "
                      f"{'acusou' if rc > 0 else 'nao acusou'}")
                print(buf.getvalue())
                falhas += 1

    MESON = "executable(\n  'bench-harness',\n  'docs/08/bench.cpp',\n)\n"

    # 1. programa citado que existe como alvo do meson.
    caso(1, "alvo existente acusado",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "Rode o `bench-harness`.\n"}, False)

    # 2. o defeito: o texto cita programa que nao existe em lugar nenhum.
    caso(2, "programa inexistente nao acusado",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "Rode o `bench-inexistente`.\n"}, True)

    # 3. script citado que existe como arquivo.
    caso(3, "script existente acusado",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "scripts/arquivar-medicao.sh": "#!/bin/sh\n",
          "d.md": "Rode `arquivar-medicao.sh`.\n"}, False)

    # 4. FONTE ORFA -- o defeito que o projeto de origem cometeu duas vezes.
    caso(4, "fonte orfa nao acusada",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "docs/08/orfa.cpp": "int main(){}"}, True)

    # 5. bloco de comando apontando para arquivo inexistente.
    caso(5, "comando para arquivo inexistente nao acusado",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "```bash\n./scripts/nao-existe.sh\n```\n"}, True)

    # 6. bloco de comando apontando para arquivo existente.
    caso(6, "comando valido acusado",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "scripts/existe.sh": "#!/bin/sh\n",
          "d.md": "```bash\n./scripts/existe.sh\n```\n"}, False)

    # 7. ISCA: ferramenta externa declarada nao e programa deste projeto.
    caso(7, "ferramenta externa declarada acusada",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "Instale `ninja-build` e rode `meson`.\n"}, False)

    # 8. ISCA: cabecalho header-only NAO precisa estar no meson. A regua e a
    #    biblioteca de contratos sao header-only de proposito, e exigir
    #    referencia direta acusaria o desenho correto.
    caso(8, "cabecalho header-only acusado como orfao",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "lib/measurement/tail.hpp": "#pragma once\n"}, False)

    # 9. ISCA: comando SEM `./` e ferramenta de sistema, nao promessa de caminho.
    caso(9, "comando de sistema tratado como caminho",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "```bash\nmeson test -C build\n```\n"}, False)

    # 10. ISCA: documento de origem preservado fica fora -- ele cita o que era
    #     verdade no dia em que foi escrito.
    caso(10, "documento de origem preservado conferido",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "docs/origem/setup.md": "Rode `programa-que-nunca-existiu.sh`.\n"}, False)

    # 11. ISCA: nome de tópico previsto, declarado "não iniciado" na tabela do
    #     índice. Acusá-lo seria exigir que o índice só citasse o que existe.
    caso(11, "topico previsto e declarado nao iniciado foi acusado",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "| `01-nao-existe-ainda` — x | **não iniciado** |\n"}, False)

    # 12. ISCA: item de ROADMAP em aberto cita o que ainda vai ser feito.
    caso(12, "pendencia aberta do roadmap foi acusada",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "- [ ] portar `verificar-medicao.py` do projeto a montante\n"}, False)

    # 13. E o contraste que mantém a regra viva: a MESMA citação fora de
    #     pendência e fora de tabela continua sendo acusada.
    caso(13, "citacao fora de pendencia nao acusada",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "d.md": "Rode `programa-que-nao-existe.sh` agora.\n"}, True)

    # 14. ISCA: diretório existente citado como programa. `01-harness` é
    #     diretório, e a primeira versão só coletava arquivos.
    caso(14, "diretorio existente acusado como inexistente",
         {"meson.build": MESON, "docs/08/bench.cpp": "int main(){}",
          "docs/08-medicao/01-harness/README.md": "# t\n",
          "d.md": "Ver `01-harness`.\n"}, False)

    # 15. FAIL-CLOSED.
    buf = _io.StringIO()
    with redirect_stdout(buf):
        rc = verificar("/diretorio/que/nao/existe")
    if rc == 0:
        print("  AUTOTESTE 15 FALHOU: raiz inexistente saiu ZERO")
        falhas += 1

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
