// CUSTOM HEADER
#include "blowfish.h"

void swapLU(unsigned long * arg1, unsigned long * arg2){
	unsigned long aux;
	aux = *arg1;
  *arg1 = *arg2;
  *arg2 = aux;
}

// TRANSFORM FUNCTION UPON THE CONTEXT
static unsigned long F(BLOWFISH_CTX *ctx, unsigned long arg) {
    unsigned short first, second, third, fourth;
    unsigned long transformed;

    // MAKING THE SHIFT OPERATIONS
    fourth = (unsigned short)(arg & 0xFF);
    arg >>= 8;

    third = (unsigned short)(arg & 0xFF);
    arg >>= 8;

    second = (unsigned short)(arg & 0xFF);
    arg >>= 8;

    first = (unsigned short)(arg & 0xFF);

    // GETTING THE TRANSFORMED NUMBERS
    transformed = ctx->S[0][first] + ctx->S[1][second];
    transformed = transformed ^ ctx->S[2][third];
    transformed = transformed + ctx->S[3][fourth];

    return transformed;
}

// INITIALIZATION
void blowfish_Init(BLOWFISH_CTX *ctx, unsigned char *key, int keyLen) {
    unsigned long data, left_data, right_data;
    int j;

    // INITIALIZE BOTH DATA
    for (int i = 0; i < N + 2; i += 2) {
        blowfish_Crypt(ctx, &left_data, &right_data, ENCRYPT);
        ctx->P[i] = left_data;
        ctx->P[i + 1] = right_data;
    }

    for (int i = 0; i < 4; ++i) {
        for (j = 0; j < 256; j += 2) {
            blowfish_Crypt(ctx, &left_data, &right_data, ENCRYPT);
            ctx->S[i][j] = left_data;
            ctx->S[i][j + 1] = right_data;
        }
    }

    for (int i = 0; i < 4; i++) {
        for (j = 0; j < 256; j++)
            ctx->S[i][j] = S_ARRAY[i][j];
    }

    j = 0;

    for (int i = 0; i < N + 2; ++i) {
        data = 0x00000000;
        for (int k = 0; k < 4; ++k) {
            data = (data << 8) | key[j];
            j = j + 1;
            if (j >= keyLen)
                j = 0;
        }
        ctx->P[i] = P_ARRAY[i] ^ data;
    }

    // INITIALIZE
    left_data = 0x00000000;
    right_data = 0x00000000;

}


// FUNCTION TO ENCRYPT AND DECRYPT DEPENDING ON THE MODE
void blowfish_Crypt(BLOWFISH_CTX *ctx, unsigned long *left, unsigned long *right, int mode){
    unsigned long  xOrLeft, xOrRight;

    xOrLeft = *left;
    xOrRight = *right;

    // FOR ENCRYPTION
    if(mode == ENCRYPT){
        for (short i = 0; i < N; ++i) {
            xOrLeft = xOrLeft ^ ctx->P[i];
            xOrRight = F(ctx, xOrLeft) ^ xOrRight;

            // SWAPPING XORLEFT AND RIGHT
            swapLU(&xOrLeft, &xOrRight);
        }
    // FOR DECRYPTION
    }else if(mode == DECRYPT){
        for (short i = N + 1; i > 1; --i) {
            xOrLeft = xOrLeft ^ ctx->P[i];
            xOrRight = F(ctx, xOrLeft) ^ xOrRight;

            // SWAPPING XORLEFT AND RIGHT
            swapLU(&xOrLeft, &xOrRight);
        }
    }

    // SWAPPING XORLEFT AND RIGHT
    swapLU(&xOrLeft, &xOrRight);

    if(mode == ENCRYPT){
        // BITWISE OPERATING
        xOrRight = xOrRight ^ ctx->P[N];
        xOrLeft = xOrLeft ^ ctx->P[N + 1];
    }else if(mode == DECRYPT){
        // BITWISE OPERATING
        xOrRight = xOrRight ^ ctx->P[1];
        xOrLeft = xOrLeft ^ ctx->P[0];
    }

    *left = xOrLeft;
    *right = xOrRight;

}

// FUNCTION TO TEST ENCRYPTION AND DECRYPTION
int blowfish_Test(BLOWFISH_CTX *ctx) {
    // INSTANTIATE A VARIABLE AND SAVE CURRENT STATE
    unsigned long left = 1, right = 2, left_two = left, left_right = right;

    // INITIALIZE
    blowfish_Init(ctx, (unsigned char*)"TESTKEY", 7);

    // ENCRYPT
    blowfish_Crypt(ctx, &left, &right, ENCRYPT);

    // DECRYPT
    blowfish_Crypt(ctx, &left, &right, DECRYPT);

    // CHECK IF NEW STATE OF DECRYPTION IS THE SAME AS
    // PREVIOUS STATE OF ENCRYPTION
    if(left != left_two || right != left_right)
        return -1;
    return 1;
}
