> 🇧🇷 Português | [🇺🇸 English](README.en.md)

# 10 — Código gerado

> **Estado: não iniciado**, com uma peça já em uso. Ver a
> [rotulagem de estado](../padrao-do-projeto.md#5-rotulagem-de-estado).

## A pergunta do módulo

> o que o compilador faz com `custom/` que ele não faz com `std/`?

É a pergunta que os módulos de estrutura não fazem. Este eixo era o objeto
próprio do projeto absorvido (ver o [ROADMAP](../../ROADMAP.md), Etapa 1) e
sobreviveu como **módulo transversal**, em vez de ser diluído nos outros — a
seção 36 da norma o define.

## O que este módulo traz

- `constexpr`, *template*, `[[assume]]` e o deslocamento de trabalho para tempo
  de compilação como **hipótese medida**, nunca como regra de estilo;
- **varredura, não ponto único**, porque a variável sob teste interage com a
  hierarquia de memória;
- o **custo de compilar** entre as métricas.

## O que este módulo não é

Não é comparação entre compiladores, nem catálogo de *flags*. O par continua
sendo `std` e `custom`, e o compilador é o **instrumento**, não o objeto.

## A peça que já existe

[`verificar-suposicao.py`](../../ferramental/qualidade/verificar-suposicao.py)
compila uma sonda e confere **no assembly**, com braço de controle, que a
pré-condição da porta R chega ao otimizador. Ele nasceu de um defeito medido — o
Clang descartava a suposição em silêncio — e é a prova de que neste eixo a
afirmação sobre código gerado se verifica lendo código gerado.

Ele é o único dos sete verificadores que precisa de compilador, e por isso não
roda no modo rápido do
[portão](../../ferramental/qualidade/pre-commit.sh): roda na suíte, onde os
compiladores existem.

## Tópicos previstos

Nenhum ainda. Os primeiros candidatos saem do que os módulos de estrutura
produzirem: onde o `custom/` ganhar por geração de código e não por layout, o
achado vira tópico aqui.

## Navegação

- [Documentação e norma](../README.md)
- [README do repositório](../../README.md)

> 🇧🇷 Português | [🇺🇸 English](README.en.md)
