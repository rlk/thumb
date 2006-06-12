#include "util.hpp"
#include "opengl.hpp"
#include "matrix.hpp"
#include "constraint.hpp"

//-----------------------------------------------------------------------------

constraint::constraint()
{
    set_grid(3);
    set_mode(0);
    set_axis(1);

    load_idt(M);
    load_idt(T);

    orient();
}

//-----------------------------------------------------------------------------

void constraint::orient()
{
    T[12] = M[12];
    T[13] = M[13];
    T[14] = M[14];

    switch (axis)
    {
    case 0:
        T[0] = -M[ 8]; T[4] =  M[ 4]; T[ 8] = M[ 0];
        T[1] = -M[ 9]; T[5] =  M[ 5]; T[ 9] = M[ 1];
        T[2] = -M[10]; T[6] =  M[ 6]; T[10] = M[ 2];
        break;
    case 1:
        T[0] =  M[ 0]; T[4] = -M[ 8]; T[ 8] = M[ 4];
        T[1] =  M[ 1]; T[5] = -M[ 9]; T[ 9] = M[ 5];
        T[2] =  M[ 2]; T[6] = -M[10]; T[10] = M[ 6];
        break;
    case 2:
        T[0] =  M[ 0]; T[4] =  M[ 4]; T[ 8] = M[ 8];
        T[1] =  M[ 1]; T[5] =  M[ 5]; T[ 9] = M[ 9];
        T[2] =  M[ 2]; T[6] =  M[ 6]; T[10] = M[10];
        break;
    }
}

void constraint::set_grid(int g)
{
    static const int a[] = {
         1,
         3,
         5,
        10,
        15,
        20,
        30,
        45,
        60,
        90,
    };
    static const float d[] = {
         0.0625f,
         0.1250f,
         0.2500f,
         0.5000f,
         1.0000f,
         2.0000f,
         4.0000f,
         8.0000f,
        16.0000f,
        32.0000f,
    };

    grid_a = a[CLAMP(g, 0, 9)];
    grid_d = d[CLAMP(g, 0, 9)];
}

void constraint::set_mode(int m)
{
    mode = m;
}

void constraint::set_axis(int a)
{
    axis = a;
    orient();
}

void constraint::set_transform(const float A[16])
{
    memcpy(M, A, 16 * sizeof (float));
    orient();
}

//-----------------------------------------------------------------------------

static float snap(float f, float d)
{
    float f0 = float(floor(f / d)) * d;
    float f1 = float(ceil (f / d)) * d;

    return (fabs(f0 - f) < fabs(f1 - f)) ? f0 : f1;
}

void constraint::calc_rot(float& a, float& d, const float p[3],
                                              const float v[3]) const
{
    float q[3], t = (DOT3(T + 12, T + 8) - DOT3(p, T + 8)) / DOT3(v, T + 8);

    q[0] = (p[0] + v[0] * t) - T[12];
    q[1] = (p[1] + v[1] * t) - T[13];
    q[2] = (p[2] + v[2] * t) - T[14];

    float aa = float(DEG(atan2(DOT3(q, T + 4), DOT3(q, T + 0))));
    float dd = float(sqrt(DOT3(q, q)));

    a = snap(aa, float(grid_a));
    d = snap(dd,      (grid_d));
}

void constraint::calc_pos(float& x, float& y, const float p[3],
                                              const float v[3]) const
{
    float q[3], t = (DOT3(T + 12, T + 8) - DOT3(p, T + 8)) / DOT3(v, T + 8);

    q[0] = (p[0] + v[0] * t) - T[12];
    q[1] = (p[1] + v[1] * t) - T[13];
    q[2] = (p[2] + v[2] * t) - T[14];

    float xx = DOT3(q, T + 0);
    float yy = DOT3(q, T + 4);

    x = snap(xx, grid_d);
    y = snap(yy, grid_d);
}

//-----------------------------------------------------------------------------

bool constraint::point(float M[16], const float p[3], const float v[3])
{
    if (mode)
    {
        float a;
        float d;

        calc_rot(a, d, p, v);

        if (fabs(a - mouse_a) > 0.0 || fabs(d - mouse_d) > 0.0)
        {
            load_xlt_inv(M, T[12], T[13], T[14]);
            Lmul_rot_mat(M, T[ 8], T[ 9], T[10], a - mouse_a);
            Lmul_xlt_mat(M, T[12], T[13], T[14]);

            return true;
        }
    }
    else
    {
        float x;
        float y;

        calc_pos(x, y, p, v);

        if (fabs(x - mouse_x) > 0.0 || fabs(y - mouse_y) > 0.0)
        {
            load_xlt_mat(M, T[0] * (x - mouse_x) + T[4] * (y - mouse_y),
                            T[1] * (x - mouse_x) + T[5] * (y - mouse_y),
                            T[2] * (x - mouse_x) + T[6] * (y - mouse_y));
            return true;
        }
    }
    return false;
}

void constraint::click(const float p[3], const float v[3])
{
    calc_rot(mouse_a, mouse_d, p, v);
    calc_pos(mouse_x, mouse_y, p, v);
}

//-----------------------------------------------------------------------------

void constraint::draw_rot(int n) const
{
    int i, j;

    // Draw a radial checker board.

    glBegin(GL_QUADS);
    {
        for (i = 0; i < n; ++i)
            for (j = grid_a * (i & 1); j < 360; j += 2 * grid_a)
            {
                glVertex2f(grid_d * (i    ) * cosi(j),
                           grid_d * (i    ) * sini(j));
                glVertex2f(grid_d * (i + 1) * cosi(j),
                           grid_d * (i + 1) * sini(j));
                glVertex2f(grid_d * (i + 1) * cosi(j + grid_a),
                           grid_d * (i + 1) * sini(j + grid_a));
                glVertex2f(grid_d * (i    ) * cosi(j + grid_a),
                           grid_d * (i    ) * sini(j + grid_a));
            }
    }
    glEnd();
}

void constraint::draw_pos(int n) const
{
    int i, j;

    // Draw a checker board grid.

    glBegin(GL_QUADS);
    {
        for (i = -n; i < n; ++i)
            for (j = -n + (i & 1); j < n; j += 2)
            {
                glVertex2f(-grid_d * (i    ), -grid_d * (j    ));
                glVertex2f(-grid_d * (i    ), -grid_d * (j + 1));
                glVertex2f(-grid_d * (i + 1), -grid_d * (j + 1));
                glVertex2f(-grid_d * (i + 1), -grid_d * (j    ));
            }
    }
    glEnd();
}

void constraint::draw(int n) const
{
    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        // Set up for Z-offset anti-aliased line drawing.

        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

        // Draw the oriented constraint grid.

        glMultMatrixf(T);

        if (mode)
            draw_rot(n);
        else
            draw_pos(n);
    }
    glPopMatrix();
    glPopAttrib();
}

//-----------------------------------------------------------------------------

