#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <vector>
#include <iterator>
#include <sstream>
#include <string>
#include <cstring>
#include "common.hpp"
#include "parser.hpp"

// 解析対象の文字列が入るvector.
typedef std::vector<char> statement_str;

// ---- lex data.
namespace lex_data{
    // 字句解析結果のrange
    typedef std::pair<statement_str::const_iterator, statement_str::const_iterator> token_range;

    // 字句解析結果のtoken種別と範囲
    typedef std::pair<lexer::token, token_range> lex_result;

    // 字句解析結果
    typedef std::vector<lex_result> token_sequence;
}


// ---- ast element type.

struct eval_target{
    virtual ~eval_target(){}

    virtual std::string ast_str() const = 0;
};

struct value : eval_target{
    virtual std::string ast_str() const{
        std::stringstream ss;
        ss << v;
        return ss.str();
    }

    fpoint v;
};

struct symbol : eval_target{
    virtual std::string ast_str() const{
        return s;
    }

    std::string s;
};

struct binary_operator : eval_target{
    virtual std::string ast_str() const{
        std::string str;
        str += "(";
        str += op_s + " " + lhs->ast_str() + rhs->ast_str();
        str += ")";
        return str;
    }

    std::string op_s;
    std::function<value(const eval_target*, const eval_target*)> op_fn;
    std::unique_ptr<eval_target> lhs, rhs;
};

struct sequence{
    // 評価対象の式
    // もしこのsequenceが先頭にあれば, eは必ず記号を指す
    std::unique_ptr<eval_target> e;

    // リンクリスト 次の評価対象の式
    std::unique_ptr<sequence> next;
};

struct lambda : eval_target{
    virtual std::string ast_str() const{
        std::string str;
        str += "(lambda";
        for(const sequence *ptr = args.get(); ptr; ptr = ptr->next.get()){
            str += " " + ptr->e->ast_str();
        }
        str += " -> " + e->ast_str() + ")";
    }

    // lambda式の引数
    std::unique_ptr<sequence> args;

    // lambda式の本体
    std::unique_ptr<eval_target> e;
};

struct call : eval_target{
    virtual std::string ast_str() const{
        std::string str;
        str += "(call";
        for(const sequence *ptr = fn_and_args.get(); ptr; ptr = ptr->next.get()){
            str += " " + ptr->e->ast_str();
        }
        str += ")";
    }

    // 関数及び関数の引数
    // 先頭は必ず関数を指す
    std::unique_ptr<sequence> fn_and_args;
};

struct equality{
    // 左辺 記号
    std::unique_ptr<symbol> s;

    // 右辺 式
    std::unique_ptr<eval_target> e;
};

struct equality_sequence{
    // 等式
    std::unique_ptr<equality> e;

    // リンクリスト 次の等式
    std::unique_ptr<equality_sequence> next;
};

struct statement{
    // 評価対象の式
    std::unique_ptr<eval_target> e;

    // where部
    std::unique_ptr<equality_sequence> w;
};


// ---- semantic action.

#include <exception>

class error : public std::runtime_error{
public:
    error(std::string message) throw() : std::runtime_error(message){} 

    error(const error &other) throw(): runtime_error(other){}

    error &operator =(const error &other) throw(){
        std::runtime_error::operator =(other);
        return *this;
    }

    virtual ~error() throw(){}
};

class semantic_action{
public:
    void syntax_error(){
        throw(error("syntax error."));
    }

    void stack_overflow(){
        throw(error("stack overflow"));
    }

    template<class T>
    static void downcast(T *&x, eval_target *y){
        x = static_cast<T*>(y);
    }

    template<class T>
    static void upcast(eval_target *&x, T *y){
        x = y;
    }

    template<class T>
    T *identity(T *subtree){
        return subtree;
    }
};

int main(){
    int argc = 2;
    char *argv[] = {
        "dummy.exe",
        "1 + 1^a"
    };

    if(argc != 2){ return 0; }
    statement_str target_str;
    lex_data::token_sequence token_sequence;
    std::size_t n = std::strlen(argv[1]);
    target_str.resize(n);
    for(std::size_t i = 0; i < n; ++i){
        target_str[i] = argv[1][i];
    }

    lexer::lexer::tokenize(
        target_str.begin(),
        target_str.end(),
        std::insert_iterator<lex_data::token_sequence>(
            token_sequence,
            token_sequence.begin()
        )
    );

    return 0;
}
