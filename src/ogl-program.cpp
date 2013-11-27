//  Copyright (C) 2007-2011 Robert Kooima
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

#include <ogl-uniform.hpp>
#include <ogl-program.hpp>
#include <app-glob.hpp>
#include <app-data.hpp>

//-----------------------------------------------------------------------------

bool ogl::program::program_log(GLuint handle, const std::string& name)
{
    char *log = 0;
    GLint len = 0;
    GLint tst = 0;

    // Dump the contents of the log, if any.

    glGetProgramiv(handle, GL_LINK_STATUS,     &tst);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);

    if ((tst == 0) && (len > 1) && (log = new char[len + 1]))
    {
        glGetProgramInfoLog(handle, len, NULL, log);

        fprintf(stderr, "%s\n%s", name.c_str(), log);

        delete [] log;

        return true;
    }
    return false;
}

bool ogl::program::shader_log(GLuint handle, const std::string& name)
{
    char *log = 0;
    GLint len = 0;
    GLint tst = 0;

    // Dump the contents of the log, if any.

    glGetShaderiv(handle, GL_COMPILE_STATUS,  &tst);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);

    if ((tst == 0) && (len > 1) && (log = new char[len + 1]))
    {
        glGetShaderInfoLog(handle, len, NULL, log);

        fprintf(stderr, "%s\n%s", name.c_str(), log);

        delete [] log;

        return true;
    }
    return false;
}

GLuint ogl::program::compile(GLenum type, const std::string& name,
                                          const std::string& text)
{
    GLint handle = 0;

    // Compile the given shader text.

    if (!text.empty())
    {
        handle = glCreateShader(type);

        const char *data =       text.data();
        GLint       size = GLint(text.size());

        glShaderSource (handle, 1, &data, &size);
        glCompileShader(handle);

        bindable = !shader_log(handle, name);
    }
    return handle;
}

//-----------------------------------------------------------------------------

const ogl::program *ogl::program::current = NULL;

ogl::program::program(std::string name) :
    name(name), vert(0), frag(0), prog(0), bindable(false)
{
    init();
}

ogl::program::~program()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::program::prep() const
{
    uniform_map::const_iterator i;

    if (uniforms.empty() == false)
    {
        bind();

        for (i = uniforms.begin(); i != uniforms.end(); ++i)
            i->first->apply(i->second);

        free();
    }
}

void ogl::program::bind() const
{
    if (bindable)
    {
        glUseProgram(prog);
        current = this;
    }
}

void ogl::program::free() const
{
}

//-----------------------------------------------------------------------------

std::string ogl::program::load(const std::string& name)
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

void ogl::program::init_attributes(app::node p)
{
    // Bind the attributes.

    for (app::node n = p.find("attribute"); n; n = p.next(n, "attribute"))

        glBindAttribLocation(prog, n.get_i("location"),
                                      n.get_s("name").c_str());
}

void ogl::program::init_samplers(app::node p)
{
    // Set the samplers.

    for (app::node n = p.find("sampler"); n; n = p.next(n, "sampler"))
    {
        const std::string name = n.get_s("name");
        const int         unit = n.get_i("unit");

        if (!name.empty())
        {
            uniform(name, unit);
            samplers[name] = GL_TEXTURE0 + unit;
        }
    }
}

void ogl::program::init_uniforms(app::node p)
{
    // Get the uniforms.

    for (app::node n = p.find("uniform"); n; n = p.next(n, "uniform"))
    {
        const std::string name  = n.get_s("name");
        const std::string value = n.get_s("value");
        const int         size  = n.get_i("size");

        if (!value.empty())
        {
            if (ogl::uniform *u = ::glob->load_uniform(value, size))
                uniforms[u] = glGetUniformLocation(prog, name.c_str());
        }
    }
}

//-----------------------------------------------------------------------------

void ogl::program::init()
{
    std::string path = "program/" + name;

    app::file file(path);

    if (app::node root = file.get_root().find("program"))
    {
        const std::string vert_name = root.get_s("vert");
        const std::string frag_name = root.get_s("frag");

        // Load the shader files.

        const std::string vert_text = load(vert_name);
        const std::string frag_text = load(frag_name);

        // Compile the shaders.

        vert = compile(GL_VERTEX_SHADER,   vert_name, vert_text);
        frag = compile(GL_FRAGMENT_SHADER, frag_name, frag_text);

        // Link the shader objects to a program object.

        prog = glCreateProgram();

        if (vert) glAttachShader(prog, vert);
        if (frag) glAttachShader(prog, frag);

        // Link the program.

        init_attributes(root);
        glLinkProgram(prog);

        bindable = !program_log(prog, path);

        // Configure the program.

        if (bindable)
        {
            bind();
            {
                init_samplers(root);
                init_uniforms(root);
            }
            free();
            prep();
        }
    }
}

void ogl::program::fini()
{
    uniform_map::iterator i;

    for (i = uniforms.begin(); i != uniforms.end(); ++i)
        ::glob->free_uniform(i->first);

    uniforms.clear();
    samplers.clear();

    if (prog) glDeleteProgram(prog);
    if (vert) glDeleteShader(vert);
    if (frag) glDeleteShader(frag);

    prog = 0;
    vert = 0;
    frag = 0;
}

//-----------------------------------------------------------------------------

GLenum ogl::program::unit(std::string name) const
{
    // Determine the texture unit of the named sampler uniform.

    std::map<std::string, GLenum>::const_iterator i;

    if ((i = samplers.find(name)) == samplers.end())
        return 0;
    else
        return i->second;
}

//-----------------------------------------------------------------------------

void ogl::program::uniform(std::string name, int d) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
            glUniform1i(loc, d);
    }
}

void ogl::program::uniform(std::string name, double a) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
            glUniform1f(loc, GLfloat(a));
    }
}

void ogl::program::uniform(std::string name, double a,
                                             double b) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
            glUniform2f(loc, GLfloat(a),
                             GLfloat(b));
    }
}

void ogl::program::uniform(std::string name, double a,
                                             double b,
                                             double c) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
            glUniform3f(loc, GLfloat(a),
                             GLfloat(b),
                             GLfloat(c));
    }
}

void ogl::program::uniform(std::string name, double a,
                                             double b,
                                             double c,
                                             double d) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
            glUniform4f(loc, GLfloat(a),
                             GLfloat(b),
                             GLfloat(c),
                             GLfloat(d));
    }
}

void ogl::program::uniform(std::string name, const double *M, bool t) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
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

            glUniformMatrix4fv(loc, 1, t, T);
        }
    }
}

void ogl::program::uniform(std::string name, const mat3& M, bool t) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
        {
            GLfloat T[9];

            T[0] = GLfloat(M[0][0]);
            T[1] = GLfloat(M[1][0]);
            T[2] = GLfloat(M[2][0]);
            T[3] = GLfloat(M[0][1]);
            T[4] = GLfloat(M[1][1]);
            T[5] = GLfloat(M[2][1]);
            T[6] = GLfloat(M[0][2]);
            T[7] = GLfloat(M[1][2]);
            T[8] = GLfloat(M[2][2]);

            glUniformMatrix3fv(loc, 1, t, T);
        }
    }
}

void ogl::program::uniform(std::string name, const mat4& M, bool t) const
{
    if (bindable)
    {
        int loc;

        if ((loc = glGetUniformLocation(prog, name.c_str())) >= 0)
        {
            GLfloat T[16];

            T[ 0] = GLfloat(M[0][0]);
            T[ 1] = GLfloat(M[1][0]);
            T[ 2] = GLfloat(M[2][0]);
            T[ 3] = GLfloat(M[3][0]);
            T[ 4] = GLfloat(M[0][1]);
            T[ 5] = GLfloat(M[1][1]);
            T[ 6] = GLfloat(M[2][1]);
            T[ 7] = GLfloat(M[3][1]);
            T[ 8] = GLfloat(M[0][2]);
            T[ 9] = GLfloat(M[1][2]);
            T[10] = GLfloat(M[2][2]);
            T[11] = GLfloat(M[3][2]);
            T[12] = GLfloat(M[0][3]);
            T[13] = GLfloat(M[1][3]);
            T[14] = GLfloat(M[2][3]);
            T[15] = GLfloat(M[3][3]);

            glUniformMatrix4fv(loc, 1, t, T);
        }
    }
}

//-----------------------------------------------------------------------------
