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

namespace lex_data{
    // 字句解析結果のrange
    typedef std::pair<statement_str::const_iterator, statement_str::const_iterator> token_range;

    // 字句解析結果のtoken種別と範囲
    typedef std::pair<lexer::token, token_range> lex_result;

    // 字句解析結果
    typedef std::vector<lex_result> token_sequence;
}

namespace analyzer{
    struct eval_target;
    struct symbol;

    struct stack_element{
        stack_element() : node(nullptr), v(nullptr){}
        poly::node *node;
        const eval_target *v;
    };

    class semantic_data{
    public:
        // スタックに計算結果を積む
        void push_stack(poly::node *ptr);

        // スタックに評価前のを積む
        void push_stack(const eval_target *ptr);

        // スタックから計算結果を取り出す
        stack_element pop_stack();

        bool empty() const{
            return stack.empty();
        }

        // 記号が変数かどうかを問い合わせる
        // 変数であれば値を返し, そうでなければ入力記号を返す
        const stack_element inquiry_symbol(const symbol *s);

        // ローカル引数の領域を新たに生成する
        std::map<str_wrapper, const stack_element> &push_local_args(){
            local_args.push_back(std::map<str_wrapper, const stack_element>());
            return local_args.back();
        }

        // ローカル引数の最も新しい領域を破棄する
        void pop_local_args(){
            local_args.pop_back();
        }

        // ローカル引数に仮引数を登録する
        void register_local_arg(const symbol *ptr, const stack_element target);

        // 定数を登録する
        void register_let_value(const symbol *ptr, const stack_element target);

    private:
        // 計算の中途結果が入るstack
        std::vector<stack_element> stack;

        // local args
        std::vector<std::map<str_wrapper, const stack_element>> local_args;

        // global variable
        std::map<str_wrapper, stack_element> global_variable_map;
    };

    void semantic_data::push_stack(poly::node *ptr){
        stack_element a;
        a.node = ptr;
        stack.push_back(a);
    }

    void semantic_data::push_stack(const eval_target *ptr){
        stack_element a;
        a.v = ptr;
        stack.push_back(a);
    }

    struct eval_target{
        virtual ~eval_target(){}
        virtual std::string ast_str() const = 0;
        virtual void eval(semantic_data&) const{ throw(error("missing eval function.")); }
        virtual void call(semantic_data&) const{ throw(error("missing call function.")); }
    };

    struct value : eval_target{
        virtual std::string ast_str() const{
            std::stringstream ss;
            ss << v;
            if(!real){ ss << "i"; }
            return ss.str();
        }

        virtual void eval(semantic_data &sd) const{
            poly::node *ptr = poly::constant(1, 1);
            if(real){
                ptr->next->real = v;
                ptr->next->imag = 0;
            }else{
                ptr->next->real = 0;
                ptr->next->imag = v;
            }
            sd.push_stack(ptr);
        }

        fpoint v;
        bool real;
    };

    struct symbol : eval_target{
        virtual std::string ast_str() const{
            return *s.ptr;
        }

        virtual void eval(semantic_data &sd) const{
            stack_element se = sd.inquiry_symbol(this);
            if(se.v){
                sd.push_stack(se.v);
            }else{
                sd.push_stack(se.node);
            }
        }

        str_wrapper s;
    };

    void semantic_data::register_local_arg(const symbol *ptr, const stack_element target){
        local_args.back().insert(std::make_pair(ptr->s, target));
    }

    void semantic_data::register_let_value(const symbol *ptr, const stack_element target){
        global_variable_map[ptr->s] = target;
    }

    stack_element semantic_data::pop_stack(){
        if(stack.empty()){
            throw(error("stack is empty."));
        }
        stack_element a = stack.back();
        stack.pop_back();
        return a;
    }

    const stack_element semantic_data::inquiry_symbol(const symbol *s){
        for(auto iter = local_args.rbegin(); iter != local_args.rend(); ++iter){
            auto jter = iter->find(s->s);
            if(jter != iter->end()){ return jter->second; }
        }
        auto iter = global_variable_map.find(s->s);
        if(iter != global_variable_map.end()){ return iter->second; }
        stack_element se;
        se.v = s;
        return se;
    }

    struct binary_operator : eval_target{
        binary_operator() : lhs(nullptr), rhs(nullptr){}

        virtual std::string ast_str() const{
            std::string str;
            str += "(";
            str += *op_s.ptr + " " + lhs->ast_str() + " " + rhs->ast_str();
            str += ")";
            return str;
        }

        virtual void eval(semantic_data &sd) const{
            lhs->eval(sd);
            rhs->eval(sd);
            stack_element er = sd.pop_stack(), el = sd.pop_stack();
            if(!er.node || !el.node){
                throw(error("stack element is lambda expression, in binary operator."));
            }
            poly::node *l = el.node, *r = er.node;
            if(op_s == "+"){
                poly::add(r, l);
                sd.push_stack(r);
            }else if(op_s == "-"){
                poly::sub(r, l);
                sd.push_stack(r);
            }else if(op_s == "*"){
                sd.push_stack(poly::multiply(r, l));
                poly::dispose(l);
                poly::dispose(r);
            }else if(op_s == "/" || op_s == "//"){
                sd.push_stack(poly::divide(l, r, nullptr));
                poly::dispose(l);
                poly::dispose(r);
            }else if(op_s == "^"){
                sd.push_stack(poly::power(l, r));
                poly::dispose(l);
                poly::dispose(r);
            }
        }

        str_wrapper op_s;
        std::unique_ptr<eval_target> lhs, rhs;
    };

    struct negate_expr : eval_target{
        negate_expr() : operand(nullptr){}

        virtual std::string ast_str() const{
            return "-" + operand->ast_str();
        }

        virtual void eval(semantic_data &sd) const{
            operand->eval(sd);
            stack_element a = sd.pop_stack();
            if(!a.node){
                throw(error("operand is lambda expression, in negative operator."));
            }
            a.node->negate();
            sd.push_stack(a.node);
        }

        std::unique_ptr<eval_target> operand;
    };

    struct sequence : eval_target{
        sequence() : e(nullptr), next(nullptr), head(nullptr){}

        virtual std::string ast_str() const{
            std::string str;
            str += "(seq";
            for(const sequence *ptr = head; ptr; ptr = ptr->next.get()){
                str += " " + ptr->e->ast_str();
            }
            str += ")";
            return str;
        }

        virtual void eval(semantic_data &sd) const{
            if(head == this){
                e->eval(sd);
                return;
            }
            for(const sequence *ptr = head->next.get(); ptr; ptr = ptr->next.get()){
                ptr->e->eval(sd);
            }
            sd.push_stack(head->e.get());
        }

        // 評価対象の式
        std::unique_ptr<eval_target> e;

        // リンクリスト 次の評価対象の式
        std::unique_ptr<sequence> next;

        // 先頭
        sequence *head;
    };

    struct lambda : sequence{
        lambda() : args(nullptr), name(to_string(lambda_counter()) + "_lambda"){}

        virtual std::string ast_str() const{
            std::string str;
            str += *name.ptr;
            for(const sequence *ptr = args->head; ptr; ptr = ptr->next.get()){
                str += " " + ptr->e->ast_str();
            }
            str += " -> " + e->ast_str();
            return str;
        }

        virtual void eval(semantic_data &sd) const{
            sd.push_stack(this);
        }

        // lambda式の引数
        std::unique_ptr<sequence> args;

        // lambda式の名前
        str_wrapper name;

    private:
        static std::size_t lambda_counter(){
            static std::size_t i = 0;
            return i++;
        }
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

        virtual void eval(semantic_data &sd) const{
            stack_element se;
            se.v = e.get();
            sd.register_let_value(s.get(), se);
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

        virtual void eval(semantic_data &sd) const{
            for(const equality_sequence *ptr = head; ptr; ptr = ptr->next.get()){
                ptr->e->eval(sd);
            }
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

        virtual void eval(semantic_data &sd) const{
            if(w){ w->eval(sd); }
            e->eval(sd);
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


    // ---- semantic action
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

        sequence *make_seq(eval_target *e){
            sequence *ptr = new sequence;
            ptr->e.reset(e);
            ptr->head = ptr;
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
}

int main(){
    {
        int argc = 2;
        char *argv[] = {
            "dummy.exe",
            "(a b c d -> a b c d) (x y z -> x + y + z) 1 2 ((p -> 3 * p) 4)"
            //"(a b c d -> a b c d) (x y z -> x + y + z) 1 2 ((a -> 2 * a) 4) where p = 1, q = 2, r = (a b c -> a + b + c)"
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

        using namespace analyzer;

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
                        b->op_s = std::string(iter->second.first, iter->second.second);
                        target_ptr = b;
                    }
                    break;

                case parser::token_identifier:
                    {
                        value *v = new value;
                        std::stringstream ss;
                        std::string str(iter->second.first, iter->second.second);
                        ss << std::string(str);
                        ss >> v->v;
                        v->real = str.back() != 'i';
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
        // std::cout << root->ast_str() << std::endl;
        semantic_data sd;
        root->eval(sd);
    }

    //{
    //    using namespace poly;
    //    node *n = new_node(), *m = new_node();
    //    add(n, variable("x", variable("y", variable("z"))));
    //    node *o = power(n, variable("x", variable("y", variable("w"))));
    //    std::cout << poly_to_string(o) << std::endl;
    //}

    //{
    //    using namespace poly;
    //    node *n = new_node(), *m = new_node();
    //    add(n, variable("x", variable("y", variable("z"))));
    //    node *o = power(n, constant(0, 1));
    //    std::cout << poly_to_string(o) << std::endl;
    //}
    //std::cout << "----" << std::endl;

    //{
    //    using namespace poly;
    //    node *l = constant(2.2, 3.3);
    //    add(l, variable("x"));
    //    l = multiply(l, variable("y"));
    //    l = power(l, constant(3));

    //    std::cout << poly_to_string(l) << std::endl;
    //}
    //std::cout << "----" << std::endl;

    //{
    //    using namespace poly;
    //    node *l = variable("x", 2);
    //    l = multiply(l, variable("y", 3));
    //    l = multiply(l, variable("z", 4));
    //    l = multiply(l, constant(1.5, 0.5));
    //    add(l, variable("x", 2));

    //    node *r = variable("x");
    //    r = multiply(r, variable("y"));
    //    add(r, constant(2, 3));
    //    add(r, variable("y", 3));

    //    node *rem = new_node();
    //    node *q = divide(l, r, rem);
    //    std::cout << poly_to_string(l) << std::endl;
    //    std::cout << poly_to_string(r) << std::endl;
    //    std::cout << poly_to_string(q) << std::endl;
    //    std::cout << poly_to_string(rem) << std::endl;
    //}
    //std::cout << "----" << std::endl;

    //{
    //    using namespace poly;
    //    node *l = variable("x", 2);
    //    l = multiply(l, variable("y", 3));
    //    l = multiply(l, variable("z", 4));
    //    l = multiply(l, constant(1.5, 0.5));
    //    add(l, variable("x", 2));

    //    node *r = copy(l);

    //    add(l, copy(l));
    //    add(l, copy(l));
    //    add(l, copy(l));
    //    add(l, copy(l));
    //    add(l, copy(l));

    //    node *rem = new_node();
    //    node *q = divide(r, l, rem);
    //    std::cout << poly_to_string(r) << std::endl;
    //    std::cout << poly_to_string(l) << std::endl;
    //    std::cout << poly_to_string(q) << std::endl;
    //    std::cout << poly_to_string(rem) << std::endl;
    //}

    return 0;
}
