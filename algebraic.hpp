#include "rational.hpp"

namespace algebraic_impl{
;
struct term{
    term();
    term(int n);
    term(const rational &n);

    // 加算
    // qは破棄する
    static void add(term *p, term *q);

    // 符号反転
    static void change_sign(term *p);

    // コピー
    static term *copy(const term *p);

    // 定数を生成
    static term *constant(rational n);

    // 比較
    static int sequential_compare(const term *lhs, const term *rhs);

    // 特定の項を削除する
    static void dispose_node(term *p);

    // 全ての項を削除する
    static void dispose(term *p);

    rational value;
    term *e, *next;
};

}
