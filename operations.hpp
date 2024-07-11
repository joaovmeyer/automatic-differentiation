#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

#include "scalar.hpp"
#include "vector.hpp"
#include "matrix.hpp"

#include <cmath>

struct Add : Scalar {

	Var a, b;

	Add() {}

	static Var build(const Var& v1, const Var& v2) {

		std::shared_ptr<Add> node = std::make_shared<Add>();

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "(" + v1->name + " + " + v2->name + ")";
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		value = a->value + b->value;
	}

	void derive() override final {
		a->partial += partial;
		b->partial += partial;
	}
};

struct Mult : Scalar {

	Var a, b;

	Mult() {}

	static Var build(const Var& v1, const Var& v2) {

		std::shared_ptr<Mult> node = std::make_shared<Mult>();

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "(" + v1->name + " * " + v2->name + ")";
		#endif
		
		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		value = a->value * b->value;
	}

	void derive() override final {
		a->partial += partial * b->value;
		b->partial += partial * a->value;
	}
};

struct Sin : Scalar {

	Var a;

	Sin() {}

	static Var build(const Var& v) {

		std::shared_ptr<Sin> node = std::make_shared<Sin>();

		node->a = v;
		#if USE_NAME
			node->name = "sin(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = std::sin(a->value);
	}

	void derive() override final {
		a->partial += partial * std::cos(a->value);
	}
};

struct Cos : Scalar {

	Var a;

	Cos() {}

	static Var build(const Var& v) {

		std::shared_ptr<Cos> node = std::make_shared<Cos>();

		node->a = v;
		#if USE_NAME
			node->name = "cos(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = std::cos(a->value);
	}

	void derive() override final {
		a->partial -= partial * std::sin(a->value);
	}
};

struct Exp : Scalar {

	Var a;

	Exp() {}

	static Var build(const Var& v) {

		std::shared_ptr<Exp> node = std::make_shared<Exp>();

		node->a = v;
		#if USE_NAME
			node->name = "exp(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = std::exp(a->value);
	}

	void derive() override final {
		a->partial += partial * value;
	}
};

struct Ln : Scalar {

	Var a;

	Ln() {}

	static Var build(const Var& v) {

		std::shared_ptr<Ln> node = std::make_shared<Ln>();

		node->a = v;
		#if USE_NAME
			node->name = "ln(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = std::log(a->value);
	}

	void derive() override final {
		a->partial += partial / a->value;
	}
};

struct Div : Scalar {

	Var a, b;

	Div() {}

	static Var build(const Var& v1, const Var& v2) {

		std::shared_ptr<Div> node = std::make_shared<Div>();

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "(" + v1->name + " / " + v2->name + ")";
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		value = a->value / b->value;
	}

	void derive() override final {
		float inv = 1.0f / (b->value * b->value);
		a->partial += partial * b->value * inv;
		b->partial -= partial * a->value * inv;
	}
};


inline Var operator + (const Var& v1, const Var& v2) {
	return Add::build(v1, v2);
}

inline Var operator * (const Var& v1, const Var& v2) {
	return Mult::build(v1, v2);
}

inline Var operator / (const Var& v1, const Var& v2) {
	return Div::build(v1, v2);
}


inline Var operator + (const Var& v, float f) {
	return Add::build(v, Scalar::build(f));
}

inline Var operator * (const Var& v, float f) {
	return Mult::build(v, Scalar::build(f));
}

inline Var operator / (float f, const Var& v) {
	return Div::build(Scalar::build(f), v);
}







struct VecDotVec : Scalar {

	Vec a, b;

	VecDotVec() {}

	static Var build(const Vec& v1, const Vec& v2) {

		std::shared_ptr<VecDotVec> node = std::make_shared<VecDotVec>();

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "(" + v1->name + " * " + v2->name + ")";
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		value = 0.0f;

		for (size_t i = 0; i < a->size; ++i) {
			value += a->value[i] * b->value[i];
		}
	}

	void derive() override final {
		a->partial += b->value * partial;
		b->partial += a->value * partial;
	}
};

inline Var operator * (const Vec& v1, const Vec& v2) {
	return VecDotVec::build(v1, v2);
}





struct VecHadamardVec : Vector {

	Vec a, b;

	VecHadamardVec(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Vec& v2) {

		std::shared_ptr<VecHadamardVec> node = std::make_shared<VecHadamardVec>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "hadamard(" + v1->name + ", " + v2->name + ")";
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->size; ++i) {
			value[i] = a->value[i] * b->value[i];
		}
	}

	void derive() override final {

		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += b->value[i] * partial[i];
			b->partial[i] += a->value[i] * partial[i];
		}
	}
};

inline Vec hadamard(const Vec& v1, const Vec& v2) {
	return VecHadamardVec::build(v1, v2);
}






struct VecMinusVec : Vector {

	Vec a, b;

	VecMinusVec(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Vec& v2) {

		std::shared_ptr<VecMinusVec> node = std::make_shared<VecMinusVec>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "(" + v1->name + " - " + v2->name + ")";
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = a->value[i] - b->value[i];
		}
	}

	void derive() override final {
		a->partial += partial;
		b->partial += partial * -1.0f;
	}
};

inline Vec operator - (const Vec& v1, const Vec& v2) {
	return VecMinusVec::build(v1, v2);
}




struct VecPlusVec : Vector {

	Vec a, b;

	VecPlusVec(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Vec& v2) {

		std::shared_ptr<VecPlusVec> node = std::make_shared<VecPlusVec>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = "(" + v1->name + " + " + v2->name + ")";
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = a->value[i] + b->value[i];
		}
	}

	void derive() override final {
		a->partial += partial;
		b->partial += partial;
	}
};

inline Vec operator + (const Vec& v1, const Vec& v2) {
	return VecPlusVec::build(v1, v2);
}






struct VecTanh : Vector {

	Vec a;

	VecTanh(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v) {

		std::shared_ptr<VecTanh> node = std::make_shared<VecTanh>(v->size);

		node->a = v;
		#if USE_NAME
			node->name = "tanh(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = 2.0f / (1.0f + std::exp(-2.0f * a->value[i])) - 1.0f;
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += (1.0f - value[i] * value[i]) * partial[i];
		}
	}
};

Vec tanh(const Vec& v) {
	return VecTanh::build(v);
}




struct VecSigmoid : Vector {

	Vec a;

	VecSigmoid(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v) {

		std::shared_ptr<VecSigmoid> node = std::make_shared<VecSigmoid>(v->size);

		node->a = v;
		#if USE_NAME
			node->name = "sigmoid(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = 1.0f / (1.0f + std::exp(-a->value[i]));
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += value[i] * (1.0f - value[i]) * partial[i];
		}
	}
};

Vec sigmoid(const Vec& v) {
	return VecSigmoid::build(v);
}






struct MatSigmoid : Matrix {

	Mat a;

	MatSigmoid(size_t r = 0, size_t c = 0, float fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue));
		partial = std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f));
	}

	static Mat build(const Mat& m) {

		std::shared_ptr<MatSigmoid> node = std::make_shared<MatSigmoid>(m->rows, m->cols);

		node->a = m;
		#if USE_NAME
			node->name = "sigmoid(" + m->name + ")";
		#endif

		node->parents.push_back(m);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				value[i][j] = 1.0f / (1.0f + std::exp(-a->value[i][j]));
			}
		}
	}

	void derive() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				a->partial[i][j] += value[i][j] * (1.0f - value[i][j]) * partial[i][j];
			}
		}
	}
};

Mat sigmoid(const Mat& m) {
	return MatSigmoid::build(m);
}





















struct MatDotVec : Vector {

	Mat a;
	Vec b;

	MatDotVec(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Mat& m, const Vec& v) {

		std::shared_ptr<MatDotVec> node = std::make_shared<MatDotVec>(m->rows);

		node->a = m;
		node->b = v;
		#if USE_NAME
			node->name = "(" + m->name + " * " + v->name + ")";
		#endif

		node->parents.push_back(m);
		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
	//	value = std::vector<float>(size, 0.0f);

		for (size_t i = 0; i < a->rows; ++i) {
			value[i] = 0.0f;
			for (size_t j = 0; j < a->cols; ++j) {
				value[i] += a->value[i][j] * b->value[j];
			}
		}
	}

	void derive() override final {

		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {

				// b->partial = A^T * partial
				b->partial[j] += a->value[i][j] * partial[i];

				a->partial[i][j] += b->value[j] * partial[i];
			}
		}
	}
};

inline Vec operator * (const Mat& m, const Vec& v) {
	return MatDotVec::build(m, v);
}




struct MatDotMat : Matrix {

	Mat a, b;

	MatDotMat(size_t r = 0, size_t c = 0, float fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue));
		partial = std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f));
	}

	static Mat build(const Mat& m1, const Mat& m2) {

		std::shared_ptr<MatDotMat> node = std::make_shared<MatDotMat>(m1->rows, m2->cols);

		node->a = m1;
		node->b = m2;
		#if USE_NAME
			node->name = "(" + m1->name + " * " + m2->name + ")";
		#endif

		node->parents.push_back(m1);
		node->parents.push_back(m2);

		return node;
	}

	void evaluate() override final {

		size_t n = a->rows;
		size_t p = a->cols;
		size_t m = b->cols;

		for (size_t i = 0; i < n; ++i) {

			// reset i-th row
			std::fill(value[i].begin(), value[i].end(), 0.0f);

			for (size_t k = 0; k < p; ++k) {
				for (size_t j = 0; j < m; ++j) {
					value[i][j] += a->value[i][k] * b->value[k][j];
				}
			}
		}
	}

	void derive() override final {

		size_t n = a->rows;
		size_t p = a->cols;
		size_t m = b->cols;

		// A: (n, p), B: (p, m), C: (n, m)

		// a->partial = partial * b->value^T
		// b->partial = a->value^T * partial

		for (size_t i = 0; i < n; ++i) {
			for (size_t j = 0; j < p; ++j) {
				for (size_t k = 0; k < m; ++k) {
					a->partial[i][j] += partial[i][k] * b->value[j][k];
				}
			}
		}

		for (size_t i = 0; i < p; ++i) {
			for (size_t k = 0; k < n; ++k) {
				for (size_t j = 0; j < m; ++j) {
					b->partial[i][j] += a->value[k][i] * partial[k][j];
				}
			}
		}
	}
};

inline Mat operator * (const Mat& m1, const Mat& m2) {
	return MatDotMat::build(m1, m2);
}





struct TransposeMat : Matrix {

	Mat a;

	TransposeMat(size_t r = 0, size_t c = 0, float fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue));
		partial = std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f));
	}

	static Mat build(const Mat& m) {

		std::shared_ptr<TransposeMat> node = std::make_shared<TransposeMat>(m->cols, m->rows);

		node->a = m;
		#if USE_NAME
			node->name = m->name + "^T";
		#endif

		node->parents.push_back(m);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				value[i][j] = a->value[j][i];
			}
		}
	}

	void derive() override final {
		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				partial[i][j] += a->partial[j][i];
			}
		}
	}
};

inline Mat transpose(const Mat& m) {
	return TransposeMat::build(m);
}




// adds the vec to every column of the matrix
struct MatPlusVec : Matrix {

	Mat a;
	Vec b;

	MatPlusVec(size_t r = 0, size_t c = 0, float fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue));
		partial = std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f));
	}

	static Mat build(const Mat& m, const Vec& v) {

		std::shared_ptr<MatPlusVec> node = std::make_shared<MatPlusVec>(m->rows, m->cols);

		node->a = m;
		node->b = v;
		#if USE_NAME
			node->name = "(" + m->name + " - " + v->name + ")";
		#endif

		node->parents.push_back(m);
		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				value[i][j] = a->value[i][j] + b->value[i];
			}
		}
	}

	void derive() override final {
		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				a->partial[i][j] += partial[i][j];
				b->partial[i] += partial[i][j];
			}
		}
	}
};

inline Mat operator + (const Mat& m, const Vec& v) {
	return MatPlusVec::build(m, v);
}











struct MatMinusMat : Matrix {

	Mat a, b;

	MatMinusMat(size_t r = 0, size_t c = 0, float fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue));
		partial = std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f));
	}

	static Mat build(const Mat& m1, const Mat& m2) {

		std::shared_ptr<MatMinusMat> node = std::make_shared<MatMinusMat>(m1->rows, m1->cols);

		node->a = m1;
		node->b = m2;
		#if USE_NAME
			node->name = "(" + m1->name + " - " + m2->name + ")";
		#endif

		node->parents.push_back(m1);
		node->parents.push_back(m2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				value[i][j] = a->value[i][j] - b->value[i][j];
			}
		}
	}

	void derive() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				a->partial[i][j] += partial[i][j];
				b->partial[i][j] -= partial[i][j];
			}
		}
	}
};

inline Mat operator - (const Mat& m1, const Mat& m2) {
	return MatMinusMat::build(m1, m2);
}









struct MatHadamardMat : Matrix {

	Mat a, b;

	MatHadamardMat(size_t r = 0, size_t c = 0, float fillValue = 0.0f) {
		rows = r;
		cols = c;

		value = std::vector<std::vector<float>>(r, std::vector<float>(c, fillValue));
		partial = std::vector<std::vector<float>>(r, std::vector<float>(c, 0.0f));
	}

	static Mat build(const Mat& m1, const Mat& m2) {

		std::shared_ptr<MatHadamardMat> node = std::make_shared<MatHadamardMat>(m1->rows, m1->cols);

		node->a = m1;
		node->b = m2;
		#if USE_NAME
			node->name = "hadamard(" + m1->name + ", " + m2->name + ")";
		#endif

		node->parents.push_back(m1);
		node->parents.push_back(m2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				value[i][j] = a->value[i][j] * b->value[i][j];
			}
		}
	}

	void derive() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				a->partial[i][j] += b->value[i][j] * partial[i][j];
				b->partial[i][j] += a->value[i][j] * partial[i][j];
			}
		}
	}
};

inline Mat hadamard(const Mat& m1, const Mat& m2) {
	return MatHadamardMat::build(m1, m2);
}











struct MatSum : Scalar {

	Mat a;

	MatSum() {

	}

	static Var build(const Mat& m) {

		std::shared_ptr<MatSum> node = std::make_shared<MatSum>();

		node->a = m;
		#if USE_NAME
			node->name = "sum(" + m->name + ")";
		#endif

		node->parents.push_back(m);

		return node;
	}

	void evaluate() override final {

		value = 0.0f;

		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				value += a->value[i][j];
			}
		}
	}

	void derive() override final {
		for (size_t i = 0; i < a->rows; ++i) {
			for (size_t j = 0; j < a->cols; ++j) {
				a->partial[i][j] += partial;
			}
		}
	}
};

inline Var sum(const Mat& m) {
	return MatSum::build(m);
}








#endif
