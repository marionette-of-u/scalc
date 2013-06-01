#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <vector>
#include <iterator>
#include <sstream>
#include <string>
#include <exception>
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
    binary_operator() : lhs(nullptr), rhs(nullptr){}

    virtual std::string ast_str() const{
        std::string str;
        str += "(";
        str += op_s + " " + lhs->ast_str() + " " + rhs->ast_str();
        str += ")";
        return str;
    }

    std::string op_s;
    std::function<value(const eval_target*, const eval_target*)> op_fn;
    std::unique_ptr<eval_target> lhs, rhs;
};

struct negate_expr : eval_target{
    negate_expr() : operand(nullptr){}

    virtual std::string ast_str() const{
        return "-" + operand->ast_str();
    }

    std::unique_ptr<eval_target> operand;
};

struct sequence : eval_target{
    sequence() : e(nullptr), next(nullptr), head(nullptr){}

    virtual std::string ast_str() const{
        std::string str;
        str += "(";
        for(const sequence *ptr = head; ptr; ptr = ptr->next.get()){
            str += (ptr != head ? " " : "") + ptr->e->ast_str();
        }
        str += ")";
        return str;
    }

    // 評価対象の式
    std::unique_ptr<eval_target> e;

    // リンクリスト 次の評価対象の式
    std::unique_ptr<sequence> next;

    // 先頭
    sequence *head;
};

struct lambda : sequence{
    lambda() : args(nullptr){}

    virtual std::string ast_str() const{
        std::string str;
        str += "(lambda";
        for(const sequence *ptr = args->head; ptr; ptr = ptr->next.get()){
            str += " " + ptr->e->ast_str();
        }
        str += " -> " + e->ast_str() + ")";
        return str;
    }

    // lambda式の引数
    std::unique_ptr<sequence> args;
};

struct equality : eval_target{
    equality() : s(nullptr), e(nullptr){}

    virtual std::string ast_str() const{
        std::string str;
        str += s->ast_str();
        str += " = ";
        str += e->ast_str();
        return str;
    }

    // 左辺 記号
    std::unique_ptr<symbol> s;

    // 右辺 式
    std::unique_ptr<eval_target> e;
};

struct equality_sequence : eval_target{
    equality_sequence() : e(nullptr), next(nullptr), head(nullptr){}

    virtual std::string ast_str() const{
        return "";
    }

    // 等式
    std::unique_ptr<equality> e;

    // リンクリスト 次の等式
    std::unique_ptr<equality_sequence> next;

    // 先頭
    equality_sequence *head;
};

struct statement : eval_target{
    statement() : e(nullptr), w(nullptr){}

    virtual std::string ast_str() const{
        std::string str;
        str += e->ast_str();
        if(w){
            str += " where";
            for(const equality_sequence *ptr = w.get()->head; ptr; ptr = ptr->next.get()){
                str += (ptr != w.get()->head ? ", " : " ") + ptr->e->ast_str();
            }
        }
        return str;
    }

    // 評価対象の式
    std::unique_ptr<eval_target> e;

    // where部
    std::unique_ptr<equality_sequence> w;
};

struct defined_symbol : eval_target{
    defined_symbol() : e(nullptr), s(nullptr){}

    virtual std::string ast_str() const{
        std::string str;
        str += s->ast_str();
        str += " = ";
        str += e->ast_str();
        return str;
    }

    // 束縛対象の式
    std::unique_ptr<eval_target> e;

    // 束縛対象に結び付けられる名前
    std::unique_ptr<symbol> s;
};


// ---- semantic action.

class semantic_action{
public:
    void syntax_error(){
        throw(error("syntax error."));
    }

    void stack_overflow(){
        throw(error("stack overflow."));
    }

    template<class T>
    static void downcast(T *&x, eval_target *y){
        x = static_cast<T*>(y);
    }

    template<class U>
    static void upcast(eval_target *&x, U *y){
        x = y;
    }

    eval_target *make_statement(eval_target *e, equality_sequence *es){
        statement *s = new statement;
        s->e.reset(e);
        s->w.reset(es);
        return s;
    }

    eval_target *define_symbol(symbol *s, eval_target *e){
        defined_symbol *d = new defined_symbol;
        d->e.reset(e);
        d->s.reset(s);
        return d;
    }

    equality_sequence *make_equality_sequence(equality *e){
        equality_sequence *es = new equality_sequence;
        es->e.reset(e);
        es->head = es;
        return es;
    }

    equality_sequence *make_equality_sequence(equality_sequence *es, equality *e){
        equality_sequence *ptr = new equality_sequence;
        ptr->e.reset(e);
        if(es){
            ptr->head = es->head;
            es->next.reset(ptr);
        }else{ ptr->head = ptr; }
        return ptr;
    }

    equality *make_equality(symbol *s, eval_target *e){
        equality *ptr = new equality;
        ptr->s.reset(s);
        ptr->e.reset(e);
        return ptr;
    }

    eval_target *make_binary_op(binary_operator *e, eval_target *lhs, eval_target *rhs){
        e->lhs.reset(lhs);
        e->rhs.reset(rhs);
        return e;
    }

    negate_expr *make_negate_expr(eval_target *e){
        negate_expr *n = new negate_expr;
        n->operand.reset(e);
        return n;
    }

    sequence *make_seq(sequence *s, eval_target *e){
        sequence *ptr = new sequence;
        ptr->e.reset(e);
        if(s){
            ptr->head = s->head;
            s->next.reset(ptr);
        }else{ ptr->head = ptr; }
        return ptr;
    }

    sequence *make_lambda(sequence *s, eval_target *e){
        lambda *l = new lambda;
        l->args.reset(s);
        l->e.reset(e);
        l->head = l;
        return make_seq(nullptr, l);
    }

    template<class T>
    T *identity(T *subtree){
        return subtree;
    }
};

int main(){
    {
        int argc = 2;
        char *argv[] = {
            "dummy.exe",
            "q_fn 512 * -512 * -1024 // -512 (a b -> a + b) c d where c = 10, d = c + 10"
            //"let hogepiyo = q_fn 512 * -512 * -1024 // -512 (a b -> a + b) c d"
        };

        if(argc != 2){ return 0; }
        statement_str target_str;
        lex_data::token_sequence token_sequence;
        std::size_t n = std::strlen(argv[1]);
        target_str.resize(n);
        for(std::size_t i = 0; i < n; ++i){
            target_str[i] = argv[1][i];
        }

        auto lex_result = lexer::lexer::tokenize(
            target_str.begin(),
            target_str.end(),
            std::insert_iterator<lex_data::token_sequence>(
                token_sequence,
                token_sequence.begin()
            )
        );
        if(!lex_result.first){
            std::cout << "lexical error.";
            return 0;
        }

        semantic_action sa;
        eval_target *target_ptr;
        parser::parser<eval_target*, semantic_action> p(sa);
        try{
            for(auto iter = token_sequence.begin(); iter != token_sequence.end(); ++iter){
                parser::token t = static_cast<parser::token>(iter->first);
                switch(t){
                case parser::token_double_slash:
                case parser::token_hat:
                case parser::token_asterisk:
                case parser::token_slash:
                case parser::token_plus:
                case parser::token_minus:
                    {
                        binary_operator *b = new binary_operator;
                        b->op_s.assign(iter->second.first, iter->second.second);
                        target_ptr = b;
                    }
                    break;

                case parser::token_identifier:
                    {
                        value *v = new value;
                        std::stringstream ss;
                        ss << std::string(iter->second.first, iter->second.second);
                        ss >> v->v;
                        target_ptr = v;
                    }
                    break;

                case parser::token_symbol:
                    {
                        symbol *s = new symbol;
                        s->s = std::string(iter->second.first, iter->second.second);
                        target_ptr = s;
                    }
                    break;

                default:
                    target_ptr = nullptr;
                }
        
                if(p.post(t, target_ptr)){ break; }
            }
            p.post(parser::token_0, target_ptr);
        }catch(std::runtime_error &e){
            std::cout << "parsing error: " << e.what();
        }
    
        eval_target *root = nullptr;
        if(!p.accept(root)){
            std::cout << "parsing error.";
            return 0;
        }
        std::string r = root->ast_str();
    }

    {
        using namespace poly;
        node *x;
        {
            node *a = variable("x", 3);
            {
                node *b = variable("x", 2), *c = variable("x", 1);
                add(b, c);
                add(a, b);
            }
            {
                node *b = variable("y", 3), *c = variable("y", 1);
                add(b, c);
                add(a, b);
            }
            x = variable("a", a);
        }
        node *x_;
        {
            node *a = variable("x", 3);
            {
                node *b = variable("y", 3), *c = variable("y", 2);
                add(b, c);
                add(a, b);
            }
            {
                node *b = variable("x", 2), *c = variable("x", 1);
                add(b, c);
                add(a, b);
            }
            x_ = variable("a", a);
        }
        int r = lexicographic_compare(x, x_);
        std::cout << r << std::endl;
        dispose(x);
        dispose(x_);
    }

    return 0;
}
