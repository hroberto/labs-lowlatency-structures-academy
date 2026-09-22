#!/usr/bin/env python3
"""Confere que a pré-condição da porta R CHEGA ao otimizador em release.

POR QUE ESTE VERIFICADOR EXISTE

A porta R do contrato (lib/contract/contract.hpp) promete uma coisa específica:
em release, `PERF_EXPECTS(D::accepts(v))` não custa nada E entrega a
pré-condição ao otimizador, que pode usá-la para eliminar verificação redundante
a jusante. Se a suposição for descartada, o código continua CORRETO, continua
compilando, a suíte continua verde -- e a promessa central do desenho deixou de
valer sem que nada acuse.

Isso não é hipótese. MEDIDO em 16/09/2026:

    compilador   configuração              verificação redundante a jusante
    ----------   -----------------------   --------------------------------
    GCC 15.2     assume, sem gnu::const    ELIMINADA
    Clang 21.1   assume, sem gnu::const    SOBREVIVE  <-- suposição ignorada
    Clang 21.1   assume + gnu::const       ELIMINADA

O Clang trata a chamada `D::accepts(v)` como possivelmente com efeito colateral
e descarta a suposição, avisando por `-Wassume` -- um AVISO, não um erro. A
correção foi `PERF_PURE_PREDICATE` (`[[gnu::const]]`) em todo `accepts()`.

`-Werror=assume` no Clang fecha essa porta específica, mas não serve como
garantia geral: o GCC **não tem** `-Wassume` (conferido: `cc1plus: error:
'-Werror=assume': no option '-Wassume'`), e um compilador futuro pode descartar
a suposição por outro motivo, sem aviso nenhum. Este verificador olha o CÓDIGO
GERADO, então não depende de o compilador ter a gentileza de avisar.

COMO FUNCIONA

Ele compila uma sonda que usa os cabeçalhos DE VERDADE -- uma regressão nos
cabeçalhos é pega -- e cujo caminho morto devolve uma constante sentinela que
não aparece em nenhum outro lugar do programa:

    extern void caminho_morto_alcancado();

    int probe(std::uint32_t v)
    {
        PERF_EXPECTS(probe_domain::accepts(v));     // aceita [1, 10000]
        if (v == 0) { caminho_morto_alcancado(); }  // impossível pelo contrato
        return 1;
    }

Se a suposição chegou ao otimizador, o `if` é removível e **o símbolo
desaparece do assembly**.

POR QUE SÍMBOLO E NÃO CONSTANTE -- O SEGUNDO DEFEITO MEDIDO DESTE ARQUIVO

A primeira versão fazia o caminho morto devolver a constante `0x5EEDBEEF` e
procurava esse imediato no assembly. O braço de controle a reprovou, e a razão é
instrutiva: o GCC MANTEVE o desvio -- o `cmpl`/`sbbl` estava lá -- mas converteu
a seleção em aritmética sem ramo, emitindo `andl $0x5EEDBEEE` seguido de
`addl $1`. A constante nunca apareceu como imediato: apareceu **menos um**.

Procurar constante não é robusto, porque o compilador pode CALCULÁ-LA. Procurar
instrução (`cmpl`, `setae`) também não serve: são de x86, e arm64 está no escopo
declarado do projeto.

Um símbolo externo resolve as duas coisas. Chamada a função de outra unidade de
tradução não se reduz a aritmética, e nome de símbolo aparece no assembly de
qualquer arquitetura. O teste passa a ser binário de verdade: ou a chamada está
lá, ou o otimizador provou que é inalcançável.

O BRAÇO DE CONTROLE NÃO É OPCIONAL

A sonda é compilada DUAS vezes: com a suposição e com ela neutralizada. Exige-se
que a sentinela esteja AUSENTE na primeira e PRESENTE na segunda.

Sem o braço de controle este verificador seria verde por qualquer motivo -- e
não é teoria: foi ele que pegou os DOIS defeitos desta sonda, o `main` que
permitia ao GCC propagar a faixa de `argc` e a constante reduzida a aritmética.
Nas duas vezes ele disse "não estou medindo o que digo que meço" em vez de
reportar sucesso. É o protocolo de comparação simétrica da seção 11 da norma
aplicado a um verificador: uma comparação que não controla o método mede o
método.
"""

import os
import subprocess
import sys
import tempfile

# O símbolo do caminho morto. A busca por substring encontra tanto a forma crua
# quanto a decorada (`_Z25caminho_morto_alcancadov`).
SIMBOLO = "caminho_morto_alcancado"

SONDA = r"""
#include "lib/contract/domain.hpp"

#include <cstdint>
#include <string_view>

struct probe_domain : perf::contract::closed_range<std::uint32_t, 1, 10000>
{
    static constexpr std::string_view name = "sonda de suposicao";
};

// SEM `main`, E A AUSENCIA E O MECANISMO.
//
// A primeira versao desta sonda tinha `int main(int argc, char**) { return
// probe(argc); }`, e o BRACO DE CONTROLE a reprovou: o GCC removia o ramo morto
// mesmo sem a suposicao, porque `argc >= 1` e conhecido e a analise
// interprocedural propagava a faixa do argumento ate dentro de `probe`. O teste
// media a analise do GCC, nao a suposicao.
//
// Com `probe` sendo a unica funcao e tendo ligacao externa, nenhum compilador
// pode supor nada sobre `v`: a unica informacao disponivel sobre a faixa dele e
// a que o contrato declara. E por isso que so se gera ASSEMBLY, sem link --
// um binario sem `main` nao linkaria, e nao e preciso executar nada.
//
// `noinline` fica como cinto de seguranca: o corpo precisa existir como funcao
// no assembly para ser inspecionavel.
//
// `caminho_morto_alcancado` e DECLARADA E NAO DEFINIDA de proposito: sem corpo,
// o compilador nao pode provar que a chamada e inofensiva nem convertê-la em
// aritmetica. Ou ela esta no assembly, ou foi provada inalcancavel.
extern void caminho_morto_alcancado();

[[gnu::noinline]] int probe(std::uint32_t v)
{
    PERF_EXPECTS(probe_domain::accepts(v));
    if (v == 0)
    {
        caminho_morto_alcancado();
    }
    return 1;
}
"""

# Neutraliza a suposicao SEM tocar o cabecalho: e o braco de controle.
NEUTRALIZA = r"""
#undef PERF_EXPECTS
#define PERF_EXPECTS(...) ((void) 0)
"""


def compiladores():
    """Os compiladores disponíveis, entre os de referência do projeto."""
    achados = []
    for cc in ("g++", "clang++"):
        try:
            subprocess.run([cc, "--version"], capture_output=True, check=True)
            achados.append(cc)
        except (OSError, subprocess.CalledProcessError):
            pass
    return achados


def gerar_sonda(destino, com_suposicao):
    # A neutralizacao entra DEPOIS do include, senao o cabecalho a sobrescreve.
    fonte = SONDA
    if not com_suposicao:
        fonte = fonte.replace('#include "lib/contract/domain.hpp"',
                              '#include "lib/contract/domain.hpp"\n' + NEUTRALIZA)
    with open(destino, "w", encoding="utf-8") as f:
        f.write(fonte)
    return fonte


def simbolo_presente(asm):
    """O caminho morto sobreviveu no assembly?"""
    return SIMBOLO in asm


def compilar_asm(cc, fonte, raiz):
    r = subprocess.run(
        [cc, "-std=c++23", "-O2", "-DNDEBUG", "-I", raiz, "-S", "-o", "-", fonte],
        capture_output=True, text=True)
    if r.returncode != 0:
        return None, r.stderr
    return r.stdout, r.stderr


def verificar(raiz="."):
    ccs = compiladores()
    if not ccs:
        print("  PULADO - nenhum compilador de referencia (g++, clang++) disponivel.")
        print("  Esta verificacao NAO passou: ela nao rodou.")
        return 77

    problemas = 0
    with tempfile.TemporaryDirectory() as d:
        com = os.path.join(d, "com_suposicao.cpp")
        sem = os.path.join(d, "sem_suposicao.cpp")
        gerar_sonda(com, com_suposicao=True)
        gerar_sonda(sem, com_suposicao=False)

        for cc in ccs:
            asm_com, err_com = compilar_asm(cc, com, raiz)
            asm_sem, err_sem = compilar_asm(cc, sem, raiz)

            if asm_com is None or asm_sem is None:
                print(f"  {cc}: a sonda NAO COMPILOU -- a porta R nao pode ser verificada")
                print("    " + (err_com or err_sem).strip().replace("\n", "\n    ")[:600])
                problemas += 1
                continue

            # Braco de controle: sem a suposicao, o caminho morto DEVE sobreviver.
            if not simbolo_presente(asm_sem):
                print(f"  {cc}: BRACO DE CONTROLE INVALIDO -- o caminho morto desapareceu")
                print("    tambem SEM a suposicao. O otimizador o removeu por outro motivo,")
                print("    entao este teste nao mede a suposicao. Sonda precisa ser revista.")
                problemas += 1
                continue

            if simbolo_presente(asm_com):
                print(f"  {cc}: SUPOSICAO DESCARTADA -- o caminho morto sobreviveu em release.")
                print("    A porta R nao esta entregando a pre-condicao ao otimizador: ela e")
                print("    um no-op que parece funcionar. Conferir PERF_PURE_PREDICATE em")
                print("    lib/contract/contract.hpp e o `accepts()` do dominio.")
                problemas += 1
            else:
                print(f"  {cc}: suposicao confirmada"
                      " (caminho morto eliminado; o controle o mantem)")

    print(f"\n  {len(ccs)} compilador(es) conferido(s); {problemas} com a suposicao nao entregue")
    return problemas


def autoteste():
    falhas = 0

    def caso(numero, descricao, ok):
        nonlocal falhas
        if not ok:
            print(f"  AUTOTESTE {numero} FALHOU: {descricao}")
            falhas += 1

    # 1/2. O detector reconhece o simbolo cru e o decorado. Falso negativo aqui
    #      significa declarar a suposicao confirmada quando ela nao chegou.
    caso(1, "simbolo cru nao reconhecido", simbolo_presente(f"\tcall\t{SIMBOLO}\n"))
    caso(2, "simbolo decorado nao reconhecido",
         simbolo_presente(f"\tcall\t_Z25{SIMBOLO}v@PLT\n"))

    # 3. ISCA DE FALSO POSITIVO: assembly sem o simbolo nao pode acusa-lo. Um
    #    detector que casasse com qualquer `call` passaria em 1 e 2 e falharia
    #    aqui.
    caso(3, "simbolo acusado em assembly que nao o contem",
         not simbolo_presente("\tmovl\t$1, %eax\n\tretq\n\tcall\toutra_coisa\n"))

    # 4. REGRESSAO TRAVADA: o caminho morto nao pode voltar a ser constante
    #    sentinela. O GCC emitiu 0x5EEDBEEF como `andl $0x5EEDBEEE` + `addl $1`
    #    -- a constante menos um --, mantendo o desvio e escapando do detector.
    caso(4, "o caminho morto voltou a ser constante em vez de chamada externa",
         "caminho_morto_alcancado();" in SONDA and "5EEDBEEF" not in SONDA)

    # 5. A sonda do braco de controle realmente difere da outra: se a
    #    neutralizacao nao entrasse depois do include, as duas fontes seriam
    #    identicas e o verificador compararia algo consigo mesmo.
    with tempfile.TemporaryDirectory() as d:
        com = os.path.join(d, "a.cpp")
        sem = os.path.join(d, "b.cpp")
        texto_com = gerar_sonda(com, com_suposicao=True)
        texto_sem = gerar_sonda(sem, com_suposicao=False)
        caso(5, "as duas sondas sairam identicas", texto_com != texto_sem)
        caso(6, "a neutralizacao nao ficou depois do include",
             texto_sem.index("#undef PERF_EXPECTS") > texto_sem.index("domain.hpp"))
        caso(7, "a sonda com suposicao foi neutralizada por engano",
             "#undef PERF_EXPECTS" not in texto_com)
        # 8. REGRESSAO TRAVADA: a sonda nao pode ter `main`. Com ele, o GCC
        #    propaga a faixa de `argc` para dentro de `probe` e remove o ramo
        #    morto sem a suposicao -- foi o defeito que o braco de controle
        #    pegou na primeira versao deste arquivo.
        #    A guarda tem de olhar CODIGO e nao comentario: o proprio texto que
        #    explica a regressao cita `int main(int argc, char**)`, e a primeira
        #    versao desta assercao casou com o comentario e reprovou a sonda
        #    correta.
        codigo_com = "\n".join(l for l in texto_com.split("\n")
                               if not l.lstrip().startswith("//"))
        caso(8, "a sonda voltou a ter main -- a analise interprocedural falseia o teste",
             "int main(" not in codigo_com)
        # 9. ISCA DA PROPRIA GUARDA: o comentario que cita `int main(` precisa
        #    continuar la. Se ele desaparecer, a explicacao do defeito se perdeu
        #    e o proximo a editar a sonda vai reintroduzi-lo.
        caso(9, "o comentario que explica por que nao ha main desapareceu",
             "int main(" in texto_com)

        # 10. A funcao do caminho morto precisa seguir DECLARADA E NAO DEFINIDA:
        #     com um corpo, o compilador pode provar que ela e inofensiva e
        #     remover a chamada mesmo sem a suposicao, invalidando o controle.
        caso(10, "a funcao do caminho morto ganhou corpo",
             "extern void caminho_morto_alcancado();" in codigo_com
             and "void caminho_morto_alcancado()\n{" not in codigo_com)

    print(f"\n  autoteste: {falhas} assercao(oes) falharam")
    return falhas


if __name__ == "__main__":
    if "--autoteste" in sys.argv:
        sys.exit(1 if autoteste() else 0)
    rc = verificar(sys.argv[1] if len(sys.argv) > 1 else ".")
    sys.exit(77 if rc == 77 else (1 if rc else 0))
