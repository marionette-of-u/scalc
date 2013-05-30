#ifndef SCALC_COMMON_HPP
#define SCALC_COMMON_HPP

#include <functional>
#include <vector>
#include <map>
#include <typeinfo>

typedef double fpoint;

// error
class error : public std::runtime_error{
public:
    inline error(std::string message) throw() : std::runtime_error(message){} 

    inline error(const error &other) throw(): runtime_error(other){}

    inline error &operator =(const error &other) throw(){
        std::runtime_error::operator =(other);
        return *this;
    }

    inline virtual ~error() throw(){}
};

// multi-method
template<class FunctionSignature>
class multi_method{
public:
    typedef std::function<FunctionSignature> function_type;
    typedef std::vector<function_type> row;
    typedef std::vector<row> function_table_type;
    inline multi_method(function_type default_function_) : default_function(default_function_){}

    inline void set(std::size_t i, std::size_t j, function_type f){
        if(function_table.size() < i + 1){ function_table.resize(i + 1); }
        std::vector<function_type> &a(function_table[i]);
        if(a.size() < j + 1){ a.resize(j + 1, default_function); }
        a[j] = f;
    }

private:
    multi_method(){}
    function_table_type function_table;
    function_type default_function;
};

namespace poly{
    namespace aux{
        inline std::size_t &get_global_type_idx(){
            static std::size_t i = 0;
            return i;
        }
    }

    template<class T>
    inline std::size_t get_type_idx(){
        static const std::size_t idx = aux::get_global_type_idx()++;
        return idx;
    }

    // 値
    class eval_target{
    public:
        eval_target(std::size_t a_type_idx);
        virtual ~eval_target();
        virtual bool equal(const eval_target *other) const;
        inline bool noteq(const  eval_target *other) const{ return !equal(other); }
        virtual eval_target *copy() const = 0;
        virtual void negate() = 0;
        virtual void complex_conjugate() = 0;

    private:
        eval_target();

    public:
        const std::size_t type_idx;
    };

    // 多項式
    struct node{
        node();
        std::unique_ptr<eval_target> value;
        node *next;
    };

    node *new_node();
    void dispose_node(node *p);
    void dispose(node *p);
    node *constant(fpoint re, fpoint im);
    node *variable(const std::string &str);
    node *variable(const std::string &str, fpoint re, fpoint im);
    node *variable(const std::string &str, eval_target *ptr);
    node *copy(node *p);
    void change_sign(node *p);
    void complex_conjugate(node *p);
    void add(node *p, node *q);
    node *multiply(node *x, node *y);
    node *power(node *x, fpoint n);
    std::string poly_to_string(const node *p);
}

#endif // SCALC_COMMON_HPP
