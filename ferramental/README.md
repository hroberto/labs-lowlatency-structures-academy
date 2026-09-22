# Ferramental

**Isto não é material de estudo. O material depende disto para poder afirmar que
não mente.**

| Diretório | Conteúdo |
|---|---|
| [qualidade/](qualidade/) | os verificadores que rodam na suíte e na CI |

## Por que aqui, e não em `scripts/`

`scripts/` é automação que **o estudante executa**: registrar ambiente, rodar
benchmark, preparar máquina. `ferramental/` é infraestrutura que **o projeto
executa contra si mesmo**. Misturar as duas coisas fez, no projeto irmão, com
que a distinção entre "ferramenta de quem estuda" e "portão de qualidade"
desaparecesse do índice.
