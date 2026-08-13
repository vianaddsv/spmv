#!/bin/bash


# Bateria GPU local: usa o build-gpu (GPU_OFFLOAD=ON) com offload obrigatorio.
# Os numeros GPU valem apenas para a maquina local (RTX 4050);
# Sirius NAO tem GPU - use job_sirius.sh apenas com os builds CPU.

BIN="./build-gpu/spmv_parallel"

MATRIZES=(
    "matrices/bcsstk14/bcsstk14.mtx"
    "matrices/bcsstk18/bcsstk18.mtx"
    "matrices/G3_circuit/G3_circuit.mtx"
)

if [[ ! -x "$BIN" ]]; then
    echo "ERRO: $BIN nao existe. Configure com:
  cmake -S . -B build-gpu -DGPU_OFFLOAD=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
  LIBRARY_PATH=/home/vianaddsv/dev/toolchain-offload/device-objs cmake --build build-gpu"
    exit 1
fi

echo "Iniciando a bateria de testes GPU locais (offload obrigatorio)..."
echo "-----------------------------------------------------------------"

mkdir -p metrics/desktop
rm -f metrics/desktop/resultado_gpu.csv

for mat in "${MATRIZES[@]}"; do

    NOME_MAT=$(basename $mat .mtx)
    NOME_BIN="build-gpu_spmv_parallel"

    ARQUIVO_SAIDA="metrics/desktop/resultado_local_${NOME_BIN}_${NOME_MAT}.md"
    ARQUIVO_CSV="metrics/desktop/resultado_gpu.csv"

    echo "Executando: $NOME_BIN com a matriz $NOME_MAT"

    OMP_TARGET_OFFLOAD=MANDATORY $BIN -f $mat -o $ARQUIVO_CSV -l $NOME_BIN --gpu-only > $ARQUIVO_SAIDA

done

echo "-----------------------------------------------------------------"
echo "Bateria GPU local finalizada! Resultados em metrics/desktop/resultado_gpu.csv"