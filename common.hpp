#ifndef SCALC_COMMON_HPP
#define SCALC_COMMON_HPP

#include <map>
#include <typeinfo>

typedef double fpoint;

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
    node *copy(node *p);
    void change_sign(node *p);
    void complex_conjugate(node *p);
    void add(node *p, node *q);
    node *multiply(node *x, node *y);
    node *power(node *x, fpoint n);
    std::string poly_to_string(const node *p);
}

#endif // SCALC_COMMON_HPP
