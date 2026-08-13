#pragma once

#include <vector>

class CSRMatrix
{

  private:
    std::vector<double> data;
    std::vector<int> col;
    std::vector<int> row;
    int nRows;
    int nColumns;
    int nThreads;
    std::vector<int> threadRowStart;

    inline double computeRowSum(const std::vector<double>& x, int i) const
    {
        double sum = 0.0;
        int init = row[i];
        int end = row[i + 1];

        for (int j = init; j < end; ++j)
        {
            sum += data[j] * x[col[j]];
        }
        return sum;
    }

  public:
    CSRMatrix(int l, int c, int nnz);
    void setValues(std::vector<double> newData, std::vector<int> newCol, std::vector<int> newLine);

    void multiplyByVector(const std::vector<double>& x, std::vector<double>& y) const;
    void multiplyByVectorOmp(const std::vector<double>& x, std::vector<double>& y,
                             int threads) const;
    void multiplyByVectorOmpGuided(const std::vector<double>& x, std::vector<double>& y,
                                   int threads) const;
    void prepareNNZPartitioning(int threads);
    void multiplyByVectorOmpBalanced(const std::vector<double>& x, std::vector<double>& y) const;
    void multiplyByVectorOmpTarget(const std::vector<double>& x, std::vector<double>& y) const;

    const std::vector<double>& getData() const { return data; };
    const std::vector<int>& getCol() const { return col; };
    const std::vector<int>& getRow() const { return row; };

    int getNumberOfRows() const { return nRows; }
    int getNumberOfColumns() const { return nColumns; }
};
