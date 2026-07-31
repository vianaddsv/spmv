#include "csr_matrix.hpp"
#include <stdexcept>

CSRMatrix::CSRMatrix(int r, int c, int nnz) : nRows(r), nColumns(c)
{

    data.resize(nnz);
    col.resize(nnz);
    row.resize(r + 1);
}

void CSRMatrix::setValues(std::vector<double> newData, std::vector<int> newCol,
                          std::vector<int> newRow)
{

    if (newData.size() != data.size() || newCol.size() != data.size())
    {
        throw std::invalid_argument(
            "vectores lenght is greater than vector initial alocation size");
    }

    data = std::move(newData);
    col = std::move(newCol);
    row = std::move(newRow);
}

void CSRMatrix::multiplyByVector(const std::vector<double>& x, std::vector<double>& y) const
{
    for (int i = 0; i < nRows; ++i)
    {
        double sum = 0.0;
        int init = row[i];
        int end = row[i + 1];

        for (int j = init; j < end; ++j)
        {
            sum += data[j] * x[col[j]];
        }
        y[i] = sum;
    }
}
