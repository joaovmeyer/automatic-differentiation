
#include "operations.hpp"
#include "../rng.h"

#include <iostream>

using namespace std;


int main() {

	// first layer
	Mat W1 = Matrix::build(2, 2, 0.0f, "W1");
	Vec b1 = Vector::build(2, 0.1f, "b1");

	// second layer
	Mat W2 = Matrix::build(1, 2, 0.0f, "W2");
	Vec b2 = Vector::build(1, 0.1f, "b2");

	// initialize weights;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			W1->value[i][j] = rng::fromNormalDistribution(0.0, 0.5);
		}
		W2->value[0][i] = rng::fromNormalDistribution(0.0, 0.5);;
	}

	Vec x = Vector::build(2, 0.0f, "x");
	Vec y = Vector::build(1, 0.0f, "y");

	Vec out1 = tanh(W1 * x + b1);
	Vec out2 = tanh(W2 * out1 + b1);

	Vec err = (out2 - y);
	Var loss = err * err;

	vector<vector<float>> X = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f }
	};
	vector<vector<float>> Y = {
		{ 0.0f },
		{ 1.0f },
		{ 1.0f },
		{ 0.0f }
	};

	float lr = -0.03f;
	for (int iter = 0; iter < 100000; ++iter) {

		x->value = X[iter % X.size()];
		y->value = Y[iter % Y.size()];

		loss->calculateDerivatives();

		W1->value += W1->partial * lr;
		b1->value += b1->partial * lr;

		W2->value += W2->partial * lr;
		b2->value += b2->partial * lr;
	}

	for (size_t i = 0; i < X.size(); ++i) {
		x->value = X[i];
		y->value = Y[i];

		out2->calculateDerivatives();

		cout << "Expected result: " << y->value << ", got: " << out2->value << "\n";
	}

	return 0;
}
