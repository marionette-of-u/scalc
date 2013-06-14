#include <map>
#include "algebraic.hpp"

namespace algebraic_impl{

// 素因数分解 
std::map<int, std::size_t> factorize(int x){
    std::map<int, std::size_t> r;
    std::size_t t = 0;
    while(x >= 4 && (x & 1) == 0){
        ++t;
        x >>= 1;
    }
    r[2] = t;
    t = 0;
    int d = 3, q = x / d;
    while(q >= d){
        if(x % d == 0){
            ++t;
            x = q;
        }else{
            r[d] = t;
            d += 2;
            t = 0;
        }
        q = x / d;
    }
    r[d] = t;
    ++r[x];
    return r;
}

term::term() : value(), e(nullptr), next(nullptr){}
term::term(int n) : value(n), e(nullptr), next(nullptr){}
term::term(const rational &n) : value(n), e(nullptr), next(nullptr){}

void term::add(term *p, term *q){
    term *p1, *q1;
    p1 = p, p = p->next;
    q1 = q, q = q->next;
    dispose_node(q1);
    while(q){
        int compare_result;
        while(p){
            compare_result = sequential_compare(p->e, q->e);
            if(compare_result <= 0){ break; }
            p1 = p, p = p->next;
        }
        if(!p || compare_result < 0){
            p1->next = q, p1 = q, q = q->next;
            p1->next = p;
        }else{
            p->value += q->value;
            if(p->value != 0){
                p1 = p, p = p->next;
            }else{
                p = p->next;
                dispose_node(p1->next);
                p1->next = p;
            }
            q1 = q, q = q->next, dispose_node(q1);
        }
    }
}

void term::change_sign(term *p){
    while(p = p->next){
        p->value = -p->value;
    }
}

term *term::copy(const term *p){
    term *q, *r;
    q = r = new term;
    while(p = p->next){
        r = r->next = new term(p->value);
        r->e = new term(*p->e);
    }
    return q;
}

term *term::constant(rational n){
    term *p = new term, *q;
    if(n != 0){
        q = new term(n);
        p->next = q;
        p->next->e = new term;
    }
    return p;
}

int term::sequential_compare(const term *lhs, const term *rhs){
    const term *p = lhs, *q = rhs;
    if(!p && !q){ return 0; }
    p = p->next, q = q->next;
    while(p && q){
        p = p->next, q = q->next;
        if(p->value < q->value){
            return -1;
        }else if(p->value > q->value){
            return +1;
        }
        int exponent_result = sequential_compare(p->e, q->e);
        if(exponent_result != 0){
            return exponent_result;
        }
    }
    if(!p && q){ return -1; }
    if(p && !q){ return +1; }
    return 0;
}

void term::dispose_node(term *p){
    if(p->e){ dispose(p->e); }
    delete p;
}

void term::dispose(term *p){
    if(!p){ return; }
    term *q = p;
    while(q->next){
        p = q;
        q = q->next;
        dispose(p->e);
        delete p;
    }
}

} // namespace algebraic_impl
