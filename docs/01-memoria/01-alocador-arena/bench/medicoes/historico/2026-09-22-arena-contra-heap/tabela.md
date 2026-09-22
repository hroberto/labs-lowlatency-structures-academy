| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| std::allocator, lote 8 — p50 | 60,00 | 33,3% | ns |
| arena, lote 8 — p50 | 20,00 | 50,0% | ns |
| std::allocator, lote 64 — p50 | 1673,00 | 26,4% | ns |
| arena, lote 64 — p50 | 60,00 | 50,0% | ns |
| std::allocator, lote 64 — p99 | 1703,00 | 145,3% | ns |
| arena, lote 64 — p99 | 71,00 | 28,2% | ns |
| std::allocator, lote 512 — p50 | 14828,00 | 3,9% | ns |
| arena, lote 512 — p50 | 371,00 | 5,1% | ns |
| std::allocator, lote 512 — p99 | 18966,00 | 35,6% | ns |
| arena, lote 512 — p99 | 481,00 | 22,9% | ns |
| std::allocator, lote 4096 — p50 | 88388,00 | 1,2% | ns |
| arena, lote 4096 — p50 | 3576,00 | 82,1% | ns |
| std::allocator, lote 4096 — p99 | 91704,30 | 12,0% | ns |
| arena, lote 4096 — p99 | 4458,00 | 62,5% | ns |

5 execuções do mesmo experimento; 10000 amostras por execução no braço de cauda.

A **amplitude entre execuções** é o que separa a cauda da estrutura da cauda
da máquina: uma diferença menor que ela não é resultado.
