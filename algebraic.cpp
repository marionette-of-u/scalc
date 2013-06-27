#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <cmath>
#include "common.hpp"
#include "algebraic.hpp"

namespace algebraic_impl{
class int_multiply_iterator : public std::insert_iterator<algebraic::value_type>{
public:
    typedef std::insert_iterator<algebraic::value_type> parent_type;
    int_multiply_iterator(algebraic::value_type &c) : parent_type(c, c.begin()){}

    int_multiply_iterator &operator =(const std::int64_t n){
        auto iter = container->find(0);
        if(iter == container->end()){
            container->insert(std::make_pair(0, n));
        }else{
            iter->second *= n;
        }
        return *this;
    }

private:
    int_multiply_iterator &operator =(const algebraic::value_type::value_type &v);
    int_multiply_iterator &operator =(algebraic::value_type::value_type &&v);
};

class nth_root_multiply_iterator : public std::insert_iterator<algebraic::value_type>{
public:
    typedef std::insert_iterator<algebraic::value_type> parent_type;
    nth_root_multiply_iterator(algebraic::value_type &c) : parent_type(c, c.begin()){}

    nth_root_multiply_iterator &operator =(const std::pair<rational, std::int64_t> &n){
        auto iter = container->find(n.first);
        if(iter == container->end()){
            container->insert(std::make_pair(n.first, n.second));
        }else{
            iter->second *= n.second;
        }
        return *this;
    }
};

void normalize_nth_root(algebraic::value_type &value, std::int64_t x, const rational &p){
    int_multiply_iterator in_int_iter(value);
    nth_root_multiply_iterator root_iter(value);
    const std::int64_t target = x;
    std::int64_t t = 0, u;
    bool f = true;
    // 最小の素数は処理できないので単体で処理しておく
    if(x == 2){
        root_iter = std::make_pair(p, 2);
        return;
    }
    // 最小の素数を除去
    while(x >= 2 && (x & 1) == 0){ ++t, x /= 2; }
    if(t > 1){
        f = false;
        in_int_iter = static_cast<std::int64_t>(std::pow(static_cast<long double>(2), static_cast<long double>(t * p.numerator() / p.denominator())));
        u = (t * p.numerator()) % p.denominator();
        if(u > 0){
            rational a(u, p.denominator());
            a.normalize();
            root_iter = std::make_pair(a, 2);
        }
    }
    // 最小の奇素数は処理できないので単体で処理しておく
    if(x == 3){
        if(t == 1){ x *= 2; }
        root_iter = std::make_pair(p, x);
        return;
    }
    std::int64_t d = 3, q = x / d;
    t = 0;
    while(q >= d){
        if(x % d == 0){
            ++t, x = q;
        }else{
            if(t > 1){
                f = false;
                if(d == target){ ++t; }
                in_int_iter = static_cast<std::int64_t>(std::pow(static_cast<long double>(d), static_cast<long double>(t * p.numerator() / p.denominator())));
                u = (t * p.numerator()) % p.denominator();
                if(u > 0){
                    rational a(u, p.denominator());
                    a.normalize();
                    root_iter = std::make_pair(a, d);
                }
            }
            d += 2;
            t = 0;
        }
        q = x / d;
    }
    ++t;
    if(t > 1){
        f = false;
        u = (t * p.numerator()) % p.denominator();
        if(u > 0){
            rational a(u, p.denominator());
            a.normalize();
            root_iter = std::make_pair(a, x);
        }
        u = (t * p.numerator()) / p.denominator();
        if(u > 0){
            in_int_iter = static_cast<std::int64_t>(std::pow(static_cast<long double>(d), static_cast<long double>(u)));
        }
    }
    // 入力が既に素数だった場合
    if(f){ root_iter = std::make_pair(p, x); }
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
        algebraic *p, *p1, *q = nullptr, *r = new_node();
        const algebraic *z;
        while(y = y->next){
            p1 = r, p = p1->next, z = x;
            while(z = z->next){
                if(!q){ q = new_node(); }
                std::function<void(std::vector<value_type>&, value_type, value_type)> rec;
                rec = [&rec](std::vector<value_type> &fq_vec, value_type fy, value_type fz) -> void{
                    if(!fy.empty() && !fz.empty()){
                        auto iter = fy.begin();
                        for(; iter != fy.end(); ++iter){
                            for(auto jter = fz.begin(); jter != fz.end(); ){
                                const rational &ye = iter->first;
                                const explicit_exponential_rational &yc = iter->second;
                                const rational &ze = jter->first;
                                const explicit_exponential_rational &zc = jter->second;
                                if(ye == ze){
                                    value_type fq;
                                    explicit_exponential_rational fc = yc * zc;
                                    normalize_nth_root(fq, fc.numerator(), ye);
                                    for(auto kter = fq.begin(); !fq.empty() && kter != fq.end(); ){
                                        explicit_exponential_rational fc_prime(kter->second.numerator(), fc.denominator());
                                        fc_prime.normalize();
                                        if(kter->first.numerator() == 0 && fc_prime.numerator() == 1 && fc_prime.denominator() == 1){
                                            kter = fq.erase(kter);
                                        }else{
                                            kter->second = fc_prime;
                                            ++kter;
                                        }
                                    }
                                    iter = fy.erase(iter);
                                    jter = fz.erase(jter);
                                    fq_vec.push_back(std::move(fq));
                                    if(fy.empty() || fz.empty()){
                                        break;
                                    }else{
                                        continue;
                                    }
                                }else if(ye.denominator() == ze.denominator() && yc == zc){
                                    value_type fq;
                                    rational fe(ye.numerator() + ze.numerator(), ye.denominator());
                                    fe.normalize();
                                    if(fe == 1){ fe = 0; }
                                    normalize_nth_root(fq, yc.numerator(), fe);
                                    iter = fy.erase(iter);
                                    jter = fz.erase(jter);
                                    fq_vec.push_back(std::move(fq));
                                    if(fy.empty() || fz.empty()){
                                        break;
                                    }else{
                                        continue;
                                    }
                                }else if(fy.size() == 1 && fz.size() == 1){
                                    value_type fq;
                                    fq.insert(std::move(*fy.begin()));
                                    fq.insert(std::move(*fz.begin()));
                                    iter = fy.erase(iter);
                                    jter = fz.erase(jter);
                                    fq_vec.push_back(std::move(fq));
                                    if(fy.empty() || fz.empty()){
                                        break;
                                    }else{
                                        continue;
                                    }
                                }
                                ++jter;
                            }
                            if(fy.empty() || fz.empty()){ break; }
                        }
                    }
                    if(!fy.empty()){
                        for(auto iter = fy.begin(); iter != fy.end(); ++iter){
                            value_type fq;
                            fq.insert(std::move(*iter));
                            fq_vec.push_back(std::move(fq));
                        }
                    }
                    if(!fz.empty()){
                        for(auto iter = fz.begin(); iter != fz.end(); ++iter){
                            value_type fq;
                            fq.insert(std::move(*iter));
                            fq_vec.push_back(std::move(fq));
                        }
                    }
                    if(fq_vec.size() > 1){
                        value_type ay = std::move(fq_vec.back());
                        fq_vec.pop_back();
                        value_type az = std::move(fq_vec.back());
                        fq_vec.pop_back();
                        rec(fq_vec, std::move(ay), std::move(az));
                    }
                };
                std::vector<value_type> factor;
                rec(factor, std::move(y->value), std::move(z->value));
                q->value = std::move(factor.back());
                int compare_result = 0;
                if(!p){ p = r; }
                while(p){
                    auto iter = p->value.begin();
                    auto jter = q->value.begin();
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

    // ----
    //algebraic *a = constant(12, rational(1, 3));
    //algebraic *b = constant(12, rational(1, 4));
    //algebraic *c = constant(144, rational(1, 4));
    //algebraic *d = constant(4, rational(1, 4));
    //algebraic *e = constant(9, rational(1, 2));

    // ----
    //algebraic *a = constant(3, rational(1, 2));
    //add(a, constant(4, rational(1, 4)));
    //algebraic *p = constant(5, rational(3, 5));
    //add(p, constant(3, rational(1, 2)));
    //algebraic *r = linked_multiply(a, p);

    // ----
    algebraic *p, *q, *a, *b, *c;
    p = constant(3, rational(2, 3));
    q = constant(2, rational(1, 2));
    a = linked_multiply(p, q);
    p = constant(3, rational(1, 3));
    q = constant(2, rational(1, 2));
    b = linked_multiply(p, q);
    add(a, b);
    c = linked_multiply(a, a);
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
            compare_result = int_lexicographical_compare(jter, q->value.end(), iter, p->value.end());
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
        normalize_nth_root(q->value, n.numerator(), e);
        for(auto iter = q->value.begin(); iter != q->value.end(); ++iter){
            explicit_exponential_rational a(iter->second.numerator(), n.denominator());
            a.normalize();
            iter->second = a;
        }
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
