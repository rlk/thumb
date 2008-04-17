#include <stdexcept>

#include "ogl-frame.hpp"

//-----------------------------------------------------------------------------

std::vector<GLuint> ogl::frame::stack;

void ogl::frame::push(GLuint o)
{
    stack.push_back(o);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stack.back());
}

void ogl::frame::pop()
{
    stack.pop_back();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stack.back());
}

//-----------------------------------------------------------------------------

ogl::frame::frame(GLsizei w, GLsizei h, GLenum t, GLenum f, bool d, bool s) :
    target(t),
    format(f),
    buffer(0),
    color(0),
    depth(0),
    has_depth(d),
    has_stencil(s),
    w(w),
    h(h)
{
    if (stack.empty()) stack.push_back(0);

    init();
}

ogl::frame::~frame()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::frame::bind_color(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, color);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::frame::free_color(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::frame::bind_depth(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, depth);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::frame::free_depth(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------

void ogl::frame::bind(bool proj) const
{
    // Store the current viewport state.

    glPushAttrib(GL_VIEWPORT_BIT);

    // Enable this framebuffer object's state.

    push(buffer);

    glViewport(0, 0, w, h);

    // Set up a one-to-one model-view-projection transformation, if requested.

    if (proj)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, w, 0, h, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    OGLCK();
}

void ogl::frame::free(bool proj) const
{
    // Restore the previous transformation, if requested.

    if (proj)
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    // Restore the previous viewport and bind the default frame buffer.

    glPopAttrib();

    pop();

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::frame::draw() const
{
    bind_color(GL_TEXTURE0);
    {

        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glRecti(-1, -1, +1, +1);

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    free_color(GL_TEXTURE0);
}

void ogl::frame::init()
{
     // Initialize the color render buffer object.

    if (format)
    {
        glGenTextures(1,     &color);
        glBindTexture(target, color);

        glTexImage2D(target, 0, format, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

        glBindTexture(target, 0);
    }

    // Initialize the depth and stencil render buffer objects.

    if (has_depth)
    {
        glGenTextures(1,     &depth);
        glBindTexture(target, depth);

#ifdef GL_DEPTH_STENCIL_EXT
        if (has_stencil)
            glTexImage2D(target, 0, GL_DEPTH24_STENCIL8_EXT, w, h, 0,
                         GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL);
        else
#endif
            glTexImage2D(target, 0, GL_DEPTH_COMPONENT24,    w, h, 0,
                         GL_DEPTH_COMPONENT,   GL_UNSIGNED_BYTE,         NULL);

        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB,
                                GL_COMPARE_R_TO_TEXTURE_ARB);

        glBindTexture(target, 0);
    }

    // Initialize the frame buffer object.

    glGenFramebuffersEXT(1, &buffer);
    push(buffer);

    if (has_stencil)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_STENCIL_ATTACHMENT_EXT,
                                  target, depth, 0);
    if (has_depth)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_DEPTH_ATTACHMENT_EXT,
                                  target, depth, 0);
    if (color)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_COLOR_ATTACHMENT0_EXT,
                                  target, color, 0);
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

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

    pop();

    OGLCK();
}

void ogl::frame::fini()
{
    if (color)  glDeleteTextures(1, &color);
    if (depth)  glDeleteTextures(1, &depth);

    if (buffer) glDeleteFramebuffersEXT(1, &buffer);
}

//-----------------------------------------------------------------------------

