#include "matrix_market_reader.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

struct Entry
{
    int row, col;
    double val;
};

CSRMatrix readMatrixMarket(const std::string& filename)
{
    FILE* file = std::fopen(filename.c_str(), "r");
    if (!file)
        throw std::runtime_error("Cannot open file: " + filename);

    char line[4096];

    if (!std::fgets(line, sizeof(line), file))
    {
        std::fclose(file);
        throw std::runtime_error("Empty file");
    }

    bool symmetric = false;
    if (std::strstr(line, "symmetric"))
        symmetric = true;
    else if (!std::strstr(line, "general"))
    {
        std::fclose(file);
        throw std::runtime_error("Unsupported matrix type (only general or symmetric)");
    }

    while (std::fgets(line, sizeof(line), file))
    {
        if (line[0] != '%')
            break;
    }

    int rows, cols, storedNnz;
    if (std::sscanf(line, "%d %d %d", &rows, &cols, &storedNnz) != 3)
    {
        std::fclose(file);
        throw std::runtime_error("Failed to parse size line");
    }

    std::vector<Entry> entries;
    entries.reserve(symmetric ? 2 * storedNnz : storedNnz);

    int r, c;
    double v;

    while (std::fgets(line, sizeof(line), file))
    {
        if (line[0] == '\n')
            continue;

        int n = std::sscanf(line, "%d %d %lf", &r, &c, &v);

        if (n >= 2)
        {
            if (n == 2)
            {
                v = 1.0;
            }

            r--;
            c--;
            entries.push_back({r, c, v});
            if (symmetric && r != c)
                entries.push_back({c, r, v});
        }
    }

    std::fclose(file);

    if (entries.empty())
        throw std::runtime_error("No entries found");

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b)
              { return a.row < b.row || (a.row == b.row && a.col < b.col); });

    std::vector<Entry> uniqueEntries;
    uniqueEntries.reserve(entries.size());

    for (const auto& e : entries)
    {
        if (!uniqueEntries.empty() && uniqueEntries.back().row == e.row &&
            uniqueEntries.back().col == e.col)
            uniqueEntries.back().val += e.val;
        else
            uniqueEntries.push_back(e);
    }

    int actualNnz = static_cast<int>(uniqueEntries.size());

    std::vector<double> data(actualNnz);
    std::vector<int> colIndices(actualNnz);
    std::vector<int> rowPtr(rows + 1, 0);

    int currentRow = 0;
    for (int i = 0; i < actualNnz; ++i)
    {
        while (currentRow <= uniqueEntries[i].row)
            rowPtr[currentRow++] = i;
        data[i] = uniqueEntries[i].val;
        colIndices[i] = uniqueEntries[i].col;
    }
    while (currentRow <= rows)
        rowPtr[currentRow++] = actualNnz;

    CSRMatrix mat(rows, cols, actualNnz);
    mat.setValues(std::move(data), std::move(colIndices), std::move(rowPtr));
    return mat;
}
