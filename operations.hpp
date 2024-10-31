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

	void updateGradientFunction() override final {
		a->gradientFunction = Add::build(a->gradientFunction, gradientFunction);
		b->gradientFunction = Add::build(b->gradientFunction, gradientFunction);
	}
};

struct Subtract : Scalar {

	Var a, b;

	Subtract() {}

	static Var build(const Var& v1, const Var& v2) {

		std::shared_ptr<Subtract> node = std::make_shared<Subtract>();

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
		value = a->value - b->value;
	}

	void derive() override final {
		a->partial += partial;
		b->partial -= partial;
	}

	void updateGradientFunction() override final {
		a->gradientFunction = Add::build(a->gradientFunction, gradientFunction);
		b->gradientFunction = Subtract::build(b->gradientFunction, gradientFunction);
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

	void updateGradientFunction() override final {
		a->gradientFunction = Add::build(a->gradientFunction, Mult::build(gradientFunction, b));
		b->gradientFunction = Add::build(b->gradientFunction, Mult::build(gradientFunction, a));
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

	void updateGradientFunction() override final {

		Var inv = Div::build(Scalar::build(1.0f), Mult::build(b, b));

		a->gradientFunction = Add::build(a->gradientFunction, Mult::build(Mult::build(gradientFunction, b), inv));
		b->gradientFunction = Subtract::build(b->gradientFunction, Mult::build(Mult::build(gradientFunction, a), inv));
	}
};


struct Sin;
struct Cos;

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

	void updateGradientFunction() override final;
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

	void updateGradientFunction() override final;
};


void Sin::updateGradientFunction() {
	a->gradientFunction = Add::build(a->gradientFunction, Mult::build(gradientFunction, Cos::build(a)));
}

void Cos::updateGradientFunction() {
	a->gradientFunction = Subtract::build(a->gradientFunction, Mult::build(gradientFunction, Sin::build(a)));
}







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

	void updateGradientFunction() override final {
		a->gradientFunction = Add::build(a->gradientFunction, Mult::build(gradientFunction, std::static_pointer_cast<Scalar>(shared_from_this())));
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

	void updateGradientFunction() override final {
		a->gradientFunction = Add::build(a->gradientFunction, Div::build(gradientFunction, a));
	}
};



struct Sqrt : Scalar {

	Var a;

	Sqrt() {}

	static Var build(const Var& v) {

		std::shared_ptr<Sqrt> node = std::make_shared<Sqrt>();

		node->a = v;
		#if USE_NAME
			node->name = "sqrt(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = std::sqrt(a->value);
	}

	void derive() override final {
		a->partial += partial / (2.0f * value);
	}

	void updateGradientFunction() override final {
		a->gradientFunction = Add::build(a->gradientFunction, Div::build(gradientFunction, Mult::build(std::static_pointer_cast<Scalar>(shared_from_this()), Scalar::build(2.0f))));
	}
};


inline Var operator + (const Var& v1, const Var& v2) {
	return Add::build(v1, v2);
}

inline Var operator - (const Var& v1, const Var& v2) {
	return Subtract::build(v1, v2);
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

inline Var operator / (const Var& v, float f) {
	return Div::build(v, Scalar::build(f));
}

Var sqrt(const Var& v) {
	return Sqrt::build(v);
}

Var log(const Var& v) {
	return Ln::build(v);
}

Var exp(const Var& v) {
	return Exp::build(v);
}

Var sin(const Var& v) {
	return Sin::build(v);
}

Var cos(const Var& v) {
	return Cos::build(v);
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



struct VecDivVec : Vector {

	Vec a, b;

	VecDivVec(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Vec& v2) {

		std::shared_ptr<VecDivVec> node = std::make_shared<VecDivVec>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = v1->name + " / " + v2->name;
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->size; ++i) {
			value[i] = a->value[i] / b->value[i];
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {

			float inv = 1.0f / (b->value[i] * b->value[i]);

			a->partial[i] += b->value[i] * inv * partial[i];
			b->partial[i] -= a->value[i] * inv * partial[i];
		}
	}
};

inline Vec operator / (const Vec& v1, const Vec& v2) {
	return VecDivVec::build(v1, v2);
}


struct VecDivVar : Vector {

	Vec a;
	Var b;

	VecDivVar(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Var& v2) {

		std::shared_ptr<VecDivVar> node = std::make_shared<VecDivVar>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = v1->name + " / " + v2->name;
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		float inv = 1.0f / b->value;
		for (size_t i = 0; i < a->size; ++i) {
			value[i] = a->value[i] * inv;
		}
	}

	void derive() override final {
		float inv = 1.0f / (b->value * b->value);

		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += b->value * inv * partial[i];
			b->partial -= a->value[i] * inv * partial[i];
		}
	}
};

inline Vec operator / (const Vec& v1, const Var& v2) {
	return VecDivVar::build(v1, v2);
}


struct VecMultVar : Vector {

	Vec a;
	Var b;

	VecMultVar(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Var& v2) {

		std::shared_ptr<VecMultVar> node = std::make_shared<VecMultVar>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = v1->name + " * " + v2->name;
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->size; ++i) {
			value[i] = a->value[i] * b->value;
		}
	}

	void derive() override final {

		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += b->value * partial[i];
			b->partial += partial[i] * a->value[i];
		}
	}
};

inline Vec operator * (const Vec& v1, const Var& v2) {
	return VecMultVar::build(v1, v2);
}






struct VecAddVar : Vector {

	Vec a;
	Var b;

	VecAddVar(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Var& v2) {

		std::shared_ptr<VecAddVar> node = std::make_shared<VecAddVar>(v1->size);

		node->a = v1;
		node->b = v2;
		#if USE_NAME
			node->name = v1->name + " + " + v2->name;
		#endif

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->size; ++i) {
			value[i] = a->value[i] + b->value;
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += partial[i];
			b->partial += partial[i];
		}
	}
};

inline Vec operator + (const Vec& v1, const Var& v2) {
	return VecAddVar::build(v1, v2);
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


struct VecMinusVar : Vector {

	Vec a;
	Var b;

	VecMinusVar(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Var& v2) {

		std::shared_ptr<VecMinusVar> node = std::make_shared<VecMinusVar>(v1->size);

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
			value[i] = a->value[i] - b->value;
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += partial[i];
			b->partial -= partial[i];
		}
	}
};

inline Vec operator - (const Vec& v1, const Var& v2) {
	return VecMinusVar::build(v1, v2);
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





struct VecExp : Vector {

	Vec a;

	VecExp(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v) {

		std::shared_ptr<VecExp> node = std::make_shared<VecExp>(v->size);

		node->a = v;
		#if USE_NAME
			node->name = "exp(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = std::exp(a->value[i]);
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += value[i] * partial[i];
		}
	}
};

Vec exp(const Vec& v) {
	return VecExp::build(v);
}


struct VecLog : Vector {

	Vec a;

	VecLog(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v) {

		std::shared_ptr<VecLog> node = std::make_shared<VecLog>(v->size);

		node->a = v;
		#if USE_NAME
			node->name = "log(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = std::log(a->value[i]);
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a->partial[i] += partial[i] / a->value[i];
		}
	}
};

Vec log(const Vec& v) {
	return VecLog::build(v);
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



















struct VecMax : Scalar {

	Vec a;
	size_t maxIndex;

	VecMax() {

	}

	static Var build(const Vec& v) {

		std::shared_ptr<VecMax> node = std::make_shared<VecMax>();

		node->a = v;
		#if USE_NAME
			node->name = "max(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {

		maxIndex = 0.0f;
		value = a->value[0];

		for (size_t i = 1; i < a->size; ++i) {
			if (a->value[i] > value) {
				value = a->value[i];
				maxIndex = i;
			}
		}

	}

	void derive() override final {
		a->partial[maxIndex] += partial;
	}
};

Var max(const Vec& v) {
	return VecMax::build(v);
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

					b->partial[j][k] += a->value[i][j] * partial[i][k];
				}
			}
		}

	/*	for (size_t i = 0; i < n; ++i) {
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
		}*/
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











struct VecSum : Scalar {

	Vec a;

	VecSum() {

	}

	static Var build(const Vec& v) {

		std::shared_ptr<VecSum> node = std::make_shared<VecSum>();

		node->a = v;
		#if USE_NAME
			node->name = "sum(" + v->name + ")";
		#endif

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {

		value = 0.0f;

		for (size_t i = 0; i < a->size; ++i) {
			value += a->value[i];
		}
	}

	void derive() override final {
		for (size_t i = 0; i < a->size; ++i) {
			a->partial[i] += partial;
		}
	}
};

inline Var sum(const Vec& v) {
	return VecSum::build(v);
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
