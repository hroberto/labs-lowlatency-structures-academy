> 🇧🇷 Português | [🇺🇸 English](SECURITY.en.md)

# Política de segurança

## O que este projeto é, e por que isso define o escopo

**Isto é material de estudo.** Não há serviço em execução, não há dados de
usuário, não há credencial, e nada aqui é implantado em produção por este
repositório. O código existe para ser lido, compilado e medido por quem estuda.

Isso muda o que "vulnerabilidade" significa aqui. O ativo a proteger é a
**integridade e a procedência do conteúdo** — que ninguém publique como se fosse
o mantenedor, e que os números publicados correspondam aos programas que os
produzem. Não é confidencialidade: não há segredo neste repositório.

## O que está no escopo

Relate como problema de segurança:

- **número publicado sem programa que o produza, ou com procedência quebrada.**
  Este é o ativo do projeto. Uma tabela cuja campanha não está na árvore, ou um
  `metadata.json` que não corresponde ao commit que ele declara, é o defeito
  mais grave possível aqui;
- **código de exemplo que induz prática insegura sem avisar.** O material
  estuda alocadores próprios, estruturas *lock-free* e leitura de contador de
  hardware. Um trecho que leve o leitor a corromper memória ou a confiar numa
  invariante que o código não garante, *sem declarar o risco*, faz o dano
  acontecer na máquina de quem estuda;
- **script que altera o host de forma não declarada.** Nenhum script deste
  repositório exige privilégio para a trilha, e o único que pede `sudo` o faz
  para ler o firmware (`ambiente.sh --cachear-memoria`). Um script que passe a
  mudar estado do sistema sem declarar é problema de segurança, não de estilo;
- **comprometimento da cadeia de build:** dependência, ação de CI ou artefato
  que execute código não pretendido. A ação de CI é fixada por SHA e o
  `pre-commit.sh` confere que o SHA ainda é o topo do major declarado;
- **qualquer conteúdo que permita publicar em nome do mantenedor.**

## O que não está no escopo

- programa **deliberadamente** defeituoso, quando a suíte exige que ele falhe.
  O braço de controle de `bench-harness` (`--braco-de-controle`) simula coleta
  inválida de propósito, e o teste L2 exige que o programa a reprove;
- limitações declaradas: medições de uma máquina só, módulos marcados como
  esqueleto, os doze verificadores ainda não portados e listados no ROADMAP;
- relatórios automatizados de *scanner* sem análise de exploração no contexto
  deste projeto.

## Como relatar

**Abra uma issue pública** em
`https://github.com/hroberto/labs-lowlatency-structures-academy/issues`.

A escolha da issue pública em vez de canal privado é coerente com o escopo: se
o ativo é a procedência do conteúdo, a correção também é pública — uma tabela
errada se corrige publicando a correção, não em silêncio. Não há dado de usuário
a proteger enquanto a correção é preparada.

## Procedência, e o que a sustenta

| Mecanismo | O que ele garante |
|---|---|
| commits assinados, e `main` exigindo assinatura verificada | quem escreveu foi quem diz ter escrito |
| campanha de medição versionada, com `metadata.json` | o número publicado aponta para o programa, a máquina e o commit que o produziram |
| `pre-commit.sh` e CI rodando os mesmos verificadores | a consistência não depende de alguém lembrar |
| ação de CI fixada por SHA, `permissions: contents: read` | a cadeia de build não executa o que não foi revisado |

## Navegação

- [README do repositório](README.md)
- [Norma do projeto](docs/padrao-do-projeto.md)

> 🇧🇷 Português | [🇺🇸 English](SECURITY.en.md)
