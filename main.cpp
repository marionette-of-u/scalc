#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <vector>
#include <iterator>
#include <string>
#include <cstring>
#include "parser.hpp"

// ��͑Ώۂ̕����񂪓���vector
typedef std::vector<char> statement_str;

// ---- lex data.
namespace lex_data{
    // �����͌��ʂ�range
    typedef std::pair<statement_str::const_iterator, statement_str::const_iterator> token_range;

    // �����͌��ʂ�token��ʂƔ͈�
    typedef std::pair<lexer::token, token_range> lex_result;

    // �����͌���
    typedef std::vector<lex_result> token_sequence;
}


// ---- ast element type.

struct eval_target{
    virtual ~eval_target(){}

    virtual std::string ast_str() const{
        return "eval_target";
    }
};

struct value : eval_target{
    virtual std::string ast_str() const{
        return "value";
    }
};

struct binary_operator{
    std::string symbol;
    std::function<value(const eval_target*, const eval_target*)> op_fn;
};

struct expr : eval_target{
    virtual std::string ast_str() const{
        std::string str;
        str += "(";
        str += op->symbol + " " + lhs->ast_str() + rhs->ast_str();
        str += ")";
        return str;
    }

    std::unique_ptr<binary_operator> op;
    std::unique_ptr<eval_target> lhs, rhs;
};

struct symbol : eval_target{
    virtual std::string ast_str() const{
        return "symbol";
    }
};

struct sequence{
    // �]���Ώۂ̎�
    // ��������sequence���擪�ɂ����, e�͕K���L��
    std::unique_ptr<eval_target> e;

    // �����N���X�g ���̕]���Ώۂ̎�
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

    // lambda���̈���
    std::unique_ptr<sequence> args;

    // lambda���̖{��
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

    // �֐��y�ъ֐��̈���
    // �擪�͕K���֐����w��
    std::unique_ptr<sequence> fn_and_args;
};

struct equality{
    // ���� �L��
    std::unique_ptr<symbol> s;

    // �E�� ��
    std::unique_ptr<expr> e;
};

struct equality_sequence{
    // ����
    std::unique_ptr<equality> e;

    // �����N���X�g ���̓���
    std::unique_ptr<equality_sequence> next;
};

struct statement{
    // �]���Ώۂ̎�
    std::unique_ptr<expr> e;

    // where��
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

private:
};

int main(){
    int argc = 2;
    char *argv[] = {
        "dummy.exe",
        "1 + 1"
    };

    if(argc != 2){ return 0; }
    statement_str target_str;
    lex_data::token_sequence token_sequence;
    std::size_t n = std::strlen(argv[1]);
    target_str.resize(n);
    for(std::size_t i = 0; i < n; ++i){
        target_str[i] = argv[1][i];
    }
    lexer::lexer::tokenize(target_str.begin(), target_str.end(), std::insert_iterator<lex_data::token_sequence>(token_sequence, token_sequence.begin()));

    return 0;
}