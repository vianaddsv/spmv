#include "csr_matrix.hpp"
#include <omp.h>
#include <stdexcept>

CSRMatrix::CSRMatrix(int r, int c, int nnz) : nRows(r), nColumns(c), nThreads(0)
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
        y[i] = computeRowSum(x, i);
    }
}

void CSRMatrix::multiplyByVectorOmp(const std::vector<double>& x, std::vector<double>& y,
                                    int threads) const
{
#pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < nRows; ++i)
    {
        y[i] = computeRowSum(x, i);
    }
}

void CSRMatrix::multiplyByVectorOmpGuided(const std::vector<double>& x, std::vector<double>& y,
                                          int threads) const
{
#pragma omp parallel for num_threads(threads) schedule(guided)
    for (int i = 0; i < nRows; ++i)
    {
        y[i] = computeRowSum(x, i);
    }
}

void CSRMatrix::prepareNNZPartitioning(int threads)
{
    nThreads = threads;
    threadRowStart.resize(nThreads + 1, 0);

    int totalNnz = data.size();
    int targetNnzPerThread = totalNnz / nThreads;

    threadRowStart[0] = 0;

    int currentThread = 1;
    int accumulatedNnz = 0;

    for (int i = 0; i < nRows; ++i)
    {
        int rowNnz = row[i + 1] - row[i];
        accumulatedNnz += rowNnz;

        if (accumulatedNnz >= currentThread * targetNnzPerThread && currentThread < nThreads)
        {
            threadRowStart[currentThread] = i + 1;
            currentThread++;
        }
    }
    threadRowStart[nThreads] = nRows;
}

void CSRMatrix::multiplyByVectorOmpBalanced(const std::vector<double>& x, std::vector<double>& y) const
{
#pragma omp parallel num_threads(nThreads)
    {
        int tid = omp_get_thread_num();

        int initRow = threadRowStart[tid];
        int endRow = threadRowStart[tid + 1];

        for (int i = initRow; i < endRow; ++i)
        {
            y[i] = computeRowSum(x, i);
        }
    }
}
