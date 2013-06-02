#include <string>
#include <sstream>
#include <utility>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include "common.hpp"

namespace poly{
node::node() : e(), real(0), imag(0), next(nullptr){}
node::~node(){
    for(auto iter = e.begin(); iter != e.end(); ++iter){
        dispose(iter->second);
    }
}

void node::negate(){
    real = -real, imag = -imag;
}

void node::complex_conjugate(){
    imag = -imag;
}

template<class T>
inline int primitive_compare(const T &lhs, const T &rhs){ return lhs < rhs ? -1 : lhs > rhs ? 1 : 0; }

// note: priority of class
// R_Only > C_Only > C > R_Onry^E > C_Onry^E > C^E
int lexicographic_compare(const node *l, const node *r){
    auto factor_class = [](const node *f) -> int{
        if(!f->e.empty()){
            if(f->imag != 0){
                if(f->real != 0){
                    return 5;
                }else{
                    return 4;
                }
            }
            return 3;
        }else{
            if(f->imag != 0){
                if(f->real != 0){
                    return 2;
                }else{
                    return 1;
                }
            }
            return 0;
        }
    };
    if(!l && !r){ return 0; }
    if(!l && r){ return -1; }
    if(l && !r){ return 1; }
    l = l->next, r = r->next;
    if(!l && !r){ return 0; }
    if(!l && r){ return -1; }
    if(l && !r){ return 1; }
    int l_class = factor_class(l), r_class = factor_class(r);
    if(l_class < r_class){ return -1; }else if(l_class > r_class){ return 1; }
    auto exponent_compare = [](const exponent_type &l, const exponent_type &r) -> int{
        int result;
        auto l_iter = l.begin(), r_iter = r.begin();
        bool l_phi, r_phi;
        for(; ; ++l_iter, ++r_iter){
            l_phi = l_iter == l.end();
            r_phi = r_iter == r.end();
            if(l_phi || r_phi){
                result = l_phi && r_phi ? 0 : l_phi ? -1 : 1;
                break;
            }
            if(l_iter->first == r_iter->first){
                result = lexicographic_compare(l_iter->second, r_iter->second);
                if(result != 0){ break; }
            }else{
                result = -primitive_compare(l_iter->first, r_iter->first);
                break;
            }
        }
        return result;
    };
    int result;
    switch(l_class){
    case 5:
        result = exponent_compare(l->e, r->e);
        if(result == 0){ result = primitive_compare(l->real, r->real); }
        if(result == 0){ result = primitive_compare(l->imag, r->imag); }
        if(result == 0){ result = lexicographic_compare(l, r); }
        break;

    case 4:
        result = exponent_compare(l->e, r->e);
        if(result == 0){ result = primitive_compare(l->imag, r->imag); }
        if(result == 0){ result = lexicographic_compare(l, r); }
        break;

    case 3:
        result = exponent_compare(l->e, r->e);
        if(result == 0){ result = primitive_compare(l->real, r->real); }
        if(result == 0){ result = lexicographic_compare(l, r); }
        break;

    case 2:
        result = primitive_compare(l->real, r->real);
        if(result == 0){ result = primitive_compare(l->imag, r->imag); }
        if(result == 0){ result = lexicographic_compare(l, r); }
        break;

    case 1:
        result = primitive_compare(l->imag, r->imag);
        if(result == 0){ result = lexicographic_compare(l, r); }
        break;

    case 0:
        result = primitive_compare(l->real, r->real);
        if(result == 0){ result = lexicographic_compare(l, r); }
        break;

    default:
        result = 0;
    }
    return result;
}

template<class T>
std::string to_string(const T &v){
    std::stringstream ss;
    ss << v;
    return ss.str();
}

// 項を生成
node *new_node(){
    return new node;
}

// 項を破棄
void dispose_node(node *p){
    delete p;
}

// 多項式を破棄
void dispose(node *p){
    while(p){
        node *q = p->next;
        dispose_node(p);
        p = q;
    }
}

// 定数を生成
node *constant(fpoint re, fpoint im){
    node *p = new_node();
    if(re != 0 || im != 0){
        node *q = new_node();
        q->real = re;
        q->imag = im;
        p->next = q;
    }
    return p;
}

// 変数を生成
node *variable(const std::string &str){
    node *p = constant(1, 0);
    p->next->e[str] = constant(1, 0);
    return p;
}

// 変数のべき乗を生成
node *variable(const std::string &str, fpoint re, fpoint im){
    node *p = constant(1, 0);
    p->next->e[str] = constant(re, im);
    return p;
}

// 変数の任意のべき乗を生成
// ptrは破棄
node *variable(const std::string &str, node *ptr){
    node *p = constant(1, 0);
    p->next->e[str] = ptr;
    return p;
}

// 多項式をコピー
node *copy(node *p){
    node *q, *r;
    q = r = new_node();
    while(p = p->next){
        r = r->next = new_node();
        r->real = p->real;
        r->imag = p->imag;
        for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
            r->e.insert(std::make_pair(iter->first, copy(iter->second)));
        }
    }
    return q;
}

// 符号を反転
void change_sign(node *p){
    while(p = p->next){
        p->negate();
    }
}

// 複素共役
void complex_conjugate(node *p){
    while(p = p->next){
        p->complex_conjugate();
    }
}

// 加算
// qは破棄
void add(node *p, node *q){
    node *p1 = p, *q1 = q;
    node *ep = nullptr, *eq = nullptr;
    p = p->next;
    q = q->next;
    dispose_node(q1);
    while(q){
        while(p){
            int compare_result;
            auto l_iter = p->e.begin(), r_iter = q->e.begin();
            bool l_phi, r_phi;
            for(; ; ++l_iter, ++r_iter){
                l_phi = l_iter == p->e.end();
                r_phi = r_iter == q->e.end();
                if(l_phi || r_phi){
                    compare_result = l_phi && r_phi ? 0 : l_phi ? -1 : 1;
                    if(!l_phi){ ep = l_iter->second; }
                    if(!r_phi){ eq = r_iter->second; }
                    break;
                }
                if(l_iter->first == r_iter->first){
                    compare_result = lexicographic_compare(l_iter->second, r_iter->second);
                    if(compare_result != 0){
                        ep = l_iter->second;
                        eq = r_iter->second;
                        break;
                    }
                }else{
                    compare_result = -primitive_compare(l_iter->first, r_iter->first);
                    if(compare_result < 0){
                        ep = nullptr;
                        eq = r_iter->second;
                    }else{
                        ep = l_iter->second;
                        eq = nullptr;
                    }
                    break;
                }
            }
            if(compare_result <= 0){ break; }
            p1 = p, p = p->next;
        }
        if(!p || lexicographic_compare(ep, eq) < 0){
            p1->next = q, p1 = q, q = q->next;
            p1->next = p;
        }else{
            p->real += q->real;
            p->imag += q->imag;
            if(p->real != 0 || p->imag != 0){
                p1 = p;
                p = p->next;
            }else{
                p = p->next;
                dispose_node(p1->next);
                p1->next = p;
            }
            q1 = q, q = q->next, dispose_node(q1);
        }
    }
}

// 減算
// qは破棄
void sub(node *p, node *q){
    change_sign(q);
    add(p, q);
}

// 乗算
// 新たな多項式を返す
node *multiply(node *x, node *y){
    node *ep = nullptr, *eq = nullptr;
    node *p, *p1, *q, *r, *z;
    r = new_node(), r->next = nullptr, q = nullptr;
    while(y = y->next){
        p1 = r, p = p1->next, z = x;
        while(z = z->next){
            if(!q){ q = new_node(); }
            q->real = y->real * z->real - y->imag * z->imag;
            q->imag = y->real * z->imag + y->imag * z->real;
            auto add_pow = [q](node *ptr){
                for(auto iter = ptr->e.begin(); iter != ptr->e.end(); ++iter){
                    auto jter = q->e.find(iter->first);
                    if(jter == q->e.end()){
                        q->e.insert(std::make_pair(iter->first, copy(iter->second)));
                    }else{
                        add(jter->second, copy(iter->second));
                        if(!jter->second->next){ q->e.erase(jter); }
                    }
                }
            };
            add_pow(y);
            add_pow(z);
            while(p){
                int compare_result;
                auto l_iter = p->e.begin(), r_iter = q->e.begin();
                bool l_phi, r_phi;
                for(; ; ++l_iter, ++r_iter){
                    l_phi = l_iter == p->e.end();
                    r_phi = r_iter == q->e.end();
                    if(l_phi || r_phi){
                        compare_result = l_phi && r_phi ? 0 : l_phi ? -1 : 1;
                        if(!l_phi){ ep = l_iter->second; }
                        if(!r_phi){ eq = r_iter->second; }
                        break;
                    }
                    if(l_iter->first == r_iter->first){
                        compare_result = lexicographic_compare(l_iter->second, r_iter->second);
                        if(compare_result != 0){
                            ep = l_iter->second;
                            eq = r_iter->second;
                            break;
                        }
                    }else{
                        compare_result = -primitive_compare(l_iter->first, r_iter->first);
                        if(compare_result < 0){
                            ep = nullptr;
                            eq = r_iter->second;
                        }else{
                            ep = l_iter->second;
                            eq = nullptr;
                        }
                        break;
                    }
                }
                if(compare_result <= 0){ break; }
                p1 = p, p = p->next;
            }
            if(!p || lexicographic_compare(ep, eq) < 0){
                p1->next = q, p1 = q, p1->next = p;
                q = nullptr;
            }else{
                p->real += q->real;
                p->imag += q->imag;
                if(p->real != 0 || p->imag != 0){
                    p1 = p, p = p->next;
                }else{
                    p = p->next;
                    dispose_node(p1->next);
                    p1->next = p;
                }
            }
        }
    }
    if(q){ dispose_node(q); }
    return r;
}

// 除算
node *divide(node *x, node *y){
    node *p = copy(y), *q;
    q = p;
    while(p = p->next){
        for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
            change_sign(iter->second);
        }
    }
    node *r = multiply(x, q);
    dispose(q);
    return r;
}

// 累乗
// xは破棄する
node *power(node *x, node *n){
    return nullptr;
    //if(n == 1){ return x; }
    //node *p, *q;
    //if(n == 0){
    //    p = constant(1, 0);
    //}else if(n > 0){
    //    e_type i = std::floor(n), f = n - i;
    //    if(f == 0){
    //        p = multiply(x, x), i -= 2;
    //        if(n > 0){
    //            q = p;
    //            if(std::fmod(i, 2.0) == 1){
    //                p = multiply(q, x);
    //            }else{
    //                p = copy(q);
    //            }
    //            dispose(x), x = q, i = std::floor(i / 2);
    //            if(std::fmod(i, 2.0) == 1){
    //                q = multiply(p, x), dispose(p), p = q;
    //            }
    //            while((i = std::floor(i / 2)) != 0){
    //                q = multiply(x, x), dispose(x), x = q;
    //                if(std::fmod(i, 2.0) == 1){
    //                    q = multiply(p, x), dispose(p), p = q;
    //                }
    //            }
    //        }
    //    }else{
    //        p = nullptr;// TODO f
    //    }
    //}else{
    //    p = nullptr; // TODO n < 0
    //}
    //dispose(x);
    //return p;
}

// 式を文字列として得る. 実装
std::pair<std::string, bool> poly_to_string_impl(const node *p){
    bool first = true, one, paren = false;
    fpoint re, im;
    node *e = nullptr, *c = constant(1);
    std::string r;
    while(p = p->next){
        one = false;
        re = p->real;
        im = p->imag;
        if(im == 0){
            if(re >= 0){
                if(!first){
                    r += "+";
                    paren = true;
                }
            }else{
                re = -re;
                r += "-";
                paren = true;
            }
            if(re == 1){ one = true; }else{ r += to_string(re); }
        }else if(re == 0){
            if(im >= 0){
                if(!first){
                    r += "+";
                    paren = true;
                }
            }else{
                im = -im;
                r += "-";
                paren = true;
            }
            r += (std::abs(im) == 1 ? std::string("") : to_string(im)) + "i";
        }else{
            if(!first){
                r += "+";
            }
            bool nega;
            if(nega = im < 0){ im = -im; }
            r += to_string(re) + (nega ? "-" : "+") + (std::abs(im) == 1 ? std::string("") : to_string(im)) + "i";
            paren = true;
        }
        first = false;
        for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
            if(e = iter->second){
                if(!one){ r += "*"; }
                one = false;
                r += iter->first;
                if(lexicographic_compare(e, c)){
                    std::pair<std::string, bool> s = poly_to_string_impl(e);
                    r += "^" + std::string(s.second ? "(" : "") + s.first + std::string(s.second ? ")" : "");
                }
            }
        }
        if(one){ r += "1"; }
    }
    if(first){ r += "0"; }
    dispose(c);
    return std::make_pair(r, paren);
}

// 式を文字列として得る
std::string poly_to_string(const node *p){
    return poly_to_string_impl(p).first;
}

}
