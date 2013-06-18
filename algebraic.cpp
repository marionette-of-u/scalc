#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <cmath>
#include "common.hpp"
#include "algebraic.hpp"

namespace algebraic_impl{
// x_prime^(num / den) = a in N
// OR
// x_prime
std::int64_t factorize_nd(std::int64_t x_prime, std::int64_t num, std::int64_t den){
    std::int64_t x = x_prime;
    std::vector<std::pair<std::int64_t, std::int64_t>> r;
    std::size_t t = 0;
    while(x >= 4 && (x & 1) == 0){
        ++t;
        x >>= 1;
    }
    if(t > 0){ r.push_back(std::make_pair(2, t)); }
    t = 0;
    std::int64_t d = 3, q = x / d;
    while(q >= d){
        if(x % d == 0){
            ++t;
            x = q;
        }else{
            if(t > 0){
                if(d == x_prime){ ++t; }
                if((t * num) % den != 0){ return x_prime; }
                r.push_back(std::make_pair(d, t));
            }
            d += 2;
            t = 0;
        }
        q = x / d;
    }
    auto iter = std::find_if(
        r.begin(),
        r.end(),
        [x](const std::pair<std::int64_t, std::int64_t> &a){ return a.first == x; }
    );
    if(iter == r.end()){
        r.push_back(std::make_pair(x, t + 1));
        iter = r.end();
        --iter;
    }else{
        iter->second += t + 1;
    }
    if((iter->second * num) % den != 0){ return x_prime; }
    std::int64_t u = 1;
    for(auto iter = r.begin(); iter != r.end(); ++iter){
        u *= static_cast<std::int64_t>(std::powl(iter->first, iter->second * num / den));
    }
    return u;
}

algebraic::algebraic() : value(0), next(nullptr), e(nullptr), c(nullptr){}

auto linked_multiply = [](algebraic *p, algebraic *q){
    p = p->next, q = q->next;
    algebraic *p1 = nullptr;
    int compare_result = 0;
    if(q){
        for(; p; p1 = p, p = p->c){
            compare_result = algebraic::compare(p->e, q->e);
            if(compare_result <= 0){ break; }
        }
    }
    if(!p || compare_result < 0){
        (p ? p : p1)->c = algebraic::copy(q);
    }else{
        algebraic *r = algebraic::new_node();
        std::int64_t n = p->e->value.numerator(), d = p->e->value.denominator();
        r->e = algebraic::copy_mono(p->e);
        r->value = p->value * q->value;
        r->value.normalize();
    }
};

void algebraic::test(){
    std::int64_t a = factorize_nd(2244531326976ll, 5, 4);

    //algebraic *a = constant(rational(29, 36));
    //a->next->e = new_node();
    //a->next->e->value = rational(1, 2);

    //algebraic *b = constant(rational(5, 36));
    //b->next->e = new_node();
    //b->next->e->value = rational(1, 2);

    //linked_multiply(a, b);

    return;
}

algebraic *algebraic::multiply(const algebraic *x, const algebraic *y){

    //algebraic *p, *p1, *q, *r = new_node();
    //const algebraic *z;
    //r = new_node(), q = nullptr;
    //while(y = y->next){
    //    p1 = r, p = p1->next, z = x;
    //    while(z = z->next){
    //        if(!q){ q = new_node(); }
    //        // ** TODO **
    //        // q->value = y->value * z->value;
    //        // q->e = y->e + z->e;
    //        while(p){
    //            for(; ; ){
    //                ;
    //            }
    //        }
    //    }
    //}

    return nullptr;
}

void algebraic::sub(algebraic *p, algebraic *q){
    change_sign(q);
    add(p, q);
}

void algebraic::add(algebraic *p, algebraic *q){
    algebraic *p1, *q1;
    p1 = p, p = p->next;
    q1 = q, q = q->next;
    dispose_node(q1);
    while(q){
        int exponent_compare_result;
        while(p){
            exponent_compare_result = compare(p->c, q->c, true);
            if(exponent_compare_result == 0){ exponent_compare_result = compare(p->e, q->e); }
            if(exponent_compare_result <= 0){ break; }
            p1 = p, p = p->next;
        }
        if(!p || exponent_compare_result < 0){
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

void algebraic::change_sign(algebraic *p){
    while(p = p->next){
        p->value = -p->value;
    }
}

algebraic *algebraic::copy_mono(const algebraic *p){
    if(!p){ return nullptr; }
    algebraic *q = new_node();
    q->value = p->value;
    q->e = copy_mono(p->e);
    q->c = copy_mono(p->c);
    return q;
}

algebraic *algebraic::copy(const algebraic *p){
    if(!p){ return nullptr; }
    algebraic *q, *r;
    q = r = new_node();
    while(p){
        r = r->next = new_node();
        r->value = p->value;
        r->e = copy_mono(p->e);
        r->c = copy_mono(p->c);
        p = p->next;
    }
    return q;
}

algebraic *algebraic::constant(const rational &n){
    algebraic *p = new_node();
    if(n != 0){
        algebraic *q = new_node();
        q->value = n;
        p->next = q;
    }
    return p;
}

int algebraic::compare(const algebraic *lhs, const algebraic *rhs, bool mono){
    auto factor_class = [](const algebraic *f) -> int{
        if(is_exist(f->e)){
            if(is_exist(f->c)){
                return 3;
            }
            return 2;
        }else{
            if(is_exist(f->c)){
                return 1;
            }
            return 0;
        }
    };
    if(!lhs && !rhs){ return 0; }
    if(!lhs && rhs){ return -1; }
    if(lhs && !rhs){ return +1; }
    if(!mono){
        lhs = lhs->next, rhs = rhs->next;
        if(!lhs && !rhs){ return 0; }
        if(!lhs && rhs){ return -1; }
        if(lhs && !rhs){ return +1; }
    }
    algebraic *lhs_e = lhs->e, *rhs_e = rhs->e;
    algebraic *lhs_c = lhs->c, *rhs_c = rhs->c;
    int lhs_class = factor_class(lhs), rhs_class = factor_class(rhs);
    if(lhs_class < rhs_class){
        return -1;
    }else if(lhs_class > rhs_class){
        return +1;
    }
    int result;
    switch(lhs_class){
    case 3:
        result = compare(lhs_e, rhs_e);
        if(result == 0){ result = compare(lhs_c, rhs_c); }
        if(result == 0){ result = primitive_compare(abs(lhs->value), abs(rhs->value)); }
        break;

    case 2:
        result = compare(lhs_e, rhs_e);
        if(result == 0){ result = primitive_compare(abs(lhs->value), abs(rhs->value)); }
        break;

    case 1:
        result = compare(lhs_c, rhs_c);
        if(result == 0){ result = primitive_compare(abs(lhs->value), abs(rhs->value)); }
        break;

    case 0:
        result = primitive_compare(abs(lhs->value), abs(rhs->value));
        break;

    default:
        result = 0;
    }
    return result;
}

algebraic *algebraic::new_node(){
    return new algebraic;
}

void algebraic::dispose_node(algebraic *p){
    delete p;
}

void algebraic::dispose(algebraic *p){
    if(!p){ return; }
    algebraic *q = p;
    while(q->next){
        dispose(q->e);
        dispose(q->c);
        p = q->next;
        dispose_node(q);
        q = p;
    }
}

bool algebraic::is_exist(const algebraic *p){
    return p != nullptr;
}

std::ostream& operator<< (std::ostream& os, const algebraic &a){
    const algebraic *p = &a;
    while(p = p->next){
        if(p != a.next){ os << "+"; }
        os << p->value;
        if(p->c){
            os << "*(";
            os << *(p->c);
            os << ")";
        }
        if(p->c && p->c->e){
            os << "^(";
            os << *(p->c->e);
            os << ")";
        }
    }
    return os;
}

} // namespace algebraic_impl
