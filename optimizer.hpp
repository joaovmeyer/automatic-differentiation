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

	template<class... Types>
	Optimizer(const std::shared_ptr<Node>& f, Types... args) {

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
};



template <typename T>
struct Momentum {
	float a, b;
	T v;

	Momentum(const T& param, float a = 0.1f, float b = 0.9f) : a(a), b(b), v(zerosLike(param)) {
		
	}

	void step(T& param, const T& partial) {
		v = v * b + partial;
		param += v * -a;
	}
};



#endif
