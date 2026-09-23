# Tópico 02.01 — SoA contra AoS.
#
# A INVARIANTE 1 NÃO RODA POR TIPO, E ISSO É DE PROPÓSITO
#
# Ela afirma que os DOIS braços guardam o mesmo dado, e uma afirmação sobre a
# relação entre dois tipos não cabe num `TYPED_TEST`. É um `TEST` comum, sobre o
# par -- e é ela que impede o modo de falha clássico do benchmark de layout: um
# braço guardando menos, e a tabela publicando a diferença de dado com o nome
# de layout.

l1_layout = executable(
  'l1-spec-layout',
  'tests/l1_spec.cpp',
  dependencies : [gtest_dep, measurement_dep],
  include_directories : academy_inc,
)

test('l1 - layout: as quatro invariantes, nos dois layouts', l1_layout,
     protocol : 'gtest',
     suite : ['l1', 'layout'], timeout : 60)
