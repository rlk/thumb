//  Copyright (C) 2007 Robert Kooima
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

#include <iostream>
#include <cstring>

#include "ogl-program.hpp"
#include "app-serial.hpp"
#include "app-data.hpp"

//-----------------------------------------------------------------------------

void ogl::program::log(GLhandleARB handle, std::string& name)
{
    char *log = 0;
    GLint len = 0;

    // Dump the contents of the log, if any.

    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);

    if ((len > 1) && (log = new char[len + 1]))
    {
        glGetInfoLogARB(handle, len, NULL, log);

        std::cerr << name << std::endl;
        std::cerr << log  << std::endl;

        delete [] log;
    }
}

GLhandleARB ogl::program::compile(GLenum type, std::string& name,
                                               std::string& text)
{
    GLhandleARB handle;

    // Compile the given shader text.

    if (!text.empty())
    {
        handle = glCreateShaderObjectARB(type);

        const char *data = text.data();
        GLint       size = text.size();

        glShaderSourceARB (handle, 1, &data, &size);
        glCompileShaderARB(handle);
        
        log(handle, name);
    }

    return handle;
}

//-----------------------------------------------------------------------------

const ogl::program *ogl::program::current = NULL;

ogl::program::program(std::string name) : name(name), vert(0), frag(0)
{
    init();
}

ogl::program::~program()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::program::bind() const
{
    current = this;
    glUseProgramObjectARB(prog);
    OGLCK();
}

void ogl::program::free() const
{
    glUseProgramObjectARB(0);
    OGLCK();
}

//-----------------------------------------------------------------------------

std::string ogl::program::load(std::string name)
{
    std::string            base;
    std::string::size_type incl = 0;

    size_t len;

    // Load the named program file.

    base.append((const char *) ::data->load(name, &len));

    ::data->free(name);

    // Scan the string for #include directives.

    while ((incl = base.find("#include", incl)) != std::string::npos)
    {
        // Parse the included file name.

        std::string::size_type lq = base.find("\"", incl);
        std::string::size_type rq = base.find("\"", lq+1);

        std::string file(base, lq + 1, rq - lq - 1);

        // Replace the include directive with the loaded string.

        base.replace(incl, rq - incl + 1, load(file));
    }

    // Return the final string.

    return base;
}

void ogl::program::init()
{
    std::string path = "program/" + name;

    app::serial file(path.c_str());
    app::node   root;
    app::node   curr;

    if ((root = app::find(file.get_head(), "program")))
    {
        std::string vert_name(app::get_attr_s(root, "vert"));
        std::string frag_name(app::get_attr_s(root, "frag"));

        // Load the shader files.

        std::string vert_text = load(vert_name);
        std::string frag_text = load(frag_name);

        // Compile the shaders.

        vert = compile(GL_VERTEX_SHADER_ARB,   vert_name, vert_text);
        frag = compile(GL_FRAGMENT_SHADER_ARB, frag_name, frag_text);

        // Link the shader objects to a program object.

        prog = glCreateProgramObjectARB();

        if (vert) glAttachObjectARB(prog, vert);
        if (frag) glAttachObjectARB(prog, frag);

        // Bind the attributes.

        for (curr = app::find(root,       "attribute"); curr;
             curr = app::next(root, curr, "attribute"))

            glBindAttribLocationARB(prog, app::get_attr_d(curr, "location"),
                                          app::get_attr_s(curr, "name"));

        // Link the program.

        glLinkProgramARB(prog);
        log(prog, vert_name);
        OGLCK();

        // Set the sampler uniforms.

        bind();
        {
            for (curr = app::find(root,       "sampler"); curr;
                 curr = app::next(root, curr, "sampler"))
            {
                std::string name(app::get_attr_s(curr, "name"));
                int         unit=app::get_attr_d(curr, "unit");

                uniform(name, unit);
            
                sampler_map[name] = GL_TEXTURE0 + unit;
            }
        }
        free();
    }
}

void ogl::program::fini()
{
    sampler_map.clear();

    if (prog) glDeleteObjectARB(prog);
    if (vert) glDeleteObjectARB(vert);
    if (frag) glDeleteObjectARB(frag);

    OGLCK();
}

//-----------------------------------------------------------------------------

GLenum ogl::program::unit(std::string name) const
{
    // Determine the texture unit of the named sampler uniform.

    std::map<std::string, GLenum>::const_iterator i;

    if ((i = sampler_map.find(name)) == sampler_map.end())
        return 0;
    else
        return i->second;
}

//-----------------------------------------------------------------------------

void ogl::program::uniform(std::string name, int d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1iARB(loc, d);

    OGLCK();
}

void ogl::program::uniform(std::string name, double a) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1fARB(loc, GLfloat(a));

    OGLCK();
}

void ogl::program::uniform(std::string name, double a,
                                             double b) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform2fARB(loc, GLfloat(a),
                            GLfloat(b));
    OGLCK();
}

void ogl::program::uniform(std::string name, double a,
                                             double b,
                                             double c) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform3fARB(loc, GLfloat(a),
                            GLfloat(b),
                            GLfloat(c));
    OGLCK();
}

void ogl::program::uniform(std::string name, double a,
                                             double b,
                                             double c,
                                             double d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform4fARB(loc, GLfloat(a),
                            GLfloat(b),
                            GLfloat(c),
                            GLfloat(d));
    OGLCK();
}

void ogl::program::uniform(std::string name, const double *M, bool t) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
    {
        GLfloat T[16];

        T[ 0] = GLfloat(M[ 0]);
        T[ 1] = GLfloat(M[ 1]);
        T[ 2] = GLfloat(M[ 2]);
        T[ 3] = GLfloat(M[ 3]);
        T[ 4] = GLfloat(M[ 4]);
        T[ 5] = GLfloat(M[ 5]);
        T[ 6] = GLfloat(M[ 6]);
        T[ 7] = GLfloat(M[ 7]);
        T[ 8] = GLfloat(M[ 8]);
        T[ 9] = GLfloat(M[ 9]);
        T[10] = GLfloat(M[10]);
        T[11] = GLfloat(M[11]);
        T[12] = GLfloat(M[12]);
        T[13] = GLfloat(M[13]);
        T[14] = GLfloat(M[14]);
        T[15] = GLfloat(M[15]);

        glUniformMatrix4fvARB(loc, 1, t, T);
    }
    OGLCK();
}

//-----------------------------------------------------------------------------
