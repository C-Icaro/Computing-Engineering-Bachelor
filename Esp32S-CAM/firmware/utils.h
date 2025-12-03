#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Função auxiliar para codificação base64 em chunks (economiza RAM)
void base64EncodeChunk(const uint8_t* data, size_t length, String& result) {
  const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  result = "";
  result.reserve(((length + 2) / 3) * 4); // Pré-alocar espaço
  
  size_t i = 0;
  while (i < length) {
    uint32_t octet_a = i < length ? data[i++] : 0;
    uint32_t octet_b = i < length ? data[i++] : 0;
    uint32_t octet_c = i < length ? data[i++] : 0;
    
    uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
    
    result += base64_chars[(triple >> 18) & 0x3F];
    result += base64_chars[(triple >> 12) & 0x3F];
    result += (i - 2 < length) ? base64_chars[(triple >> 6) & 0x3F] : '=';
    result += (i - 1 < length) ? base64_chars[triple & 0x3F] : '=';
  }
}

#endif // UTILS_H





