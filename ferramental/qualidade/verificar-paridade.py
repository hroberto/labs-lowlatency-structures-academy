#!/usr/bin/env python3
"""Confere o par de idiomas: existência, navegação, estrutura e números.

POR QUE ESTE SCRIPT EXISTE

A seção 2 da norma declara que a regra do radical idêntico — `referencias.md` e
`referencias.en.md`, nunca `references.en.md` — **não é estética: é o que
permite verificar automaticamente que todo documento tem par**. A regra existia
para ser verificável e não era verificada, e a seção 31 registrava a lacuna.

A seção 1 diz que a versão inglesa **não é versão resumida**. Isso também é
verificável, e sem verificação é promessa: um documento inglês que perde uma
seção, uma tabela ou um número não avisa ninguém — ele apenas fica menor, e
quem lê só em inglês nunca descobre o que faltou.

AS CINCO REGRAS

  1. PAR. Todo `X.md` tem `X.en.md`, e todo `X.en.md` tem `X.md`.
  2. NAVEGAÇÃO (seção 4). O documento português abre e fecha com
     `> 🇧🇷 Português | [🇺🇸 English](X.en.md)`; o inglês, com
     `> [🇧🇷 Português](X.md) | 🇺🇸 English`. O alvo do link é conferido: uma
     navegação que aponta para o arquivo errado é pior que navegação ausente,
     porque parece cumprir a regra.
  3. ESTRUTURA. Mesma contagem de títulos e de blocos de código cercados. É o
     que pega a versão resumida que a seção 1 proíbe.
  4. NÚMEROS. Nenhum número publicado num idioma falta no outro. A comparação
     normaliza o separador decimal, porque o mesmo valor se escreve `99,9` em
     português e `99.9` em inglês -- e `6.827` em português é `6,827` em inglês.
     Sem essa normalização a regra acusaria todo número de todo documento, o que
     é o mesmo que não acusar nada.
  5. IMAGENS. Todo `.svg` de `imagens/` tem par `.en.svg`. A seção 31 da norma
     diz que "a paridade PT/EN vale para as imagens", e até agora essa frase não
     era verificada: um gráfico com rótulo em português entregue ao leitor de
     inglês é uma figura pela metade, e nada acusava.

O QUE ELE NÃO FAZ

Não compara prosa, e não tenta julgar tradução. A regra 3 conta estrutura, não
qualidade. E a regra 4 ignora número dentro de bloco de código, porque ali ele é
código-fonte citado, não valor publicado.

DOCUMENTO DE ORIGEM PRESERVADO

`docs/origem/` fica fora de todas as regras, e isso é decisão declarada da
norma: o documento de origem existe só em português de propósito -- é artefato
histórico preservado, não documentação viva, e traduzi-lo o transformaria em
outra coisa.
"""
import os
import re
import sys

IGNORAR = {".git", "build", "builddir", "subprojects", "temp", "__pycache__"}
ORIGEM_PRESERVADA = "origem"
# Arquivo de campanha arquivada: `bench/medicoes/`.
#
# A EXEÇÃO É DECLARADA, E O MOTIVO NÃO É "DÁ TRABALHO TRADUZIR"
#
# O que mora ali é GERADO -- `ambiente.md` sai de `scripts/ambiente.sh
# --markdown`, `r0.csv` sai do próprio programa. Exigir par em inglês de saída
# gerada significa gerar duas vezes o mesmo registro de máquina, e um registro
# de máquina não tem idioma: o modelo da CPU, o mapa de CCDs e a versão do
# kernel são os mesmos nos dois.
#
# A tabela PUBLICÁVEL da campanha é outra coisa, e continua tendo par: quem a
# gera é `scripts/compor-medicao.py`, que emite `tabela.md` e `tabela.en.md`,
# porque ela entra nos dois READMEs do tópico.
ARQUIVO_DE_MEDICAO = "medicoes"
# Arquivos que não são material do leitor, e por isso não têm par.
#
# `CLAUDE.md` é CONFIGURAÇÃO DE FERRAMENTA, endereçada ao assistente, e escrita
# em inglês por essa razão. Exigir `CLAUDE.en.md` produziria a tradução de um
# arquivo de instrução para um público que não existe -- e, pior, duas cópias
# de instrução que divergem em silêncio é um defeito maior do que a ausência de
# par.
#
# A lista é por NOME EXATO e não por padrão: uma exceção que se estende por
# padrão cresce sozinha, e a próxima isenção deve custar uma linha aqui e uma
# justificativa como esta.
SEM_PAR = {"CLAUDE.md"}

NAV_PT = re.compile(r"^>\s*🇧🇷\s*Português\s*\|\s*\[🇺🇸\s*English\]\(([^)]+)\)\s*$", re.M)
NAV_EN = re.compile(r"^>\s*\[🇧🇷\s*Português\]\(([^)]+)\)\s*\|\s*🇺🇸\s*English\s*$", re.M)
TITULO = re.compile(r"^#{1,6}\s+\S", re.M)
CERCA = re.compile(r"^\s*```", re.M)

# Número com separador de milhar e/ou decimal.
#
# O ESPAÇO TAMBÉM É SEPARADOR DE MILHAR, e ignorá-lo produziu acusação falsa.
#
# `10 000` é grafia correta em português, e o inglês do par escreve `10,000`.
# Sem a primeira alternativa abaixo, o português rendia os tokens `10` e `000`
# e o inglês rendia `10000`: três divergências onde havia zero. A saída seria
# obrigar o texto a escrever `10.000` para agradar ao verificador -- deixar uma
# convenção tipográfica decidir a prosa é o contrário do que ele serve.
#
# RISCO ACEITO: uma sequência como "seção 4 123 vezes" tem a forma de milhar
# agrupado e seria lida como `4123`. Exige grupo de exatamente três dígitos
# depois do espaço, o que é raro em prosa técnica, e o efeito é um falso
# NEGATIVO -- dois tokens virando um nos dois idiomas --, não uma acusação
# falsa.
NUMERO = re.compile(r"(?<![\w.,])(\d{1,3}(?: \d{3})+|\d+(?:[.,]\d+)*)(?![\w])")


def sem_cercas(texto):
    """Esvazia blocos de código cercados, preservando a contagem de linhas.

    Número dentro de bloco de código é código citado -- `static_assert(200.0)`
    --, não valor publicado, e a regra 4 não fala sobre ele.
    """
    saida = []
    dentro = False
    for linha in texto.split("\n"):
        if linha.lstrip().startswith("```"):
            dentro = not dentro
            saida.append("")
            continue
        saida.append("" if dentro else linha)
    return "\n".join(saida)


def normalizar(bruto):
    """`99,9` e `99.9` viram `999`; `6.827` e `6,827` viram `6827`.

    A FORMA CANÔNICA REMOVE O SEPARADOR EM VEZ DE INTERPRETÁ-LO, e a escolha é
    deliberada. A tentação é tratar `.` como milhar em português e como decimal
    em inglês, o que funciona para `6.827` e quebra para tudo o mais: `15.2.0` é
    versão de compilador, `1003.1` é a norma POSIX e `802.3` é a IEEE. Nenhum
    dos três troca de separador ao mudar de idioma, e interpretá-los como
    milhar produzia `1520`, `10031` e `8023` só no português -- seis acusações
    falsas nos dois documentos deste repositório, medidas na primeira execução
    do verificador.

    Nenhuma regra lexical distingue `6.827` (milhar) de `1003.1`
    (identificador): a diferença está no que o número SIGNIFICA, e isso o
    verificador não sabe. Então ele não tenta.

    O que se perde: `12,3` e `123` passam a ter a mesma forma canônica, e uma
    divergência dessas escapa. É falso NEGATIVO, e é a troca certa -- um
    verificador que acusa o que está correto ensina a ignorá-lo, e aí não acusa
    mais nada.
    """
    return bruto.replace(".", "").replace(",", "").replace(" ", "")


def numeros(texto):
    from collections import Counter
    return Counter(normalizar(m.group(1))
                   for m in NUMERO.finditer(sem_cercas(texto)))


def preservado(caminho):
    partes = os.path.normpath(caminho).split(os.sep)[:-1]
    return ORIGEM_PRESERVADA in partes or ARQUIVO_DE_MEDICAO in partes


def coletar_imagens(raiz):
    """(sem par em inglês, sem par em português), para `.svg` de `imagens/`."""
    pt, en = {}, {}
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        if os.path.basename(base) != "imagens":
            continue
        for nome in sorted(arquivos):
            if not nome.endswith(".svg"):
                continue
            caminho = os.path.join(base, nome)
            if nome.endswith(".en.svg"):
                en[caminho[: -len(".en.svg")]] = caminho
            else:
                pt[caminho[: -len(".svg")]] = caminho
    return pt, en


def coletar(raiz):
    pt, en = {}, {}
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        for nome in arquivos:
            if not nome.endswith(".md"):
                continue
            caminho = os.path.join(base, nome)
            if preservado(caminho):
                continue
            if nome in SEM_PAR:
                continue
            if nome.endswith(".en.md"):
                en[caminho[: -len(".en.md")]] = caminho
            else:
                pt[caminho[: -len(".md")]] = caminho
    return pt, en


def verificar(raiz="."):
    # FAIL-CLOSED: raiz inexistente não pode sair zero. "Não apurei" publicado
    # como "está tudo certo" é o defeito que os autotestes desta pasta existem
    # para nunca repetir.
    if not os.path.isdir(raiz):
        print(f"ERRO: raiz inexistente: {raiz}")
        return 1

    pt, en = coletar(raiz)
    problemas = 0

    # --- regra 1: par -----------------------------------------------------
    for radical, caminho in sorted(pt.items()):
        if radical not in en:
            print(f"  {caminho}: sem par em ingles (esperado {radical}.en.md)")
            problemas += 1
    for radical, caminho in sorted(en.items()):
        if radical not in pt:
            print(f"  {caminho}: sem par em portugues (esperado {radical}.md)")
            problemas += 1

    # --- regra 5: imagens ------------------------------------------------
    ipt, ien = coletar_imagens(raiz)
    for radical, caminho in sorted(ipt.items()):
        if radical not in ien:
            print(f"  {caminho}: imagem sem par em ingles (esperado {radical}.en.svg)")
            problemas += 1
    for radical, caminho in sorted(ien.items()):
        if radical not in ipt:
            print(f"  {caminho}: imagem sem par em portugues (esperado {radical}.svg)")
            problemas += 1

    pares = sorted(set(pt) & set(en))
    for radical in pares:
        cpt, cen = pt[radical], en[radical]
        tpt = open(cpt, encoding="utf-8").read()
        ten = open(cen, encoding="utf-8").read()

        # --- regra 2: navegação -------------------------------------------
        # FORA DAS CERCAS, e isto custou duas acusações falsas na primeira
        # execução: a própria seção 4 da norma ENSINA a navegação, e o exemplo
        # dela é um bloco ```markdown com `[🇺🇸 English](README.en.md)` dentro.
        # Lido como navegação real, o verificador exigia que
        # `padrao-do-projeto.md` apontasse para `README.en.md`. Um documento que
        # ensina a citar mostra a citação, e o verificador tem de saber a
        # diferença entre ensinar e cumprir.
        alvos_pt = NAV_PT.findall(sem_cercas(tpt))
        alvos_en = NAV_EN.findall(sem_cercas(ten))
        esperado_pt = os.path.basename(cen)
        esperado_en = os.path.basename(cpt)
        if len(alvos_pt) < 2:
            print(f"  {cpt}: navegacao portuguesa aparece {len(alvos_pt)}x; a "
                  f"secao 4 exige abrir E fechar com ela")
            problemas += 1
        if len(alvos_en) < 2:
            print(f"  {cen}: navegacao inglesa aparece {len(alvos_en)}x; a "
                  f"secao 4 exige abrir E fechar com ela")
            problemas += 1
        for alvo in alvos_pt:
            if os.path.basename(alvo) != esperado_pt:
                print(f"  {cpt}: navegacao aponta para {alvo}, nao para {esperado_pt}")
                problemas += 1
        for alvo in alvos_en:
            if os.path.basename(alvo) != esperado_en:
                print(f"  {cen}: navegacao aponta para {alvo}, nao para {esperado_en}")
                problemas += 1

        # --- regra 3: estrutura -------------------------------------------
        for rotulo, padrao in (("titulos", TITULO), ("blocos de codigo", CERCA)):
            npt = len(padrao.findall(tpt))
            nen = len(padrao.findall(ten))
            if rotulo == "blocos de codigo":
                npt, nen = npt // 2, nen // 2
            if npt != nen:
                print(f"  {radical}: {npt} {rotulo} em portugues, {nen} em ingles"
                      f" -- a secao 1 proibe versao resumida")
                problemas += 1

        # --- regra 4: números ---------------------------------------------
        npt = numeros(tpt)
        nen = numeros(ten)
        so_en = nen - npt
        so_pt = npt - nen
        if so_en:
            print(f"  {radical}: numero(s) publicado(s) so em ingles: "
                  f"{', '.join(sorted(so_en))}")
            problemas += 1
        if so_pt:
            print(f"  {radical}: numero(s) publicado(s) so em portugues: "
                  f"{', '.join(sorted(so_pt))}")
            problemas += 1

    print(f"\n  {len(pares)} par(es) de documento e {len(set(ipt) & set(ien))} "
          f"de imagem verificado(s), {problemas} problema(s)")
    return problemas


# --------------------------------------------------------------------------
def autoteste():
    import io as _io
    import tempfile
    from contextlib import redirect_stdout

    falhas = 0
    PT = ("> 🇧🇷 Português | [🇺🇸 English](d.en.md)\n\n# t\n\n"
          "Medimos 99,9 por cento.\n\n"
          "> 🇧🇷 Português | [🇺🇸 English](d.en.md)\n")
    EN = ("> [🇧🇷 Português](d.md) | 🇺🇸 English\n\n# t\n\n"
          "We measured 99.9 per cent.\n\n"
          "> [🇧🇷 Português](d.md) | 🇺🇸 English\n")

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
            acusou = rc > 0
            if acusou != deve_acusar:
                print(f"  AUTOTESTE {n} FALHOU ({descricao}): "
                      f"{'acusou' if acusou else 'nao acusou'}")
                print(buf.getvalue())
                falhas += 1

    # 1. par completo e idêntico em estrutura e números: nada a acusar.
    caso(1, "par correto", {"d.md": PT, "d.en.md": EN}, False)

    # 2. o defeito que a seção 2 nomeia: radical diferente.
    caso(2, "radical diferente nao acusado",
         {"referencias.md": PT.replace("d.en.md", "references.en.md"),
          "references.en.md": EN.replace("d.md", "referencias.md")}, True)

    # 3. par ausente.
    caso(3, "documento sem par nao acusado", {"d.md": PT}, True)

    # 4. versão resumida: o inglês perdeu uma seção.
    caso(4, "ingles resumido nao acusado",
         {"d.md": PT.replace("# t\n", "# t\n\n## extra\n"), "d.en.md": EN}, True)

    # 5. número só num idioma.
    caso(5, "numero so em portugues nao acusado",
         {"d.md": PT.replace("99,9 por cento", "99,9 por cento e 42 vezes"),
          "d.en.md": EN}, True)

    # 6. ISCA DE FALSO POSITIVO, e é a mais importante: o MESMO número escrito
    #    com separador decimal de cada idioma não é divergência. Sem a
    #    normalização, `99,9` e `99.9` seriam dois números diferentes e a regra
    #    4 acusaria todo documento do repositório -- ruído que se aprende a
    #    ignorar, que é pior que regra ausente.
    caso(6, "separador decimal tratado como divergencia",
         {"d.md": PT, "d.en.md": EN}, False)

    # 7. ISCA: separador de MILHAR também troca de lado entre os idiomas.
    caso(7, "separador de milhar tratado como divergencia",
         {"d.md": PT.replace("99,9 por cento", "6.827 linhas"),
          "d.en.md": EN.replace("99.9 per cent", "6,827 lines")}, False)

    # 8. ISCA: número dentro de bloco de código é código citado, não valor
    #    publicado, e não entra na regra 4.
    caso(8, "numero em bloco de codigo tratado como publicado",
         {"d.md": PT + "\n```cpp\nint x = 4096;\n```\n",
          "d.en.md": EN + "\n```cpp\nint x = 4096;\n```\n"}, False)

    # 9. navegação apontando para o arquivo errado: parece cumprir a regra.
    caso(9, "navegacao para o arquivo errado nao acusada",
         {"d.md": PT.replace("d.en.md", "outro.en.md"), "d.en.md": EN}, True)

    # 10. navegação só no topo: a seção 4 exige abrir E fechar.
    caso(10, "navegacao sem fechamento nao acusada",
         {"d.md": PT.replace("\n> 🇧🇷 Português | [🇺🇸 English](d.en.md)\n", "\n", 1)
                    .rstrip() + "\n",
          "d.en.md": EN}, True)

    # 11. documento de origem preservado fica FORA das regras, por decisão
    #     declarada da norma. Sem esta isca, o verificador exigiria tradução do
    #     artefato histórico -- e quem o traduzisse o descaracterizaria.
    caso(11, "documento de origem preservado exigido em ingles",
         {"docs/origem/setup.md": "# so em portugues, de proposito\n",
          "docs/d.md": PT, "docs/d.en.md": EN}, False)

    # 12a. Arquivo de campanha arquivada fica fora da regra do par, por decisão
    #      declarada: o que está lá é gerado, e registro de máquina não tem
    #      idioma. A tabela publicável, essa sim, tem par -- e a isca abaixo
    #      exige que a exceção NÃO se estenda a ela.
    caso("12a", "arquivo gerado de medicao exigido em ingles",
         {"t/bench/medicoes/historico/2026-01-01-c/ambiente.md": "# maquina\n",
          "t/bench/medicoes/historico/2026-01-01-c/tabela.md": "# tabela\n",
          "t/d.md": PT, "t/d.en.md": EN}, False)

    # 12b. ISCA NO SENTIDO CONTRÁRIO: a exceção é do diretório de medição, e não
    #      de qualquer documento que o mencione. Um README de tópico continua
    #      exigindo par, mesmo falando de medição.
    caso("12b", "README de topico dispensado do par por falar de medicao",
         {"t/README.md": PT.replace("Medimos", "As medicoes em bench/medicoes/ mostram que medimos")},
         True)

    # 12. ISCA: a navegação de EXEMPLO, dentro de bloco de código, não é a
    #     navegação do documento. A seção 4 da norma ensina a regra mostrando-a,
    #     e ler o exemplo como se fosse cumprimento produzia acusação falsa nos
    #     dois arquivos da própria norma.
    caso(12, "navegacao de exemplo em bloco de codigo lida como real",
         {"d.md": PT.replace("# t\n", "# t\n\n```markdown\n"
                             "> 🇧🇷 Português | [🇺🇸 English](README.en.md)\n```\n"),
          "d.en.md": EN.replace("# t\n", "# t\n\n```markdown\n"
                                "> [🇧🇷 Português](README.md) | 🇺🇸 English\n```\n")},
         False)

    # 12e. IMAGEM sem par em inglês. A norma promete paridade para as imagens
    #      desde o começo, e até agora a promessa não era verificada.
    caso("12e", "imagem sem par em ingles nao acusada",
         {"d.md": PT, "d.en.md": EN, "t/imagens/g-claro.svg": "<svg/>"}, True)

    # 12f. E o par completo passa.
    caso("12f", "par de imagem completo acusado",
         {"d.md": PT, "d.en.md": EN,
          "t/imagens/g-claro.svg": "<svg/>", "t/imagens/g-claro.en.svg": "<svg/>"}, False)

    # 12d. `CLAUDE.md` é configuração de ferramenta, não material do leitor, e
    #      fica fora da regra do par por decisão declarada.
    caso("12d", "CLAUDE.md exigido em ingles",
         {"CLAUDE.md": "# instrucoes\n", "d.md": PT, "d.en.md": EN}, False)

    # 12c. ISCA: o mesmo valor com separador de milhar por ESPAÇO no português e
    #      por vírgula no inglês. Foi uma acusação falsa real, no ROADMAP.
    caso("12c", "separador de milhar por espaco tratado como divergencia",
         {"d.md": PT.replace("99,9 por cento", "10 000 amostras"),
          "d.en.md": EN.replace("99.9 per cent", "10,000 samples")}, False)

    # 13. ISCA: número de VERSÃO e identificador de norma não trocam de
    #     separador entre idiomas. Interpretar `.` como milhar no português
    #     transformava `15.2.0` em `1520` e acusava o par inteiro.
    caso(13, "versao e identificador de norma lidos como milhar",
         {"d.md": PT.replace("99,9 por cento",
                             "GCC 15.2.0, Clang 21.1.8, POSIX 1003.1 e IEEE 802.3"),
          "d.en.md": EN.replace("99.9 per cent",
                                "GCC 15.2.0, Clang 21.1.8, POSIX 1003.1 and IEEE 802.3")},
         False)

    # 14. FAIL-CLOSED: raiz inexistente acusa, e não sai zero.
    buf = _io.StringIO()
    with redirect_stdout(buf):
        rc = verificar("/diretorio/que/nao/existe")
    if rc == 0:
        print("  AUTOTESTE 14 FALHOU: raiz inexistente saiu ZERO -- nao apurado"
              " publicado como aprovacao")
        falhas += 1

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
