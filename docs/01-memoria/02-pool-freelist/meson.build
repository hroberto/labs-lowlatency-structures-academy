# Tópico 01.02 — pool com lista de livres contra o alocador global.
#
# O PADRÃO MEDIDO AQUI É OUTRO, E A SPEC TAMBÉM
#
# O 01.01 mede lote que morre inteiro; este mede rotatividade em regime, com a
# ordem de liberação diferente da de aquisição. É o padrão que a arena não
# atende -- e é por isso que a liberação individual, que lá era invariante
# condicional, aqui é exigida dos dois braços pelo `concept`.

l1_pool = executable(
  'l1-spec-pool',
  'tests/l1_spec.cpp',
  dependencies : [gtest_dep, measurement_dep],
  include_directories : academy_inc,
)

test('l1 - pool: as cinco invariantes da rotatividade, nos dois bracos', l1_pool,
     protocol : 'gtest',
     suite : ['l1', 'memoria'], timeout : 60)

bench_pool = executable(
  'bench-pool',
  'bench/bench_pool.cpp',
  dependencies : measurement_dep,
  include_directories : academy_inc,
)

test('l2 - pool: a varredura de rotatividade como o leitor a executa', bash,
     args : [meson.current_source_dir() / 'tests/l2_run.sh', bench_pool],
     suite : ['l2', 'memoria'], timeout : 300,
     depends : [bench_pool])
