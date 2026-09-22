| Metric | Median across runs | Spread across runs | Unit |
|---|---:|---:|---|
| std::allocator, 64 live — p50 | 481.00 | 20.8% | ns |
| pool, 64 live — p50 | 161.00 | 19.3% | ns |
| std::allocator, 64 live — p99 | 501.00 | 18.0% | ns |
| pool, 64 live — p99 | 171.00 | 128.7% | ns |
| std::allocator, 512 live — p50 | 421.00 | 21.4% | ns |
| pool, 512 live — p50 | 150.00 | 20.0% | ns |
| std::allocator, 512 live — p99 | 451.00 | 20.0% | ns |
| pool, 512 live — p99 | 151.00 | 26.5% | ns |
| std::allocator, 4096 live — p50 | 411.00 | 26.8% | ns |
| pool, 4096 live — p50 | 150.00 | 26.7% | ns |
| std::allocator, 4096 live — p99 | 431.00 | 23.2% | ns |
| pool, 4096 live — p99 | 151.00 | 27.2% | ns |

5 runs of the same experiment; 10000 samples per run in the tail arm.

The **spread across runs** is what separates the structure's tail from the
machine's: a difference smaller than it is not a result.
