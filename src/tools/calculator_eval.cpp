// src/tools/calculator_eval.cpp —

#include "tools/calculator.h"

#include <stack>

namespace {

using Kind = CalcToken::Kind;

bool is_operator(Kind k) {
    return k == Kind::Plus || k == Kind::Minus || k == Kind::Times || k == Kind::Divide;
}

int precedence(Kind k) {
    return (k == Kind::Times || k == Kind::Divide) ? 2 : 1;
}

// Pops one operator and two values and pushes the result
bool apply(std::stack<double>& values, std::stack<Kind>& ops, std::string* error) {
    if (ops.empty() || values.size() < 2) {
        *error = "malformed expression";
        return false;
    }
    Kind op = ops.top();
    ops.pop();
    double right = values.top();
    values.pop();
    double left = values.top();
    values.pop();

    double result = 0.0;
    switch (op) {
        case Kind::Plus:   result = left + right; break;
        case Kind::Minus:  result = left - right; break;
        case Kind::Times:  result = left * right; break;
        case Kind::Divide:
            if (right == 0.0) {
                *error = "division by zero";
                return false;
            }
            result = left / right;
            break;
        default:
            *error = "malformed expression";
            return false;
    }
    values.push(result);
    return true;
}

}  // namespace

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out, std::string* error) {
    std::stack<double> values;
    std::stack<Kind>   ops;

    for (const CalcToken& t : tokens) {
        switch (t.kind) {
            case Kind::Number:
                values.push(t.value);
                break;
            case Kind::LParen:
                ops.push(Kind::LParen);
                break;
            case Kind::RParen:
                while (!ops.empty() && ops.top() != Kind::LParen)
                    if (!apply(values, ops, error)) return false;
                if (ops.empty()) {
                    *error = "unbalanced parentheses";
                    return false;
                }
                ops.pop();   // the '('
                break;
            default:   // + - * /
                while (!ops.empty() && is_operator(ops.top()) &&
                       precedence(ops.top()) >= precedence(t.kind))
                    if (!apply(values, ops, error)) return false;
                ops.push(t.kind);
                break;
        }
    }

    while (!ops.empty()) {
        if (ops.top() == Kind::LParen) {
            *error = "unbalanced parentheses";
            return false;
        }
        if (!apply(values, ops, error)) return false;
    }

    if (values.size() != 1) {
        *error = "malformed expression";
        return false;
    }
    *out = values.top();
    return true;
}
