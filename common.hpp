#ifndef SCALC_COMMON_HPP
#define SCALC_COMMON_HPP

#include <map>
#include <set>
#include <memory>

typedef double fpoint;

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

void add(node *p, node *q);

#endif // SCALC_COMMON_HPP
