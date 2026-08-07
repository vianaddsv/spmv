#!/bin/bash


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

echo "Iniciando a bateria de testes LOCAIS..."
echo "----------------------------------------"

for bin in "${BINARIOS[@]}"; do

    NOME_BIN="$(basename $(dirname $bin))_$(basename $bin)"

    for mat in "${MATRIZES[@]}"; do
        
        NOME_MAT=$(basename $mat .mtx)
        
        mkdir -p metrics/desktop
        ARQUIVO_SAIDA="metrics/desktop/resultado_local_${NOME_BIN}_${NOME_MAT}.md"
        
        if [[ "${bin##*/}" == spmv ]]; then
            ARQUIVO_CSV="metrics/desktop/resultado_consolidado_seq.csv"
        else
            ARQUIVO_CSV="metrics/desktop/resultado_consolidado.csv"
        fi
        
        echo "Executando: $NOME_BIN com a matriz $NOME_MAT"
        
        $bin -f $mat -o $ARQUIVO_CSV -l $NOME_BIN > $ARQUIVO_SAIDA
        
    done
done

echo "----------------------------------------"
echo "Todos os testes locais foram finalizados!"
