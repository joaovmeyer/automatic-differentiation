#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "../rng.h"
#include "node.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>



#ifndef NUM_TYPE
#define NUM_TYPE NUM_TYPE
#endif


inline void operator += (std::vector<std::vector<NUM_TYPE>>& m1, const std::vector<std::vector<NUM_TYPE>>& m2) {
	for (size_t i = 0; i < m1.size(); ++i) {
		for (size_t j = 0; j < m1[0].size(); ++j) {
			m1[i][j] += m2[i][j];
		}
	}
}

inline std::vector<std::vector<NUM_TYPE>> operator + (const std::vector<std::vector<NUM_TYPE>>& m1, const std::vector<std::vector<NUM_TYPE>>& m2) {
	std::vector<std::vector<NUM_TYPE>> ans = m1;
	for (size_t i = 0; i < m1.size(); ++i) {
		for (size_t j = 0; j < m1[0].size(); ++j) {
			ans[i][j] += m2[i][j];
		}
	}

	return ans;
}

inline std::vector<std::vector<NUM_TYPE>> operator * (const std::vector<std::vector<NUM_TYPE>>& m, NUM_TYPE a) {
	std::vector<std::vector<NUM_TYPE>> ans = m;
	for (size_t i = 0; i < m.size(); ++i) {
		for (size_t j = 0; j < m[0].size(); ++j) {
			ans[i][j] *= a;
		}
	}

	return ans;
}

std::ostream& operator << (std::ostream& os, const std::vector<std::vector<NUM_TYPE>>& m) {

	for (size_t i = 0; i < m.size(); ++i) {

		os << "[";

		for (size_t j = 0; j < m[i].size(); ++j) {
			os << m[i][j];

			if (j + 1 < m[i].size()) {
				os << ", ";
			}
		}

		os << "]";

		if (i + 1 < m.size()) {
			os << "\n";
		}
	}

	return os;
}






struct Matrix;



struct Mat {
	std::shared_ptr<Matrix> ptr;

	Mat() {}
	Mat(const std::shared_ptr<Node>& p) : ptr(std::dynamic_pointer_cast<Matrix>(p)) {}
	// now this is cool
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Matrix, T>>>
	Mat(const std::shared_ptr<T>& p) : ptr(p) {}

	Matrix* operator -> () const {
		return ptr.get();
	}

	Matrix& operator * () const {
		return *ptr;
	}

	std::vector<std::vector<NUM_TYPE>> operator () () const;


	operator std::shared_ptr<Matrix>() const {
		return ptr;
	}
	operator std::shared_ptr<Node>() const {
		return std::static_pointer_cast<Node>(ptr);
	}

	Vec operator [] (size_t index) const;
};







struct Matrix : Node {

	size_t rows, cols;
	std::vector<std::vector<NUM_TYPE>> value;
	std::vector<std::vector<NUM_TYPE>> partial;
	std::shared_ptr<Matrix> gradientFunction;

	Matrix(size_t r = 0, size_t c = 0, NUM_TYPE fillValue = 0.0f, const std::string& n = "", bool trainable = false) : rows(r), cols(c), 
		value(std::vector<std::vector<NUM_TYPE>>(r, std::vector<NUM_TYPE>(c, fillValue))), partial(std::vector<std::vector<NUM_TYPE>>(r, std::vector<NUM_TYPE>(c, 0.0f))) {

		#if USE_NAME
			name = n;
		#endif

		isTrainable = trainable;
	}

	static std::shared_ptr<Matrix> build(size_t r = 0, size_t c = 0, NUM_TYPE fillValue = 0.0f, bool trainable = false, const std::string& n = "") {
		return std::make_shared<Matrix>(r, c, fillValue, n, trainable);
	}

	static std::shared_ptr<Matrix> makeRandom(size_t r = 0, size_t c = 0, NUM_TYPE mean = 0.0, NUM_TYPE stddev = 1.0, bool trainable = false, const std::string& n = "") {

		std::shared_ptr<Matrix> mat = std::make_shared<Matrix>(r, c, 0.0f, n, trainable);

		stddev /= static_cast<NUM_TYPE>(c);

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

	void resetPartial(NUM_TYPE defaultValue = 0.0f) override final {
		for (size_t i = 0; i < rows; ++i) {
			std::fill(partial[i].begin(), partial[i].end(), defaultValue);
		}
	}

	NodeTypes getType() {
		return MATRIX;
	}

	void updateGradientFunction() override {
		
	}

	void resetGradientFunction(NUM_TYPE defaultValue = 0.0f) override {
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
			file.write(reinterpret_cast<const char*>(&value[i][0]), cols * sizeof(NUM_TYPE));
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
			file.read(reinterpret_cast<char*>(&mat->value[i][0]), cols * sizeof(NUM_TYPE));
		}

		return mat;
	}


	Vec get(size_t index);
};

// nicer naming
// using Mat = std::shared_ptr<Matrix>;




std::vector<std::vector<NUM_TYPE>> Mat::operator () () const {
	ptr->eval();
	return ptr->value;
}






Mat operator + (const Mat& m1, const Mat& m2);

// a[i] += b
struct MatrixAddAtPos : Matrix {
	Mat a;
	Vec b;
	size_t index;

	MatrixAddAtPos(size_t r = 0, size_t c = 0, NUM_TYPE fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<NUM_TYPE>>(r, std::vector<NUM_TYPE>(c, fillValue));
		partial = std::vector<std::vector<NUM_TYPE>>(r, std::vector<NUM_TYPE>(c, 0.0f));
	}

	static Mat build(const Mat& m, const Vec& v, size_t index = 0) {

		std::shared_ptr<MatrixAddAtPos> node = std::make_shared<MatrixAddAtPos>(m->rows, m->cols);

		node->a = m;
		node->b = v;
		node->index = index;
		#if USE_NAME
			node->name = m->name + "[" + std::to_string(index) + "] + " + v->name;
		#endif

		node->parents.push_back(m);
		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = a->value;
		value[index] += b->value;
	}

	void derive() override final {
		a->partial += partial;
		b->partial += partial[index];
	}

	void updateGradientFunction() override final {
		a->gradientFunction = a->gradientFunction + gradientFunction;
		b->gradientFunction = b->gradientFunction + gradientFunction->get(index);
	}
};



struct GetMatrixRow : Vector {
	Mat a;
	size_t index;

	GetMatrixRow(size_t s = 0) {
		size = s;
		value = std::vector<NUM_TYPE>(s, 0.0f);
		partial = std::vector<NUM_TYPE>(s, 0.0f);
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

	void updateGradientFunction() override final {
		a->gradientFunction = MatrixAddAtPos::build(a->gradientFunction, gradientFunction, index);
	}
};


Vec Matrix::get(size_t index = 0) {
	return GetMatrixRow::build(std::dynamic_pointer_cast<Matrix>(shared_from_this()), index);
}

Vec Mat::operator [] (size_t index) const {
	return GetMatrixRow::build(*this, index);
}


// each vector will be a row of the resulting matrix
struct MatrixFromVectors : Matrix {
	std::vector<Vec> a;

	MatrixFromVectors(size_t r = 0, size_t c = 0, NUM_TYPE fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<NUM_TYPE>>(r, std::vector<NUM_TYPE>(c, fillValue));
		partial = std::vector<std::vector<NUM_TYPE>>(r, std::vector<NUM_TYPE>(c, 0.0f));
	}


	static Mat build(const std::vector<Vec>& vecs) {

		std::shared_ptr<MatrixFromVectors> node = std::make_shared<MatrixFromVectors>(vecs.size(), vecs[0]->size);

		node->a = vecs;
		for (size_t i = 0; i < vecs.size(); ++i) {
			node->parents.push_back(vecs[i]);
			node->value[i] = vecs[i]->value;
		}

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < rows; ++i) {
			value[i] = a[i]->value;
		}
	}

	void derive() override final {
		for (size_t i = 0; i < rows; ++i) {
			a[i]->partial += partial[i];
		}
	}


	void updateGradientFunction() override final {
		for (size_t i = 0; i < rows; ++i) {
			a[i]->gradientFunction = a[i]->gradientFunction + gradientFunction->get(i);
		}
	}
};


#endif
