#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "node.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>


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








struct Vector;



struct Vec {
	std::shared_ptr<Vector> ptr;

	Vec() {}
	// now this is cool
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Vector, T>>>
	Vec(const std::shared_ptr<T>& p) : ptr(p) {}

	Vector* operator->() const {
		return ptr.get();
	}

	Vector& operator*() const {
		return *ptr;
	}

	vector<float> operator () () const;


	operator std::shared_ptr<Vector>() const {
		return ptr;
	}
	operator std::shared_ptr<Node>() const {
		return static_pointer_cast<Node>(ptr);
	}

	Var operator [] (size_t index) const;
	Vec operator () (size_t start, size_t end) const;
};









struct Vector : Node {

	size_t size;
	std::vector<float> value;
	std::vector<float> partial;
	Vec gradientFunction;

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

	static Vec build(size_t s = 0, float fillValue = 0.0f, bool trainable = false, const std::string& n = "") {
		return std::make_shared<Vector>(s, fillValue, n, trainable);
	}

	void evaluate() override {

	}

	void derive() override {

	}

	void resetPartial(float defaultValue = 0.0f) override final {
		std::fill(partial.begin(), partial.end(), defaultValue);
	}

	NodeTypes getType() {
		return VECTOR;
	}

	void updateGradientFunction() override {
		
	}

	void resetGradientFunction(float defaultValue = 0.0f) override {
		gradientFunction = Vector::build(size, defaultValue);
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

	static Vec loadFromFile(const std::string& path) {

		std::ifstream file(path, std::ios::binary);

		if (!file) {
			throw std::runtime_error("Cannot open file :(");
		}

		bool trainable;
		size_t size;

		file.read(reinterpret_cast<char*>(&trainable), sizeof(trainable));
		file.read(reinterpret_cast<char*>(&size), sizeof(size));

		Vec vec(std::make_shared<Vector>(size, 0.0f, "", trainable));

		file.read(reinterpret_cast<char*>(&vec->value[0]), size * sizeof(float));

		return vec;
	}


/*	Var get(size_t index);
	Vec get(size_t start, size_t end);*/
};

// nicer naming
// using Vec = std::shared_ptr<Vector>;







vector<float> Vec::operator () () const {
	ptr->eval();
	return ptr->value;
}















// some forward declarations
Vec operator + (const Vec& v1, const Vec& v2);
Var operator + (const Var& v1, const Var& v2);

struct VectorAddAtPos : Vector {
	Vec a;
	Var b;
	size_t index;

	VectorAddAtPos(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v, const Var& s, size_t index = 0) {

		std::shared_ptr<VectorAddAtPos> node = std::make_shared<VectorAddAtPos>(v->size);

		node->a = v;
		node->b = s;
		node->index = index;
		#if USE_NAME
			node->name = v->name + "[" + std::to_string(index) + "] + " + s->name;
		#endif

		node->parents.push_back(v);
		node->parents.push_back(s);

		return node;
	}

	void evaluate() override final {
		value = a->value;
		value[index] += b->value;
	}

	void derive() override final {
		a->partial += partial;
		b->partial += partial[index];
	}

	void updateGradientFunction() override final {
		a->gradientFunction = a->gradientFunction + gradientFunction;
		b->gradientFunction = b->gradientFunction + gradientFunction[index];
	}
};




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

	void updateGradientFunction() override final {
		a->gradientFunction = VectorAddAtPos::build(a->gradientFunction, gradientFunction, index);
	}
};


Var Vec::operator [] (size_t index) const {
	return GetVectorElem::build(*this, index);
}
/*Var Vector::get(size_t index = 0) {
	return GetVectorElem::build(get_self(), index);
}*/




struct GetVectorElems : Vector {
	Vec a;
	size_t start, end;

	GetVectorElems(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v, size_t start, size_t end) {

		if (end < start) std::swap(start, end);

		std::shared_ptr<GetVectorElems> node = std::make_shared<GetVectorElems>(end - start);

		node->a = v;
		node->start = start;
		node->end = end;
		#if USE_NAME
			node->name = v->name + "[" + std::to_string(start) + ", " + std::to_string(end) + "]";
		#endif

		for (size_t i = 0; i < end - start; ++i) {
			node->value[i] = node->a->value[i + start];
		}

		node->parents.push_back(v);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < end - start; ++i) {
			value[i] = a->value[i + start];
		}
	}

	void derive() override final {
		for (size_t i = 0; i < end - start; ++i) {
			a->partial[i + start] += partial[i];
		}
	}
};


Vec Vec::operator () (size_t start, size_t end) const {
	return GetVectorElems::build(*this, start, end);
}
/*Vec Vector::get(size_t start, size_t end) {
	return GetVectorElems::build(get_self(), start, end);
}*/








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




struct VectorConcat : Vector {
	Vec a, b;

	VectorConcat(size_t s = 0, float fillValue = 0.0f) {
		size = s;
		value = std::vector<float>(s, fillValue);
		partial = std::vector<float>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Vec& v2) {

		std::shared_ptr<VectorConcat> node = std::make_shared<VectorConcat>(v1->size + v2->size);

		node->a = v1;
		node->b = v2;

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		for (size_t i = 0; i < a->size; ++i) {
			value[i] = a->value[i];
		}

		for (size_t i = 0; i < b->size; ++i) {
			value[i + a->size] = b->value[i];
		}
	}

	void derive() override final {
		for (size_t i = 0; i < a->size; ++i) {
			a->partial[i] += partial[i];
		}

		for (size_t i = 0; i < b->size; ++i) {
			b->partial[i] += partial[i + a->size];
		}
	}
};






#endif
