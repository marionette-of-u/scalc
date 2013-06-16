#include <map>
#include <cstdint>
#include "rational.hpp"

namespace algebraic_impl{
// 無理数を含む代数的数
class algebraic{
public:
    algebraic();

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

    // value^(e) * c->value^(c->e) * c->c->value^(c->c->e) * c->c->c->value^(c->c->c->e)
    // +
    // next (recursive structure...)
    // +
    // :
    // :

    // ii-class linked list.
    // this->value^(other_algebraic)
    algebraic *e;

    // iii-class linked list.
    // 
    algebraic *c;

    // i-class linked list.
    // this->value + this->next->value + this->next->next->value + ...
    algebraic *next;

private:
    inline algebraic(const algebraic&){}
};

std::ostream& operator<< (std::ostream& os, const algebraic &a);

} // algebraic_impl
