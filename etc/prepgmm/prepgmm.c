#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <png.h>

//-----------------------------------------------------------------------------

struct head_s
{
    uint32_t c;
    uint16_t n;
    uint16_t m;
};

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
    int16_t min;
    int16_t max;
    int16_t err;
    int16_t pad;

    uint32_t sub[4];
};

struct vert_s
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t u;
};

typedef struct file_s *file_p;
typedef struct page_s *page_p;
typedef struct vert_s *vert_p;

//-----------------------------------------------------------------------------

static void fail(const char *error)
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

static int number_of_pages(int n, int m)
{
    int c = (m - 1) / (n - 1);

    if (m > n)
        return (c * c) + number_of_pages(n, (m - 1) / 2 + 1);
    else
        return (c * c);
}

// Determine the interpolated value at point (I/N, J/N) of the quad with
// values Z0, Z1, Z2, Z3 at the corners.

static int interp(int z0, int z1,
                  int z2, int z3, int i, int j, int n)
{
    if (i > j)
        return z2 + (z0 - z2) * (n - i) / n + (z3 - z2) * j / n;
    else
        return z1 + (z0 - z1) * (n - j) / n + (z3 - z1) * i / n;
}

//-----------------------------------------------------------------------------

static int read_png(file_p file,
              const char * name)
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
                    memcpy((uint8_t *) file->p + s * i, bp[j], s);
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, NULL);
    fclose(fp);

    return (file->p ? 1 : 0);
}

//-----------------------------------------------------------------------------

static int write_gmm(const char * gmm,
                           page_p page,
                           vert_p vert, int count, int n, int m, int e)
{
    // Initialize an index queue.

    uint32_t *p = (uint32_t *) calloc(count, sizeof (uint32_t));
    uint32_t  i = 0;
    uint32_t  c = 0;

    // Push the root onto the queue.

    p[c++] = 0;

    // Make a breadth-first traversal of all pages meeting the error criterion.

    for (i = 0; i < c; ++i)
    {
        const uint32_t k = p[i];

        if (page[k].err > e)
        {
            const uint32_t k0 = page[k].sub[0];
            const uint32_t k1 = page[k].sub[1];
            const uint32_t k2 = page[k].sub[2];
            const uint32_t k3 = page[k].sub[3];

            page[k].sub[0] = c + 0;
            page[k].sub[1] = c + 1;
            page[k].sub[2] = c + 2;
            page[k].sub[3] = c + 3;

            p[c++] = k0;
            p[c++] = k1;
            p[c++] = k2;
            p[c++] = k3;
        }
    }

    // Open a GMM file for writing.

    FILE *fp;

    if ((fp = fopen(gmm, "wb")))
    {
        // Write the header.

        struct head_s head;

        head.c = c;
        head.n = n;
        head.m = m;

        fwrite(&head, sizeof (struct head_s), 1, fp);

        // Write the page heads.

        for (i = 0; i < c; ++i)
            fwrite(page + i,         sizeof (struct page_s), 1,         fp);

        // Write the page data.

        for (i = 0; i < c; ++i)
            fwrite(vert + i * n * n, sizeof (struct vert_s), 1 * n * n, fp);

        fclose(fp);
    }
    else fail(strerror(errno));

    free(p);

    return c;
}

//-----------------------------------------------------------------------------

// Get the value at pixel (R, C) of image FILE.

static int get_file(file_p file, int r, int c)
{
    const int w = (int) file->w;
    const int h = (int) file->h;

    return (r < h && c < w) ? ((int) file->p[w * r + c] - 32768) : -32768;
}

// Get the Z value at vertex (R, C) of page K, which has size NxN.

static uint16_t get_z(vert_p vert, int n, int k, int r, int c)
{
    return vert[n * n * k + n * r + c].z;
}

// Get the geomorph value at vertex (R, C) of page K, which has size NxN.

static void put_u(vert_p vert, int n, int k, int r, int c, int u)
{
    const int i = n * n * k + n * r + c;

    vert[i].u = (uint16_t) u;
}

// Put the value (X, Y, Z) in vertex (R, C) of page K, which has size NxN.

static void put_vert(vert_p vert, int n, int k, int r, int c,
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
// P points to the current page index and S gives the sample density.

static int domake(page_p page,
                  vert_p vert,
                  file_p file, int n, int d, int r, int c, int s, int *p)
{
    // Copy and advance the index counter.

    const int k = (*p)++;

    // Sample the page data.

    for     (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
        {
            // Copy the sample.

            const int y0 = r + i * s;
            const int x0 = c + j * s;
            const int z0 = get_file(file, y0, x0);

            put_vert(vert, n, k, i, j, x0 + d, y0 + d, z0);

            // Scan the interpolation for maximum error.

            if (s > 1 && i < n - 1 && j < n - 1)
            {
                int z1 = get_file(file, y0,     x0 + s);
                int z2 = get_file(file, y0 + s, x0    );
                int z3 = get_file(file, y0 + s, x0 + s);

                for     (int ii = 0; ii <= s; ++ii)
                    for (int jj = 0; jj <= s; ++jj)
                    {
                        const int zo = get_file(file, y0 + ii, x0 + jj);
                        const int zi = interp(z0, z1, z2, z3, ii, jj, s);

                        page[k].err = max2(page[k].err, abs(zi - zo));
                    }
            }
        }

    // Recursively process sub-pages, as necessary.

    if (s > 1)
    {
        const int m = n * s / 2;

        page[k].sub[0] = domake(page, vert, file, n, d, r,     c,     s / 2, p);
        page[k].sub[1] = domake(page, vert, file, n, d, r,     c + m, s / 2, p);
        page[k].sub[2] = domake(page, vert, file, n, d, r + m, c,     s / 2, p);
        page[k].sub[3] = domake(page, vert, file, n, d, r + m, c + m, s / 2, p);
    }

    return k;
}

// Compute the metadata of VERT[K], which has parent VERT[P] and size NxN.

static void dometa(page_p page,
                   vert_p vert, int n, int p, int k, int x, int y)
{
    const int k0 = (int) page[k].sub[0];
    const int k1 = (int) page[k].sub[1];
    const int k2 = (int) page[k].sub[2];
    const int k3 = (int) page[k].sub[3];

    // If this page has a parent...

    if (k)
    {
        // Compute all geomorph values.

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

                uint16_t z0 = get_z(vert, n, p, i0, j0);
                uint16_t z1 = get_z(vert, n, p, i1, j1);

                put_u(vert, n, k, i, j, (z0 + z1) / 2);
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
                vert_p v = vert + n * n * k + n * i + j;

                page[k].min = min3(page[k].min, v->z, v->u);
                page[k].max = max3(page[k].max, v->z, v->u);
            }
    }
}

// Process PNG file SRC, writing GMM file DST with page size N and error
// bound E.

static void proc(const char * png,
                 const char * gmm, int n, int e)
{
    // Load the input image file.

    struct file_s file;

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

        int count = number_of_pages(n, m);

        printf("%8d pages max\n", count);

        page_p page = (page_p) calloc(count,         sizeof (struct page_s));
        vert_p vert = (vert_p) calloc(count * n * n, sizeof (struct vert_s));

        if (page && vert)
        {
            // Convert.

            int d = -(m - 1) / 2;
            int s =  (m - 1) / (n - 1);
            int p =  0;

            domake(page, vert, &file, n, d, 0, 0, s, &p);
            dometa(page, vert,        n,    0, 0, 0, 0);

            // Write the output file.

            count = write_gmm(gmm, page, vert, count, n, m, max2(e, 0));

            printf("%8d pages out\n", count);
        }
        else fail("Memory allocation failure");
    }
}

//-----------------------------------------------------------------------------

static void usage(const char * name)
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
