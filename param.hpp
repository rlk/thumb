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

#ifndef PARAM_HPP
#define PARAM_HPP

#include <string>

#include <ode/ode.h>
#include <mxml.h>

#include "util.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class param
    {
    protected:

        std::string expr;

        virtual std::string name() = 0;

    public:

        param(std::string expr) : expr(expr) { }

        void set(std::string& e) { expr = e; }
        void get(std::string& e) { e = expr; }

        dReal value();

        virtual void load(mxml_node_t *) = 0;
        virtual void save(mxml_node_t *) = 0;

        virtual ~param() { }
    };
}

//-----------------------------------------------------------------------------

namespace ent
{
    typedef void (*dJointSetParam)(dJointID, int, dReal);

    class solid_param : public param
    {
    public:
        solid_param(std::string expr) : param(expr) { }

        virtual void apply(dSurfaceParameters&) = 0;
        virtual void load(mxml_node_t *);
        virtual void save(mxml_node_t *);
        virtual solid_param *clone() const = 0;
    };

    class joint_param : public param
    {
    protected:
        int axis;
    public:
        joint_param(std::string expr, int axis) : param(expr), axis(axis) { }

        virtual void apply(dJointSetParam, dJointID) = 0;
        virtual void load(mxml_node_t *);
        virtual void save(mxml_node_t *);
        virtual joint_param *clone() const = 0;
    };
}

//-----------------------------------------------------------------------------

namespace ent
{
    class solid_param_friction : public solid_param
    {
    protected:
        std::string name() { return "mu"; }
    public:
        solid_param_friction() : solid_param("100") { }

        void apply(dSurfaceParameters& surface) {
            surface.mu = MIN(surface.mu, value());
        }
        solid_param_friction *clone() const {
            return new solid_param_friction(*this);
        }
    };

    class solid_param_density : public solid_param
    {
    protected:
        std::string name() { return "density"; }
    public:
        solid_param_density() : solid_param("1.0") { }

        void apply(dSurfaceParameters& surface) {
        }
        solid_param_density *clone() const {
            return new solid_param_density(*this);
        }
    };

    class solid_param_bounce : public solid_param
    {
    protected:
        std::string name() { return "bounce"; }
    public:
        solid_param_bounce() : solid_param("0.5") { }

        void apply(dSurfaceParameters& surface) {
            surface.bounce = MAX(surface.bounce, value());
        }
        solid_param_bounce *clone() const {
            return new solid_param_bounce(*this);
        }
    };

    class solid_param_soft_erp : public solid_param
    {
    protected:
        std::string name() { return "soft_erp"; }
    public:
        solid_param_soft_erp() : solid_param("0.2") { }

        void apply(dSurfaceParameters& surface) {
            surface.soft_erp = MIN(surface.soft_erp, value());
        }
        solid_param_soft_erp *clone() const {
            return new solid_param_soft_erp(*this);
        }
    };

    class solid_param_soft_cfm : public solid_param
    {
    protected:
        std::string name() { return "soft_cfm"; }
    public:
        solid_param_soft_cfm() : solid_param("0.0") { }

        void apply(dSurfaceParameters& surface) {
            surface.soft_cfm = MAX(surface.soft_cfm, value());
        }
        solid_param_soft_cfm *clone() const {
            return new solid_param_soft_cfm(*this);
        }
    };
}

//-----------------------------------------------------------------------------

namespace ent
{
    class joint_param_velocity : public joint_param
    {
    protected:
        std::string name() { return "dParamVel"; }
    public:
        joint_param_velocity(int axis=0) : joint_param("0", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamVel + dParamGroup * axis, value());
        }
        joint_param_velocity *clone() const {
            return new joint_param_velocity(*this);
        }
    };

    class joint_param_force : public joint_param
    {
    protected:
        std::string name() { return "dParamFMax"; }
    public:
        joint_param_force(int axis=0) : joint_param("0", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamFMax + dParamGroup * axis, value());
        }
        joint_param_force *clone() const {
            return new joint_param_force(*this);
        }
    };

    class joint_param_cfm : public joint_param
    {
    protected:
        std::string name() { return "dParamCFM"; }
    public:
        joint_param_cfm(int axis=0) : joint_param("0", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamCFM + dParamGroup * axis, value());
        }
        joint_param_cfm *clone() const {
            return new joint_param_cfm(*this);
        }
    };

    class joint_param_bounce : public joint_param
    {
    protected:
        std::string name() { return "dParamBounce"; }
    public:
        joint_param_bounce(int axis=0) : joint_param("0.5", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamBounce + dParamGroup * axis, value());
        }
        joint_param_bounce *clone() const {
            return new joint_param_bounce(*this);
        }
    };

    class joint_param_lo_stop : public joint_param
    {
    protected:
        std::string name() { return "dParamLoStop"; }
    public:
        joint_param_lo_stop(int axis=0) : joint_param("-inf", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamLoStop + dParamGroup * axis, value());
        }
        joint_param_lo_stop *clone() const {
            return new joint_param_lo_stop(*this);
        }
    };

    class joint_param_hi_stop : public joint_param
    {
    protected:
        std::string name() { return "dParamHiStop"; }
    public:
        joint_param_hi_stop(int axis=0) : joint_param("inf", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamHiStop + dParamGroup * axis, value());
        }
        joint_param_hi_stop *clone() const {
            return new joint_param_hi_stop(*this);
        }
    };

    class joint_param_stop_erp : public joint_param
    {
    protected:
        std::string name() { return "dParamStopERP"; }
    public:
        joint_param_stop_erp(int axis=0) : joint_param("0.2", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamStopERP + dParamGroup * axis, value());
        }
        joint_param_stop_erp *clone() const {
            return new joint_param_stop_erp(*this);
        }
    };

    class joint_param_stop_cfm : public joint_param
    {
    protected:
        std::string name() { return "dParamStopCFM"; }
    public:
        joint_param_stop_cfm(int axis=0) : joint_param("0.0", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamStopCFM + dParamGroup * axis, value());
        }
        joint_param_stop_cfm *clone() const {
            return new joint_param_stop_cfm(*this);
        }
    };

    class joint_param_susp_erp : public joint_param
    {
    protected:
        std::string name() { return "dParamSuspensionERP"; }
    public:
        joint_param_susp_erp(int axis=0) : joint_param("0.2", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamSuspensionERP + dParamGroup * axis, value());
        }
        joint_param_susp_erp *clone() const {
            return new joint_param_susp_erp(*this);
        }
    };

    class joint_param_susp_cfm : public joint_param
    {
    protected:
        std::string name() { return "dParamSuspensionCFM"; }
    public:
        joint_param_susp_cfm(int axis=0) : joint_param("0.0", axis) { }

        void apply(dJointSetParam func, dJointID joint) {
            func(joint, dParamSuspensionCFM + dParamGroup * axis, value());
        }
        joint_param_susp_cfm *clone() const {
            return new joint_param_susp_cfm(*this);
        }
    };
}

//-----------------------------------------------------------------------------

#endif
