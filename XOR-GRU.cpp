
#include "operations.hpp"
#include "../rng.h"

#include <iostream>

using namespace std;


int main() {

	// DISABLE NAMES IN node.hpp BEFORE USING THIS!!!!

	int hiddenSize = 25;

	// GRU layer
	Mat Wz = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Uz = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 0.3);
	Vec bz = Vector::build(hiddenSize, 0.1f);

	Mat Wr = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Ur = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 0.3);
	Vec br = Vector::build(hiddenSize, 0.1f);

	Mat Wh = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Uh = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 0.3);
	Vec bh = Vector::build(hiddenSize, 0.1f);

	// fully connected layer
	Mat W = Matrix::makeRandom(1, hiddenSize, 0.0, 1.0);
	Vec b = Vector::build(1, 0.1f);



	vector<vector<float>> X = {
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }
	};

	vector<vector<Vec>> sequences(X.size(), vector<Vec>(X[0].size()));
	for (size_t i = 0; i < X.size(); ++i) {
		for (size_t j = 0; j < X[0].size(); ++j) {
			sequences[i][j] = Vector::build(1, X[i][j]);
		}
	}

	vector<Vec> Y = {
		Vector::build(1, 0.0f),
		Vector::build(1, 1.0f),
		Vector::build(1, 1.0f),
		Vector::build(1, 0.0f)
	};


	float lr = -0.5f;
	for (int iter = 0; iter < 5000; ++iter) {

		size_t trainingIndex = iter % sequences.size();
		vector<Vec> sequence = sequences[trainingIndex];

		Vec z = sigmoid(Wz * sequence[0] + bz);
		Vec r = sigmoid(Wr * sequence[0] + br);
		Vec h_hat = tanh(Wr * sequence[0] + br);
		Vec out_prev = hadamard(z, h_hat);

		for (size_t j = 1; j < sequence.size(); ++j) {
			z = sigmoid(Wz * sequence[j] + Uz * out_prev + bz);
			r = sigmoid(Wr * sequence[j] + Ur * out_prev + br);
			h_hat = tanh(Wr * sequence[j] + Uh * hadamard(r, out_prev) + br);
			out_prev = out_prev - hadamard(out_prev, z) + hadamard(z, h_hat);
		}

		Vec out = sigmoid(W * out_prev + b);

		Vec err = (out - Y[trainingIndex]);
		Var loss = err * err * (1.0f / static_cast<float>(sequence.size()));


		loss->calculateDerivatives();

		Wz->value += Wz->partial * lr;
		Uz->value += Uz->partial * lr;
		bz->value += bz->partial * lr;

		Wr->value += Wr->partial * lr;
		Ur->value += Ur->partial * lr;
		br->value += br->partial * lr;

		Wh->value += Wh->partial * lr;
		Uh->value += Uh->partial * lr;
		bh->value += bh->partial * lr;

		W->value += W->partial * lr;
		b->value += b->partial * lr;
	}


	float l = 0.0f;
	for (size_t i = 0; i < sequences.size(); ++i) {
		vector<Vec> sequence = sequences[i];

		Vec z = sigmoid(Wz * sequence[0] + bz);
		Vec r = sigmoid(Wr * sequence[0] + br);
		Vec h_hat = tanh(Wr * sequence[0] + br);
		Vec out_prev = hadamard(z, h_hat);

		for (size_t j = 1; j < sequence.size(); ++j) {
			z = sigmoid(Wz * sequence[j] + Uz * out_prev + bz);
			r = sigmoid(Wr * sequence[j] + Ur * out_prev + br);
			h_hat = tanh(Wr * sequence[j] + Uh * hadamard(r, out_prev) + br);
			out_prev = out_prev - hadamard(out_prev, z) + hadamard(z, h_hat);
		}

		Vec out = sigmoid(W * out_prev + b);

		Vec err = (out - Y[i]);
		Var loss = err * err;

		loss->eval();
		l += loss->value;

		cout << "Expected result: " << Y[i]->value << ", got: " << out->value << "\n";
	}
	cout << "MSE: " << l << "\n";

	return 0;
}
