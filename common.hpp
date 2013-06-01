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
    inline virtual ~error() throw(){}
    inline error &operator =(const error &other) throw(){
        std::runtime_error::operator =(other);
        return *this;
    }
};

// multi-method
template<class FunctionSignature, bool Symmetry = false>
class multi_method{
public:
    typedef std::function<FunctionSignature> function_type;
    typedef std::vector<function_type> row;
    typedef std::vector<row> function_table_type;
    inline multi_method(function_type default_function_) : default_function(default_function_){}
    inline void set(std::size_t i, std::size_t j, function_type f){
        set_(i, j, f);
        if(Symmetry){ set_(j, i, f); }
    }

    inline function_type operator ()(std::size_t i, std::size_t j) const{
        return function_table[i][j];
    }

private:
    multi_method(){}
    inline void set_(std::size_t i, std::size_t j, function_type f){
        if(function_table.size() < i + 1){ function_table.resize(i + 1); }
        std::vector<function_type> &a(function_table[i]);
        if(a.size() < j + 1){ a.resize(j + 1, default_function); }
        a[j] = f;
    }

    function_table_type function_table;
    function_type default_function;
};

namespace poly{
    template<class T>
    class type_idx_manager{
    private:
        inline static std::size_t &get_global_type_idx(){
            static std::size_t i = 0;
            return i;
        }

    public:
        template<class U>
        inline static std::size_t get_type_idx(){
            static const std::size_t idx = get_global_type_idx()++;
            return idx;
        }
    };

    template<class T>
    inline std::size_t get_type_idx(){
        static const std::size_t idx = type_idx_manager<void>::get_type_idx<T>();
        return idx;
    }

    // 多項式
    struct node{
        node();
        ~node();
        bool equal(const node *ptr) const;
        node *copy() const;
        void negate();
        void complex_conjugate();

        // 指数部
        std::map<std::string, node*> e;

        //実部, 虚部
        fpoint real, imag;

        // 次の項
        node *next;
    };

    node *new_node();
    void dispose_node(node *p);
    void dispose(node *p);
    node *constant(fpoint re, fpoint im);
    node *variable(const std::string &str);
    node *variable(const std::string &str, fpoint re, fpoint im = 0);
    node *variable(const std::string &str, node *ptr);
    int lexicographic_compare(const node *l, const node *r);
    node *copy(node *p);
    void change_sign(node *p);
    void complex_conjugate(node *p);
    void add(node *p, node *q);
    node *multiply(node *x, node *y);
    node *power(node *x, node *n);
    std::string poly_to_string(const node *p);
}

#endif // SCALC_COMMON_HPP
