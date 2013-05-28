#ifndef SCALC_COMMON_HPP
#define SCALC_COMMON_HPP

#include <map>
#include <utility>
#include <memory>

typedef double fpoint;

namespace poly{
    // 係数
    typedef fpoint c_type;
    // 指数
    typedef fpoint e_type;

    // 多項式の項
    struct node{
        node();

        // 指数
        std::map<std::string, e_type> e;

        // 実部, 虚部
        c_type real, imag;

        // 次の項
        node *next;
    };

    node *new_node();
    void dispose_node(node *p);
    void dispose(node *p);
    node *constant(c_type re, c_type im);
    node *variable(const std::string &str, e_type n);
    node *copy(node *p);
    void change_sign(node *p);
    void add(node *p, node *q);
    node *multiply(node *x, node *y);
    node *power(node *x, e_type n);
    std::string poly_to_string(const node *p);
}

#endif // SCALC_COMMON_HPP
