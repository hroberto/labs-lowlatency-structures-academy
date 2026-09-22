#!/usr/bin/env python3
"""Confere se âncoras de linha nos documentos ainda apontam para o alvo certo.

POR QUE ESTE SCRIPT EXISTE

Links do tipo `arquivo.c#L227` levam o leitor direto ao código que produziu um
número — leitura muito mais fluida que "procure a função no arquivo". O preço é
que números de linha mudam quando o código muda, e um link desatualizado não
quebra: ele aponta em silêncio para o trecho errado, que é pior que apontar para
lugar nenhum.

Este verificador elimina esse risco. A convenção que ele exige é simples: o
**texto do link** deve ser o nome do símbolo, e a linha apontada deve conter
esse nome. Assim:

    [`sample_count_domain`](medicoes/contract-cost.cpp#L227)

Se `sample_count_domain` deixar de estar na linha 227, o script acusa e diz onde ela foi
parar — basta corrigir o número.

O AUTOTESTE, E POR QUE ELE FALTAVA SEM QUE NINGUÉM VISSE

Este arquivo passou meses **registrado na suíte com um autoteste que não
existia**. A entrada `docs - autoteste do verificador de ancoras` chamava
`verificar-ancoras.py --autoteste`; como não havia essa opção, `--autoteste`
virava o diretório-raiz, `os.walk` de um diretório inexistente não devolve nada,
o script imprimia "0 âncoras verificadas, 0 desatualizadas" e saía **zero**.

Um verde ligado a nada, contado entre os testes da suíte e executado pela CI. E o
`meson.build` descrevia as garantias desse autoteste em detalhe, citando uma
guarda chamada `DEPOIS_DA_PALAVRA` que **nunca existiu em código nenhum** —
apenas naquele comentário.

O defeito tem duas partes, e as duas estão corrigidas aqui:

  1. não havia `--autoteste`. Agora há, com caso para cada uma das três regras;
  2. raiz inexistente saía ZERO, que é "não apurei" publicado como "está tudo
     certo". Agora sai erro: se o caminho pedido não existe, nada foi conferido,
     e isso não é aprovação.
"""
import os
import re
import sys

IGNORAR = {".git", "build", "subprojects", "temp"}
# [`simbolo`](caminho#L123)  ou  [texto](caminho#L123-L130)
PADRAO = re.compile(r"\[`?([^\]`]+)`?\]\(([^)#]+)#L(\d+)(?:-L\d+)?\)")


def verificar(raiz="."):
    problemas = 0
    total = 0
    # FAIL-CLOSED. `os.walk` de caminho inexistente devolve lista vazia sem
    # reclamar, e o resultado saía como "0 desatualizadas" -- aprovação. Era o
    # que fazia o autoteste-fantasma passar, e seria o que faria um erro de
    # digitação no meson.build desligar a verificação de verdade.
    if not os.path.isdir(raiz):
        print(f"  raiz \"{raiz}\" nao e um diretorio; NADA foi conferido")
        return 1
    for base, dirs, arquivos in os.walk(raiz):
        dirs[:] = [d for d in dirs if d not in IGNORAR]
        for nome in sorted(arquivos):
            if not nome.endswith(".md"):
                continue
            doc = os.path.join(base, nome)
            texto = open(doc, encoding="utf-8").read()
            for simbolo, caminho, linha in PADRAO.findall(texto):
                alvo = os.path.normpath(os.path.join(base, caminho))
                total += 1
                if not os.path.exists(alvo):
                    print(f"  {doc}: alvo inexistente -> {caminho}")
                    problemas += 1
                    continue
                linhas = open(alvo, encoding="utf-8").read().split("\n")
                n = int(linha)
                if n < 1 or n > len(linhas):
                    print(f"  {doc}: linha {n} fora de {caminho} ({len(linhas)} linhas)")
                    problemas += 1
                    continue
                if simbolo not in linhas[n - 1]:
                    # onde o símbolo está de fato, para facilitar a correção
                    onde = [i + 1 for i, l in enumerate(linhas) if simbolo in l]
                    sugestao = f" (está na linha {onde[0]})" if onde else " (não encontrado no arquivo)"
                    print(f"  {doc}: '{simbolo}' NAO esta em {caminho}:{n}{sugestao}")
                    print(f"      linha {n} contem: {linhas[n - 1].strip()[:60]}")
                    problemas += 1
    print(f"\n  {total} âncoras de linha verificadas, {problemas} desatualizadas")
    return problemas


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

    def caso(numero, descricao, arqs, espera_defeito):
        nonlocal falhas
        rc, saida = rodar(arqs)
        ok = (rc >= 1) if espera_defeito else (rc == 0)
        if not ok:
            print(f"  AUTOTESTE {numero} FALHOU: {descricao} (rc={rc})")
            print("    " + saida.strip().replace("\n", "\n    "))
            falhas += 1

    CODIGO = "linha 1\nvoid m_spinlock(void)\nlinha 3\n"

    # 1/2. A regra central: o símbolo precisa estar NA linha apontada.
    caso(1, "ancora correta acusada",
         {"d.md": "# d\n\n[`m_spinlock`](c.c#L2)\n", "c.c": CODIGO}, False)
    caso(2, "ancora apontando para a linha errada passou",
         {"d.md": "# d\n\n[`m_spinlock`](c.c#L3)\n", "c.c": CODIGO}, True)

    # 3. Alvo inexistente.
    caso(3, "ancora para arquivo inexistente passou",
         {"d.md": "# d\n\n[`m_spinlock`](sumiu.c#L2)\n"}, True)

    # 4. Linha além do fim do arquivo -- o caso que aparece quando o código
    #    encolhe e o número do link fica para trás.
    caso(4, "linha fora do arquivo passou",
         {"d.md": "# d\n\n[`m_spinlock`](c.c#L999)\n", "c.c": CODIGO}, True)

    # 5/8. Faixa `#L2-L3`: o padrão aceita, e confere a PRIMEIRA linha.
    #
    #      O caso 5 sozinho NÃO BASTA, e isso foi medido: estreitando o padrão
    #      para não casar faixa, a âncora deixa de ser reconhecida, nenhum
    #      problema é relatado, e o caso 5 continua verde. "Não reconheci" e
    #      "conferi e está certo" produzem a mesma saída. O caso 8 usa uma faixa
    #      ERRADA, que só passa despercebida se o padrão tiver parado de casar.
    caso(5, "ancora de faixa correta acusada",
         {"d.md": "# d\n\n[`m_spinlock`](c.c#L2-L3)\n", "c.c": CODIGO}, False)

    # 6. Isca de falso positivo: link comum, sem `#L`, não é âncora de linha.
    caso(6, "link sem ancora de linha tratado como ancora",
         {"d.md": "# d\n\n[texto](c.c) e [outro](c.c#secao)\n", "c.c": CODIGO}, False)

    caso(8, "ancora de faixa apontando para a linha errada passou",
         {"d.md": "# d\n\n[`m_spinlock`](c.c#L3-L4)\n", "c.c": CODIGO}, True)

    # 7. FAIL-CLOSED, o defeito que este autoteste existe para nunca repetir:
    #    raiz inexistente precisa ACUSAR, e não sair zero como se estivesse tudo
    #    certo. Foi exatamente assim que este arquivo passou meses registrado na
    #    suíte sem conferir nada.
    buf = _io.StringIO()
    with redirect_stdout(buf):
        rc = verificar("/diretorio/que/nao/existe")
    if rc == 0:
        print("  AUTOTESTE 7 FALHOU: raiz inexistente saiu ZERO -- nao apurado"
              " publicado como aprovacao")
        falhas += 1

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
