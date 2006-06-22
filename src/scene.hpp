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

#ifndef SCENE_HPP
#define SCENE_HPP

#include <set>
#include <map>
#include <list>

#include "entity.hpp"

//-----------------------------------------------------------------------------

namespace ops
{
    class scene;

    typedef std::map<int, dBodyID> body_map;

    //-------------------------------------------------------------------------
    // Undo-able / redo-able operation interface.

    class operation
    {
    protected:

        ent::set selection;
        bool done;

    public:

        operation(ent::set& s) : selection(s), done(false) { }

        virtual ent::set& undo(scene *) = 0;
        virtual ent::set& redo(scene *) = 0;

        virtual ~operation() { }
    };

    typedef operation                       *operation_p;
    typedef std::list<operation_p>           operation_l;
    typedef std::list<operation_p>::iterator operation_i;

    //-------------------------------------------------------------------------
    // Object creation operation.

    class create_op : public operation
    {
    public:

        create_op(ent::set&);
       ~create_op();

        ent::set& undo(scene *);
        ent::set& redo(scene *);
    };

    //-------------------------------------------------------------------------
    // Object deletion operation.

    class delete_op : public operation
    {
    public:

        delete_op(ent::set&);

        ent::set& undo(scene *);
        ent::set& redo(scene *);
    };

    //-------------------------------------------------------------------------
    // Object transform modification operation.

    class modify_op : public operation
    {
        float T[16];
        float I[16];

    public:

        modify_op(ent::set&, const float[16]);

        ent::set& undo(scene *);
        ent::set& redo(scene *);
    };

    //-------------------------------------------------------------------------
    // Body creation operation.

    class embody_op : public operation
    {
        std::map<ent::entity *, int> old_id;
        int                          new_id;

    public:

        embody_op(ent::set&, int);

        ent::set& undo(scene *);
        ent::set& redo(scene *);
    };

    //-------------------------------------------------------------------------
    // Joint attachment operation.

    class enjoin_op : public operation
    {
        std::map<ent::entity *, int> old_id;
        int                          new_id;

    public:

        enjoin_op(ent::set&);

        ent::set& undo(scene *);
        ent::set& redo(scene *);
    };

    //-------------------------------------------------------------------------
    // Scene representation.

    class scene
    {
        ent::entity& camera;

        ent::set all;
        ent::set sel;
        
        body_map bodies;

        int serial;

        operation_l undo_list;
        operation_l redo_list;

        void doop(operation_p);

    public:

        scene(ent::entity&);
       ~scene();

        // Editing operations.

        bool selected(ent::entity *);
        bool selected();

        void click_selection(ent::entity *);
        void clone_selection();
        void clear_selection();

        void invert_selection();
        void extend_selection();

        void create_set(ent::set&);
        void delete_set(ent::set&);
        void modify_set(ent::set&, const float[16]);

        // Physical system handlers.

        void embody_all();
        void debody_all();

        void set_param(int, std::string&);
        int  get_param(int, std::string&);

        // Undo-able / redo-able operation handlers.

        void do_create();
        void do_delete();
        void do_enjoin();
        void do_embody();
        void do_debody();
        void do_modify(const float[16]);

        void undo();
        void redo();

        // Physics stepping and rendering handlers.

        void step(float);
        void draw(int) const;

        // Scene I/O.

        void init();
        void load(std::string);
        void save(std::string, bool);
    };
}

//-----------------------------------------------------------------------------

#endif
