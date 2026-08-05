#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <omp.h>

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

        std::vector<double> yRef(rows, 0.0);
        std::vector<double> yStatic(rows, 0.0);
        std::vector<double> yGuided(rows, 0.0);
        std::vector<double> yBalanced(rows, 0.0);

        double flopsPerSpmv = 2.0 * nnz;

        double bytesPerSpmv =
            ((rows + 1) * 4.0) + (nnz * 4.0) + (nnz * 8.0) + (cols * 8.0) + (rows * 8.0);

        double arithmeticIntensity = flopsPerSpmv / bytesPerSpmv;

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "--------------------------------------------------------\n";
        std::cout << "FLOPs (Y)          : " << flopsPerSpmv << "\n";
        std::cout << "Bandwidth (Bytes)  : " << bytesPerSpmv << "\n";
        std::cout << "Arith. Intens.(X)  : " << arithmeticIntensity << " FLOPs/Byte\n";
        std::cout << "--------------------------------------------------------\n\n";

        mat.multiplyByVector(x, yRef);
        double sumRef = 0.0;
        for (double v : yRef)
            sumRef += v;

        const int benchIters = 100;
        const int warmupIters = 5;
        std::vector<int> threadCounts = {1, 2, 4, 8, 12, 16, 20, 24};

        std::cout << " Threads | Static (GB/s) | Guided (GB/s) | Balanced (GB/s) | Checksum OK?\n";
        std::cout
            << "---------------------------------------------------------------------------\n";

        for (int t : threadCounts)
        {
            for (int i = 0; i < warmupIters; ++i)
                mat.multiplyByVectorOmp(x, yStatic, t);

            double startStatic = omp_get_wtime();
            for (int i = 0; i < benchIters; ++i)
                mat.multiplyByVectorOmp(x, yStatic, t);
            double timeStatic = (omp_get_wtime() - startStatic) / benchIters;
            double gbpsStatic = (bytesPerSpmv / timeStatic) / 1e9;

            for (int i = 0; i < warmupIters; ++i)
                mat.multiplyByVectorOmpGuided(x, yGuided, t);

            double startGuided = omp_get_wtime();
            for (int i = 0; i < benchIters; ++i)
                mat.multiplyByVectorOmpGuided(x, yGuided, t);
            double timeGuided = (omp_get_wtime() - startGuided) / benchIters;
            double gbpsGuided = (bytesPerSpmv / timeGuided) / 1e9;

            mat.prepareNNZPartitioning(t);

            for (int i = 0; i < warmupIters; ++i)
                mat.multiplyByVectorOmpBalanced(x, yBalanced);

            double startBalanced = omp_get_wtime();
            for (int i = 0; i < benchIters; ++i)
                mat.multiplyByVectorOmpBalanced(x, yBalanced);
            double timeBalanced = (omp_get_wtime() - startBalanced) / benchIters;
            double gbpsBalanced = (bytesPerSpmv / timeBalanced) / 1e9;

            double sumTest = 0.0;
            for (double v : yBalanced)
                sumTest += v;
            std::string status = (std::abs(sumRef - sumTest) < 1e-5) ? "YES" : "NO!";

            std::cout << std::setw(8) << t << " | " << std::setw(13) << gbpsStatic << " | "
                      << std::setw(13) << gbpsGuided << " | " << std::setw(15) << gbpsBalanced
                      << " | " << std::setw(10) << status << "\n";
        }
        std::cout
            << "---------------------------------------------------------------------------\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
