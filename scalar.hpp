#ifndef SCALAR_HPP
#define SCALAR_HPP

#include "node.hpp"


struct Scalar : Node {

	float value;
	float partial;

	Scalar(float v = 0.0f, const std::string& n = "") : value(v), partial(0.0f) {
		#if USE_NAME
			name = (n == "") ? std::to_string(v) : n;
		#endif
	}

	static std::shared_ptr<Scalar> build(float v = 0.0f, const std::string& n = "") {
		return std::make_shared<Scalar>(v, n);
	}

	void evaluate() override {

	}

	void derive() override {

	}

	void resetPartial() override {
		partial = 0.0f;
	}

	void calculateDerivatives() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->evaluate();
			ordering[i]->resetPartial();
		}

		partial = 1.0f; // dx/dx is 1 for whatever x
		for (size_t i = ordering.size(); i > 0; --i) {
			ordering[i - 1]->derive();
		}
	}



	void saveToFile(const std::string& path) {

		// creates the path directory if it doesn't exist
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream file(path, std::ios::binary);

		file.write(reinterpret_cast<const char*>(&isTrainable), sizeof(isTrainable));
		file.write(reinterpret_cast<const char*>(&value), sizeof(float));

		file.close();
	}

	static std::shared_ptr<Scalar> loadFromFile(const std::string& path) {

		std::ifstream file(path, std::ios::binary);

		if (!file) {
			throw std::runtime_error("Cannot open file :(");
		}

		bool trainable;
		file.read(reinterpret_cast<char*>(&trainable), sizeof(trainable));

		std::shared_ptr<Scalar> var = std::make_shared<Scalar>(0.0f, "", trainable);
		file.read(reinterpret_cast<char*>(&var->value), sizeof(float));

		return var;
	}
};

// nicer naming
using Var = std::shared_ptr<Scalar>;


#endif
