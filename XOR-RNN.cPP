
#include "operations.hpp"
#include "../rng.h"

#include <iostream>

using namespace std;


int main() {

	int hiddenSize = 25;

	// first layer
	Mat W1 = Matrix::build(hiddenSize, 1);
	Mat U1 = Matrix::build(hiddenSize, hiddenSize);
	Vec b1 = Vector::build(hiddenSize, 0.1f);

	// second layer
	Mat W2 = Matrix::build(1, hiddenSize);
	Vec b2 = Vector::build(1, 0.1f);

	// initialize weights;
	for (int i = 0; i < W1->rows; ++i) {
		for (int j = 0; j < W1->cols; ++j) {
			W1->value[i][j] = rng::fromNormalDistribution(0.0, 1.0);
		}

		for (int j = 0; j < W1->rows; ++j) {
			U1->value[i][j] = rng::fromNormalDistribution(0.0, 0.3);
		}

		W2->value[0][i] = rng::fromNormalDistribution(0.0, 1.0);
	}

	Vec out_prev;
	Vec y = Vector::build(1);

	vector<vector<float>> X = {
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }
	};
	vector<vector<float>> Y = {
		{ 0.0f },
		{ 1.0f },
		{ 1.0f },
		{ 0.0f }
	};

	float lr = -0.5f;
	for (int iter = 0; iter < 25000; ++iter) {

		vector<float> sequence = X[iter % X.size()];
		y->value = Y[iter % Y.size()];

		out_prev = tanh(W1 * Vector::build(1, sequence[0]) + b1);
		for (size_t j = 1; j < sequence.size(); ++j) {
			out_prev = tanh(W1 * Vector::build(1, sequence[j]) + U1 * out_prev + b1);
		}

		Vec out = sigmoid(W2 * out_prev + b2);

		Vec err = (out - y);
		Var loss = err * err;

		loss->calculateDerivatives();

		W1->value += W1->partial * lr;
		U1->value += U1->partial * lr;
		b1->value += b1->partial * lr;

		W2->value += W2->partial * lr;
		b2->value += b2->partial * lr;
	}

	float l = 0.0f;
	for (size_t i = 0; i < X.size(); ++i) {
		vector<float> sequence = X[i];
		y->value = Y[i];

		out_prev = tanh(W1 * Vector::build(1, sequence[0]) + b1);
		for (size_t j = 1; j < sequence.size(); ++j) {
			out_prev = tanh(W1 * Vector::build(1, sequence[j]) + U1 * out_prev + b1);
		}

		Vec out = sigmoid(W2 * out_prev + b2);

		Vec err = (out - y);
		Var loss = err * err;

		loss->eval();
		l += loss->value;

		cout << "Expected result: " << y->value << ", got: " << out->value << "\n";
	}
	cout << "MSE: " << l << "\n";

	return 0;
}
