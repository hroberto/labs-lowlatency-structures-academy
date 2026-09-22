"""Primitivas de SVG, escritas à mão.

POR QUE À MÃO, E NÃO MATPLOTLIB

A seção 16 da norma declara "nenhuma dependência de sistema além do compilador,
do Meson e do Ninja". Um gráfico que exigisse matplotlib transformaria a
promessa em asterisco — e o leitor que só quer reproduzir uma medição passaria a
precisar de um ambiente Python científico para ver a figura que já está no
repositório.

SVG é texto. Escrevê-lo à mão custa estas setenta linhas, uma vez.

AS CORES VÊM DE PALETA VALIDADA, NÃO DE GOSTO

Os dois tons abaixo passaram nas verificações de separação para daltonismo
(Machado-Oliveira-Fernandes, severidade 1.0), piso de croma, faixa de
luminosidade e contraste contra a superfície — nos DOIS modos. A separação
medida foi 23,8 no claro e 25,7 no escuro, contra alvo de 8,0; o contraste passa
de 3:1 nos dois.

O tom de estado (`critico`) nunca carrega significado sozinho: todo ponto que o
usa também recebe rótulo direto.
"""

TIPOGRAFIA = "system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif"

TEMA = {
    "claro": {
        "superficie": "#fcfcfb",
        "tinta": "#0b0b0b",
        "tinta_fraca": "#52514e",
        "grade": "#e3e2df",
        "serie": "#2a78d6",
        "critico": "#d03b3b",
    },
    "escuro": {
        "superficie": "#1a1a19",
        "tinta": "#ffffff",
        "tinta_fraca": "#c3c2b7",
        "grade": "#33322f",
        "serie": "#3987e5",
        "critico": "#d03b3b",
    },
}


def escapar(t):
    return (str(t).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"))


def texto(x, y, s, cor, tam=13, peso=400, ancora="start", tabular=False):
    extra = ' font-variant-numeric="tabular-nums"' if tabular else ""
    return (f'<text x="{x:.1f}" y="{y:.1f}" fill="{cor}" font-size="{tam}" '
            f'font-weight="{peso}" text-anchor="{ancora}" '
            f'font-family="{TIPOGRAFIA}"{extra}>{escapar(s)}</text>')


def linha(x1, y1, x2, y2, cor, larg=1, tracejado=None):
    d = f' stroke-dasharray="{tracejado}"' if tracejado else ""
    return (f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" '
            f'stroke="{cor}" stroke-width="{larg}"{d}/>')


def ponto(x, y, cor, anel, r=5):
    """Marcador com anel da superfície: é o separador de 2px que impede dois
    pontos sobrepostos de virarem uma mancha só."""
    return (f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{cor}" '
            f'stroke="{anel}" stroke-width="2"/>')


def documento(larg, alt, tema, titulo, descricao, corpo):
    """O `<title>` e o `<desc>` não são enfeite: são o texto que um leitor de
    tela recebe, e a única forma de a figura não ser um buraco para quem não a
    enxerga."""
    t = TEMA[tema]
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{larg}" height="{alt}" '
            f'viewBox="0 0 {larg} {alt}" role="img" '
            f'aria-labelledby="titulo descricao">\n'
            f'<title id="titulo">{escapar(titulo)}</title>\n'
            f'<desc id="descricao">{escapar(descricao)}</desc>\n'
            f'<rect width="{larg}" height="{alt}" fill="{t["superficie"]}"/>\n'
            + corpo + "\n</svg>\n")
