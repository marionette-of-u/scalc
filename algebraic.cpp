#include "common.hpp"
#include "algebraic.hpp"

namespace algebraic_impl{

// 素因数分解 
std::map<std::uint64_t, std::size_t> factorize(std::uint64_t x){
    std::map<std::uint64_t, std::size_t> r;
    std::size_t t = 0;
    while(x >= 4 && (x & 1) == 0){
        ++t;
        x >>= 1;
    }
    r[2] = t;
    t = 0;
    std::uint64_t d = 3, q = x / d;
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

algebraic::algebraic() : value(0), e(nullptr), c(nullptr), next(nullptr){}

void algebraic::change_sign(algebraic *p){
    while(p = p->next){
        p->value = -p->value;
    }
}

algebraic *algebraic::copy(const algebraic *p){
    algebraic *q, *r;
    q = r = new_node();
    while(p = p->next){
        r = r->next = new_node();
        r->value = p->value;
        r->e = copy(p->e);
        r->c = copy(p->c);
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

int algebraic::compare(const algebraic *lhs, const algebraic *rhs){
    // note: priority of class
    // Q > Coefficient * Q > Q^Exponent > Coefficient * Q^Exponent
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
    algebraic *lhs_e = lhs->e, *rhs_e = rhs->e;
    algebraic *lhs_c = lhs->c, *rhs_c = rhs->c;
    lhs = lhs->next, rhs = rhs->next;
    if(!lhs && !rhs){ return 0; }
    if(!lhs && rhs){ return -1; }
    if(lhs && !rhs){ return +1; }
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
        if(result == 0){ result = primitive_compare(lhs->value, rhs->value); }
        if(result == 0){ result = compare(lhs, rhs); }
        break;

    case 2:
        result = compare(lhs_e, rhs_e);
        if(result == 0){ result = primitive_compare(lhs->value, rhs->value); }
        if(result == 0){ result = compare(lhs, rhs); }
        break;

    case 1:
        result = compare(lhs_c, rhs_c);
        if(result == 0){ result = primitive_compare(lhs->value, rhs->value); }
        if(result == 0){ result = compare(lhs, rhs); }
        break;

    case 0:
        result = primitive_compare(lhs->value, rhs->value);
        if(result == 0){ result = compare(lhs, rhs); }
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
    algebraic *q = p;
    while(q->next){
        dispose(q->e);
        dispose(q->c);
        q = q->next;
        dispose_node(q);
    }
}

bool algebraic::is_exist(const algebraic *p){
    return p && p->next;
}

} // namespace algebraic_impl
