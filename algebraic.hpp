#include <map>
#include <cstdint>
#include "rational.hpp"

namespace algebraic_impl{
// 無理数を含む代数的数
class algebraic{
public:
    algebraic();

    algebraic(algebraic &&other);

    // テスト
    static void test();

    // 乗算
    // 新たな多項式を返す
    static algebraic *multiply(const algebraic *x, const algebraic *y);

    // 減算
    // qは破棄する
    static void sub(algebraic *p, algebraic *q);

    // 加算
    // qは破棄する
    static void add(algebraic *p, algebraic *q);

    // 符号反転
    static void change_sign(algebraic *p);

    // コピー
    static algebraic *copy(const algebraic *p);

    // 定数を生成
    // n.num^e / n.den
    static algebraic *constant(explicit_exponential_rational n, const rational &e = 0);

    // 式を比較
    static int compare(const algebraic *lhs, const algebraic *rhs);

    // 項を比較
    static int compare_term(const algebraic *lhs, const algebraic *rhs);

    // 新たな要素を生成する
    static algebraic *new_node();

    // 特定の項を削除する
    static void dispose_node(algebraic *p);

    // 全ての項を削除する
    static void dispose(algebraic *p);

    // value.n^key / value.d
    typedef std::map<rational, explicit_exponential_rational> value_type;
    value_type value;

    // 次の項
    algebraic *next;

private:
    inline algebraic(const algebraic&){}
};

std::ostream& operator<< (std::ostream& os, const algebraic &a);

} // algebraic_impl
