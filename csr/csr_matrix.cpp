#include "csr_matrix.hpp"
#include <stdexcept>

CSRMatrix::CSRMatrix(int r, int c, int nnz) : nRows(r), nColumns(c) {

    data.resize(nnz);
    col.resize(nnz);
    row.resize(r + 1);
}

void CSRMatrix::setValues(const std::vector<double> &newData,
                          const std::vector<int> &newCol,
                          const std::vector<int> &newRow) {

    if (newData.size() != data.size() && newCol.size() != data.size()) {
        throw std::invalid_argument(
            "vectores lenght is greater than vector initial alocation size");
    }

    data = newData;
    col = newCol;
    row = newRow;
}

void CSRMatrix::multiplyByVector(const CSRMatrix &matrix,
                                 const std::vector<double> &x,
                                 std::vector<double> &y) {
    const auto &val = matrix.getData();
    const auto &col = matrix.getCol();
    const auto &row = matrix.getRow();
    int totalOfRows = matrix.getNumberOfRows();

    for (int i = 0; i < totalOfRows; ++i) {
        double sum = 0.0;
        int init = row[i];
        int end = row[i + 1];

        for (int j = init; j < end; ++j) {
            sum += val[j] * x[col[j]];
        }
        y[i] = sum;
    }
}
