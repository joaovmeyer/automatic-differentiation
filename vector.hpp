#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "node.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>



#ifndef NUM_TYPE
#define NUM_TYPE NUM_TYPE
#endif


inline void operator += (std::vector<NUM_TYPE>& v1, const std::vector<NUM_TYPE>& v2) {
	for (size_t i = 0; i < v1.size(); ++i) {
		v1[i] += v2[i];
	}
}

inline std::vector<NUM_TYPE> operator + (const std::vector<NUM_TYPE>& v1, const std::vector<NUM_TYPE>& v2) {
	std::vector<NUM_TYPE> ans = v1;
	for (size_t i = 0; i < v1.size(); ++i) {
		ans[i] += v2[i];
	}

	return ans;
}

inline std::vector<NUM_TYPE> operator * (const std::vector<NUM_TYPE>& v, NUM_TYPE a) {
	std::vector<NUM_TYPE> ans = v;
	for (size_t i = 0; i < ans.size(); ++i) {
		ans[i] *= a;
	}

	return ans;
}

std::ostream& operator << (std::ostream& os, const std::vector<NUM_TYPE>& v) {

	os << "[";

	for (size_t i = 0; i < v.size(); ++i) {
		os << v[i];

		if (i + 1 < v.size()) {
			os << ", ";
		}
	}

	os << "]";

	return os;
}








struct Vector;



struct Vec {
	std::shared_ptr<Vector> ptr;

	Vec() {}
	Vec(const std::shared_ptr<Node>& p) : ptr(std::dynamic_pointer_cast<Vector>(p)) {}
	// now this is cool
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Vector, T>>>
	Vec(const std::shared_ptr<T>& p) : ptr(p) {}

	Vector* operator -> () const {
		return ptr.get();
	}

	Vector& operator * () const {
		return *ptr;
	}

	std::vector<NUM_TYPE> operator () () const;


	operator std::shared_ptr<Vector>() const {
		return ptr;
	}
	operator std::shared_ptr<Node>() const {
		return std::static_pointer_cast<Node>(ptr);
	}

	Var operator [] (size_t index) const;
	Vec operator () (size_t start, size_t end) const;
};









struct Vector : Node {

	size_t size;
	std::vector<NUM_TYPE> value;
	std::vector<NUM_TYPE> partial;
	Vec gradientFunction;

	Vector(size_t s = 0, NUM_TYPE fillValue = 0.0f, const std::string& n = "", bool trainable = false) : size(s), value(std::vector<NUM_TYPE>(s, fillValue)), partial(std::vector<NUM_TYPE>(s, 0.0f)) {
		
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

	static Vec build(size_t s = 0, NUM_TYPE fillValue = 0.0f, bool trainable = false, const std::string& n = "") {
		return std::make_shared<Vector>(s, fillValue, n, trainable);
	}

	void evaluate() override {

	}

	void derive() override {

	}

	void resetPartial(NUM_TYPE defaultValue = 0.0f) override final {
		std::fill(partial.begin(), partial.end(), defaultValue);
	}

	NodeTypes getType() {
		return VECTOR;
	}

	void updateGradientFunction() override {
		
	}

	void resetGradientFunction(NUM_TYPE defaultValue = 0.0f) override {
		gradientFunction = Vector::build(size, defaultValue);
	}



	void saveToFile(const std::string& path) {

		// creates the path directory if it doesn't exist
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream file(path, std::ios::binary);

		file.write(reinterpret_cast<const char*>(&isTrainable), sizeof(isTrainable));
		file.write(reinterpret_cast<const char*>(&size), sizeof(size));
		file.write(reinterpret_cast<const char*>(&value[0]), size * sizeof(NUM_TYPE));

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

		file.read(reinterpret_cast<char*>(&vec->value[0]), size * sizeof(NUM_TYPE));

		return vec;
	}


	Var get(size_t index);
	Vec get(size_t start, size_t end);
};

// nicer naming
// using Vec = std::shared_ptr<Vector>;







std::vector<NUM_TYPE> Vec::operator () () const {
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

	VectorAddAtPos(size_t s = 0, NUM_TYPE fillValue = 0.0f) {
		size = s;
		value = std::vector<NUM_TYPE>(s, fillValue);
		partial = std::vector<NUM_TYPE>(s, 0.0f);
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

// a + b', where b' = concat({0, 0, ..., 0}, b). If b' is smaller of bigger than a, it'll be padded with 0s or it's last elements will be ignored
struct VectorAddVecWithOffset : Vector {
	Vec a;
	Vec b;
	size_t offset;

	VectorAddVecWithOffset(size_t s = 0, NUM_TYPE fillValue = 0.0f) {
		size = s;
		value = std::vector<NUM_TYPE>(s, fillValue);
		partial = std::vector<NUM_TYPE>(s, 0.0f);
	}

	static Vec build(const Vec& v1, const Vec& v2, size_t offset = 0) {

		std::shared_ptr<VectorAddVecWithOffset> node = std::make_shared<VectorAddVecWithOffset>(v1->size);

		node->a = v1;
		node->b = v2;
		node->offset = offset;

		node->parents.push_back(v1);
		node->parents.push_back(v2);

		return node;
	}

	void evaluate() override final {
		value = a->value;
		for (size_t i = 0; i < b->size && i + offset < a->size; ++i) {
			value[i + offset] += b->value[i];
		}
	}

	void derive() override final {
		a->partial += partial;

		for (size_t i = 0; i < b->size && i + offset < a->size; ++i) {
			b->partial[i] += partial[i + offset];
		}
	}

/*	void updateGradientFunction() override final {
		a->gradientFunction = a->gradientFunction + gradientFunction;
		b->gradientFunction = VectorAddVecWithOffset::build(b->gradientFunction, )
		b->gradientFunction = b->gradientFunction + gradientFunction[index];
	}*/
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
Var Vector::get(size_t index = 0) {
	return GetVectorElem::build(Vec(shared_from_this()), index);
}




struct GetVectorElems : Vector {
	Vec a;
	size_t start, end;

	GetVectorElems(size_t s = 0, NUM_TYPE fillValue = 0.0f) {
		size = s;
		value = std::vector<NUM_TYPE>(s, fillValue);
		partial = std::vector<NUM_TYPE>(s, 0.0f);
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

	void updateGradientFunction() override final {
		a->gradientFunction = VectorAddVecWithOffset::build(a->gradientFunction, gradientFunction, start);
	}
};


Vec Vec::operator () (size_t start, size_t end) const {
	return GetVectorElems::build(*this, start, end);
}
Vec Vector::get(size_t start, size_t end) {
	return GetVectorElems::build(Vec(shared_from_this()), start, end);
}








struct VectorFromScalars : Vector {
	std::vector<Var> a;

	VectorFromScalars(size_t s = 0, NUM_TYPE fillValue = 0.0f) {
		size = s;
		value = std::vector<NUM_TYPE>(s, fillValue);
		partial = std::vector<NUM_TYPE>(s, 0.0f);
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


	void updateGradientFunction() override final {
		for (size_t i = 0; i < size; ++i) {
			a[i]->gradientFunction = a[i]->gradientFunction + gradientFunction[i];
		}
	}
};




struct VectorConcat : Vector {
	Vec a, b;

	VectorConcat(size_t s = 0, NUM_TYPE fillValue = 0.0f) {
		size = s;
		value = std::vector<NUM_TYPE>(s, fillValue);
		partial = std::vector<NUM_TYPE>(s, 0.0f);
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


	void updateGradientFunction() override final;

/*	void updateGradientFunction() override final {
		a->gradientFunction = gradientFunction->get(0, a->size);
		b->gradientFunction = gradientFunction->get(a->size, size);
	}*/
};

inline Vec concat(const Vec& v1, const Vec& v2) {
	return VectorConcat::build(v1, v2);
}




#endif
