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

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>

#include "util.hpp"
#include "opengl.hpp"

//-----------------------------------------------------------------------------

#ifdef __APPLE__
#define PROC(t, n) { }
#else
#define PROC(t, n) if (!(n = (t) glGetProcAddress(#n))) std::runtime_error(#n)

// GL_ARB_multitexture

PFNGLACTIVETEXTUREARBPROC          glActiveTextureARB;

// GL_ARB_shader_objects

PFNGLGETOBJECTPARAMETERIVARBPROC   glGetObjectParameterivARB;
PFNGLCREATEPROGRAMOBJECTARBPROC    glCreateProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC     glCreateShaderObjectARB;
PFNGLUSEPROGRAMOBJECTARBPROC       glUseProgramObjectARB;
PFNGLVALIDATEPROGRAMARBPROC        glValidateProgramARB;
PFNGLCOMPILESHADERARBPROC          glCompileShaderARB;
PFNGLDELETEOBJECTARBPROC           glDeleteObjectARB;
PFNGLATTACHOBJECTARBPROC           glAttachObjectARB;
PFNGLSHADERSOURCEARBPROC           glShaderSourceARB;
PFNGLLINKPROGRAMARBPROC            glLinkProgramARB;
PFNGLGETINFOLOGARBPROC             glGetInfoLogARB;

PFNGLGETUNIFORMLOCATIONARBPROC     glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC              glUniform1iARB;
PFNGLUNIFORM1FARBPROC              glUniform1fARB;
PFNGLUNIFORM2FARBPROC              glUniform2fARB;
PFNGLUNIFORM3FARBPROC              glUniform3fARB;
PFNGLUNIFORM4FARBPROC              glUniform4fARB;

PFNGLBINDATTRIBLOCATIONARBPROC     glBindAttribLocationARB;

// GL_EXT_framebuffer_object

PFNGLGENFRAMEBUFFERSEXTPROC        glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC        glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC     glDeleteFramebuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC   glFramebufferTexture2DEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;

// GL_ARB_vertex_buffer_object

PFNGLGENBUFFERSARBPROC             glGenBuffersARB;
PFNGLBINDBUFFERARBPROC             glBindBufferARB;
PFNGLMAPBUFFERARBPROC              glMapBufferARB;
PFNGLBUFFERDATAARBPROC             glBufferDataARB;
PFNGLUNMAPBUFFERARBPROC            glUnmapBufferARB;
PFNGLDELETEBUFFERSARBPROC          glDeleteBuffersARB;

#endif

//-----------------------------------------------------------------------------

void ogl::check_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    // Search for the given string in the OpenGL extension strings.

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return;
    }

    throw std::runtime_error(needle);
}

void ogl::check_err(const char *file, int line)
{
    GLenum err;

    if ((err = glGetError()) != GL_NO_ERROR)
    {
        std::ostringstream str;

        str << file << ":"
            << line << " ";

        switch (err)
        {
        case GL_INVALID_ENUM:      str << "Invalid enumerant"; break;
        case GL_INVALID_VALUE:     str << "Invalid value";     break;
        case GL_INVALID_OPERATION: str << "Invalid operation"; break;
        case GL_STACK_OVERFLOW:    str << "Stack overflow";    break;
        case GL_OUT_OF_MEMORY:     str << "Out of memory";     break;
        case GL_TABLE_TOO_LARGE:   str << "Table too large";   break;
        }

        str << std::endl;

        throw std::runtime_error(str.str().c_str());
    }
}

//-----------------------------------------------------------------------------

void ogl::init()
{
    check_ext("ARB_multitexture");
    check_ext("ARB_shader_objects");
    check_ext("ARB_vertex_shader");
    check_ext("ARB_fragment_shader");
    check_ext("EXT_framebuffer_object");
    check_ext("ARB_vertex_buffer_object");

#ifndef __APPLE__

    // GL_ARB_multitexture

    PROC(PFNGLACTIVETEXTUREARBPROC,          glActiveTextureARB);

    // GL_ARB_shader_objects

    PROC(PFNGLGETOBJECTPARAMETERIVARBPROC,   glGetObjectParameterivARB);
    PROC(PFNGLCREATEPROGRAMOBJECTARBPROC,    glCreateProgramObjectARB);
    PROC(PFNGLCREATESHADEROBJECTARBPROC,     glCreateShaderObjectARB);
    PROC(PFNGLUSEPROGRAMOBJECTARBPROC,       glUseProgramObjectARB);
    PROC(PFNGLVALIDATEPROGRAMARBPROC,        glValidateProgramARB);
    PROC(PFNGLSHADERSOURCEARBPROC,           glShaderSourceARB);
    PROC(PFNGLCOMPILESHADERARBPROC,          glCompileShaderARB);
    PROC(PFNGLATTACHOBJECTARBPROC,           glAttachObjectARB);
    PROC(PFNGLLINKPROGRAMARBPROC,            glLinkProgramARB);
    PROC(PFNGLGETINFOLOGARBPROC,             glGetInfoLogARB);
    PROC(PFNGLDELETEOBJECTARBPROC,           glDeleteObjectARB);

    PROC(PFNGLGETUNIFORMLOCATIONARBPROC,     glGetUniformLocationARB);
    PROC(PFNGLUNIFORM1IARBPROC,              glUniform1iARB);
    PROC(PFNGLUNIFORM1FARBPROC,              glUniform1fARB);
    PROC(PFNGLUNIFORM2FARBPROC,              glUniform2fARB);
    PROC(PFNGLUNIFORM3FARBPROC,              glUniform3fARB);
    PROC(PFNGLUNIFORM4FARBPROC,              glUniform4fARB);

    // GL_ARB_vertex_shader

    PROC(PFNGLBINDATTRIBLOCATIONARBPROC,     glBindAttribLocationARB);

    // GL_EXT_framebuffer_object

    PROC(PFNGLGENFRAMEBUFFERSEXTPROC,        glGenFramebuffersEXT);
    PROC(PFNGLBINDFRAMEBUFFEREXTPROC,        glBindFramebufferEXT);
    PROC(PFNGLDELETEFRAMEBUFFERSEXTPROC,     glDeleteFramebuffersEXT);
    PROC(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,   glFramebufferTexture2DEXT);
    PROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);

    // GL_ARB_vertex_buffer_object

    PROC(PFNGLGENBUFFERSARBPROC,             glGenBuffersARB);
    PROC(PFNGLBINDBUFFERARBPROC,             glBindBufferARB);
    PROC(PFNGLMAPBUFFERARBPROC,              glMapBufferARB);
    PROC(PFNGLBUFFERDATAARBPROC,             glBufferDataARB);
    PROC(PFNGLUNMAPBUFFERARBPROC,            glUnmapBufferARB);
    PROC(PFNGLDELETEBUFFERSARBPROC,          glDeleteBuffersARB);
#endif
}

//-----------------------------------------------------------------------------

ogl::fbo::fbo(GLint color_format,
              GLint depth_format, GLsizei w, GLsizei h) : w(w), h(h)
{
    GLint  o[1], v[4];
    GLenum T;

    if (((w & (w - 1)) == 0) && ((h & (w - 1)) == 0))
        T = GL_TEXTURE_2D;
    else
        T = GL_TEXTURE_RECTANGLE_ARB;

    get_framebuffer(o, v);

    frame = 0;
    color = 0;
    depth = 0;

    // Initialize the color render buffer object.

    glGenTextures(1, &color);
    glBindTexture(T,  color);

    glTexImage2D(T, 0, color_format, w, h, 0, GL_RGBA, GL_INT, NULL);

    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    // Initialize the depth render buffer object.

    glGenTextures(1, &depth);
    glBindTexture(T,  depth);

    glTexImage2D(T, 0, depth_format, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_INT, NULL);

    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexParameteri(T, GL_TEXTURE_COMPARE_MODE_ARB,
                       GL_COMPARE_R_TO_TEXTURE_ARB);

    // Initialize the frame buffer object.

    glGenFramebuffersEXT(1, &frame);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT, T, color, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_DEPTH_ATTACHMENT_EXT,  T, depth, 0);

    // Confirm the frame buffer object status.

    switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        break; 
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer incomplete attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer missing attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer duplicate attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        throw std::runtime_error("Framebuffer dimensions");
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        throw std::runtime_error("Framebuffer formats");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        throw std::runtime_error("Framebuffer draw buffer");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        throw std::runtime_error("Framebuffer read buffer");
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        throw std::runtime_error("Framebuffer unsupported");
    default:
        throw std::runtime_error("Framebuffer error");
    }

    // Zero the buffer.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    set_framebuffer(o, v);

    OGLCK();
}

ogl::fbo::~fbo()
{
    glDeleteFramebuffersEXT(1, &frame);
    glDeleteTextures       (1, &depth);
    glDeleteTextures       (1, &color);

    OGLCK();
}

void ogl::fbo::push_frame(bool inset)
{
    glGetIntegerv(GL_VIEWPORT,                vp_cache);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, fb_cache);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);

    if (inset)
        glViewport(1, 1, w - 1, h - 1);
    else
        glViewport(0, 0, w, h);

    OGLCK();
}

void ogl::fbo::pop_frame()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb_cache[0]);
    glViewport(vp_cache[0], vp_cache[1],
               vp_cache[2], vp_cache[3]);

    OGLCK();
}

void ogl::fbo::bind_color(GLenum T) const
{
    glActiveTextureARB(T);
    {
        glBindTexture(GL_TEXTURE_2D, color);
    }
    glActiveTextureARB(GL_TEXTURE0);

    OGLCK();
}

void ogl::fbo::bind_depth(GLenum T) const
{
    glActiveTextureARB(T);
    {
        glBindTexture(GL_TEXTURE_2D, depth);
    }
    glActiveTextureARB(GL_TEXTURE0);

    OGLCK();
}

//-----------------------------------------------------------------------------

ogl::vbo::vbo(GLenum target, GLenum usage, GLsizei size)
{
    glGenBuffersARB(1, &buffer);

    glBindBufferARB(target, buffer);
    glBufferDataARB(target, size, NULL, usage);

    glBindBufferARB(target, 0);

    OGLCK();
}

ogl::vbo::~vbo()
{
    glDeleteBuffersARB(1, &buffer);
    OGLCK();
}

void ogl::vbo::bind(GLenum target) const
{
    glBindBufferARB(target, buffer);
    OGLCK();
}

//-----------------------------------------------------------------------------
/*
void ogl::shader::check_log(GLhandleARB handle)
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

ogl::shader::shader(std::string vert_str, std::string frag_str)
{
    prog = glCreateProgramObjectARB();
    vert = 0;
    frag = 0;

    // Compile the vertex shader.

    if (vert_str.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) vert_str.c_str();

        vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        glShaderSourceARB (vert, 1, &p, 0);
        glCompileShaderARB(vert);
        
        check_log(vert);
    }

    // Compile the frag shader.

    if (frag_str.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) frag_str.c_str();

        frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        glShaderSourceARB (frag, 1, &p, 0);
        glCompileShaderARB(frag);
        
        check_log(frag);
    }

    // Link these shader objects to a program object.

    glAttachObjectARB(prog, vert);
    glAttachObjectARB(prog, frag);

    glLinkProgramARB(prog); check_log(prog);

    OGLCK();
}

ogl::shader::~shader()
{
    glDeleteObjectARB(prog);
    glDeleteObjectARB(vert);
    glDeleteObjectARB(frag);
    OGLCK();
}

void ogl::shader::bind() const
{
    glUseProgramObjectARB(prog);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::shader::uniform(std::string name, int d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1iARB(loc, d);
}

void ogl::shader::uniform(std::string name, float a) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1fARB(loc, a);
}

void ogl::shader::uniform(std::string name, float a, float b) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform2fARB(loc, a, b);
}

void ogl::shader::uniform(std::string name, float a, float b, float c) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform3fARB(loc, a, b, c);
}

void ogl::shader::uniform(std::string name, float a, float b,
                                            float c, float d) const
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform4fARB(loc, a, b, c, d);
}
*/
//-----------------------------------------------------------------------------

void ogl::get_framebuffer(GLint o[1], GLint v[4])
{
    glGetIntegerv(GL_VIEWPORT,                v);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, o);
}

void ogl::set_framebuffer(GLint o[1], GLint v[4])
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, o[0]);
    glViewport(v[0], v[1], v[2], v[3]);
}

//-----------------------------------------------------------------------------

void ogl::draw_disc(int a)
{
    glBegin(GL_POLYGON);
    {
        for (int i = 0; i < 360; i += a)
            glVertex2f(cosi(i), sini(i));
    }
    glEnd();
}

void ogl::draw_cube()
{
    static const GLfloat point[8][3] = {
        { -1.0f, -1.0f, -1.0f },
        { +1.0f, -1.0f, -1.0f },
        { -1.0f, +1.0f, -1.0f },
        { +1.0f, +1.0f, -1.0f },
        { -1.0f, -1.0f, +1.0f },
        { +1.0f, -1.0f, +1.0f },
        { -1.0f, +1.0f, +1.0f },
        { +1.0f, +1.0f, +1.0f },
    };

    static const GLuint index[6][4] = {
        { 0, 4, 6, 2 },
        { 5, 1, 3, 7 },
        { 0, 1, 5, 4 },
        { 6, 7, 3, 2 },
        { 1, 0, 2, 3 },
        { 4, 5, 7, 6 },
    };

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_FLOAT, 0, point);
        glEnableClientState(GL_VERTEX_ARRAY);

        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, index);
    }
    glPopClientAttrib();
}

void ogl::draw_axes()
{
    glBegin(GL_LINES);
    {
        float k = 16.0f;

        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glVertex3f(-k, 0.0f, 0.0f);
        glVertex3f(+k, 0.0f, 0.0f);

        glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        glVertex3f(0.0f, -k, 0.0f);
        glVertex3f(0.0f, +k, 0.0f);

        glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, -k);
        glVertex3f(0.0f, 0.0f, +k);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
