#include <gtest/gtest.h>
#include <stdexcept>
#include "csr_matrix.hpp"
#include "matrix_market_reader.hpp"

TEST(CSRMatrixTest, constructor_allocatesCorrectSizes) {
    CSRMatrix mat(3, 3, 5);
    EXPECT_EQ(mat.getNumberOfRows(), 3);
    EXPECT_EQ(mat.getNumberOfColumns(), 3);
    EXPECT_EQ(mat.getData().size(), 5);
    EXPECT_EQ(mat.getCol().size(), 5);
    EXPECT_EQ(mat.getRow().size(), 4);
}

TEST(CSRMatrixTest, setValues_storesDataAndColumnsCorrectly) {
    CSRMatrix mat(3, 3, 6);
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    std::vector<int> col = {0, 2, 1, 0, 1, 2};
    std::vector<int> row = {0, 2, 3, 6};

    mat.setValues(data, col, row);

    ASSERT_EQ(mat.getData().size(), 6);
    EXPECT_DOUBLE_EQ(mat.getData()[0], 1.0);
    EXPECT_DOUBLE_EQ(mat.getData()[5], 6.0);
    EXPECT_EQ(mat.getCol()[1], 2);
    EXPECT_EQ(mat.getRow()[3], 6);
}

TEST(CSRMatrixTest, setValues_withMismatchedSizes_throwsInvalidArgument) {
    CSRMatrix mat(3, 3, 6);
    std::vector<double> data = {1.0, 2.0};
    std::vector<int> col = {0, 1};
    std::vector<int> row = {0, 2, 3, 6};

    EXPECT_THROW(mat.setValues(data, col, row), std::invalid_argument);
}

TEST(CSRMatrixTest, multiplyByVector_with3x3Matrix_returnsCorrectResult) {
    CSRMatrix mat(3, 3, 6);
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    std::vector<int> col = {0, 2, 1, 0, 1, 2};
    std::vector<int> row = {0, 2, 3, 6};
    mat.setValues(data, col, row);

    std::vector<double> x = {1.0, 2.0, 3.0};
    std::vector<double> y(3, 0.0);

    mat.multiplyByVector(x, y);

    ASSERT_EQ(y.size(), 3);
    EXPECT_DOUBLE_EQ(y[0], 7.0);   // 1*1 + 2*3
    EXPECT_DOUBLE_EQ(y[1], 6.0);   // 3*2
    EXPECT_DOUBLE_EQ(y[2], 32.0);  // 4*1 + 5*2 + 6*3
}

TEST(CSRMatrixTest, readMatrixMarket_loadsBcsstk14_correctDimensionsAndNnz) {
    std::string path = std::string(PROJECT_SOURCE_DIR) + "/matrices/bcsstk14/bcsstk14.mtx";
    CSRMatrix mat = readMatrixMarket(path);

    EXPECT_EQ(mat.getNumberOfRows(), 1806);
    EXPECT_EQ(mat.getNumberOfColumns(), 1806);
    EXPECT_EQ(mat.getData().size(), 63454);
    EXPECT_EQ(mat.getRow().size(), 1807);
    EXPECT_EQ(mat.getRow()[0], 0);
    EXPECT_EQ(mat.getRow()[1806], 63454);

    std::vector<double> x(1806, 1.0);
    std::vector<double> y(1806, 0.0);
    mat.multiplyByVector(x, y);
    EXPECT_EQ(y.size(), 1806);

    double sum = 0.0;
    for (double v : y) sum += v;
    EXPECT_GT(sum, 0.0);
}
