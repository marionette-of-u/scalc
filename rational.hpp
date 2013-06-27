#ifndef SCALC_ALGEBRAIC_HPP
#define SCALC_ALGEBRAIC_HPP

#include <iostream>              // for std::istream and std::ostream
#include <ios>                   // for std::noskipws
#include <stdexcept>             // for std::domain_error
#include <string>                // for std::string implicit constructor
#include <cstdlib>               // for std::abs
#include <limits>                // for std::numeric_limits
#include <cstdint>               // for std::int64_t

namespace rational_impl{
//----------------
// rational (from boost 1.53)

class bad_rational : public std::domain_error{
public:
    explicit bad_rational() : std::domain_error("bad rational: zero denominator") {}
};

template<class Signature>
inline std::int64_t gcd_binary(std::int64_t u, std::int64_t v){
    if(u && v){
        std::int64_t shifts = 0;
        while(!(u & 1u) && !(v & 1u)){
            ++shifts;
            u >>= 1;
            v >>= 1;
        }
        std::int64_t r[] = { u, v };
        std::int64_t which = static_cast<bool>(u & 1u);
        do{
            while(!(r[which] & 1u)){
                r[which] >>= 1;
            }
            if(r[!which] > r[which]){
                which ^= 1u;
            }
            r[which] -= r[!which];
        }while(r[which]);
        return r[!which] << shifts;
    }else{
        return u + v;
    }
}

template<class Signature>
inline std::int64_t lcm(std::int64_t u, std::int64_t v){
    return (u / gcd_binary(u, v)) * v;
}

template<bool ImplicitExponentialNumerator = true>
class rational{
private:
    struct helper{ std::int64_t parts[2]; };
    typedef std::int64_t (helper::* bool_type)[2];

public:
    inline rational() : num(0), den(1){}
    inline rational(std::int64_t n) : num(n), den(1){}
    inline rational(std::int64_t n, std::int64_t d) : num(n), den(d){}
    inline rational &operator =(std::int64_t n){ return assign(n, 1); }
    rational &assign(std::int64_t n, std::int64_t d);
    inline std::int64_t numerator () const{ return num; }
    inline std::int64_t denominator () const{ return den; }
    rational &operator +=(const rational &r);
    rational &operator -=(const rational &r);
    rational &operator *=(const rational &r);
    rational &operator /=(const rational &r);
    rational &operator +=(std::int64_t i);
    rational &operator -=(std::int64_t i);
    rational &operator *=(std::int64_t i);
    rational &operator /=(std::int64_t i);
    rational operator +(const rational &r) const;
    rational operator -(const rational &r) const;
    rational operator *(const rational &r) const;
    rational operator /(const rational &r) const;
    const rational &operator ++();
    const rational &operator --();
    bool operator !() const{ return !num; }
    bool operator <(const rational &r) const;
    bool operator >(const rational &r) const;
    bool operator ==(const rational &r) const;
    bool operator !=(const rational &r) const;
    bool operator <=(const rational &r) const;
    bool operator >=(const rational &r) const;
    bool operator <(std::int64_t i) const;
    bool operator >(std::int64_t i) const;
    bool operator ==(std::int64_t i) const;
    void normalize();

private:
    std::int64_t num, den;
    bool test_invariant() const;
};

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::assign(std::int64_t n, std::int64_t d){
    num = n;
    den = d;
    if(ImplicitExponentialNumerator){
        normalize();
    }
    return *this;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> operator +(const rational<ImplicitExponentialNumerator> &r){
    return r;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> operator -(const rational<ImplicitExponentialNumerator> &r){
    return rational<ImplicitExponentialNumerator>(-r.numerator(), r.denominator());
}

template<bool ImplicitExponentialNumerator>
rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::operator+= (const rational<ImplicitExponentialNumerator> &r){
    std::int64_t r_num = r.num;
    std::int64_t r_den = r.den;
    std::int64_t g = gcd_binary<void>(den, r_den);
    den /= g;
    num = num * (r_den / g) + r_num * den;
    g = gcd_binary<void>(num, g);
    num /= g;
    den *= r_den/g;
    return *this;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> rational<ImplicitExponentialNumerator>::operator+ (const rational<ImplicitExponentialNumerator> &r) const{
    rational a = *this;
    a += r;
    return a;
}

template<bool ImplicitExponentialNumerator>
rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::operator-= (const rational<ImplicitExponentialNumerator> &r){
    std::int64_t r_num = r.num;
    std::int64_t r_den = r.den;
    std::int64_t g = gcd_binary<void>(den, r_den);
    den /= g;
    num = num * (r_den / g) - r_num * den;
    g = gcd_binary<void>(num, g);
    num /= g;
    den *= r_den/g;
    return *this;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> rational<ImplicitExponentialNumerator>::operator- (const rational<ImplicitExponentialNumerator> &r) const{
    rational a = *this;
    a -= r;
    return a;
}

template<bool ImplicitExponentialNumerator>
rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::operator*= (const rational<ImplicitExponentialNumerator> &r){
    std::int64_t r_num = r.num;
    std::int64_t r_den = r.den;
    std::int64_t gcd1 = gcd_binary<void>(num, r_den);
    std::int64_t gcd2 = gcd_binary<void>(r_num, den);
    num = (num/gcd1) * (r_num/gcd2);
    den = (den/gcd2) * (r_den/gcd1);
    return *this;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> rational<ImplicitExponentialNumerator>::operator* (const rational<ImplicitExponentialNumerator> &r) const{
    rational a = *this;
    a *= r;
    return a;
}

template<bool ImplicitExponentialNumerator>
rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::operator/= (const rational<ImplicitExponentialNumerator> &r){
    std::int64_t r_num = r.num;
    std::int64_t r_den = r.den;
    std::int64_t zero(0);
    if(r_num == zero)
        throw bad_rational();
    if(num == zero)
        return *this;
    std::int64_t gcd1 = gcd_binary<void>(num, r_num);
    std::int64_t gcd2 = gcd_binary<void>(r_den, den);
    num = (num/gcd1) * (r_den/gcd2);
    den = (den/gcd2) * (r_num/gcd1);
    if(den < zero){
        num = -num;
        den = -den;
    }
    return *this;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator> rational<ImplicitExponentialNumerator>::operator/ (const rational<ImplicitExponentialNumerator> &r) const{
    rational a = *this;
    a += r;
    return a;
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator>& rational<ImplicitExponentialNumerator>::operator+= (std::int64_t i){
    return operator+= (rational<ImplicitExponentialNumerator>(i));
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator>& rational<ImplicitExponentialNumerator>::operator -=(std::int64_t i){
    return operator-= (rational<ImplicitExponentialNumerator>(i));
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator>& rational<ImplicitExponentialNumerator>::operator*= (std::int64_t i){
    return operator*= (rational<ImplicitExponentialNumerator>(i));
}

template<bool ImplicitExponentialNumerator>
inline rational<ImplicitExponentialNumerator>& rational<ImplicitExponentialNumerator>::operator/= (std::int64_t i){
    return operator/= (rational<ImplicitExponentialNumerator>(i));
}

template<bool ImplicitExponentialNumerator>
inline const rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::operator ++(){
    num += den;
    return *this;
}

template<bool ImplicitExponentialNumerator>
inline const rational<ImplicitExponentialNumerator> &rational<ImplicitExponentialNumerator>::operator --(){
    num -= den;
    return *this;
}

template<bool ImplicitExponentialNumerator>
bool rational<ImplicitExponentialNumerator>::operator <(const rational<ImplicitExponentialNumerator> &r) const{
    std::int64_t const  zero(0);
    struct { std::int64_t  n, d, q, r; }  ts = { this->num, this->den, this->num /
     this->den, this->num % this->den }, rs = { r.num, r.den, r.num / r.den,
     r.num % r.den };
    unsigned reverse = 0u;
    while(ts.r < zero){ ts.r += ts.d; --ts.q; }
    while(rs.r < zero){ rs.r += rs.d; --rs.q; }
    while(true){
        if(ts.q != rs.q){
            return reverse ? ts.q > rs.q : ts.q < rs.q;
        }
        reverse ^= 1u;
        if((ts.r == zero) || (rs.r == zero)){
            break;
        }
        ts.n = ts.d;         ts.d = ts.r;
        ts.q = ts.n / ts.d;  ts.r = ts.n % ts.d;
        rs.n = rs.d;         rs.d = rs.r;
        rs.q = rs.n / rs.d;  rs.r = rs.n % rs.d;
    }
    if(ts.r == rs.r){
        return false;
    }else{
        return (ts.r != zero) != (reverse != 0);
    }
}

template<bool ImplicitExponentialNumerator>
bool rational<ImplicitExponentialNumerator>::operator >(const rational<ImplicitExponentialNumerator> &r) const{
    return r.operator <(*this);
}

template<bool ImplicitExponentialNumerator>
bool rational<ImplicitExponentialNumerator>::operator <=(const rational<ImplicitExponentialNumerator> &r) const{
    std::int64_t const  zero(0);
    struct { std::int64_t  n, d, q, r; }  ts = { this->num, this->den, this->num /
     this->den, this->num % this->den }, rs = { r.num, r.den, r.num / r.den,
     r.num % r.den };
    unsigned  reverse = 0u;
    while(ts.r < zero){ ts.r += ts.d; --ts.q; }
    while(rs.r < zero){ rs.r += rs.d; --rs.q; }
    while(true){
        if(ts.q != rs.q){
            return reverse ? ts.q > rs.q : ts.q < rs.q;
        }
        reverse ^= 1u;
        if((ts.r == zero) || (rs.r == zero)){
            break;
        }
        ts.n = ts.d;         ts.d = ts.r;
        ts.q = ts.n / ts.d;  ts.r = ts.n % ts.d;
        rs.n = rs.d;         rs.d = rs.r;
        rs.q = rs.n / rs.d;  rs.r = rs.n % rs.d;
    }
    if(ts.r == rs.r){
        return true;
    }else{
        return (ts.r != zero) != (reverse != 0);
    }
}

template<bool ImplicitExponentialNumerator>
bool rational<ImplicitExponentialNumerator>::operator >=(const rational<ImplicitExponentialNumerator> &r) const{
    return r.operator <=(*this);
}

template<bool ImplicitExponentialNumerator>
bool rational<ImplicitExponentialNumerator>::operator <(std::int64_t i) const{
    std::int64_t const  zero(0);
    std::int64_t  q = this->num / this->den, r = this->num % this->den;
    while(r < zero){ r += this->den; --q; }
    return q < i;
}

template<bool ImplicitExponentialNumerator>
bool rational<ImplicitExponentialNumerator>::operator> (std::int64_t i) const{
    if(num == i && den == std::int64_t(1))
        return false;
    return !operator <(i);
}

template<bool ImplicitExponentialNumerator>
inline bool rational<ImplicitExponentialNumerator>::operator ==(const rational<ImplicitExponentialNumerator> &r) const{
    return ((num == r.num) && (den == r.den));
}

template<bool ImplicitExponentialNumerator>
inline bool rational<ImplicitExponentialNumerator>::operator !=(const rational<ImplicitExponentialNumerator> &r) const{
    return !(*this == r);
}

template<bool ImplicitExponentialNumerator>
inline bool rational<ImplicitExponentialNumerator>::operator ==(std::int64_t i) const{
    return ((den == std::int64_t(1)) && (num == i));
}

template<bool ImplicitExponentialNumerator>
inline bool rational<ImplicitExponentialNumerator>::test_invariant() const{
    return (this->den > std::int64_t(0)) && (gcd_binary<void>(this->num, this->den) ==
     std::int64_t(1));
}

template<bool ImplicitExponentialNumerator>
void rational<ImplicitExponentialNumerator>::normalize(){
    std::int64_t zero(0);
    if(den == zero)
        throw bad_rational();
    if(num == zero){
        den = std::int64_t(1);
        return;
    }
    std::int64_t g = gcd_binary<void>(num, den);
    num /= g;
    den /= g;
    if(den < zero){
        num = -num;
        den = -den;
    }
}

template<bool ImplicitExponentialNumerator>
std::ostream& operator<< (std::ostream& os, const rational<ImplicitExponentialNumerator> &r){
    os << r.numerator() << '/' << r.denominator();
    return os;
}

} // namespace rational_impl

typedef rational_impl::rational<true>  rational;
typedef rational_impl::rational<false> explicit_exponential_rational;

template<class T, bool Cond>
inline T rational_cast(const rational_impl::rational<Cond> &src){
    return static_cast<T>(src.numerator()) / static_cast<T>(src.denominator());
}

template<bool Cond>
inline rational_impl::rational<Cond> abs(const rational_impl::rational<Cond> &r){
    if(r.numerator() >= std::int64_t(0)){
        return r;
    }
    return rational_impl::rational<Cond>(-r.numerator(), r.denominator());
}

#endif // SCALC_ALGEBRAIC_HPP
