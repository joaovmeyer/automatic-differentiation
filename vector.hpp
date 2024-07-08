#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "node.hpp"

#include <iostream>


void operator += (std::vector<float>& v1, const std::vector<float>& v2) {
    for (size_t i = 0; i < v1.size(); ++i) {
        v1[i] += v2[i];
    }
}

std::vector<float> operator * (const std::vector<float>& v, float a) {
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

    Vector(size_t s = 0, float fillValue = 0.0f, const std::string& n = "") : size(s), value(std::vector<float>(s, fillValue)), partial(std::vector<float>(s, 0.0f)) {
        if (n == "") {
            name = "(";
            for (size_t i = 0; i < s; ++i) {
                name += std::to_string(fillValue) + ", ";
            }

            if (s > 0) name.pop_back(); name.pop_back();

            name += ")";
        } else {
            name = n;
        }
    }

    static std::shared_ptr<Vector> build(size_t s = 0, float fillValue = 0.0f, const std::string& n = "") {
        return std::make_shared<Vector>(s, fillValue, n);
    }

    void evaluate() override {

    }

    void derive() override {

    }

    void calculateDerivatives() {
    	std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

	    for (size_t i = 0; i < ordering.size(); ++i) {
	        ordering[i]->evaluate();
		}

	    partial = std::vector<float>(size, 1.0f); // dx/dx is 1 for whatever x
	    for (size_t i = ordering.size(); i > 0; --i) {
	        ordering[i - 1]->derive();
		}
    }
};

// nicer naming
using Vec = std::shared_ptr<Vector>;


#endif
