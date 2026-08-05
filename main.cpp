#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "csr_matrix.hpp"
#include "matrix_market_reader.hpp"

int main(int argc, char* argv[])
{
    std::string filePath;

    for (int i = 1; i < argc; ++i)
    {
        if ((std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--file") == 0) &&
            i + 1 < argc)
            filePath = argv[++i];
    }

    if (filePath.empty())
    {
        std::cerr << "Usage: " << argv[0] << " -f <matrix.mtx>\n";
        return 1;
    }

    try
    {
        std::cout << "Loading matrix: " << filePath << "\n";
        CSRMatrix mat = readMatrixMarket(filePath);

        long long rows = mat.getNumberOfRows();
        long long cols = mat.getNumberOfColumns();
        long long nnz = mat.getData().size();

        std::cout << "Rows: " << rows << "\n";
        std::cout << "Cols: " << cols << "\n";
        std::cout << "Nonzeros (NNZ): " << nnz << "\n";

        std::vector<double> x(cols, 1.0);
        std::vector<double> y(rows, 0.0);

        const int warmupIters = 10;
        for (int i = 0; i < warmupIters; ++i)
            mat.multiplyByVector(x, y);

        const int benchIters = 100;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < benchIters; ++i)
            mat.multiplyByVector(x, y);

        auto end = std::chrono::high_resolution_clock::now();

        double totalElapsed = std::chrono::duration<double>(end - start).count();
        double avgTimeSec = totalElapsed / benchIters;

        double flopsPerSpmv = 2.0 * nnz;
        double gflopsPerSec = (flopsPerSpmv / avgTimeSec) / 1e9;

        double bytesPerSpmv =
            ((rows + 1) * 4.0) + (nnz * 4.0) + (nnz * 8.0) + (cols * 8.0) + (rows * 8.0);
        double gbPerSec = (bytesPerSpmv / avgTimeSec) / 1e9;

        double arithmeticIntensity = flopsPerSpmv / bytesPerSpmv;

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "--------------------------------------\n";
        std::cout << "Avg Time per SpMV : " << avgTimeSec << " s\n";
        std::cout << "Bandwidth         : " << gbPerSec << " GB/s\n";
        std::cout << "Performance (Y)   : " << gflopsPerSec << " GFLOP/s\n";
        std::cout << "Arith. Intens.(X) : " << arithmeticIntensity << " FLOPs/Byte\n";
        std::cout << "--------------------------------------\n";

        double sum = 0.0;
        for (double v : y)
            sum += v;
        std::cout << "Sum(y) validation : " << sum << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
