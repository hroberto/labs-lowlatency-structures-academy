<!-- cita-defeito -->
> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# Qualidade — os verificadores

<!-- Este documento REPRODUZ a conta errada que o verificador de aritmética
     pega, para explicar o que ele faz. A marca acima o isenta desse portão e
     aparece na contagem final dele: a isenção é declarada, não silenciosa. -->

Portados de `ferramental/qualidade/` do
[labs-dpdk-academy](https://github.com/hroberto/labs-dpdk-academy). **Cada um
nasceu de um defeito medido, não de teoria** — e a razão de portar em vez de
reescrever está no cabeçalho de `verificar-retratacoes.py`: no defeito que o
originou, quem escreveu a retratação sabia da regra, tinha acabado de enunciá-la,
e ainda assim deixou o valor derrubado circulando em outras páginas.
**Correção manual não escala.**

## Os quatro portados do DPDK Academy

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
| `verificar-paridade.py` | par de idiomas: existência, navegação, estrutura e números | a regra do radical idêntico existia para ser verificável e não era verificada |

O primeiro é o único dos seis que olha **código gerado**. Ele existe porque
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

**Os seis têm autoteste**, e ele verifica o próprio verificador contra casos
montados, incluindo iscas de falso positivo:

```bash
for v in links ancoras aritmetica retratacoes suposicao paridade; do
    python3 ferramental/qualidade/verificar-$v.py --autoteste
done
```

Isso importa porque um verificador que passa a aceitar tudo continua imprimindo
`N links verificados, 0 quebrados` — verde indistinguível do legítimo.

E todos **pulam com código 77** quando falta `python3`, em vez de sumir da
suíte: `pular-sem-python3.sh` existe porque a ausência desse tratamento fazia a
suíte encolher e reportar verde.

## Os dois do DPDK Academy que ainda não foram portados

| Verificador | Regra | Por que ainda não |
|---|---|---|
| `verificar-promessa.py` | todo programa citado existe na árvore e entra na compilação | 18 pontos de acoplamento com a estrutura do projeto de origem |
| `verificar-autodescricao.py` | o que o material afirma sobre si corresponde ao disco | 838 linhas, 34 pontos de acoplamento |

O segundo é o que sustenta a rotulagem de estado da norma
([seção 5](../../docs/padrao-do-projeto.md#5-rotulagem-de-estado)), e sua
ausência é dívida declarada no [ROADMAP](../../ROADMAP.md).

## O que o verificador de paridade acusou na primeira execução

A regra do **par de idiomas com radical idêntico**
([seção 2](../../docs/padrao-do-projeto.md#2-convenção-de-arquivos)) existe
justamente para ser verificável por máquina, e passou a ser. Na primeira
execução contra a árvore ele acusou dez problemas, e **oito eram dele mesmo** —
seis de número e dois de navegação:

- interpretava `.` como separador de milhar no português, o que transformava
  `15.2.0` em `1520`, `1003.1` em `10031` e `802.3` em `8023` — versão de
  compilador e identificador de norma não trocam de separador ao mudar de
  idioma. A forma canônica passou a **remover** o separador em vez de
  interpretá-lo, aceitando um falso negativo raro (`12,3` e `123` colidem) em
  troca de zero acusação falsa;
- lia a navegação de **exemplo** da [seção
  4](../../docs/padrao-do-projeto.md#4-navegação-entre-idiomas), que mora num
  bloco de código porque a norma ensina a regra mostrando-a, como se fosse a
  navegação do documento — e exigia que a norma apontasse para `README.en.md`.

Os dois defeitos viraram isca de falso positivo no autoteste. Os **dois**
achados legítimos eram os READMEs de `ferramental/` sem par em inglês, agora
escritos.

A proporção é o argumento a favor de autoteste com isca: quatro em cada cinco
acusações da primeira execução eram do verificador, e todas pareciam achados.

## O portão: `pre-commit.sh`

Roda nos dois lugares, e a razão é que cada um cobre uma lacuna que o outro não
cobre: um gancho local não roda em *pull request* de terceiro nem do Dependabot,
por construção; uma etapa de CI não impede que o commit ruim exista, só o
reprova depois.

| Passo | O que confere |
|---|---|
| sintaxe | `bash -n` em todo script e `compileall` em todo `.py` |
| documentação | os seis verificadores **e os seis autotestes** |
| segredos | chave privada e token de padrão reconhecível nos arquivos rastreados |
| pin da CI | o SHA fixado da ação ainda é o topo do major declarado no comentário |
| suíte | só no modo completo, que é o do gancho local |

O modo `--rapido` existe porque, sem ele, a CI rodaria a suíte duas vezes e uma
falha ficaria sem endereço: o vermelho apareceria antes do `meson setup`, sem
dizer qual configuração quebrou.

A conferência do pin já se pagou: ela acusou que o SHA escrito na primeira
versão do *workflow* não era o topo de `v5`.

Para instalar o gancho:

```bash
ln -sf ../../ferramental/qualidade/pre-commit.sh .git/hooks/pre-commit
```

## Navegação

- [Ferramental](../README.md)
- [Documentação e norma](../../docs/README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
