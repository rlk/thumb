#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj.h"

/*---------------------------------------------------------------------------*/

int strrep(char *dst, const char *src, const char *A, const char *B)
{
    char *rep;

    if ((rep = strstr(src, A)))
    {
        strncpy(dst, src, rep - src);
        if (B) strcat(dst, B);
        strcat(dst, rep + strlen(A));

        return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static const char *toxml(const char *origname, const char *dir)
{
    static char mtrlname[256];

    char filename[256];
    char diffname[256];
    char normname[256];

    FILE *fp;

    assert(origname);

    memset(mtrlname, 0, 256);
    memset(filename, 0, 256);
    memset(diffname, 0, 256);
    memset(normname, 0, 256);

    /* foo_diffuse.png -> dir/foo_diffuse.png */

    if (dir)
    {
        strcpy(diffname, dir);
        strcat(diffname, "/");
    }
    strcat(diffname, origname);

    /* foo_diffuse.png -> foo.xml */

    strrep(filename, origname, "_diffuse.png", ".xml");

    /* dir/foo_diffuse.png -> dir/foo */

    strrep(mtrlname, diffname, "_diffuse.png", 0);

    /* dir/foo_diffuse.png -> dir/foo_normal.xml */

    strrep(normname, diffname, "_diffuse.png", "_normal.png");

    /* Write the XML */

    if ((fp = fopen(filename, "w")))
    {
        fprintf(fp, "<?xml version=\"1.0\"?>\n");
        fprintf(fp, "<material>\n");
        fprintf(fp, "  <program mode=\"depth\"    file=\"object-depth.xml\">\n");
        fprintf(fp, "    <texture sampler=\"diff_map\"  name=\"%s\"/>\n", diffname);
        fprintf(fp, "  </program mode>\n");
        fprintf(fp, "  <program mode=\"color\"    file=\"object-color.xml\">\n");
        fprintf(fp, "    <texture sampler=\"spec_map\"  name=\"matte-specular.png\"/>\n");
        fprintf(fp, "    <texture sampler=\"diff_map\"  name=\"%s\"/>\n", diffname);
        fprintf(fp, "    <texture sampler=\"norm_map\"  name=\"%s\"/>\n", normname);
        fprintf(fp, "    <process sampler=\"shadow[0]\" name=\"shadow0\"/>\n");
        fprintf(fp, "    <process sampler=\"shadow[1]\" name=\"shadow1\"/>\n");
        fprintf(fp, "    <process sampler=\"shadow[2]\" name=\"shadow2\"/>\n");
        fprintf(fp, "  </program mode>\n");
        fprintf(fp, "</material>\n");

        fclose(fp);
    }

    return mtrlname;
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

