#!/usr/bin/env python3
"""Compõe `metadata.json` e a tabela publicável de uma campanha de medição.

POR QUE ESTE PROGRAMA EXISTE

A norma (seções 28 e 29) exige que cada número publicado permita responder: qual
CPU, qual CCD, qual kernel, qual compilador, quais flags, quantas execuções,
qual configuração de build, qual commit, e **quanta variação houve entre
execuções**. Escrever isso à mão, por medição, é o defeito que `ambiente.sh`
existe para evitar -- só deslocado um passo: a descrição volta a divergir, agora
entre a tabela e o ambiente.

Então a campanha gera as duas coisas do MESMO material: as repetições do
programa e a coleta de ambiente.

A VARIAÇÃO ENTRE EXECUÇÕES É O CAMPO QUE MAIS IMPORTA

Uma tabela de percentis de uma única execução não distingue a cauda da estrutura
da cauda da máquina. O DPDK Academy registrou o caso: o critério de validade
estava publicado e a calibração dele não estava em lugar nenhum. Aqui a
campanha roda o experimento inteiro várias vezes e publica a amplitude relativa
de cada métrica entre execuções.

Uso:
    compor-medicao.py <diretorio-da-campanha> <ambiente.json> <r0.csv> [r1.csv ...]
"""
import json
import os
import statistics
import subprocess
import sys

# Métricas que entram na tabela publicável, na ordem em que saem.
#
# A lista é explícita, e não "tudo o que o CSV traz", porque a tabela é
# EDITORIAL: publicar toda métrica coletada produz tabela que ninguém lê, e a
# norma pede o que sustenta a conclusão do tópico.
#
# A TABELA SAI NOS DOIS IDIOMAS, e não porque um verificador exige: ela entra
# nos dois READMEs do tópico, e a seção 1 da norma proíbe que a versão inglesa
# seja resumida. Uma tabela publicada só em português tornaria o README inglês
# uma versão reduzida do outro por construção.
#
# O que NÃO ganha par é o resto da campanha -- `ambiente.md`, `r0.csv` --, que é
# registro gerado de máquina, e registro de máquina não tem idioma.
PUBLICAVEIS = [
    ("clock_read", "median", "ler o relógio (mediana)", "reading the clock (median)", "ns"),
    ("clock_read", "minimum", "ler o relógio (mínimo)", "reading the clock (minimum)", "ns"),
    ("record_sample", "median", "registrar amostra (mediana)", "recording a sample (median)", "ns"),
    ("per_op_floor", "p50", "piso por operação p50", "per-operation floor p50", "ns"),
    ("per_op_floor", "p99", "piso por operação p99", "per-operation floor p99", "ns"),
    ("per_op_floor", "p999", "piso por operação p99,9", "per-operation floor p99.9", "ns"),
    ("per_op_floor", "maximum", "piso por operação máximo", "per-operation floor maximum", "ns"),
]

CABECALHO = {
    "pt": "| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |",
    "en": "| Metric | Median across runs | Spread across runs | Unit |",
}


def ler_csv(caminho):
    """CSV -> {(braco, metrica): (valor, unidade)}."""
    fora = {}
    with open(caminho, encoding="utf-8") as f:
        cabecalho = next(f).strip()
        if cabecalho != "arm,metric,value,unit":
            sys.exit(f"ERRO: {caminho} nao tem o cabecalho esperado: {cabecalho!r}")
        for n, linha in enumerate(f, start=2):
            linha = linha.strip()
            if not linha:
                continue
            campos = linha.split(",")
            if len(campos) != 4:
                sys.exit(f"ERRO: {caminho}:{n} tem {len(campos)} campos, esperados 4")
            braco, metrica, valor, unidade = campos
            fora[(braco, metrica)] = (float(valor), unidade)
    return fora


def variacao(valores):
    """Amplitude relativa entre execuções, em porcento da mediana.

    Não é desvio padrão: com três a cinco execuções, o desvio padrão de uma
    amostra tão pequena informa menos que a amplitude, e a amplitude é o que o
    leitor precisa para saber se a diferença que ele vê entre duas tabelas cabe
    dentro do ruído da máquina.
    """
    if not valores:
        return None
    mediana = statistics.median(valores)
    if mediana == 0:
        return {"min": min(valores), "max": max(valores), "mediana": mediana,
                "amplitude_pct": None}
    return {
        "min": min(valores),
        "max": max(valores),
        "mediana": mediana,
        "amplitude_pct": 100.0 * (max(valores) - min(valores)) / mediana,
    }


def comando(args):
    try:
        return subprocess.run(args, capture_output=True, text=True,
                              check=False).stdout.strip() or None
    except OSError:
        return None


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    destino, ambiente_json, *csvs = sys.argv[1:]

    with open(ambiente_json, encoding="utf-8") as f:
        ambiente = json.load(f)

    execucoes = [ler_csv(c) for c in csvs]
    chaves = sorted({k for e in execucoes for k in e})

    # Uma métrica que não aparece em TODAS as execuções é defeito de coleta, e
    # não dado faltante: seguir com ela produziria variação calculada sobre
    # subconjunto sem dizer qual.
    for k in chaves:
        faltando = [c for c, e in zip(csvs, execucoes) if k not in e]
        if faltando:
            sys.exit(f"ERRO: a metrica {k} falta em {faltando} -- coleta incompleta")

    metricas = {}
    for braco, metrica in chaves:
        valores = [e[(braco, metrica)][0] for e in execucoes]
        unidade = execucoes[0][(braco, metrica)][1]
        metricas[f"{braco}.{metrica}"] = {"unidade": unidade, "por_execucao": valores,
                                          **(variacao(valores) or {})}

    # `parameters.*` em inglês: o CSV é SAÍDA DE PROGRAMA, e a seção 3 da norma
    # põe saída de programa em inglês junto com os identificadores. A estrutura
    # do `metadata.json`, essa, segue o idioma do ferramental que a compõe --
    # como `ambiente.json`, que é português.
    params = {k.split(".")[1]: v["mediana"] for k, v in metricas.items()
              if k.startswith("parameters.")}

    meta = {
        "campanha": os.path.basename(os.path.abspath(destino)),
        "programa": "trilha/08-medicao/01-harness/bench/bench_harness.cpp",
        "repeticoes": len(execucoes),
        # OS CSVs SAO LOCAIS, E O METADATA DIZ ISSO.
        #
        # Listar `r0.csv` sem mais nada faria quem clona procurar um arquivo que
        # o repositorio nao tem. A serie completa de cada metrica esta em
        # `metricas[].por_execucao`, aqui dentro -- versionar as duas coisas
        # guardaria o mesmo dado duas vezes, e a segunda em formato que ninguem
        # le.
        "saida_crua_por_execucao": {
            "arquivos": [os.path.basename(c) for c in csvs],
            "versionados": False,
            "onde_esta_o_dado": "metricas[].por_execucao, neste arquivo",
        },
        "parametros_do_experimento": params,
        "build": {
            "configuracao": os.environ.get("MEDICAO_CONFIGURACAO"),
            "compilador": os.environ.get("MEDICAO_COMPILADOR"),
            "flags_efetivas": os.environ.get("MEDICAO_FLAGS"),
        },
        "commit": comando(["git", "rev-parse", "HEAD"]),
        "arvore_suja": bool(comando(["git", "status", "--porcelain"])),
        "coletado_em": ambiente.get("coletado_em"),
        "duracao_s": float(os.environ.get("MEDICAO_DURACAO", "0")) or None,
        "metricas": metricas,
        "ambiente": ambiente,
    }

    # A PRECISÃO DO INSTRUMENTO ENTRA AQUI, E NÃO COMO NOTA DE RODAPÉ
    #
    # A norma exige "precisão e faixa do histograma" em cada medição. Este
    # projeto não usa histograma: guarda as amostras e calcula percentil exato
    # (ver lib/measurement/tail.hpp). O campo diz isso explicitamente, em vez de
    # ficar ausente -- um campo ausente seria lido como omissão, e a decisão de
    # não usar histograma é justamente o que precisa estar registrado.
    meta["precisao_do_instrumento"] = {
        "metodo": "amostras cruas em buffer pre-alocado, percentil exato",
        "perda_de_precisao": "nenhuma",
        "resolucao_do_relogio_ns": params.get("clock_period"),
        "percentil_maximo_sustentado": metricas.get(
            "per_op_floor.highest_supported", {}).get("mediana"),
    }

    os.makedirs(destino, exist_ok=True)
    caminho_meta = os.path.join(destino, "metadata.json")
    with open(caminho_meta, "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2, ensure_ascii=False)
        f.write("\n")

    # --- tabela publicável, nos dois idiomas ------------------------------
    teto = metricas.get("per_op_floor.highest_supported", {}).get("mediana")
    descartadas = metricas.get("per_op_floor.discarded", {}).get("max")
    amostras_cauda = int(metricas.get("per_op_floor.samples", {}).get("mediana") or 0)

    def tabela(idioma):
        linhas = [CABECALHO[idioma], "|---|---:|---:|---|"]
        for braco, metrica, rotulo_pt, rotulo_en, unidade in PUBLICAVEIS:
            m = metricas.get(f"{braco}.{metrica}")
            if not m:
                continue
            amp = m.get("amplitude_pct")
            rotulo = rotulo_pt if idioma == "pt" else rotulo_en
            # O separador decimal segue o idioma da tabela, como no resto do
            # material -- nas DUAS colunas numéricas. A primeira versão trocava
            # o separador do valor e esquecia o da amplitude, produzindo
            # "16,94" e "28.3%" na mesma linha.
            # `verificar-paridade.py` compara os números por forma canônica
            # justamente porque a grafia muda e o valor não.
            valor = f"{m['mediana']:.2f}"
            amp_txt = "—" if amp is None else f"{amp:.1f}%"
            if idioma == "pt":
                valor = valor.replace(".", ",")
                amp_txt = amp_txt.replace(".", ",")
            linhas.append(f"| {rotulo} | {valor} | {amp_txt} | {unidade} |")

        if idioma == "pt":
            rodape = [
                "",
                f"{len(execucoes)} execuções do mesmo experimento; "
                f"{amostras_cauda} amostras por execução no braço de cauda.",
                "",
                "A **amplitude entre execuções** é o que separa a cauda da estrutura da cauda",
                "da máquina: uma diferença menor que ela não é resultado.",
            ]
            if descartadas:
                rodape.append(
                    f"\n**{int(descartadas)} amostra(s) descartada(s)** por buffer cheio.")
            if teto is not None and teto < 0.999:
                rodape.append(
                    f"\n**O p99,9 desta tabela não é publicável:** o percentil máximo "
                    f"sustentado pela contagem de amostras é {teto:g}.")
        else:
            rodape = [
                "",
                f"{len(execucoes)} runs of the same experiment; "
                f"{amostras_cauda} samples per run in the tail arm.",
                "",
                "The **spread across runs** is what separates the structure's tail from the",
                "machine's: a difference smaller than it is not a result.",
            ]
            if descartadas:
                rodape.append(
                    f"\n**{int(descartadas)} sample(s) discarded** because the buffer was full.")
            if teto is not None and teto < 0.999:
                rodape.append(
                    f"\n**The p99.9 in this table is not publishable:** the highest percentile "
                    f"supported by the sample count is {teto:g}.")
        return "\n".join(linhas + rodape) + "\n"

    caminho_tabela = os.path.join(destino, "tabela.md")
    with open(caminho_tabela, "w", encoding="utf-8") as f:
        f.write(tabela("pt"))
    with open(os.path.join(destino, "tabela.en.md"), "w", encoding="utf-8") as f:
        f.write(tabela("en"))

    print(f"  metadata: {caminho_meta}")
    print(f"  tabela:   {caminho_tabela} (+ .en.md)")
    print(f"  metricas: {len(metricas)} em {len(execucoes)} execucao(oes)")
    if teto is not None and teto < 0.999:
        print(f"  ATENCAO: percentil maximo sustentado = {teto:g}; p99,9 nao e publicavel")
    return 0


if __name__ == "__main__":
    sys.exit(main())
