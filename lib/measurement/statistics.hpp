// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Henrique M Roberto
//
// Estatística mínima para microbenchmarks.
//
// ORIGEM
//
// Portado de `docs/01-fundamentos/medicoes/statistics.h` do projeto irmão
// labs-dpdk-academy. O padrão de apuração é dele; este arquivo não o reinventa.
// A razão de portar em vez de reescrever está registrada em
// docs/padrao-do-projeto.md: um padrão de apuração escrito duas vezes por duas
// mãos diverge, e duas academias da mesma família publicando números sob réguas
// diferentes não podem ser comparadas entre si.
//
// POR QUE ISTO EXISTE
//
// Publicar um número solto ("8,4 ns") esconde a pergunta que importa: quão
// confiável ele é? Uma medição de 8,4 com dispersão de 0,1 e outra de 8,4 com
// dispersão de 6,0 sustentam conclusões muito diferentes, e o leitor não tem
// como distinguir se só lhe damos a média.
//
// O QUE REPORTAMOS, E POR QUÊ
//
//   mediana   - tendência central robusta. Diferente da média, não é arrastada
//               por uma única amostra ruim (uma interrupção do sistema no meio
//               da coleta).
//
//   mínimo    - em microbenchmark de operação determinística, o ruído é
//               UNILATERAL: interferência só faz a medição demorar mais, nunca
//               menos. Por isso o mínimo é a melhor estimativa do custo real da
//               operação, e a mediana estima o que se observa na prática.
//               Quando os dois quase coincidem, a medição está limpa.
//
//   p25-p75   - intervalo interquartil: onde caem as 50% amostras centrais.
//
//   amplitude - min-max, a faixa completa observada. Vale reportá-la mesmo
//               sendo sensível a um único outlier: em trabalho acadêmico,
//               esconder a extensão do que se observou é pior que exibi-la.
//               Quando a amplitude é muito maior que o IQR, há interferência
//               esporádica que o leitor precisa conhecer.
//
//   CV        - coeficiente de variação (desvio padrão / média, em %).
//               Reportado, mas NÃO usado para julgar a medição, porque é
//               sensível a um único outlier: uma amostra ruim entre 25 pode
//               levá-lo de 2% a 27% sem que a mediana se mova. Serve como
//               DETECTOR DE OUTLIER, não como medida de confiança.
//
// COMO LER disp E CV JUNTOS
//
// O selo (~ ou !) sai de `disp`, e só dela: é o indicador de CONFIANÇA, robusto
// por não olhar as caudas. Diz se o valor típico é reprodutível.
//
// O CV entra como segunda leitura, pela RELAÇÃO com disp:
//
//   CV parecido com disp    -> distribuição bem comportada.
//   CV muito maior que disp -> o miolo é firme, mas houve amostras isoladas
//                              destoantes: interferência esporádica, não
//                              instabilidade do valor.
//
// Deliberadamente não há marcador binário para outlier. Um limiar do tipo
// "máximo > 1,25x a mediana" produz um penhasco arbitrário: duas linhas com
// excursão praticamente igual (1,246x e 1,264x) receberiam selos opostos por
// uma diferença de 1,4%. Exibir os dois números e ensinar a lê-los juntos é
// mais honesto que esconder a continuidade atrás de um limiar.
//
// O QUE ISTO NÃO É
//
// Não é análise estatística rigorosa: não há intervalo de confiança formal nem
// teste de hipótese, porque as amostras de um microbenchmark não são
// independentes nem normalmente distribuídas (há autocorrelação por estado de
// cache e por frequência da CPU). O objetivo é honestidade sobre a dispersão,
// não inferência. Para o tratamento rigoroso, ver [KALIBERA-JONES-2013] em
// docs/referencias.md.

#ifndef PERF_MEASUREMENT_STATISTICS_HPP
#define PERF_MEASUREMENT_STATISTICS_HPP

#include <algorithm>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <vector>

namespace perf::measurement
{

// Limiares sobre a dispersão ROBUSTA (IQR/mediana), não sobre o CV.
inline constexpr double disp_stable = 3.0;
inline constexpr double disp_suspect = 10.0;

// Número de amostras padrão.
inline constexpr int default_samples_fixed = 25;

// Pisos da coleta, nomeados em vez de literais no corpo de `samples()` e
// `rounds()`. O valor e a razão estavam documentados em prosa e escritos como
// número solto no meio de uma chamada -- que é como um piso muda num lugar e
// não no outro.
//
//   min_samples  menos de 3 amostras não permite quartis úteis
//   min_rounds   abaixo de 1000 rodadas o relógio domina o que se quer medir
inline constexpr int min_samples = 3;
inline constexpr int min_rounds = 1000;

struct statistics
{
    double median = 0.0;
    double minimum = 0.0;
    double maximum = 0.0;
    double p25 = 0.0;
    double p75 = 0.0;
    double p99 = 0.0; // cauda: só é significativo com amostras suficientes
    double cv = 0.0;  // coeficiente de variação, em % -- sensível a outliers
    double disp = 0.0; // dispersão robusta: (p75-p25)/mediana, em %
    int samples = 0;
};

namespace detail
{

// Leitura de teto numérico em variável de ambiente, sem `atoi`.
//
// `atoi` não distingue "0" de entrada inválida, e o cabeçalho de origem o usava
// -- um `CPP_ACADEMY_AMOSTRAS=abc` virava 0 em silêncio e depois era elevado ao
// piso. `from_chars` devolve o erro, e aqui a entrada inválida é IGNORADA em
// favor do padrão, não silenciosamente convertida em outro número.
[[nodiscard]] inline int env_ceiling(const char *name, int fallback, int floor_value) noexcept
{
    const char *e = std::getenv(name);
    if (e == nullptr)
    {
        return fallback;
    }
    const std::string_view text{e};
    int parsed = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (ec != std::errc{} || ptr != text.data() + text.size())
    {
        return fallback;
    }
    const int ceiling = parsed >= floor_value ? parsed : floor_value;
    return fallback < ceiling ? fallback : ceiling;
}

} // namespace detail

// Quantas amostras usar. Medições caras declaram um padrão menor; a variável de
// ambiente, quando presente, é um TETO aplicado a todas -- para que a integração
// contínua verifique que os programas EXECUTAM sem gastar minutos coletando
// estatística que ninguém vai ler. Medição de verdade usa o padrão.
[[nodiscard]] inline int samples(int fallback = default_samples_fixed) noexcept
{
    return detail::env_ceiling("CPP_ACADEMY_AMOSTRAS", fallback, min_samples);
}

// Teto de RODADAS por amostra, análogo ao de amostras acima.
//
// POR QUE EXISTE (lição do projeto de origem): reduzir amostras não basta em
// máquina lenta. Num runner de CI de 2 núcleos, uma medição que passa por futex
// e troca de contexto é uma ordem de grandeza mais caro que na máquina de
// referência. Lá a primeira execução da CI estourou 300 s com o teto de
// amostras JÁ aplicado: o custo estava nas rodadas POR amostra, que a variável
// não tocava.
//
// Só faz sentido como TETO, nunca como padrão: baixar rodadas piora a resolução
// da medida, então quem quer número publicável não define a variável.
[[nodiscard]] inline int rounds(int fallback) noexcept
{
    return detail::env_ceiling("CPP_ACADEMY_RODADAS", fallback, min_rounds);
}

// Percentil por interpolação linear sobre a sequência JÁ ORDENADA.
//
// `constexpr` não é enfeite: é o que permite que os testes desta função sejam
// `static_assert` em vez de casos de runtime -- a diretriz de contratos do
// projeto aplicada à própria régua de medição. Ver os asserts ao fim do arquivo.
[[nodiscard]] constexpr double percentile(std::span<const double> v, double p) noexcept
{
    if (v.empty())
    {
        return 0.0;
    }
    if (v.size() == 1)
    {
        return v[0];
    }
    const double pos = p * static_cast<double>(v.size() - 1);
    const auto i = static_cast<std::size_t>(pos);
    const double frac = pos - static_cast<double>(i);
    return i + 1 < v.size() ? v[i] + frac * (v[i + 1] - v[i]) : v.back();
}

// Resume um vetor de amostras JÁ coletadas. ORDENA `v` no lugar.
//
// Existe separado de `collect()` porque nem toda medição cabe no formato "chame
// esta função n vezes": travessia entre processos, por exemplo, produz uma
// amostra por mensagem recebida, e o vetor já chega pronto.
[[nodiscard]] inline statistics summarize(std::span<double> v) noexcept
{
    statistics e{};
    if (v.empty())
    {
        return e;
    }

    double sum = 0.0;
    for (const double x : v)
    {
        sum += x;
    }
    const double mean = sum / static_cast<double>(v.size());

    double var = 0.0;
    for (const double x : v)
    {
        var += (x - mean) * (x - mean);
    }
    var /= static_cast<double>(v.size() > 1 ? v.size() - 1 : 1); // variância amostral

    std::ranges::sort(v);
    const std::span<const double> s{v};

    e.samples = static_cast<int>(v.size());
    e.minimum = s.front();
    e.maximum = s.back();
    e.median = percentile(s, 0.50);
    e.p25 = percentile(s, 0.25);
    e.p75 = percentile(s, 0.75);
    e.p99 = percentile(s, 0.99);
    e.cv = mean > 0.0 ? 100.0 * std::sqrt(var) / mean : 0.0;
    e.disp = e.median > 0.0 ? 100.0 * (e.p75 - e.p25) / e.median : 0.0;

    return e;
}

// Executa `measurement` n vezes e resume as amostras.
//
// O parâmetro é um concept e não um ponteiro de função: isso permite passar
// lambda com captura -- o laço de medição quase sempre precisa do estado que
// está medindo --, e permite ao compilador inlinar a chamada, o que importa
// quando o corpo medido custa poucos nanossegundos.
template <std::invocable<> Fn>
    requires std::convertible_to<std::invoke_result_t<Fn>, double>
[[nodiscard]] statistics collect(Fn &&measurement, int n)
{
    if (n <= 0)
    {
        return statistics{};
    }
    std::vector<double> v;
    v.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        v.push_back(static_cast<double>(measurement()));
    }
    return summarize(v);
}

// O selo avisa QUEM LÊ a tabela. O estado abaixo avisa a SUÍTE, que não lê
// tabela nenhuma -- ela lê o código de saída.
//
// A distinção é o ponto, e vem de um defeito real do projeto de origem. As
// funções de medição devolvem valor negativo quando a operação medida falha.
// `collect()` grava esse negativo no vetor como se fosse tempo, `summarize()`
// tira mediana em cima dele, e o programa imprimia a tabela e saía com 0. O
// selo "?" denunciava a linha para quem lesse; o código de saída dizia OK para
// quem não lê.
//
// `below_resolution` existe separado de `invalid` porque mediana zero não é
// sentinela de erro: é operação mais rápida que a resolução do relógio (ver
// `clock_period_ns()` em clock.hpp). Não é publicável como custo, e não é
// defeito. Os callbacks devem devolver negativo ou NaN em falha, nunca zero.
enum class collection_state
{
    invalid,
    below_resolution,
    valid
};

[[nodiscard]] inline collection_state state_of(const statistics &e, int n) noexcept
{
    const bool finite = std::isfinite(e.median) && std::isfinite(e.minimum) &&
                        std::isfinite(e.maximum) && std::isfinite(e.p25) &&
                        std::isfinite(e.p75) && std::isfinite(e.p99) && std::isfinite(e.cv) &&
                        std::isfinite(e.disp);
    if (n <= 0 || e.samples != n || !finite || e.minimum < 0.0)
    {
        return collection_state::invalid;
    }
    return e.median > 0.0 ? collection_state::valid : collection_state::below_resolution;
}

[[nodiscard]] inline bool is_valid(const statistics &e, int n) noexcept
{
    return state_of(e, n) == collection_state::valid;
}

// Marca visual de confiança, para o leitor não precisar interpretar o CV.
//
//   " "  dispersão baixa: o valor típico é confiável
//   "~"  dispersão moderada: leia com reserva
//   "!"  dispersão alta: o valor típico diz pouco
//   "?"  NÃO HOUVE MEDIÇÃO -- ver abaixo
//
// O SELO "?" EXISTE POR CAUSA DE UM DEFEITO REAL DO ARQUIVO DE ORIGEM.
//
// Quando uma medição falha por completo -- por exemplo, uma função que devolve
// 0.0 porque a criação da thread falhou --, todas as amostras são zero. Aí média
// e mediana são zero, e os guardas `mean > 0` / `median > 0` de `summarize()`,
// que existem para evitar divisão por zero, fazem cv e disp valerem **0,0%**.
// Com disp = 0, o selo devolvia " ": a linha da tabela em que nada foi medido
// aparecia como a MAIS confiável do conjunto.
//
// Uma medição de tempo cuja mediana é exatamente zero não é uma medição rápida:
// é uma medição que não aconteceu. O selo diz isso.
[[nodiscard]] inline std::string_view badge(const statistics &e) noexcept
{
    if (!is_valid(e, e.samples))
    {
        return "?";
    }
    if (e.disp <= disp_stable)
    {
        return " ";
    }
    if (e.disp <= disp_suspect)
    {
        return "~";
    }
    return "!";
}

// Casas decimais conforme a magnitude: sem isso, valores abaixo de 1 ns saem
// todos iguais na exibição e contradizem o CV, que é calculado sobre os valores
// reais.
[[nodiscard]] constexpr int decimals(double v) noexcept
{
    if (v < 1.0)
    {
        return 3;
    }
    if (v < 100.0)
    {
        return 2;
    }
    return 1;
}

// Régua de traços, fatiada conforme a largura de cada coluna.
//
// `std::string_view` NÃO tem construtor (contagem, caractere) -- a primeira
// versão deste arquivo o chamou e não compilou. Fatiar uma constante é a forma
// correta, e é `constexpr`.
inline constexpr std::string_view rule =
    "--------------------------------------------------------";

[[nodiscard]] constexpr std::string_view dashes(std::size_t n) noexcept
{
    return rule.substr(0, n <= rule.size() ? n : rule.size());
}

inline void print_header()
{
    std::print("  {:<34} {:>9}  {:<15} {:<17} {:>5} {:>5}\n", "medicao", "mediana",
               "p25-p75 (IQR)", "amplitude min-max", "disp", "CV");
    std::print("  {:<34} {:>9}  {:<15} {:<17} {:>5} {:>5}\n", dashes(34), dashes(9), dashes(15),
               dashes(17), dashes(5), dashes(5));
}

inline void print_row(std::string_view label, const statistics &e)
{
    const int d = decimals(e.median);
    const auto iqr = std::format("{:.{}f}-{:.{}f}", e.p25, d, e.p75, d);
    const auto range = std::format("{:.{}f}-{:.{}f}", e.minimum, d, e.maximum, d);
    std::print("  {:<34} {:>9.{}f}  {:<15} {:<17} {:>4.1f}% {:>4.1f}% {}\n", label, e.median, d,
               iqr, range, e.disp, e.cv, badge(e));
}

// Variante com conversão para CICLOS. Comparar em ciclos neutraliza a diferença
// de frequência entre máquinas e gerações -- é assim que se separa "ficou mais
// rápido porque o clock subiu" de "ficou mais rápido de verdade".
inline void print_header_cycles()
{
    std::print("  {:<34} {:>9}  {:<15} {:>8}      {:>5}\n", "medicao", "mediana",
               "p25-p75 (IQR)", "ciclos", "disp");
}

inline void print_row_cycles(std::string_view label, const statistics &e, double period_ns)
{
    const int d = decimals(e.median);
    const auto iqr = std::format("{:.{}f}-{:.{}f}", e.p25, d, e.p75, d);
    // O rótulo desta coluna é `disp`, e não `CV`: é `e.disp` que ela publica. No
    // projeto de origem a tabela mais acadêmica rotulava a dispersão robusta com
    // o nome do coeficiente de variação, que é outra grandeza.
    std::print("  {:<34} {:>9.{}f}  {:<15} {:>8.0f}      {:>4.1f}% {}\n", label, e.median, d, iqr,
               period_ns > 0.0 ? e.median / period_ns : 0.0, e.disp, badge(e));
}

// Variante para LATÊNCIA, que se reporta por percentis e não por valor típico.
// Publica p99 no lugar da amplitude: quando a cauda é o requisito, o máximo
// isolado é uma amostra só -- informa menos do que parece.
inline void print_header_tail()
{
    std::print("  {:<30} {:>9} {:>9} {:>9} {:>9}  {:>7}\n", "medicao", "minimo", "mediana", "p75",
               "p99", "amostras");
}

inline void print_row_tail(std::string_view label, const statistics &e)
{
    const int d = decimals(e.median);
    std::print("  {:<30} {:>9.{}f} {:>9.{}f} {:>9.{}f} {:>9.{}f}  {:>7}\n", label, e.minimum, d,
               e.median, d, e.p75, d, e.p99, d, e.samples);
}

// --- Testes em tempo de compilação ---------------------------------------
//
// A diretriz de contratos deste projeto aplicada à própria régua: onde a função
// é `constexpr`, o teste dela é `static_assert` -- custo zero em runtime, e a
// violação quebra o BUILD em vez de quebrar a suíte. Ver
// docs/padrao-do-projeto.md, seção sobre as três portas de contrato.
namespace
{
constexpr double sorted_five[] = {1.0, 2.0, 3.0, 4.0, 5.0};
static_assert(percentile(std::span<const double>{sorted_five}, 0.50) == 3.0);
static_assert(percentile(std::span<const double>{sorted_five}, 0.00) == 1.0);
static_assert(percentile(std::span<const double>{sorted_five}, 1.00) == 5.0);
static_assert(percentile(std::span<const double>{sorted_five}, 0.25) == 2.0);

// Interpolação: com quatro amostras a mediana cai entre a segunda e a terceira.
constexpr double sorted_four[] = {10.0, 20.0, 30.0, 40.0};
static_assert(percentile(std::span<const double>{sorted_four}, 0.50) == 25.0);

// Vetor de uma amostra: todo percentil é o próprio valor. Caso de borda que o
// arquivo de origem trata e que não tinha teste nenhum.
constexpr double single[] = {7.5};
static_assert(percentile(std::span<const double>{single}, 0.99) == 7.5);

static_assert(dashes(5) == "-----");
static_assert(dashes(999).size() == rule.size());

static_assert(decimals(0.5) == 3);
static_assert(decimals(1.0) == 2);
static_assert(decimals(99.9) == 2);
static_assert(decimals(100.0) == 1);
} // namespace

} // namespace perf::measurement

#endif // PERF_MEASUREMENT_STATISTICS_HPP
