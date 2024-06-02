#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

#include "../rng.h"

using namespace std;


struct Expression {
	float value;
	virtual inline void evaluate() = 0;
	virtual inline void derive(float seed = 1.0) = 0;
};

using ptrExp = std::shared_ptr<Expression>;

void operator -= (const ptrExp& exp, float val) {
	exp->value -= val;
}



struct Variable: public Expression {
	float partial = 0.0;

	Variable(float v) {
		value = v;
	}

	void evaluate() { partial = 0.0; } // don't actually think this should be here

	void derive(float seed = 1.0) {
		partial += seed;
	}
};


struct Plus: public Expression {
	ptrExp a, b;

	Plus(ptrExp a, ptrExp b) : a(a), b(b) {

	}

	void evaluate() {
		a->evaluate();
		b->evaluate();

		value = a->value + b->value;
	}

	void derive(float seed = 1.0) {
		a->derive(seed);
		b->derive(seed);
	}

};


struct Minus: public Expression {
	ptrExp a, b;

	Minus(ptrExp a, ptrExp b) : a(a), b(b) {

	}

	void evaluate() {
		a->evaluate();
		b->evaluate();

		value = a->value - b->value;
	}

	void derive(float seed = 1.0) {
		a->derive(seed);
		b->derive(-seed);
	}

};


struct Multiply: public Expression {
	ptrExp a, b;

	Multiply(ptrExp a, ptrExp b) : a(a), b(b) {

	}

	void evaluate() {
		a->evaluate();
		b->evaluate();

		value = a->value * b->value;
	}

	void derive(float seed = 1.0) {
		a->derive(b->value * seed);
		b->derive(a->value * seed);
	}
};


struct Sigmoid: public Expression {
	ptrExp a;

	Sigmoid(ptrExp a) : a(a) {

	}

	void evaluate() {
		a->evaluate();

		value = 1.0 / (1.0 + std::exp(-a->value));
	}

	void derive(float seed = 1.0) {
		float da = value * (1.0 - value);
		a->derive(da * seed);
	}
};



ptrExp operator * (const ptrExp& exp1, const ptrExp& exp2) {
	return std::make_shared<Multiply>(exp1, exp2);
}

ptrExp operator + (const ptrExp& exp1, const ptrExp& exp2) {
	return std::make_shared<Plus>(exp1, exp2);
}

ptrExp operator - (const ptrExp& exp1, const ptrExp& exp2) {
	return std::make_shared<Minus>(exp1, exp2);
}

ptrExp sigmoid(const ptrExp& x) {
	return std::make_shared<Sigmoid>(x);
}



struct Vec {
	vector<ptrExp> data;
	size_t size;

	Vec(const vector<float>& vec) {
		for (size_t i = 0; i < vec.size(); ++i) {
			data.push_back(std::make_shared<Variable>(vec[i]));
		}

		size = vec.size();
	}
	Vec(size_t s, float fillValue = 0) {
		for (size_t i = 0; i < s; ++i) {
			data.push_back(std::make_shared<Variable>(fillValue));
		}

		size = s;
	}


	ptrExp& operator [] (int i) {
		return data[i];
	}

	// read only
	ptrExp operator [] (int i) const {
		return data[i];
	}







	static ptrExp dot(const Vec& v1, const Vec& v2) {
		ptrExp f = std::make_shared<Variable>(0.0);

		for (size_t i = 0; i < v1.size; ++i) {
			f = f + v1[i] * v2[i];
		}

		return f;
	}

	static Vec hadamard(const Vec& v1, const Vec& v2) {
		Vec ans(v1.size);

		for (size_t i = 0; i < v1.size; ++i) {
			ans[i] = v1[i] * v2[i];
		}

		return ans;
	}

	static Vec sigmoid(const Vec& v) {
		Vec ans(v.size);

		for (size_t i = 0; i < v.size; ++i) {
			ans[i] = std::make_shared<Sigmoid>(v[i]);
		}

		return ans;

	}


	vector<float> getGradient() {
		vector<float> grad(size);

		for (size_t i = 0; i < size; ++i) {
			grad[i] = std::dynamic_pointer_cast<Variable>(data[i])->partial;
		}

		return grad;
	}





	Vec operator + (const Vec& other) const {
		Vec ans(size, 0.0);

		for (size_t i = 0; i < size; ++i) {
			ans[i] = data[i] + other[i];
		}

		return ans;
	}

	Vec operator - (const Vec& other) const {
		Vec ans(size, 0.0);

		for (size_t i = 0; i < size; ++i) {
			ans[i] = data[i] - other[i];
		}

		return ans;
	}

};



struct Mat {
	vector<Vec> data;
	size_t rows, cols;


	Mat(const vector<vector<float>>& mat) {
		for (size_t i = 0; i < mat.size(); ++i) {
			data.push_back(Vec(mat[i].size(), 0.0f));

			for (size_t j = 0; j < mat[i].size(); ++j){
				data[i][j] = std::make_shared<Variable>(mat[i][j]);
			}
		}

		rows = mat.size();
		cols = mat[0].size();
	}

	Mat(size_t r, size_t c, float fillValue = 0) {
		for (size_t i = 0; i < r; ++i) {
			data.push_back(Vec(c, fillValue));
		}

		rows = r;
		cols = c;
	}


	Vec& operator [] (int i) {
		return data[i];
	}

	// read only
	Vec operator [] (int i) const {
		return data[i];
	}


	Vec operator * (const Vec& v) {
		Vec ans(rows, 0.0);

		for (size_t i = 0; i < rows; ++i) {
			ans[i] = Vec::dot(data[i], v);
		}

		return ans;
	}


	Mat operator * (const Mat& mat2) {
		Mat ans(rows, mat2.cols);

		for (size_t i = 0; i < rows; ++i) {
			for (size_t k = 0; k < cols; ++k) {
				for (size_t j = 0; j < mat2.cols; ++j) {
					ans[i][j] = ans[i][j] + data[i][k] * mat2[k][j];
				}
			}
		}

		return ans;
	}



	static Mat getRandom(size_t rows, size_t cols, float mean = 0, float stddev = 1) {
		Mat ans(rows, cols);

		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				ans[i][j]->value = rng::fromNormalDistribution(mean, stddev);
			}
		}

		return ans;
	}


	vector<vector<float>> getGradient() {
		vector<vector<float>> grad(rows, vector<float>(cols, 0.0));

		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				grad[i][j] = std::dynamic_pointer_cast<Variable>(data[i][j])->partial;
			}
		}

		return grad;
	}
};





struct SGDMomentum {
	float b = 0.9;
	float a = 0.1;

	vector<vector<float>> V;

	SGDMomentum(Mat parameter, float stepSize = 0.1) : V(vector<vector<float>>(parameter.rows, vector<float>(parameter.cols, 0.0))), a(stepSize) {
		
	}

	void step(const vector<vector<float>>& gradient, Mat& parameter) {
		for (size_t i = 0; i < parameter.rows; ++i) {
			for (size_t j = 0; j < parameter.cols; ++j) {
				V[i][j] = V[i][j] * b + gradient[i][j];

				parameter[i][j] -= V[i][j] * a;
			}
		}
	}
};




int main () {

	Mat A({
		{ 1, 2 },
		{ 2, 3 }
	});
	Mat invA(2, 2, 0.0);

	Mat C = A * invA;

	ptrExp loss = std::make_shared<Variable>(0.0);
	for (size_t i = 0; i < C.rows; ++i) {
		for (size_t j = 0; j < C.cols; ++j) {
			ptrExp signedDifference = (C[i][j] - std::make_shared<Variable>(i == j));
			loss = loss + signedDifference * signedDifference;
		}
	}

	SGDMomentum optimizer(invA, 0.1);
	for (int iter = 0; iter < 200; ++iter) {
		loss->evaluate();
		loss->derive();

		vector<vector<float>> gradInv = invA.getGradient();
		optimizer.step(gradInv, invA);
	}

	loss->evaluate();

	cout << "Inverse: \n";
	for (size_t i = 0; i < C.rows; ++i) {
		for (size_t j = 0; j < C.cols; ++j) {
			cout << invA[i][j]->value << ", ";
		}

		cout << "\n";
	}

	cout << "\nResult: \n";
	for (size_t i = 0; i < C.rows; ++i) {
		for (size_t j = 0; j < C.cols; ++j) {
			cout << C[i][j]->value << ", ";
		}

		cout << "\n";
	}


	return 0;
}
