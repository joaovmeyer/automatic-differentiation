#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

#include "scalar.hpp"
#include "vector.hpp"

#include <cmath>

struct Add : Scalar {

    Var a, b;

    Add() {}

    static Var build(const Var& v1, const Var& v2) {

        std::shared_ptr<Add> add = std::make_shared<Add>();

        add->a = v1;
        add->b = v2;
        add->name = "(" + v1->name + " + " + v2->name + ")";

        add->parents.push_back(v1);
        add->parents.push_back(v2);

        return add;
    }

    void evaluate() override final {
        value = a->value + b->value;
    }

    void derive() override final {
        a->partial += partial;
        b->partial += partial;
    }
};

struct Mult : Scalar {

    Var a, b;

    Mult() {}

    static Var build(const Var& v1, const Var& v2) {

        std::shared_ptr<Mult> mult = std::make_shared<Mult>();

        mult->a = v1;
        mult->b = v2;
        mult->name = "(" + v1->name + " * " + v2->name + ")";

        mult->parents.push_back(v1);
        mult->parents.push_back(v2);

        return mult;
    }

    void evaluate() override final {
        value = a->value * b->value;
    }

    void derive() override final {
        a->partial += partial * b->value;
        b->partial += partial * a->value;
    }
};

struct Sin : Scalar {

    Var a;

    Sin() {}

    static Var build(const Var& v) {

        std::shared_ptr<Sin> sin = std::make_shared<Sin>();

        sin->a = v;
        sin->name = "sin(" + v->name + ")";

        sin->parents.push_back(v);

        return sin;
    }

    void evaluate() override final {
        value = std::sin(a->value);
    }

    void derive() override final {
        a->partial += partial * std::cos(a->value);
    }
};

struct Cos : Scalar {

    Var a;

    Cos() {}

    static Var build(const Var& v) {

        std::shared_ptr<Cos> cos = std::make_shared<Cos>();

        cos->a = v;
        cos->name = "cos(" + v->name + ")";

        cos->parents.push_back(v);

        return cos;
    }

    void evaluate() override final {
        value = std::cos(a->value);
    }

    void derive() override final {
        a->partial -= partial * std::sin(a->value);
    }
};

struct Exp : Scalar {

    Var a;

    Exp() {}

    static Var build(const Var& v) {

        std::shared_ptr<Exp> exp = std::make_shared<Exp>();

        exp->a = v;
        exp->name = "exp(" + v->name + ")";

        exp->parents.push_back(v);

        return exp;
    }

    void evaluate() override final {
        value = std::exp(a->value);
    }

    void derive() override final {
        a->partial += partial * value;
    }
};

struct Ln : Scalar {

    Var a;

    Ln() {}

    static Var build(const Var& v) {

        std::shared_ptr<Ln> ln = std::make_shared<Ln>();

        ln->a = v;
        ln->name = "ln(" + v->name + ")";

        ln->parents.push_back(v);

        return ln;
    }

    void evaluate() override final {
        value = std::log(a->value);
    }

    void derive() override final {
        a->partial += partial / a->value;
    }
};

struct Div : Scalar {

    Var a, b;

    Div() {}

    static Var build(const Var& v1, const Var& v2) {

        std::shared_ptr<Div> div = std::make_shared<Div>();

        div->a = v1;
        div->b = v2;
        div->name = "(" + v1->name + " / " + v2->name + ")";

        div->parents.push_back(v1);
        div->parents.push_back(v2);

        return div;
    }

    void evaluate() override final {
        value = a->value / b->value;
    }

    void derive() override final {
    	float inv = 1.0f / (b->value * b->value);
        a->partial += partial * b->value * inv;
        b->partial -= partial * a->value * inv;
    }
};


inline Var operator + (const Var& v1, const Var& v2) {
    return Add::build(v1, v2);
}

inline Var operator * (const Var& v1, const Var& v2) {
    return Mult::build(v1, v2);
}

inline Var operator / (const Var& v1, const Var& v2) {
    return Div::build(v1, v2);
}


inline Var operator + (const Var& v, float f) {
	return Add::build(v, Scalar::build(f));
}

inline Var operator / (float f, const Var& v) {
	return Div::build(Scalar::build(f), v);
}







struct VecDotVec : Scalar {

    Vec a, b;

    VecDotVec() {}

    static Var build(const Vec& v1, const Vec& v2) {

        std::shared_ptr<VecDotVec> dot = std::make_shared<VecDotVec>();

        dot->a = v1;
        dot->b = v2;
        dot->name = "(" + v1->name + " * " + v2->name + ")";

        dot->parents.push_back(v1);
        dot->parents.push_back(v2);

        return dot;
    }

    void evaluate() override final {
        value = 0.0f;

        for (size_t i = 0; i < a->size; ++i) {
            value += a->value[i] * b->value[i];
        }
    }

    void derive() override final {
        a->partial += b->value * partial;
        b->partial += a->value * partial;
    }
};

inline Var operator * (const Vec& v1, const Vec& v2) {
    return VecDotVec::build(v1, v2);
}






#endif
