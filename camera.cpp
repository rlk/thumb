#include <iostream>

#include "main.hpp"
#include "opengl.hpp"
#include "camera.hpp"

//-----------------------------------------------------------------------------

ent::camera::camera() : free(-1)
{
    move_world(0, 2, 5);
}

//-----------------------------------------------------------------------------

void ent::camera::proj() const
{
    int   w = conf->get_i("window_w");
    int   h = conf->get_i("window_h");
    float f = conf->get_f("camera_far");
    float n = conf->get_f("camera_near");
    float z = conf->get_f("camera_zoom");

    float a = float(w) / float(h);

    // Apply the camera projection.

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glFrustum(-a * z, +a * z, -z, +z, n, f);
    }
    glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------

void ent::camera::pick(float p[3], float v[3], int x, int y) const
{
    int   w = conf->get_i("window_w");
    int   h = conf->get_i("window_h");
    float n = conf->get_f("camera_near");
    float z = conf->get_f("camera_zoom");

    // Compute the screen-space pointer vector.

    float r = +(2.0f * x / w - 1.0f) * z * w / h;
    float u = -(2.0f * y / h - 1.0f) * z;

    // Transform the pointer to camera space and normalize.

    p[0] = current_M[12];
    p[1] = current_M[13];
    p[2] = current_M[14];

    v[0] = current_M[0] * r + current_M[4] * u - current_M[ 8] * n;
    v[1] = current_M[1] * r + current_M[5] * u - current_M[ 9] * n;
    v[2] = current_M[2] * r + current_M[6] * u - current_M[10] * n;

    float k = 1.0f / float(sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));

    v[0] *= k;
    v[1] *= k;
    v[2] *= k;

    // Apply the camera position and pointer vector to the picking ray.

    dGeomRaySet(point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

//-----------------------------------------------------------------------------
