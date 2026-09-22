| Item | Valor |
|---|---|
| CPU | AMD Ryzen 9 9900X 12-Core Processor |
| Núcleos / threads | 12 físicos, 24 lógicos (2 por núcleo, smt/active=1) |
| Soquetes / nós NUMA | 1 / 1 |
| L3 | 32768K x 2 instancia(s) |
| Domínios de L3 (CCD) | `0-5,12-17 6-11,18-23` |
| CPUs lógicas por domínio | `L3#0=[0,1,2,3,4,5,12,13,14,15,16,17] L3#1=[6,7,8,9,10,11,18,19,20,21,22,23]` |
| Sinalizadores de TSC | constant_tsc nonstop_tsc |
| TSC invariante | sim (constant_tsc e nonstop_tsc) |
| Governor / turbo | powersave / intel_no_turbo=- amd_boost=1 |
| PMU | perf_event_paranoid=2 — so o proprio processo, sem eventos de kernel |
| Memória | 14,2 GiB (disponível: 4,7 GiB) |
| Velocidade da memória | NAO LIDO -- rode: sudo ./scripts/ambiente.sh --cachear-memoria |
| Pentes / canais | NAO LIDO -- idem |
| Hugepages | 1024 de 2048 kB (1024 livres) |
| Placa | ASRock B850 Steel Legend WiFi |
| BIOS | 4.10 de 02/10/2026 |
| Kernel / arquitetura | 7.0.0-31-generic / x86_64 |
| Distribuição | Ubuntu 26.04.1 LTS |
| Linha de comando do kernel | `(nenhum parametro relevante)` |
| Mitigações ativas | spec_rstack_overflow=Mitigation: IBPB on VMEXIT only; spec_store_bypass=Mitigation: Speculative Store Bypass disabled via prctl; spectre_v1=Mitigation: usercopy/swapgs barriers and __user pointer sanitization; spectre_v2=Mitigation: Enhanced / Automatic IBRS; IBPB: conditional; STIBP: always-on; PBRSB-eIBRS: Not affected; BHI: Not affected; vmscape=Mitigation: IBPB on VMEXIT |
| Mitigações não aplicáveis | gather_data_sampling, ghostwrite, indirect_target_selection, itlb_multihit, l1tf, mds, meltdown, mmio_stale_data, old_microcode, reg_file_data_sampling, retbleed, srbds, tsa, tsx_async_abort |
| GCC | gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0 |
| Clang | Ubuntu clang version 21.1.8 (6ubuntu1) |
| Meson / Ninja | 1.10.1 / 1.13.2 |
| perf | perf version 7.0.14 |
| Configurações de build na árvore | build-clang-release-checked (release b_ndebug=false); build-debug (debug b_ndebug=false); build-release (release b_ndebug=true); build-release-checked (release b_ndebug=false) |
| Commit / árvore suja | 39c1794 / sim |
| Coletado em | 2026-09-22T10:46:05-03:00 |
