# Tópico 01.03 — `std::pmr` como meio-termo.
#
# O TERCEIRO BRAÇO NÃO É ARTESANAL, E É ESSE O PONTO
#
# Os dois tópicos anteriores compararam `std::allocator` com código escrito à
# mão. Este compara com outra coisa da PRÓPRIA biblioteca padrão -- e a pergunta
# deixa de ser "quanto se ganha escrevendo" para ser "a linguagem já oferece?".
#
# O L1 roda os TRÊS braços sobre a spec do 01.01. Se a `pmr_arm` cumprisse uma
# lei ligeiramente diferente, este arquivo não compilaria.

l1_pmr = executable(
  'l1-spec-pmr',
  'tests/l1_spec.cpp',
  dependencies : [gtest_dep, measurement_dep],
  include_directories : academy_inc,
)

test('l1 - pmr: as sete invariantes do 01.01, nos tres bracos', l1_pmr,
     protocol : 'gtest',
     suite : ['l1', 'memoria'], timeout : 60)

bench_pmr = executable(
  'bench-pmr',
  'bench/bench_pmr.cpp',
  dependencies : measurement_dep,
  include_directories : academy_inc,
)

test('l2 - pmr: os tres bracos como o leitor os executa', bash,
     args : [meson.current_source_dir() / 'tests/l2_run.sh', bench_pmr],
     suite : ['l2', 'memoria'], timeout : 300,
     depends : [bench_pmr])
