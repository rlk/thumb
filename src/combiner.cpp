/*    Copyright (C) 2006 Robert Kooima                                       */
/*                                                                           */
/*    Varrier Combiner is free software;  you can  redistribute it and/or    */
/*    modify  it under  the terms  of the  GNU General Public License  as    */
/*    published by the Free Software Foundation;  either version 2 of the    */
/*    License, or (at your option) any later version                         */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "combiner.hpp"
#include "matrix.hpp"
#include "opengl.hpp"

/*===========================================================================*/
#ifdef SNIP

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>
#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else
static PFNGLACTIVETEXTUREARBPROC        glActiveTextureARB;
static PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgramObjectARB;
static PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgramObjectARB;
static PFNGLCREATESHADEROBJECTARBPROC   glCreateShaderObjectARB;
static PFNGLSHADERSOURCEARBPROC         glShaderSourceARB;
static PFNGLCOMPILESHADERARBPROC        glCompileShaderARB;
static PFNGLATTACHOBJECTARBPROC         glAttachObjectARB;
static PFNGLLINKPROGRAMARBPROC          glLinkProgramARB;
static PFNGLDELETEOBJECTARBPROC         glDeleteObjectARB;
static PFNGLGETINFOLOGARBPROC           glGetInfoLogARB;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
static PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocationARB;
static PFNGLUNIFORM1IARBPROC            glUniform1iARB;
static PFNGLUNIFORM1FARBPROC            glUniform1fARB;
static PFNGLUNIFORM2FARBPROC            glUniform2fARB;
static PFNGLUNIFORM3FARBPROC            glUniform3fARB;
static PFNGLGENFRAMEBUFFERSEXTPROC      glGenFramebuffersEXT;
static PFNGLBINDFRAMEBUFFEREXTPROC      glBindFramebufferEXT;
static PFNGLDELETEFRAMEBUFFERSEXTPROC   glDeleteFramebuffersEXT;
static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
#endif

/*---------------------------------------------------------------------------*/

static void check_log(GLhandleARB handle)
{
    char *log;
    GLint len;

    /* Dump the contents of the log, if any. */

    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);

    if ((len > 1) && (log = (char *) malloc(len + 1)))
    {
        glGetInfoLogARB(handle, len, NULL, log);

        fprintf(stderr, log);
        free(log);
    }
}

static int check_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    /* Search for the given string in the OpenGL extension strings. */

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return 1;
    }

    fprintf(stderr, "Missing OpenGL extension %s\n", needle);
    return 0;
}

/*---------------------------------------------------------------------------*/

#define init_proc(t, n) if (!(n = (t) glGetProcAddress(#n))) return 0;

static int init_ogl(void)
{
    /* Confirm support for necessary OpenGL extensions. */

    if (check_ext("ARB_vertex_shader")   &&
        check_ext("ARB_shader_objects")  &&
        check_ext("ARB_fragment_shader") &&
        check_ext("EXT_framebuffer_object"))
    {
        /* Initialize extension entry points. */

#ifndef __APPLE__
        init_proc(PFNGLACTIVETEXTUREARBPROC,        glActiveTextureARB);
        init_proc(PFNGLUSEPROGRAMOBJECTARBPROC,     glUseProgramObjectARB);
        init_proc(PFNGLCREATESHADEROBJECTARBPROC,   glCreateShaderObjectARB);
        init_proc(PFNGLCREATEPROGRAMOBJECTARBPROC,  glCreateProgramObjectARB);
        init_proc(PFNGLSHADERSOURCEARBPROC,         glShaderSourceARB);
        init_proc(PFNGLCOMPILESHADERARBPROC,        glCompileShaderARB);
        init_proc(PFNGLATTACHOBJECTARBPROC,         glAttachObjectARB);
        init_proc(PFNGLLINKPROGRAMARBPROC,          glLinkProgramARB);
        init_proc(PFNGLDELETEOBJECTARBPROC,         glDeleteObjectARB);
        init_proc(PFNGLGETINFOLOGARBPROC,           glGetInfoLogARB);
        init_proc(PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameterivARB);
        init_proc(PFNGLGETUNIFORMLOCATIONARBPROC,   glGetUniformLocationARB);
        init_proc(PFNGLUNIFORM1IARBPROC,            glUniform1iARB);
        init_proc(PFNGLUNIFORM1FARBPROC,            glUniform1fARB);
        init_proc(PFNGLUNIFORM2FARBPROC,            glUniform2fARB);
        init_proc(PFNGLUNIFORM3FARBPROC,            glUniform3fARB);
        init_proc(PFNGLGENFRAMEBUFFERSEXTPROC,      glGenFramebuffersEXT);
        init_proc(PFNGLDELETEFRAMEBUFFERSEXTPROC,   glDeleteFramebuffersEXT);
        init_proc(PFNGLBINDFRAMEBUFFEREXTPROC,      glBindFramebufferEXT);
        init_proc(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC, glFramebufferTexture2DEXT);
#endif
        return 1;
    }
    return 0;
}

/*===========================================================================*/

static void init_buffers(GLuint *frame,
                         GLuint *color,
                         GLuint *depth, int w, int h)
{
    GLenum T = GL_TEXTURE_RECTANGLE_ARB;

    /* Generate frame buffer and render buffer objects. */

    glGenFramebuffersEXT(1, frame);
    glGenTextures       (1, color);
    glGenTextures       (1, depth);

    /* Initialize the color render buffer. */

    glBindTexture(T, *color);
    glTexImage2D (T, 0, GL_RGBA8, w, h, 0,
                  GL_RGBA, GL_INT, NULL);

    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Initialize the depth render buffer. */

    glBindTexture(T, *depth);
    glTexImage2D (T, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                  GL_DEPTH_COMPONENT, GL_INT, NULL);

    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Initialize the frame buffer object. */

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *frame);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT, T, *color, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_DEPTH_ATTACHMENT_EXT,  T, *depth, 0);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB,  0);
}

static void init_program(GLhandleARB *prog,
                         GLhandleARB *vert,
                         GLhandleARB *frag,
                         const char *vert_txt, const char *frag_txt)
{
    *vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    *frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    *prog = glCreateProgramObjectARB();

    /* Compile the given shader source. */

    glShaderSourceARB(*vert, 1, &vert_txt, NULL);
    glShaderSourceARB(*frag, 1, &frag_txt, NULL);

    glCompileShaderARB(*vert);
    check_log(*vert);

    glCompileShaderARB(*frag);
    check_log(*frag);

    /* Link these shader objects to a program object. */

    glAttachObjectARB(*prog, *vert);
    glAttachObjectARB(*prog, *frag);

    glLinkProgramARB(*prog);
    check_log(*prog);
}

/*===========================================================================*/

static char *vert_txt = \

    "uniform vec3 offset;                                              \n"\
    "varying vec3 L_phase;                                             \n"\
    "varying vec3 R_phase;                                             \n"\

    "void main()                                                       \n"\
    "{                                                                 \n"\
    "    vec4 dr = vec4(offset.r, 0.0, 0.0, 0.0);                      \n"\
    "    vec4 dg = vec4(offset.g, 0.0, 0.0, 0.0);                      \n"\
    "    vec4 db = vec4(offset.b, 0.0, 0.0, 0.0);                      \n"\

    "    L_phase.r = (gl_TextureMatrix[0] * (gl_Vertex + dr)).x;       \n"\
    "    R_phase.r = (gl_TextureMatrix[1] * (gl_Vertex + dr)).x;       \n"\

    "    L_phase.g = (gl_TextureMatrix[0] * (gl_Vertex + dg)).x;       \n"\
    "    R_phase.g = (gl_TextureMatrix[1] * (gl_Vertex + dg)).x;       \n"\

    "    L_phase.b = (gl_TextureMatrix[0] * (gl_Vertex + db)).x;       \n"\
    "    R_phase.b = (gl_TextureMatrix[1] * (gl_Vertex + db)).x;       \n"\

    "    gl_Position = ftransform();                                   \n"\
    "}                                                                 \n";

static char *frag_txt = \

    "uniform samplerRect L_map;                                        \n"\
    "uniform samplerRect R_map;                                        \n"\
    "uniform float       cycle;                                        \n"\
    "uniform float       quality;                                      \n"\

    "varying vec3 L_phase;                                             \n"\
    "varying vec3 R_phase;                                             \n"\

    "void main()                                                       \n"\
    "{                                                                 \n"\
    "    const vec4 L = textureRect(L_map, gl_FragCoord.xy * quality); \n"\
    "    const vec4 R = textureRect(R_map, gl_FragCoord.xy * quality); \n"\

    "    vec3 Lk = step(vec3(cycle), fract(L_phase));                  \n"\
    "    vec3 Rk = step(vec3(cycle), fract(R_phase));                  \n"\

    "    gl_FragColor = vec4(max(L.rgb * Lk, R.rgb * Rk), 1.0);        \n"\
    "}                                                                 \n";

static GLuint frame_buf[2] = { 0, 0 };
static GLuint color_buf[2] = { 0, 0 };
static GLuint depth_buf[2] = { 0, 0 };

static GLuint vert_obj = 0;
static GLuint frag_obj = 0;
static GLuint prog_obj = 0;

/*---------------------------------------------------------------------------*/

int vc_init(int w, int h)
{
    if (1) //init_ogl())
    {
        init_buffers(frame_buf + 0, color_buf + 0, depth_buf + 0, w, h);
        init_buffers(frame_buf + 1, color_buf + 1, depth_buf + 1, w, h);

        init_program(&prog_obj, &vert_obj, &frag_obj, vert_txt, frag_txt);

        return 1;
    }
    return 0;
}

void vc_fini(void)
{
    if (prog_obj)  glDeleteObjectARB(prog_obj);
    if (vert_obj)  glDeleteObjectARB(vert_obj);
    if (frag_obj)  glDeleteObjectARB(frag_obj);

    if (color_buf) glDeleteTextures(2, color_buf);
    if (depth_buf) glDeleteTextures(2, depth_buf);
    if (frame_buf) glDeleteTextures(2, frame_buf);
}

/*---------------------------------------------------------------------------*/

static void normalize(float v[3])
{
    float k = 1.0f / (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    v[0] *= k;
    v[1] *= k;
    v[2] *= k;
}

static void basis(const struct vc_display *V, float R[3],
                                              float U[3],
                                              float N[3])
{
    /* Find the basis of the screen space. */

    R[0] = V->screen_BR[0] - V->screen_BL[0];
    R[1] = V->screen_BR[1] - V->screen_BL[1];
    R[2] = V->screen_BR[2] - V->screen_BL[2];

    U[0] = V->screen_TL[0] - V->screen_BL[0];
    U[1] = V->screen_TL[1] - V->screen_BL[1];
    U[2] = V->screen_TL[2] - V->screen_BL[2];

    N[0] = R[1] * U[2] - R[2] * U[1];
    N[1] = R[2] * U[0] - R[0] * U[2];
    N[2] = R[0] * U[1] - R[1] * U[0];

    normalize(R);
    normalize(U);
    normalize(N);
}

/*---------------------------------------------------------------------------*/

static void draw_transform(const struct vc_display *V, const float P[3])
{
    float R[3];
    float U[3];
    float N[3];
    float v[3];
    float w[3];

    float dx, dy;
    float pp, ss;

    basis(V, R, U, N);

    /* Find the vector from the center of the screen to the eye. */

    v[0] = P[0] - (V->screen_TL[0] + V->screen_BR[0]) * 0.5f;
    v[1] = P[1] - (V->screen_TL[1] + V->screen_BR[1]) * 0.5f;
    v[2] = P[2] - (V->screen_TL[2] + V->screen_BR[2]) * 0.5f;

    /* Transform this vector into screen space. */

    w[0] = v[0] * R[0] + v[1] * R[1] + v[2] * R[2];
    w[1] = v[0] * U[0] + v[1] * U[1] + v[2] * U[2];
    w[2] = v[0] * N[0] + v[1] * N[1] + v[2] * N[2];

    /* Compute the parallax due to optical thickness. */

    dx = V->thick * w[0] / w[2];
    dy = V->thick * w[1] / w[2];

    /* Compute the pitch and shift reduction due to optical thickness. */

    pp = V->pitch * (w[2] - V->thick) / w[2];
    ss = V->shift * (w[2] - V->thick) / w[2];

    /* Compose the line screen transformation matrix. */

    glMatrixMode(GL_TEXTURE);
    {
        glPushMatrix();
        glLoadIdentity();

        glScalef(pp, pp, 1.0f);
        glRotatef(-V->angle, 0.0f, 0.0f, 1.0f);
        glTranslatef(dx - ss, dy, 0.0f);
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

void vc_prepare(const struct vc_display *V, int e)
{
    int x = (int) (V->viewport_x * V->quality);
    int y = (int) (V->viewport_y * V->quality);
    int w = (int) (V->viewport_w * V->quality);
    int h = (int) (V->viewport_h * V->quality);

    /* Scale and shift the rendered area to control quality. */

    glViewport(x, y, w, h);
    glScissor (x, y, w, h);

    /* Bind the selected eye buffer as render target. */

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf[e]);
}

void vc_combine(const struct vc_display *V, const float L[3], const float R[3])
{
    /* Compute the screen size and subpixel offset. */

    float w = (float) sqrt(pow(V->screen_BR[0] - V->screen_BL[0], 2.0) +
                           pow(V->screen_BR[1] - V->screen_BL[1], 2.0) +
                           pow(V->screen_BR[2] - V->screen_BL[2], 2.0));
    float h = (float) sqrt(pow(V->screen_TL[0] - V->screen_BL[0], 2.0) +
                           pow(V->screen_TL[1] - V->screen_BL[1], 2.0) +
                           pow(V->screen_TL[2] - V->screen_BL[2], 2.0));

    float d = (float) w / (3 * V->viewport_w);

    /* Bind the eye view off-screen buffers as textures. */

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glActiveTextureARB(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, color_buf[1]);

    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, color_buf[0]);

    /* Enable the Varrier combiner fragment shader. */

    glUseProgramObjectARB(prog_obj);

    glUniform1iARB(glGetUniformLocationARB(prog_obj, "L_map"),   0);
    glUniform1iARB(glGetUniformLocationARB(prog_obj, "R_map"),   1);
    glUniform1fARB(glGetUniformLocationARB(prog_obj, "cycle"),   V->cycle);
    glUniform1fARB(glGetUniformLocationARB(prog_obj, "quality"), V->quality);
    glUniform3fARB(glGetUniformLocationARB(prog_obj, "offset"), -d, 0, +d);

    /* Apply the linescreen transforms to the texture matrices. */

    glActiveTextureARB(GL_TEXTURE1);
    draw_transform(V, R);
    glActiveTextureARB(GL_TEXTURE0);
    draw_transform(V, L);

    /* Set up a transform mapping screen space to real space. */

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-w / 2, +w / 2, -h / 2, +h / 2, -1, +1);
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glPushMatrix();
        glLoadIdentity();
    }

    /* Draw a screen-filling quad. */

    glViewport(V->viewport_x, V->viewport_y, V->viewport_w, V->viewport_h);
    glScissor (V->viewport_x, V->viewport_y, V->viewport_w, V->viewport_h);

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glDepthMask(GL_FALSE);

        glBegin(GL_POLYGON);
        {
            glVertex2f(-w / 2, -h / 2);
            glVertex2f(+w / 2, -h / 2);
            glVertex2f(+w / 2, +h / 2);
            glVertex2f(-w / 2, +h / 2);
        }
        glEnd();
    }
    glPopAttrib();

    /* Revert the OpenGL state. */

    glMatrixMode(GL_TEXTURE);
    {
        glActiveTextureARB(GL_TEXTURE1);
        glPopMatrix();
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

        glActiveTextureARB(GL_TEXTURE0);
        glPopMatrix();
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    }
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glPopMatrix();
    }

    glUseProgramObjectARB(0);
}

/*---------------------------------------------------------------------------*/

static void basis(const struct vc_display *V, float R[3],
                                              float U[3],
                                              float N[3])
{
    /* Find the basis of the screen space. */

    R[0] = V->screen_BR[0] - V->screen_BL[0];
    R[1] = V->screen_BR[1] - V->screen_BL[1];
    R[2] = V->screen_BR[2] - V->screen_BL[2];

    U[0] = V->screen_TL[0] - V->screen_BL[0];
    U[1] = V->screen_TL[1] - V->screen_BL[1];
    U[2] = V->screen_TL[2] - V->screen_BL[2];

    N[0] = R[1] * U[2] - R[2] * U[1];
    N[1] = R[2] * U[0] - R[0] * U[2];
    N[2] = R[0] * U[1] - R[1] * U[0];

    normalize(R);
    normalize(U);
    normalize(N);
}

void vc_frustum(const struct vc_display *V, const float P[3], float n, float f)
{
    float R[3];
    float U[3];
    float N[3];
    float k;

    basis(V, R, U, N);

    k = N[0] * (V->screen_BL[0] - P[0]) + 
        N[1] * (V->screen_BL[1] - P[1]) +
        N[2] * (V->screen_BL[2] - P[2]);

    glMatrixMode(GL_PROJECTION);
    {
        float M[16];

        /* Compute the world-space edges of the view port. */

        double l = n * (R[0] * (P[0] - V->screen_BL[0]) +
                        R[1] * (P[1] - V->screen_BL[1]) +
                        R[2] * (P[2] - V->screen_BL[2])) / k;
        double r = n * (R[0] * (P[0] - V->screen_BR[0]) +
                        R[1] * (P[1] - V->screen_BR[1]) +
                        R[2] * (P[2] - V->screen_BR[2])) / k;
        double b = n * (U[0] * (P[0] - V->screen_BL[0]) +
                        U[1] * (P[1] - V->screen_BL[1]) +
                        U[2] * (P[2] - V->screen_BL[2])) / k;
        double t = n * (U[0] * (P[0] - V->screen_TL[0]) +
                        U[1] * (P[1] - V->screen_TL[1]) +
                        U[2] * (P[2] - V->screen_TL[2])) / k;

        glLoadIdentity();

        /* Apply the projection. */

        glFrustum(l, r, b, t, n, f);

        /* Account for the orientation of the display. */

        M[0] = R[0]; M[4] = R[1]; M[ 8] = R[2]; M[12] = 0.0f;
        M[1] = U[0]; M[5] = U[1]; M[ 9] = U[2]; M[13] = 0.0f;
        M[2] = N[0]; M[6] = N[1]; M[10] = N[2]; M[14] = 0.0f;
        M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;

        glMultMatrixf(M);

        /* Move the apex of the frustum to the origin. */

        glTranslatef(-P[0], -P[1], -P[2]);
    }
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
}
#endif
/*===========================================================================*/
