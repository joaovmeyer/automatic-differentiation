
#include "operations.hpp"
#include "optimizer.hpp"
#include "../rng.h"

#include <iostream>

using namespace std;


int main() {

	int hiddenSize = 2;

	// first layer
	Mat W1 = Matrix::build(hiddenSize, 2, 0.0f, true);
	Vec b1 = Vector::build(hiddenSize, 0.1f, true);

	// second layer
	Mat W2 = Matrix::build(1, hiddenSize, 0.0f, true);
	Vec b2 = Vector::build(1, 0.1f, true);

	// initialize weights;
	for (int i = 0; i < W1->rows; ++i) {
		for (int j = 0; j < W1->cols; ++j) {
			W1->value[i][j] = rng::fromNormalDistribution(0.0, 1.0);
		}
		W2->value[0][i] = rng::fromNormalDistribution(0.0, 1.0);
	}

	int batchSize = 4;

	Mat x = Matrix::build(2, batchSize, 0.0f);
	Mat y = Matrix::build(1, batchSize, 0.0f);

	Mat out1 = sigmoid(W1 * x + b1);
	Mat out2 = sigmoid(W2 * out1 + b2);

	Mat err = (out2 - y);
	Var loss = sum(hadamard(err, err)); // MSE

	x->value = {
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f }
	};

	y->value = {
		{ 0.0f, 1.0f, 1.0f, 0.0f }
	};


	Optimizer<Momentum> optimizer(loss, 2.0f, 0.7f);


	float lr = -5.0f;
	for (int iter = 0; iter < 500; ++iter) {

		loss->calculateDerivatives();

		optimizer.step();
	}

	loss->eval();
	cout << "Expected result: " << y->value << ", got: " << out2->value << "\n";
	cout << "MSE: " << loss->value << "\n";

	return 0;
}
