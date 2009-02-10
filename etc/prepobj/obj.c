/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    obj.[ch] is free software; you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define MAXSTR  1024
#define EPSILON 0.000976563

/*---------------------------------------------------------------------------*/

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

/*===========================================================================*/

#include "obj.h"

struct obj_prop
{
    char  *str;
    int    opt;
    GLuint map;

    float c[4];
    float o[3];
    float s[3];
};

struct obj_mtrl
{
    char *name;

    struct obj_prop kv[5];
};

struct obj_vert
{
    float u[3];
    float n[3];
    float t[2];
    float v[3];
};

struct obj_poly
{
    int vi[3];
};

struct obj_line
{
    int vi[2];
};

struct obj_surf
{
    int mi;

    int pc;
    int pm;
    int lc;
    int lm;

    GLuint pibo;
    GLuint libo;

    struct obj_poly *pv;
    struct obj_line *lv;
};

struct obj_file
{
    GLuint vbo;

    int mc;
    int mm;
    int vc;
    int vm;
    int sc;
    int sm;

    struct obj_mtrl *mv;
    struct obj_vert *vv;
    struct obj_surf *sv;
};

/*---------------------------------------------------------------------------*/
/* Global OBJ State                                                          */

static int fc;
static int fm;

static struct obj_file *fv;

static void invalidate(int);

/*---------------------------------------------------------------------------*/

#define file(i)       (fv + i)

#define surf(i, j)    (fv[i].sv + j)
#define vert(i, j)    (fv[i].vv + j)
#define mtrl(i, j)    (fv[i].mv + j)

#define poly(i, j, k) (fv[i].sv[j].pv + k)
#define line(i, j, k) (fv[i].sv[j].lv + k)
#define prop(i, j, k) (fv[i].mv[j].kv + k)

/*---------------------------------------------------------------------------*/

#define assert_file(i) \
      { assert(0 <= i && i < fc); }

#define assert_surf(i, j) \
      { assert_file(i); assert(0 <= j && j < file(i)->sc); }
#define assert_vert(i, j) \
      { assert_file(i); assert(0 <= j && j < file(i)->vc); }
#define assert_mtrl(i, j) \
      { assert_file(i); assert(0 <= j && j < file(i)->mc); }

#define assert_line(i, j, k) \
      { assert_surf(i, j); assert(0 <= k && k < surf(i, j)->lc); }
#define assert_poly(i, j, k) \
      { assert_surf(i, j); assert(0 <= k && k < surf(i, j)->pc); }
#define assert_prop(i, j, k) \
      { assert_mtrl(i, j); assert(0 <= k && k < 5); }

/*===========================================================================*/
/* OpenGL State                                                              */

#ifndef __APPLE__
static PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERARBPROC     glVertexAttribPointer;
static PFNGLGENBUFFERSARBPROC              glGenBuffers;
static PFNGLBINDBUFFERARBPROC              glBindBuffer;
static PFNGLBUFFERDATAARBPROC              glBufferData;
static PFNGLDELETEBUFFERSARBPROC           glDeleteBuffers;
static PFNGLACTIVETEXTUREARBPROC           glActiveTexture;
#endif

static GLint     GL_max_texture_image_units;
static GLboolean GL_has_vertex_buffer_object;
static GLboolean GL_has_multitexture;
static GLboolean GL_is_initialized;

/*---------------------------------------------------------------------------*/

static GLboolean gl_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return GL_TRUE;
    }
    return GL_FALSE;
}

static void obj_init_gl(void)
{
    if (GL_is_initialized == 0)
    {
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB,
                     &GL_max_texture_image_units);

        GL_has_vertex_buffer_object = gl_ext("GL_ARB_vertex_buffer_object");
        GL_has_multitexture         = gl_ext("GL_ARB_multitexture");

#ifndef __APPLE__
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)
                      glGetProcAddress("glEnableVertexAttribArrayARB");
        glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERARBPROC)
                      glGetProcAddress("glVertexAttribPointerARB");
        glGenBuffers              = (PFNGLGENBUFFERSARBPROC)
                      glGetProcAddress("glGenBuffersARB");
        glBindBuffer              = (PFNGLBINDBUFFERARBPROC)
                      glGetProcAddress("glBindBufferARB");
        glBufferData              = (PFNGLBUFFERDATAARBPROC)
                      glGetProcAddress("glBufferDataARB");
        glDeleteBuffers           = (PFNGLDELETEBUFFERSARBPROC)
                      glGetProcAddress("glDeleteBuffersARB");
        glActiveTexture           = (PFNGLACTIVETEXTUREARBPROC)
                      glGetProcAddress("glActiveTextureARB");
#endif
        GL_is_initialized = 1;
    }
}

/*===========================================================================*/
/* Vector cache                                                              */

struct vec2
{
    float v[2];
    int _ii;
};

struct vec3
{
    float v[3];
    int _ii;
};

struct iset
{
    int vi;
    int gi;

    int _vi;
    int _ti;
    int _ni;
    int _ii;
};

static int _vc, _vm;
static int _tc, _tm;
static int _nc, _nm;
static int _ic, _im;

static struct vec3 *_vv;
static struct vec2 *_tv;
static struct vec3 *_nv;
static struct iset *_iv;

/*---------------------------------------------------------------------------*/

static int add__(void **_v, int *_c, int *_m, size_t _s)
{
    int   m = (*_m > 0) ? *_m * 2 : 2;
    void *v;

    /* If space remains in the current block, return it. */

    if (*_m > *_c)
        return (*_c)++;

    /* Else, try to increase the size of the block. */

    else if ((v = realloc(*_v, _s * m)))
    {
        *_v = v;
        *_m = m;
        return (*_c)++;
    }

    /* Else, indicate failure. */

    else return -1;
}

static int add_v(void)
{
    return add__((void **) &_vv, &_vc, &_vm, sizeof (struct vec3));
}

static int add_t(void)
{
    return add__((void **) &_tv, &_tc, &_tm, sizeof (struct vec2));
}

static int add_n(void)
{
    return add__((void **) &_nv, &_nc, &_nm, sizeof (struct vec3));
}

static int add_i(void)
{
    return add__((void **) &_iv, &_ic, &_im, sizeof (struct iset));
}

/*===========================================================================*/
/* Handy functions                                                           */

static void cross(float z[3], const float x[3], const float y[3])
{
    float t[3];

    t[0] = x[1] * y[2] - x[2] * y[1];
    t[1] = x[2] * y[0] - x[0] * y[2];
    t[2] = x[0] * y[1] - x[1] * y[0];

    z[0] = t[0];
    z[1] = t[1];
    z[2] = t[2];
}

static void normalize(float v[3])
{
    float k = 1.0f / (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    v[0] *= k;
    v[1] *= k;
    v[2] *= k;
}

static void normal(float n[3], const float a[3],
                               const float b[3],
                               const float c[3])
{
    float u[3];
    float v[3];

    u[0] = b[0] - a[0];
    u[1] = b[1] - a[1];
    u[2] = b[2] - a[2];

    v[0] = c[0] - a[0];
    v[1] = c[1] - a[1];
    v[2] = c[2] - a[2];

    cross(n, u, v);
    normalize(n);
}

/*===========================================================================*/

#ifndef CONF_NO_PNG

#include <png.h>

static void *read_png(const char *filename, int *w, int *h, int *b)
{
    FILE       *filep = NULL;
    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    unsigned char *p = NULL;

    /* Initialize all PNG import data structures. */

    if (!(filep = fopen(filename, "rb")))
        return NULL;
    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return NULL;
    if (!(infop = png_create_info_struct(readp)))
        return NULL;

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        /* Read the PNG header. */

        png_init_io (readp, filep);
        png_read_png(readp, infop, PNG_TRANSFORM_STRIP_16 |
                                   PNG_TRANSFORM_PACKING, NULL);
        
        /* Extract image properties. */

        *w = (int) png_get_image_width (readp, infop);
        *h = (int) png_get_image_height(readp, infop);

        switch (png_get_color_type(readp, infop))
        {
        case PNG_COLOR_TYPE_GRAY:       *b = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: *b = 2; break;
        case PNG_COLOR_TYPE_RGB:        *b = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  *b = 4; break;
        default:                        *b = 0;
        }

        /* Read the pixel data. */

        if ((bytep = png_get_rows(readp, infop)))
        {
            int i, r, c;

            /* Allocate the final pixel buffer and copy pixels there. */

            if ((p = (unsigned char *) malloc((*w) * (*h) * (*b))))
                for (r = 0; r < (*h); ++r)
                    for (c = 0; c < (*w); ++c)
                        for (i = 0; i < (*b); ++i)
                            p[(*w)*(*b)*r + (*b)*c + i] =
                                (unsigned char) bytep[(*h)-r-1][(*b)*c + i];
        }
    }

    /* Release all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);
    fclose(filep);

    return p;
}

#endif /* CONF_NO_PNG */

/*---------------------------------------------------------------------------*/

#ifndef CONF_NO_JPG

#include <jpeglib.h>

static void *read_jpg(const char *filename, int *w, int *h, int *b)
{
    unsigned char *p = NULL;

    FILE *fin;

    if ((fin = fopen(filename, "rb")))
    {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr         jerr;

        /* Initialize the JPG decompressor. */

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, fin);

        /* Grab the JPG header info. */

        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        *w = cinfo.output_width;
        *h = cinfo.output_height;
        *b = cinfo.output_components;

        /* Allocate the pixel buffer and copy pixels there. */

        if ((p = (unsigned char *) malloc ((*w) * (*h) * (*b))))
        {
            int i = *h - 1;

            while (cinfo.output_scanline < cinfo.output_height)
            {
                unsigned char *s = p + i * (*w) * (*b);
                i -= jpeg_read_scanlines(&cinfo, &s, 1);
            }
        }

        /* Finalize the decompression. */

        jpeg_finish_decompress (&cinfo);
        jpeg_destroy_decompress(&cinfo);

        fclose(fin);
    }
    return p;
}

#endif /* CONF_NO_JPG */

/*---------------------------------------------------------------------------*/

void *obj_read_image(const char *filename, int *w, int *h, int *b)
{
    const char *ext = filename + strlen(filename) - 4;
    
#ifndef CONF_NO_PNG
    if (!strcmp(ext, ".png") || !strcmp(ext, ".PNG"))
        return read_png(filename, w, h, b);
#endif
#ifndef CONF_NO_JPG
    if (!strcmp(ext, ".jpg") || !strcmp(ext, ".JPG"))
        return read_jpg(filename, w, h, b);
#endif

    return NULL;
}

static GLuint read_img(const char *filename)
{
    static const GLenum format[5] = {
        0,
        GL_LUMINANCE,
        GL_LUMINANCE_ALPHA,
        GL_RGB,
        GL_RGBA,
    };

    GLuint o = 0;

    if (filename)
    {
        int w;
        int h;
        int b;

        void *p = obj_read_image(filename, &w, &h, &b);

        /* Read the image data from the named file to a new pixel buffer. */

        if (p)
        {
            obj_init_gl();

            /* Create an OpenGL texture object using these pixels. */

            glGenTextures(1, &o);
            glBindTexture(GL_TEXTURE_2D, o);

            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

            glTexImage2D(GL_TEXTURE_2D, 0, format[b], w, h, 0,
                         format[b], GL_UNSIGNED_BYTE, p);

            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            /* Discard the unnecessary pixel buffer. */

            free(p);
        }
    }
    return o;
}

static void dirpath(char *pathname)
{
    int i;

    /* Find the path by cutting a file name at the last directory delimiter. */

    for (i = (int) strlen(pathname) - 1; i >= 0; --i)
        if (pathname[i] == '/' || pathname[i] == '\\')
        {
            pathname[i] = '\0';
            return;
        }

    /* If no delimiter was found, return the current directory. */

    strcpy(pathname, ".");
}

/*---------------------------------------------------------------------------*/

static void read_image(int fi, int mi, int ki, const char *line,
                                               const char *path)
{
    unsigned int clamp  = 0;

    float o[3] = { 0.0f, 0.0f, 0.0f };
    float s[3] = { 1.0f, 1.0f, 1.0f };

    char pathname[MAXSTR];

    char map[MAXSTR];
    char val[MAXSTR];

    while (line[0] != '\0' && line[0] != '\r' && line[0] != '\n')
    {
        int n = 0;

        /* Parse property map options. */

        if (sscanf(line, " -clamp %s%n", val, &n) >= 1)
        {
            clamp  = (strcmp(val, "on") == 0) ? OBJ_OPT_CLAMP : 0;
            line  += n;
        }

        /* Parse property map scale. */

        else if (sscanf(line, " -s %f %f %f%n", s + 0, s + 1, s + 2, &n) >= 3)
            line += n;
        else if (sscanf(line, " -s %f %f%n",    s + 0, s + 1,        &n) >= 2)
            line += n;
        else if (sscanf(line, " -s %f%n",       s + 0,               &n) >= 1)
            line += n;

        /* Parse property map offset. */

        else if (sscanf(line, " -o %f %f %f%n", o + 0, o + 1, o + 2, &n) >= 3)
            line += n;
        else if (sscanf(line, " -o %f %f%n",    o + 0, o + 1,        &n) >= 2)
            line += n;
        else if (sscanf(line, " -o %f%n",       o + 0,               &n) >= 1)
            line += n;

        /* Parse a word.  The last word seen is taken to be the file name. */

        else if (sscanf(line, " %s%n", map, &n) >= 1)
            line += n;
    }

    /* Apply all parsed property attributes to the material. */

    if (strcmp(path, "."))
        sprintf(pathname, "%s/%s", path, map);
    else
        sprintf(pathname, "%s", map);

    obj_set_mtrl_opt(fi, mi, ki, clamp);
    obj_set_mtrl_map(fi, mi, ki, pathname);
    obj_set_mtrl_o  (fi, mi, ki, o);
    obj_set_mtrl_s  (fi, mi, ki, s);
}

static void read_color(int fi, int mi, int ki, const char *line)
{
    float c[4];

    /* Merge incoming color components with existing defaults. */

    obj_get_mtrl_c(fi, mi, ki, c);
    sscanf(line, "%f %f %f", c, c + 1, c + 2);
    obj_set_mtrl_c(fi, mi, ki, c);
}

static void read_alpha(int fi, int mi, int ki, const char *line)
{
    float c[4];

    /* Merge incoming color components with existing defaults. */

    obj_get_mtrl_c(fi, mi, ki, c);
    sscanf(line, "%f", c + 3);
    obj_set_mtrl_c(fi, mi, ki, c);
}

static void read_mtl(const char *path,
                     const char *file,
                     const char *name, int fi, int mi)
{
    char pathname[MAXSTR];

    char buf[MAXSTR];
    char key[MAXSTR];
    char arg[MAXSTR];

    FILE *fin;

    int scanning = 1;
    int n        = 0;

    sprintf(pathname, "%s/%s", path, file);

    if ((fin = fopen(pathname, "r")))
    {
        /* Process each line of the MTL file. */

        while  (fgets (buf, MAXSTR, fin))
            if (sscanf(buf, "%s%n", key, &n) >= 1)
            {
                const char *c = buf + n;

                if (scanning)
                {
                    /* Determine if we've found the MTL we're looking for. */

                    if (!strcmp(key, "newmtl"))
                    {
                        sscanf(c, "%s", arg);

                        if ((scanning = strcmp(arg, name)) == 0)
                            obj_set_mtrl_name(fi, mi, name);
                    }
                }
                else
                {
                    /* Stop scanning when the next MTL begins. */

                    if (!strcmp(key, "newmtl"))
                        break;

                    /* Parse this material's properties. */

                    else if (!strcmp(key, "map_Kd"))
                        read_image(fi, mi, OBJ_KD, c, path);
                    else if (!strcmp(key, "map_Ka"))
                        read_image(fi, mi, OBJ_KA, c, path);
                    else if (!strcmp(key, "map_Ke"))
                        read_image(fi, mi, OBJ_KE, c, path);
                    else if (!strcmp(key, "map_Ks"))
                        read_image(fi, mi, OBJ_KS, c, path);
                    else if (!strcmp(key, "map_Ns"))
                        read_image(fi, mi, OBJ_NS, c, path);

                    else if (!strcmp(key, "Kd"))
                        read_color(fi, mi, OBJ_KD, c);
                    else if (!strcmp(key, "Ka"))
                        read_color(fi, mi, OBJ_KA, c);
                    else if (!strcmp(key, "Ke"))
                        read_color(fi, mi, OBJ_KE, c);
                    else if (!strcmp(key, "Ks"))
                        read_color(fi, mi, OBJ_KS, c);
                    else if (!strcmp(key, "Ns"))
                        read_color(fi, mi, OBJ_NS, c);

                    else if (!strcmp(key, "d"))
                        read_alpha(fi, mi, OBJ_KD, c);
                }
            }
        fclose(fin);
    }
}

static void read_mtllib(char *file, const char *line)
{
    /* Parse the first file name from the given line. */

    sscanf(line, "%s", file);
}

static int read_usemtl(const char *path,
                       const char *file,
                       const char *line, int fi)
{
    char name[MAXSTR];

    int si;
    int mi;

    sscanf(line, "%s", name);

    /* Create a new material for the incoming definition. */

    if ((mi = obj_add_mtrl(fi)) >= 0)
    {
        /* Create a new surface to contain geometry with the new material. */

        if ((si = obj_add_surf(fi)) >= 0)
        {
            /* Read the material definition and apply it to the new surface. */

            read_mtl(path, file, name, fi, mi);
            obj_set_surf(fi, si, mi);

            /* Return the surface so that new geometry may be added to it. */

            return si;
        }
    }

    /* On failure, return the default surface. */

    return 0;
}

/*---------------------------------------------------------------------------*/

static int read_poly_indices(const char *line, int *_vi, int *_ti, int *_ni)
{
    int n;

    *_vi = 0;
    *_ti = 0;
    *_ni = 0;

    /* Parse a face vertex specification from the given line. */

    if (sscanf(line, "%d/%d/%d%n", _vi, _ti, _ni, &n) >= 3) return n;
    if (sscanf(line, "%d/%d%n",    _vi, _ti,      &n) >= 2) return n;
    if (sscanf(line, "%d//%d%n",   _vi,      _ni, &n) >= 2) return n;
    if (sscanf(line, "%d%n",       _vi,           &n) >= 1) return n;

    /*
    static char vert[MAXSTR];

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        if (sscanf(vert, "%d/%d/%d", _vi, _ti, _ni) == 3) return n;
        if (sscanf(vert, "%d/%d",    _vi, _ti     ) == 2) return n;
        if (sscanf(vert, "%d//%d",   _vi,      _ni) == 2) return n;
        if (sscanf(vert, "%d",       _vi          ) == 1) return n;
    }
    */
    return 0;
}

static int read_poly_vertices(const char *line, int fi, int gi)
{
    const char *c = line;

    int _vi;
    int _ti;
    int _ni;
    int _ii;
    int _ij;

    int  dc;
    int  vi;
    int  ic = 0;

    /* Scan the face string, converting index sets to vertices. */

    while ((dc = read_poly_indices(c, &_vi, &_ti, &_ni)))
    {
        /* Convert face indices to vector cache indices. */

        _vi += (_vi < 0) ? _vc : -1;
        _ti += (_ti < 0) ? _tc : -1;
        _ni += (_ni < 0) ? _nc : -1;

        /* Initialize a new index set. */

        if ((_ii = add_i()) >= 0)
        {
            _iv[_ii]._vi = _vi;
            _iv[_ii]._ni = _ni;
            _iv[_ii]._ti = _ti;

            /* Search the vector reference list for a repeated index set. */

            for (_ij = _vv[_vi]._ii; _ij >= 0; _ij = _iv[_ij]._ii)
                if (_iv[_ij]._vi == _vi &&
                    _iv[_ij]._ti == _ti &&
                    _iv[_ij]._ni == _ni &&
                    _iv[_ij]. gi ==  gi)
                {
                    /* A repeat has been found.  Link new to old. */

                    _vv[_vi]._ii = _ii;
                    _iv[_ii]._ii = _ij;
                    _iv[_ii]. vi = _iv[_ij].vi;
                    _iv[_ii]. gi = _iv[_ij].gi;

                    break;
                }

            /* If no repeat was found, add a new vertex. */

            if ((_ij < 0) && (vi = obj_add_vert(fi)) >= 0)
            {
                _vv[_vi]._ii = _ii;
                _iv[_ii]._ii =  -1;
                _iv[_ii]. vi =  vi;
                _iv[_ii]. gi =  gi;

                /* Initialize the new vertex using valid cache references. */

                if (0 <= _vi && _vi < _vc) obj_set_vert_v(fi, vi, _vv[_vi].v);
                if (0 <= _ni && _ni < _nc) obj_set_vert_n(fi, vi, _nv[_ni].v);
                if (0 <= _ti && _ti < _tc) obj_set_vert_t(fi, vi, _tv[_ti].v);
            }
            ic++;
        }
        c  += dc;
    }
    return ic;
}

static void read_f(const char *line, int fi, int si, int gi)
{
    float n[3];
    int i, pi;

    /* Create new vertices references for this face. */

    int i0 = _ic;
    int ic = read_poly_vertices(line, fi, gi);

    /* If smoothing, apply this face's normal to vertices that need it. */

    if (gi)
    {
        normal(n, _vv[_iv[i0 + 0]._vi].v,
                  _vv[_iv[i0 + 1]._vi].v,
                  _vv[_iv[i0 + 2]._vi].v);

        for (i = 0; i < ic; ++i)
            if (_iv[i0 + 0]._ni < 0)
            {
                vert(fi, _iv[i0 + i]._vi)->n[0] += n[0];
                vert(fi, _iv[i0 + i]._vi)->n[1] += n[1];
                vert(fi, _iv[i0 + i]._vi)->n[2] += n[2];
            }
    }

    /* Convert our N new vertex references into N-2 new triangles. */

    for (i = 0; i < ic - 2; ++i)

        if ((pi = obj_add_poly(fi, si)) >= 0)
        {
            int vi[3];

            vi[0] = _iv[i0        ].vi;
            vi[1] = _iv[i0 + i + 1].vi;
            vi[2] = _iv[i0 + i + 2].vi;

            obj_set_poly(fi, si, pi, vi);
        }
}

/*---------------------------------------------------------------------------*/

static int read_line_indices(const char *line, int *_vi, int *_ti)
{
    int n;

    *_vi = 0;
    *_ti = 0;

    /* Parse a line vertex specification from the given line. */

    if (sscanf(line, "%d/%d%n", _vi, _ti, &n) >= 2) return n;
    if (sscanf(line, "%d%n",    _vi,      &n) >= 1) return n;

    /*
    static char vert[MAXSTR];

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        if (sscanf(vert, "%d/%d", _vi, _ti) == 2) return n;
        if (sscanf(vert, "%d",    _vi     ) == 1) return n;
    }
    */
    return 0;
}

static int read_line_vertices(const char *line, int fi)
{
    const char *c = line;

    int _vi;
    int _ti;
    int _ii;
    int _ij;

    int  dc;
    int  vi;
    int  ic = 0;

    /* Scan the line string, converting index sets to vertices. */

    while ((dc = read_line_indices(c, &_vi, &_ti)))
    {
        /* Convert line indices to vector cache indices. */

        _vi += (_vi < 0) ? _vc : -1;
        _ti += (_ti < 0) ? _tc : -1;

        /* Initialize a new index set. */

        if ((_ii = add_i()) >= 0)
        {
            _iv[_ii]._vi = _vi;
            _iv[_ii]._ti = _ti;

            /* Search the vector reference list for a repeated index set. */

            for (_ij = _vv[_vi]._ii; _ij >= 0; _ij = _iv[_ij]._ii)
                if (_iv[_ij]._vi == _vi &&
                    _iv[_ij]._ti == _ti)
                {
                    /* A repeat has been found.  Link new to old. */

                    _vv[_vi]._ii = _ii;
                    _iv[_ii]._ii = _ij;
                    _iv[_ii]. vi = _iv[_ij].vi;

                    break;
                }

            /* If no repeat was found, add a new vertex. */

            if ((_ij < 0) && (vi = obj_add_vert(fi)) >= 0)
            {
                _vv[_vi]._ii = _ii;
                _iv[_ii]._ii =  -1;
                _iv[_ii]. vi =  vi;

                /* Initialize the new vertex using valid cache references. */

                if (0 <= _vi && _vi < _vc) obj_set_vert_v(fi, vi, _vv[_vi].v);
                if (0 <= _ti && _ti < _tc) obj_set_vert_t(fi, vi, _tv[_ti].v);
            }
            ic++;
        }
        c  += dc;
    }
    return ic;
}

static void read_l(const char *line, int fi, int si)
{
    int i, li;

    /* Create new vertices for this line. */

    int i0 = _ic;
    int ic = read_line_vertices(line, fi);

    /* Convert our N new vertices into N-1 new lines. */

    for (i = 0; i < ic - 1; ++i)

        if ((li = obj_add_line(fi, si)) >= 0)
        {
            int vi[2];

            vi[0] = _iv[i0 + i    ].vi;
            vi[1] = _iv[i0 + i + 1].vi;

            obj_set_line(fi, si, li, vi);
        }
}

/*---------------------------------------------------------------------------*/

static void read_v(const char *line)
{
    int _vi;

    /* Parse a vertex position. */

    if ((_vi = add_v()) >= 0)
    {
        sscanf(line, "%f %f %f", _vv[_vi].v + 0,
                                 _vv[_vi].v + 1,
                                 _vv[_vi].v + 2);
        _vv[_vi]._ii = -1;
    }
}

static void read_vt(const char *line)
{
    int _ti;

    /* Parse a texture coordinate. */

    if ((_ti = add_t()) >= 0)
    {
        sscanf(line, "%f %f", _tv[_ti].v + 0,
                              _tv[_ti].v + 1);
        _tv[_ti]._ii = -1;
    }
}

static void read_vn(const char *line)
{
    int _ni;

    /* Parse a normal. */

    if ((_ni = add_n()) >= 0)
    {
        sscanf(line, "%f %f %f", _nv[_ni].v + 0,
                                 _nv[_ni].v + 1,
                                 _nv[_ni].v + 2);
        _nv[_ni]._ii = -1;
    }
}

/*---------------------------------------------------------------------------*/

static void read_obj(int fi, const char *filename)
{
    char buf[MAXSTR];
    char key[MAXSTR];

    char L[MAXSTR];
    char D[MAXSTR];

    FILE *fin;

    /* Flush the vector caches. */

    _vc = 0;
    _tc = 0;
    _nc = 0;
    _ic = 0;

    /* Add the named file to the given object. */

    if ((fin = fopen(filename, "r")))
    {
        /* Ensure there exists a default surface 0 and default material 0. */

        int si = obj_add_surf(fi);
        int mi = obj_add_mtrl(fi);
        int gi = 0;
        int n;

        obj_set_surf(fi, si, mi);

        /* Extract the directory from the filename for use in MTL loading. */

        strncpy(D, filename, MAXSTR);
        dirpath(D);

        /* Process each line of the OBJ file, invoking the handler for each. */

        while  (fgets (buf, MAXSTR, fin))
            if (sscanf(buf, "%s%n", key, &n) >= 1)
            {
                const char *c = buf + n;

                if      (!strcmp(key, "f" )) read_f (c, fi, si, gi);
                else if (!strcmp(key, "l" )) read_l (c, fi, si);
                else if (!strcmp(key, "vt")) read_vt(c);
                else if (!strcmp(key, "vn")) read_vn(c);
                else if (!strcmp(key, "v" )) read_v (c);

                else if (!strcmp(key, "mtllib"))      read_mtllib(L, c);
                else if (!strcmp(key, "usemtl")) si = read_usemtl(D, L, c, fi);
                else if (!strcmp(key, "s"     )) gi = atoi(c);
            }
            
        fclose(fin);
    }
}

/*===========================================================================*/

int obj_add_mtrl(int fi)
{
    unsigned int opt = 0;
    
    const float Kd[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    const float Ka[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    const float Ke[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const float Ks[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const float Ns[4] = { 8.0f, 0.0f, 0.0f, 0.0f };
    const float  s[3] = { 1.0f, 1.0f, 1.0f       };

    int mi;

    assert_file(fi);

    /* Allocate and initialize a new material. */

    if ((mi = add__((void **) &file(fi)->mv,
                              &file(fi)->mc,
                              &file(fi)->mm, sizeof (struct obj_mtrl))) >= 0)
    {
        memset(mtrl(fi, mi), 0, sizeof (struct obj_mtrl));

        obj_set_mtrl_opt(fi, mi, OBJ_KD, opt);
        obj_set_mtrl_opt(fi, mi, OBJ_KA, opt);
        obj_set_mtrl_opt(fi, mi, OBJ_KE, opt);
        obj_set_mtrl_opt(fi, mi, OBJ_KS, opt);
        obj_set_mtrl_opt(fi, mi, OBJ_NS, opt);

        obj_set_mtrl_c  (fi, mi, OBJ_KD, Kd);
        obj_set_mtrl_c  (fi, mi, OBJ_KA, Ka);
        obj_set_mtrl_c  (fi, mi, OBJ_KE, Ke);
        obj_set_mtrl_c  (fi, mi, OBJ_KS, Ks);
        obj_set_mtrl_c  (fi, mi, OBJ_NS, Ns);

        obj_set_mtrl_s  (fi, mi, OBJ_KD, s);
        obj_set_mtrl_s  (fi, mi, OBJ_KA, s);
        obj_set_mtrl_s  (fi, mi, OBJ_KE, s);
        obj_set_mtrl_s  (fi, mi, OBJ_KS, s);
        obj_set_mtrl_s  (fi, mi, OBJ_NS, s);
    }
    return mi;
}

int obj_add_vert(int fi)
{
    int vi;

    assert_file(fi);

    /* Allocate and initialize a new vertex. */

    if ((vi = add__((void **) &file(fi)->vv,
                              &file(fi)->vc,
                              &file(fi)->vm, sizeof (struct obj_vert))) >= 0)
    {
        memset(vert(fi, vi), 0, sizeof (struct obj_vert));
    }
    return vi;
}

int obj_add_poly(int fi, int si)
{
    int pi;

    assert_surf(fi, si);

    /* Allocate and initialize a new polygon. */

    if ((pi = add__((void **) &surf(fi, si)->pv,
                              &surf(fi, si)->pc,
                              &surf(fi, si)->pm, sizeof (struct obj_poly)))>=0)
    {
        memset(poly(fi, si, pi), 0, sizeof (struct obj_poly));
    }
    return pi;
}

int obj_add_line(int fi, int si)
{
    int li;

    assert_surf(fi, si);

    /* Allocate and initialize a new line. */

    if ((li = add__((void **) &surf(fi, si)->lv,
                              &surf(fi, si)->lc,
                              &surf(fi, si)->lm, sizeof (struct obj_line)))>=0)
    {
        memset(line(fi, si, li), 0, sizeof (struct obj_line));
    }
    return li;
}

int obj_add_surf(int fi)
{
    int si;

    assert_file(fi);

    /* Allocate and initialize a new surface. */

    if ((si = add__((void **) &file(fi)->sv,
                              &file(fi)->sc,
                              &file(fi)->sm, sizeof (struct obj_surf))) >= 0)
    {
        memset(surf(fi, si), 0, sizeof (struct obj_surf));
    }
    return si;
}

int obj_add_file(const char *filename)
{
    int fi;

    /* Allocate and initialize a new file. */

    if ((fi = add__((void **) &fv,
                              &fc,
                              &fm, sizeof (struct obj_file))) >= 0)
    {
        memset(file(fi), 0, sizeof (struct obj_file));

        if (filename)
        {
            /* Read the named file. */

            read_obj(fi, filename);

            /* Post-process the loaded object. */

            obj_mini_file(fi);
            obj_proc_file(fi);
        }
    }
    return fi;
}

/*---------------------------------------------------------------------------*/

int obj_num_mtrl(int fi)
{
    assert_file(fi);
    return file(fi)->mc;
}

int obj_num_vert(int fi)
{
    assert_file(fi);
    return file(fi)->vc;
}

int obj_num_poly(int fi, int si)
{
    assert_surf(fi, si);
    return surf(fi, si)->pc;
}

int obj_num_line(int fi, int si)
{
    assert_surf(fi, si);
    return surf(fi, si)->lc;
}

int obj_num_surf(int fi)
{
    assert_file(fi);
    return file(fi)->sc;
}

int obj_num_file(void)
{
    return fc;
}

/*---------------------------------------------------------------------------*/

static void obj_rel_mtrl(struct obj_mtrl *mp)
{
    /* Release any resources held by this material. */

    if (mp->kv[0].str) free(mp->kv[0].str);
    if (mp->kv[1].str) free(mp->kv[1].str);
    if (mp->kv[2].str) free(mp->kv[2].str);
    if (mp->kv[3].str) free(mp->kv[3].str);

    if (mp->kv[0].map) glDeleteTextures(1, &mp->kv[0].map);
    if (mp->kv[1].map) glDeleteTextures(1, &mp->kv[1].map);
    if (mp->kv[2].map) glDeleteTextures(1, &mp->kv[2].map);
    if (mp->kv[3].map) glDeleteTextures(1, &mp->kv[3].map);
}

static void obj_rel_surf(struct obj_surf *sp)
{
    if (sp->pibo) glDeleteBuffers(1, &sp->pibo);
    if (sp->libo) glDeleteBuffers(1, &sp->libo);

    sp->pibo = 0;
    sp->libo = 0;

    /* Release this surface's polygon and line vectors. */

    if (sp->pv) free(sp->pv);
    if (sp->lv) free(sp->lv);
}

static void obj_rel_file(struct obj_file *fp)
{
    int si;
    int mi;

    if (fp->vbo) glDeleteBuffers(1, &fp->vbo);

    fp->vbo = 0;

    /* Release resources held by this file and it's materials and surfaces. */

    for (mi = 0; mi < fp->mc; ++mi) obj_rel_mtrl(fp->mv + mi);
    for (si = 0; si < fp->sc; ++si) obj_rel_surf(fp->sv + si);
}

/*---------------------------------------------------------------------------*/

void obj_del_mtrl(int fi, int mi)
{
    int si;

    assert_mtrl(fi, mi);

    /* Remove this material from the material vector. */

    obj_rel_mtrl(mtrl(fi, mi));

    memmove(mtrl(fi, mi),
            mtrl(fi, mi + 1),
            (file(fi)->mc - mi - 1) * sizeof (struct obj_mtrl));

    file(fi)->mc--;

    /* Remove all references to this material. */

    for (si = file(fi)->sc - 1; si >= 0; --si)
    {
        struct obj_surf *sp = surf(fi, si);

        if (sp->mi == mi)
            obj_del_surf(fi, si);
        else
            if (sp->mi > mi) sp->mi--;
    }
}

void obj_del_vert(int fi, int vi)
{
    int si;
    int pi;
    int li;

    assert_vert(fi, vi);

    /* Remove this vertex from the file's vertex vector. */

    memmove(vert(fi, vi),
            vert(fi, vi + 1),
           (file(fi)->vc - vi - 1) * sizeof (struct obj_vert));

    file(fi)->vc--;

    /* Remove all references to this vertex from all surfaces. */

    for (si = 0; si < file(fi)->sc; ++si)
    {
        /* Delete all referencing polygons.  Decrement later references. */

        for (pi = surf(fi, si)->pc - 1; pi >= 0; --pi)
        {
            struct obj_poly *pp = poly(fi, si, pi);

            if (pp->vi[0] == vi || pp->vi[1] == vi || pp->vi[2] == vi)
                obj_del_poly(fi, si, pi);
            else
            {
                if (pp->vi[0] > vi) pp->vi[0]--;
                if (pp->vi[1] > vi) pp->vi[1]--;
                if (pp->vi[2] > vi) pp->vi[2]--;
            }
        }

        /* Delete all referencing lines.  Decrement later references. */

        for (li = surf(fi, si)->lc - 1; li >= 0; --li)
        {
            struct obj_line *lp = line(fi, si, li);

            if (lp->vi[0] == vi || lp->vi[1] == vi)
                obj_del_line(fi, si, li);
            else
            {
                if (lp->vi[0] > vi) lp->vi[0]--;
                if (lp->vi[1] > vi) lp->vi[1]--;
            }
        }
    }

    /* Schedule the VBO for refresh. */

    invalidate(fi);
}

void obj_del_poly(int fi, int si, int pi)
{
    assert_poly(fi, si, pi);

    /* Remove this polygon from the surface's polygon vector. */

    memmove(poly(fi, si, pi),
            poly(fi, si, pi + 1),
           (surf(fi, si)->pc - pi - 1) * sizeof (struct obj_poly));

    surf(fi, si)->pc--;
}

void obj_del_line(int fi, int si, int li)
{
    assert_line(fi, si, li);

    /* Remove this line from the surface's line vector. */

    memmove(line(fi, si, li),
            line(fi, si, li + 1),
           (surf(fi, si)->lc - li - 1) * sizeof (struct obj_line));

    surf(fi, si)->lc--;
}

void obj_del_surf(int fi, int si)
{
    assert_surf(fi, si);

    /* Remove this surface from the file's surface vector. */

    obj_rel_surf(fv[fi].sv + si);

    memmove(surf(fi, si),
            surf(fi, si + 1),
           (file(fi)->sc - si - 1) * sizeof (struct obj_surf));

    file(fi)->sc--;
}

void obj_del_file(int fi)
{
    assert_file(fi);

    /* Remove this file from the global file vector. */

    obj_rel_file(fv + fi);

    memmove(file(fi),
            file(fi + 1),
           (fc - fi - 1) * sizeof (struct obj_file));

    fc--;
}

/*---------------------------------------------------------------------------*/

static char *set_name(char *old, const char *src)
{
    char *dst = NULL;

    if (old)
        free(old);

    if (src && (dst = (char *) malloc(strlen(src) + 1)))
        strcpy(dst, src);

    return dst;
}

void obj_set_mtrl_name(int fi, int mi, const char *name)
{
    assert_mtrl(fi, mi);
    mtrl(fi, mi)->name = set_name(mtrl(fi, mi)->name, name);
}

void obj_set_mtrl_map(int fi, int mi, int ki, const char *str)
{
    assert_prop(fi, mi, ki);
    
    if (prop(fi, mi, ki)->map)
        glDeleteTextures(1, &prop(fi, mi, ki)->map);

    prop(fi, mi, ki)->map = read_img(str);
    prop(fi, mi, ki)->str = set_name(prop(fi, mi, ki)->str, str);
}

void obj_set_mtrl_opt(int fi, int mi, int ki, unsigned int opt)
{
    assert_prop(fi, mi, ki);

    prop(fi, mi, ki)->opt = opt;
}

void obj_set_mtrl_c(int fi, int mi, int ki, const float c[4])
{
    assert_prop(fi, mi, ki);

    prop(fi, mi, ki)->c[0] = c[0];
    prop(fi, mi, ki)->c[1] = c[1];
    prop(fi, mi, ki)->c[2] = c[2];
    prop(fi, mi, ki)->c[3] = c[3];
}

void obj_set_mtrl_s(int fi, int mi, int ki, const float s[3])
{
    assert_prop(fi, mi, ki);

    prop(fi, mi, ki)->s[0] = s[0];
    prop(fi, mi, ki)->s[1] = s[1];
    prop(fi, mi, ki)->s[2] = s[2];
}

void obj_set_mtrl_o(int fi, int mi, int ki, const float o[3])
{
    assert_prop(fi, mi, ki);

    prop(fi, mi, ki)->o[0] = o[0];
    prop(fi, mi, ki)->o[1] = o[1];
    prop(fi, mi, ki)->o[2] = o[2];
}

/*---------------------------------------------------------------------------*/

static void invalidate(int fi)
{
    if (file(fi)->vbo)
    {
        glDeleteBuffers(1, &file(fi)->vbo);
        file(fi)->vbo = 0;
    }
}

void obj_set_vert_v(int fi, int vi, const float v[3])
{
    assert_vert(fi, vi);

    vert(fi, vi)->v[0] = v[0];
    vert(fi, vi)->v[1] = v[1];
    vert(fi, vi)->v[2] = v[2];

    invalidate(fi);
}

void obj_set_vert_t(int fi, int vi, const float t[2])
{
    assert_vert(fi, vi);

    vert(fi, vi)->t[0] = t[0];
    vert(fi, vi)->t[1] = t[1];

    invalidate(fi);
}

void obj_set_vert_n(int fi, int vi, const float n[3])
{
    assert_vert(fi, vi);

    vert(fi, vi)->n[0] = n[0];
    vert(fi, vi)->n[1] = n[1];
    vert(fi, vi)->n[2] = n[2];

    invalidate(fi);
}

/*---------------------------------------------------------------------------*/

void obj_set_poly(int fi, int si, int pi, const int vi[3])
{
    assert_poly(fi, si, pi);

    poly(fi, si, pi)->vi[0] = vi[0];
    poly(fi, si, pi)->vi[1] = vi[1];
    poly(fi, si, pi)->vi[2] = vi[2];
}

void obj_set_line(int fi, int si, int li, const int vi[2])
{
    assert_line(fi, si, li);

    line(fi, si, li)->vi[0] = vi[0];
    line(fi, si, li)->vi[1] = vi[1];
}

void obj_set_surf(int fi, int si, int mi)
{
    assert_surf(fi, si);

    surf(fi, si)->mi = mi;
}

/*===========================================================================*/

const char *obj_get_mtrl_name(int fi, int mi)
{
    assert_mtrl(fi, mi);
    return mtrl(fi, mi)->name;
}

const char *obj_get_mtrl_map(int fi, int mi, int ki)
{
    assert_prop(fi, mi, ki);
    return prop(fi, mi, ki)->str;
}

unsigned int obj_get_mtrl_opt(int fi, int mi, int ki)
{
    assert_prop(fi, mi, ki);
    return prop(fi, mi, ki)->opt;
}

void obj_get_mtrl_c(int fi, int mi, int ki, float c[4])
{
    assert_prop(fi, mi, ki);

    c[0] = prop(fi, mi, ki)->c[0];
    c[1] = prop(fi, mi, ki)->c[1];
    c[2] = prop(fi, mi, ki)->c[2];
    c[3] = prop(fi, mi, ki)->c[3];
}

void obj_get_mtrl_s(int fi, int mi, int ki, float s[3])
{
    assert_prop(fi, mi, ki);

    s[0] = prop(fi, mi, ki)->s[0];
    s[1] = prop(fi, mi, ki)->s[1];
    s[2] = prop(fi, mi, ki)->s[2];
}

void obj_get_mtrl_o(int fi, int mi, int ki, float o[3])
{
    assert_prop(fi, mi, ki);

    o[0] = prop(fi, mi, ki)->o[0];
    o[1] = prop(fi, mi, ki)->o[1];
    o[2] = prop(fi, mi, ki)->o[2];
}

/*---------------------------------------------------------------------------*/

void obj_get_vert_v(int fi, int vi, float v[3])
{
    assert_vert(fi, vi);
    
    v[0] = vert(fi, vi)->v[0];
    v[1] = vert(fi, vi)->v[1];
    v[2] = vert(fi, vi)->v[2];
}

void obj_get_vert_t(int fi, int vi, float t[2])
{
    assert_vert(fi, vi);
    
    t[0] = vert(fi, vi)->t[0];
    t[1] = vert(fi, vi)->t[1];
}

void obj_get_vert_n(int fi, int vi, float n[3])
{
    assert_vert(fi, vi);
    
    n[0] = vert(fi, vi)->n[0];
    n[1] = vert(fi, vi)->n[1];
    n[2] = vert(fi, vi)->n[2];
}

/*---------------------------------------------------------------------------*/

void obj_get_poly(int fi, int si, int pi, int vi[3])
{
    assert_poly(fi, si, pi);

    vi[0] = poly(fi, si, pi)->vi[0];
    vi[1] = poly(fi, si, pi)->vi[1];
    vi[2] = poly(fi, si, pi)->vi[2];
}

void obj_get_line(int fi, int si, int li, int vi[2])
{
    assert_line(fi, si, li);

    vi[0] = line(fi, si, li)->vi[0];
    vi[1] = line(fi, si, li)->vi[1];
}

int obj_get_surf(int fi, int si)
{
    assert_surf(fi, si);
    return surf(fi, si)->mi;
}

/*===========================================================================*/

void obj_mini_file(int fi)
{
    int si;
    int mi;

    /* Remove empty surfaces. */

    for (si = file(fi)->sc - 1; si >= 0; --si)
        if (surf(fi, si)->pc == 0 &&
            surf(fi, si)->lc == 0)
            obj_del_surf(fi, si);

    /* Remove unreferenced materials. */
    
    for (mi = file(fi)->mc - 1; mi >= 0; --mi)
    {
        int cc = 0;

        for (si = 0; si < file(fi)->sc; ++si)
            if (surf(fi, si)->mi == mi)
                cc++;

        if (cc == 0)
            obj_del_mtrl(fi, mi);
    }
}

void obj_norm_file(int fi)
{
    int si;
    int pi;

    assert_file(fi);

    /* Compute normals for all faces. */

    for (si = 0; si < file(fi)->sc; ++si)
        for (pi = 0; pi < surf(fi, si)->pc; ++pi)
        {
            struct obj_vert *v0 = vert(fi, poly(fi, si, pi)->vi[0]);
            struct obj_vert *v1 = vert(fi, poly(fi, si, pi)->vi[1]);
            struct obj_vert *v2 = vert(fi, poly(fi, si, pi)->vi[2]);

            float n[3];

            /* Compute the normal formed by these 3 vertices. */

            normal(n, v0->v, v1->v, v2->v);

            /* Sum this normal to all vertices. */

            v0->n[0] += n[0];
            v0->n[1] += n[1];
            v0->n[2] += n[0];

            v1->n[0] += n[0];
            v1->n[1] += n[1];
            v1->n[2] += n[0];

            v2->n[0] += n[0];
            v2->n[1] += n[1];
            v2->n[2] += n[0];
        }
}

void obj_proc_file(int fi)
{
    int si;
    int sj;
    int pi;
    int vi;

    assert_file(fi);

    /* Normalize all normals.  Zero all tangent vectors. */

    for (vi = 0; vi < file(fi)->vc; ++vi)
    {
        normalize(vert(fi, vi)->n);

        vert(fi, vi)->u[0] = 0.0f;
        vert(fi, vi)->u[1] = 0.0f;
        vert(fi, vi)->u[2] = 0.0f;
    }

    /* Compute tangent vectors for all vertices. */

    for (si = 0; si < file(fi)->sc; ++si)
        for (pi = 0; pi < surf(fi, si)->pc; ++pi)
        {
            struct obj_vert *v0 = vert(fi, poly(fi, si, pi)->vi[0]);
            struct obj_vert *v1 = vert(fi, poly(fi, si, pi)->vi[1]);
            struct obj_vert *v2 = vert(fi, poly(fi, si, pi)->vi[2]);

            float dt1, dv1[3];
            float dt2, dv2[3];

            float u[3];

            /* Compute the tangent vector for this polygon. */

            dv1[0] = v1->v[0] - v0->v[0];
            dv1[1] = v1->v[1] - v0->v[1];
            dv1[2] = v1->v[2] - v0->v[2];

            dv2[0] = v2->v[0] - v0->v[0];
            dv2[1] = v2->v[1] - v0->v[1];
            dv2[2] = v2->v[2] - v0->v[2];

            dt1    = v1->t[1] - v0->t[1];
            dt2    = v2->t[1] - v0->t[1];

            u[0]   = dt2 * dv1[0] - dt1 * dv2[0];
            u[1]   = dt2 * dv1[1] - dt1 * dv2[1];
            u[2]   = dt2 * dv1[2] - dt1 * dv2[2];

            normalize(u);

            /* Accumulate the tangent vectors for this polygon's vertices. */ 

            v0->u[0] += u[0];  v0->u[1] += u[1];  v0->u[2] += u[2];
            v1->u[0] += u[0];  v1->u[1] += u[1];  v1->u[2] += u[2];
            v2->u[0] += u[0];  v2->u[1] += u[1];  v2->u[2] += u[2];
        }

    /* Orthonormalize each tangent basis. */

    for (vi = 0; vi < file(fi)->vc; ++vi)
    {
        float *n = vert(fi, vi)->n;
        float *u = vert(fi, vi)->u;

        float v[3];

        cross(v, n, u);
        cross(u, v, n);
        normalize(u);
    }

    /* Sort surfaces such that transparent ones appear later. */

    for (si = 0; si < file(fi)->sc; ++si)
        for (sj = si + 1; sj < file(fi)->sc; ++sj)
            if (prop(fi, surf(fi, si)->mi, OBJ_KD)->c[3] <
                prop(fi, surf(fi, sj)->mi, OBJ_KD)->c[3])
            {
                struct obj_surf temp;

                 temp         = *surf(fi, si);
                *surf(fi, si) = *surf(fi, sj);
                *surf(fi, sj) =  temp;
            }
}

void obj_init_file(int fi)
{
    if (file(fi)->vbo == 0 && GL_has_vertex_buffer_object)
    {
        int si;

        /* Store all vertex data in a vertex buffer object. */

        glGenBuffers(1, &file(fi)->vbo);
        glBindBuffer(GL_ARRAY_BUFFER_ARB, file(fi)->vbo);
        glBufferData(GL_ARRAY_BUFFER_ARB,
                     file(fi)->vc * sizeof (struct obj_vert),
                     file(fi)->vv,  GL_STATIC_DRAW_ARB);

        /* Store all index data in index buffer objects. */

        for (si = 0; si < file(fi)->sc; ++si)
        {
            if (surf(fi, si)->pc > 0)
            {
                glGenBuffers(1, &surf(fi, si)->pibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,
                             surf(fi, si)->pibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,
                             surf(fi, si)->pc * sizeof (struct obj_poly),
                             surf(fi, si)->pv, GL_STATIC_DRAW_ARB);
            }

            if (surf(fi, si)->lc > 0)
            {
                glGenBuffers(1, &surf(fi, si)->libo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,
                             surf(fi, si)->libo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,
                             surf(fi, si)->lc * sizeof (struct obj_line),
                             surf(fi, si)->lv, GL_STATIC_DRAW_ARB);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

int obj_cmp_vert(void *data, const void *a, const void *b)
{
    const int vi = *((const int *) a);
    const int vj = *((const int *) b);

    const struct obj_vert *v = (const struct obj_vert *) data;

    const struct obj_vert *va = v + vi;
    const struct obj_vert *vb = v + vj;

    if (va->v[0] < vb->v[0] - EPSILON) return -1;
    if (va->v[0] > vb->v[0] + EPSILON) return +1;

    if (va->v[1] < vb->v[1] - EPSILON) return -1;
    if (va->v[1] > vb->v[1] + EPSILON) return +1;

    if (va->v[2] < vb->v[2] - EPSILON) return -1;
    if (va->v[2] > vb->v[2] + EPSILON) return +1;

    if (va->n[0] < vb->n[0] - EPSILON) return -1;
    if (va->n[0] > vb->n[0] + EPSILON) return +1;

    if (va->n[1] < vb->n[1] - EPSILON) return -1;
    if (va->n[1] > vb->n[1] + EPSILON) return +1;

    if (va->n[2] < vb->n[2] - EPSILON) return -1;
    if (va->n[2] > vb->n[2] + EPSILON) return +1;

    if (va->t[0] < vb->t[0] - EPSILON) return -1;
    if (va->t[0] > vb->t[0] + EPSILON) return +1;

    if (va->t[1] < vb->t[1] - EPSILON) return -1;
    if (va->t[1] > vb->t[1] + EPSILON) return +1;

    return 0;
}

void obj_uniq_file(int fi)
{
    struct obj_vert *vv = file(fi)->vv;
    const int        vc = file(fi)->vc;

    struct obj_vert *wv =
        (struct obj_vert *) malloc(vc * sizeof (struct obj_vert));
    
    int *src_map = (int *) malloc(vc * sizeof (int));
    int *dst_map = (int *) malloc(vc * sizeof (int));
    int *src_inv = (int *) malloc(vc * sizeof (int));
    int *dst_inv = (int *) malloc(vc * sizeof (int));
    int *del     = (int *) malloc(vc * sizeof (int));

    int vi;
    int vj;
    int si;
    int pi;
    int li;
    int wc = 0;

    memcpy(wv, vv, vc * sizeof (struct obj_vert));
    memset(del, 0, vc * sizeof (int));

    if (1)
    {
        /* Initialize the source index mapping. */

        for (vi = 0; vi < vc; ++vi)
            src_map[vi] = vi;

        /* Rearrange the vertex indices to sort the vertices. */

        qsort_r(src_map, vc, sizeof (int), vv, obj_cmp_vert);

        /* Copy the source map to the destination. */

        for (vi = 0; vi < vc; ++vi)
            dst_map[vi] = src_map[vi];

        /* Invert the index mappings. */

        for (vi = 0; vi < vc; ++vi)
        {
            src_inv[src_map[vi]] = vi;
            dst_inv[dst_map[vi]] = vi;
        }

        /* Eliminate duplicate vertices from the destination maps. */

        for (vi = 0, vj = 1; vj < vc; ++vi, ++vj)
            if (obj_cmp_vert(vv, src_map + vi, src_map + vj) == 0)
            {
                dst_inv[src_map[vj]] = dst_inv[src_map[vi]];
                dst_map[vj]          = dst_map[vi];

                del[vj] = 1;
            }

        /* Compress the indices to remove gaps. */

        for (vi = vc - 1; vi >= 0; --vi)
            if (del[vi])
            {
                for (vj = vc - 1; vj >= 0; --vj)
                    if (dst_inv[vj] > vi)
                        dst_inv[vj]--;
            }

        /* Create the reordered vertex array. */

        for (vi = 0; vi < vc; ++vi)
            vv[dst_inv[vi]] = wv[vi];

        /* Determine the final number of vertices. */

        for (vi = 0; vi < vc; ++vi)
            wc = (wc > dst_inv[vi] + 1)
                ? wc : dst_inv[vi] + 1;

        file(fi)->vc = wc;

        /* Replace vertex indices in the polygons and lines. */

        for (si = 0; si < file(fi)->sc; ++si)
        {
            for (pi = 0; pi < surf(fi, si)->pc; ++pi)
            {
                poly(fi, si, pi)->vi[0] = dst_inv[poly(fi, si, pi)->vi[0]];
                poly(fi, si, pi)->vi[1] = dst_inv[poly(fi, si, pi)->vi[1]];
                poly(fi, si, pi)->vi[2] = dst_inv[poly(fi, si, pi)->vi[2]];
            }
            for (li = 0; li < surf(fi, si)->lc; ++li)
            {
                line(fi, si, li)->vi[0] = dst_inv[line(fi, si, li)->vi[0]];
                line(fi, si, li)->vi[1] = dst_inv[line(fi, si, li)->vi[1]];
            }
        }
    }

    free(dst_inv);
    free(src_inv);
    free(dst_map);
    free(src_map);
}

/*---------------------------------------------------------------------------*/

void obj_sort_file(int fi, int qc)
{
    const int vc = file(fi)->vc;

    struct vert
    {
        int  qs;    /* Cache insertion serial number */
        int *iv;    /* Polygon reference list buffer */
        int  ic;    /* Polygon reference list length */
    };

    /* Vertex optimization data; vertex FIFO cache */

    struct vert *vv = (struct vert *) malloc(vc * sizeof (struct vert));
    int         *qv = (int         *) malloc(qc * sizeof (int        ));

    int qs = 1;   /* Current cache insertion serial number */
    int qi = 0;   /* Current cache insertion point [0, qc) */

    int si;
    int pi;
    int vi;
    int ii;
    int qj;

    /* Initialize the vertex cache to empty. */

    for (qj = 0; qj < qc; ++qj)
        qv[qj] = -1;

    /* Process each surface of this file in turn. */

    for (si = 0; si < file(fi)->sc; ++si)
    {
        const int pc = surf(fi, si)->pc;

        /* Allocate the polygon reference list buffers. */

        int *ip, *iv = (int *) malloc(3 * pc * sizeof (int));

        /* Count the number of polygon references per vertex. */

        memset(vv, 0, vc * sizeof (struct vert));

        for (pi = 0; pi < pc; ++pi)
        {
            const int *i = poly(fi, si, pi)->vi;

            vv[i[0]].ic++;
            vv[i[1]].ic++;
            vv[i[2]].ic++;
        }

        /* Initialize all vertex optimization data. */

        for (vi = 0, ip = iv; vi < vc; ++vi)
        {
            vv[vi].qs = -qc;
            vv[vi].iv =  ip;
            ip += vv[vi].ic;
            vv[vi].ic =   0;
        }

        /* Fill the polygon reference list buffers. */

        for (pi = 0; pi < pc; ++pi)
        {
            const int *i = poly(fi, si, pi)->vi;

            vv[i[0]].iv[vv[i[0]].ic++] = pi;
            vv[i[1]].iv[vv[i[1]].ic++] = pi;
            vv[i[2]].iv[vv[i[2]].ic++] = pi;
        }

        /* Iterate over the polygon array of this surface. */

        for (pi = 0; pi < pc; ++pi)
        {
            const int *i = poly(fi, si, pi)->vi;

            int qd = qs - qc;

            int dk = -1;    /* The best polygon score */
            int pk = pi;    /* The best polygon index */

            /* Find the best polygon among those referred-to by the cache. */

            for (qj = 0; qj < qc; ++qj)
                if (qv[qj] >= 0)

                    for (ii = 0;  ii < vv[qv[qj]].ic; ++ii)
                    {
                        int pj = vv[qv[qj]].iv[ii];
                        int dj = 0;

                        const int *j = poly(fi, si, pj)->vi;

                        /* Recently-used vertex bonus. */

                        if (vv[j[0]].qs > qd) dj += vv[j[0]].qs - qd;
                        if (vv[j[1]].qs > qd) dj += vv[j[1]].qs - qd;
                        if (vv[j[2]].qs > qd) dj += vv[j[2]].qs - qd;

                        /* Low-valence vertex bonus. */

                        dj -= vv[j[0]].ic;
                        dj -= vv[j[1]].ic;
                        dj -= vv[j[2]].ic;

                        if (dk < dj)
                        {
                            dk = dj;
                            pk = pj;
                        }
                    }

            if (pk != pi)
            {
                /* Update the polygon reference list. */

                for (vi = 0; vi < 3; ++vi)
                    for (ii = 0; ii < vv[i[vi]].ic; ++ii)
                        if (vv[i[vi]].iv[ii] == pi)
                        {
                            vv[i[vi]].iv[ii] =  pk;
                            break;
                        }

                /* Swap the best polygon into the current position. */

                struct obj_poly t = *poly(fi, si, pi);
                *poly(fi, si, pi) = *poly(fi, si, pk);
                *poly(fi, si, pk) =                 t;
            }

            /* Iterate over the current polygon's vertices. */

            for (vi = 0; vi < 3; ++vi)
            {
                struct vert *vp = vv + i[vi];

                /* If this vertex was a cache miss then queue it. */

                if (qs - vp->qs >= qc)
                {
                    vp->qs = qs++;
                    qv[qi] = i[vi];
                    qi = (qi + 1) % qc;
                }

                /* Remove the current polygon from the reference list. */

                vp->ic--;

                for (ii = 0; ii < vp->ic; ++ii)
                    if (vp->iv[ii] == pk)
                    {
                        vp->iv[ii] = vp->iv[vp->ic];
                        break;
                    }
            }
        }
        free(iv);
    }
    free(qv);
    free(vv);
}

float obj_acmr_file(int fi, int qc)
{
    int *vs = (int *) malloc(file(fi)->vc * sizeof (int));
    int  qs = 1;

    int si;
    int vi;
    int pi;

    int nn = 0;
    int dd = 0;

    for (si = 0; si < file(fi)->sc; ++si)
    {
        for (vi = 0; vi < file(fi)->vc; ++vi)
            vs[vi] = -qc;

        for (pi = 0; pi < surf(fi, si)->pc; ++pi)
        {
            const int *i = poly(fi, si, pi)->vi;

            if (qs - vs[i[0]] >= qc) { vs[i[0]] = qs++; nn++; }
            if (qs - vs[i[1]] >= qc) { vs[i[1]] = qs++; nn++; }
            if (qs - vs[i[2]] >= qc) { vs[i[2]] = qs++; nn++; }

            dd++;
        }
    }

    return (float) nn / (float) dd;
}

/*---------------------------------------------------------------------------*/

static void obj_draw_prop(int fi, int mi, int ki)
{
    struct obj_prop *kp = prop(fi, mi, ki);

    if (kp->map)
    {
        GLenum wrap = GL_REPEAT;

        /* Bind the property map. */

        glBindTexture(GL_TEXTURE_2D, kp->map);
        glEnable(GL_TEXTURE_2D);

        /* Apply the property options. */

        if (kp->opt & OBJ_OPT_CLAMP)
            wrap = GL_CLAMP_TO_EDGE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

        /* Apply the texture coordinate offset and scale. */

        glMatrixMode(GL_TEXTURE);
        {
            glLoadIdentity();
            glTranslatef(kp->o[0], kp->o[1], kp->o[2]);
            glScalef    (kp->s[0], kp->s[1], kp->s[2]);
        }
        glMatrixMode(GL_MODELVIEW);
    }
}

#define OFFSET(i) ((char *) NULL + (i))

void obj_draw_vert(int fi)
{
    GLsizei s = sizeof (struct obj_vert);

    obj_init_gl();

    /* Enable all necessary vertex attribute pointers. */

    glEnableVertexAttribArray(6);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
     
    if (GL_has_vertex_buffer_object)
    {
        /* Bind attributes to a vertex buffer object. */

        glBindBuffer(GL_ARRAY_BUFFER_ARB, file(fi)->vbo);

        glVertexAttribPointer(6, 3, GL_FLOAT, 0, s, OFFSET(0));
        glNormalPointer      (      GL_FLOAT,    s, OFFSET(12));
        glTexCoordPointer    (2,    GL_FLOAT,    s, OFFSET(24));
        glVertexPointer      (3,    GL_FLOAT,    s, OFFSET(32));
    }
    else
    {
        /* Bind attributes in main memory. */

        glVertexAttribPointer(6, 3, GL_FLOAT, 0, s, file(fi)->vv->u);
        glNormalPointer      (      GL_FLOAT,    s, file(fi)->vv->n);
        glTexCoordPointer    (2,    GL_FLOAT,    s, file(fi)->vv->t);
        glVertexPointer      (3,    GL_FLOAT,    s, file(fi)->vv->v);
    }
}

void obj_draw_mtrl(int fi, int mi)
{
    obj_init_gl();

    /* Bind as many of the texture maps as the GL implementation will allow. */

    if (GL_has_multitexture)
    {
        if (GL_max_texture_image_units > 3 && obj_get_mtrl_map(fi, mi, OBJ_NS))
        {
            glActiveTexture(GL_TEXTURE3_ARB);
            obj_draw_prop(fi, mi, OBJ_NS);
        }
        if (GL_max_texture_image_units > 2 && obj_get_mtrl_map(fi, mi, OBJ_KS))
        {
            glActiveTexture(GL_TEXTURE2_ARB);
            obj_draw_prop(fi, mi, OBJ_KS);
        }
        if (GL_max_texture_image_units > 1 && obj_get_mtrl_map(fi, mi, OBJ_KA))
        {
            glActiveTexture(GL_TEXTURE1_ARB);
            obj_draw_prop(fi, mi, OBJ_KA);
        }
        if (GL_max_texture_image_units > 0 && obj_get_mtrl_map(fi, mi, OBJ_KD))
        {
            glActiveTexture(GL_TEXTURE0_ARB);
            obj_draw_prop(fi, mi, OBJ_KD);
        }
    }
    else obj_draw_prop(fi, mi, OBJ_KD);

    /* Apply the material properties. */

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   prop(fi, mi, OBJ_KD)->c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   prop(fi, mi, OBJ_KA)->c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  prop(fi, mi, OBJ_KE)->c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  prop(fi, mi, OBJ_KS)->c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, prop(fi, mi, OBJ_NS)->c);
}

void obj_draw_surf(int fi, int si)
{
    struct obj_surf *sp = surf(fi, si);

    obj_init_gl();

    if (0 < sp->pc || sp->lc > 0)
    {
        /* Apply this surface's material. */

        if (0 <= sp->mi && sp->mi < file(fi)->mc)
            obj_draw_mtrl(fi, sp->mi);

        /* Render all polygons. */

        if (sp->pibo)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, sp->pibo);
            glDrawElements(GL_TRIANGLES, 3 * sp->pc,
                           GL_UNSIGNED_INT, OFFSET(0));
        }

        /* Render all lines. */

        if (sp->libo)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, sp->libo);
            glDrawElements(GL_LINES, 2 * sp->lc,
                           GL_UNSIGNED_INT, OFFSET(0));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}

void obj_draw_file(int fi)
{
    int si;

    assert_file(fi);

    obj_init_gl();
    obj_init_file(fi);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);
    {
        glDisable(GL_COLOR_MATERIAL);

        /* Load the vertex buffer. */

        obj_draw_vert(fi);
        
        /* Render each surface. */

        for (si = 0; si < file(fi)->sc; ++si)
            obj_draw_surf(fi, si);
    }
    glPopAttrib();
    glPopClientAttrib();
}

void obj_draw_axes(int fi, float k)
{
    int vi;

    assert_file(fi);

    obj_init_gl();
    obj_init_file(fi);

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);

        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINES);
        {
            for (vi = 0; vi < file(fi)->vc; ++vi)
            {
                const float *p = vert(fi, vi)->v;
                const float *x = vert(fi, vi)->u;
                const float *z = vert(fi, vi)->n;

                float y[3];

                /* Compute the bitangent vector. */

                y[0] = z[1] * x[2] - z[2] * x[1];
                y[1] = z[2] * x[0] - z[0] * x[2];
                y[2] = z[0] * x[1] - z[1] * x[0];

                /* Render the tangent-bitangent-normal basis. */

                glColor3f(1.0f, 0.0f, 0.0f);

                glVertex3f(p[0],            p[1],            p[2]);
                glVertex3f(p[0] + x[0] * k, p[1] + x[1] * k, p[2] + x[2] * k);

                glColor3f(0.0f, 1.0f, 0.0f);

                glVertex3f(p[0],            p[1],            p[2]);
                glVertex3f(p[0] + y[0] * k, p[1] + y[1] * k, p[2] + y[2] * k);

                glColor3f(0.0f, 0.0f, 1.0f);

                glVertex3f(p[0],            p[1],            p[2]);
                glVertex3f(p[0] + z[0] * k, p[1] + z[1] * k, p[2] + z[2] * k);
            }
        }
        glEnd();
    }
    glPopAttrib();
}

/*===========================================================================*/

void obj_scale_file(int fi, float s)
{
    int vi;

    assert_file(fi);

    /* Apply the given scale factor to all vertices. */

    for (vi = 0; vi < file(fi)->vc; ++vi)
    {
        vert(fi, vi)->v[0] *= s;
        vert(fi, vi)->v[1] *= s;
        vert(fi, vi)->v[2] *= s;
    }
}

void obj_bound_file(int fi, float b[6])
{
    int vi;

    assert_file(fi);

    /* Compute the bounding box of this object. */

    if (file(fi)->vc > 0)
    {
        const float *v = vert(fi, 0)->v;

        b[0] = b[3] = v[0];
        b[1] = b[4] = v[1];
        b[2] = b[5] = v[2];
    }

    for (vi = 0; vi < file(fi)->vc; ++vi)
    {
        const float *v = vert(fi, vi)->v;

        if (b[0] > v[0]) b[0] = v[0];
        if (b[1] > v[1]) b[1] = v[1];
        if (b[2] > v[2]) b[2] = v[2];

        if (b[3] < v[0]) b[3] = v[0];
        if (b[4] < v[1]) b[4] = v[1];
        if (b[5] < v[2]) b[5] = v[2];
    }
}

/*===========================================================================*/

static void obj_write_map(FILE *fout, int fi, int mi, int ki, const char *s)
{
    struct obj_prop *kp = prop(fi, mi, ki);

    /* If this property has a map... */

    if (kp->str)
    {
        fprintf(fout, "map_%s ", s);

        /* Store all map options. */

        if ((kp->opt & OBJ_OPT_CLAMP) != 0) fprintf(fout, "-clamp on ");

        /* Store the map offset, if any. */

        if (fabs(kp->o[0]) > 0 ||
            fabs(kp->o[1]) > 0 ||
            fabs(kp->o[2]) > 0) fprintf(fout, "-o %f %f %f ",
                                        kp->o[0], kp->o[1], kp->o[2]);

        /* Store the map scale, if any. */

        if (fabs(kp->s[0] - 1) > 0 ||
            fabs(kp->s[1] - 1) > 0 ||
            fabs(kp->s[2] - 1) > 0) fprintf(fout, "-s %f %f %f ",
                                            kp->s[0], kp->s[1], kp->s[2]);

        /* Store the map image file name. */

        fprintf(fout, "%s\n", kp->str);
    }
}

static void obj_write_mtl(int fi, const char *mtl)
{
    FILE *fout;
    int   mi;

    if ((fout = fopen(mtl, "w")))
    {
        for (mi = 0; mi < file(fi)->mc; ++mi)
        {
            struct obj_mtrl *mp = mtrl(fi, mi);

            /* Start a new material. */

            if (mp->name)
                fprintf(fout, "newmtl %s\n", mp->name);
            else
                fprintf(fout, "newmtl default\n");

            /* Store all material property colors. */

            fprintf(fout, "Kd %12.8f %12.8f %12.8f\n", mp->kv[OBJ_KD].c[0],
                                                       mp->kv[OBJ_KD].c[1],
                                                       mp->kv[OBJ_KD].c[2]);
            fprintf(fout, "Ka %12.8f %12.8f %12.8f\n", mp->kv[OBJ_KA].c[0],
                                                       mp->kv[OBJ_KA].c[1],
                                                       mp->kv[OBJ_KA].c[2]);
            fprintf(fout, "Ke %12.8f %12.8f %12.8f\n", mp->kv[OBJ_KE].c[0],
                                                       mp->kv[OBJ_KE].c[1],
                                                       mp->kv[OBJ_KE].c[2]);
            fprintf(fout, "Ks %12.8f %12.8f %12.8f\n", mp->kv[OBJ_KS].c[0],
                                                       mp->kv[OBJ_KS].c[1],
                                                       mp->kv[OBJ_KS].c[2]);

            fprintf(fout, "Ns %12.8f\n", mp->kv[OBJ_NS].c[0]);
            fprintf(fout, "d  %12.8f\n", mp->kv[OBJ_KD].c[3]);

            /* Store all material property maps. */

            obj_write_map(fout, fi, mi, OBJ_KD, "Kd");
            obj_write_map(fout, fi, mi, OBJ_KA, "Ka");
            obj_write_map(fout, fi, mi, OBJ_KA, "Ke");
            obj_write_map(fout, fi, mi, OBJ_KS, "Ks");
            obj_write_map(fout, fi, mi, OBJ_NS, "Ns");
        }
    }
    fclose(fout);
}

static void obj_write_obj(int fi, const char *obj, const char *mtl)
{
    FILE *fout;

    if ((fout = fopen(obj, "w")))
    {
        int si;
        int vi;
        int pi;
        int li;

        if (mtl) fprintf(fout, "mtllib %s\n", mtl);

        /* Store all vertex data. */

        for (vi = 0; vi < file(fi)->vc; ++vi)
            fprintf(fout, "v  %12.8f %12.8f %12.8f\n", vert(fi, vi)->v[0],
                                                       vert(fi, vi)->v[1],
                                                       vert(fi, vi)->v[2]);
        for (vi = 0; vi < file(fi)->vc; ++vi)
            fprintf(fout, "vt %12.8f %12.8f\n",        vert(fi, vi)->t[0],
                                                       vert(fi, vi)->t[1]);
        for (vi = 0; vi < file(fi)->vc; ++vi)
            fprintf(fout, "vn %12.8f %12.8f %12.8f\n", vert(fi, vi)->n[0],
                                                       vert(fi, vi)->n[1],
                                                       vert(fi, vi)->n[2]);

        for (si = 0; si < file(fi)->sc; ++si)
        {
            int mi = surf(fi, si)->mi;

            /* Store the surface's material reference */

            if (0 <= mi && mi < file(fi)->mc && mtrl(fi, mi)->name)
                fprintf(fout, "usemtl %s\n", mtrl(fi, surf(fi, si)->mi)->name);
            else
                fprintf(fout, "usemtl default\n");

            /* Store all polygon definitions. */

            for (pi = 0; pi < surf(fi, si)->pc; pi++)
            {
                int vi0 = poly(fi, si, pi)->vi[0] + 1;
                int vi1 = poly(fi, si, pi)->vi[1] + 1;
                int vi2 = poly(fi, si, pi)->vi[2] + 1;

                fprintf(fout, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", vi0, vi0, vi0,
                                                                vi1, vi1, vi1,
                                                                vi2, vi2, vi2);
            }

            /* Store all line definitions. */

            for (li = 0; li < surf(fi, si)->lc; li++)
            {
                int vi0 = line(fi, si, li)->vi[0] + 1;
                int vi1 = line(fi, si, li)->vi[1] + 1;

                fprintf(fout, "l %d/%d/%d %d/%d/%d\n", vi0, vi0, vi0,
                                                       vi1, vi1, vi1);
            }
        }

        fclose(fout);
    }
}

void obj_write_file(int fi, const char *obj, const char *mtl)
{
    assert_file(fi);

    if (obj) obj_write_obj(fi, obj, mtl);
    if (mtl) obj_write_mtl(fi, mtl);
}

/*===========================================================================*/
