#include <map>
#include <cstdint>
#include "rational.hpp"

namespace algebraic_impl{
// 無理数を含む代数的数
class algebraic{
public:
    algebraic();

    // 加算
    // qは破棄する
    static void add(algebraic *p, algebraic *q);

    // 符号反転
    static void change_sign(algebraic *p);

    // コピー
    static algebraic *copy(const algebraic *p);

    // 定数を生成
    static algebraic *constant(const rational &n);

    // 比較
    static int compare(const algebraic *lhs, const algebraic *rhs);

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

    // 指数部
    // 第2級連結
    algebraic *e;

    // 係数部
    // 第2級連結
    algebraic *c;

    // 次の項
    // 第1級連結
    algebraic *next;

private:
    inline algebraic(const algebraic&){}
};

} // algebraic_impl
