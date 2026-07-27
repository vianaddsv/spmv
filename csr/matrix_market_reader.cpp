#include "matrix_market_reader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

static void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
}

CSRMatrix readMatrixMarket(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    std::getline(file, line);
    trim(line);

    bool symmetric = false;
    if (line.find("symmetric") != std::string::npos) {
        symmetric = true;
    } else if (line.find("general") == std::string::npos) {
        throw std::runtime_error("Unsupported matrix type (only general or symmetric)");
    }

    while (std::getline(file, line)) {
        trim(line);
        if (!line.empty() && line[0] != '%') break;
    }

    int rows, cols, stored_nnz;
    {
        std::istringstream iss(line);
        if (!(iss >> rows >> cols >> stored_nnz)) {
            throw std::runtime_error("Failed to parse size line");
        }
    }

    std::vector<int> row_counts(rows, 0);
    std::vector<int> all_rows, all_cols;
    std::vector<double> all_vals;
    all_rows.reserve(stored_nnz);
    all_cols.reserve(stored_nnz);
    all_vals.reserve(stored_nnz);

    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;

        int r, c;
        double v;
        std::istringstream iss(line);
        if (!(iss >> r >> c >> v)) continue;

        r--; c--;
        all_rows.push_back(r);
        all_cols.push_back(c);
        all_vals.push_back(v);

        row_counts[r]++;
        if (symmetric && r != c) {
            row_counts[c]++;
        }
    }

    int actual_nnz = 0;
    for (int cnt : row_counts) actual_nnz += cnt;

    CSRMatrix mat(rows, cols, actual_nnz);

    std::vector<double> data(actual_nnz);
    std::vector<int> col_indices(actual_nnz);
    std::vector<int> row_ptr(rows + 1, 0);

    for (int i = 0; i < rows; ++i) row_ptr[i + 1] = row_ptr[i] + row_counts[i];

    std::vector<int> pos = row_ptr;
    for (size_t k = 0; k < all_rows.size(); ++k) {
        int r = all_rows[k], c = all_cols[k];
        double v = all_vals[k];
        int p = pos[r]++;
        col_indices[p] = c;
        data[p] = v;

        if (symmetric && r != c) {
            int p2 = pos[c]++;
            col_indices[p2] = r;
            data[p2] = v;
        }
    }

    mat.setValues(data, col_indices, row_ptr);
    return mat;
}
