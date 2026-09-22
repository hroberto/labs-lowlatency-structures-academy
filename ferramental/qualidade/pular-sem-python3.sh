#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Marcador de verificacao de documentacao ausente por falta do python3.
#
# POR QUE ISTO EXISTE
#
# Os tres verificadores de documentacao -- ancoras de linha, autoteste do
# verificador de ancoras, e links/ancoras de titulo -- viviam dentro de
# `if python3.found()` SEM `else`. Sem o interpretador eles nao eram
# registrados: a suite l1 encolhia em tres testes e reportava verde.
#
# Isto e pior aqui do que nos testes unitarios, e vale dizer por que. Estes tres
# sao o mecanismo que confere se o material DIZ A VERDADE: se uma ancora aponta
# para a linha certa, se um link leva a um arquivo que existe. Quando eles somem
# em silencio, o repositorio deixa de conseguir provar a propria honestidade --
# e continua afirmando, pela suite verde, que provou.
#
# Reproduzido assim (com o `else` ausente):
#
#     PATH=/usr/bin:/bin meson setup build-sem-py   # sem python3 no PATH
#     meson test -C build-sem-py --suite l1         # tres testes a menos, rc=0
#
# O remedio e o mesmo que o projeto irmao labs-dpdk-academy ja aplicou ao
# GoogleTest em pular-sem-gtest.sh -- script que NAO foi portado para ca,
# porque aqui ainda nao ha dependencia de GoogleTest a pular; era promessa
# sem codigo. E e o mesmo remedio que o codigo 77 ja resolveu para os testes
# L3: dizer PULADO em vez de sumir. A causa muda (dependencia ausente, nao
# ambiente insuficiente); a regra do projeto nao muda -- o que nao foi
# verificado nao pode ser reportado como verificado.
#
# Uso: pular-sem-python3.sh <nome-do-teste-que-nao-rodou>
set -u

NOME=${1:-"(sem nome)"}

echo "  PULADO - $NOME"
echo ""
echo "  O python3 nao esta disponivel, entao esta verificacao de documentacao"
echo "  nao foi executada. Ela NAO passou: ela nao rodou."
echo ""
echo "  O que deixou de ser conferido enquanto isto durar:"
echo "    - se as ancoras de linha apontam para o trecho de codigo certo;"
echo "    - se os links relativos levam a arquivos que existem."
echo ""
echo "  Para habilitar (Debian/Ubuntu):"
echo "    sudo apt-get install -y python3"

# 77 = PULADO para o Meson, e nao sucesso.
exit 77
