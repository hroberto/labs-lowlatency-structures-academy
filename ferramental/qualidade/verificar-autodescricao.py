#!/usr/bin/env python3
"""Confere afirmações que o material faz SOBRE SI MESMO.

POR QUE ISTO EXISTE

As afirmações mais fáceis de deixar envelhecer são as que o projeto faz a
respeito do próprio projeto. Elas não saem de medição nenhuma, ninguém as
reexecuta, e o texto continua dizendo o que era verdade no dia em que foi
escrito.

Este verificador nasceu de um defeito desta árvore, em 2026-09-22. Uma varredura
manual de pendências procurou por `graficos` e devolveu **zero arquivos** --
porque o texto diz `gráficos`, com acento. A varredura só não passou como
completa porque foi conferida duas vezes à mão. No mesmo dia, o `ROADMAP.md`
afirmava que os módulos "entram aqui quando reconciliados com a norma" **depois**
de eles terem sido reconciliados, e o `README.md` dizia "os outros oito módulos",
quando são dez.

Nenhum dos três é detectável por quem lê: todos exigem cruzar o texto com o
disco. As três contas são triviais para um programa e improváveis para uma
pessoa.

AS REGRAS

  1. CENSO DA NORMA -- "N seções, M partes" contra os títulos do documento.
  2. CENSO DA TRILHA -- "N tópicos previstos, M escrito" contra os diretórios de
     módulo e as tabelas de tópico dentro deles.
  3. CENSO DOS VERIFICADORES -- "os N têm autoteste" contra os arquivos que de
     fato têm `--autoteste`.
  4. RÓTULO DE ESTADO -- o que a seção 5 da norma exige: um documento que se
     declara `não iniciado` não pode ter campanha arquivada embaixo, e um que se
     declara `medido` tem de ter.
  5. CENSO DA SUÍTE -- "N testes" contra a introspecção do Meson, quando há
     diretório de build configurado.

NUMERAL POR EXTENSO CONTA

A norma escreve "os **seis** têm autoteste", não "os 6". Uma regra que só casasse
dígito passaria por cima da metade das afirmações deste repositório -- e seria
verde sem verificar, que é o defeito que ela existe para impedir.

O QUE ELE DELIBERADAMENTE NÃO PEGA

- afirmação sobre projeto de terceiro: "o DPDK Academy tem 16 verificadores" não
  é derivável deste disco, e inventar uma leitura remota seria pior que a lacuna;
- número ATRIBUÍDO à grandeza errada. Se o texto disser "36 partes, 7 seções", a
  regra 1 acusa; se disser "20 testes" onde 20 é o número de arquivos, ela não
  tem como saber. Esta regra fecha a classe "contagem que envelheceu", não a
  classe "contagem trocada de lugar";
- prosa que descreve estado sem o rótulo da seção 5. A regra 4 confere o rótulo,
  e só o rótulo, porque ele é a forma que a norma tornou obrigatória;
- contagem envelhecida numa frase onde o valor CORRETO aparece por outro motivo.
  A regra do *registro de mudança* a isenta, e isso já aconteceu aqui: a frase
  "a suíte tem 20 testes [...] reporta 1 de 22" foi aceita porque continha 22.
  É o preço de não acusar o histórico, e o preço é falso NEGATIVO -- a contagem
  velha sozinha continua sendo pega.
"""
import json
import os
import re
import sys

IGNORAR = {".git", "build", "builddir", "subprojects", "__pycache__", "temp"}
ORIGEM_PRESERVADA = "origem"

EXTENSO = {
    "um": 1, "uma": 1, "dois": 2, "duas": 2, "três": 3, "tres": 3, "quatro": 4,
    "cinco": 5, "seis": 6, "sete": 7, "oito": 8, "nove": 9, "dez": 10,
    "onze": 11, "doze": 12, "treze": 13, "catorze": 14, "quinze": 15,
    "dezesseis": 16, "vinte": 20, "trinta": 30,
    "one": 1, "two": 2, "three": 3, "four": 4, "five": 5, "six": 6, "seven": 7,
    "eight": 8, "nine": 9, "ten": 10, "eleven": 11, "twelve": 12,
    "thirteen": 13, "fourteen": 14, "fifteen": 15, "sixteen": 16,
    "twenty": 20, "thirty": 30,
}


def numero(texto):
    """"seis" e "6" viram 6; qualquer outra coisa vira None."""
    t = texto.strip().lower().replace("*", "").replace(",", "").replace(".", "")
    if t.isdigit():
        return int(t)
    return EXTENSO.get(t)


# FRONTEIRA DE PALAVRA OBRIGATÓRIA, e ela custou um caso do autoteste.
#
# Sem `\b`, o intervalo guloso entre os dois números de uma frase engolia parte
# da palavra seguinte: em "trinta seções, sete partes" o segundo número era
# capturado como `e` -- resto de "sete" --, `numero("e")` devolvia None, e o
# casamento era DESCARTADO por ter menos números do que a regra pedia.
#
# O efeito não era acusação falsa: era a regra **não disparar**, em silêncio,
# sobre uma frase que ela existia para conferir. Verificação que não dispara é
# indistinguível de verificação ausente, e aqui ela ainda imprimia "0
# problema(s)".
NUM = r"(\*\*)?\b([0-9]+|[A-Za-zÀ-ÿ]+)\b(\*\*)?"


def frase(texto, inicio, fim):
    """A frase que contém o trecho, de ponto a ponto."""
    ini = texto.rfind(".", 0, inicio) + 1
    f = texto.find(".", fim)
    return texto[ini: f if f != -1 else len(texto)]


def registro_de_mudanca(texto, inicio, fim, corretos):
    """A frase diz o ANTES e o DEPOIS? Então não é contagem envelhecida.

    O `ROADMAP.md` escreve "33 seções e 6 partes viraram 36 e 7" -- um registro
    de mudança, não uma afirmação sobre o estado atual. Acusá-lo seria pedir que
    o histórico mentisse sobre o passado para agradar ao verificador, e um
    verificador que acusa o que está correto ensina a ser ignorado.

    O critério é estreito de propósito: os valores CORRETOS têm de aparecer na
    MESMA frase. Uma contagem velha sozinha continua sendo acusada.
    """
    contexto = frase(texto, inicio, fim)
    return all(re.search(rf"\b{v}\b", contexto) for v in corretos)


def achar(texto, padrao, quantos):
    """Casamentos em que TODOS os `quantos` números foram legíveis.

    A exigência de quantidade não é defensiva: a primeira versão devolvia o que
    conseguisse ler, e uma regra que espera dois números recebia um e estourava
    no índice. Pior que estourar seria não estourar -- meio casamento produz
    comparação contra número errado, e isso sai como acusação falsa.
    """
    fora = []
    for m in re.finditer(padrao, texto, re.I):
        vals = [numero(g) for g in m.groups() if g and g != "**"]
        vals = [v for v in vals if v is not None]
        if len(vals) == quantos:
            fora.append((vals, m.group(0).replace("\n", " "), m.start(), m.end()))
    return fora


def documentos(raiz):
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        if ORIGEM_PRESERVADA in os.path.normpath(base).split(os.sep):
            continue
        for nome in sorted(arquivos):
            if nome.endswith(".md"):
                yield os.path.join(base, nome)


# --------------------------------------------------------------------------
def censo_norma(raiz):
    """(seções, partes) da norma, lidos do próprio documento."""
    caminho = os.path.join(raiz, "docs", "padrao-do-projeto.md")
    if not os.path.isfile(caminho):
        return None
    t = open(caminho, encoding="utf-8").read()
    return (len(re.findall(r"^## \d+\.", t, re.M)),
            len(re.findall(r"^# Parte ", t, re.M)))


def censo_trilha(raiz):
    """(tópicos previstos, tópicos escritos, módulos)."""
    docs = os.path.join(raiz, "docs")
    if not os.path.isdir(docs):
        return None
    modulos = sorted(d for d in os.listdir(docs)
                     if re.match(r"^\d\d-", d) and os.path.isdir(os.path.join(docs, d)))
    previstos = escritos = 0
    for m in modulos:
        indice = os.path.join(docs, m, "README.md")
        if os.path.isfile(indice):
            # Linha de tabela cujo primeiro campo é um tópico em crase.
            previstos += len(re.findall(r"^\| \[?`[^`]+`\]?[^|]*\|", 
                                        open(indice, encoding="utf-8").read(), re.M))
        for sub in sorted(os.listdir(os.path.join(docs, m))):
            if re.match(r"^\d\d-", sub) and os.path.isfile(
                    os.path.join(docs, m, sub, "README.md")):
                escritos += 1
    return previstos, escritos, len(modulos)


def censo_verificadores(raiz):
    """(quantos existem, quantos têm --autoteste)."""
    pasta = os.path.join(raiz, "ferramental", "qualidade")
    if not os.path.isdir(pasta):
        return None
    arquivos = sorted(f for f in os.listdir(pasta)
                      if f.startswith("verificar-") and f.endswith(".py"))
    com_autoteste = [f for f in arquivos
                     if "--autoteste" in open(os.path.join(pasta, f), encoding="utf-8").read()]
    return len(arquivos), len(com_autoteste)


def censo_suite(raiz):
    """Número de testes registrados, ou None se não houver build configurado."""
    for d in sorted(os.listdir(raiz)) if os.path.isdir(raiz) else []:
        intro = os.path.join(raiz, d, "meson-info", "intro-tests.json")
        if d.startswith("build") and os.path.isfile(intro):
            return len(json.load(open(intro, encoding="utf-8")))
    return None


# --------------------------------------------------------------------------
def verificar(raiz="."):
    if not os.path.isdir(raiz):
        print(f"ERRO: raiz inexistente: {raiz}")
        return 1

    problemas = 0
    conferidas = 0
    norma = censo_norma(raiz)
    trilha = censo_trilha(raiz)
    verif = censo_verificadores(raiz)
    suite = censo_suite(raiz)

    for caminho in documentos(raiz):
        texto = open(caminho, encoding="utf-8").read()

        # --- regra 1: censo da norma -------------------------------------
        if norma:
            for vals, trecho, ini, fim in achar(
                    texto, NUM + r"\s+(?:seções|secoes|sections)[^.\n]{0,12}?" + NUM +
                    r"\s+(?:partes|parts)", 2):
                conferidas += 1
                if tuple(vals[:2]) != norma and not registro_de_mudanca(
                        texto, ini, fim, norma):
                    print(f"  {caminho}: diz {vals[0]} seções e {vals[1]} partes; "
                          f"a norma tem {norma[0]} e {norma[1]} -- \"{trecho}\"")
                    problemas += 1

        # --- regra 2: censo da trilha ------------------------------------
        if trilha:
            previstos, escritos, _ = trilha
            for vals, trecho, ini, fim in achar(
                    texto, NUM + r"\s+(?:tópicos?|topics?)\s+(?:previstos?|planned)?[^.\n]{0,24}?"
                    + NUM + r"\s+(?:escritos?|written)", 2):
                conferidas += 1
                if (vals[0], vals[1]) != (previstos, escritos) and not registro_de_mudanca(
                        texto, ini, fim, (previstos, escritos)):
                    print(f"  {caminho}: diz {vals[0]} previstos e {vals[1]} escrito(s); "
                          f"o disco tem {previstos} e {escritos} -- \"{trecho}\"")
                    problemas += 1
            for vals, trecho, ini, fim in achar(
                    texto, NUM + r"\s+(?:tópico|topic)\s+(?:de|out of)\s+" + NUM, 2):
                conferidas += 1
                if (vals[0], vals[1]) != (escritos, previstos) and not registro_de_mudanca(
                        texto, ini, fim, (escritos, previstos)):
                    print(f"  {caminho}: diz {vals[0]} de {vals[1]}; o disco tem "
                          f"{escritos} de {previstos} -- \"{trecho}\"")
                    problemas += 1

        # --- regra 3: censo dos verificadores ----------------------------
        if verif:
            _, com_autoteste = verif
            for vals, trecho, ini, fim in achar(
                    texto, r"os\s+" + NUM + r"\s+têm\s+autoteste|all\s+" + NUM +
                    r"\s+have\s+a\s+self-test", 1):
                conferidas += 1
                if vals[0] != com_autoteste and not registro_de_mudanca(
                        texto, ini, fim, (com_autoteste,)):
                    print(f"  {caminho}: diz que {vals[0]} têm autoteste; "
                          f"{com_autoteste} têm -- \"{trecho}\"")
                    problemas += 1

        # --- regra 5: censo da suíte -------------------------------------
        if suite is not None:
            for vals, trecho, ini, fim in achar(texto, NUM + r"\s+(?:testes|tests)\b", 1):
                # Só conta afirmação sobre a SUÍTE, não sobre teste em geral --
                # e o contexto é a FRASE, não o trecho casado.
                #
                # A primeira versão procurava "suíte" dentro de `trecho`, que é
                # só "20 testes" e nunca contém a palavra. A regra ficou morta:
                # ela nunca disparou, e `ROADMAP.md` passou com "a suíte tem 20
                # testes" enquanto o Meson registrava 22. Regra que não dispara
                # imprime o mesmo "0 problema(s)" de uma árvore correta.
                if not re.search(r"su[ií]te|suite", frase(texto, ini, fim), re.I):
                    continue
                conferidas += 1
                if vals[0] != suite and not registro_de_mudanca(
                        texto, ini, fim, (suite,)):
                    print(f"  {caminho}: diz {vals[0]} testes na suíte; "
                          f"o Meson registra {suite} -- \"{trecho}\"")
                    problemas += 1

    # --- regra 4: rótulo de estado contra o disco -------------------------
    for caminho in documentos(raiz):
        texto = open(caminho, encoding="utf-8").read()
        pasta = os.path.dirname(caminho)
        tem_campanha = any(
            "metadata.json" in arqs
            for _, _, arqs in os.walk(pasta) if True)
        declara_nao_iniciado = re.search(
            r"\*\*Estado:\s*n[ãa]o iniciado|\*\*State:\s*not started", texto)
        declara_medido = re.search(
            r"\*\*Estado:\s*medido|\*\*State:\s*measured", texto)
        if declara_nao_iniciado and tem_campanha:
            conferidas += 1
            print(f"  {caminho}: declara 'não iniciado' e tem campanha arquivada embaixo")
            problemas += 1
        if declara_medido:
            conferidas += 1
            if not tem_campanha:
                print(f"  {caminho}: declara 'medido' e não há campanha arquivada embaixo")
                problemas += 1

    extra = "" if suite is not None else " (censo da suíte PULADO: sem build configurado)"
    print(f"\n  {conferidas} autodescrição(ões) conferida(s), {problemas} problema(s){extra}")
    return problemas


# --------------------------------------------------------------------------
def autoteste():
    import io as _io
    import tempfile
    from contextlib import redirect_stdout

    falhas = 0
    NORMA = "\n".join([f"## {i}. secao" for i in range(1, 4)] + ["# Parte I — a"])

    def arvore(raiz, arquivos):
        for nome, conteudo in arquivos.items():
            caminho = os.path.join(raiz, nome)
            os.makedirs(os.path.dirname(caminho), exist_ok=True)
            open(caminho, "w", encoding="utf-8").write(conteudo)

    def caso(n, descricao, arquivos, deve_acusar):
        nonlocal falhas
        with tempfile.TemporaryDirectory() as raiz:
            arvore(raiz, arquivos)
            buf = _io.StringIO()
            with redirect_stdout(buf):
                rc = verificar(raiz)
            if (rc > 0) != deve_acusar:
                print(f"  AUTOTESTE {n} FALHOU ({descricao}): "
                      f"{'acusou' if rc > 0 else 'nao acusou'}")
                print(buf.getvalue())
                falhas += 1

    # 1. censo da norma correto: 3 seções, 1 parte.
    caso(1, "censo correto da norma acusado",
         {"docs/padrao-do-projeto.md": NORMA, "d.md": "Tem 3 seções, 1 partes.\n"}, False)

    # 2. o defeito real: a contagem envelheceu.
    caso(2, "censo da norma envelhecido nao acusado",
         {"docs/padrao-do-projeto.md": NORMA, "d.md": "Tem 36 seções, 7 partes.\n"}, True)

    # 3. NUMERAL POR EXTENSO, que é como a norma deste projeto escreve.
    caso(3, "numeral por extenso nao conferido",
         {"docs/padrao-do-projeto.md": NORMA, "d.md": "São trinta seções, sete partes.\n"}, True)

    # 4. censo da trilha: um módulo com dois tópicos previstos e um escrito.
    trilha_ok = {
        "docs/01-memoria/README.md": "| `01-a` — a | nao iniciado |\n| `02-b` — b | nao iniciado |\n",
        "docs/01-memoria/01-a/README.md": "# a\n",
    }
    caso(4, "censo correto da trilha acusado",
         dict(trilha_ok, **{"d.md": "São 2 tópicos previstos, 1 escrito.\n"}), False)
    caso(5, "censo da trilha errado nao acusado",
         dict(trilha_ok, **{"d.md": "São 31 tópicos previstos, 1 escrito.\n"}), True)

    # 6. a forma curta, "1 tópico de 31", que o README usa.
    caso(6, "forma curta do censo nao conferida",
         dict(trilha_ok, **{"d.md": "**1 tópico de 31**.\n"}), True)

    # 7. rótulo de estado: declara medido sem campanha.
    caso(7, "declara medido sem campanha, nao acusado",
         {"m/README.md": "**Estado: medido.**\n"}, True)

    # 8. declara medido COM campanha: legítimo.
    caso(8, "declara medido com campanha, acusado",
         {"m/README.md": "**Estado: medido.**\n",
          "m/bench/medicoes/historico/2026-01-01-c/metadata.json": "{}"}, False)

    # 9. declara não iniciado e tem campanha embaixo: o inverso do 7.
    caso(9, "declara nao iniciado com campanha, nao acusado",
         {"m/README.md": "**Estado: não iniciado.**\n",
          "m/bench/medicoes/historico/2026-01-01-c/metadata.json": "{}"}, True)

    # 10. ISCA: documento de origem preservado fica fora -- ele carrega o censo
    #     que era verdade no dia em que foi escrito, e corrigi-lo o
    #     descaracterizaria.
    caso(10, "documento de origem preservado conferido",
         {"docs/padrao-do-projeto.md": NORMA,
          "docs/origem/setup.md": "Tem 99 seções, 9 partes.\n"}, False)

    # 11. ISCA: "999 testes" fora de contexto de suíte não é afirmação sobre a
    #     suíte.
    SUITE = {"build-x/meson-info/intro-tests.json": "[{},{},{}]"}
    caso(11, "numero de teste fora de contexto de suite acusado",
         dict(SUITE, **{"d.md": "Escrevi 999 testes na minha vida.\n"}), False)

    # 11a. E o caso que a regra existia para pegar, e não pegava: a contagem da
    #      suíte envelhecida. A primeira versão procurava "suíte" DENTRO do
    #      trecho casado, que é só "20 testes" -- a regra nunca disparava.
    caso("11a", "contagem da suite envelhecida nao acusada",
         dict(SUITE, **{"d.md": "A suíte tem 20 testes verdes.\n"}), True)

    # 11b. A mesma frase com o número certo passa.
    caso("11b", "contagem correta da suite acusada",
         dict(SUITE, **{"d.md": "A suíte tem 3 testes verdes.\n"}), False)

    # 12. FAIL-CLOSED: raiz inexistente acusa.
    buf = _io.StringIO()
    with redirect_stdout(buf):
        rc = verificar("/diretorio/que/nao/existe")
    if rc == 0:
        print("  AUTOTESTE 12 FALHOU: raiz inexistente saiu ZERO")
        falhas += 1

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
