#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "../rng.h"
#include "node.hpp"

#include <iostream>


void operator += (std::vector<std::vector<float>>& m1, const std::vector<std::vector<float>>& m2) {
	for (size_t i = 0; i < m1.size(); ++i) {
		for (size_t j = 0; j < m1[0].size(); ++j) {
			m1[i][j] += m2[i][j];
		}
	}
}

std::vector<std::vector<float>> operator * (const std::vector<std::vector<float>>& m, float a) {
	std::vector<std::vector<float>> ans = m;
	for (size_t i = 0; i < m.size(); ++i) {
		for (size_t j = 0; j < m[0].size(); ++j) {
			ans[i][j] *= a;
		}
	}

	return ans;
}

std::ostream& operator << (std::ostream& os, const std::vector<std::vector<float>>& m) {

	for (size_t i = 0; i < m.size(); ++i) {

		os << "(";

		for (size_t j = 0; j < m[i].size(); ++j) {
			os << m[i][j];

			if (j + 1 < m[i].size()) {
				os << ", ";
			}
		}

		os << ")";

		if (i + 1 < m.size()) {
			os << "\n";
		}
	}

	return os;
}


struct Matrix : Node {

	size_t rows, cols;
	std::vector<std::vector<float>> value;
	std::vector<std::vector<float>> partial;

	Matrix(size_t r = 0, size_t c = 0, float fillValue = 0.0f, const std::string& n = "") : rows(r), cols(c), 
		value(std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue))), partial(std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f))) {

		// gave up trying to give a kinda nice name to it
		name = n;
	}

	static std::shared_ptr<Matrix> build(size_t r = 0, size_t c = 0, float fillValue = 0.0f, const std::string& n = "") {
		return std::make_shared<Matrix>(r, c, fillValue, n);
	}

	static std::shared_ptr<Matrix> makeRandom(size_t r = 0, size_t c = 0, double mean = 0.0, double stddev = 1.0, const std::string& n = "") {

		std::shared_ptr<Matrix> mat = std::make_shared<Matrix>(r, c, 0.0f, n);

		for (size_t i = 0; i < r; ++i) {
			for (size_t j = 0; j < c; ++j) {
				mat->value[i][j] = rng::fromNormalDistribution(mean, stddev);
			}
		}

		return mat;
	}

	void evaluate() override {

	}

	void derive() override {

	}

	void resetPartial() override {
		for (size_t i = 0; i < rows; ++i) {
			std::fill(partial[i].begin(), partial[i].end(), 0.0f);
		}
	}

	void calculateDerivatives() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->evaluate();
			ordering[i]->resetPartial();
		}

		partial = std::vector<std::vector<float>>(rows, std::vector<float>(cols, 1.0f)); // dx/dx is 1 for whatever x
		for (size_t i = ordering.size(); i > 0; --i) {
			ordering[i - 1]->derive();
		}
	}
};

// nicer naming
using Mat = std::shared_ptr<Matrix>;


#endif
