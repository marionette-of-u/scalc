#include "common.hpp"
#include "algebraic.hpp"

namespace algebraic_impl{

// 素因数分解 
std::map<std::int64_t, std::size_t> factorize(std::int64_t x, std::size_t coefficient){
    std::map<std::int64_t, std::size_t> r;
    std::size_t t = 0;
    while(x >= 4 && (x & 1) == 0){
        ++t;
        x >>= 1;
    }
    r[2] += t * coefficient;
    t = 0;
    std::int64_t d = 3, q = x / d;
    while(q >= d){
        if(x % d == 0){
            ++t;
            x = q;
        }else{
            r[d] += t * coefficient;
            d += 2;
            t = 0;
        }
        q = x / d;
    }
    r[d] += t * coefficient;
    if(x > 0){ r[x] += coefficient; }
    return r;
}

algebraic::algebraic() : value(0), e(nullptr), c(nullptr), next(nullptr){}

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
            exponent_compare_result = compare(p->e, q->e);
            if(exponent_compare_result == 0){ exponent_compare_result = compare(p->c, q->c, true); }
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

algebraic *algebraic::copy(const algebraic *p){
    if(!p){ return nullptr; }
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
