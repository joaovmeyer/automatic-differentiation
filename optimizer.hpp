#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "node.hpp"
#include <iostream>


// this feels like really stupid implementation but who cares, right?

struct Optimizer {

	std::vector<std::shared_ptr<Scalar>> scalarParameters;
	std::vector<std::shared_ptr<Vector>> vectorParameters;
	std::vector<std::shared_ptr<Matrix>> matrixParameters;

	Optimizer(const std::shared_ptr<Node>& f) {

		std::vector<std::shared_ptr<Node>> ordering = f->topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			if (ordering[i]->isTrainable) {

				switch (ordering[i]->getType()) {
					case Node::SCALAR:
						scalarParameters.push_back(std::dynamic_pointer_cast<Scalar>(ordering[i]));
						break;
					case Node::VECTOR:
						vectorParameters.push_back(std::dynamic_pointer_cast<Vector>(ordering[i]));
						break;
					case Node::MATRIX:
						matrixParameters.push_back(std::dynamic_pointer_cast<Matrix>(ordering[i]));
						break;
				}

			}
		}
	}

	virtual void step() = 0;

};

using ScalarType = float;
using VectorType = std::vector<float>;
using MatrixType = std::vector<std::vector<float>>;


struct SGDMomentum : Optimizer {

	float a, b;

	std::vector<ScalarType> scalarV;
	std::vector<VectorType> vectorV;
	std::vector<MatrixType> matrixV;

	SGDMomentum(const std::shared_ptr<Node>& f, float b = 0.9f, float a = 0.1f) : Optimizer(f), a(a), b(b) {
		for (size_t i = 0; i < scalarParameters.size(); ++i) {
			scalarV.push_back(0.0f);
		}

		for (size_t i = 0; i < vectorParameters.size(); ++i) {
			vectorV.push_back(VectorType(vectorParameters[i]->size, 0.0f));
		}

		for (size_t i = 0; i < matrixParameters.size(); ++i) {
			matrixV.push_back(MatrixType(matrixParameters[i]->rows, VectorType(matrixParameters[i]->cols, 0.0f)));
		}
	}

	void step() override final {
		for (size_t i = 0; i < scalarParameters.size(); ++i) {
			scalarV[i] = scalarV[i] * b + scalarParameters[i]->partial;
			scalarParameters[i]->value += scalarV[i] * -a;
		}

		for (size_t i = 0; i < vectorParameters.size(); ++i) {
			vectorV[i] = vectorV[i] * b + vectorParameters[i]->partial;
			vectorParameters[i]->value += vectorV[i] * -a;
		}

		for (size_t i = 0; i < matrixParameters.size(); ++i) {
			matrixV[i] = matrixV[i] * b + matrixParameters[i]->partial;
			matrixParameters[i]->value += matrixV[i] * -a;
		}
	}

};





#endif
