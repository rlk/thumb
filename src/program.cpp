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

#include "program.hpp"
#include "data.hpp"

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

//-----------------------------------------------------------------------------

const ogl::program *ogl::program::current = NULL;

ogl::program::program(std::string vert_name,
                      std::string frag_name) :
    vert_name(vert_name),
    frag_name(frag_name),
    vert(0), frag(0)
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

void ogl::program::init()
{
    // Load the shader files.

    size_t vert_siz;
    size_t frag_siz;

    const GLcharARB *vert_txt =
        (const GLcharARB *) ::data->load(vert_name, &vert_siz);
    const GLcharARB *frag_txt =
        (const GLcharARB *) ::data->load(frag_name, &frag_siz);

    GLint vert_len = GLint(vert_siz);
    GLint frag_len = GLint(frag_siz);

    // Compile the vertex shader.

    if (vert_txt)
    {
        vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        glShaderSourceARB (vert, 1, &vert_txt, &vert_len);
        glCompileShaderARB(vert);
        
        log(vert, vert_name);
    }

    // Compile the frag shader.

    if (frag_txt)
    {
        frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        glShaderSourceARB (frag, 1, &frag_txt, &frag_len);
        glCompileShaderARB(frag);
        
        log(frag, frag_name);
    }

    // Link these shader objects to a program object.

    prog = glCreateProgramObjectARB();

    if (vert) glAttachObjectARB(prog, vert);
    if (frag) glAttachObjectARB(prog, frag);

    // Bind the tangent attribute if needed.  (HACK)

    if (strstr(vert_txt,  "attribute vec3 Tangent"))
        glBindAttribLocationARB(prog, 6, "Tangent");

    glLinkProgramARB(prog);

    log(prog, vert_name);

    // Free the shader files.

    ::data->free(frag_name);
    ::data->free(vert_name);

    OGLCK();
}

void ogl::program::fini()
{
    if (prog) glDeleteObjectARB(prog);
    if (vert) glDeleteObjectARB(vert);
    if (frag) glDeleteObjectARB(frag);

    OGLCK();
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
