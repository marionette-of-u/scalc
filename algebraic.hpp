#ifndef SCALC_ALGEBRAIC_HPP
#define SCALC_ALGEBRAIC_HPP

#include <iostream>              // for std::istream and std::ostream
#include <ios>                   // for std::noskipws
#include <stdexcept>             // for std::domain_error
#include <string>                // for std::string implicit constructor
#include <cstdlib>               // for std::abs
#include <limits>                // for std::numeric_limits

namespace rational_impl{
//----------------
// boost.rational

class bad_rational : public std::domain_error{
public:
    explicit bad_rational() : std::domain_error("bad rational: zero denominator") {}
};

template<class Signature>
unsigned gcd_binary(unsigned u, unsigned v){
if(u && v){
    unsigned shifts = 0;
    while(!(u & 1u) && !(v & 1u)){
        ++shifts;
        u >>= 1;
        v >>= 1;
    }
    unsigned r[] = { u, v };
    unsigned which = static_cast<bool>(u & 1u);
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
class rational{
private:
    struct helper{ int parts[2]; };
    typedef int (helper::* bool_type)[2];

public:
    rational() : num(0), den(1){}
    rational(int n) : num(n), den(1){}
    rational(int n, int d) : num(n), den(d){}
    rational &operator =(int n){ return assign(n, 1); }
    rational &assign(int n, int d);
    int numerator () const{ return num; }
    int denominator () const{ return den; }
    rational &operator +=(const rational &r);
    rational &operator -=(const rational &r);
    rational &operator *=(const rational &r);
    rational &operator /=(const rational &r);
    rational &operator +=(int i);
    rational &operator -=(int i);
    rational &operator *=(int i);
    rational &operator /=(int i);
    const rational &operator ++();
    const rational &operator --();
    bool operator !() const{ return !num; }
    bool operator <(const rational &r) const;
    bool operator ==(const rational &r) const;
    bool operator <(int i) const;
    bool operator >(int i) const;
    bool operator ==(int i) const;

private:
    int num, den;
    bool test_invariant() const;
    void normalize();
};

template<class Signature>
inline rational<Signature> &rational<Signature>::assign(int n, int d){
    num = n;
    den = d;
    normalize();
    return *this;
}

template<class Signature>
inline rational<Signature> operator +(const rational<Signature> &r){
    return r;
}

template<class Signature>
inline rational<Signature> operator -(const rational<Signature> &r){
    return rational<Signature>(-r.numerator(), r.denominator());
}

template<class Signature>
rational<Signature> &rational<Signature>::operator+= (const rational<Signature> &r){
    int r_num = r.num;
    int r_den = r.den;
    int g = gcd_binary<void>(den, r_den);
    den /= g;
    num = num * (r_den / g) + r_num * den;
    g = gcd_binary<void>(num, g);
    num /= g;
    den *= r_den/g;
    return *this;
}

template<class Signature>
rational<Signature> &rational<Signature>::operator-= (const rational<Signature> &r){
    int r_num = r.num;
    int r_den = r.den;
    int g = gcd_binary<void>(den, r_den);
    den /= g;
    num = num * (r_den / g) - r_num * den;
    g = gcd_binary<void>(num, g);
    num /= g;
    den *= r_den/g;
    return *this;
}

template<class Signature>
rational<Signature> &rational<Signature>::operator*= (const rational<Signature> &r){
    int r_num = r.num;
    int r_den = r.den;
    int gcd1 = gcd_binary<void>(num, r_den);
    int gcd2 = gcd_binary<void>(r_num, den);
    num = (num/gcd1) * (r_num/gcd2);
    den = (den/gcd2) * (r_den/gcd1);
    return *this;
}

template<class Signature>
rational<Signature> &rational<Signature>::operator/= (const rational<Signature> &r){
    int r_num = r.num;
    int r_den = r.den;
    int zero(0);
    if(r_num == zero)
        throw bad_rational();
    if(num == zero)
        return *this;
    int gcd1 = gcd_binary<void>(num, r_num);
    int gcd2 = gcd_binary<void>(r_den, den);
    num = (num/gcd1) * (r_den/gcd2);
    den = (den/gcd2) * (r_num/gcd1);
    if(den < zero){
        num = -num;
        den = -den;
    }
    return *this;
}

template<class Signature>
inline rational<Signature>&
rational<Signature>::operator+= (int i){
    return operator+= (rational<Signature>(i));
}

template<class Signature>
inline rational<Signature>& rational<Signature>::operator -=(int i){
    return operator-= (rational<Signature>(i));
}

template<class Signature>
inline rational<Signature>& rational<Signature>::operator*= (int i){
    return operator*= (rational<Signature>(i));
}

template<class Signature>
inline rational<Signature>& rational<Signature>::operator/= (int i){
    return operator/= (rational<Signature>(i));
}

template<class Signature>
inline const rational<Signature> &rational<Signature>::operator ++(){
    num += den;
    return *this;
}

template<class Signature>
inline const rational<Signature> &rational<Signature>::operator --(){
    num -= den;
    return *this;
}

template<class Signature>
bool rational<Signature>::operator <(const rational<Signature> &r) const{
    int const  zero(0);
    struct { int  n, d, q, r; }  ts = { this->num, this->den, this->num /
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
        return false;
    }else{
        return (ts.r != zero) != static_cast<bool>(reverse);
    }
}

template<class Signature>
bool rational<Signature>::operator <(int i) const{
    int const  zero(0);
    int  q = this->num / this->den, r = this->num % this->den;
    while(r < zero){ r += this->den; --q; }
    return q < i;
}

template<class Signature>
bool rational<Signature>::operator> (int i) const{
    if(num == i && den == int(1))
        return false;
    return !operator <(i);
}

template<class Signature>
inline bool rational<Signature>::operator ==(const rational<Signature> &r) const{
    return ((num == r.num) && (den == r.den));
}

template<class Signature>
inline bool rational<Signature>::operator ==(int i) const{
    return ((den == int(1)) && (num == i));
}

template<class Signature>
inline bool rational<Signature>::test_invariant() const{
    return (this->den > int(0)) && (gcd_binary<void>(this->num, this->den) ==
     int(1));
}

template<class Signature>
void rational<Signature>::normalize(){
    int zero(0);
    if(den == zero)
        throw bad_rational();
    if(num == zero){
        den = int(1);
        return;
    }
    int g = gcd_binary<void>(num, den);
    num /= g;
    den /= g;
    if(den < zero){
        num = -num;
        den = -den;
    }
}

template<class Signature>
std::ostream& operator<< (std::ostream& os, const rational<Signature> &r){
    os << r.numerator() << '/' << r.denominator();
    return os;
}
}

typedef rational_impl::rational<void> rational;

template<class T>
inline T rational_cast(const rational &src){
    return static_cast<T>(src.numerator()) / static_cast<T>(src.denominator());
}

inline rational abs(const rational &r){
    if(r.numerator() >= int(0)){
        return r;
    }
    return rational(-r.numerator(), r.denominator());
}

#endif // SCALC_ALGEBRAIC_HPP
