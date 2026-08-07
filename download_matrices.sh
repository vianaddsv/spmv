#!/bin/bash

# Download de matrizes da SuiteSparse Matrix Collection (ex-UF Sparse Matrix Collection)
# Cada entrada é "GRUPO/NOME". O modelo de leitura é o mesmo usado nos benchmarks:
# "matrices/NOME/NOME.mtx"
#
# Uso:
#   ./download_matrices.sh           # baixa apenas as que faltam
#   ./download_matrices.sh --force   # baixa todas novamente (sobrescreve)

BASE_URL="https://suitesparse-collection-website.herokuapp.com/MM"

MATRIZES=(
    "HB/bcsstk14"
    "HB/bcsstk18"
    "AMD/G3_circuit"
)

FORCE=false
if [[ "$1" == "--force" ]]; then
    FORCE=true
fi

echo "Baixando matrizes da SuiteSparse Matrix Collection..."
echo "------------------------------------------------------"

for mat in "${MATRIZES[@]}"; do
    GRUPO="${mat%%/*}"
    NOME="${mat##*/}"

    mkdir -p "matrices/$NOME"
    DEST="matrices/$NOME/$NOME.mtx"

    if [[ -f "$DEST" ]] && [[ "$FORCE" == false ]]; then
        echo "SKIP : $NOME já existe ($DEST)"
        continue
    fi

    URL="$BASE_URL/$GRUPO/$NOME.tar.gz"
    echo -n "Baixando: $NOME (grupo $GRUPO) .. "

    # Baixa e extrai o .mtx do tar.gz dentro da própria pasta da matriz
    if curl -fsSL --retry 3 --max-time 300 -o "matrices/$NOME/$NOME.tar.gz" "$URL" &&
       tar -xzf "matrices/$NOME/$NOME.tar.gz" -C "matrices/$NOME" --strip-components=1; then

        if head -1 "$DEST" | grep -q "%%MatrixMarket"; then
            echo "OK ($(du -h "$DEST" | cut -f1))"
        else
            echo "ERRO: arquivo não é uma matriz (removendo)"
            rm -f "$DEST" "matrices/$NOME/$NOME.tar.gz"
        fi
    else
        echo "FALHOU"
        rm -f "$DEST" "matrices/$NOME/$NOME.tar.gz"
    fi
done

# Limpa os tar.gz usados como intermediário
rm -f matrices/*/*.tar.gz

echo "------------------------------------------------------"
echo "Download concluído!"