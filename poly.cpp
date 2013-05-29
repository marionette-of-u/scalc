#include <string>
#include <sstream>
#include <cstdio>
#include <utility>
#include <memory>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include "common.hpp"

namespace poly{
eval_target::eval_target() : type_idx(0){}
eval_target::~eval_target(){}
eval_target::eval_target(const std::size_t a_type_idx) : type_idx(a_type_idx){}

bool eval_target::equal(const eval_target *other) const{
    return this == other || type_idx == other->type_idx;
}

// factor
class factor : public eval_target{
public:
    factor() : eval_target(get_type_idx<factor>()){}

    virtual bool equal(const eval_target *other) const{
        if(!eval_target::equal(other)){
            return false;
        }
        const factor *ptr = static_cast<const factor*>(other);
        if(real != ptr->real || imag != ptr->imag){
            return false;
        }
        for(auto iter = e.begin(); iter != e.end(); ++iter){
            auto jter = ptr->e.find(iter->first);
            if(jter == ptr->e.end() || !iter->second->equal(jter->second.get())){
                return false;
            }
        }
        for(auto iter = ptr->e.begin(); iter != ptr->e.end(); ++iter){
            if(e.find(iter->first) == e.end()){ return false; }
        }
        return true;
    }

    virtual eval_target *copy() const{
        factor *other = new factor;
        for(auto iter = e.begin(); iter != e.end(); ++iter){
            other->e[iter->first].reset(iter->second->copy());
        }
        other->real = real;
        other->imag = imag;
        return other;
    }

    virtual void negate(){
        real = -real, imag = -imag;
    }

    virtual void complex_conjugate(){
        imag = -imag;
    }

    // 指数
    std::map<std::string, std::unique_ptr<eval_target>> e;

    //実部, 虚部
    fpoint real, imag;
};

factor *new_factor(fpoint re, fpoint im){
    factor *t = new factor;
    t->real = re, t->imag = im;
    return t;
}

factor *new_factor(fpoint re, fpoint im, const std::string &str){
    factor *t = new factor;
    t->real = re, t->imag = im;
    t->e[str].reset(new_factor(1, 0));
    return t;
}

factor *new_factor(fpoint re, fpoint im, const std::string &str, eval_target *e){
    factor *t = new factor;
    t->real = re, t->imag = im;
    t->e[str].reset(e);
    return t;
}

// identity
class identity : public eval_target{
public:
    identity() : eval_target(get_type_idx<identity>()){}
    virtual ~identity(){
        dispose(expression);
    }

    virtual bool equal(const eval_target *other) const{
        if(!eval_target::equal(other)){
            return false;
        }
        const identity *ptr = static_cast<const identity*>(other);
        for(node *p = expression->next, *q = ptr->expression->next; ; p = p->next, q = q->next){
            if(!p->value->equal(q->value.get())){ return false; }
            if(!p && !q){
                break;
            }else if(p && !q || !p && q){
                return false;
            }
        }
        return true;
    }

    virtual eval_target *copy() const{
        identity *e = new identity;
        e->expression = poly::copy(expression);
        return e;
    }

    virtual void negate(){
        poly::change_sign(expression);
    }

    virtual void complex_conjugate(){
        poly::complex_conjugate(expression);
    }

    node *expression;
};

// binary operator
class binary_operator : public eval_target{
public:
    binary_operator(std::size_t idx) : eval_target(idx){}

    virtual bool equal(const eval_target *other) const{
        if(!eval_target::equal(other)){
            return false;
        }
        const binary_operator *ptr = static_cast<const binary_operator*>(other);
        return lhs->equal(ptr->lhs.get()) && rhs->equal(ptr->rhs.get());
    }

    virtual void negate(){
        lhs->negate();
        rhs->negate();
    }

    virtual void complex_conjugate(){
        lhs->complex_conjugate();
        rhs->complex_conjugate();
    }

    std::unique_ptr<eval_target> lhs, rhs;

private:
    binary_operator() : eval_target(0){}
};

// add
class add : public binary_operator{
public:
    add() : binary_operator(get_type_idx<add>()){}

    virtual eval_target *copy() const{
        add *ptr = new add;
        ptr->lhs.reset(lhs->copy());
        ptr->rhs.reset(rhs->copy());
        return ptr;
    }
};

// sub
class sub : public binary_operator{
public:
    sub() : binary_operator(get_type_idx<sub>()){}

    virtual eval_target *copy() const{
        sub *ptr = new sub;
        ptr->lhs.reset(lhs->copy());
        ptr->rhs.reset(rhs->copy());
        return ptr;
    }
};

// multiply
class multiply : public binary_operator{
public:
    multiply() : binary_operator(get_type_idx<multiply>()){}

    virtual eval_target *copy() const{
        multiply *ptr = new multiply;
        ptr->lhs.reset(lhs->copy());
        ptr->rhs.reset(rhs->copy());
        return ptr;
    }
};

// division
class division : public binary_operator{
public:
    division() : binary_operator(get_type_idx<division>()){}

    virtual eval_target *copy() const{
        division *ptr = new division;
        ptr->lhs.reset(lhs->copy());
        ptr->rhs.reset(rhs->copy());
        return ptr;
    }
};

// power
class power : public binary_operator{
public:
    power() : binary_operator(get_type_idx<power>()){}

    virtual eval_target *copy() const{
        power *ptr = new power;
        ptr->lhs.reset(lhs->copy());
        ptr->rhs.reset(rhs->copy());
        return ptr;
    }
};

node::node() : value(nullptr), next(nullptr){}

template<class T>
inline std::string to_string(const T &v){
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
        q->value.reset(new_factor(re, im));
    }
    return p;
}

// 変数を生成
node *variable(const std::string &str){
    node *p = constant(1, 0);
    static_cast<factor*>(p->value.get())->e[str].reset(new_factor(1, 0));
    return p;
}

// 多項式をコピー
node *copy(node *p){
    node *q, *r;
    q = r = new_node();
    while(p = p->next){
        r = r->next = new_node();
        r->value.reset(p->value->copy());
    }
    return q;
}

// 符号を反転
void change_sign(node *p){
    while(p = p->next){
        p->value->negate();
    }
}

// 複素共役
void complex_conjugate(node *p){
    while(p = p->next){
        p->value->complex_conjugate();
    }
}

// 加算
// qは消す
void add(node *p, node *q){
    //node *p1 = p, *q1 = q;
    //e_type ep = 0, eq = 0;
    //p = p->next;
    //q = q->next;
    //dispose_node(q1);
    //while(q){
    //    while(p){
    //        for(auto p_iter = p->e.begin(); p_iter != p->e.end(); ++p_iter){
    //            ep = p_iter->second;
    //            auto q_iter = q->e.find(p_iter->first);
    //            if(q_iter == q->e.end()){
    //                eq = 0;
    //                goto breakpoint;
    //            }else{ eq = q_iter->second; }
    //            if(ep != eq){ goto breakpoint; }
    //        }
    //        for(auto q_iter = q->e.begin(); q_iter != q->e.end(); ++q_iter){
    //            eq = q_iter->second;
    //            auto p_iter = p->e.find(q_iter->first);
    //            if(p_iter != p->e.end()){ continue; }else{
    //                ep = 0;
    //                goto breakpoint;
    //            }
    //        }
    //        breakpoint:;
    //        if(ep <= eq){ break; }
    //        p1 = p, p = p->next;
    //    }
    //    if(!p || ep < eq){
    //        p1->next = q, p1 = q, q = q->next;
    //        p1->next = p;
    //    }else{
    //        p->real += q->real;
    //        p->imag += q->imag;
    //        if(p->real != 0 || p->imag != 0){
    //            p1 = p;
    //            p = p->next;
    //        }else{
    //            p = p->next;
    //            dispose_node(p1->next);
    //            p1->next = p;
    //        }
    //        q1 = q, q = q->next, dispose_node(q1);
    //    }
    //}
}

// 乗算
// 新たな多項式を返す
node *multiply(node *x, node *y){
    return nullptr;
    //e_type ep = 0, eq = 0;
    //node *p, *p1, *q, *r, *z;
    //r = new_node(), q = nullptr;
    //while(y = y->next){
    //    p1 = r, p = p1->next, z = x;
    //    while(z = z->next){
    //        if(!q){ q = new_node(); }
    //        q->real = y->real * z->real - y->imag * z->imag;
    //        q->imag = y->real * z->imag + y->imag * z->real;
    //        q->e = y->e;
    //        for(auto iter = z->e.begin(); iter != z->e.end(); ++iter){
    //            q->e[iter->first] += iter->second;
    //        }
    //        while(p){
    //            for(auto p_iter = p->e.begin(); p_iter != p->e.end(); ++p_iter){
    //                ep = p_iter->second;
    //                auto q_iter = q->e.find(p_iter->first);
    //                if(q_iter == q->e.end()){
    //                    eq = 0;
    //                    goto breakpoint;
    //                }else{ eq = q_iter->second; }
    //                if(ep != eq){ goto breakpoint; }
    //            }
    //            for(auto q_iter = q->e.begin(); q_iter != q->e.end(); ++q_iter){
    //                eq = q_iter->second;
    //                auto p_iter = p->e.find(q_iter->first);
    //                if(p_iter != p->e.end()){ continue; }else{
    //                    ep = 0;
    //                    goto breakpoint;
    //                }
    //            }
    //            breakpoint:;
    //            if(ep <= eq){ break; }
    //            p1 = p, p = p->next;
    //        }
    //        if(p == nullptr || ep < eq){
    //            p1->next = q, p1 = q, p1->next = p;
    //            q = nullptr;
    //        }else{
    //            p->real += q->real;
    //            p->imag += q->imag;
    //            if(p->real != 0 || p->imag != 0){
    //                p1 = p, p = p->next;
    //            }else{
    //                p = p->next;
    //                dispose_node(p1->next);
    //                p1->next = p;
    //            }
    //        }
    //    }
    //}
    //if(q){ dispose_node(q); }
    //return r;
}

// 累乗
// xは破棄する
node *power(node *x, eval_target *n){
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

// 式を文字列として得る
std::string poly_to_string(const node *p){
    return "";
    //bool first, one;
    //c_type re, im;
    //e_type e;
    //std::string r;

    //first = true;
    //while(p = p->next){
    //    one = false;
    //    re = p->real;
    //    im = p->imag;
    //    if(im == 0){
    //        if(re >= 0){
    //            if(!first){ r += "+"; }
    //        }else{
    //            re = -re;
    //            r += "-";
    //        }
    //        if(re == 1){ one = true; }else{ r += to_string(re); }
    //    }else if(re == 0){
    //        if(im >= 0){
    //            if(!first){ r += "+"; }
    //        }else{
    //            im = -im;
    //            r += "-";
    //        }
    //        r += (std::abs(im) == 1 ? std::string("") : to_string(im)) + "i";
    //    }else{
    //        if(!first){ r += "+"; }
    //        bool nega;
    //        if(nega = im < 0){ im = -im; }
    //        r += to_string(re) + (nega ? "-" : "+") + (std::abs(im) == 1 ? std::string("") : to_string(im)) + "i";
    //    }
    //    first = false;
    //    for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
    //        if((e = iter->second) != 0){
    //            if(!one){ r += "*"; }
    //            one = false;
    //            r += iter->first;
    //            if(e != 1){
    //                r += "^" + to_string(e);
    //            }
    //        }
    //    }
    //    if(one){ r += "1"; }
    //}
    //if(first){ r += "0"; }
    //return r;
}

}
