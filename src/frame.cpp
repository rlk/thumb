#include <stdexcept>

#include "frame.hpp"

//-----------------------------------------------------------------------------

ogl::frame::frame(GLsizei w, GLsizei h, GLenum t, GLenum cf, GLenum df) :
    target(t),
    buffer(0),
    color(0),
    depth(0),
    color_format(cf),
    depth_format(df),
    w(w),
    h(h)
{
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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer);
    glViewport(0, 0, w, h);

    OGLCK();
}

void ogl::frame::free(bool proj) const
{
    // Restore the previous viewport and bind the default frame buffer.

    glPopAttrib();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::frame::init()
{
     // Initialize the color render buffer object.

    if (color_format)
    {
        glGenTextures(1,     &color);
        glBindTexture(target, color);

        glTexImage2D(target, 0, color_format, w, h, 0,
                     color_format, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

        glBindTexture(target, 0);
    }

    // Initialize the depth render buffer object.

    if (depth_format)
    {
        glGenTextures(1,     &depth);
        glBindTexture(target, depth);

        glTexImage2D(target, 0, color_format, w, h, 0,
                     color_format, GL_UNSIGNED_BYTE, NULL);

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
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer);

    if (color) glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                         GL_COLOR_ATTACHMENT0_EXT,
                                         target, color, 0);
    if (depth) glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                         GL_DEPTH_ATTACHMENT_EXT,
                                         target, depth, 0);

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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    OGLCK();
}

void ogl::frame::fini()
{
    if (color)  glDeleteTextures(1, &color);
    if (depth)  glDeleteTextures(1, &depth);

    if (buffer) glDeleteFramebuffersEXT(1, &buffer);
}

//-----------------------------------------------------------------------------

