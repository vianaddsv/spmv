# SpMV (CSR) em Nós Heterogêneos

Código-fonte do trabalho [Análise Microarquitetural da SpMV (CSR) em Nós Heterogêneos: Escalabilidade e Gargalos de Memória entre CPU e GPU](https://github.com/vianaddsv/spmv), disciplina de Computação Paralela e Distribuída (CEFET/RJ).

Avalia a operação Sparse Matrix-Vector Multiplication (SpMV) em formato CSR contrastando uma linha de base sequencial com a paralelização via OpenMP (Static, Guided e particionamento balanceado por NNZ) em um nó de cluster de 24 threads, e com o descarregamento (`target offloading`) para GPU, sob as métricas de SpeedUp, vazão de banda e os limites teóricos do modelo Roofline.

## Setup

Dependências:

- CMake >= 3.14
- Compilador C++ com suporte a OpenMP (GCC para CPU; Clang LLVM com `libomptarget` para GPU offload)
- GTest (opcional, apenas para `-DBUILD_TESTS=ON`)

Compilação:

```bash
# CPU otimizado (Release, -march=native) e linha de base sem otimização (-O0)
make
make noopt

# GPU offload (requer clang++ + CUDA toolchain + libomptarget, ex. RTX 4050 sm_89)
cmake -S . -B build-gpu -DGPU_OFFLOAD=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
cmake --build build-gpu

# Testes unitários (GTest)
make test
```

Baixe as matrizes da SuiteSparse Matrix Collection (bcsstk14, bcsstk18, G3_circuit):

```bash
./download_matrices.sh
```

## Experiments

| Estratégia | Onde | Como |
|------------|------|------|
| Sequencial (com e sem SIMD/-O3) | Local ou Sirius | `./build/spmv -f matrices/bcsstk14/bcsstk14.mtx` |
| OpenMP Static/Guided/Balanced (1–24 threads) | Sirius (Grid Engine, `-pe openmp 24`) | `./build/spmv_parallel -f ... -l build_spmv_parallel` |
| GPU offload (OMP_TARGET_OFFLOAD=MANDATORY) | Local (RTX 4050) | `./build-gpu/spmv_parallel -f ... --gpu-only` |

Baterias completas:

```bash
./run_local.sh        # sequencial + OpenMP na máquina local
./run_local_gpu.sh    # GPU offload na máquina local
qsub job_sirius.sh    # sequencial + OpenMP no cluster Sirius (24 threads)
```

Saídas: logs por execução em `metrics/{desktop,sirius}/` e resultados consolidados em CSV (tempo médio, GB/s, GFLOPs, intensidade aritmética, checksum de correção).

## Results

Matrizes: bcsstk14 (M=1.806, NNZ=63.454), bcsstk18 (M=11.948, NNZ=149.090), G3_circuit (M=1.585.478, NNZ=7.660.840).

Picos das estratégias OpenMP no cluster Sirius (24 threads, i7-13700, AVX2):

| Matriz | Estratégia | Threads | Banda (GB/s) |
|--------|------------|---------|--------------|
| bcsstk14 | Guided | 12 | 46.68 |
| bcsstk18 | Guided | 16 | 98.52 |
| G3_circuit | Guided | 8 | 57.45 |

GPU offload (NVIDIA RTX 4050 Laptop, sm_89):

| Matriz | Tempo Alvo (s) | Banda (GB/s) | SpeedUp vs. seq. |
|--------|----------------|--------------|------------------|
| bcsstk14 | 0.000249 | 3.21 | < 1.0× |
| bcsstk18 | 0.000651 | 3.11 | < 1.0× |
| G3_circuit | 0.023890 | 5.18 | < 1.0× |

Principais achados: vetorização SIMD (AVX2, `-O3`) satura o núcleo único; o escalonamento Guided mitiga o load imbalance do CSR; o SpeedUp na CPU pico de 5.35× (bcsstk18, 16 threads, Guided) com saturação em 24 threads pelo Memory Wall; na GPU o custo de transferência via PCI-Express e os acessos desalinhados (`uncoalesced memory accesses`) mantêm o SpeedUp abaixo de 1.0×. O modelo Roofline confirma que a SpMV opera estritamente na região Memory Bound (< 0.2 FLOPs/byte).

## Citation

```bibtex
@inproceedings{dosSantosViana2026,
  title = {Análise Microarquitetural da SpMV (CSR) em Nós Heterogêneos: Escalabilidade e Gargalos de Memória entre CPU e GPU},
  author = {dos Santos Viana, Daniel Dante},
  booktitle = {Computação Paralela e Distribuída (CEFET/RJ)},
  year = {2026},
  url = {https://github.com/vianaddsv/spmv},
}
```
