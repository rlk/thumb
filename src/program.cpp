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

#include "program.hpp"
#include "main.hpp"

//-----------------------------------------------------------------------------

void ogl::program::log(GLhandleARB handle)
{
    char *log;
    GLint len;

    // Dump the contents of the log, if any.

    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);

    if ((len > 1) && (log = new char[len + 1]))
    {
        glGetInfoLogARB(handle, len, NULL, log);

        std::cerr << log << std::endl;

        delete [] log;
    }
}

//-----------------------------------------------------------------------------

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

    const  GLcharARB *vert_txt
        = (GLcharARB *) ::data->load(name + ".vert", &vert_siz);
    const  GLcharARB *frag_txt
        = (GLcharARB *) ::data->load(name + ".frag", &frag_siz);

    GLint vert_len = GLint(vert_siz);
    GLint frag_len = GLint(frag_siz);

    // Compile the vertex shader.

    if (vert_txt)
    {
        vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        glShaderSourceARB (vert, 1, &vert_txt, &vert_len);
        glCompileShaderARB(vert);
        
        log(vert);
    }

    // Compile the frag shader.

    if (frag_txt)
    {
        frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        glShaderSourceARB (frag, 1, &frag_txt, &frag_len);
        glCompileShaderARB(frag);
        
        log(frag);
    }

    // Link these shader objects to a program object.

    prog = glCreateProgramObjectARB();

    if (vert) glAttachObjectARB(prog, vert);
    if (frag) glAttachObjectARB(prog, frag);

    glLinkProgramARB(prog);

    log(prog);

    // Free the shader files.

    ::data->free(name + ".frag");
    ::data->free(name + ".vert");

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

void ogl::program::uniform(std::string name, float a) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1fARB(loc, a);

    OGLCK();
}

void ogl::program::uniform(std::string name, float a, float b) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform2fARB(loc, a, b);

    OGLCK();
}

void ogl::program::uniform(std::string name, float a, float b, float c) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform3fARB(loc, a, b, c);

    OGLCK();
}

void ogl::program::uniform(std::string name, float a, float b,
                                             float c, float d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform4fARB(loc, a, b, c, d);

    OGLCK();
}

//-----------------------------------------------------------------------------
