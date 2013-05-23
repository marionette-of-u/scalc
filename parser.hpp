#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <cstdlib>
#include <cassert>
#include <vector>
#include "lexer.hpp"

namespace grammar{

enum token{
    token_unary_minus,
    token_double_slash = lexer::token_double_slash,
    token_hat = lexer::token_hat,
    token_asterisk = lexer::token_asterisk,
    token_slash = lexer::token_slash,
    token_plus = lexer::token_plus,
    token_minus = lexer::token_minus,
    token_left_paren = lexer::token_left_paren,
    token_right_paren = lexer::token_right_paren,
    token_right_arrow = lexer::token_right_arrow,
    token_equal = lexer::token_equal,
    token_comma = lexer::token_comma,
    token_identifier = lexer::token_identifier,
    token_symbol = lexer::token_symbol,
    token_keyword_where = lexer::token_keyword_where,
    token_keyword_let = lexer::token_keyword_let,
    token_0
};

template<class T, int StackSize>
class stack{
public:
    stack(){ gap_ = 0; }
    ~stack(){}
    
    void reset_tmp()
    {
        gap_ = stack_.size();
        tmp_.clear();
    }

    void commit_tmp()
    {
        // may throw
        stack_.reserve(gap_ + tmp_.size());

        // expect not to throw
        stack_.erase(stack_.begin() + gap_, stack_.end());
        stack_.insert(stack_.end(), tmp_.begin(), tmp_.end());
    }

    bool push(const T& f)
    {
        if(StackSize != 0 && StackSize <= stack_.size() + tmp_.size()){
            return false;
        }
        tmp_.push_back(f);
        return true;
    }

    void pop(std::size_t n)
    {
        if(tmp_.size() < n){
            n -= tmp_.size();
            tmp_.clear();
            gap_ -= n;
        }else{
            tmp_.erase(tmp_.end() - n, tmp_.end());
        }
    }

    const T& top()
    {
        if(!tmp_.empty()){
            return tmp_.back();
        } else {
            return stack_[gap_ - 1];
        }
    }

    const T& get_arg(std::size_t base, std::size_t index)
    {
        std::size_t n = tmp_.size();
        if(base - index <= n){
            return tmp_[n - (base - index)];
        }else{
            return stack_[gap_ - (base - n) + index];
        }
    }

    void clear()
    {
        stack_.clear();
    }

private:
    std::vector<T> stack_;
    std::vector<T> tmp_;
    std::size_t gap_;

};

template<class Value, class SemanticAction, int StackSize = 0>
class parser {
public:
    typedef token token_type;
    typedef Value value_type;

public:
    parser(SemanticAction& sa) : sa_(sa){ reset(); }

    void reset()
    {
        error_ = false;
        accepted_ = false;
        clear_stack();
        reset_tmp_stack();
        if(push_stack(&parser::state_0, &parser::gotof_0, value_type())){
            commit_tmp_stack();
        }else{
            sa_.stack_overflow();
            error_ = true;
        }
    }

    bool post(token_type token, const value_type& value)
    {
        assert(!error_);
        reset_tmp_stack();
        while((this->*(stack_top()->state))(token, value)); // may throw
        if(!error_){
            commit_tmp_stack();
        }
        return accepted_ || error_;
    }

    bool accept(value_type& v)
    {
        assert(accepted_);
        if(error_){ return false; }
        v = accepted_value_;
        return true;
    }

    bool error(){ return error_; }

private:
    typedef parser<Value, SemanticAction, StackSize> self_type;
    typedef bool (self_type::*state_type)(token_type, const value_type&);
    typedef bool ( self_type::*gotof_type )(int, const value_type&);

    bool            accepted_;
    bool            error_;
    value_type      accepted_value_;
    SemanticAction& sa_;

    struct stack_frame{
        state_type state;
        gotof_type gotof;
        value_type value;

        stack_frame(state_type s, gotof_type g, const value_type& v)
            : state(s), gotof(g), value(v){}
    };

    stack<stack_frame, StackSize> stack_;
    bool push_stack(state_type s, gotof_type g, const value_type& v)
    {
        bool f = stack_.push(stack_frame(s, g, v));
        assert(!error_);
        if(!f){
            error_ = true;
            sa_.stack_overflow();
        }
        return f;
    }

    void pop_stack(std::size_t n)
    {
        stack_.pop(n);
    }

    const stack_frame* stack_top()
    {
        return &stack_.top();
    }

    const value_type& get_arg(std::size_t base, std::size_t index)
    {
        return stack_.get_arg(base, index).value;
    }

    void clear_stack()
    {
        stack_.clear();
    }

    void reset_tmp_stack()
    {
        stack_.reset_tmp();
    }

    void commit_tmp_stack()
    {
        stack_.commit_tmp();
    }

    bool call_0_call_func(int nonterminal_index, int base, int arg_index0)
    {
        sequence* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* r = sa_.call_func(arg0);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_seq(int nonterminal_index, int base, int arg_index0)
    {
        symbol* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        sequence* r = sa_.make_seq(arg0);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_1_make_seq(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        sequence* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        sequence* r = sa_.make_seq(arg0, arg1);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_equality(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        symbol* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        equality* r = sa_.make_equality(arg0, arg1);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_equality_sequence(int nonterminal_index, int base, int arg_index0)
    {
        equality* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        equality_sequence* r = sa_.make_equality_sequence(arg0);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_identity(int nonterminal_index, int base, int arg_index0)
    {
        equality_sequence* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        equality_sequence* r = sa_.identity(arg0);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_1_make_equality_sequence(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        equality_sequence* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        equality* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        equality_sequence* r = sa_.make_equality_sequence(arg0, arg1);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_(int nonterminal_index, int base)
    {
        equality_sequence* r = sa_.();
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_negate_expr(int nonterminal_index, int base, int arg_index0)
    {
        expr* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* r = sa_.make_negate_expr(arg0);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_lambda(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        sequence* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        expr* r = sa_.make_lambda(arg0, arg1);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_expr(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2)
    {
        bin_op* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        expr* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        expr* r = sa_.make_expr(arg0, arg1, arg2);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_define_symbol(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        symbol* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        statement* r = sa_.make_define_symbol(arg0, arg1);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_1_make_expr(int nonterminal_index, int base, int arg_index0)
    {
        expr* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        expr* r = sa_.make_expr(arg0);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_statement(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        expr* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        equality_sequence* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        statement* r = sa_.make_statement(arg0, arg1);
        value_type v; sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool gotof_0(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 0: return push_stack(&parser::state_38, &parser::gotof_38, v);
        case 2: return push_stack(&parser::state_22, &parser::gotof_22, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_0(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        case token_keyword_let:
            // shift
            push_stack(&parser::state_35, &parser::gotof_35, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_1(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_14, &parser::gotof_14, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_1(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        case token_double_slash:
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_keyword_where:
        case token_0:
            return call_0_call_func(2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_2(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_18, &parser::gotof_18, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_2(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_3(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_23, &parser::gotof_23, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_3(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_4(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_24, &parser::gotof_24, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_4(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_5(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_25, &parser::gotof_25, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_5(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_6(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_26, &parser::gotof_26, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_6(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_7(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_27, &parser::gotof_27, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_7(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_8(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_28, &parser::gotof_28, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_8(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_9(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_29, &parser::gotof_29, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_9(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_10(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_30, &parser::gotof_30, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_10(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_11(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_31, &parser::gotof_31, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_11(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_12(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 2: return push_stack(&parser::state_32, &parser::gotof_32, v);
        case 5: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_12(token_type token, const value_type& value)
    {
        switch(token){
        case token_minus:
            // shift
            push_stack(&parser::state_3, &parser::gotof_3, value);
            return false;
        case token_left_paren:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case token_identifier:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case token_symbol:
            // shift
            push_stack(&parser::state_13, &parser::gotof_13, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_13(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_13(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_seq(5, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_14(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_14(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_1_make_seq(5, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_15(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 1: return push_stack(&parser::state_19, &parser::gotof_19, v);
        case 4: return push_stack(&parser::state_20, &parser::gotof_20, v);
        default: assert(0); return false;
        }
    }

    bool state_15(token_type token, const value_type& value)
    {
        switch(token){
        case token_symbol:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_16(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 1: return push_stack(&parser::state_21, &parser::gotof_21, v);
        default: assert(0); return false;
        }
    }

    bool state_16(token_type token, const value_type& value)
    {
        switch(token){
        case token_symbol:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_17(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_17(token_type token, const value_type& value)
    {
        switch(token){
        case token_equal:
            // shift
            push_stack(&parser::state_2, &parser::gotof_2, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_18(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_18(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case token_minus:
            // shift
            push_stack(&parser::state_11, &parser::gotof_11, value);
            return false;
        case token_comma:
        case token_0:
            return call_0_make_equality(1, 3, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_19(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_19(token_type token, const value_type& value)
    {
        switch(token){
        case token_comma:
        case token_0:
            return call_0_make_equality_sequence(4, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_20(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_20(token_type token, const value_type& value)
    {
        switch(token){
        case token_comma:
            // shift
            push_stack(&parser::state_16, &parser::gotof_16, value);
            return false;
        case token_0:
            return call_0_identity(3, 2, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_21(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_21(token_type token, const value_type& value)
    {
        switch(token){
        case token_comma:
        case token_0:
            return call_1_make_equality_sequence(4, 3, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_22(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 3: return push_stack(&parser::state_36, &parser::gotof_36, v);
        default: assert(0); return false;
        }
    }

    bool state_22(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case token_minus:
            // shift
            push_stack(&parser::state_11, &parser::gotof_11, value);
            return false;
        case token_keyword_where:
            // shift
            push_stack(&parser::state_15, &parser::gotof_15, value);
            return false;
        case token_0:
            return call_0_(3, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_23(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_23(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_negate_expr(2, 2, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_24(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_24(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case token_minus:
            // shift
            push_stack(&parser::state_11, &parser::gotof_11, value);
            return false;
        case token_right_paren:
            // shift
            push_stack(&parser::state_34, &parser::gotof_34, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_25(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_25(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case token_minus:
            // shift
            push_stack(&parser::state_11, &parser::gotof_11, value);
            return false;
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_lambda(2, 3, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_26(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_26(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_expr(2, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_27(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_27(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_expr(2, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_28(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_28(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_expr(2, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_29(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_29(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_expr(2, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_30(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_30(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_expr(2, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_31(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_31(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_0_make_expr(2, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_32(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_32(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
            // shift
            push_stack(&parser::state_6, &parser::gotof_6, value);
            return false;
        case token_hat:
            // shift
            push_stack(&parser::state_7, &parser::gotof_7, value);
            return false;
        case token_asterisk:
            // shift
            push_stack(&parser::state_8, &parser::gotof_8, value);
            return false;
        case token_slash:
            // shift
            push_stack(&parser::state_9, &parser::gotof_9, value);
            return false;
        case token_plus:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case token_minus:
            // shift
            push_stack(&parser::state_11, &parser::gotof_11, value);
            return false;
        case token_0:
            return call_0_make_define_symbol(0, 4, 1, 3);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_33(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_33(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_1_make_expr(2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_34(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_34(token_type token, const value_type& value)
    {
        switch(token){
        case token_double_slash:
        case token_hat:
        case token_asterisk:
        case token_slash:
        case token_plus:
        case token_minus:
        case token_left_paren:
        case token_right_paren:
        case token_right_arrow:
        case token_comma:
        case token_identifier:
        case token_symbol:
        case token_keyword_where:
        case token_0:
            return call_1_make_expr(2, 3, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_35(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_35(token_type token, const value_type& value)
    {
        switch(token){
        case token_symbol:
            // shift
            push_stack(&parser::state_37, &parser::gotof_37, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_36(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_36(token_type token, const value_type& value)
    {
        switch(token){
        case token_0:
            return call_0_make_statement(0, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_37(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_37(token_type token, const value_type& value)
    {
        switch(token){
        case token_equal:
            // shift
            push_stack(&parser::state_12, &parser::gotof_12, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_38(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_38(token_type token, const value_type& value)
    {
        switch(token){
        case token_0:
            // accept
            // run_semantic_action();
            accepted_ = true;
            accepted_value_ = get_arg(1, 0);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

};

} // namespace grammar

#endif // PARSER_HPP_
