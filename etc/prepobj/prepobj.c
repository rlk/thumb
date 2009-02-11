#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj.h"

/*---------------------------------------------------------------------------*/

static const char *toxml(const char *image, const char *dir)
{
    static char newname[256];
    static char xmlname[256];
    char *c;
    FILE *fp;

    assert(image);

    if (dir)
    {
        strcpy(newname, dir);
        strcat(newname, "/");
        strcat(newname, image);
    }
    else
        strcpy(newname, image);

    strcpy(xmlname, image);

    if ((c = strrchr(newname, '.'))) *c = '\0';
    if ((c = strrchr(xmlname, '.'))) *c = '\0';

    strcat(xmlname, ".xml");

    if ((fp = fopen(xmlname, "w")))
    {
        fprintf(fp, "<?xml version=\"1.0\"?>\n");
        fprintf(fp, "<material>\n");
        fprintf(fp, "  <program mode=\"depth\"    file=\"object-depth.xml\"/>\n");
        fprintf(fp, "  <program mode=\"color\"    file=\"object-color.xml\"/>\n");
        fprintf(fp, "  <texture name=\"specular\" file=\"matte-specular.png\"/>\n");

        if (dir)
            fprintf(fp, "  <texture name=\"diffuse\"  file=\"%s/%s\"/>\n", dir, image);
        else
            fprintf(fp, "  <texture name=\"diffuse\"  file=\"%s\"/>\n", image);

        fprintf(fp, "  <texture name=\"normal\"   file=\"default-normal.png\"/>\n");
        fprintf(fp, "</material>\n");

        fclose(fp);
    }

    return newname;
}

static int load(const char *filename, float scale)
{
    int fi = obj_add_file(filename);
    int qc = 32;

    obj_scale_file(fi, scale);

    printf("%s initial vertex count: %d\n", filename, obj_num_vert(fi));
    obj_uniq_file(fi);
    printf("%s final   vertex count: %d\n", filename, obj_num_vert(fi));

    printf("%s initial ACMR: %f\n", filename, obj_acmr_file(fi, qc));
    obj_sort_file(fi, qc);
    printf("%s final   ACMR: %f\n", filename, obj_acmr_file(fi, qc));

    return fi;
}

static void mtltoxml(const char *in,
                     const char *out,
                     const char *dir, float scale)
{
    int fi = load(in, scale);
    int mc = obj_num_mtrl(fi);
    int mi;

    for (mi = 0; mi < mc; ++mi)
    {
        const char *image = obj_get_mtrl_map(fi, mi, OBJ_KD);

        if (image)
        {
            const char *name = toxml(image, dir);
            obj_set_mtrl_name(fi, mi, name);
        }
        else
            obj_set_mtrl_name(fi, mi, "default");
    }

    obj_write_file(fi, out, NULL);
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    if      (argc > 4) mtltoxml(argv[1], argv[2], argv[3], atof(argv[4]));
    else if (argc > 3) mtltoxml(argv[1], argv[2], argv[3], 1.0f);
    else if (argc > 2) mtltoxml(argv[1], argv[2], NULL,    1.0f);
    else
        fprintf(stderr, "Usage: %s <in.obj> <out.obj> [dir] [scale]\n", argv[0]);

    return 0;
}

/*---------------------------------------------------------------------------*/

