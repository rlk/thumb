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

#ifndef OBJ_H
#define OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#define OBJ_KD 0
#define OBJ_KA 1
#define OBJ_KE 2
#define OBJ_KS 3
#define OBJ_NS 4

#define OBJ_OPT_CLAMP  1

/*---------------------------------------------------------------------------*/

int  obj_add_mtrl(int);
int  obj_add_vert(int);
int  obj_add_poly(int, int);
int  obj_add_line(int, int);
int  obj_add_surf(int);
int  obj_add_file(const char *);

int  obj_num_mtrl(int);
int  obj_num_vert(int);
int  obj_num_poly(int, int);
int  obj_num_line(int, int);
int  obj_num_surf(int);
int  obj_num_file(void);

void obj_del_mtrl(int, int);
void obj_del_vert(int, int);
void obj_del_poly(int, int, int);
void obj_del_line(int, int, int);
void obj_del_surf(int, int);
void obj_del_file(int);

/*---------------------------------------------------------------------------*/

void obj_set_mtrl_name(int, int,      const char *);
void obj_set_mtrl_map (int, int, int, const char *);
void obj_set_mtrl_opt (int, int, int, unsigned int);
void obj_set_mtrl_c   (int, int, int, const float[4]);
void obj_set_mtrl_o   (int, int, int, const float[3]);
void obj_set_mtrl_s   (int, int, int, const float[3]);

void obj_set_vert_v(int, int, const float[3]);
void obj_set_vert_t(int, int, const float[2]);
void obj_set_vert_n(int, int, const float[3]);

void obj_set_poly(int, int, int, const int[3]);
void obj_set_line(int, int, int, const int[2]);
void obj_set_surf(int, int, int);

/*---------------------------------------------------------------------------*/

const char  *obj_get_file_name(int);
const char  *obj_get_mtrl_name(int, int);
unsigned int obj_get_mtrl_map (int, int, int);
unsigned int obj_get_mtrl_opt (int, int, int);
void         obj_get_mtrl_c   (int, int, int, float[4]);
void         obj_get_mtrl_o   (int, int, int, float[3]);
void         obj_get_mtrl_s   (int, int, int, float[3]);

void obj_get_vert_v(int, int, float[3]);
void obj_get_vert_t(int, int, float[2]);
void obj_get_vert_n(int, int, float[3]);

void obj_get_poly(int, int, int, int[3]);
void obj_get_line(int, int, int, int[2]);
int  obj_get_surf(int, int);

/*---------------------------------------------------------------------------*/

void  obj_get_file_aabb(int, float[6]);
float obj_get_file_sphr(int);

/*---------------------------------------------------------------------------*/

void obj_mini_file(int);
void obj_norm_file(int);
void obj_proc_file(int);
void obj_init_file(int);

void obj_draw_vert(int);
void obj_draw_mtrl(int, int);
void obj_draw_surf(int, int);
void obj_draw_file(int);
void obj_draw_axes(int, float);

void  obj_write_file(int, const char *, const char *);
void *obj_read_image(const char *, int *, int *, int *);

void obj_reset_all();

/*===========================================================================*/

#ifdef __cplusplus
}
#endif
#endif
