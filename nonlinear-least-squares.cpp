#define USE_NAME true

#include <iostream>
#include <cmath>

#include "../graph.h"
#include "operations.hpp"
#include "../rng.h"

using namespace std;



// should probably make a way to avoid calculating partials that are not in the wrt list
// also, doing get->(i) everytime is not needed, as it creates a whole new operation
vector<vector<float>> getJacobian(const Vec& F, const Vec& wrt) {
	vector<vector<float>> jacobian(F->size);

	for (size_t i = 0; i < F->size; ++i) {
		F->get(i)->calculateDerivatives();

		jacobian[i] = wrt->partial;
	}

	return jacobian;
}

// gaussian elimination with partial pivoting
vector<float> solveLinearSystem(vector<vector<float>> a, vector<float> b) {

	size_t n = a.size();


	for (size_t k = 0; k < n - 1; ++k) {

		// partial pivoting
		size_t pivotIndex = k;
		for (size_t i = k + 1; i < n; ++i) {
			if (std::abs(a[i][k]) > std::abs(a[pivotIndex][k])) {
				pivotIndex = i;
			}
		}

		std::swap(a[k], a[pivotIndex]);
		std::swap(b[k], b[pivotIndex]);

		for (size_t i = k + 1; i < n; ++i) {
			double m = a[i][k] / a[k][k];

			// L[i] <- L[i] - m * L[k]
			a[i][k] = 0;
			for (size_t j = k + 1; j < n; ++j) {
				a[i][j] -= m * a[k][j];
			}
			b[i] -= m * b[k];
		}
	}

	vector<float> x(n);
	x[n - 1] = b[n - 1] / a[n - 1][n - 1];

	for (int k = n - 2; k >= 0; --k) {
		double sum = 0;

		for (int j = k + 1; j < n; ++j) {
			sum += a[k][j] * x[j];
		}

		x[k] = (b[k] - sum) / a[k][k];
	}

	return x;
}





// return multiplication A^T * A of given A matrix
vector<vector<float>> ATA(const vector<vector<float>>& A) {

	size_t n = A.size(), m = A[0].size();

	vector<vector<float>> res(m, vector<float>(m, 0.0f));

	for (size_t k = 0; k < n; ++k) {
		for (size_t i = 0; i < m; ++i) {
			for (size_t j = 0; j < m; ++j) {
				res[i][j] += A[k][i] * A[k][j];
			}
		}
	}

	return res;
}

// return multiplication A^T * b of given A matrix and b vector
vector<float> ATb(const vector<vector<float>>& A, const vector<float>& b) {

	size_t n = A.size(), m = A[0].size();

	vector<float> res(m, 0.0f);

	for (size_t k = 0; k < n; ++k) {
		for (size_t i = 0; i < m; ++i) {
			res[i] += A[k][i] * b[k];
		}
	}

	return res;
}



int main() {

	Graph graph;

	float expectedA = 1.2f, expectedB = 0.5f;
	float noise = 1.5f;
	vector<float> X, Y;

	Vec params = Vector::build(2); params->value = { 1.9f, -0.1f }; // example initial value
	vector<Var> residuals;

	for (float x = -10.0f; x <= 5.0f; x += 0.1f) {
		float y = expectedA * std::exp(expectedB * x) + rng::fromUniformDistribution(-0.5, 0.5) * noise;

		X.push_back(x);
		Y.push_back(y);

		graph.addPoint(Point(x, y, 2, olc::RED));

		residuals.push_back(params->get(0) * Exp::build(params->get(1) * x) + -y);
	}

	Vec f = VectorFromScalars::build(residuals);

	int maxIter = 10;
	for (int iter = 0; iter < maxIter; ++iter) {
		vector<vector<float>> jac = getJacobian(f, params);

		// Gauss-Newton iteration for Nonlinear Least Squares problems
		params->value += solveLinearSystem(ATA(jac), ATb(jac, f->value * -1.0f));
	}

	cout << "Final params: " << params->value << "\n"; // show final solution

	Line l;
	for (float x = -10.0f; x <= 5.0f; x += 0.1f) {
		float y = params->value[0] * std::exp(params->value[1] * x);
		l.addPoint(Point(x, y));
	}

	graph.addLine(l);

	graph.waitFinish();

	return 0;
}
