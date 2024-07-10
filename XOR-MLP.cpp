
#include "operations.hpp"
#include "../rng.h"

#include <iostream>

using namespace std;


int main() {

	int hiddenSize = 2;

	// first layer
	Mat W1 = Matrix::build(hiddenSize, 2, 0.0f);
	Vec b1 = Vector::build(hiddenSize, 0.1f);

	// second layer
	Mat W2 = Matrix::build(1, hiddenSize, 0.0f);
	Vec b2 = Vector::build(1, 0.1f);

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

	float lr = -5.0f;
	for (int iter = 0; iter < 25000; ++iter) {

		loss->calculateDerivatives();

		W1->value += W1->partial * lr;
		b1->value += b1->partial * lr;

		W2->value += W2->partial * lr;
		b2->value += b2->partial * lr;
	}

	loss->eval();
	cout << "Expected result: " << y->value << ", got: " << out2->value << "\n";
	cout << "MSE: " << loss->value << "\n";

	return 0;
}
