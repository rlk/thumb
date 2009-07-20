#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <png.h>

//-----------------------------------------------------------------------------

struct page_s
{
    uint32_t sub[4];
    uint16_t min[3];
    uint16_t max[3];
    float    err;
};

struct vert_s
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t u;
};

typedef struct page_s page;
typedef struct vert_s vert;

//-----------------------------------------------------------------------------

static void fail(const char * restrict error)
{
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

// Test whether N is a power of two.

static int is_power_of_two(int n)
{
    return ((n & (n - 1)) == 0);
}

// Determine the total number of NxN pages in the mipmap hierarchy of an image
// of size WxH.

static int number_of_pages(int n, int w, int h)
{
    const int r = (h > n) ? ((h - 1) / (n - 1)) : 1;
    const int c = (w > n) ? ((w - 1) / (n - 1)) : 1;

    if (h > n || w > n)
        return (r * c) + number_of_pages(n, (w - 1) / 2 + 1,
                                            (h - 1) / 2 + 1);
    else
        return (r * c);
}

//-----------------------------------------------------------------------------

static void *read_png(const char * restrict filename, int * restrict w,
                                                      int * restrict h,
                                                      int * restrict c,
                                                      int * restrict b)
{
    FILE         *fp = NULL;
    png_structp   rp = NULL;
    png_infop     ip = NULL;
    png_bytep    *bp = NULL;
    unsigned char *p = NULL;

    // Initialize all PNG import data structures.

    if (!(fp = fopen(filename, "rb")))
        fail(strerror(errno));

    if (!(rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        fail("Failure to allocate PNG read structure");

    if (!(ip = png_create_info_struct(rp)))
        fail("Failure to allocate PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(rp)) == 0)
    {
        // Read the PNG header.

        png_init_io (rp, fp);
        png_read_png(rp, ip, PNG_TRANSFORM_SWAP_ENDIAN, NULL);
        
        *w = (int) png_get_image_width (rp, ip);
        *h = (int) png_get_image_height(rp, ip);
        *c = (int) png_get_channels    (rp, ip);
        *b = (int) png_get_bit_depth   (rp, ip) >> 3;

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            // Allocate the final buffer and flip-copy the pixels there.

            int i, j, s = png_get_rowbytes(rp, ip);

            if ((p = (unsigned char *) malloc((*w) * (*h) * (*c) * (*b))))
                for (i = 0, j = *h - 1; j >= 0; ++i, --j)
                    memcpy(p + s * i, bp[j], s);
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, NULL);
    fclose(fp);

    return p;
}

//-----------------------------------------------------------------------------

// Copy a subsampling of a portion of a buffer.  N is the destination size.
// W is the source width.  R and C are the row and column of source corner.
// S is the pixel skip.

static void copy(uint16_t * restrict dst, int n,
           const uint16_t * restrict src, int w, int r, int c, int s)
{
    int i;
    int j;

    for     (i = 0; i < n; ++i)
        for (j = 0; j < n; ++j)
            dst[i * n + j] = src[(r + i * s) * w + (c + j * s)];
}

static void step(page * restrict head,
                 vert * restrict data, int n,
                 const uint16_t * restrict src,
                 int w, int h, int r, int c, int s)
{
    if (s > 0)
    {
    }
}

// Process PNG file SRC, writing GMM file DST with page size N and error
// bound E.

static void proc(const char * restrict src,
                 const char * restrict dst, int n, float e)
{
    // Load the input image file.

    uint16_t *p;
    int w;
    int h;
    int c;
    int b;

    if ((p = (uint16_t *) read_png(src, &w, &h, &c, &b)))
    {
        int m = w > h ? w : h;

        // Validate all inputs.

        if (!is_power_of_two(n - 1))
            fail("Output page size must be power of 2 plus 1");

        if (!is_power_of_two(w - 1) ||
            !is_power_of_two(h - 1))
            fail("Input image size must be power of 2 plus 1");

        if (c != 1) fail("Input image color must be grayscale");
        if (b != 2) fail("Input image depth must be 16 bit");

        // Allocate storage for the generated data.

        int count = number_of_pages(n, w, h);

        page *head = calloc(count,         sizeof (page));
        vert *data = calloc(count * n * n, sizeof (vert));

        // Convert.

        step(head, data, n, p, w, h, 0, 0, (m - 1) / (n - 1));

        free(p);
    }
}

//-----------------------------------------------------------------------------

static void usage(const char * restrict name)
{
    fprintf(stderr, "usage: %s src.png dst.gmm n e\n", name);
    fprintf(stderr, "\tsrc.png ... input  PNG image\n");
    fprintf(stderr, "\tdst.gmm ... output GMM file\n");
    fprintf(stderr, "\tn       ... output page size\n");
    fprintf(stderr, "\te       ... output error bound\n");
}

int main(int argc, char *argv[])
{
    if (argc == 5)
        proc(argv[1], argv[2], (int)   strtol(argv[3], NULL, 0),
                               (float) strtod(argv[4], NULL));
    else
        usage(argv[0]);

    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
