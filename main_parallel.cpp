#include <cmath>
#include <cstring>
#include <fstream>
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
    std::string csvPath = "resultado_consolidado.csv";
    std::string buildLabel = "unknown";
    bool gpuOnly = false;

    for (int i = 1; i < argc; ++i)
    {
        if ((std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--file") == 0) &&
            i + 1 < argc)
            filePath = argv[++i];
        else if ((std::strcmp(argv[i], "-o") == 0 || std::strcmp(argv[i], "--output") == 0) &&
                 i + 1 < argc)
            csvPath = argv[++i];
        else if ((std::strcmp(argv[i], "-l") == 0 || std::strcmp(argv[i], "--label") == 0) &&
                 i + 1 < argc)
            buildLabel = argv[++i];
        else if (std::strcmp(argv[i], "--gpu-only") == 0)
            gpuOnly = true;
    }

    if (filePath.empty())
    {
        std::cerr << "Usage: " << argv[0]
                  << " -f <matrix.mtx> [-o <csv_path>] [-l <label>] [--gpu-only]\n";
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

        std::string matrixName = filePath;
        size_t lastSlash = matrixName.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            matrixName = matrixName.substr(lastSlash + 1);

        if (!gpuOnly)
        {
        const int benchIters = 100;
        const int warmupIters = 5;
        std::vector<int> threadCounts = {1, 2, 4, 8, 12, 16, 20, 24};

        std::ofstream csvFile(csvPath, std::ios::app);

        csvFile.seekp(0, std::ios::end);
        if (csvFile.tellp() == 0)
        {
            csvFile << "Matriz,Build,Threads,Static_Time,Static_GBps,Guided_Time,Guided_GBps,"
                       "Balanced_Time,Balanced_GBps,Checksum_OK\n";
        }

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
            std::string status = (std::abs(sumRef - sumTest) < 1e-5) ? "YES" : "NO";

            std::cout << std::setw(8) << t << " | " << std::setw(13) << std::fixed
                      << std::setprecision(2) << gbpsStatic << " GB/s | " << std::setw(13)
                      << gbpsGuided << " GB/s | " << std::setw(15) << gbpsBalanced
                      << " GB/s | " << std::setw(10) << status << "\n";

            csvFile << matrixName << "," << buildLabel << "," << t << "," << timeStatic << ","
                    << gbpsStatic << "," << timeGuided << "," << gbpsGuided << ","
                    << timeBalanced << "," << gbpsBalanced << "," << status << "\n";
        }
        std::cout
            << "---------------------------------------------------------------------------\n";

        csvFile.close();
        }

        if (gpuOnly)
        {
            std::vector<double> yGpu(rows, 0.0);

            std::ofstream gpuCsv(csvPath, std::ios::app);
            gpuCsv.seekp(0, std::ios::end);
            if (gpuCsv.tellp() == 0)
                gpuCsv << "Matriz,Build,Target_Time,Target_GBps,Checksum_OK\n";

            int ndev = omp_get_num_devices();
            std::cout << "\nOffload (GPU) benchmark - devices: " << ndev << "\n";

            const int gpuBenchIters = 1000;
            for (int i = 0; i < 10; ++i)
                mat.multiplyByVectorOmpTarget(x, yGpu);

            double startGpu = omp_get_wtime();
            for (int i = 0; i < gpuBenchIters; ++i)
                mat.multiplyByVectorOmpTarget(x, yGpu);
            double timeGpu = (omp_get_wtime() - startGpu) / gpuBenchIters;
            double gbpsGpu = (bytesPerSpmv / timeGpu) / 1e9;

            double sumGpu = 0.0;
            for (double v : yGpu)
                sumGpu += v;
            std::string statusGpu = (std::abs(sumRef - sumGpu) < 1e-5) ? "YES" : "NO";

            std::cout << "Target offload: " << std::fixed << std::setprecision(2) << gbpsGpu
                      << " GB/s | checksum " << statusGpu << "\n";

            gpuCsv << matrixName << "," << buildLabel << "," << timeGpu << "," << gbpsGpu << ","
                   << statusGpu << "\n";
            gpuCsv.close();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
