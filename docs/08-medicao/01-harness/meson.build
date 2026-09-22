# Tópico 08.01 — o harness medindo a si mesmo.
#
# O PRIMEIRO TÓPICO DA TRILHA É ESTE, E A ORDEM É DELIBERADA
#
# O ROADMAP o coloca antes de qualquer tópico de conteúdo porque ele prova o
# CICLO INTEIRO -- compila, roda, arquiva medição com metadata -- antes de
# existir número que dependa dele. Um tópico de estrutura escrito primeiro
# publicaria percentis por operação sem que o piso do instrumento estivesse
# medido, e a diferença entre "a estrutura custa 20 ns" e "o instrumento custa
# 20 ns" não apareceria em teste nenhum.
#
# NÃO HÁ `spec.hpp` AQUI, E A AUSÊNCIA É CORRETA
#
# A seção 34 da norma manda que todo tópico que COMPARA tenha spec e dois
# braços. Este não compara duas estruturas: ele mede o instrumento. Inventar uma
# spec para caber no formato produziria arquivo vazio, que é autodescrição falsa
# (seção 5).

bench_harness = executable(
  'bench-harness',
  'bench/bench_harness.cpp',
  dependencies : measurement_dep,
)

# L2 roda o binário como o leitor o roda, e tem braço de controle: com cauda
# curta o programa TEM de avisar que o p99,9 não é publicável.
test('l2 - harness: o programa que o leitor executa', bash,
     args : [meson.current_source_dir() / 'tests/l2_run.sh', bench_harness],
     suite : ['l2', 'harness'], timeout : 180,
     depends : [bench_harness])
