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
    std::int64_t t = 0, u = 1, tt = 0;
    while(x >= 2 && (x & 1) == 0){
        ++tt;
        x >>= 1;
    }
    std::int64_t d = 3, q = x / d;
    while(q >= d){
        if(x % d == 0){
            ++t;
            x = q;
        }else{
            if(t > 0){
                if(d == x_prime){ ++t; }
                if((t * num) % den != 0){ return x_prime; }
                u *= static_cast<std::int64_t>(std::powl(long double(d), long double(t * num / den)));
            }
            d += 2;
            t = 0;
        }
        q = x / d;
    }
    ++t;
    if(t > 1 && (t * num) % den != 0){ return x_prime; }
    u *= static_cast<std::int64_t>(std::powl(long double(x), long double(t * num / den)));
    u *= static_cast<std::int64_t>(std::powl(long double(2), long double(tt * num / den)));
    return u;
}

// lexicographical_compare (int ver.)
template<class LhsIter, class RhsIter>
int int_lexicographical_compare(LhsIter first1, LhsIter last1, RhsIter first2, RhsIter last2){
    for(; first1 != last1 && first2 != last2; ++first1, ++first2){
        if(*first1 < *first2){ return -1; }
        if(*first2 < *first1){ return +1; };
    }
    return first1 == last1 && first2 == last2 ? 0 : first1 != last1 ? -1 : +1;
}

algebraic::algebraic() : value(), next(nullptr){}

algebraic::algebraic(algebraic &&other) :
    value(std::move(other.value)),
    next(std::move(other.next))
{}

void algebraic::test(){
    auto linked_multiply = [](const algebraic *x, const algebraic *y) -> algebraic*{
        auto prime_multiply = [](const rational &e, const explicit_exponential_rational &r){
            std::int64_t n = e.numerator(), d = e.denominator();
            std::int64_t s = factorize_nd(r.numerator(), n, d), t = factorize_nd(r.denominator(), n, d);
            return explicit_exponential_rational(s, t);
        };

        algebraic *p, *p1, *q = nullptr, *r = new_node();
        const algebraic *z;
        while(y = y->next){
            p1 = r, p = p1->next, z = x;
            while(z = z->next){
                if(!q){ q = new_node(); }
                q->value.insert(std::make_pair(0, 1));
                {
                    auto iter = y->value.begin(); ++iter;
                    for(; iter != y->value.end(); ++iter){
                        auto jter = z->value.find(iter->first);
                        if(jter == z->value.end()){ continue; }
                        explicit_exponential_rational r = iter->second * jter->second;
                        explicit_exponential_rational v = prime_multiply(iter->first, r);
                        if(v == r){
                            q->value.insert(std::make_pair(iter->first, v));
                        }else{
                            q->value[0] *= v.numerator();
                        }
                    }
                }
                const algebraic *a[2][2] = { { y, z }, { z, y } };
                for(int i = 0; i < 2; ++i){
                    auto iter = a[i][0]->value.begin(); ++iter;
                    for(; iter != a[i][0]->value.end(); ++iter){
                        auto jter = a[i][1]->value.find(iter->first);
                        if(jter != a[i][1]->value.end()){ continue; }
                        q->value.insert(*iter);
                    }
                }
                int compare_result = 0;
                while(p){
                    auto iter = p->value.begin(); ++iter;
                    auto jter = q->value.begin(); ++jter;
                    compare_result = int_lexicographical_compare(iter, p->value.end(), jter, q->value.end());
                    if(compare_result <= 0){ break; }
                    p1 = p, p = p->next;
                }
                if(!p || compare_result < 0){
                    p1->next = q, p1 = q, p1->next = p;
                    q = nullptr;
                }else{
                    p->value[0] += q->value[0];
                    if(p->value[0] != 0){
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
    };

    //algebraic *p, *q, *r, *a, *b, *c, *d;

    //p = constant(explicit_exponential_rational(3, 1), rational(1, 3));
    //q = constant(explicit_exponential_rational(3, 1), rational(1, 3));
    //r = linked_multiply(p, q);
    //q = constant(explicit_exponential_rational(2, 1), rational(1, 2));
    //a = linked_multiply(r, q);

    //p = constant(explicit_exponential_rational(3, 1), rational(1, 3));
    //q = constant(explicit_exponential_rational(2, 1), rational(1, 2));
    //b = linked_multiply(p, q);

    //add(a, b);
    //c = copy(a);

    //d = linked_multiply(a, c);

    explicit_exponential_rational a = factorize_nd(81, 1, 3);
}

algebraic *algebraic::multiply(const algebraic *x, const algebraic *y){

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
        int compare_result;
        while(p){
            auto iter = p->value.begin(), jter = q->value.begin();
            ++iter, ++jter;
            compare_result = int_lexicographical_compare(iter, p->value.end(), jter, q->value.end());
            if(compare_result <= 0){ break; }
            p1 = p, p = p->next;
        }
        if(!p || compare_result < 0){
            p1->next = q, p1 = q, q = q->next;
            p1->next = p;
        }else{
            p->value[0] += q->value[0];
            if(p->value[0] != 0){
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
        explicit_exponential_rational &r = p->value.begin()->second;
        r = -r;
    }
}

algebraic *algebraic::copy(const algebraic *p){
    if(!p){ return nullptr; }
    algebraic *q, *r;
    q = r = new_node();
    while(p = p->next){
        r = r->next = new_node();
        r->value = p->value;
    }
    return q;
}

algebraic *algebraic::constant(explicit_exponential_rational n, const rational &e){
    algebraic *p = new_node();
    if(n != 0){
        algebraic *q = new_node();
        if(e != 0){
            q->value.insert(std::make_pair(0, n < 0 ? -1 : +1));
            if(n < 0){ n = -n; }
        }
        q->value.insert(std::make_pair(e, n));
        p->next = q;
    }
    return p;
}

template<bool Cond>
int compare_common(const algebraic *lhs, const algebraic *rhs){
    if(!lhs && !rhs){ return 0; }
    if(!lhs && rhs){ return rhs->value.begin()->second >= 0 ? -1 : +1; }
    if(lhs && !rhs){ return lhs->value.begin()->second >= 0 ? +1 : -1; }
    return
        lhs->value < rhs->value
            ? (rhs->value.begin()->second >= 0 ? -1 : +1)
            : lhs->value > rhs->value
                ? (lhs->value.begin()->second >= 0 ? +1 : -1)
                : Cond
                    ? compare_common<true>(lhs->next, rhs->next)
                    : 0;
}

int algebraic::compare(const algebraic *lhs, const algebraic *rhs){
    return compare_common<true>(lhs, rhs);
}

int algebraic::compare_term(const algebraic *lhs, const algebraic *rhs){
    return compare_common<false>(lhs, rhs);
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
        p = q->next;
        dispose_node(q);
        q = p;
    }
}

} // namespace algebraic_impl
