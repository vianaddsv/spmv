#!/bin/bash
#$ -S /bin/bash
#$ -V
#$ -cwd
#$ -q all.q@sirius.eic.cefet-rj.br
#$ -o job_geral.log
#$ -e job_erro.log

BINARIOS=(
    "./build-noopt/spmv"
    "./build/spmv"
)

MATRIZES=(
    "matrices/bcsstk14/bcsstk14.mtx"
    "matrices/bcsstk18/bcsstk18.mtx"
    "matrices/G3_circuit/G3_circuit.mtx"
)

echo "Iniciando a bateria de testes..."
echo "----------------------------------------"

for bin in "${BINARIOS[@]}"; do

    NOME_BIN=$(basename $(dirname $bin))

    for mat in "${MATRIZES[@]}"; do
        
        NOME_MAT=$(basename $mat .mtx)
        
        ARQUIVO_SAIDA="resultado_${NOME_BIN}_${NOME_MAT}.md"
        
        echo "Executando: $NOME_BIN com a matriz $NOME_MAT"
        
        $bin -f $mat > $ARQUIVO_SAIDA
        
    done
done

echo "----------------------------------------"
echo "Todos os testes foram finalizados!"
