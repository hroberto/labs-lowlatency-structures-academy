#!/usr/bin/env python3
"""Recalcula percentuais que o próprio texto torna verificáveis.

POR QUE ISTO EXISTE

Um documento que escreve

    "100 dos 123 ms, 83% do custo de inicializar a EAL"

põe os três números na mesma frase: o numerador, o denominador e o resultado.
Quem lê pode conferir, e alguém confere. Em 15/09/2026 alguém conferiu: 100/123
é **81,3%**, não 83%. Os 83% saíam de 100/120,0 -- um denominador publicado em
OUTRA tabela, cuja amplitude nem continha o valor da tabela citada.

O erro sobreviveu a várias leituras porque ninguém multiplica ao ler. Esta é a
classe inteira: **conta que o texto oferece pronta para conferência e ninguém
confere.** Ela não precisa de registro nem de declaração -- os operandos estão
ali. Basta multiplicar, e computador não se cansa de multiplicar.

O QUE ELE PEGA

Frases em que numerador, denominador e percentual aparecem juntos:

    100 dos 123 ms, 83% ...        -> 100/123 = 81,3%  DIVERGE
    2 de 4 lcores, 50% ...         -> ok

O QUE ELE DELIBERADAMENTE NÃO PEGA, e vale dizer para que o verde não prometa
demais:

- percentual cujo denominador está em outra frase, outra tabela ou outro
  arquivo -- que é justamente como o defeito dos 83% NASCEU. Este verificador
  pega a inconsistência depois que ela chega à mesma frase, não a origem;
- razões "N×" entre colunas de tabela (ficam para o portão de tabelas);
- percentuais de variação ("cresce 350%"), que precisam de base declarada;
- contas com unidades diferentes nos dois lados.

TOLERÂNCIA

1,0 ponto percentual. Publicar "81%" para 81,2% é arredondamento legítimo;
publicar "83%" para 81,3% não é. A tolerância existe para não transformar
arredondamento em falha, e é folgada de propósito: verificador que acusa o
correto é desligado, e verificador desligado não protege nada.
"""
import os
import re
import sys

IGNORAR = {".git", "build", "subprojects", "__pycache__", "temp"}

TOLERANCIA_PP = 1.0

# "100 dos 123 ms, 83%" / "2 de 4 lcores, 50%" / "45 de 60 — 75%"
# O miolo entre o denominador e o percentual é curto de propósito: quanto mais
# longo, maior a chance de casar dois números que não se relacionam.
# O `(?<![A-Za-z])` no numerador existe por causa de uma isca real: "p99 de
# 800 us" casava como se 99 fosse fracao de 800, e a frase seguinte dizia
# "99% dos pacotes". Percentil nao e fracao de um todo, e o unico sinal que
# separa os dois casos e a letra colada no numero.
#
# O `0-9` no lookbehind nao e redundancia: com `(?<![A-Za-z])` apenas, o motor
# casava a partir do SEGUNDO digito de "p99" -- numerador "9", precedido de "9",
# que nao e letra e passava. Isca reencontrada depois de a primeira correcao
# parecer suficiente.
CONTA = re.compile(
    r"(?<![A-Za-z0-9])(\d+(?:[.,]\d+)?)\s+d(?:e|os|as)\s+(\d+(?:[.,]\d+)?)"
    r"[^.;:%]{0,45}?(\d+(?:[.,]\d+)?)\s*%",
    re.IGNORECASE,
)

# Valvula declarada: um documento que ENSINA sobre uma conta errada precisa
# reproduzi-la. Sem isto, o estudo que inventariou o defeito dos 83% seria
# acusado de comete-lo. A isencao e por arquivo, explicita, e aparece na
# contagem final -- ninguem a usa sem deixar rastro.
CITA = re.compile(r"<!--\s*cita-defeito\s*-->", re.IGNORECASE)


def num(txt):
    return float(txt.replace(".", "").replace(",", ".")) if txt.count(",") else float(txt)


def arquivos(raiz):
    for pasta, subpastas, nomes in os.walk(raiz):
        subpastas[:] = [d for d in subpastas if d not in IGNORAR and not d.startswith("build")]
        for nome in nomes:
            if nome.endswith(".md"):
                yield os.path.join(pasta, nome)


def verificar(raiz="."):
    conferidas = 0
    problemas = 0
    isentos = []
    for doc in sorted(arquivos(raiz)):
        try:
            texto = open(doc, encoding="utf-8").read()
        except (OSError, UnicodeDecodeError):
            continue
        if CITA.search(texto):
            isentos.append(os.path.relpath(doc, raiz))
            continue
        linhas = texto.splitlines()
        for i, linha in enumerate(linhas, 1):
            for m in CONTA.finditer(linha):
                try:
                    a, b, pct = num(m.group(1)), num(m.group(2)), num(m.group(3))
                except ValueError:
                    continue
                if b == 0 or a > b:
                    continue  # não é fração de um todo; fora do contrato
                conferidas += 1
                real = a / b * 100.0
                if abs(real - pct) > TOLERANCIA_PP:
                    print(f"  {os.path.relpath(doc, raiz)}:{i}: {m.group(1)}/{m.group(2)}"
                          f" = {real:.1f}%, mas o texto publica {m.group(3)}%")
                    print(f"      {linha.strip()[:110]}")
                    problemas += 1
    print(f"\n  {conferidas} conta(s) conferida(s); {problemas} divergente(s)"
          f" (tolerância de {TOLERANCIA_PP} ponto percentual)")
    if isentos:
        print(f"\n  {len(isentos)} documento(s) ISENTO(s) por declararem"
              f" `<!-- cita-defeito -->` -- nenhuma conta deles foi conferida:")
        for d in isentos:
            print(f"    {d}")
    return problemas


def autoteste():
    """Um caso certo, um errado e uma isca de falso positivo."""
    import io as _io
    import tempfile
    from contextlib import redirect_stdout

    def rodar(conteudo):
        with tempfile.TemporaryDirectory() as d:
            open(os.path.join(d, "t.md"), "w", encoding="utf-8").write(conteudo)
            buf = _io.StringIO()
            with redirect_stdout(buf):
                rc = verificar(d)
            return rc, buf.getvalue()

    falhas = 0

    # 1. Conta certa (arredondada dentro da tolerância): passa.
    rc, _ = rodar("# t\n\n100 dos 123 ms, 81% do custo, não são trabalho.\n")
    if rc != 0:
        print("  AUTOTESTE FALHOU: conta correta acusada"); falhas += 1

    # 2. Conta errada -- o defeito real de 15/09/2026: falha.
    rc, saida = rodar("# t\n\n100 dos 123 ms, 83% do custo, não são trabalho.\n")
    if rc != 1:
        print(f"  AUTOTESTE FALHOU: 100/123=81,3% publicado como 83% passou (rc={rc})")
        falhas += 1
    elif "81.3%" not in saida:
        print("  AUTOTESTE FALHOU: não disse qual era o valor certo"); falhas += 1

    # 3. Isca: numerador maior que o denominador não é fração de um todo.
    #    Sem esta guarda, "de 8 a 128 objetos ... 435%" viraria falso positivo.
    rc, _ = rodar("# t\n\nO modo MP/MC custa 435% a mais, de 8 para 128 objetos.\n")
    if rc != 0:
        print("  AUTOTESTE FALHOU: isca de variação percentual acusada"); falhas += 1

    # 4. Isca do percentil: "p99 de 800 us ... 99%" nao e fracao de um todo.
    rc, _ = rodar("# t\n\n**p99 de 800 µs** significa que 99% dos pacotes chegaram.\n")
    if rc != 0:
        print("  AUTOTESTE FALHOU: percentil tratado como fracao"); falhas += 1

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    sys.exit(1 if verificar(sys.argv[1] if len(sys.argv) > 1 else ".") else 0)
