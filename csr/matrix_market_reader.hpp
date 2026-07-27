#pragma once

#include <string>
#include "csr_matrix.hpp"

CSRMatrix readMatrixMarket(const std::string& filename);
