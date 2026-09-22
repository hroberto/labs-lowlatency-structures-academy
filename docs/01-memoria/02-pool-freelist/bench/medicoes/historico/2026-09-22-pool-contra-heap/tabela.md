| Métrica | Mediana entre execuções | Amplitude entre execuções | Unidade |
|---|---:|---:|---|
| std::allocator, 64 vivos — p50 | 481,00 | 20,8% | ns |
| pool, 64 vivos — p50 | 161,00 | 19,3% | ns |
| std::allocator, 64 vivos — p99 | 501,00 | 18,0% | ns |
| pool, 64 vivos — p99 | 171,00 | 128,7% | ns |
| std::allocator, 512 vivos — p50 | 421,00 | 21,4% | ns |
| pool, 512 vivos — p50 | 150,00 | 20,0% | ns |
| std::allocator, 512 vivos — p99 | 451,00 | 20,0% | ns |
| pool, 512 vivos — p99 | 151,00 | 26,5% | ns |
| std::allocator, 4096 vivos — p50 | 411,00 | 26,8% | ns |
| pool, 4096 vivos — p50 | 150,00 | 26,7% | ns |
| std::allocator, 4096 vivos — p99 | 431,00 | 23,2% | ns |
| pool, 4096 vivos — p99 | 151,00 | 27,2% | ns |

5 execuções do mesmo experimento; 10000 amostras por execução no braço de cauda.

A **amplitude entre execuções** é o que separa a cauda da estrutura da cauda
da máquina: uma diferença menor que ela não é resultado.
