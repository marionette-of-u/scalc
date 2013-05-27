#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include "common.hpp"

node::node() : e(), real(), imag(), next(nullptr){}
node::~node(){ delete next; }

template<class T>
inline std::string to_string(const T &v){
    std::stringstream ss;
    ss << v;
    return ss.str();
}

// 項を生成
node *new_node(){
    return new node;
}

// 項を破棄
void dispose_node(node *p){
    delete p;
}

// 多項式を破棄
void dispose(node *p){
    while(p){
        node *q = p->next;
        dispose_node(p);
        p = q;
    }
}

// 定数を生成
node *constant(c_type re, c_type im){
    node *p = new_node();
    if(re != 0 || im != 0){
        node *q = new_node();
        q->real = re, q->imag = im;
        p->next = q;
    }
    return p;
}

// 多項式をコピー
node *copy(node *p){
    node *q, *r;
    q = r = new_node();
    while(p = p->next){
        r = r->next = new_node();
        r->real = p->real;
        r->imag = p->imag;
        r->e = p->e;
    }
    return q;
}

// 符号を反転
void change_sign(node *p){
    while(p = p->next){
        p->real = -p->real;
        p->imag = -p->imag;
    }
}

// 微分
void differentiate(node *p, const std::string &str){
    node *p1;
    p1 = p, p = p->next;
    while(p){
        auto iter = p->e.find(str);
        if(iter != p->e.end()){
            fpoint e = iter->second;
            iter->second = e - 1;
            p->real *= e;
            p->imag *= e;
            p1 = p, p = p->next;
        }else{
            p = p->next;
            dispose_node(p1->next);
            p1->next = p;
        }
    }
}

// 複素共役
void complex_conjugate(node *p){
    while(p = p->next){
        p->imag = -p->imag;
    }
}

// 加算
// qは消失する
void add(node *p, node *q){
    node *p1 = p, *q1 = q;
    e_type ep, eq;
    p = p->next;
    q = q->next;
    dispose_node(q1);
    while(q){
        while(p){
            for(auto p_iter = p->e.begin(); p_iter != p->e.end(); ++p_iter){
                ep = p_iter->second;
                auto q_iter = q->e.find(p_iter->first);
                if(q_iter == q->e.end()){
                    eq = 0;
                    goto breakpoint;
                }else{ eq = q_iter->second; }
                if(ep != eq){ goto breakpoint; }
            }
            for(auto q_iter = q->e.begin(); q_iter != q->e.end(); ++q_iter){
                eq = q_iter->second;
                auto p_iter = p->e.find(q_iter->first);
                if(p_iter != p->e.end()){ continue; }else{
                    ep = 0;
                    goto breakpoint;
                }
            }
            breakpoint:;
            if(ep <= eq){ break; }
            p1 = p, p = p->next;
        }
        if(p || ep < eq){
            p1->next = q, p1 = q, q = q->next;
            p1->next = p;
        }else{
            p->real += q->real;
            p->imag += q->imag;
            if(p->real != 0 || p->imag != 0){
                p = p->next;
                dispose_node(p1->next);
                p1->next = p;
            }
            q1 = q, q = q->next, dispose_node(q1);
        }
    }
}

// 式を文字列として得る
std::string poly_to_string(const node *p){
    bool first, one;
    c_type re, im;
    e_type e;
    std::string r;

    first = true;
    while(p = p->next){
        one = false;
        re = p->real;
        im = p->imag;
        if(im == 0){
            if(re >= 0){
                if(!first){ r += " + "; }
            }else{
                re = -re;
                r += " - ";
            }
            if(re == 1){ one = true; }else{ r += to_string(re); }
        }else if(re == 0){
            if(im >= 0){
                if(!first){ r += " + "; }
            }else{
                im = -im;
                r += " - ";
            }
            r += to_string(im) + "i";
        }else{
            if(!first){ r += " + "; }
            r += to_string(re) + "+" + to_string(im) + "i";
        }
        first = false;
        for(auto iter = p->e.begin(); iter != p->e.end(); ++iter){
            if((e = iter->second) != 0){
                if(!one){ r += " * "; }
                one = false;
                r += iter->first;
                if(e != 1){
                    r += "^" + to_string(e);
                }
            }
        }
        if(one){ r += "1"; }
    }
    if(first){ r += "0"; }
    return r;
}
