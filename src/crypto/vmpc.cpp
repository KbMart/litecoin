#include <algorithm>
#include <crypto/vmpc.h>
#include <cstdint>
#include <cstring>
/**
 * http://www.pieknafunkcja.pl/publi/VMPC_Enigma_2005.pdf
 */

static uint8_t s = 0;
uint8_t P[256];
uint8_t T[32];
uint8_t M[20];
uint8_t c = 16;
uint8_t K[32] = {0,  8,  16, 24, 32, 40,  48,  56,
                 64, 72, 80, 88, 96, 104, 112, 120};
uint8_t z = 16;
uint8_t V[16] = {128, 136, 144, 152, 160, 168, 176, 184,
                 192, 200, 208, 216, 224, 232, 240, 248};
uint8_t const initK[16] = {0,  8,  16, 24, 32, 40,  48,  56,
                           64, 72, 80, 88, 96, 104, 112, 120};

static void VMPC_KSA() {
  s = 0;
  for (int n = 0; n < 256; n++) {
    P[n] = n;
  }

  for (int m = 0; m < 768; m++) {
    uint8_t n = m % 256;
    s = (uint8_t)(P[(uint8_t)(s + P[n] + K[m % c])]);
    std::swap(P[n], P[s]);
  }

  for (int m = 0; m < 768; m++) {
    uint8_t n = m % 256;
    s = (uint8_t)(P[(uint8_t)(s + P[n] + V[(m % z)])]);
    std::swap(P[n], P[s]);
  }
}

static void VMPC(char *const output) {
  for (uint8_t n = 0; n < 32; n++) {
    s = P[(uint8_t)(s + P[n])];
    output[n] = P[(uint8_t)(P[P[s]] + 1)];
    std::swap(P[n], P[s]);
  }
}

static void HASH(uint8_t const msglen, char const *const input,
                 char *const output) {
  memcpy(K, initK, sizeof(uint8_t) * 16);
  c = 16;

  VMPC_KSA();

  uint8_t x1 = 0;
  uint8_t x2 = 0;
  uint8_t x3 = 0;
  uint8_t x4 = 0;
  uint8_t n = 0;
  uint8_t g = 0;

  memset(T, 0, sizeof(uint8_t) * 32);

  for (uint8_t m = 0; m < msglen; m++, n++) {
    s = P[(uint8_t)(s + P[n])];
    x4 = P[(uint8_t)(x4 + x3)];
    x3 = P[(uint8_t)(x3 + x2)];
    x2 = P[(uint8_t)(x2 + x1)];
    x1 = P[(uint8_t)(x1 + s + (input[m] ^ P[(uint8_t)(P[P[s]] + 1)]))];

    T[g] ^= x1;
    T[g + 1] ^= x2;
    T[g + 2] ^= x3;
    T[g + 3] ^= x4;

    std::swap(P[n], P[s]);
    g = (g + 4) % 32;
  }

  for (uint8_t R = 1; R < 25; R++, n++) {
    s = P[(uint8_t)(s + P[n])];

    x4 = P[(uint8_t)(x4 + x3 + R)];
    x3 = P[(uint8_t)(x3 + x2 + R)];
    x2 = P[(uint8_t)(x2 + x1 + R)];
    x1 = P[(uint8_t)(x1 + s + R)];

    T[g] ^= x1;
    T[g + 1] ^= x2;
    T[g + 2] ^= x3;
    T[g + 3] ^= x4;

    std::swap(P[n], P[s]);
    g = (g + 4) % 32;
  }

  memcpy(K, T, sizeof(uint8_t) * 32);
  c = 32;
  for (int m = 0; m < 768; m++) {
    n = m % 256;
    s = P[(uint8_t)(s + P[n] + K[m % c])];
    std::swap(P[n], P[s]);
  }
  VMPC(output);
}

void vmpc_hash(char const *const input, char *const output) {
  HASH(80, input, output);
}
