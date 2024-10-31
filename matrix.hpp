#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "../rng.h"
#include "node.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>


inline void operator += (std::vector<std::vector<float>>& m1, const std::vector<std::vector<float>>& m2) {
	for (size_t i = 0; i < m1.size(); ++i) {
		for (size_t j = 0; j < m1[0].size(); ++j) {
			m1[i][j] += m2[i][j];
		}
	}
}

inline std::vector<std::vector<float>> operator + (const std::vector<std::vector<float>>& m1, const std::vector<std::vector<float>>& m2) {
	std::vector<std::vector<float>> ans = m1;
	for (size_t i = 0; i < m1.size(); ++i) {
		for (size_t j = 0; j < m1[0].size(); ++j) {
			ans[i][j] += m2[i][j];
		}
	}

	return ans;
}

inline std::vector<std::vector<float>> operator * (const std::vector<std::vector<float>>& m, float a) {
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
	std::shared_ptr<Matrix> gradientFunction;

	Matrix(size_t r = 0, size_t c = 0, float fillValue = 0.0f, const std::string& n = "", bool trainable = false) : rows(r), cols(c), 
		value(std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue))), partial(std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f))) {

		#if USE_NAME
			name = n;
		#endif

		isTrainable = trainable;
	}

	static std::shared_ptr<Matrix> build(size_t r = 0, size_t c = 0, float fillValue = 0.0f, bool trainable = false, const std::string& n = "") {
		return std::make_shared<Matrix>(r, c, fillValue, n, trainable);
	}

	static std::shared_ptr<Matrix> makeRandom(size_t r = 0, size_t c = 0, double mean = 0.0, double stddev = 1.0, bool trainable = false, const std::string& n = "") {

		std::shared_ptr<Matrix> mat = std::make_shared<Matrix>(r, c, 0.0f, n, trainable);

		stddev /= static_cast<double>(c);

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

	void resetPartial(float defaultValue = 0.0f) override final {
		for (size_t i = 0; i < rows; ++i) {
			std::fill(partial[i].begin(), partial[i].end(), defaultValue);
		}
	}

	NodeTypes getType() {
		return MATRIX;
	}

	void updateGradientFunction() override {
		
	}

	void resetGradientFunction(float defaultValue = 0.0f) override {
		gradientFunction = Matrix::build(rows, cols, defaultValue);
	}



	void saveToFile(const std::string& path) {

		// creates the path directory if it doesn't exist
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream file(path, std::ios::binary);

		file.write(reinterpret_cast<const char*>(&isTrainable), sizeof(isTrainable));
		file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
		file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));

		for (size_t i = 0; i < rows; ++i) {
			file.write(reinterpret_cast<const char*>(&value[i][0]), cols * sizeof(float));
		}

		file.close();
	}

	static std::shared_ptr<Matrix> loadFromFile(const std::string& path) {

		std::ifstream file(path, std::ios::binary);

		if (!file) {
			throw std::runtime_error("Cannot open file :(");
		}

		bool trainable;
		size_t rows, cols;

		file.read(reinterpret_cast<char*>(&trainable), sizeof(trainable));
		file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
		file.read(reinterpret_cast<char*>(&cols), sizeof(cols));

		std::shared_ptr<Matrix> mat = std::make_shared<Matrix>(rows, cols, 0.0f, "", trainable);

		for (size_t i = 0; i < rows; ++i) {
			file.read(reinterpret_cast<char*>(&mat->value[i][0]), cols * sizeof(float));
		}

		return mat;
	}


	Vec get(size_t index);
};

// nicer naming
using Mat = std::shared_ptr<Matrix>;








struct GetMatrixRow : Vector {
	Mat a;
	size_t index;

	GetMatrixRow(size_t s = 0) {
		size = s;
		value = std::vector<float>(s, 0.0f);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Mat& m, size_t index = 0) {

		std::shared_ptr<GetMatrixRow> node = std::make_shared<GetMatrixRow>(m->cols);

		node->a = m;
		node->index = index;
		#if USE_NAME
			node->name = m->name + "[" + std::to_string(index) + "]";
		#endif

		node->parents.push_back(m);

		return node;
	}

	void evaluate() override final {
		value = a->value[index];
	}

	void derive() override final {
		a->partial[index] += partial;
	}
};


Vec Matrix::get(size_t index = 0) {
	return GetMatrixRow::build(std::dynamic_pointer_cast<Matrix>(shared_from_this()), index);
}




#endif
