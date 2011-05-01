//  Copyright (C) 2005 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#include <cstring>
#include <cstdio>
#include <cctype>

#include <wrl-param.hpp>

//-----------------------------------------------------------------------------
// Wave functions.  These have the same period and phase as sine.

static double sqr(double x)
{
    if (fmod(x, 2.0 * M_PI) > M_PI)
        return -1.0;
    else
        return +1.0;
}

static double tri(double x)
{
    return 0.0;
}

static double saw(double x)
{
    return 0.0;
}

static double sat(double x)
{
    if (x > 1.0) return 1.0;
    if (x < 0.0) return 0.0;

    return x;
}

//-----------------------------------------------------------------------------
// System state functions.  These mark an expression as varying.
// TODO: Reimplement these.

static bool cacheable;
/*
static double key(double x)
{
    cacheable = false;
    return get_key(int(x));
}

static double btn(double x)
{
    cacheable = false;
    return get_btn(int(x));
}

static double trg(double x)
{
    cacheable = false;
    return get_trg(int(x));
}

static double joy(double x)
{
    cacheable = false;
    return get_joy(int(x));
}
*/
//-----------------------------------------------------------------------------

static const char *num(double& k, const char *p)
{
    if (p)
    {
        double v;
        int    n;

        if (sscanf(p, "%lf%n", &v, &n) >= 1)
        {
            k = v;
            return p + n;
        }
    }
    return 0;
}

static const char *sym(const char *s, const char *p)
{
    if (p)
    {
        const char *q = p;

        while (q[0] && isspace(q[0]))
            ++q;

        size_t n = strlen(s);

        if (strncmp(s, q, n) == 0)
            return q + n;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Recursive descent parser implementation.  Inefficient, but simple.

static const char *exE(double& k, const char *p);

static const char *exF(double& k, const char *p)
{
    const char *q = 0;

    if (p)
    {
        // Literal number.

        if ((q = num(k, p)))
            ;

        // Parenthetical expression.

        else if ((q = sym(")", exE(k, sym("(", p)))))
            ;

        // Built-in symbols.

        else if ((q = sym("inf", p))) k = dInfinity;
        else if ((q = sym("pi",  p))) k = M_PI;

        // Unary negation.

        else if ((q = exE(k, sym("-", p)))) k = -k;

        // Wave function call.

        else if ((q = sym(")", exE(k, sym("(", sym("sin", p)))))) k = sin(k);
        else if ((q = sym(")", exE(k, sym("(", sym("cos", p)))))) k = cos(k);
        else if ((q = sym(")", exE(k, sym("(", sym("sqr", p)))))) k = sqr(k);
        else if ((q = sym(")", exE(k, sym("(", sym("tri", p)))))) k = tri(k);
        else if ((q = sym(")", exE(k, sym("(", sym("saw", p)))))) k = saw(k);
        else if ((q = sym(")", exE(k, sym("(", sym("sat", p)))))) k = sat(k);

        // System state function call.

        /* TODO: Reimplement these with cluster awareness
        else if ((q = sym(")", exE(k, sym("(", sym("key", p)))))) k = key(k);
        else if ((q = sym(")", exE(k, sym("(", sym("btn", p)))))) k = btn(k);
        else if ((q = sym(")", exE(k, sym("(", sym("joy", p)))))) k = joy(k);
        else if ((q = sym(")", exE(k, sym("(", sym("trg", p)))))) k = trg(k);
        */
        // System time reference.

        // TODO: Reimplement time expression
        // else if ((q = sym("t", p)))
        // {
        //     cacheable = false;
        //     k = get_time();
        // }
    }
    return q;
}

static const char *exT(double& k, const char *p)
{
    const char *q = 0;
    double a;
    double b;

    if (p)
    {
        // Parse a multiplication or addition expression.

        if      ((q = exT(b, sym("*", exF(a, p))))) k = a * b;
        else if ((q = exT(b, sym("/", exF(a, p))))) k = a / b;

        else q = exF(k, p);
    }
    return q;
}

static const char *exE(double& k, const char *p)
{
    const char *q = 0;
    double a;
    double b;

    if (p)
    {
        // Parse an addition or subtraction expression.

        if      ((q = exE(b, sym("+", exT(a, p))))) k = a + b;
        else if ((q = exE(b, sym("-", exT(a, p))))) k = a - b;

        else q = exT(k, p);
    }
    return q;
}

//=============================================================================

wrl::param::param(std::string name, std::string expr) :
    state(false), cache(0), name(name), expr(expr)
{
}

//-----------------------------------------------------------------------------

double wrl::param::value()
{
    // If we have a chached value, return it.

    if (state)
        return cache;
    else
    {
        cacheable = true;

        // Evaluate the expression and cache as cache can.

        if (exE(cache, expr.c_str()))
            state = cacheable;
        else
        {
            // A syntax error is a cacheable zero.

            state = true;
            cache = 0.0;
        }
        return cache;
    }
}

//-----------------------------------------------------------------------------

void wrl::param::load(app::node node)
{
    if (app::node n = node.find("param", "name", name))
        expr = n.get_s();
}

void wrl::param::save(app::node node)
{
    app::node n("param");

    n.set_s("name", name);
    n.set_s(expr);
    n.insert(node);
}

//-----------------------------------------------------------------------------
