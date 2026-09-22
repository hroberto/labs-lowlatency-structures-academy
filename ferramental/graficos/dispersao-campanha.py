#!/usr/bin/env python3
"""Gera o gráfico de dispersão entre execuções de uma campanha.

POR QUE ESTE GRÁFICO, E NÃO OUTRO

A tabela publicada diz "amplitude entre execuções: 10681%". O número está certo
e engana: ele não descreve variação do instrumento, e sim **quantas execuções
colheram uma interrupção**. A série é `[2264, 21, 21, 21, 1864]` — duas pegaram,
três não. Numa tabela isso é uma célula; num gráfico é óbvio ao olhar.

E o contraste com os percentis é o argumento inteiro do tópico 08.01: p50, p99 e
p99,9 saem **idênticos** nas cinco execuções, enquanto o máximo salta duas ordens
de grandeza. Uma linha reta ao lado de uma serrilhada diz isso sem prosa.

A FORMA: PEQUENOS MÚLTIPLOS, E NÃO UM GRÁFICO SÓ

Um único par de eixos com 20 ns e 2264 ns na mesma escala esconde o que importa:
os percentis viram uma linha colada no zero. Escala logarítmica resolveria o
alcance e criaria outro problema — quem lê passa a comparar distâncias que não
são distâncias.

Cada métrica ganha painel próprio, com escala própria, e os painéis
compartilham o eixo das execuções. É a forma que a literatura de visualização
chama de *small multiples*, e ela existe exatamente para comparar formatos de
série entre grandezas de escala diferente.

QUATRO ARQUIVOS POR FIGURA

Claro e escuro porque o material é lido nos dois temas do GitHub, e um SVG de
fundo branco num tema escuro é um retângulo aceso no meio do texto. Português e
inglês porque a paridade da seção 2 da norma vale para as imagens — um leitor de
inglês que recebesse rótulo em português teria uma figura pela metade.

Uso:
    dispersao-campanha.py <metadata.json> <diretorio-de-saida> <prefixo>
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import svg  # noqa: E402

# As métricas do painel, na ordem em que entram. A lista é explícita, e não
# "tudo o que o metadata traz", porque a figura é EDITORIAL: plotar toda métrica
# coletada produz parede de painéis que ninguém lê.
# O rótulo é só a métrica. Repetir "piso por operação" nas quatro linhas é
# redundância que rouba largura do painel -- e, em inglês, vazava da tela. O
# que as quatro têm em comum está no subtítulo, dito uma vez.
PAINEIS = [
    ("per_op_floor.p50", "p50", "p50"),
    ("per_op_floor.p99", "p99", "p99"),
    ("per_op_floor.p999", "p99,9", "p99.9"),
    ("per_op_floor.maximum", "máximo", "maximum"),
]

LARG, MARGEM_E, MARGEM_D, MARGEM_TEXTO = 760, 150, 70, 24
ALT_PAINEL, TOPO = 96, 84

TEXTOS = {
    "pt": {
        "titulo": "Dispersão entre execuções da campanha",
        "sub": "piso por operação, em {n} execuções do mesmo experimento — mesma máquina, nada mudou entre elas",
        "rodape": ["Os percentis saem idênticos entre execuções. O máximo é UMA amostra,",
                   "e salta conforme a execução tenha colhido uma interrupção ou não."],
        "exec": "execução",
        "amplitude": "amplitude {v}",
        "desc": "Quatro painéis, um por métrica, cada um com os valores das {n} execuções da campanha. Os painéis de p50, p99 e p99,9 mostram valores idênticos entre execuções; o painel do máximo mostra dois valores muito acima dos demais.",
    },
    "en": {
        "titulo": "Spread across the campaign's runs",
        "sub": "per-operation floor, across {n} runs of the same experiment — same machine, nothing changed",
        "rodape": ["The percentiles come out identical across runs. The maximum is ONE",
                   "sample, and it jumps depending on whether that run caught an interrupt."],
        "exec": "run",
        "amplitude": "spread {v}",
        "desc": "Four panels, one per metric, each with the values of the campaign's {n} runs. The p50, p99 and p99.9 panels show identical values across runs; the maximum panel shows two values far above the others.",
    },
}


def painel(y0, rotulo, valores, amplitude, t, idioma):
    """Um painel: rótulo à esquerda, eixo de execuções, pontos.

    O ponto que destoa recebe RÓTULO DIRETO além da cor. Cor de estado sozinha
    não carrega significado -- quem não distingue os tons continua lendo o
    número ao lado do ponto.
    """
    corpo = []
    x0, x1 = MARGEM_E, LARG - MARGEM_D
    base, topo = y0 + ALT_PAINEL - 26, y0 + 12
    lo, hi = min(valores), max(valores)
    # Série constante: uma faixa em volta do valor, para o ponto não colar na
    # borda e a reta ficar visivelmente reta.
    if hi == lo:
        lo, hi = lo * 0.9 or -1, hi * 1.1 or 1

    corpo.append(svg.texto(x0 - 14, y0 + ALT_PAINEL / 2 - 4, rotulo, t["tinta"], 13, 600, "end"))
    corpo.append(svg.texto(x0 - 14, y0 + ALT_PAINEL / 2 + 13,
                           TEXTOS[idioma]["amplitude"].format(v=amplitude),
                           t["tinta_fraca"], 11, 400, "end", tabular=True))
    corpo.append(svg.linha(x0, base + 8, x1, base + 8, t["grade"], 1))

    n = len(valores)
    mediana = sorted(valores)[n // 2]
    for i, v in enumerate(valores):
        x = x0 + (x1 - x0) * (i / max(1, n - 1))
        y = base - (base - topo) * ((v - lo) / (hi - lo))
        destoa = mediana > 0 and v > 2 * mediana
        cor = t["critico"] if destoa else t["serie"]
        corpo.append(svg.ponto(x, y, cor, t["superficie"]))
        if destoa:
            corpo.append(svg.texto(x, y - 12, f"{v:g} ns", t["tinta"], 11, 600, "middle", tabular=True))
    corpo.append(svg.texto(x1 + 10, base + 12, f"{mediana:g} ns", t["tinta_fraca"], 11, 400, "start", tabular=True))
    return "\n".join(corpo)


def gerar(meta, idioma, tema):
    t = svg.TEMA[tema]
    txt = TEXTOS[idioma]
    metricas = meta["metricas"]
    n = meta["repeticoes"]
    alt = TOPO + ALT_PAINEL * len(PAINEIS) + 62

    # Título, subtítulo e rodapé ancoram à ESQUERDA na margem do documento.
    #
    # A primeira versão os ancorava em `end` na coluna dos rótulos, como se
    # fossem rótulo de painel: um título de 37 caracteres a 17px precisa de
    # ~300px e terminava em x=176, ou seja, começava FORA da tela. Rótulo de
    # painel alinha à direita porque o eixo começa ali; texto de documento
    # alinha à esquerda porque a página começa ali.
    corpo = [svg.texto(MARGEM_TEXTO, 34, txt["titulo"], t["tinta"], 17, 600, "start"),
             svg.texto(MARGEM_TEXTO, 54, txt["sub"].format(n=n), t["tinta_fraca"], 12, 400, "start")]

    for i, (chave, rot_pt, rot_en) in enumerate(PAINEIS):
        m = metricas[chave]
        amp = m.get("amplitude_pct")
        amp_txt = "—" if amp is None else (f"{amp:.1f}%".replace(".", ",") if idioma == "pt"
                                           else f"{amp:.1f}%")
        corpo.append(painel(TOPO + i * ALT_PAINEL, rot_pt if idioma == "pt" else rot_en,
                            m["por_execucao"], amp_txt, t, idioma))

    eixo_y = TOPO + ALT_PAINEL * len(PAINEIS) - 6
    x0, x1 = MARGEM_E, LARG - MARGEM_D
    for i in range(n):
        x = x0 + (x1 - x0) * (i / max(1, n - 1))
        corpo.append(svg.texto(x, eixo_y + 14, f"r{i}", t["tinta_fraca"], 11, 400, "middle"))
    corpo.append(svg.texto(x0 - 14, eixo_y + 14, txt["exec"], t["tinta_fraca"], 11, 400, "end"))
    for i, linha_rodape in enumerate(txt["rodape"]):
        corpo.append(svg.texto(MARGEM_TEXTO, alt - 28 + i * 14, linha_rodape,
                               t["tinta_fraca"], 11, 400, "start"))

    return svg.documento(LARG, alt, tema, txt["titulo"], txt["desc"].format(n=n), "\n".join(corpo))


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    caminho, destino, prefixo = sys.argv[1:]
    meta = json.load(open(caminho, encoding="utf-8"))
    os.makedirs(destino, exist_ok=True)
    escritos = []
    for idioma in ("pt", "en"):
        for tema in ("claro", "escuro"):
            sufixo = f"-{tema}{'' if idioma == 'pt' else '.en'}.svg"
            alvo = os.path.join(destino, prefixo + sufixo)
            open(alvo, "w", encoding="utf-8").write(gerar(meta, idioma, tema))
            escritos.append(alvo)
    for a in escritos:
        print(f"  {a}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
