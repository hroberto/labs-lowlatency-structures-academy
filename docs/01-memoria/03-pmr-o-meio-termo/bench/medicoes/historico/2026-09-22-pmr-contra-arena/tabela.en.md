| Metric | Median across runs | Spread across runs | Unit |
|---|---:|---:|---|
| std::allocator, batch 64 — p50 | 1674.00 | 26.9% | ns |
| hand-written arena, batch 64 — p50 | 60.00 | 16.7% | ns |
| std::pmr, batch 64 — p50 | 90.00 | 33.3% | ns |
| std::allocator, batch 512 — p50 | 14758.00 | 1.0% | ns |
| hand-written arena, batch 512 — p50 | 371.00 | 2.4% | ns |
| std::pmr, batch 512 — p50 | 591.00 | 1.7% | ns |
| std::allocator, batch 512 — p99 | 20710.10 | 56.5% | ns |
| hand-written arena, batch 512 — p99 | 381.00 | 2.6% | ns |
| std::pmr, batch 512 — p99 | 601.00 | 0.2% | ns |
| std::allocator, batch 4096 — p50 | 88829.00 | 1.6% | ns |
| hand-written arena, batch 4096 — p50 | 3526.00 | 70.4% | ns |
| std::pmr, batch 4096 — p50 | 5020.00 | 71.6% | ns |

5 runs of the same experiment; 10000 samples per run in the tail arm.

The **spread across runs** is what separates the structure's tail from the
machine's: a difference smaller than it is not a result.
