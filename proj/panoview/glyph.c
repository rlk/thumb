#include <GL/glew.h>

static const unsigned char g20[] = {
    0xff, 0xff
};
static const unsigned char g21[] = {
    0x60, 0x80, 0x62, 0x82, 0xff, 0x64, 0x84, 0x6e,
    0x8e, 0xff, 0xff
};
static const unsigned char g22[] = {
    0x6a, 0x6e, 0x4c, 0x4e, 0xff, 0x8a, 0xac, 0x8e,
    0xae, 0xff, 0xff
};
static const unsigned char g23[] = {
    0x42, 0x62, 0x44, 0x64, 0x46, 0x66, 0x48, 0x68,
    0x4a, 0x6a, 0x4c, 0x6c, 0xff, 0x82, 0xa2, 0x84,
    0xa4, 0x86, 0xa6, 0x88, 0xa8, 0x8a, 0xaa, 0x8c,
    0xac, 0xff, 0x24, 0x44, 0x26, 0x46, 0xff, 0x64,
    0x84, 0x66, 0x86, 0xff, 0xa4, 0xc4, 0xa6, 0xc6,
    0xff, 0x28, 0x48, 0x2a, 0x4a, 0xff, 0x68, 0x88,
    0x6a, 0x8a, 0xff, 0xa8, 0xc8, 0xaa, 0xca, 0xff,
    0xff
};
static const unsigned char g24[] = {
    0x46, 0x48, 0x26, 0x38, 0x08, 0x29, 0x0a, 0x3a,
    0x2c, 0x4a, 0x4c, 0xff, 0xa8, 0xa6, 0xc8, 0xb6,
    0xe6, 0xc5, 0xe4, 0xb4, 0xc2, 0xa4, 0xa2, 0xff,
    0x25, 0x04, 0x34, 0x22, 0x44, 0x42, 0xff, 0xc9,
    0xea, 0xba, 0xcc, 0xaa, 0xac, 0xff, 0x40, 0x60,
    0x42, 0x62, 0x44, 0x64, 0x46, 0x66, 0x48, 0x68,
    0x4a, 0x6a, 0x4c, 0x6c, 0x4e, 0x6e, 0xff, 0x80,
    0xa0, 0x82, 0xa2, 0x84, 0xa4, 0x86, 0xa6, 0x88,
    0xa8, 0x8a, 0xaa, 0x8c, 0xac, 0x8e, 0xae, 0xff,
    0x62, 0x82, 0x64, 0x84, 0xff, 0x66, 0x86, 0x68,
    0x88, 0xff, 0x6a, 0x8a, 0x6c, 0x8c, 0xff, 0xff
};
static const unsigned char g25[] = {
    0x00, 0x20, 0x02, 0xec, 0xce, 0xee, 0xff, 0xa0,
    0xb2, 0x82, 0xa3, 0x84, 0xb4, 0xa6, 0xc6, 0xff,
    0xc6, 0xb4, 0xe4, 0xc3, 0xe2, 0xb2, 0xc0, 0xa0,
    0xff, 0x28, 0x3a, 0x0a, 0x2b, 0x0c, 0x3c, 0x2e,
    0x4e, 0xff, 0x4e, 0x3c, 0x6c, 0x4b, 0x6a, 0x3a,
    0x48, 0x28, 0xff, 0xff
};
static const unsigned char g26[] = {
    0xa0, 0xe0, 0x91, 0xb3, 0x73, 0x95, 0x55, 0x77,
    0x37, 0x59, 0x28, 0x4a, 0x2c, 0x4b, 0x4e, 0x5c,
    0x6e, 0x6b, 0x8c, 0x6a, 0x88, 0x59, 0x77, 0xff,
    0x91, 0x73, 0x80, 0x62, 0x20, 0x42, 0x02, 0x33,
    0x04, 0x55, 0x37, 0xff, 0x95, 0xb3, 0xa6, 0xc4,
    0xff, 0xff
};
static const unsigned char g27[] = {
    0x6a, 0x8c, 0x6e, 0x8e, 0xff, 0xff
};
static const unsigned char g28[] = {
    0x80, 0xa0, 0x62, 0x82, 0x6c, 0x8c, 0x8e, 0xae,
    0xff, 0xff
};
static const unsigned char g29[] = {
    0x40, 0x60, 0x62, 0x82, 0x6c, 0x8c, 0x4e, 0x6e,
    0xff, 0xff
};
static const unsigned char g2A[] = {
    0x77, 0xa8, 0x8a, 0xb9, 0x9b, 0xff, 0x77, 0x8a,
    0x6a, 0x8c, 0x6c, 0xff, 0x77, 0x6a, 0x48, 0x5b,
    0x39, 0xff, 0x77, 0x48, 0x46, 0x28, 0x26, 0xff,
    0x77, 0x46, 0x64, 0x35, 0x53, 0xff, 0x77, 0x64,
    0x84, 0x62, 0x82, 0xff, 0x77, 0x84, 0xa6, 0x93,
    0xb5, 0xff, 0x77, 0xa6, 0xa8, 0xc6, 0xc8, 0xff,
    0xff
};
static const unsigned char g2B[] = {
    0x62, 0x82, 0x66, 0x86, 0x88, 0xc6, 0xc8, 0xff,
    0x8c, 0x6c, 0x88, 0x68, 0x66, 0x28, 0x26, 0xff,
    0xff
};
static const unsigned char g2C[] = {
    0x84, 0x64, 0x82, 0x60, 0xff, 0xff
};
static const unsigned char g2D[] = {
    0x28, 0x26, 0xc8, 0xc6, 0xff, 0xff
};
static const unsigned char g2E[] = {
    0x60, 0x80, 0x62, 0x82, 0xff, 0xff
};
static const unsigned char g2F[] = {
    0x00, 0x20, 0x02, 0xec, 0xce, 0xee, 0xff, 0xff
};
static const unsigned char g30[] = {
    0x24, 0x42, 0xac, 0xca, 0xff, 0xb2, 0xc0, 0xc3,
    0xe2, 0xca, 0xec, 0xac, 0xce, 0x3c, 0x2e, 0x2b,
    0x0c, 0x24, 0x02, 0x42, 0x20, 0xb2, 0xc0, 0xff,
    0xff
};
static const unsigned char g31[] = {
    0x80, 0xff, 0x4c, 0x4a, 0x6e, 0x6a, 0x8e, 0x60,
    0x80, 0xff, 0xff
};
static const unsigned char g32[] = {
    0x0c, 0x2b, 0x2e, 0x3c, 0xce, 0xbc, 0xec, 0xcb,
    0xe8, 0xc9, 0xc6, 0xb8, 0x36, 0x28, 0x25, 0x06,
    0x22, 0x00, 0xe2, 0xe0, 0xff, 0xff
};
static const unsigned char g33[] = {
    0x0c, 0x2b, 0x2e, 0x3c, 0xce, 0xbc, 0xec, 0xcb,
    0xe8, 0xc9, 0xd7, 0xb8, 0xb6, 0x68, 0x66, 0xff,
    0xd7, 0xb6, 0xe6, 0xc5, 0xe2, 0xc3, 0xc0, 0xb2,
    0x20, 0x32, 0x02, 0x23, 0xff, 0xff
};
static const unsigned char g34[] = {
    0xc6, 0xc8, 0x06, 0x28, 0x0c, 0x2c, 0xff, 0xc0,
    0xe0, 0xc6, 0xe7, 0xc8, 0xee, 0xce, 0xff, 0xff
};
static const unsigned char g35[] = {
    0xec, 0xee, 0x2c, 0x0e, 0x29, 0x08, 0x38, 0x26,
    0xc8, 0xb6, 0xe6, 0xc5, 0xe2, 0xc3, 0xc0, 0xb2,
    0x20, 0x32, 0x02, 0x23, 0xff, 0xff
};
static const unsigned char g36[] = {
    0xcb, 0xec, 0xbc, 0xce, 0x3c, 0x2e, 0x2b, 0x0c,
    0x28, 0x07, 0x26, 0x02, 0x23, 0x20, 0x32, 0xc0,
    0xb2, 0xe2, 0xc3, 0xe6, 0xc5, 0xc8, 0xb6, 0x28,
    0x26, 0xff, 0xff
};
static const unsigned char g37[] = {
    0x0e, 0x0c, 0xee, 0xbc, 0xec, 0x67, 0x86, 0x60,
    0x80, 0xff, 0xff
};
static const unsigned char g38[] = {
    0x36, 0x17, 0x25, 0x06, 0x23, 0x02, 0x32, 0x20,
    0xb2, 0xc0, 0xc3, 0xe2, 0xc5, 0xe6, 0xb6, 0xd7,
    0xb8, 0xe8, 0xc9, 0xec, 0xcb, 0xce, 0xbc, 0x2e,
    0x3c, 0x0c, 0x2b, 0x08, 0x29, 0x17, 0x38, 0x36,
    0xb8, 0xb6, 0xff, 0xff
};
static const unsigned char g39[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xc6, 0xe7, 0xc8, 0xec, 0xcb, 0xce, 0xbc, 0x2e,
    0x3c, 0x0c, 0x2b, 0x08, 0x29, 0x26, 0x38, 0xc6,
    0xc8, 0xff, 0xff
};
static const unsigned char g3A[] = {
    0x60, 0x80, 0x62, 0x82, 0xff, 0x66, 0x86, 0x68,
    0x88, 0xff, 0xff
};
static const unsigned char g3B[] = {
    0x84, 0x64, 0x82, 0x60, 0xff, 0x66, 0x86, 0x68,
    0x88, 0xff, 0xff
};
static const unsigned char g3C[] = {
    0xbc, 0x8c, 0x67, 0x37, 0xb2, 0x82, 0xff, 0xff
};
static const unsigned char g3D[] = {
    0x24, 0x26, 0xc4, 0xc6, 0xff, 0x28, 0x2a, 0xc8,
    0xca, 0xff, 0xff
};
static const unsigned char g3E[] = {
    0x32, 0x62, 0x87, 0xb7, 0x3c, 0x6c, 0xff, 0xff
};
static const unsigned char g3F[] = {
    0x60, 0x80, 0x62, 0x82, 0xff, 0x64, 0x84, 0x65,
    0xea, 0xcb, 0xec, 0xbc, 0xce, 0x3c, 0x2e, 0x2b,
    0x0c, 0xff, 0xff
};
static const unsigned char g40[] = {
    0xe2, 0xc3, 0xc0, 0xb2, 0x20, 0x32, 0x02, 0x23,
    0x0c, 0x2b, 0x2e, 0x3c, 0xce, 0xbc, 0xec, 0xcb,
    0xe6, 0xc7, 0xc4, 0xb6, 0xa4, 0xa6, 0x84, 0x86,
    0x64, 0x76, 0x46, 0x67, 0x48, 0x78, 0x6a, 0x88,
    0xaa, 0x86, 0xa6, 0xff, 0xff
};
static const unsigned char g41[] = {
    0x00, 0x20, 0x08, 0x26, 0x38, 0xc6, 0xb8, 0xff,
    0xc0, 0xe0, 0xc6, 0xe8, 0xb8, 0x8e, 0x7c, 0x6e,
    0x38, 0x08, 0xff, 0xff
};
static const unsigned char g42[] = {
    0xb8, 0xd7, 0xc9, 0xe8, 0xcb, 0xec, 0xbc, 0xce,
    0x2c, 0x0e, 0x28, 0x07, 0x26, 0x00, 0x22, 0xc0,
    0xb2, 0xe2, 0xc3, 0xe6, 0xc5, 0xd7, 0xb6, 0xb8,
    0x26, 0x28, 0xff, 0xff
};
static const unsigned char g43[] = {
    0xcb, 0xec, 0xbc, 0xce, 0x3c, 0x2e, 0x2b, 0x0c,
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xff, 0xff
};
static const unsigned char g44[] = {
    0x22, 0x00, 0xb2, 0xc0, 0xc3, 0xe2, 0xcb, 0xec,
    0xbc, 0xce, 0x2c, 0x0e, 0x22, 0x00, 0xff, 0xff
};
static const unsigned char g45[] = {
    0xe0, 0xe2, 0x00, 0x22, 0x06, 0x26, 0x08, 0x28,
    0x0e, 0x2c, 0xee, 0xec, 0xff, 0x28, 0x26, 0x88,
    0x86, 0xff, 0xff
};
static const unsigned char g46[] = {
    0x00, 0x20, 0x06, 0x26, 0x08, 0x28, 0x0e, 0x2c,
    0xee, 0xec, 0xff, 0x28, 0x26, 0x88, 0x86, 0xff,
    0xff
};
static const unsigned char g47[] = {
    0xcb, 0xec, 0xbc, 0xce, 0x3c, 0x2e, 0x2b, 0x0c,
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xc6, 0xe8, 0x86, 0x88, 0xff, 0xff
};
static const unsigned char g48[] = {
    0x2e, 0x0e, 0x28, 0x07, 0x26, 0x00, 0x20, 0xff,
    0xc0, 0xe0, 0xc6, 0xe7, 0xc8, 0xee, 0xce, 0xff,
    0x28, 0x26, 0xc8, 0xc6, 0xff, 0xff
};
static const unsigned char g49[] = {
    0x42, 0x40, 0x62, 0x60, 0x82, 0x80, 0xa2, 0xa0,
    0xff, 0x4e, 0x4c, 0x6e, 0x6c, 0x8e, 0x8c, 0xae,
    0xac, 0xff, 0x62, 0x82, 0x6c, 0x8c, 0xff, 0xff
};
static const unsigned char g4A[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xce, 0xee, 0xff, 0xff
};
static const unsigned char g4B[] = {
    0x2e, 0x0e, 0x28, 0x07, 0x26, 0x00, 0x20, 0xff,
    0x28, 0x26, 0x68, 0x66, 0x97, 0xc0, 0xe2, 0xe0,
    0xff, 0x68, 0x97, 0xce, 0xec, 0xee, 0xff, 0xff
};
static const unsigned char g4C[] = {
    0xe0, 0xe2, 0x00, 0x22, 0x0e, 0x2e, 0xff, 0xff
};
static const unsigned char g4D[] = {
    0x00, 0x20, 0x0e, 0x2b, 0x2e, 0x67, 0x79, 0x87,
    0xce, 0xcb, 0xee, 0xc0, 0xe0, 0xff, 0xff
};
static const unsigned char g4E[] = {
    0x00, 0x20, 0x0e, 0x2b, 0x2e, 0xc0, 0xc3, 0xe0,
    0xce, 0xee, 0xff, 0xff
};
static const unsigned char g4F[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xcb, 0xec, 0xbc, 0xce, 0x3c, 0x2e, 0x2b, 0x0c,
    0x23, 0x02, 0xff, 0xff
};
static const unsigned char g50[] = {
    0x00, 0x20, 0x06, 0x26, 0x08, 0x28, 0x0c, 0x2c,
    0x0e, 0xbc, 0xce, 0xcb, 0xec, 0xc9, 0xe8, 0xb8,
    0xc6, 0x28, 0x26, 0xff, 0xff
};
static const unsigned char g51[] = {
    0xc0, 0xe2, 0xc4, 0xec, 0xcb, 0xce, 0xbc, 0x2e,
    0x3c, 0x0c, 0x2b, 0x02, 0x23, 0x20, 0x32, 0xc0,
    0xa2, 0xc4, 0x84, 0xa6, 0xff, 0xff
};
static const unsigned char g52[] = {
    0xc0, 0xe0, 0xc5, 0xe6, 0xb6, 0xd7, 0xb8, 0xe8,
    0xc9, 0xec, 0xcb, 0xce, 0xbc, 0x0e, 0x2c, 0x08,
    0x28, 0x06, 0x26, 0x00, 0x20, 0xff, 0x28, 0x26,
    0xb8, 0xb6, 0xff, 0xff
};
static const unsigned char g53[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xc5, 0xe6, 0xb6, 0xc8, 0x26, 0x38, 0x08, 0x29,
    0x0c, 0x2b, 0x2e, 0x3c, 0xce, 0xbc, 0xec, 0xcb,
    0xff, 0xff
};
static const unsigned char g54[] = {
    0x0e, 0x0c, 0x6e, 0x6c, 0x8e, 0x8c, 0xee, 0xec,
    0xff, 0x60, 0x80, 0x6c, 0x8c, 0xff, 0xff
};
static const unsigned char g55[] = {
    0x2e, 0x0e, 0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0,
    0xc3, 0xe2, 0xce, 0xee, 0xff, 0xff
};
static const unsigned char g56[] = {
    0x2e, 0x0e, 0x27, 0x06, 0x72, 0x60, 0xff, 0x60,
    0x80, 0x72, 0xe6, 0xc7, 0xee, 0xce, 0xff, 0xff
};
static const unsigned char g57[] = {
    0xee, 0xce, 0xe0, 0xc3, 0xc0, 0x87, 0x75, 0x67,
    0x20, 0x23, 0x00, 0x2e, 0x0e, 0xff, 0xff
};
static const unsigned char g58[] = {
    0x00, 0x20, 0x02, 0x75, 0x57, 0x97, 0x79, 0xec,
    0xce, 0xee, 0xff, 0xe0, 0xe2, 0xc0, 0x97, 0x75,
    0xff, 0x0e, 0x0c, 0x2e, 0x57, 0x79, 0xff, 0xff
};
static const unsigned char g59[] = {
    0x0e, 0x0c, 0x2e, 0x66, 0x79, 0x86, 0xce, 0xec,
    0xee, 0xff, 0x60, 0x80, 0x66, 0x86, 0xff, 0xff
};
static const unsigned char g5A[] = {
    0x0e, 0x0c, 0xee, 0xbc, 0xec, 0x02, 0x32, 0x00,
    0xe2, 0xe0, 0xff, 0xff
};
static const unsigned char g5B[] = {
    0xa0, 0xa2, 0x60, 0x82, 0x6e, 0x8c, 0xae, 0xac,
    0xff, 0xff
};
static const unsigned char g5C[] = {
    0xe0, 0xe2, 0xc0, 0x2e, 0x0c, 0x0e, 0xff, 0xff
};
static const unsigned char g5D[] = {
    0x4e, 0x4c, 0x8e, 0x6c, 0x80, 0x62, 0x40, 0x42,
    0xff, 0xff
};
static const unsigned char g5E[] = {
    0x3a, 0x6a, 0x7e, 0x7b, 0xba, 0x8a, 0xff, 0xff
};
static const unsigned char g5F[] = {
    0x02, 0x00, 0xe2, 0xe0, 0xff, 0xff
};
static const unsigned char g60[] = {
    0x8a, 0x8e, 0x6c, 0x6e, 0xff, 0xff
};
static const unsigned char g61[] = {
    0x08, 0x27, 0x2a, 0x38, 0xca, 0xb8, 0xe8, 0xc7,
    0xe6, 0xc6, 0xe4, 0xc4, 0xe0, 0xc2, 0x20, 0x32,
    0x02, 0x23, 0x04, 0x34, 0x26, 0xc4, 0xc6, 0xff,
    0xff
};
static const unsigned char g62[] = {
    0x2e, 0x0e, 0x2a, 0x09, 0x28, 0x00, 0x22, 0xc0,
    0xb2, 0xe2, 0xc3, 0xe8, 0xc7, 0xca, 0xb8, 0x2a,
    0x28, 0xff, 0xff
};
static const unsigned char g63[] = {
    0xc7, 0xe8, 0xb8, 0xca, 0x38, 0x2a, 0x27, 0x08,
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xff, 0xff
};
static const unsigned char g64[] = {
    0xc8, 0xca, 0x38, 0x2a, 0x27, 0x08, 0x23, 0x02,
    0x32, 0x20, 0xc2, 0xe0, 0xc8, 0xe9, 0xca, 0xee,
    0xce, 0xff, 0xff
};
static const unsigned char g65[] = {
    0xe2, 0xc3, 0xc0, 0xb2, 0x20, 0x32, 0x02, 0x23,
    0x04, 0x24, 0x06, 0x26, 0x08, 0x27, 0x2a, 0x38,
    0xca, 0xb8, 0xe8, 0xc7, 0xe4, 0xc6, 0x24, 0x26,
    0xff, 0xff
};
static const unsigned char g66[] = {
    0x4a, 0x48, 0x6a, 0x68, 0x8a, 0x88, 0xaa, 0xa8,
    0xff, 0x60, 0x80, 0x68, 0x88, 0xff, 0x6a, 0x8a,
    0x6c, 0x8b, 0x8e, 0x9c, 0xae, 0xac, 0xff, 0xff
};
static const unsigned char g67[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xc4, 0xe6, 0xc6, 0xea, 0xc8, 0x2a, 0x38, 0x08,
    0x27, 0x06, 0x36, 0x24, 0xc6, 0xc4, 0xff, 0xff
};
static const unsigned char g68[] = {
    0x00, 0x20, 0x08, 0x28, 0x0a, 0x2a, 0x0e, 0x2e,
    0xff, 0x2a, 0x28, 0xca, 0xb8, 0xe8, 0xc7, 0xe0,
    0xc0, 0xff, 0xff
};
static const unsigned char g69[] = {
    0x60, 0x80, 0x6a, 0x8a, 0xff, 0x6c, 0x8c, 0x6e,
    0x8e, 0xff, 0xff
};
static const unsigned char g6A[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xca, 0xea, 0xff, 0xcc, 0xec, 0xce, 0xee, 0xff,
    0xff
};
static const unsigned char g6B[] = {
    0x00, 0x20, 0x04, 0x24, 0x06, 0x26, 0x0e, 0x2e,
    0xff, 0x26, 0x24, 0x86, 0x84, 0xb5, 0xc0, 0xe2,
    0xe0, 0xff, 0x86, 0xb5, 0xca, 0xe8, 0xea, 0xff,
    0xff
};
static const unsigned char g6C[] = {
    0x60, 0x80, 0x6e, 0x8e, 0xff, 0xff
};
static const unsigned char g6D[] = {
    0x00, 0x20, 0x0a, 0x28, 0x6a, 0x68, 0xff, 0x60,
    0x80, 0x68, 0x88, 0x6a, 0xb8, 0xca, 0xc7, 0xe8,
    0xc0, 0xe0, 0xff, 0xff
};
static const unsigned char g6E[] = {
    0x00, 0x20, 0x0a, 0x28, 0xca, 0xb8, 0xe8, 0xc7,
    0xe0, 0xc0, 0xff, 0xff
};
static const unsigned char g6F[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xc7, 0xe8, 0xb8, 0xca, 0x38, 0x2a, 0x27, 0x08,
    0x23, 0x02, 0xff, 0xff
};
static const unsigned char g70[] = {
    0x00, 0x20, 0x04, 0x24, 0x06, 0x26, 0x0a, 0x28,
    0xca, 0xb8, 0xe8, 0xc7, 0xe6, 0xb6, 0xc4, 0x26,
    0x24, 0xff, 0xff
};
static const unsigned char g71[] = {
    0xc0, 0xe0, 0xc4, 0xe5, 0xc6, 0xea, 0xc8, 0x2a,
    0x38, 0x08, 0x27, 0x06, 0x36, 0x24, 0xc6, 0xc4,
    0xff, 0xff
};
static const unsigned char g72[] = {
    0x00, 0x20, 0x0a, 0x28, 0xca, 0xb8, 0xe8, 0xc7,
    0xff, 0xff
};
static const unsigned char g73[] = {
    0xc3, 0xe4, 0xb4, 0xc6, 0x24, 0x36, 0x06, 0x27,
    0x08, 0x38, 0x2a, 0xb8, 0xca, 0xc7, 0xe8, 0xff,
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xe4, 0xff, 0xff
};
static const unsigned char g74[] = {
    0x4a, 0x48, 0x6a, 0x68, 0xff, 0x8a, 0x88, 0xaa,
    0xa8, 0xff, 0xa0, 0xa2, 0x80, 0x92, 0x62, 0x83,
    0x68, 0x88, 0x6a, 0x8a, 0x6c, 0x8c, 0xff, 0xff
};
static const unsigned char g75[] = {
    0x2a, 0x0a, 0x23, 0x02, 0x32, 0x20, 0xc2, 0xe0,
    0xca, 0xea, 0xff, 0xff
};
static const unsigned char g76[] = {
    0xea, 0xca, 0xe6, 0xc7, 0x80, 0x72, 0x60, 0x27,
    0x06, 0x2a, 0x0a, 0xff, 0xff
};
static const unsigned char g77[] = {
    0x2a, 0x0a, 0x22, 0x00, 0x52, 0x60, 0x63, 0x71,
    0x83, 0x80, 0x92, 0xc0, 0xb2, 0xe2, 0xc3, 0xea,
    0xca, 0xff, 0x8a, 0x6a, 0x83, 0x63, 0xff, 0xff
};
static const unsigned char g78[] = {
    0x00, 0x20, 0x02, 0x74, 0x45, 0x76, 0x08, 0x2a,
    0x0a, 0xff, 0xea, 0xca, 0xe8, 0x76, 0xa5, 0x74,
    0xe2, 0xc0, 0xe0, 0xff, 0xff
};
static const unsigned char g79[] = {
    0x23, 0x02, 0x32, 0x20, 0xb2, 0xc0, 0xc3, 0xe2,
    0xc4, 0xe5, 0xc6, 0xea, 0xca, 0xff, 0x2a, 0x0a,
    0x27, 0x06, 0x36, 0x24, 0xc6, 0xc4, 0xff, 0xff
};
static const unsigned char g7A[] = {
    0x0a, 0x08, 0xea, 0xa8, 0xe8, 0x02, 0x42, 0x00,
    0xe2, 0xe0, 0xff, 0xff
};
static const unsigned char g7B[] = {
    0x80, 0xa0, 0x62, 0x82, 0x66, 0x86, 0x57, 0x77,
    0x68, 0x88, 0x6c, 0x8c, 0x8e, 0xae, 0xff, 0xff
};
static const unsigned char g7C[] = {
    0x60, 0x80, 0x6f, 0x8f, 0xff, 0xff
};
static const unsigned char g7D[] = {
    0x6e, 0x4e, 0x8c, 0x6c, 0x88, 0x68, 0x97, 0x77,
    0x86, 0x66, 0x82, 0x62, 0x60, 0x40, 0xff, 0xff
};
static const unsigned char g7E[] = {
    0xbc, 0xbe, 0x9a, 0x9c, 0x5c, 0x5e, 0x3a, 0x3c,
    0xff, 0xff
};
static const unsigned char g7F[] = {
    0x44, 0x22, 0xa4, 0xc2, 0xaa, 0xcc, 0x4a, 0x2c,
    0x44, 0x22, 0xff, 0xff
};

static const unsigned char *glyph[] = {
    g7F, g7F, g7F, g7F, g7F, g7F, g7F, g7F,
    g7F, g7F, g7F, g7F, g7F, g7F, g7F, g7F,
    g7F, g7F, g7F, g7F, g7F, g7F, g7F, g7F,
    g7F, g7F, g7F, g7F, g7F, g7F, g7F, g7F,
    g20, g21, g22, g23, g24, g25, g26, g27,
    g28, g29, g2A, g2B, g2C, g2D, g2E, g2F,
    g30, g31, g32, g33, g34, g35, g36, g37,
    g38, g39, g3A, g3B, g3C, g3D, g3E, g3F,
    g40, g41, g42, g43, g44, g45, g46, g47,
    g48, g49, g4A, g4B, g4C, g4D, g4E, g4F,
    g50, g51, g52, g53, g54, g55, g56, g57,
    g58, g59, g5A, g5B, g5C, g5D, g5E, g5F,
    g60, g61, g62, g63, g64, g65, g66, g67,
    g68, g69, g6A, g6B, g6C, g6D, g6E, g6F,
    g70, g71, g72, g73, g74, g75, g76, g77,
    g78, g79, g7A, g7B, g7C, g7D, g7E, g7F,
};

void draw_glyph(int c)
{
    int i = 0;

    while (glyph[c][i] != 0xff)
    {
        glBegin(GL_TRIANGLE_STRIP);
        {
            while (glyph[c][i] != 0xff)
            {
                glVertex2f((glyph[c][i] / 16) / 16.0f,
                           (glyph[c][i] % 16) / 16.0f);
                i++;
            }
        }
        glEnd();

        i++;
    }

    glTranslatef(1.0f, 0.0f, 0.0f);
}

