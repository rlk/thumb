#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <png.h>

//-----------------------------------------------------------------------------

struct file_s
{
    uint16_t *p;
    uint16_t  w;
    uint16_t  h;
    uint16_t  c;
    uint16_t  b;
};

struct page_s
{
    uint32_t sub[4];
    uint16_t min;
    uint16_t max;
    uint16_t err;
};

struct vert_s
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t u;
};

typedef struct file_s file;
typedef struct page_s page;
typedef struct vert_s vert;

//-----------------------------------------------------------------------------

static void fail(const char * restrict error)
{
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

// Find the extrema of two values.

#define min2(A, B) (((A) < (B)) ? (A) : (B))
#define max2(A, B) (((A) > (B)) ? (A) : (B))

// Find the extrema of three values.

static uint16_t min3(uint16_t a, uint16_t b, uint16_t c)
{
    return min2(min2(a, b), c);
}
static uint16_t max3(uint16_t a, uint16_t b, uint16_t c)
{
    return max2(max2(a, b), c);
}

// Find the extrema of four values.

static uint16_t min4(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
    return min2(min2(a, b), min2(c, d));
}
static uint16_t max4(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
    return max2(max2(a, b), max2(c, d));
}

// Test whether N is a power of two.

static int is_power_of_two(int n)
{
    return ((n & (n - 1)) == 0);
}

// Determine the maximum number of NxN pages in the mipmap hierarchy of an
// image of size WxH.

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

static int read_png(file * restrict file,
              const char * restrict name)
{
    png_structp rp = NULL;
    png_infop   ip = NULL;
    png_bytep  *bp = NULL;
    FILE       *fp = NULL;

    file->p = 0;
    file->w = 0;
    file->h = 0;
    file->b = 0;
    file->c = 0;

    // Initialize all PNG import data structures.

    if (!(fp = fopen(name, "rb")))
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
        
        file->w = (uint16_t) png_get_image_width (rp, ip);
        file->h = (uint16_t) png_get_image_height(rp, ip);
        file->c = (uint16_t) png_get_channels    (rp, ip);
        file->b = (uint16_t) png_get_bit_depth   (rp, ip) >> 3;

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            // Allocate the final buffer and flip-copy the pixels there.

            int i, j, s = png_get_rowbytes(rp, ip);

            if ((file->p = (uint16_t *) malloc((size_t) file->w *
                                               (size_t) file->h *
                                               (size_t) file->c *
                                               (size_t) file->b)))

                for (i = 0, j = file->h - 1; j >= 0; ++i, --j)
                    memcpy(file->p + s * i, bp[j], s);
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, NULL);
    fclose(fp);

    return (file->p ? 1 : 0);
}

//-----------------------------------------------------------------------------

// Copy a subsampling of a portion of a buffer. The destination size is NxN.
// The total source size is WxH. R and C give the row and column of source
// corner. D is the pixel skip. Samples outside of the source give zero.
/*
static void copy(vert * restrict data, 
       const uint16_t * restrict buff, int n, int w, int h,
                                              int r, int c, int d)
{
    int i;
    int j;

    for     (i = 0; i < n; ++i)
        for (j = 0; j < n; ++j)
        {
            const int k = j + i * n;
            const int y = r + i * d;
            const int x = c + j * d;

            int z = (y < h && x < w) ? buff[(r + i * d) * w + (c + j * d)] : 0;

            data[k].x = x;
            data[k].y = y;
            data[k].z = z;
        }
}

static int step(page * restrict head,
                vert * restrict data,
      const uint16_t * restrict buff, int n, int w, int h,
                                      int r, int c, int d, int p, int k)
{
    if (d > 0)
    {
        int i;
        int j;
    }
}
*/
//-----------------------------------------------------------------------------

// Get the value at pixel (R, C) of image FILE.

static uint16_t get_file(file * restrict file, int r, int c)
{
    const int w = (int) file->w;
    const int h = (int) file->h;

    return (r < h && c < w) ? file->p[w * r + c] : 0;
}

// Get the Z value at vertex (R, C) of page K, which has size NxN.

static uint16_t get_vert(vert * restrict vert, int n, int k, int r, int c)
{
    return vert[n * n * k + n * r + c].z;
}

// Put the value (X, Y, Z) in vertex (R, C) of page K, which has size NxN.

static void put_vert(vert * restrict vert, int n, int k, int r, int c,
                                                  int x, int y, int z)
{
    const int i = n * n * k + n * r + c;

    vert[i].x = (uint16_t) x;
    vert[i].y = (uint16_t) y;
    vert[i].z = (uint16_t) z;
    vert[i].u = (uint16_t) z;
}

//-----------------------------------------------------------------------------

// Recursively create the NxN page hierarchy starting at pixel (R, C) of FILE.
// P points to the current page index and D gives the sample density.

static int domake(page * restrict page,
                  vert * restrict vert,
                  file * restrict file, int n, int r, int c, int d, int *p)
{
    // Copy and advance the index counter.

    const int k = (*p)++;

    // Copy the page data.

    for     (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
        {
            const int y = r + i * d;
            const int x = c + j * d;

            put_vert(vert, n, k, i, j, x, y, get_file(file, y, x));
        }

    // Recursively process sub-pages, as necessary.

    if (d > 1)
    {
        const int m = n * d / 2;

        page[k].sub[0] = domake(page, vert, file, n, r,     c,     d / 2, p);
        page[k].sub[1] = domake(page, vert, file, n, r,     c + m, d / 2, p);
        page[k].sub[2] = domake(page, vert, file, n, r + m, c,     d / 2, p);
        page[k].sub[3] = domake(page, vert, file, n, r + m, c + m, d / 2, p);
    }

    return k;
}

// Compute the metadata of VERT[K], which has parent VERT[P] and size NxN.

static void dometa(page * restrict page,
                   vert * restrict vert, int n, int p, int k, int x, int y)
{
    const int k0 = (int) page[k].sub[0];
    const int k1 = (int) page[k].sub[1];
    const int k2 = (int) page[k].sub[2];
    const int k3 = (int) page[k].sub[3];

    int err = 0;

    // If this page has a parent...

    if (k)
    {
        // Compute all geomorph values.  Find the maximum.

        const int di = y * (n - 1) / 2;
        const int dj = x * (n - 1) / 2;

        for     (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                int i0, i1;
                int j0, j1;

                if (i % 2) { i0 = di + (i-1) / 2; i1 = di + (i+1) / 2; }
                else       { i0 = di + (i  ) / 2; i1 = di + (i  ) / 2; }
                if (j % 2) { j0 = dj + (j-1) / 2; j1 = dj + (j+1) / 2; }
                else       { j0 = dj + (j  ) / 2; j1 = dj + (j  ) / 2; }

                uint16_t z  = get_z(vert, n, p, i,  j);
                uint16_t z0 = get_z(vert, n, p, i0, j0);
                uint16_t z1 = get_z(vert, n, p, i1, j1);

                uint16_t u  = (z0 + z1) / 2;

                put_u(vert, n, k, i, j, u);

                err = max2(err, abs(z - u));
            }
    }

    // If this page has children...

    if (k0 && k1 && k2 && k3)
    {
        // Recursively process the children.

        dometa(page, vert, n, k, k0, 0, 0);
        dometa(page, vert, n, k, k1, 0, 1);
        dometa(page, vert, n, k, k2, 1, 0);
        dometa(page, vert, n, k, k3, 1, 1);

        // Take the union of the child bounds.

        page[k].min = min4(page[k0].min, page[k1].min,
                           page[k2].min, page[k3].min);
        page[k].max = max4(page[k0].max, page[k1].max,
                           page[k2].max, page[k3].max);
    }
    else
    {
        // Compute leaf bounds from vertex extreme plus geomorph.

        page[k].min =  32767;
        page[k].max = -32768;

        for     (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                vert *v = vert + n * n * k + n * i + j;

                page[k].min = min3(page[k].min, v->z, v->u);
                page[k].max = max3(page[k].max, v->z, v->u);
            }
    }

    // Push the maximum geomorph magnitude up the tree.

    page[p].err = max3(page[p].err, page[k].err, err);
}

// Process PNG file SRC, writing GMM file DST with page size N and error
// bound E.

static void proc(const char * restrict png,
                 const char * restrict gmm, int n, int e)
{
    // Load the input image file.

    file file;

    if (read_png(&file, png))
    {
        const int w = (int) file.w;
        const int h = (int) file.h;
        const int m = w > h ? w : h;

        // Validate all inputs.

        if (!is_power_of_two(n - 1))
            fail("Output page size must be power of 2 plus 1");

        if (!is_power_of_two(w - 1) ||
            !is_power_of_two(h - 1))
            fail("Input image size must be power of 2 plus 1");

        if (file.c != 1) fail("Input image color must be grayscale");
        if (file.b != 2) fail("Input image depth must be 16 bit");

        // Allocate storage for the generated data.

        int count = number_of_pages(n, w, h);

        page *page = calloc(count,         sizeof (page));
        vert *vert = calloc(count * n * n, sizeof (vert));

        // Convert.

        int d = (m - 1) / (n - 1);
        int p = 0;

        domake(page, vert, &file, n, 0, 0, d, &p);
        dometa(page, vert,        n, 0, 0, 0, 0);

        // Write the output file.

        // (in breadth-first order, limited by the error bound)
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
