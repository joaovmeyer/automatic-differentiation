#ifndef SCALAR_HPP
#define SCALAR_HPP

#include "node.hpp"


struct Scalar : Node {

    float value;
    float partial;

    Scalar(float v = 0.0f, const std::string& n = "") : value(v), partial(0.0f) {
        name = (n == "") ? std::to_string(v) : n;
    }

    static std::shared_ptr<Scalar> build(float v = 0.0f, const std::string& n = "") {
        return std::make_shared<Scalar>(v, n);
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

	    partial = 1.0; // dx/dx is 1 for whatever x
	    for (size_t i = ordering.size(); i > 0; --i) {
	        ordering[i - 1]->derive();
		}
    }
};

// nicer naming
using Var = std::shared_ptr<Scalar>;


#endif
