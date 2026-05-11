#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define COUNT 20000000

typedef struct {
    unsigned int Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} IID;

static const unsigned char order[16] = {
    3,2,1,0,   5,4,  7,6, 8,9,10,11,12,13,14,15
};

// Ultra lookup
static const char hex2[512] =
    "000102030405060708090A0B0C0D0E0F"
    "101112131415161718191A1B1C1D1E1F"
    "202122232425262728292A2B2C2D2E2F"
    "303132333435363738393A3B3C3D3E3F"
    "404142434445464748494A4B4C4D4E4F"
    "505152535455565758595A5B5C5D5E5F"
    "606162636465666768696A6B6C6D6E6F"
    "707172737475767778797A7B7C7D7E7F"
    "808182838485868788898A8B8C8D8E8F"
    "909192939495969798999A9B9C9D9E9F"
    "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
    "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
    "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
    "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
    "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
    "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";

// Fast Lookup
static const char hex[] = "0123456789ABCDEF";

static const uint16_t hex16[256] = {
    0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,0x3630,0x3730,0x3830,0x3930,0x4130,0x4230,0x4330,0x4430,0x4530,0x4630,
    0x3031,0x3131,0x3231,0x3331,0x3431,0x3531,0x3631,0x3731,0x3831,0x3931,0x4131,0x4231,0x4331,0x4431,0x4531,0x4631,
    0x3032,0x3132,0x3232,0x3332,0x3432,0x3532,0x3632,0x3732,0x3832,0x3932,0x4132,0x4232,0x4332,0x4432,0x4532,0x4632,
    0x3033,0x3133,0x3233,0x3333,0x3433,0x3533,0x3633,0x3733,0x3833,0x3933,0x4133,0x4233,0x4333,0x4433,0x4533,0x4633,
    0x3034,0x3134,0x3234,0x3334,0x3434,0x3534,0x3634,0x3734,0x3834,0x3934,0x4134,0x4234,0x4334,0x4434,0x4534,0x4634,
    0x3035,0x3135,0x3235,0x3335,0x3435,0x3535,0x3635,0x3735,0x3835,0x3935,0x4135,0x4235,0x4335,0x4435,0x4535,0x4635,
    0x3036,0x3136,0x3236,0x3336,0x3436,0x3536,0x3636,0x3736,0x3836,0x3936,0x4136,0x4236,0x4336,0x4436,0x4536,0x4636,
    0x3037,0x3137,0x3237,0x3337,0x3437,0x3537,0x3637,0x3737,0x3837,0x3937,0x4137,0x4237,0x4337,0x4437,0x4537,0x4637,
    0x3038,0x3138,0x3238,0x3338,0x3438,0x3538,0x3638,0x3738,0x3838,0x3938,0x4138,0x4238,0x4338,0x4438,0x4538,0x4638,
    0x3039,0x3139,0x3239,0x3339,0x3439,0x3539,0x3639,0x3739,0x3839,0x3939,0x4139,0x4239,0x4339,0x4439,0x4539,0x4639,
    0x3041,0x3141,0x3241,0x3341,0x3441,0x3541,0x3641,0x3741,0x3841,0x3941,0x4141,0x4241,0x4341,0x4441,0x4541,0x4641,
    0x3042,0x3142,0x3242,0x3342,0x3442,0x3542,0x3642,0x3742,0x3842,0x3942,0x4142,0x4242,0x4342,0x4442,0x4542,0x4642,
    0x3043,0x3143,0x3243,0x3343,0x3443,0x3543,0x3643,0x3743,0x3843,0x3943,0x4143,0x4243,0x4343,0x4443,0x4543,0x4643,
    0x3044,0x3144,0x3244,0x3344,0x3444,0x3544,0x3644,0x3744,0x3844,0x3944,0x4144,0x4244,0x4344,0x4444,0x4544,0x4644,
    0x3045,0x3145,0x3245,0x3345,0x3445,0x3545,0x3645,0x3745,0x3845,0x3945,0x4145,0x4245,0x4345,0x4445,0x4545,0x4645,
    0x3046,0x3146,0x3246,0x3346,0x3446,0x3546,0x3646,0x3746,0x3846,0x3946,0x4146,0x4246,0x4346,0x4446,0x4546,0x4646,
};

static uint32_t hex32[65536];

void init_hex32(void) {
    for (uint32_t i = 0; i < 65536; i++) {
        uint8_t b0 = (i >> 12) & 0xF;
        uint8_t b1 = (i >> 8) & 0xF;
        uint8_t b2 = (i >> 4) & 0xF;
        uint8_t b3 = i & 0xF;
        hex32[i] = hex[b0] | (hex[b1] << 8) | (hex[b2] << 16) | (hex[b3] << 24);
    }
}

static const char iid_template[] = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX";


static inline void to_hex_arith(uint8_t b, char *out) {
    uint8_t hi = b >> 4;
    uint8_t lo = b & 0xF;
    out[0] = hi + '0' + (((9 - hi) >> 7) & ('A' - '0' - 10));
    out[1] = lo + '0' + (((9 - lo) >> 7) & ('A' - '0' - 10));
}

void IIDtoStringAlignedStores(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    
    out[8] = '-'; out[13] = '-'; out[18] = '-'; out[23] = '-'; out[36] = '\0';
    
    *(uint64_t*)&out[0] = (uint64_t)hex16[p[3]]
                        | ((uint64_t)hex16[p[2]] << 16)
                        | ((uint64_t)hex16[p[1]] << 32)
                        | ((uint64_t)hex16[p[0]] << 48);
    
    *(uint32_t*)&out[9] = (uint32_t)hex16[p[5]] | ((uint32_t)hex16[p[4]] << 16);
    *(uint32_t*)&out[14] = (uint32_t)hex16[p[7]] | ((uint32_t)hex16[p[6]] << 16);
    *(uint32_t*)&out[19] = (uint32_t)hex16[p[8]] | ((uint32_t)hex16[p[9]] << 16);
    *(uint64_t*)&out[24] = (uint64_t)hex16[p[10]]
                         | ((uint64_t)hex16[p[11]] << 16)
                         | ((uint64_t)hex16[p[12]] << 32)
                         | ((uint64_t)hex16[p[13]] << 48);
    *(uint32_t*)&out[32] = (uint32_t)hex16[p[14]] | ((uint32_t)hex16[p[15]] << 16);
}

void IIDtoStringHex16Aligned(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    
    out[8] = '-'; out[13] = '-'; out[18] = '-'; out[23] = '-'; out[36] = '\0';
    
    // 4 chars at a time: hex16[high_byte] | (hex16[low_byte] << 16)
    *(uint32_t*)&out[0] = (uint32_t)hex16[p[3]] | ((uint32_t)hex16[p[2]] << 16);
    *(uint32_t*)&out[4] = (uint32_t)hex16[p[1]] | ((uint32_t)hex16[p[0]] << 16);
    *(uint32_t*)&out[9] = (uint32_t)hex16[p[5]] | ((uint32_t)hex16[p[4]] << 16);
    *(uint32_t*)&out[14] = (uint32_t)hex16[p[7]] | ((uint32_t)hex16[p[6]] << 16);
    *(uint32_t*)&out[19] = (uint32_t)hex16[p[8]] | ((uint32_t)hex16[p[9]] << 16);
    *(uint32_t*)&out[24] = (uint32_t)hex16[p[10]] | ((uint32_t)hex16[p[11]] << 16);
    *(uint32_t*)&out[28] = (uint32_t)hex16[p[12]] | ((uint32_t)hex16[p[13]] << 16);
    *(uint32_t*)&out[32] = (uint32_t)hex16[p[14]] | ((uint32_t)hex16[p[15]] << 16);
}

void IIDtoStringArith(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;

    memcpy(out, iid_template, 37);

    to_hex_arith(p[3], &out[0]);
    to_hex_arith(p[2], &out[2]);
    to_hex_arith(p[1], &out[4]);
    to_hex_arith(p[0], &out[6]);
    to_hex_arith(p[5], &out[9]);
    to_hex_arith(p[4], &out[11]);
    to_hex_arith(p[7], &out[14]);
    to_hex_arith(p[6], &out[16]);
    to_hex_arith(p[8], &out[19]);
    to_hex_arith(p[9], &out[21]);
    to_hex_arith(p[10], &out[24]);
    to_hex_arith(p[11], &out[26]);
    to_hex_arith(p[12], &out[28]);
    to_hex_arith(p[13], &out[30]);
    to_hex_arith(p[14], &out[32]);
    to_hex_arith(p[15], &out[34]);
}


void IIDtoStringHex32Prefill(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    uint16_t w;

    out[8] = '-'; out[13] = '-'; out[18] = '-'; out[23] = '-';
    out[36] = '\0';

    w = (p[3] << 8) | p[2]; *(uint32_t*)&out[0] = hex32[w];
    w = (p[1] << 8) | p[0]; *(uint32_t*)&out[4] = hex32[w];
    w = (p[5] << 8) | p[4]; *(uint32_t*)&out[9] = hex32[w];
    w = (p[7] << 8) | p[6]; *(uint32_t*)&out[14] = hex32[w];
    w = (p[8] << 8) | p[9];   *(uint32_t*)&out[19] = hex32[w];
    w = (p[10] << 8) | p[11]; *(uint32_t*)&out[24] = hex32[w];
    w = (p[12] << 8) | p[13]; *(uint32_t*)&out[28] = hex32[w];
    w = (p[14] << 8) | p[15]; *(uint32_t*)&out[32] = hex32[w];
}




void IIDtoStringHex16MemCpy(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    uint16_t w;

    memcpy(out, iid_template, 37);

    w = (p[3] << 8) | p[2]; *(uint16_t*)&out[0] = hex16[w >> 8]; *(uint16_t*)&out[2] = hex16[w & 0xFF];
    w = (p[1] << 8) | p[0]; *(uint16_t*)&out[4] = hex16[w >> 8]; *(uint16_t*)&out[6] = hex16[w & 0xFF];
    w = (p[5] << 8) | p[4]; *(uint16_t*)&out[9] = hex16[w >> 8]; *(uint16_t*)&out[11] = hex16[w & 0xFF];
    w = (p[7] << 8) | p[6]; *(uint16_t*)&out[14] = hex16[w >> 8]; *(uint16_t*)&out[16] = hex16[w & 0xFF];
    w = (p[8] << 8) | p[9];   *(uint16_t*)&out[19] = hex16[w >> 8]; *(uint16_t*)&out[21] = hex16[w & 0xFF];
    w = (p[10] << 8) | p[11]; *(uint16_t*)&out[24] = hex16[w >> 8]; *(uint16_t*)&out[26] = hex16[w & 0xFF];
    w = (p[12] << 8) | p[13]; *(uint16_t*)&out[28] = hex16[w >> 8]; *(uint16_t*)&out[30] = hex16[w & 0xFF];
    w = (p[14] << 8) | p[15]; *(uint16_t*)&out[32] = hex16[w >> 8]; *(uint16_t*)&out[34] = hex16[w & 0xFF];
}


void IIDtoStringHex16Prefill(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    uint16_t w;

    out[8] = '-'; out[13] = '-'; out[18] = '-'; out[23] = '-';
    out[36] = '\0';
    
    w = p[3] | (p[2] << 8); *(uint16_t*)&out[0] = hex16[w & 0xFF]; *(uint16_t*)&out[2] = hex16[w >> 8];
    w = p[1] | (p[0] << 8); *(uint16_t*)&out[4] = hex16[w & 0xFF]; *(uint16_t*)&out[6] = hex16[w >> 8];
    w = p[5] | (p[4] << 8); *(uint16_t*)&out[9] = hex16[w & 0xFF]; *(uint16_t*)&out[11] = hex16[w >> 8];
    w = p[7] | (p[6] << 8); *(uint16_t*)&out[14] = hex16[w & 0xFF]; *(uint16_t*)&out[16] = hex16[w >> 8];
    w = p[8] | (p[9] << 8);   *(uint16_t*)&out[19] = hex16[w & 0xFF]; *(uint16_t*)&out[21] = hex16[w >> 8];
    w = p[10] | (p[11] << 8); *(uint16_t*)&out[24] = hex16[w & 0xFF]; *(uint16_t*)&out[26] = hex16[w >> 8];
    w = p[12] | (p[13] << 8); *(uint16_t*)&out[28] = hex16[w & 0xFF]; *(uint16_t*)&out[30] = hex16[w >> 8];
    w = p[14] | (p[15] << 8); *(uint16_t*)&out[32] = hex16[w & 0xFF]; *(uint16_t*)&out[34] = hex16[w >> 8];
    
}

void IIDtoStringHex16(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    uint16_t w;

    w = p[3] | (p[2] << 8); *(uint16_t*)&out[0] = hex16[w & 0xFF]; *(uint16_t*)&out[2] = hex16[w >> 8];
    w = p[1] | (p[0] << 8); *(uint16_t*)&out[4] = hex16[w & 0xFF]; *(uint16_t*)&out[6] = hex16[w >> 8];
    out[8] = '-';
    w = p[5] | (p[4] << 8); *(uint16_t*)&out[9] = hex16[w & 0xFF]; *(uint16_t*)&out[11] = hex16[w >> 8];
    out[13] = '-';
    w = p[7] | (p[6] << 8); *(uint16_t*)&out[14] = hex16[w & 0xFF]; *(uint16_t*)&out[16] = hex16[w >> 8];
    out[18] = '-';
    w = p[8] | (p[9] << 8);   *(uint16_t*)&out[19] = hex16[w & 0xFF]; *(uint16_t*)&out[21] = hex16[w >> 8];
    out[23] = '-';
    w = p[10] | (p[11] << 8); *(uint16_t*)&out[24] = hex16[w & 0xFF]; *(uint16_t*)&out[26] = hex16[w >> 8];
    w = p[12] | (p[13] << 8); *(uint16_t*)&out[28] = hex16[w & 0xFF]; *(uint16_t*)&out[30] = hex16[w >> 8];
    w = p[14] | (p[15] << 8); *(uint16_t*)&out[32] = hex16[w & 0xFF]; *(uint16_t*)&out[34] = hex16[w >> 8];
    out[36] = '\0';
}

void IIDtoStringHex2(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    unsigned char b;
    
    b = p[3]; out[0] = hex2[b*2]; out[1] = hex2[b*2+1];
    b = p[2]; out[2] = hex2[b*2]; out[3] = hex2[b*2+1];
    b = p[1]; out[4] = hex2[b*2]; out[5] = hex2[b*2+1];
    b = p[0]; out[6] = hex2[b*2]; out[7] = hex2[b*2+1];
    out[8] = '-';
    b = p[5]; out[9] = hex2[b*2]; out[10] = hex2[b*2+1];
    b = p[4]; out[11] = hex2[b*2]; out[12] = hex2[b*2+1];
    out[13] = '-';
    b = p[7]; out[14] = hex2[b*2]; out[15] = hex2[b*2+1];
    b = p[6]; out[16] = hex2[b*2]; out[17] = hex2[b*2+1];
    out[18] = '-';
    b = p[8]; out[19] = hex2[b*2]; out[20] = hex2[b*2+1];
    b = p[9]; out[21] = hex2[b*2]; out[22] = hex2[b*2+1];
    out[23] = '-';
    b = p[10]; out[24] = hex2[b*2]; out[25] = hex2[b*2+1];
    b = p[11]; out[26] = hex2[b*2]; out[27] = hex2[b*2+1];
    b = p[12]; out[28] = hex2[b*2]; out[29] = hex2[b*2+1];
    b = p[13]; out[30] = hex2[b*2]; out[31] = hex2[b*2+1];
    b = p[14]; out[32] = hex2[b*2]; out[33] = hex2[b*2+1];
    b = p[15]; out[34] = hex2[b*2]; out[35] = hex2[b*2+1];
    out[36] = '\0';
}

void IIDtoStringFast(const IID *iid, char *out)
{
    const unsigned char *p = (const unsigned char *)iid;
    int i, j = 0;
    
    for(i = 0; i < 16; i++)
    {
        unsigned char b = p[order[i]];
        out[j++] = hex[b >> 4];
        out[j++] = hex[b & 0xF];
        if( j == 8 || j == 13 ||j == 18 ||j == 23)
        {
            out[j++] = '-';
        }
    }
    
    out[j] = '\0';
    //return out;
}

void IIDtoStringOriginal(const IID *iid, char *out) {
   sprintf(out, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
           iid->Data1, iid->Data2, iid->Data3,
           iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
           iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
    
}

int main()
{
    IID *iids = malloc(COUNT * sizeof(IID));

    clock_t gen_start = clock();
    
    for (int i = 0; i < COUNT; i++) {
        iids[i].Data1 = rand();
        iids[i].Data2 = rand();
        iids[i].Data3 = rand();
        for (int j = 0; j < 8; j++) iids[i].Data4[j] = rand() & 0xFF;
    }
    clock_t gen_end = clock();
    
    printf("Generation Time: %.3f seconds\n", (double)(gen_end - gen_start) / CLOCKS_PER_SEC);

    char *original_results = malloc(COUNT * 38);
    char *fast_results = malloc(COUNT * 38);
    char *hex2_results = malloc(COUNT * 38);
    char *hex16_results = malloc(COUNT * 38);
    char *hex16prefill_results = malloc(COUNT * 38);
    char *hex16aligned_results = malloc(COUNT * 38);
    char *hex16alignedstored_results = malloc(COUNT * 38);
    char *hex32prefill_results = malloc(COUNT * 38);
    char *hex16memcpy_results = malloc(COUNT * 38);
    char *arith_results = malloc(COUNT * 38);

    clock_t start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringOriginal(&iids[i], &original_results[i * 38]);
    }
    clock_t end = clock();

    printf("Slow Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringFast(&iids[i], &fast_results[i * 38]);
    }
    end = clock();

    printf("Fast Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
   
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringHex2(&iids[i], &hex2_results[i * 38]);
    }
    end = clock();

    printf("Hex2 Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringHex16(&iids[i], &hex16_results[i * 38]);
    }
    end = clock();
    
    printf("Hex16 Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringHex16Prefill(&iids[i], &hex16prefill_results[i * 38]);
    }
    end = clock();
    
    printf("Hex16 Prefill Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringHex16Aligned(&iids[i], &hex16aligned_results[i * 38]);
    }
    end = clock();
    
    printf("Hex16 Aligned Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringAlignedStores(&iids[i], &hex16alignedstored_results[i * 38]);
    }
    end = clock();
    
    printf("Hex16 Aligned Stored Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    
    
    
    init_hex32();
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringHex32Prefill(&iids[i], &hex32prefill_results[i * 38]);
    }
    end = clock();
    
    printf("Hex32 Prefill Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringHex16MemCpy(&iids[i], &hex16memcpy_results[i * 38]);
    }
    end = clock();
    
    printf("Hex16 MemCpy Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    start = clock();
    for (int i = 0; i < COUNT; i++) {
        IIDtoStringArith(&iids[i], &arith_results[i * 38]);
    }
    end = clock();
    
    printf("Arith Runtime: %.3f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    
    int mismatches = 0;
    for (size_t i = 0; i < COUNT; i++) {
        if (memcmp(&original_results[i * 38], &fast_results[i * 38], 38) != 0)
        {
            // printf("Mismatch at %zu: '%s' vs '%s'\n", i, &slow_results[i * 38], &fast_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex2_results[i * 38], 38) != 0)
        {
            // printf("Mismatch at %zu: '%s' vs '%s'\n", i, &slow_results[i * 38], &ultra_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex16_results[i * 38], 38) != 0)
        {
            // printf("Mismatch at %zu: '%s' vs '%s'\n", i, &slow_results[i * 38], &ultra2_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex16prefill_results[i * 38], 38) != 0)
        {
            // printf("Mismatch at %zu: '%s' vs '%s'\n", i, &slow_results[i * 38], &ultra2_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex32prefill_results[i * 38], 38) != 0)
        {
            //printf("Mismatch at %zu: '%s' vs '%s'\n", i, &original_results[i * 38], &hex32prefill_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex16memcpy_results[i * 38], 38) != 0)
        {
            printf("Mismatch at %zu: '%s' vs '%s'\n", i, &original_results[i * 38], &hex16memcpy_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &arith_results[i * 38], 38) != 0)
        {
            printf("Mismatch at %zu: '%s' vs '%s'\n", i, &original_results[i * 38], &arith_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex16aligned_results[i * 38], 38) != 0)
        {
            printf("Mismatch at %zu: '%s' vs '%s'\n", i, &original_results[i * 38], &hex16aligned_results[i * 38]);
            mismatches++;
        }
        if (memcmp(&original_results[i * 38], &hex16alignedstored_results[i * 38], 38) != 0)
        {
            printf("Mismatch at %zu: '%s' vs '%s'\n", i, &original_results[i * 38], &hex16alignedstored_results[i * 38]);
            mismatches++;
        }
        
    }
    printf("Mismatches: %d / %d\n", mismatches, COUNT);
    
    free(iids);
    free(original_results);
    free(fast_results);
    free(hex2_results);
    free(hex16_results);
    free(hex32prefill_results);
    free(hex16prefill_results);
    free(hex16aligned_results);
    free(hex16alignedstored_results);
    free(hex16memcpy_results);
    free(arith_results);

    return 0;
}
