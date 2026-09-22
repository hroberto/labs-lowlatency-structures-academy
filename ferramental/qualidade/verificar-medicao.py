#!/usr/bin/env python3
"""Confere que a tabela publicada é a tabela arquivada.

POR QUE ISTO EXISTE, E POR QUE CUSTOU TRÊS VEZES

A regra da seção 8 da norma é que número publicado precisa de programa que o
produza. A campanha arquivada é o elo entre os dois -- e nada verificava que o
número no `README.md` é o número do arquivo.

Ele já divergiu três vezes nesta árvore, sempre pelo mesmo mecanismo: refazer a
campanha muda os números, e o documento continua com os antigos. Nas três a
correção foi manual, e nas três o `pre-commit` ficou verde:

  - 341,00 contra 201,00 no máximo do piso, quando o esquema do CSV mudou;
  - 16,09 contra 16,08 no relógio, quando o rodapé do compositor foi corrigido;
  - a amplitude do máximo, de 10681% para 6052%, na mesma reexecução.

Nenhuma dessas diferenças muda a conclusão do tópico. É justamente isso que as
torna perigosas: elas não incomodam ninguém, e o documento passa a afirmar um
número que nenhuma campanha da árvore produz. O ativo do projeto é a
procedência, e procedência com um dígito trocado não é procedência.

A REGRA: IGUALDADE LITERAL

Para cada campanha arquivada, a tabela dela precisa aparecer **caractere a
caractere** no README do tópico -- `tabela.md` no português, `tabela.en.md` no
inglês. Não é comparação de valores com tolerância: é igualdade de texto.

Tolerância seria o lugar errado para julgar. Quem decide se 16,08 e 16,09 são a
mesma medição é o autor, ao reescrever o texto e dizer o que mudou; o
verificador só recusa a divergência silenciosa.

O QUE ELE DELIBERADAMENTE NÃO PEGA

- número citado na PROSA e não na tabela. "o piso é de 20 ns" continua sendo
  responsabilidade de quem escreve. A tabela é o caso decidível, e é o caso em
  que a divergência é invisível;
- tabela publicada que não corresponde a campanha nenhuma -- um tópico pode ter
  tabela de outra natureza. A regra vai da campanha para o documento, não ao
  contrário;
- se a campanha mediu o que ela diz ter medido. Isso é o `metadata.json`, e é
  outro assunto.
"""
import os
import re
import sys

IGNORAR = {".git", "build", "builddir", "subprojects", "__pycache__", "temp"}


def campanhas(raiz):
    """(diretório da campanha, diretório do tópico) para cada campanha arquivada."""
    fora = []
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR and not d.startswith("build-")]
        if "tabela.md" not in arquivos:
            continue
        partes = os.path.normpath(base).split(os.sep)
        if "medicoes" not in partes:
            continue
        # .../<topico>/bench/medicoes/historico/<campanha>
        i = partes.index("medicoes")
        topico = os.sep.join(partes[: max(0, i - 1)])
        fora.append((base, topico))
    return sorted(fora)


def tabela(texto):
    """O bloco de tabela Markdown do arquivo, sem o rodapé em prosa."""
    linhas = [l for l in texto.split("\n")]
    bloco = []
    for l in linhas:
        if l.startswith("|"):
            bloco.append(l)
        elif bloco:
            break
    return "\n".join(bloco)


def verificar(raiz="."):
    if not os.path.isdir(raiz):
        print(f"ERRO: raiz inexistente: {raiz}")
        return 1

    problemas = 0
    conferidas = 0
    lista = campanhas(raiz)
    for campanha, topico in lista:
        for arq, doc in (("tabela.md", "README.md"), ("tabela.en.md", "README.en.md")):
            caminho = os.path.join(campanha, arq)
            readme = os.path.join(topico, doc)
            if not os.path.isfile(caminho):
                print(f"  {campanha}: campanha sem {arq} -- a tabela publicavel nao foi gerada")
                problemas += 1
                continue
            if not os.path.isfile(readme):
                print(f"  {topico}: campanha arquivada e nenhum {doc} que a publique")
                problemas += 1
                continue
            conferidas += 1
            esperada = tabela(open(caminho, encoding="utf-8").read())
            publicado = open(readme, encoding="utf-8").read()
            if not esperada:
                print(f"  {caminho}: nao contem bloco de tabela")
                problemas += 1
            elif esperada not in publicado:
                print(f"  {readme}: a tabela publicada NAO e a de {os.path.basename(campanha)}")
                # Diz QUAL linha divergiu: sem isso, quem le tem de comparar 15
                # linhas a olho, que e o trabalho que este verificador existe
                # para tirar de alguem.
                for linha in esperada.split("\n"):
                    if linha.startswith("|---") or linha not in publicado:
                        if not linha.startswith("|---"):
                            print(f"      falta: {linha}")
                problemas += 1

    print(f"\n  {len(lista)} campanha(s) e {conferidas} tabela(s) conferida(s), "
          f"{problemas} divergencia(s)")
    return problemas


# --------------------------------------------------------------------------
def autoteste():
    import io as _io
    import tempfile
    from contextlib import redirect_stdout

    falhas = 0
    TAB = "| Métrica | Mediana |\n|---|---:|\n| piso p50 | 20,00 |\n"
    TAB_EN = "| Metric | Median |\n|---|---:|\n| floor p50 | 20.00 |\n"

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

    base = "t/bench/medicoes/historico/2026-01-01-c/"

    # 1. a tabela publicada é a arquivada: nada a acusar.
    caso(1, "tabela igual acusada",
         {base + "tabela.md": TAB, base + "tabela.en.md": TAB_EN,
          "t/README.md": "# t\n\n" + TAB, "t/README.en.md": "# t\n\n" + TAB_EN}, False)

    # 2. O DEFEITO, três vezes real: um dígito trocado no documento.
    caso(2, "digito trocado nao acusado",
         {base + "tabela.md": TAB, base + "tabela.en.md": TAB_EN,
          "t/README.md": "# t\n\n" + TAB.replace("20,00", "21,00"),
          "t/README.en.md": "# t\n\n" + TAB_EN}, True)

    # 3. campanha arquivada e nenhum README que a publique.
    caso(3, "campanha sem README nao acusada",
         {base + "tabela.md": TAB, base + "tabela.en.md": TAB_EN}, True)

    # 4. a tabela em inglês divergindo, com a portuguesa certa.
    caso(4, "divergencia so no ingles nao acusada",
         {base + "tabela.md": TAB, base + "tabela.en.md": TAB_EN,
          "t/README.md": "# t\n\n" + TAB,
          "t/README.en.md": "# t\n\n" + TAB_EN.replace("20.00", "99.00")}, True)

    # 5. ISCA: o README pode ter MAIS que a tabela em volta dela.
    caso(5, "tabela cercada de prosa acusada",
         {base + "tabela.md": TAB, base + "tabela.en.md": TAB_EN,
          "t/README.md": "# t\n\nAntes.\n\n" + TAB + "\nDepois.\n",
          "t/README.en.md": "# t\n\nBefore.\n\n" + TAB_EN + "\nAfter.\n"}, False)

    # 6. ISCA: árvore sem campanha nenhuma não é problema -- é um projeto que
    #    ainda não mediu, e a regra 4 do autodescricao é quem cuida disso.
    caso(6, "arvore sem campanha acusada", {"t/README.md": "# t\n"}, False)

    # 7. FAIL-CLOSED.
    buf = _io.StringIO()
    with redirect_stdout(buf):
        rc = verificar("/diretorio/que/nao/existe")
    if rc == 0:
        print("  AUTOTESTE 7 FALHOU: raiz inexistente saiu ZERO")
        falhas += 1

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
