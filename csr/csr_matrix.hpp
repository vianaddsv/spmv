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

  public:
    CSRMatrix(int l, int c, int nnz);
    void setValues(std::vector<double> newData, std::vector<int> newCol, std::vector<int> newLine);

    void multiplyByVector(const std::vector<double>& x, std::vector<double>& y) const;

    const std::vector<double>& getData() const { return data; };
    const std::vector<int>& getCol() const { return col; };
    const std::vector<int>& getRow() const { return row; };

    int getNumberOfRows() const { return nRows; }
    int getNumberOfColumns() const { return nColumns; }
};
