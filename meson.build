project(
  'lowlatency-structures-academy',
  'cpp',
  version : '0.01.00',
  meson_version : '>=1.1',
  default_options : [
    'warning_level=3',
    # C++23 ESTRITO, e a escolha tem consequência de projeto, não de gosto.
    #
    # O mecanismo de contrato deste repositório é `[[assume]]`, de C++23
    # (P1774), e NÃO os Contracts de C++26 (P2900). Isso não é preferência
    # editorial: foi medido em 16/09/2026 que o GCC 15.2.0 desta máquina rejeita
    # `int f(int x) pre(x>0)` mesmo com `-std=c++2c -fcontracts`. Ver
    # docs/referencias.md#p2900.
    #
    # Fixar `c++23` em vez de `c++2c` transforma "o recurso não existe" numa
    # decisão declarada, em vez de num erro de sintaxe no meio do build.
    'cpp_std=c++23',
    'buildtype=debugoptimized',
  ],
)

# ---------------------------------------------------------------------------
# A matriz de três configurações (docs/padrao-do-projeto.md, seção 16) NÃO
# aparece aqui, e a ausência é deliberada.
#
# Ela se expressa em três diretórios de build, não em opções deste arquivo:
#
#   meson setup build-debug           -Dbuildtype=debug   -Db_ndebug=false
#   meson setup build-release         -Dbuildtype=release -Db_ndebug=true
#   meson setup build-release-checked -Dbuildtype=release -Db_ndebug=false
#
# A terceira é a que importa e a que costuma faltar: `[[assume]]` com predicado
# falso é comportamento indefinido, e só ela prova que os contratos valem no
# código OTIMIZADO.
#
# O `-Db_ndebug` EXPLÍCITO nas três linhas não é redundância, e isto foi MEDIDO
# em 16/09/2026 com Meson 1.10.1: `-Dbuildtype=release` SOZINHO deixa
# `b_ndebug=false` e não emite `-DNDEBUG`. A primeira versão deste comentário
# omitia a flag no release -- e com ela omitida, `build-release` e
# `build-release-checked` seriam a MESMA configuração. A matriz teria três
# diretórios e duas configurações, com a única que prova o contrato no código
# otimizado silenciosamente anulada: exatamente o "verde indistinguível do
# legítimo" contra o qual o resto deste arquivo foi escrito.
#
# Enquanto `lib/contract/` não existir, não há contrato para verificar, e
# declarar a opção aqui seria promessa sem código -- o que a própria norma
# proíbe. Registrado como pendência na Etapa 2 do ROADMAP.md.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# Régua de apuração: cabeçalhos, sem biblioteca compilada.
#
# É header-only de propósito. Os programas de medição precisam que o corpo do
# laço cronometrado seja inlinável -- uma chamada através de fronteira de
# tradução mediria a chamada junto com a operação, e a operação custa poucos
# nanossegundos.
# ---------------------------------------------------------------------------
academy_inc = include_directories('.')

measurement_dep = declare_dependency(
  include_directories : academy_inc,
)

# ---------------------------------------------------------------------------
# Biblioteca de contratos: tambem header-only, e pela mesma razao -- o corpo de
# `PERF_EXPECTS` precisa estar no local de chamada para que `[[assume]]` sirva
# de algo.
# ---------------------------------------------------------------------------
contract_dep = declare_dependency(
  include_directories : academy_inc,
)

# ---------------------------------------------------------------------------
# `-Werror=assume`: uma suposicao DESCARTADA passa a quebrar o build.
#
# POR QUE, E POR QUE CONDICIONAL. Medido em 16/09/2026: com o predicado sendo a
# chamada `D::accepts(v)`, o Clang 21.1.8 trata a chamada como possivelmente com
# efeito colateral e IGNORA a suposicao, avisando por `-Wassume`. Um aviso nao
# quebra nada, e a porta R passa a ser um no-op que parece funcionar.
#
# `PERF_PURE_PREDICATE` corrige a causa; isto fecha a porta para o proximo caso.
#
# E condicional porque o GCC nao tem essa opcao -- conferido:
# `cc1plus: error: '-Werror=assume': no option '-Wassume'`. Passa-la
# incondicionalmente faria o build falhar no GCC por causa da FLAG, nao do
# codigo. `has_argument` pergunta ao compilador em vez de adivinhar pelo nome.
#
# Nem isto nem o atributo substituem o teste de codigo gerado
# (ferramental/qualidade/verificar-suposicao.py): ele e o unico dos tres que
# pega a regressao num compilador que descarte a suposicao SEM avisar.
cpp = meson.get_compiler('cpp')
if cpp.has_argument('-Werror=assume')
  add_project_arguments('-Werror=assume', language : 'cpp')
  message('-Werror=assume habilitado: suposicao descartada quebra o build')
else
  message('-Werror=assume indisponivel neste compilador; o teste de codigo gerado cobre o caso')
endif

# ---------------------------------------------------------------------------
# Verificadores de documentação.
#
# Registrados na suíte `l1+docs`, e cada um com o SEU AUTOTESTE registrado ao
# lado. O autoteste monta uma árvore de casos, roda o verificador de verdade
# contra ela e faz asserções sobre a saída real -- inclusive iscas de falso
# positivo. Sem ele, um verificador que passa a aceitar tudo continua imprimindo
# "N links verificados, 0 quebrados": verde indistinguível do legítimo.
# ---------------------------------------------------------------------------
python3 = find_program('python3', required : false)
bash = find_program('bash')

verificadores = [
  ['links', 'links relativos e ancoras de titulo'],
  ['ancoras', 'ancoras de linha apontam para o codigo certo'],
  ['aritmetica', 'percentual que o texto torna conferivel fecha'],
  ['retratacoes', 'valor retratado nao sobrevive fora do bloco'],
  ['suposicao', 'a pre-condicao da porta R chega ao otimizador'],
  # A regra do radical identico (norma, secao 2) existe para ser verificavel por
  # maquina, e ate agora nao era -- a propria secao 31 registrava a lacuna.
  ['paridade', 'par de idiomas: existencia, navegacao, estrutura e numeros'],
]

foreach v : verificadores
  nome = v[0]
  descricao = v[1]
  script = meson.project_source_root() / 'ferramental/qualidade/verificar-@0@.py'.format(nome)

  if python3.found()
    test('docs - @0@'.format(descricao), python3,
         args : [script, meson.project_source_root()],
         suite : ['l1', 'docs'], timeout : 120)

    # O verificador do verificador.
    test('docs - autoteste de verificar-@0@'.format(nome), python3,
         args : [script, '--autoteste'],
         suite : ['l1', 'docs'], timeout : 120)
  else
    # PULADO com código 77, e não ausente da suíte: o que não foi verificado
    # não pode ser reportado como verificado. Sem este `else`, a suíte encolhia
    # em silêncio e reportava verde -- foi o defeito que originou o script.
    test('docs - @0@'.format(descricao), bash,
         args : [meson.project_source_root() / 'ferramental/qualidade/pular-sem-python3.sh',
                 'docs - @0@'.format(descricao)],
         suite : ['l1', 'docs'])
  endif
endforeach

# ---------------------------------------------------------------------------
# Sanidade da régua de apuração.
#
# Este teste tem DUAS funções, e a primeira é a que não se vê: compilar este
# alvo avalia todos os `static_assert` de lib/measurement/. Os testes de
# `percentile`, `decimals` e `dashes` são asserções de compilação -- a diretriz
# de contratos aplicada à própria régua --, e uma regressão neles quebra o BUILD
# antes de quebrar a suíte.
#
# A segunda é verificar que a coleta EXECUTA e produz resultado publicável: o
# programa sai com código diferente de zero quando a coleta é inválida ou fica
# abaixo da resolução do relógio.
# ---------------------------------------------------------------------------
sanidade_medicao = executable(
  'measurement-sanity',
  'lib/tests/measurement_sanity.cpp',
  dependencies : measurement_dep,
)

# ---------------------------------------------------------------------------
# Sanidade das tres portas de contrato.
#
# Como no alvo de medicao, compilar isto ja avalia os `static_assert` de
# domain.hpp, measurement_domains.hpp e budget.hpp -- dezenas de assercoes de
# dominio que nao custam nada em runtime e que quebram o BUILD quando um limite
# muda sem que o teste mude com ele.
# ---------------------------------------------------------------------------
sanidade_contrato = executable(
  'contract-sanity',
  'lib/tests/contract_sanity.cpp',
  dependencies : contract_dep,
)

test('l1 - contratos: as tres portas em runtime', sanidade_contrato,
     suite : ['l1', 'contract'], timeout : 60)

# A violacao DEVE abortar. Um contrato que nao dispara e indistinguivel de um
# contrato ausente, e nenhum `static_assert` prova que o caminho de falha
# existe.
#
# `should_fail` porque o sucesso deste caso E o processo morrer. Em release o
# programa devolve 77 e o Meson reporta PULADO -- violar `[[assume]]` e
# comportamento indefinido, nao abort, e um teste que esperasse falha ali
# esperaria algo que o padrao nao promete.
test('l1 - contratos: violacao da porta R aborta', sanidade_contrato,
     args : ['--violate'],
     should_fail : true,
     suite : ['l1', 'contract'], timeout : 60)

test('l1 - regua de apuracao coleta resultado publicavel', sanidade_medicao,
     suite : ['l1', 'measurement'], timeout : 120,
     # Teto de amostras e de rodadas: na CI o objetivo é verificar que o
     # programa EXECUTA, não coletar estatística que ninguém vai ler. Medição
     # publicável não define estas variáveis.
     env : {'HARNESS_SAMPLES': '5', 'HARNESS_ROUNDS': '20000'})

# ---------------------------------------------------------------------------
# Scripts de ambiente, na suite `l1+scripts`.
#
# O autoteste de `ambiente.sh` entra na suite, e nao a execucao dele: o
# ambiente e COLETA, e uma coleta nao passa nem falha -- ela descreve a
# maquina. O que pode falhar, e o que a suite verifica, e o script devolver
# campo vazio EM SILENCIO, perder um dominio de L3 no resumo, ou produzir JSON
# invalido para o metadata da medicao. Ver a secao 29 da norma.
#
# O caso do agrupamento de L3 tem BRACO SINTETICO, com dois dominios na
# entrada: numa maquina de um unico CCD, um teste que olhasse so a saida real
# passaria com o defeito presente. Verificacao que nunca dispara e
# indistinguivel de verificacao ausente.
# ---------------------------------------------------------------------------
test('l1 - ambiente.sh nao devolve campo vazio em silencio', bash,
     args : [meson.project_source_root() / 'scripts/tests/l1_ambiente.sh'],
     suite : ['l1', 'scripts'], timeout : 120)

test('l1 - check-env.sh acusa dependencia ausente', bash,
     args : [meson.project_source_root() / 'scripts/tests/l1_check_env.sh'],
     suite : ['l1', 'scripts'], timeout : 120)

# ---------------------------------------------------------------------------
# Sanidade do coletor de cauda.
#
# Como nos outros alvos de sanidade, compilar isto já avalia os `static_assert`
# de tail.hpp -- inclusive o que exige que a formula de percentil de la coincida
# com a de statistics.hpp. Essa divergencia nao apareceria em teste de runtime:
# os dois numeros continuariam plausiveis, e duas tabelas do mesmo repositorio
# passariam a ser incomparaveis em silencio.
# ---------------------------------------------------------------------------
sanidade_cauda = executable(
  'tail-sanity',
  'lib/tests/tail_sanity.cpp',
  dependencies : [measurement_dep, contract_dep],
)

test('l1 - cauda: descarte declarado e percentil exato', sanidade_cauda,
     suite : ['l1', 'measurement'], timeout : 60)

# Pedir publicacao de percentil que a amostra nao sustenta DEVE abortar. Em
# release o programa devolve 77 e o Meson reporta PULADO, pela mesma razao do
# alvo de contratos: violar `[[assume]]` e comportamento indefinido, nao abort.
test('l1 - cauda: publicar percentil sem sustentacao aborta', sanidade_cauda,
     args : ['--violate'],
     should_fail : true,
     suite : ['l1', 'measurement'], timeout : 60)

# ---------------------------------------------------------------------------
# A trilha.
# ---------------------------------------------------------------------------
subdir('trilha/08-medicao/01-harness')
