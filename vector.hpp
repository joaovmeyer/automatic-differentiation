#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "node.hpp"

#include <iostream>


inline void operator += (std::vector<float>& v1, const std::vector<float>& v2) {
	for (size_t i = 0; i < v1.size(); ++i) {
		v1[i] += v2[i];
	}
}

inline std::vector<float> operator + (const std::vector<float>& v1, const std::vector<float>& v2) {
	std::vector<float> ans = v1;
	for (size_t i = 0; i < v1.size(); ++i) {
		ans[i] += v2[i];
	}

	return ans;
}

inline std::vector<float> operator * (const std::vector<float>& v, float a) {
	std::vector<float> ans = v;
	for (size_t i = 0; i < ans.size(); ++i) {
		ans[i] *= a;
	}

	return ans;
}

std::ostream& operator << (std::ostream& os, const std::vector<float>& v) {

	os << "(";

	for (size_t i = 0; i < v.size(); ++i) {
		os << v[i];

		if (i + 1 < v.size()) {
			os << ", ";
		}
	}

	os << ")";

	return os;
}


struct Vector : Node {

	size_t size;
	std::vector<float> value;
	std::vector<float> partial;

	Vector(size_t s = 0, float fillValue = 0.0f, const std::string& n = "", bool trainable = false) : size(s), value(std::vector<float>(s, fillValue)), partial(std::vector<float>(s, 0.0f)) {
		
		#if USE_NAME
			if (n == "") {
				name = "(";
				for (size_t i = 0; i < s; ++i) {
					name += std::to_string(fillValue) + ", ";
				}

				// remove the last ", "
				if (s > 0) name.pop_back(); name.pop_back();

				name += ")";
			} else {
				name = n;
			}
		#endif

		isTrainable = trainable;
	}

	static std::shared_ptr<Vector> build(size_t s = 0, float fillValue = 0.0f, bool trainable = false, const std::string& n = "") {
		return std::make_shared<Vector>(s, fillValue, n, trainable);
	}

	void evaluate() override {

	}

	void derive() override {

	}

	void resetPartial() override final {
		std::fill(partial.begin(), partial.end(), 0.0f);
	}

	NodeTypes getType() {
		return VECTOR;
	}


	void calculateDerivatives() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->evaluate();
			ordering[i]->resetPartial();
		}

		std::fill(partial.begin(), partial.end(), 1.0f); // dx/dx is 1 for whatever x
		for (size_t i = ordering.size(); i > 0; --i) {
			ordering[i - 1]->derive();
		}
	}



	void saveToFile(const std::string& path) {

		// creates the path directory if it doesn't exist
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream file(path, std::ios::binary);

		file.write(reinterpret_cast<const char*>(&isTrainable), sizeof(isTrainable));
		file.write(reinterpret_cast<const char*>(&size), sizeof(size));
		file.write(reinterpret_cast<const char*>(&value[0]), size * sizeof(float));

		file.close();
	}

	static std::shared_ptr<Vector> loadFromFile(const std::string& path) {

		std::ifstream file(path, std::ios::binary);

		if (!file) {
			throw std::runtime_error("Cannot open file :(");
		}

		bool trainable;
		size_t size;

		file.read(reinterpret_cast<char*>(&trainable), sizeof(trainable));
		file.read(reinterpret_cast<char*>(&size), sizeof(size));

		std::shared_ptr<Vector> vec = std::make_shared<Vector>(size, 0.0f, "", trainable);

		file.read(reinterpret_cast<char*>(&vec->value[0]), size * sizeof(float));

		return vec;
	}


	Var get(size_t index);
};

// nicer naming
using Vec = std::shared_ptr<Vector>;













struct GetVectorElem : Scalar {
	Vec a;
	size_t index;

	GetVectorElem() {}

	static Var build(const Vec& v, size_t index = 0) {

		std::shared_ptr<GetVectorElem> node = std::make_shared<GetVectorElem>();

		node->a = v;
		node->index = index;
		#if USE_NAME
			node->name = v->name + "[" + std::to_string(index) + "]";
		#endif
		node->value = v->value[index];

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		value = a->value[index];
	}

	void derive() override final {
		a->partial[index] += partial;
	}
};


Var Vector::get(size_t index = 0) {
	return GetVectorElem::build(dynamic_pointer_cast<Vector>(shared_from_this()), index);
}


struct VectorFromScalars : Vector {
	std::vector<Var> a;

	VectorFromScalars(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const std::vector<Var>& vars) {

		std::shared_ptr<VectorFromScalars> node = std::make_shared<VectorFromScalars>(vars.size());

		node->a = vars;
		for (size_t i = 0; i < vars.size(); ++i) {
			node->parents.push_back(vars[i]);
			node->value[i] = vars[i]->value;
		}

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < size; ++i) {
			value[i] = a[i]->value;
		}
	}

	void derive() override final {
		for (size_t i = 0; i < size; ++i) {
			a[i]->partial += partial[i];
		}
	}
};






#endif
