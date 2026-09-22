#!/usr/bin/env python3
"""Verifica links relativos e âncoras de título nos documentos Markdown.

POR QUE A REGRA DE SLUG IMPORTA

O GitHub gera o identificador de uma seção com o github-slugger, cujo núcleo é:

    value.toLowerCase().replace(regex, '').replace(/ /g, '-')

O detalhe que engana: o último passo troca **cada espaço por um hífen**, sem
colapsar sequências. Um título como

    ### Nível 4 — Mempool, mbuf, ring e ciclo de dados

perde o travessão na remoção de pontuação e fica com **dois espaços seguidos**,
que viram **dois hífens**:

    nível-4--mempool-mbuf-ring-e-ciclo-de-dados
            ^^

Um validador que colapse espaços (`\\s+` -> `-`) produz um hífen só, considera o
link válido e deixa passar uma âncora morta. Este script reproduz a regra real.

O AUTOTESTE, e por que ele demorou a existir

Este era o único dos cinco verificadores sem autoteste, e é o de maior alcance:
sozinho, confere todos os links relativos e âncoras de título de todo `.md` da
árvore -- o número sai na própria execução, e por isso não está escrito aqui.
A assimetria importa: um verificador que passa a aceitar
tudo continua imprimindo "N links verificados, 0 quebrados", e esse verde é
indistinguível do verde legítimo. Quanto mais coisa ele cobre, mais cara sai a
falha silenciosa.

O caso 5 é o que justifica o arquivo inteiro: a âncora com hífen DUPLO precisa
passar e a mesma com hífen simples precisa falhar. Se alguém "simplificar" o
slug colapsando espaços, os outros casos continuam verdes e só esse acusa.
"""
import os
import re
import sys

IGNORAR = {".git", "build", "subprojects", "temp"}

# Documento preservado de origem: os links dele obedecem a uma norma ANTERIOR e
# corrigi-los destruiria a razão de ele existir -- ser auditável contra a norma
# vigente. A exclusão é por componente de caminho, declarada aqui e coberta por
# autoteste, em vez de ser uma marca dentro do arquivo: acrescentar marca ao
# arquivo já o tornaria não-intacto.
ORIGEM_PRESERVADA = "origem"
# Pontuação removida pelo github-slugger (aproximação suficiente: tudo que não é
# letra, dígito, sublinhado, espaço ou hífen). Acentos são PRESERVADOS.
PONTUACAO = re.compile(r"[^\w\s-]", re.UNICODE)


def slug(titulo):
    """Reproduz github-slugger: minúsculas, remove pontuação, cada espaço -> '-'."""
    return PONTUACAO.sub("", titulo.strip().lower()).replace(" ", "-")


def preservado(caminho):
    """Está sob um diretório de documento de origem preservado?"""
    return ORIGEM_PRESERVADA in os.path.normpath(caminho).split(os.sep)[:-1]


def sem_cercas(texto):
    """Esvazia blocos de código cercados, preservando a contagem de linhas.

    POR QUE ISTO EXISTE

    Um documento que ENSINA a citar mostra a citação:

        ```markdown
        [INTEL-ORM](../../docs/referencias.md#intel-orm)
        ```

    Aquilo é exemplo de sintaxe, não navegação -- o caminho é relativo ao
    módulo que vai citar, não ao documento que ensina. Sem esta função o
    verificador cobrava o exemplo como link real, e o documento da norma deste
    projeto acusou seis quebras que não existem.

    O mesmo vale para TÍTULO dentro de cerca: um `## Objetivos` num exemplo de
    estrutura de capítulo não cria seção nenhuma, e contá-lo como âncora faria
    o verificador ACEITAR link para âncora inexistente -- falso negativo, que é
    o defeito pior.
    """
    saida = []
    abertura = None
    for linha in texto.split("\n"):
        m = re.match(r"^\s*(```+|~~~+)", linha)
        if abertura is None:
            if m:
                abertura = m.group(1)[:3]
                saida.append("")
                continue
            saida.append(linha)
        else:
            saida.append("")
            if m and m.group(1).startswith(abertura):
                abertura = None
    return "\n".join(saida)


def coletar_ancoras(caminho):
    vistos = {}
    ancoras = []
    bruto = sem_cercas(open(caminho, encoding="utf-8").read())
    for titulo in re.findall(r"^#{1,6}\s+(.+?)\s*$", bruto, re.M):
        # títulos podem conter links; o slug usa o texto visível
        texto = re.sub(r"\[([^\]]*)\]\[[^\]]*\]", r"\1", titulo)
        texto = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", texto)
        base = slug(texto)
        # github-slugger desambigua repetições com sufixo -1, -2...
        n = vistos.get(base, 0)
        vistos[base] = n + 1
        ancoras.append(base if n == 0 else f"{base}-{n}")
    return ancoras


def verificar(raiz="."):
    ancoras = {}
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR]
        for nome in arquivos:
            if nome.endswith(".md"):
                p = os.path.normpath(os.path.join(base, nome))
                ancoras[p] = coletar_ancoras(p)  # alvo válido mesmo se preservado

    problemas = total = orfas = 0
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR]
        for nome in sorted(arquivos):
            if not nome.endswith(".md"):
                continue
            doc = os.path.join(base, nome)
            if preservado(doc):
                continue
            bruto = sem_cercas(open(doc, encoding="utf-8").read())

            # Referência órfã: `[texto][rotulo]` sem a definição `[rotulo]: url`
            # no rodapé. O GitHub não avisa — renderiza os colchetes literais no
            # meio do parágrafo, e quem escreveu só descobre relendo a página.
            # Não conta como "link quebrado" porque não chega a ser um link.
            definidas = {m.group(1).lower() for m in re.finditer(r"^\[([^\]^]+)\]:\s*\S+", bruto, re.M)}
            for rotulo in sorted({m.group(1).lower() for m in re.finditer(r"\]\[([^\]]+)\]", bruto)}):
                if rotulo not in definidas:
                    print(f"  {doc}: referência sem definição -> [{rotulo}]")
                    orfas += 1

            texto = re.sub(r"^\[[^\]]+\]:.*$", "", bruto, flags=re.M)
            for alvo in re.findall(r"\]\((?!https?:|mailto:)([^)]+)\)", texto):
                total += 1
                arquivo, _, frag = alvo.partition("#")
                destino = os.path.normpath(os.path.join(base, arquivo)) if arquivo else os.path.normpath(doc)
                if not os.path.exists(destino):
                    print(f"  {doc}: arquivo inexistente -> {alvo}")
                    problemas += 1
                elif frag and not frag.startswith("L") and destino in ancoras:
                    if frag not in ancoras[destino]:
                        parecidos = [a for a in ancoras[destino] if a.replace("--", "-") == frag.replace("--", "-")]
                        dica = f" (você quis dizer '{parecidos[0]}'?)" if parecidos else ""
                        print(f"  {doc}: âncora inexistente -> #{frag}{dica}")
                        problemas += 1
    print(f"\n  {total} links relativos verificados, {problemas} quebrados"
          f"; {orfas} referência(s) sem definição")
    return problemas + orfas


def autoteste():
    import io as _io
    import tempfile
    from contextlib import redirect_stdout

    def rodar(arqs):
        with tempfile.TemporaryDirectory() as d:
            for nome, conteudo in arqs.items():
                caminho = os.path.join(d, nome)
                os.makedirs(os.path.dirname(caminho), exist_ok=True)
                open(caminho, "w", encoding="utf-8").write(conteudo)
            buf = _io.StringIO()
            with redirect_stdout(buf):
                rc = verificar(d)
            return rc, buf.getvalue()

    falhas = 0

    def caso(numero, descricao, arqs, esperado):
        nonlocal falhas
        rc, saida = rodar(arqs)
        ok = (rc == 0) if esperado == 0 else (rc >= 1)
        if not ok:
            print(f"  AUTOTESTE {numero} FALHOU: {descricao} (rc={rc})")
            print("    " + saida.strip().replace("\n", "\n    "))
            falhas += 1

    # 1/2. Arquivo de destino: existe passa, não existe falha.
    caso(1, "link para arquivo existente acusado",
         {"a.md": "# a\n\n[b](b.md)\n", "b.md": "# b\n"}, 0)
    caso(2, "link para arquivo inexistente passou",
         {"a.md": "# a\n\n[b](b.md)\n"}, 1)

    # 3/4. Referência de rodapé: o GitHub não avisa, renderiza os colchetes.
    caso(3, "referência definida acusada",
         {"a.md": "# a\n\nveja [isto][r].\n\n[r]: https://exemplo\n"}, 0)
    caso(4, "referência sem definição passou",
         {"a.md": "# a\n\nveja [isto][r].\n"}, 1)

    # 5. O MOTIVO DO ARQUIVO. "Nível 4 — Mempool" perde o travessão e fica com
    #    dois espaços, que viram DOIS hífens. Quem colapsar produz um só, aceita
    #    a âncora morta, e só este caso acusa.
    doc = "# t\n\n## Nível 4 — Mempool\n\ntexto.\n"
    caso(5, "âncora com hífen duplo (regra real do github-slugger) acusada",
         {"a.md": doc + "\n[ir](#nível-4--mempool)\n"}, 0)
    caso(6, "âncora com hífen colapsado passou -- o slug está errado",
         {"a.md": doc + "\n[ir](#nível-4-mempool)\n"}, 1)

    # 7. Títulos repetidos: o github-slugger desambigua com sufixo -1.
    caso(7, "desambiguação de título repetido não reconhecida",
         {"a.md": "# t\n\n## Exercícios\n\n## Exercícios\n\n[ir](#exercícios-1)\n"}, 0)

    # 8. Âncora de LINHA (#L42) aponta para o GitHub, não para um título: não é
    #    verificável aqui e não pode ser acusada. Quem confere é verificar-ancoras.
    caso(8, "âncora de linha acusada como título inexistente",
         {"a.md": "# a\n\n[ir](b.md#L42)\n", "b.md": "# b\n"}, 0)

    # 9. URL externa não é alvo deste verificador: rede não entra na suíte.
    caso(9, "URL externa acusada",
         {"a.md": "# a\n\n[fora](https://exemplo/nao-existe)\n"}, 0)

    # 10. Título com link dentro: o slug usa o texto VISÍVEL, não a marcação.
    caso(10, "título com link dentro gerou slug errado",
         {"a.md": "# t\n\n## O [guia][g] oficial\n\n[ir](#o-guia-oficial)\n\n[g]: https://x\n"}, 0)

    # 11-13. Bloco de código cercado: exemplo de sintaxe, não navegação.
    caso(11, "link dentro de cerca cobrado como link real",
         {"a.md": "# a\n\n```markdown\n[x](../../nao/existe.md#foo)\n```\n"}, 0)
    caso(12, "título dentro de cerca virou âncora -- aceita link para seção inexistente",
         {"a.md": "# a\n\n```markdown\n## Objetivos\n```\n\n[ir](#objetivos)\n"}, 1)
    # ISCA DE FALSO POSITIVO: a linha seguinte ao fechamento da cerca é texto
    # normal e CONTINUA sendo verificada. Quem esvaziar até o fim do arquivo
    # passa nos casos 11 e 12 e falha neste.
    caso(13, "cerca fechada não devolveu a verificação ao texto seguinte",
         {"a.md": "# a\n\n```\ncodigo\n```\n\n[b](b.md)\n"}, 1)

    # 14-15. Documento de origem preservado.
    caso(14, "documento sob origem/ cobrado pela norma vigente",
         {"docs/origem/setup.md": "# s\n\n[x](nao-existe.md)\n"}, 0)
    # ISCA: `origem.md` é ARQUIVO, não diretório preservado. Quem casar por
    # substring do caminho passa no 14 e falha aqui.
    caso(15, "arquivo chamado origem.md tratado como preservado",
         {"origem.md": "# o\n\n[x](nao-existe.md)\n"}, 1)
    # ISCA: âncora DENTRO do documento preservado continua sendo alvo válido --
    # a norma vigente cita o documento de origem, e esse link tem de funcionar.
    caso(16, "âncora de documento preservado deixou de ser alvo válido",
         {"a.md": "# a\n\n[o](docs/origem/setup.md#titulo)\n",
          "docs/origem/setup.md": "# titulo\n"}, 0)

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
