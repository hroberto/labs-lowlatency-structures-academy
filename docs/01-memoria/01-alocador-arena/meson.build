# Tópico 01.01 — alocador de arena contra o alocador global.
#
# O PRIMEIRO PAR `std` E `custom` DA TRILHA, e com ele três estreias:
# `spec.hpp` como lei, `traits` declarando a divergência entre os braços, e o
# L1 parametrizado por tipo.
#
# GOOGLETEST ENTRA AQUI, E COM `protocol: 'gtest'`
#
# Sem o protocolo, os dezesseis casos deste arquivo -- oito invariantes × dois
# braços -- apareceriam como UM teste do Meson. Com ele, o Meson lê o XML do
# GoogleTest e conta cada caso, inclusive os PULADOS. E os pulados são metade
# do assunto: a invariante 6 não se aplica ao `std::allocator` e a 7 não se
# aplica à arena, e é isso que o tópico existe para explicar.

gtest_dep = dependency('gtest_main', required : true)

l1_spec = executable(
  'l1-spec-arena',
  'tests/l1_spec.cpp',
  dependencies : [gtest_dep, measurement_dep],
  include_directories : academy_inc,
)

test('l1 - arena: as sete invariantes da spec, nos dois bracos', l1_spec,
     protocol : 'gtest',
     suite : ['l1', 'memoria'], timeout : 60)

bench_arena = executable(
  'bench-arena',
  'bench/bench_arena.cpp',
  dependencies : measurement_dep,
  include_directories : academy_inc,
)

test('l2 - arena: a varredura como o leitor a executa', bash,
     args : [meson.current_source_dir() / 'tests/l2_run.sh', bench_arena],
     suite : ['l2', 'memoria'], timeout : 300,
     depends : [bench_arena])
