/*
 * Nhân hai đa thức trong trường hữu hạn GF(2^n), n = 4 và n = 8.
 *
 * Biểu diễn: mỗi đa thức là một số nguyên, bit i = hệ số của x^i.
 *   Ví dụ: x^3 + x + 1  ->  0b1011  ->  0x0B
 * Phép cộng trong GF(2^n) là XOR.
 *
 * Đa thức tối giản (kể cả số hạng x^n):
 *   GF(2^4): x^4 + x + 1            = 0x13
 *   GF(2^8): x^8 + x^4 + x^3 + x + 1 = 0x11B  (đa thức của AES)
 *
 * Biên dịch: gcc gf.c -o gf
 * Chạy:      ./gf        (Windows: gf.exe)
 */
#include <stdio.h>

#define POLY_GF16  0x13   /* x^4 + x + 1 */
#define POLY_GF256 0x11B  /* x^8 + x^4 + x^3 + x + 1 */

/* Bậc của đa thức (vị trí bit cao nhất), đa thức 0 trả về -1 */
static int degree(unsigned a) {
    int d = -1;
    while (a) { d++; a >>= 1; }
    return d;
}

/* Bước 1: nhân hai đa thức trên GF(2) (carry-less), chưa rút gọn */
unsigned poly_mul(unsigned a, unsigned b) {
    unsigned result = 0;
    while (b) {
        if (b & 1) result ^= a;   /* cộng (XOR) a đã dịch vào kết quả */
        a <<= 1;                  /* nhân a với x */
        b >>= 1;
    }
    return result;
}

/* Bước 2: lấy phần dư của a khi chia cho m trên GF(2) */
unsigned poly_mod(unsigned a, unsigned m) {
    int dm = degree(m);
    while (degree(a) >= dm)
        a ^= m << (degree(a) - dm);
    return a;
}

/* Nhân trong GF(2^n) với đa thức tối giản m */
unsigned gf_mul(unsigned a, unsigned b, unsigned m) {
    return poly_mod(poly_mul(a, b), m);
}

/* Cách 2: nhân và rút gọn dần (như AES) */
unsigned gf_mul_fast(unsigned a, unsigned b, unsigned m) {
    int n = degree(m);
    unsigned result = 0;
    while (b) {
        if (b & 1) result ^= a;
        a <<= 1;
        if (a >> n) a ^= m;       /* bậc đạt n -> rút gọn */
        b >>= 1;
    }
    return result;
}

/* In đa thức dạng x^3 + x + 1 */
void print_poly(unsigned a) {
    int first = 1;
    if (a == 0) { printf("0"); return; }
    for (int i = degree(a); i >= 0; i--) {
        if ((a >> i) & 1) {
            if (!first) printf(" + ");
            if (i == 0) printf("1");
            else if (i == 1) printf("x");
            else printf("x^%d", i);
            first = 0;
        }
    }
}

/* Kiểm tra hai cách khớp nhau và mọi phần tử khác 0 đều có nghịch đảo */
int check_field(unsigned m) {
    unsigned size = 1u << degree(m);
    for (unsigned x = 0; x < size; x++)
        for (unsigned y = 0; y < size; y++)
            if (gf_mul(x, y, m) != gf_mul_fast(x, y, m)) return 0;
    for (unsigned x = 1; x < size; x++) {
        int has_inverse = 0;
        for (unsigned y = 1; y < size && !has_inverse; y++)
            if (gf_mul(x, y, m) == 1) has_inverse = 1;
        if (!has_inverse) return 0;
    }
    return 1;
}

int main(void) {
    unsigned a, b, c;

    /* GF(2^4): (x^2+x+1) * (x^2+x) */
    a = 0x7; b = 0x6;
    c = gf_mul(a, b, POLY_GF16);
    printf("GF(2^4): ("); print_poly(a); printf(") * ("); print_poly(b);
    printf(") = "); print_poly(c); printf("  (0x%X)\n", c);

    /* GF(2^8): ví dụ kinh điển của AES, 0x57 * 0x83 = 0xC1 */
    a = 0x57; b = 0x83;
    c = gf_mul(a, b, POLY_GF256);
    printf("GF(2^8): ("); print_poly(a); printf(") * ("); print_poly(b);
    printf(") = "); print_poly(c); printf("  (0x%02X)\n", c);

    printf("GF(2^4) kiem tra: %s\n", check_field(POLY_GF16)  ? "OK" : "SAI");
    printf("GF(2^8) kiem tra: %s\n", check_field(POLY_GF256) ? "OK" : "SAI");
    return 0;
}
