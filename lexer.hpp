#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <utility>
#include <iterator>
#include <cstring>
#include <cctype>
#include <ctype.h>

namespace lexer{

enum token{
    token_whitespace,
    token_right_arrow,
    token_double_slash,
    token_hat,
    token_asterisk,
    token_slash,
    token_plus,
    token_minus,
    token_left_paren,
    token_right_paren,
    token_equal,
    token_comma,
    token_identifier,
    token_keyword_where,
    token_keyword_let,
    token_symbol
};

class lexer{
public:
    template<class InputIter>
    static std::pair<bool, InputIter> reg_whitespace(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        do{
            if(iter == last){ match = false; }else{
                InputIter iter_prime = iter;
                do{
                    if(iter != last && *iter == ' '){
                        ++iter;
                        match = true;
                    }else{ match = false; }
                    if(!match){ iter = iter_prime; break; }
                }while(false);
            }
            if(!match){ break; }
            InputIter iter_prime = iter;
            while(iter != last){
                if(iter == last){ match = false; }else{
                    InputIter iter_prime = iter;
                    do{
                        if(iter != last && *iter == ' '){
                            ++iter;
                            match = true;
                        }else{ match = false; }
                        if(!match){ iter = iter_prime; break; }
                    }while(false);
                }
                if(match){ iter_prime = iter; }else{
                    iter = iter_prime;
                    match = true;
                    break;
                }
            }
        }while(false);
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_right_arrow(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '-'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == '>'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_double_slash(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '/'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == '/'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_hat(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '^'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_asterisk(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '*'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_slash(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '/'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_plus(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '+'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_minus(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '-'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_left_paren(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '('){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_right_paren(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == ')'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_equal(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == '='){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_comma(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == ','){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_identifier(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter == last){ match = false; }else{ 
                    InputIter iter_prime = iter;
                    do{
                        if(iter == last){ match = false; }else{
                            InputIter iter_prime = iter;
                            do{
                                if(iter == last){ match = false; }else{
                                    char c = *iter;
                                    if(
                                        ((c >= '1') && (c <= '9'))
                                    ){
                                        ++iter;
                                        match = true;
                                    }else{ match = false; }
                                }
                                if(!match){ iter = iter_prime; break; }
                                {
                                    InputIter iter_prime = iter;
                                    while(iter != last){
                                        if(iter == last){ match = false; }else{
                                            char c = *iter;
                                            if(
                                                ((c >= '0') && (c <= '9'))
                                            ){
                                                ++iter;
                                                match = true;
                                            }else{ match = false; }
                                        }
                                        if(match){ iter_prime = iter; }else{
                                            iter = iter_prime;
                                            match = true;
                                            break;
                                        }
                                    }
                                }
                                if(!match){ iter = iter_prime; break; }
                            }while(false);
                        }
                        if(match){ break; }else{ iter = iter_prime; }
                        if(iter != last && *iter == '0'){
                            ++iter;
                            match = true;
                        }else{ match = false; }
                        if(!match){ iter = iter_prime; }
                    }while(false);
                }
                if(!match){ iter = iter_prime; break; }
                if(iter == last){ match = false; }else{
                    InputIter iter_prime = iter;
                    do{
                        if(iter == last){ match = false; }else{
                            InputIter iter_prime = iter;
                            do{
                                if(iter != last && *iter == '.'){
                                    ++iter;
                                    match = true;
                                }else{ match = false; }
                                if(!match){ iter = iter_prime; break; }
                            }while(false);
                        }
                        if(!match){ iter = iter_prime; break; }
                        do{
                            if(iter == last){ match = false; }else{
                                char c = *iter;
                                if(
                                    ((c >= '0') && (c <= '9'))
                                ){
                                    ++iter;
                                    match = true;
                                }else{ match = false; }
                            }
                            if(!match){ break; }
                            InputIter iter_prime = iter;
                            while(iter != last){
                                if(iter == last){ match = false; }else{
                                    char c = *iter;
                                    if(
                                        ((c >= '0') && (c <= '9'))
                                    ){
                                        ++iter;
                                        match = true;
                                    }else{ match = false; }
                                }
                                if(match){ iter_prime = iter; }else{
                                    iter = iter_prime;
                                    match = true;
                                    break;
                                }
                            }
                        }while(false);
                        if(!match){ iter = iter_prime; break; }
                    }while(false);
                }
                match = true;
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_keyword_where(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == 'w'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == 'h'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == 'e'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == 'r'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == 'e'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_keyword_let(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter != last && *iter == 'l'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == 'e'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
                if(iter != last && *iter == 't'){
                    ++iter;
                    match = true;
                }else{ match = false; }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter>
    static std::pair<bool, InputIter> reg_symbol(InputIter first, InputIter last){
        InputIter iter = first;
        bool match = true;
        if(iter == last){ match = false; }else{
            InputIter iter_prime = iter;
            do{
                if(iter == last){ match = false; }else{
                    char c = *iter;
                    if(
                        ((c >= 'a') && (c <= 'z')) ||
                        ((c >= 'A') && (c <= 'Z')) ||
                        (c == '_')
                    ){
                        ++iter;
                        match = true;
                    }else{ match = false; }
                }
                if(!match){ iter = iter_prime; break; }
                {
                    InputIter iter_prime = iter;
                    while(iter != last){
                        if(iter == last){ match = false; }else{
                            char c = *iter;
                            if(
                                ((c >= 'a') && (c <= 'z')) ||
                                ((c >= 'A') && (c <= 'Z')) ||
                                ((c >= '0') && (c <= '9')) ||
                                (c == '_')
                            ){
                                ++iter;
                                match = true;
                            }else{ match = false; }
                        }
                        if(match){ iter_prime = iter; }else{
                            iter = iter_prime;
                            match = true;
                            break;
                        }
                    }
                }
                if(!match){ iter = iter_prime; break; }
            }while(false);
        }
        return std::make_pair(match, iter);
    }

    template<class InputIter, class InsertIter>
    static std::pair<bool, InputIter> tokenize(InputIter first, InputIter last, InsertIter token_inserter){
        InputIter iter = first;
        std::pair<bool, InputIter> result;
        while(iter != last){
            result = reg_whitespace(iter, last);
            if(result.first){
                iter = result.second;
                continue;
            }
            result = reg_right_arrow(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_right_arrow, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_double_slash(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_double_slash, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_hat(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_hat, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_asterisk(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_asterisk, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_slash(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_slash, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_plus(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_plus, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_minus(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_minus, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_left_paren(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_left_paren, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_right_paren(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_right_paren, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_equal(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_equal, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_comma(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_comma, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_identifier(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_identifier, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_keyword_where(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_keyword_where, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_keyword_let(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_keyword_let, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            result = reg_symbol(iter, last);
            if(result.first){
                *token_inserter = std::make_pair(token_symbol, std::make_pair(iter, result.second));
                iter = result.second;
                continue;
            }
            break;
        }
        return result;
    }
};
} // namespace lexer

#endif // LEXER_HPP_

