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
		node->name = "(" + v1->name + " + " + v2->name + ")";

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
		node->name = "(" + v1->name + " * " + v2->name + ")";

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
		node->name = "sin(" + v->name + ")";

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
		node->name = "cos(" + v->name + ")";

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
		node->name = "exp(" + v->name + ")";

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
		node->name = "ln(" + v->name + ")";

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
		node->name = "(" + v1->name + " / " + v2->name + ")";

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
		node->name = "(" + v1->name + " * " + v2->name + ")";

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
		node->name = "(" + m->name + " * " + v->name + ")";

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
		node->name = "(" + v1->name + " - " + v2->name + ")";

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
		node->name = "(" + v1->name + " + " + v2->name + ")";

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
		node->name = "tanh(" + v->name + ")";

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



#endif
