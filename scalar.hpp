#ifndef SCALAR_HPP
#define SCALAR_HPP

#include "node.hpp"

#include <fstream>
#include <filesystem>



struct Scalar;



struct Var {
	std::shared_ptr<Scalar> ptr;

	Var() {}
	Var(const std::shared_ptr<Node>& p) : ptr(std::dynamic_pointer_cast<Scalar>(p)) {}
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Scalar, T>>>
	Var(const std::shared_ptr<T>& p) : ptr(p) {}

	Scalar* operator -> () const {
		return ptr.get();
	}

	Scalar& operator * () const {
		return *ptr;
	}

	float operator () () const;


	operator std::shared_ptr<Scalar>() const {
		return ptr;
	}
	operator std::shared_ptr<Node>() const {
		return std::static_pointer_cast<Node>(ptr);
	}
};





struct Scalar : Node {

	float value;
	float partial;
	std::shared_ptr<Scalar> gradientFunction;

	Scalar(float v = 0.0f, const std::string& n = "", bool trainable = false) : value(v), partial(0.0f) {
		#if USE_NAME
			name = (n == "") ? std::to_string(v) : n;
		#endif

		isTrainable = trainable;
	}

	static std::shared_ptr<Scalar> build(float v = 0.0f, bool trainable = false, const std::string& n = "") {
		return std::make_shared<Scalar>(v, n, trainable);
	}

	void evaluate() override {

	}

	void derive() override {

	}

	void resetPartial(float defaultValue = 0.0f) override final {
		partial = defaultValue;
	}

	NodeTypes getType() override final {
		return SCALAR;
	}

	void updateGradientFunction() override {
		
	}

	void resetGradientFunction(float defaultValue = 0.0f) override {
		gradientFunction = Scalar::build(defaultValue);
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


float Var::operator () () const {
	ptr->eval();
	return ptr->value;
}


#endif
