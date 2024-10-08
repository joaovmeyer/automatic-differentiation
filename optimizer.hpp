#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "scalar.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include <iostream>
#include <variant>



using ScalarType = float;
using VectorType = std::vector<float>;
using MatrixType = std::vector<std::vector<float>>;

ScalarType zerosLike(const ScalarType& a) {
	return 0.0f;
}

VectorType zerosLike(const VectorType& a) {
	return VectorType(a.size(), 0.0f);
}

MatrixType zerosLike(const MatrixType& a) {
	return MatrixType(a.size(), VectorType(a[0].size(), 0.0f));
}


template <template<typename> class OptimizerParam>
struct Optimizer {

	std::vector<std::shared_ptr<Scalar>> scalarParameters;
	std::vector<std::shared_ptr<Vector>> vectorParameters;
	std::vector<std::shared_ptr<Matrix>> matrixParameters;

	std::vector<OptimizerParam<ScalarType>> optimScalar;
	std::vector<OptimizerParam<VectorType>> optimVector;
	std::vector<OptimizerParam<MatrixType>> optimMatrix;

	std::shared_ptr<Node> func;

	template<class... Types>
	Optimizer(const std::shared_ptr<Node>& f, Types... args) : func(f) {

		// doing this to get every parameter that affects the function f
		std::vector<std::shared_ptr<Node>> ordering = f->topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {

			// do not want to optimize non-trainable nodes
			if (ordering[i]->isTrainable) {

				// separate them in scalar, vector and matrix
				switch (ordering[i]->getType()) {
					case Node::SCALAR:
						scalarParameters.push_back(std::dynamic_pointer_cast<Scalar>(ordering[i]));
						optimScalar.push_back(OptimizerParam<ScalarType>(scalarParameters.back()->value, args...));
						break;
					case Node::VECTOR:
						vectorParameters.push_back(std::dynamic_pointer_cast<Vector>(ordering[i]));
						optimVector.push_back(OptimizerParam<VectorType>(vectorParameters.back()->value, args...));
						break;
					case Node::MATRIX:
						matrixParameters.push_back(std::dynamic_pointer_cast<Matrix>(ordering[i]));
						optimMatrix.push_back(OptimizerParam<MatrixType>(matrixParameters.back()->value, args...));
						break;
				}

			}
		}
	}


	template<class... Types>
	Optimizer(const std::vector<std::shared_ptr<Node>>& params, Types... args) : func(nullptr) {

		for (size_t i = 0; i < params.size(); ++i) {

			// separate them in scalar, vector and matrix
			switch (params[i]->getType()) {
				case Node::SCALAR:
					scalarParameters.push_back(std::dynamic_pointer_cast<Scalar>(params[i]));
					optimScalar.push_back(OptimizerParam<ScalarType>(scalarParameters.back()->value, args...));
					break;
				case Node::VECTOR:
					vectorParameters.push_back(std::dynamic_pointer_cast<Vector>(params[i]));
					optimVector.push_back(OptimizerParam<VectorType>(vectorParameters.back()->value, args...));
					break;
				case Node::MATRIX:
					matrixParameters.push_back(std::dynamic_pointer_cast<Matrix>(params[i]));
					optimMatrix.push_back(OptimizerParam<MatrixType>(matrixParameters.back()->value, args...));
					break;
			}
		}
	}


	void optimize(int maxIter) {

		if (!func) return;

		for (int iter = 0; iter < maxIter; ++iter) {
			prepare();
			func->calculateDerivatives();
			step();
		}
	}



	void step() {
		for (size_t i = 0; i < optimScalar.size(); ++i) {
			optimScalar[i].step(scalarParameters[i]->value, scalarParameters[i]->partial);
		}
		for (size_t i = 0; i < optimVector.size(); ++i) {
			optimVector[i].step(vectorParameters[i]->value, vectorParameters[i]->partial);
		}
		for (size_t i = 0; i < optimMatrix.size(); ++i) {
			optimMatrix[i].step(matrixParameters[i]->value, matrixParameters[i]->partial);
		}
	}

	void prepare() {
		for (size_t i = 0; i < optimScalar.size(); ++i) {
			optimScalar[i].prepare(scalarParameters[i]->value);
		}
		for (size_t i = 0; i < optimVector.size(); ++i) {
			optimVector[i].prepare(vectorParameters[i]->value);
		}
		for (size_t i = 0; i < optimMatrix.size(); ++i) {
			optimMatrix[i].prepare(matrixParameters[i]->value);
		}
	}
};


template <typename T>
struct GradientDescent {
	float lr;

	GradientDescent(const T& param, float lr = 0.01f) : lr(-lr) {
		
	}

	void step(T& param, const T& partial) {
		param += partial * lr;
	}

	void prepare(T& param) {}
};




template <typename T>
struct Momentum {
	float a, b;
	T v;

	Momentum(const T& param, float a = 0.1f, float b = 0.9f) : a(a), b(b), v(zerosLike(param)) {
		
	}

	void step(T& param, const T& partial) {
		v = v * b + partial * a;
		param += v * -1.0f;
	}

	void prepare(T& param) {}
};






ScalarType sqrt(const ScalarType& a) {
	return std::sqrt(a);
}
VectorType sqrt(VectorType a) {

	for (size_t i = 0; i < a.size(); ++i) {
		a[i] = std::sqrt(a[i]);
	}

	return a;
}
MatrixType sqrt(MatrixType a) {

	for (size_t i = 0; i < a.size(); ++i) {
		for (size_t j = 0; j < a[0].size(); ++j) {
			a[i][j] = std::sqrt(a[i][j]);
		}
	}

	return a;
}



ScalarType hadamard(const ScalarType& a, const ScalarType& b) {
	return a * b;
}
VectorType hadamard(VectorType a, const VectorType& b) {

	for (size_t i = 0; i < a.size(); ++i) {
		a[i] *= b[i];
	}

	return a;
}
MatrixType hadamard(MatrixType a, const MatrixType& b) {

	for (size_t i = 0; i < a.size(); ++i) {
		for (size_t j = 0; j < a[0].size(); ++j) {
			a[i][j] *= b[i][j];
		}
	}

	return a;
}


// division for adagrad special
ScalarType div(float n, const ScalarType& a) {
	return n / (std::sqrt(a) + 1e-8);
}
VectorType div(float n, VectorType a) {

	for (size_t i = 0; i < a.size(); ++i) {
		a[i] = n / (std::sqrt(a[i]) + 1e-8);
	}

	return a;
}
MatrixType div(float n, MatrixType a) {

	for (size_t i = 0; i < a.size(); ++i) {
		for (size_t j = 0; j < a[0].size(); ++j) {
			a[i][j] = n / (std::sqrt(a[i][j]) + 1e-8);
		}
	}

	return a;
}


template <typename T>
struct AdaGrad {
	float n;
	T G;

	AdaGrad(const T& param, float n = 0.1f) : n(n), G(zerosLike(param)) {
		
	}

	void step(T& param, const T& partial) {

		G += hadamard(partial, partial);

		param += hadamard(div(-n, G), partial);
	}

	void prepare(T& param) {}
};






template <typename T>
struct Adam {
	float n;
	float b1, b2;
	float b1_power_t = 1.0f, b2_power_t = 1.0f;

	T v;
	T m;

	Adam(const T& param, float n = 0.01f, float b1 = 0.9f, float b2 = 0.999f) : n(n), b1(b1), b2(b2), v(zerosLike(param)), m(zerosLike(param)) {
		
	}

	void step(T& param, const T& partial) {

		b1_power_t *= b1;
		b2_power_t *= b2;

		m = m * b1 + partial * (1.0f - b1);
		v = v * b2 + hadamard(partial, partial) * (1.0f - b2);

		T m_hat = m * (1.0f / (1.0f - b1_power_t));
		T v_hat = v * (1.0f / (1.0f - b2_power_t));

		param += hadamard(div(-n, v_hat), m_hat);
	}

	void prepare(T& param) {}
};








template <typename T>
struct RMSProp {
	float a, b;
	T V;

	RMSProp(const T& param, float a = 0.01f, float b = 0.9f) : a(a), b(b), V(zerosLike(param)) {
		
	}

	void step(T& param, const T& partial) {

		V = V * b + hadamard(partial, partial) * (1.0f - b);

		param += hadamard(div(-a, V), partial);
	}

	void prepare(T& param) {}
};



// I think this is wrong (?) is working really poorly in my tests, but it seems like the formula is correct??
template <typename T>
struct NAG {
	float a, b;
	T V;

	NAG(const T& param, float a = 0.01f, float b = 0.9f) : a(a), b(b), V(zerosLike(param)) {
		
	}

	void step(T& param, const T& partial) {
		param += partial * a;
		V = V * b + partial * a;
	}

	// anticipate the next step
	void prepare(T& param) {
		param += V * -b;
	}
};



#endif
