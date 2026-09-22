| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| std::allocator, lote 64 — p50 | 1674,00 | 26,9% | ns |
| arena artesanal, lote 64 — p50 | 60,00 | 16,7% | ns |
| std::pmr, lote 64 — p50 | 90,00 | 33,3% | ns |
| std::allocator, lote 512 — p50 | 14758,00 | 1,0% | ns |
| arena artesanal, lote 512 — p50 | 371,00 | 2,4% | ns |
| std::pmr, lote 512 — p50 | 591,00 | 1,7% | ns |
| std::allocator, lote 512 — p99 | 20710,10 | 56,5% | ns |
| arena artesanal, lote 512 — p99 | 381,00 | 2,6% | ns |
| std::pmr, lote 512 — p99 | 601,00 | 0,2% | ns |
| std::allocator, lote 4096 — p50 | 88829,00 | 1,6% | ns |
| arena artesanal, lote 4096 — p50 | 3526,00 | 70,4% | ns |
| std::pmr, lote 4096 — p50 | 5020,00 | 71,6% | ns |

5 execuções do mesmo experimento; 10000 amostras por execução no braço de cauda.

A **amplitude entre execuções** é o que separa a cauda da estrutura da cauda
da máquina: uma diferença menor que ela não é resultado.
