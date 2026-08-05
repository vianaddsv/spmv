#!/bin/bash
#$ -S /bin/bash
#$ -V
#$ -cwd
#$ -q all.q@sirius.eic.cefet-rj.br
#$ -o job_sirius.log
#$ -e job_sirius_erro.log

BINARIOS=(
    "./build/spmv"
    "./build/spmv_parallel"
    "./build-noopt/spmv"
    "./build-noopt/spmv_parallel"
)

MATRIZES=(
    "matrices/bcsstk14/bcsstk14.mtx"
    "matrices/bcsstk18/bcsstk18.mtx"
    "matrices/G3_circuit/G3_circuit.mtx"
)

echo "Iniciando a bateria de testes no cluster SIRIUS..."
echo "----------------------------------------"

for bin in "${BINARIOS[@]}"; do

    # A sua lógica brilhante para o nome do binário
    NOME_BIN="$(basename $(dirname $bin))_$(basename $bin)"

    for mat in "${MATRIZES[@]}"; do
        
        NOME_MAT=$(basename $mat .mtx)
        
        # Alterado para a pasta do cluster
        mkdir -p metrics/sirius
        ARQUIVO_SAIDA="metrics/sirius/resultado_sirius_${NOME_BIN}_${NOME_MAT}.md"
        
        echo "Executando: $NOME_BIN com a matriz $NOME_MAT"
        
        # O OpenMP vai usar os 24 núcleos físicos/lógicos do nó Sirius
        $bin -f $mat > $ARQUIVO_SAIDA
        
    done
done

echo "----------------------------------------"
echo "Todos os testes no Sirius foram finalizados!"