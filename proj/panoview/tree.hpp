// Copyright (c) 2011 Robert Kooima
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef TREE_HPP
#define TREE_HPP

#include <iostream>

//------------------------------------------------------------------------------

template<typename T> class tree
{
public:

    tree() : root(0) { }
   ~tree();
   
    T    search(T);
    void insert(T);
    void remove(T);
    
    T    eject();
    void clear();
    void dump();
    
private:

    // Splay tree implementation.

    struct node
    {
        node *l;
        node *r;
        T     p;
    };
    
    node *splay(node *t, T p);
    node *root;

    // LRU ejection handlers.

    node *dig(node *,  int &);
    T     cut(node *, node *);
    
    void dump(node *, int);
};

//------------------------------------------------------------------------------

template<typename T> tree<T>::~tree()
{
}

template<typename T> typename tree<T>::node *tree<T>::splay(node *t, T p)
{
    if (t)
    {
        node  n;
        node *l = &n;
        node *r = &n;
        node *y;

        n.l = 0;
        n.r = 0;

        for (;;)
        {
            if (p < t->p)
            {
                if (t->l != 0 && p < t->l->p)
                {
                    y    = t->l;
                    t->l = y->r;
                    y->r = t;
                    t    = y;
                }
                if (t->l == 0) break;

                r->l = t;
                r    = t;
                t    = t->l;
                continue;
            }

            if (t->p < p)
            {
                if (t->r != 0 && t->r->p < p)
                {
                    y    = t->r;
                    t->r = y->l;
                    y->l = t;
                    t    = y;
                }
                if (t->r == 0) break;

                l->r = t;
                l    = t;
                t    = t->r;
                continue;
            }
            break;
        }

        l->r = t->l;
        r->l = t->r;
        t->l = n.r;
        t->r = n.l;
    }
    return t;
}

//------------------------------------------------------------------------------
// The following functions implement the standard set of splay tree operations,
// search, insert, and remove. Each of these splays the requested node to the
// root, thus optimizing access to related items.

template<typename T> T tree<T>::search(T p)
{
    if (root)
    {
        root = splay(root, p);
        return root->p;
    }
    return 0;
}

template<typename T> void tree<T>::insert(T p)
{
    node *m = root = splay(root, p);

    if (m == 0 || p < m->p || m->p < p)
    {
        if (node *n = new node)
        {
            n->p = p;
            root = n;

            if (m == 0)
            {
                n->l = 0;
                n->r = 0;
            }
            else if (p < m->p)
            {
                n->l = m->l;
                n->r = m;
                m->l = 0;
            }
            else if (m->p < p)
            {
                n->r = m->r;
                n->l = m;
                m->r = 0;
            }
        }
    }
}

template<typename T> void tree<T>::remove(T p)
{
    node *m = root = splay(root, p);

    if (m == 0 || p < m->p || m->p < p)
        return;
    else
    {
        if (m->l)
        {
            root    = splay(m->l, p);
            root->r = m->r;
        }
        else
            root    = m->r;

        delete m;
    }
}

template<typename T> void tree<T>::clear()
{
    while (root) remove(root->p);
}

//------------------------------------------------------------------------------
// The following functions implement a non-standard splay tree operation, LRU
// eject. The dig function searches for the deepest node in the tree and the cut
// function removes it. Both of these actions are performed without splaying the
// tree, as to do so would optimize it toward items related to the LRU. 

template<typename T> T tree<T>::eject()
{
    int d = 0;

    if (node *m = dig(root, d))
    {
        if (m == root)
        {
            T p = m->p;
            root = 0;
            delete m;
            return p;
        }
        else return cut(root, m);
    }
    return 0;
}

template<typename T> typename tree<T>::node *tree<T>::dig(node *t, int& d)
{
    if (t)
    {
        if (t->l == 0 && t->r == 0)
        {
            d = 0;
            return t;
        }
        else
        {
            int dl = -1;
            int dr = -1;
            
            node *l = t->l ? dig(t->l, dl) : 0;
            node *r = t->r ? dig(t->r, dr) : 0;

            if (dl > dr) { d = dl + 1; return l; }
            else         { d = dr + 1; return r; }
        }
    }
    return 0;
}

template<typename T> T tree<T>::cut(node *t, node *c)
{
    if (t && c)
    {
        if (t->l == c)
        {
            T p = c->p;        
            t->l = 0;
            delete c;
            return p;
        }
        if (t->r == c)
        {
            T p = c->p;        
            t->r = 0;
            delete c;
            return p;
        }
        
        if (c->p < t->p)
            return cut(t->l, c);
        else
            return cut(t->r, c);
    }
    return 0;
}

//------------------------------------------------------------------------------

template<typename T> void tree<T>::dump(node *t, int d)
{
    if (t)
    {
        dump(t->l, d + 1);

        for (int i = 0; i < d; ++i)
            std::cout << "  ";            
        std::cout << t->p << std::endl;
        
        dump(t->r, d + 1);
    }
}

template<typename T> void tree<T>::dump()
{
    dump(root, 0);
}

//------------------------------------------------------------------------------

#endif