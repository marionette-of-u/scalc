#include <cstdint>
#include "rational.hpp"

namespace algebraic_impl{
// 無理数を含む代数的数
class algebraic{
public:
    algebraic();

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

    // 単体コピー
    static algebraic *copy_mono(const algebraic *p);

    // コピー
    static algebraic *copy(const algebraic *p);

    // 定数を生成
    static algebraic *constant(const rational &n);

    // 式を比較
    static int compare(const algebraic *lhs, const algebraic *rhs, bool mono = false);

    // 新たな要素を生成する
    static algebraic *new_node();

    // 特定の項を削除する
    static void dispose_node(algebraic *p);

    // 全ての項を削除する
    static void dispose(algebraic *p);

    // 存在判定
    static bool is_exist(const algebraic *p);

    // 値
    rational value;

    // n^(e)/d * c->n^(c->e)/c->d * c->c->n^(c->c->e)/c->c->d * c->c->c->n^(c->c->c->e)/c->c->c->d * ...
    // +
    // next (recursive structure...)
    // +
    // :
    // :

    // i-class linked list.
    algebraic *next;

    // ii-class linked list.
    algebraic *e;

    // iii-class linked list.
    algebraic *c;

private:
    inline algebraic(const algebraic&){}
};

std::ostream& operator<< (std::ostream& os, const algebraic &a);

} // algebraic_impl
