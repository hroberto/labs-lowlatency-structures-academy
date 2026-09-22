<!-- cita-defeito -->
# Qualidade — os verificadores

<!-- Este documento REPRODUZ a conta errada que o verificador de aritmética
     pega, para explicar o que ele faz. A marca acima o isenta desse portão e
     aparece na contagem final dele: a isenção é declarada, não silenciosa. -->

Portados de `ferramental/qualidade/` do projeto irmão
[labs-dpdk-academy](https://github.com/hroberto/labs-dpdk-academy). **Cada um
nasceu de um defeito medido, não de teoria** — e a razão de portar em vez de
reescrever está no cabeçalho de `verificar-retratacoes.py`: no defeito que o
originou, quem escreveu a retratação sabia da regra, tinha acabado de enunciá-la,
e ainda assim deixou o valor derrubado circulando em outras páginas.
**Correção manual não escala.**

## Os quatro portados do projeto irmão

| Verificador | Regra | Defeito que o originou |
|---|---|---|
| `verificar-links.py` | link relativo aponta para arquivo e âncora que existem | âncora morta em título com travessão |
| `verificar-ancoras.py` | âncora de linha aponta para o trecho certo do código | número de linha envelhece em silêncio quando o código muda |
| `verificar-retratacoes.py` | valor declarado retratado não sobrevive fora do bloco que o retrata | retratação escrita, e o número derrubado continuou publicado noutra página |
| `verificar-aritmetica.py` | percentual que o texto torna conferível fecha | "100 dos 123 ms, 83%" — e 100/123 é 81,3% |

## O que nasceu aqui

| Verificador | Regra | Defeito que o originou |
|---|---|---|
| `verificar-suposicao.py` | a pré-condição da porta R chega ao otimizador em release | o Clang descartava a suposição em silêncio, porque o predicado é chamada de função |

Este é o único dos cinco que olha **código gerado**. Ele existe porque
`PERF_EXPECTS` promete duas coisas — não custar nada em release E entregar a
pré-condição ao otimizador — e a segunda pode deixar de valer sem que nada
quebre: o código continua correto, compila, e a suíte continua verde.

Ele tem **braço de controle**: a sonda é compilada com a suposição e com ela
neutralizada, e exige-se que o caminho morto desapareça na primeira e
**sobreviva** na segunda. Isso não é zelo — o braço de controle pegou os dois
defeitos da própria sonda:

1. a primeira versão tinha `main` chamando `probe(argc)`, e o GCC removia o
   caminho morto sem a suposição, por propagar a faixa de `argc`;
2. a segunda usava uma constante sentinela, e o GCC a emitiu como
   `andl $0x5EEDBEEE` + `addl $1` — a constante **menos um** — mantendo o
   desvio e escapando do detector.

Nas duas vezes ele disse "não estou medindo o que digo que meço" em vez de
reportar sucesso. A correção final procura um **símbolo externo**, que não se
reduz a aritmética e aparece no assembly de qualquer arquitetura.

## Autoteste

**Os cinco têm autoteste**, e ele verifica o próprio verificador contra casos
montados, incluindo iscas de falso positivo:

```bash
for v in links ancoras aritmetica retratacoes suposicao; do
    python3 ferramental/qualidade/verificar-$v.py --autoteste
done
```

Isso importa porque um verificador que passa a aceitar tudo continua imprimindo
`N links verificados, 0 quebrados` — verde indistinguível do legítimo.

E todos **pulam com código 77** quando falta `python3`, em vez de sumir da
suíte: `pular-sem-python3.sh` existe porque a ausência desse tratamento fazia a
suíte encolher e reportar verde.

## Os dois do irmão que ainda não foram portados

| Verificador | Regra | Por que ainda não |
|---|---|---|
| `verificar-promessa.py` | todo programa citado existe na árvore e entra na compilação | 18 pontos de acoplamento com a estrutura do projeto irmão |
| `verificar-autodescricao.py` | o que o material afirma sobre si corresponde ao disco | 838 linhas, 34 pontos de acoplamento |

O segundo é o que sustenta a rotulagem de estado da norma
([seção 5](../../docs/padrao-do-projeto.md#5-rotulagem-de-estado)), e sua
ausência é dívida declarada no [ROADMAP](../../ROADMAP.md).

## Um verificador que falta, e que o irmão não tem

A regra do **par de idiomas com radical idêntico**
([seção 2](../../docs/padrao-do-projeto.md#2-convenção-de-arquivos)) existe
justamente para ser verificável por máquina. Hoje não é verificada.
