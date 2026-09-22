> 🇧🇷 Português | [🇺🇸 English](ROADMAP.en.md)

# Roadmap do projeto

**Este documento é a ordem de CONSTRUÇÃO do material. Não é ordem de leitura.**

A ordem de estudo é outro documento: [`docs/plano-estudo.md`](docs/plano-estudo.md).
As duas divergem de propósito — o módulo 08 é o primeiro a construir e o
penúltimo a ler. A distinção existe porque no projeto a montante duas numerações
concorrentes conviveram no mesmo README e "nível 5" passou a significar duas
coisas diferentes conforme o documento.

> **Esqueleto.** Este documento é um esqueleto e se declara como tal. As etapas
> abaixo da 2 vêm de
> [`docs/origem/setup-lowlatency-structures-academy.md`](docs/origem/setup-lowlatency-structures-academy.md)
> e ainda não foram reconciliadas com a norma absorvida.

## Etapa 1 — Absorção da fundação · **parcial**

O `labs_cpp_performance_tuning_academy` — C++23, mesma máquina, mesmo método,
trilha não iniciada, **zero commits e sem remoto** — foi absorvido por este
repositório em 2026-09-21. Veio o que já era executável:

- [x] `git init`, licença MIT, `.gitignore`, `.gitattributes`, `.clang-format`,
      `.clang-tidy`
- [x] os **dois** documentos de origem preservados intactos em
      [`docs/origem/`](docs/origem/)
- [x] [norma](docs/padrao-do-projeto.md) — chegou com 33 seções, 6 partes e hoje
      tem 36 e 7, com as 14 divergências em relação à sua própria origem
      registradas de forma auditável
- [x] [catálogo de referências](docs/referencias.md) com identificadores estáveis
- [x] `lib/measurement/` — régua de apuração em C++23, *header-only* de
      propósito: uma chamada através de fronteira de tradução mediria a chamada
      junto com a operação
- [x] `lib/contract/` — as três portas T/R/E, e `verificar-suposicao.py`, que
      confere no assembly que a pré-condição da porta R chega ao otimizador
- [x] 5 verificadores, cada um com o **seu autoteste** registrado ao lado
- [x] `meson.build` com a matriz de três configurações e o `-Werror=assume`
      condicional
- [x] suíte verde nas três configurações **nesta** máquina (GCC 15.2.0):
      `debug` e `release-checked` com 11 OK + 1 falha esperada,
      `release` com 11 OK + 1 pulado
- [x] **delta da absorção na norma** — 33 seções e 6 partes viraram 36 e 7:
      seção 30 reconciliada, seção 11 promovida a portão de publicação do par,
      seção 16 sem arm64 e com dois perfis de sanitizer, seções 28 e 29 com CCD,
      precisão de histograma, variação entre execuções e a máquina nomeada, e a
      **Parte VII** nova — par `std` e `custom` (34), regra de decisão (35),
      eixo do código gerado (36)
- [x] tabela de divergências em relação ao setup desta trilha: 12 itens, todos
      verificados nesta máquina antes de entrarem na norma
- [x] colisão de nome de `contract.hpp`: a lei do tópico é `spec.hpp` (seção 34)
- [x] colisão de caminho, e a **regra por trás dela invertida**: o nome é
      `bench/medicoes/`, e o histórico de campanha passa a ser **versionado**. O
      `.gitignore` herdado excluía saída bruta; num projeto cujo ativo é a
      procedência do número, isso remove a evidência. Vale a prática do DPDK
      Academy: `medicoes/historico/<data>-<campanha>/`, ambiente ao lado, uma
      saída por repetição
- [x] o eixo `linguagem ↔ compilador ↔ código gerado` virou a seção 36, e o
      módulo transversal `10-codigo-gerado` está previsto na trilha
- [x] `SECURITY.md` nos dois idiomas, `CITATION.cff` em 0.01.00 com o projeto
      absorvido citado como referência não publicada, e `CLAUDE.md` — este
      último isento da regra do par por decisão declarada no verificador:
      é configuração de ferramenta, e duas cópias de instrução que divergem em
      silêncio são pior defeito que a ausência de par
- [x] prefixo das variáveis de ambiente resolvido: **`HARNESS_SAMPLES`,
      `HARNESS_ROUNDS`, `HARNESS_TAIL_SAMPLES`**. O prefixo nomeia o
      **instrumento**, não a academia, porque `lib/` é candidato a repositório
      próprio consumido pelos três projetos da família. E o sufixo saiu do
      português: variável de ambiente é interface de programa, não prosa
- [x] a fronteira do idioma escrita na norma (seção 3): identificador, saída de
      programa, flag, variável de ambiente e esquema de dado em inglês;
      comentário, script e registro emitido por script em português. A ausência
      dessa fronteira produziu deriva medida — 35 mensagens em português,
      `custo_do_relogio()` e `--braco-de-controle` no primeiro programa do
      repositório, tudo corrigido
- [x] a cláusula "mensagens de commit em inglês", herdada do documento de
      origem e contrariada por todo o histórico dos dois projetos, passou a
      dizer **português**, com a razão registrada: o assunto do commit é o
      achado, e achado é prosa
- [x] `pre-commit.sh` — sintaxe, os nove verificadores mais os nove autotestes,
      varredura de segredo com escopo declarado, e conferência de que o SHA
      fixado da ação de CI ainda é o topo do major declarado no comentário. O
      modo `--rapido` é o da CI; o gancho local roda a suíte também
- [x] CI em dois *jobs*: consistência antes de build, e build com matriz de
      compilador (GCC 14 e Clang 18 da imagem `ubuntu-24.04`) × as três
      configurações, que vêm de `lib-configuracoes.sh` e não são duplicadas no
      *workflow*. Ação fixada por SHA, `permissions: contents: read`, gatilho só
      em `main` e `pull_request`
- [x] `dependabot.yml` só para `github-actions`, com a razão de não haver regra
      `ignore` registrada no próprio arquivo
- [x] **o contrato CI ↔ máquina de referência, resolvido por medição.** A CI
      rodava `ubuntu-24.04` com GCC 14 e Clang 18 e falhou na primeira execução:
      o **Clang 18.1.3 não compila** `<expected>` nem
      `hardware_destructive_interference_size`, e a norma afirmava "Clang 18+".
      Quem pegou foi o `check-env.sh`, porque sonda recurso em vez de confiar na
      versão. A imagem passou a ser `ubuntu-26.04`, que traz **os mesmos GCC
      15.2.0 e Clang 21.1.8 da máquina de referência**, e a matriz virou dois
      *jobs*: `maquina` e `piso` (GCC 14 + Clang 20). Piso que nada compila é
      promessa, não piso. Portabilidade continua sendo da CI e medição da
      máquina — nenhum número publicado sai de lá
- [x] verificador de **par de idiomas**: `verificar-paridade.py`, com quatro
      regras — par, navegação, estrutura e números —, 14 casos de autoteste e
      duas iscas que nasceram de acusações falsas dele mesmo (separador decimal
      por idioma, e a navegação de exemplo dentro de bloco de código)

## Etapa 2 — Ambiente · **parcial**

- [x] `scripts/ambiente.sh`, portado **sem reescrever o parsing**: a versão a
      montante usa `lscpu -p=` e sysfs porque parsing do texto do `lscpu`
      devolve campo vazio em silêncio em máquina com locale traduzido — e esta
      máquina tem. Três modos: texto, `--markdown`, `--json`
- [x] registro de disponibilidade do PMU (`perf_event_paranoid`)
- [x] registro de quais configurações de build existem na árvore
- [x] CCD e *SMT sibling* de cada CPU lógica, com uma linha por CPU
- [x] `scripts/check-env.sh` — diagnóstico com sonda de C++23 real (compila
      `<expected>`, `<print>`, `<span>` e `hardware_destructive_interference_size`
      em vez de olhar a versão do compilador), e cada aviso dizendo o que deixa
      de ser possível
- [x] `scripts/build-all.sh`, `scripts/test-all.sh` e `scripts/lib-configuracoes.sh`,
      com a matriz de configurações num lugar só
- [x] autotestes na suíte `l1+scripts`: braço sintético de dois domínios de L3
      para o agrupamento, e braço de controle que esconde o Meson do `PATH` e
      exige que o `check-env.sh` acuse
- [x] **`perf_event_paranoid` resolvido em 2, e não em 0.** A máquina vinha com
      **4** — extensão do Ubuntu, que bloqueia `perf_event_open` inteiro; o
      kernel original para em 2. O ajuste aplicado foi o **menor que serve**: o
      2 permite contador do próprio processo em espaço de usuário, que é o que a
      trilha mede. Escopo de CPU e perfilagem de kernel continuam fechados, e
      pela seção 30 da norma eles são assunto do DPDK Academy e do EX442.
      Verificado: `perf stat` devolve `cycles:u`, `cache-misses:u` e
      `branch-misses:u` do próprio benchmark

- [x] `governor=powersave`: **fica como está, e a decisão é por medição.** A
      primeira campanha sugeria aquecimento (r0 e r1 ~26% mais caros); a segunda
      refutou — o desvio foi para r3 e os percentis saíram idênticos nas cinco
      execuções. O que varia é o **máximo**, conforme a execução tenha colhido
      uma interrupção ou não. Fixar `performance` exigiria root e quebraria a
      promessa de que nada didático exige privilégio — e quem reproduz também
      estará em `powersave`. A amplitude entre execuções continua publicada, e
      ela mede o estado da máquina durante a campanha
- [x] `perf_event_paranoid` **persistente**: `/etc/sysctl.d/80-perf-event-paranoid.conf`
      fixa o valor 2, com o motivo de não ser 0 escrito no próprio arquivo

## Etapa 3 — Harness e o ciclo completo · **parcial**

- [x] **decidido: amostras cruas em buffer pré-alocado, não HdrHistogram.**
      `lib/measurement/tail.hpp`, percentil exato, sem precisão a declarar
      porque não há perda a declarar. Duas razões verificadas: o HdrHistogram_c
      não está no WrapDB e seu escritor de log arrasta zlib, contra a seção 16;
      e o problema que ele resolve — fluxo ilimitado em espaço constante — não é
      o de um laboratório que escolhe quantas operações medir. O que se perde
      está escrito no cabeçalho do arquivo
- [x] a convenção de quantas amostras um percentil exige: `min_samples_for(p)`,
      com 10 observações além do percentil — 1000 amostras para p99, 10 000 para
      p99,9 —, e `tail_statistics` carregando o percentil máximo sustentado para
      que publicar acima dele seja erro visível
- [x] `docs/08-medicao/01-harness/`: compila, roda, e arquiva campanha de 5
      execuções, versionando `metadata.json` — com a série completa por
      execução —, a tabela publicável nos dois idiomas e `ambiente.md`. A saída
      crua por repetição e o `ambiente.json` ficam locais: guardar o mesmo dado
      duas vezes não aumenta procedência, aumenta a chance de divergirem
- [x] `scripts/arquivar-medicao.sh` e `scripts/compor-medicao.py`: exigem
      `release` conferido na configuração real do Meson (não no nome do
      diretório), recusam campanha de uma execução só, e publicam a **amplitude
      entre execuções** de cada métrica
- [x] `docs/README.md` como índice que não anuncia tópico inexistente
- [ ] `docs/regras-de-decisao.md` com esquema de dados e verificador que confira
      cada linha contra um `metadata.json` existente — **só faz sentido a partir
      do primeiro tópico que compara**, e 08.01 não compara
- [x] **decidido: GoogleTest onde for necessário, e TAP onde ele não alcança.**
      A suíte conta menos testes do que asserções — `tail-sanity` é **um** teste
      com **onze** casos, e uma regressão em um deles reporta uma falha só.
      O déficit não vinha da ausência do GTest: vinha de `protocol: 'exitcode'`,
      o padrão do Meson, que também aceita `tap` e `gtest`. Então:
      **GoogleTest** (`protocol: 'gtest'`) para teste de C++ com vários casos,
      com nome de caso em português porque é prosa; **TAP** (`protocol: 'tap'`)
      para os testes de shell e Python, que o GTest nunca cobriria — `l2_run.sh`,
      os autotestes de script e os autotestes dos próprios verificadores
- [ ] executar a decisão: `subprojects/gtest.wrap` fixado por hash (está no
      WrapDB), conversão de `tail_sanity` e `contract_sanity`, e saída TAP nos
      testes de shell e Python. Um framework sem usuário é dependência sem
      pagador, então o wrap entra no mesmo commit do primeiro teste que o use
- [ ] **`CITATION.cff` é ponto cego do `verificar-autodescricao.py`**, que só
      confere `.md`. Três contagens dele envelheceram em silêncio e foram
      corrigidas à mão em 2026-09-22 — "doze verificadores" virou dez, "oito
      módulos" virou nove de dez. Ou o verificador passa a ler `.cff` e `.yml`,
      ou o arquivo continua fora do portão e isso fica dito nele (está)
- [ ] portar `verificar-medicao.py` do DPDK Academy — o **13º verificador**, e o
      único acoplamento hoje sem portão: número publicado num README contra a
      campanha que ele cita. Refazer uma campanha muda os números e nada fica
      vermelho
- [x] [`docs/plano-estudo.md`](docs/plano-estudo.md) — a ordem de leitura, nos
      dois idiomas, com a divergência em relação a esta ordem justificada módulo
      a módulo: alocação antes de layout porque medir SoA sob pressão de alocador
      mede o alocador; contêineres antes do livro porque o array plano só não
      parece mágica depois de se ver a estrutura genérica que ele bate; parsing
      perto do fim porque o resultado incômodo pesa mais depois de seis módulos
      em que a alternativa compensou

## Etapa 3.5 — Dívida de ferramental, declarada

Estes quatro eram **prometidos pelo material e não rastreados aqui** — a norma e
os READMEs os citavam como se fossem trabalho conhecido, e não eram item de
nenhuma etapa. Uma dívida que só existe em prosa não é dívida: é esquecimento
com data marcada.

- [x] `verificar-autodescricao.py` — **reescrito, não portado**. As 838 linhas
      do original têm 34 pontos de acoplamento com a estrutura do projeto de
      origem; o que se porta é a ideia, não o arquivo. Ele confere cinco censos
      contra o disco — norma, trilha, verificadores, suíte e o rótulo de estado
      da seção 5 — e entende numeral por extenso, porque a norma escreve "os
      nove têm autoteste", não "os 7".
      **Na primeira execução acusou onze problemas; nove eram reais**: o censo
      dizia 31 tópicos previstos onde o disco tem 23, seis verificadores com
      autoteste onde há sete, e a contagem da suíte defasada.
      Dois eram dele, e viraram a regra do *registro de mudança*
- [x] `verificar-promessa.py` — três regras: todo programa citado existe, toda
      fonte entra na compilação, e todo comando `./caminho` de bloco aponta para
      arquivo. **Na primeira execução acusou 160 problemas, e nenhum era real** —
      a regra aceitava sublinhado como separador e pegava `unordered_map`, e
      acusava os nomes de tópico que o próprio índice declara não iniciados.
      Quatro iscas depois, 105 promessas conferidas e zero quebradas. Um defeito
      dele era de fato bug: `01-harness` existe **como diretório**, e a coleta
      só olhava arquivos
- [x] `ferramental/graficos/` — SVG escrito à mão, **sem dependência**: exigir
      matplotlib transformaria em asterisco a promessa de que nada além do
      compilador é necessário. Quatro arquivos por figura, claro e escuro ×
      PT e EN.
      A paleta não saiu de gosto: as verificações de separação para daltonismo
      (Machado-Oliveira-Fernandes, severidade 1,0), piso de croma, faixa de
      luminosidade e contraste foram **computadas** nos dois modos — separação
      23,8 no claro e 25,7 no escuro, contra alvo de 8,0. O tom de estado nunca
      carrega significado sozinho: todo ponto que o usa recebe rótulo direto
- [x] a paridade de **imagens** passou a ser verificada. A seção 31 da norma
      prometia "a paridade PT/EN vale para as imagens" e a frase não tinha
      portão: um gráfico com rótulo em português entregue ao leitor de inglês é
      uma figura pela metade, e nada acusava
- [x] **decidido: não haverá documento separado sobre como ler os números.** O
      [plano de estudo](docs/plano-estudo.md) roteia o leitor para as seções 6 a
      9 e 28 da norma antes de qualquer módulo, e cada tópico carrega *O que esta
      medição não mostra* por exigência da seção 27. Um terceiro documento
      restaria o mesmo conteúdo e criaria uma terceira cópia para divergir —
      que é o defeito que a fusão das duas árvores acabou de corrigir

## Etapas 4 em diante — a trilha

Os dez módulos foram reconciliados com a norma e têm índice em
[`docs/`](docs/README.md), com pergunta, pergunta de falha e par `std` →
`custom`. **23 tópicos previstos, 2 escritos.**

O critério de pronto é o mesmo em todas: cada tópico com `std/`, `custom/`,
`spec.hpp`, L1 parametrizado, campanha arquivada e **regra de decisão**.

### Etapa 4 — Memória e layout · módulos [01](docs/01-memoria/README.md) e [02](docs/02-layout/README.md)

- [ ] 6 tópicos. É a etapa que estreia o par `std` → `custom` de verdade, e com
      ele o `spec.hpp`, os `traits` de invariante e o L1 parametrizado — os três
      são decisões de desenho que só se firmam com código na frente
- [ ] é também onde `docs/regras-de-decisao.md` nasce, com a primeira regra real

### Etapa 5 — Contêineres e livro de ofertas · módulos [03](docs/03-conteineres/README.md) e [04](docs/04-livro-de-ofertas/README.md)

- [ ] 6 tópicos. O módulo 04 é onde a `spec.hpp` é posta à prova: `std::map` e
      array plano por *tick* **não** cumprem a mesma lei, e a divergência é o
      assunto do tópico
- [ ] pendente de decisão editorial: o livro de ofertas é cenário ou
      protagonista? A §1.2 do documento de origem diz cenário, e a trilha lhe dá
      um módulo inteiro mais o capstone (divergência 12 da norma)

### Etapa 6 — Filas e tempo · módulos [05](docs/05-filas-e-concorrencia/README.md) e [06](docs/06-tempo-e-erros/README.md)

- [ ] 6 tópicos. TSan limpo é critério, e o perfil `sanitize-thread` existe para
      isso — mas o que o TSan **não** reporta não é prova de correção, e o
      argumento vem da norma citada por cláusula
- [ ] teste negativo em cada invariante de fila

### Etapa 7 — Parsing e medição · módulos [07](docs/07-parsing/README.md) e [08.02](docs/08-medicao/README.md)

- [ ] 3 tópicos. Inclui o resultado incômodo que dá crédito ao resto: os casos
      em que `string_view` com `from_chars` já é a resposta
- [ ] o 08.02 depende de PMU, que a máquina de referência já tem em
      `perf_event_paranoid=2`

### Etapa 8 — Capstone · módulo [09](docs/09-capstone/README.md)

- [ ] o motor de livro em duas versões, com *feed* sintético reproduzível
- [ ] a tabela que **atribui** a diferença ponta a ponta a cada tópico — sem ela
      o capstone é demonstração, não fecho

### Etapa 9 — Código gerado · módulo [10](docs/10-codigo-gerado/README.md)

- [ ] transversal, e alimentado pelos outros: onde o `custom/` ganhar por geração
      de código e não por layout, o achado vira tópico aqui

### Etapa 10 — Extração de `lib/`

- [ ] `lib/measurement` e `lib/contract` viram repositório próprio, consumido por
      este projeto, pelo DPDK Academy e pelo Messaging Academy
- [ ] a fronteira tem de ser **ABI C**, e isso é restrição desde já: o DPDK
      Academy é escrito em C, e um harness em `<expected>` e *template* não é
      consumível de lá (norma, seção 33)

## Navegação

- [README do repositório](README.md)
- [Documentação e norma](docs/README.md)

> 🇧🇷 Português | [🇺🇸 English](ROADMAP.en.md)
