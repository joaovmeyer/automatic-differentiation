
#include "operations.hpp"
#include "../rng.h"

#include <iostream>

using namespace std;


int main() {

	// DISABLE NAMES IN node.hpp BEFORE USING THIS!!!!

	int hiddenSize = 15;

	// LSMT layer
	Mat Wf = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Uf = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 1.0);
	Vec bf = Vector::build(hiddenSize, 0.1f);

	Mat Wi = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Ui = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 1.0);
	Vec bi = Vector::build(hiddenSize, 0.1f);

	Mat Wo = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Uo = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 1.0);
	Vec bo = Vector::build(hiddenSize, 0.1f);

	Mat Wc = Matrix::makeRandom(hiddenSize, 1, 0.0, 1.0);
	Mat Uc = Matrix::makeRandom(hiddenSize, hiddenSize, 0.0, 1.0);
	Vec bc = Vector::build(hiddenSize, 0.1f);

	// fully connected layer
	Mat W = Matrix::makeRandom(1, hiddenSize, 0.0, 1.0);
	Vec b = Vector::build(1, 0.1f);


	// a lot of dummy values just to test it's memory
	vector<vector<float>> X = { //                                                                                                      here
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
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


	float lr = -0.05f;
	for (int iter = 0; iter < 5000; ++iter) {

		size_t trainingIndex = iter % sequences.size();
		vector<Vec> sequence = sequences[trainingIndex];

		Vec f;
		Vec i = sigmoid(Wi * sequence[0] + bi);
		Vec o = sigmoid(Wo * sequence[0] + bo);
		Vec c_hat = tanh(Wc * sequence[0] + bc);

		Vec c_prev = hadamard(i, c_hat);
		Vec out_prev = hadamard(o, tanh(c_prev));

		for (size_t j = 1; j < sequence.size(); ++j) {
			f = sigmoid(Wf * sequence[j] + Uf * out_prev + bf);
			i = sigmoid(Wi * sequence[j] + Ui * out_prev + bi);
			o = sigmoid(Wo * sequence[j] + Uo * out_prev + bo);
			c_hat = tanh(Wc * sequence[j] + Uc * out_prev + bc);

			c_prev = hadamard(f, c_prev) + hadamard(i, c_hat);
			out_prev = hadamard(o, tanh(c_prev));
		}

		Vec out = sigmoid(W * out_prev + b);

		Vec err = (out - Y[trainingIndex]);
		Var loss = err * err * (1.0f / static_cast<float>(sequence.size()));


		loss->calculateDerivatives();

		Wf->value += Wf->partial * lr;
		Uf->value += Uf->partial * lr;
		bf->value += bf->partial * lr;

		Wi->value += Wi->partial * lr;
		Ui->value += Ui->partial * lr;
		bi->value += bi->partial * lr;

		Wo->value += Wo->partial * lr;
		Uo->value += Uo->partial * lr;
		bo->value += bo->partial * lr;

		Wc->value += Wc->partial * lr;
		Uc->value += Uc->partial * lr;
		bc->value += bc->partial * lr;

		W->value += W->partial * lr;
		b->value += b->partial * lr;
	}


	float l = 0.0f;
	for (size_t j = 0; j < sequences.size(); ++j) {
		vector<Vec> sequence = sequences[j];

		Vec f;
		Vec i = sigmoid(Wi * sequence[0] + bi);
		Vec o = sigmoid(Wo * sequence[0] + bo);
		Vec c_hat = tanh(Wc * sequence[0] + bc);

		Vec c_prev = hadamard(i, c_hat);
		Vec out_prev = hadamard(o, tanh(c_prev));

		for (size_t j = 1; j < sequence.size(); ++j) {
			f = sigmoid(Wf * sequence[j] + Uf * out_prev + bf);
			i = sigmoid(Wi * sequence[j] + Ui * out_prev + bi);
			o = sigmoid(Wo * sequence[j] + Uo * out_prev + bo);
			c_hat = tanh(Wc * sequence[j] + Uc * out_prev + bc);

			c_prev = hadamard(f, c_prev) + hadamard(i, c_hat);
			out_prev = hadamard(o, tanh(c_prev));
		}

		Vec out = sigmoid(W * out_prev + b);

		Vec err = (out - Y[j]);
		Var loss = err * err * (1.0f / static_cast<float>(sequence.size()));

		loss->eval();
		l += loss->value;

		cout << "Expected result: " << Y[j]->value << ", got: " << out->value << "\n";
	}
	cout << "MSE: " << l << "\n";

	return 0;
}
