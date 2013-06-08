#include <set>
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

inline int primitive_lexicographic_compare(const str_wrapper &lhs, const str_wrapper &rhs){
    return str_wrapper_less()(lhs, rhs);
}

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
                result = -primitive_lexicographic_compare(l_iter->first, r_iter->first);
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

// 先頭の1項だけ多項式としてコピー
node *copy_node(const node *p){
    node *q = new_node(), *r;
    r = q;
    q->next = new_node();
    p = p->next;
    q = q->next;
    q->real = p->real;
    q->imag = p->imag;
    for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
        q->e.insert(std::make_pair(iter->first, copy(iter->second)));
    }
    return r; 
}

// 多項式をコピー
node *copy(const node *p){
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
                    compare_result = -primitive_lexicographic_compare(l_iter->first, r_iter->first);
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
node *multiply(const node *x, const node *y){
    node *ep = nullptr, *eq = nullptr;
    const node *z;
    node *p, *p1, *q, *r;
    r = new_node(), q = nullptr;
    while(y = y->next){
        p1 = r, p = p1->next, z = x;
        while(z = z->next){
            dispose_node(q);
            q = new_node();
            q->real = y->real * z->real - y->imag * z->imag;
            q->imag = y->real * z->imag + y->imag * z->real;
            auto add_exponent = [q](const node *ptr){
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
            add_exponent(y);
            add_exponent(z);
            int compare_result;
            while(p){
                for(auto l_iter = p->e.begin(), r_iter = q->e.begin(); ; ++l_iter, ++r_iter){
                    bool l_phi = l_iter == p->e.end(), r_phi = r_iter == q->e.end();
                    if(l_phi || r_phi){
                        compare_result = l_phi && r_phi ? 0 : l_phi ? -1 : 1;
                        if(!l_phi){ ep = l_iter->second; }else{ ep = nullptr; }
                        if(!r_phi){ eq = r_iter->second; }else{ eq = nullptr; }
                        break;
                    }
                    if(l_iter->first == r_iter->first){
                        compare_result = lexicographic_compare(l_iter->second, r_iter->second);
                        if(compare_result != 0){
                            ep = l_iter->second;
                            eq = r_iter->second;
                            break;
                        }else{
                            ep = nullptr;
                            eq = nullptr;
                        }
                    }else{
                        compare_result = -primitive_lexicographic_compare(l_iter->first, r_iter->first);
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
            if(!p || compare_result < 0){
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
// 新たな多項式を返す
node *divide(const node *f_, const node *g, node *rem){
    auto check_exponent = [](const node *z, const node *y) -> bool{
        for(auto iter = y->e.begin(); iter != y->e.end(); ++iter){
            if(z->e.find(iter->first) == z->e.end()){
                return false;
            }
        }
        return true;
    };

    auto primitive_divide = [](node *q, const node *y, const node *z) -> void{
        if(y->imag == 0 && z->imag == 0){
            q->real = y->real / z->real;
            q->imag = 0;
        }else{
            fpoint w, d;
            if(std::fabs(z->real) >= std::fabs(z->imag)){
                w = z->imag / z->real, d = z->real + z->imag * w;
                q->real = (y->real + y->imag * w) / d;
                q->imag = (y->imag - y->real * w) / d;
            }else{
                w = z->real / z->imag, d = z->real * w + z->imag;
                q->real = (y->real * w + y->imag) / d;
                q->imag = (y->imag * w - y->real) / d;
            }
        }
    };

    auto exponent_divide = [](node *q, const node *z, const node *y) -> void{
        for(auto iter = z->e.begin(); iter != z->e.end(); ++iter){
            auto jter = y->e.find(iter->first);
            node *exponent = copy(iter->second);
            if(jter != y->e.end()){
                sub(exponent, copy(jter->second));
                if(exponent->next){
                    q->e.insert(std::make_pair(iter->first, exponent));
                }else{
                    dispose(exponent);
                }
            }else{
                q->e.insert(std::make_pair(iter->first, exponent));
            }
        }
    };

    node *q = new_node();
    if(!f_->next){ return q; }
    node *f = copy(f_);
    while(f->next){
        if(!check_exponent(f->next, g->next)){
            node *head = f->next;
            f->next = f->next->next;
            if(rem){
                node *dummy_head = new_node();
                dummy_head->next = head;
                head->next = nullptr;
                add(rem, dummy_head);
            }else{ dispose_node(head); }
            continue;
        }
        node *p = new_node();
        p->next = new_node();
        primitive_divide(p->next, f->next, g->next);
        exponent_divide(p->next, f->next, g->next);
        add(q, copy(p));
        node *head = f->next ? copy_node(f) : nullptr;
        sub(f, multiply(g, p));
        dispose(p);
        if(!head || !f->next){ dispose_node(head); }else{
            node *new_head = copy_node(f);
            head->real = 0, head->imag = 0;
            new_head->real = 0, new_head->imag = 0;
            if(lexicographic_compare(head, new_head) == 0){
                node *f_head = f->next;
                f->next = f->next->next;
                dispose_node(f_head);
            }
            dispose(head), dispose(new_head);
        }
    }
    dispose(f);
    return q;
}

// x^n
node *power(node *x, node *y){
    // symbol     = 0
    // constant   = 1
    // expression = 2
    auto kind = [](const node *p) -> int{
        p = p->next;
        if(!p->next){
            if(p->real != 0 || p->imag != 0){
                if(!p->e.empty()){
                    return 0;
                }else{
                    return 1;
                }
            }
        }
        return 2;
    };

    std::function<node*(node*, node*)> common_a = [](node *p, node *q) -> node*{
        node *r = constant(1), *s = r;
        p = p->next;
        r = r->next;
        for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
            r->e.insert(std::make_pair(iter->first, multiply(iter->second, q)));
        }
        return s;
    };

    std::function<node*(node*, node*)> function_table[3][3] = {
        // symbol^symbol     -> accept
        // symbol^constant   -> accept
        // symbol^polynomial -> accept
        { common_a, common_a, common_a },

        // constant^symbol     -> reject
        // constant^constant   -> calc
        // constant^polynomial -> reject
        {
            [](node *p, node *q) -> node*{
                throw(error("reject, constant^symbol."));
                return nullptr;
            },

            [](node *p, node *q) -> node*{
                auto c_mul = [](fpoint x_re, fpoint x_im, fpoint y_re, fpoint y_im) -> std::pair<fpoint, fpoint>{
                    std::pair<fpoint, fpoint> z;
                    z.first = x_re * y_re - x_im * y_im;
                    z.second = x_re * y_im + x_im * y_re;
                    return z;
                };

                auto c_exp = [](fpoint x_re, fpoint x_im) -> std::pair<fpoint, fpoint>{
                    fpoint a = std::exp(x_re);
                    x_re = a * std::cos(x_im);
                    x_im = a * std::sin(x_im);
                    return std::make_pair(x_re, x_im);
                };

                auto c_log = [](fpoint x_re, fpoint x_im) -> std::pair<fpoint, fpoint>{
                    std::pair<fpoint, fpoint> z;
                    z.first = 0.5 * std::log(x_re * x_re + x_im * x_im);
                    z.second = std::atan2(x_im, x_re);
                    return z;
                };

                p = p->next, q = q->next;
                auto log_result = c_log(p->real, p->imag);
                auto mul_result = c_mul(q->real, q->imag, log_result.first, log_result.second);
                auto exp_result = c_exp(mul_result.first, mul_result.second);

                return constant(exp_result.first, exp_result.second);
            },

            [](node *p, node *q) -> node*{
                throw(error("reject, constant^polynomial."));
                return nullptr;
            }
        },

        // polynomial^symbol     -> reject
        // polynomial^constant   -> calc
        // polynomial^polynomial -> reject
        {
            [](node *p, node *q) -> node*{
                throw(error("reject, polynomial^symbol."));
                return nullptr;
            },

            [](node *x_, node *n_) -> node*{
                node *a = n_;
                a = a->next;
                if(a->real < 0){ throw(error("reject, polynomial^negative.")); }
                if(a->imag != 0){ throw(error("reject, polynomial^complex.")); }
                fpoint integer = 0, frac = std::modf(a->real, &integer);
                if(frac != 0){ throw(error("reject, polynomial^" + to_string(frac))); }
                unsigned int n = static_cast<unsigned int>(integer);
                node *x = copy(x_), *p, *q;
                if(n == 1){ return x; }
                if(n == 0){ p = constant(1); }else{
                    auto odd = [](unsigned int n) -> bool{ return (n & 1) == 1; };
                    p = multiply(x, x);  n -= 2;
                    if (n > 0) {
                        q = p;
                        if (odd(n)) p = multiply(q, x);
                        else        p = copy(q);
                        dispose(x);  x = q;  n /= 2;
                        if (odd(n)) {
                            q = multiply(p, x);  dispose(p);  p = q;
                        }
                        while ((n /= 2) != 0) {
                            q = multiply(x, x);  dispose(x);  x = q;
                            if (odd(n)) {
                                q = multiply(p, x);  dispose(p);  p = q;
                            }
                        }
                    }
                }
                dispose(x);
                return p;
            },

            [](node *p, node *q) -> node*{
                throw(error("reject, polynomial^polynomial."));
                return nullptr;
            }
        }
    };

    return function_table[kind(x)][kind(y)](x, y);
}

// 式を文字列として得る. 実装
std::pair<std::string, bool> poly_to_string_impl(const node *p, bool ext_paren = false){
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
            if(first && !p->e.empty()){
                r += "(";
                r += to_string(re);
                if(im > 0){
                    r += "+" + to_string(im == 1 ? "i" : to_string(im) + "i");
                }else if(im < 0){
                    im = -im;
                    r += "-" + to_string(im == 1 ? "i" : to_string(im) + "i");
                }
                r += ")";
            }else{
                bool sign_re = re > 0, sign_im = im > 0;
                std::string s;
                if(!sign_re && !sign_im || sign_re != sign_im){ s += "-"; }else if(sign_re && sign_im){ s += "+"; }
                if(!p->e.empty()){
                    s += "(";
                    if(!sign_re && !sign_im){
                        s += to_string(-re) + "+" + to_string(im == -1 ? std::string("-i") : to_string(-im) + "i");
                    }else if(!sign_re && sign_im){
                        s += to_string(-re) + "-" + to_string(im == 1 ? std::string("i") : to_string(im) + "i");
                    }else if(sign_re && !sign_im){
                        s += to_string(re) + to_string(im == -1 ? std::string("-i") : to_string(im) + "i");
                    }else{
                        s += to_string(re) + "+" + to_string(im == 1 ? std::string("i") : to_string(im) + "i");
                    }
                    s += ")";
                }else{
                    s += to_string(re);
                    if(im > 0){ s += "+"; }else{
                        im = -im;
                        s += "-";
                    }
                    s += to_string(im == 1 ? std::string("i") : to_string(im) + "i");
                }
                r += s;
            }
            paren = true;
        }
        first = false;
        for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
            if(e = iter->second){
                if(!one){
                    r += "*";
                    if(ext_paren){ paren = true; }
                }
                one = false;
                r += *iter->first.ptr;
                if(lexicographic_compare(e, c) != 0){
                    std::pair<std::string, bool> s = poly_to_string_impl(e, true);
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
