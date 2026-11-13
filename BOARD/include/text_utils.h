#pragma once

#include <Arduino.h>

namespace TextUtils {

inline uint32_t nextUtf8Char(const char*& ptr) {
  if (!ptr || !*ptr) return 0;
  uint8_t c = (uint8_t)*ptr++;
  if (c < 0x80) return c;
  if ((c & 0xE0) == 0xC0) {
    if (!*ptr) return 0;
    uint8_t c2 = (uint8_t)*ptr++;
    return ((c & 0x1F) << 6) | (c2 & 0x3F);
  }
  if ((c & 0xF0) == 0xE0) {
    if (!*ptr) return 0;
    uint8_t c2 = (uint8_t)*ptr++;
    if (!*ptr) return 0;
    uint8_t c3 = (uint8_t)*ptr++;
    return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
  }
  if ((c & 0xF8) == 0xF0) {
    if (!*ptr) return 0;
    uint8_t c2 = (uint8_t)*ptr++;
    if (!*ptr) return 0;
    uint8_t c3 = (uint8_t)*ptr++;
    if (!*ptr) return 0;
    uint8_t c4 = (uint8_t)*ptr++;
    return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
  }
  return 0;
}

inline char unicodeToAscii(uint32_t cp) {
  if (cp >= 32 && cp <= 126) return (char)cp;
  if (cp == '\n' || cp == '\r') return (char)cp;

  switch (cp) {
    case 0x00E0: return 'a';
    case 0x1EA3: return 'a';
    case 0x00E3: return 'a';
    case 0x00E1: return 'a';
    case 0x1EA1: return 'a';
    case 0x0103: return 'a';
    case 0x1EB1: return 'a';
    case 0x1EB3: return 'a';
    case 0x1EB5: return 'a';
    case 0x1EAF: return 'a';
    case 0x1EB7: return 'a';
    case 0x00E2: return 'a';
    case 0x1EA7: return 'a';
    case 0x1EA9: return 'a';
    case 0x1EAB: return 'a';
    case 0x1EA5: return 'a';
    case 0x1EAD: return 'a';
    case 0x00C0: return 'A';
    case 0x1EA2: return 'A';
    case 0x00C3: return 'A';
    case 0x00C1: return 'A';
    case 0x1EA0: return 'A';
    case 0x0102: return 'A';
    case 0x1EB0: return 'A';
    case 0x1EB2: return 'A';
    case 0x1EB4: return 'A';
    case 0x1EAE: return 'A';
    case 0x1EB6: return 'A';
    case 0x00C2: return 'A';
    case 0x1EA6: return 'A';
    case 0x1EA8: return 'A';
    case 0x1EAA: return 'A';
    case 0x1EA4: return 'A';
    case 0x1EAC: return 'A';
    case 0x0111: return 'd';
    case 0x0110: return 'D';
    case 0x00E8: return 'e';
    case 0x1EBB: return 'e';
    case 0x1EBD: return 'e';
    case 0x00E9: return 'e';
    case 0x1EB9: return 'e';
    case 0x00EA: return 'e';
    case 0x1EC1: return 'e';
    case 0x1EC3: return 'e';
    case 0x1EC5: return 'e';
    case 0x1EBF: return 'e';
    case 0x1EC7: return 'e';
    case 0x00C8: return 'E';
    case 0x1EBA: return 'E';
    case 0x1EBC: return 'E';
    case 0x00C9: return 'E';
    case 0x1EB8: return 'E';
    case 0x00CA: return 'E';
    case 0x1EC0: return 'E';
    case 0x1EC2: return 'E';
    case 0x1EC4: return 'E';
    case 0x1EBE: return 'E';
    case 0x1EC6: return 'E';
    case 0x00EC: return 'i';
    case 0x1EC9: return 'i';
    case 0x0129: return 'i';
    case 0x00ED: return 'i';
    case 0x1ECB: return 'i';
    case 0x00CC: return 'I';
    case 0x1EC8: return 'I';
    case 0x0128: return 'I';
    case 0x00CD: return 'I';
    case 0x1ECA: return 'I';
    case 0x00F2: return 'o';
    case 0x1ECF: return 'o';
    case 0x00F5: return 'o';
    case 0x00F3: return 'o';
    case 0x1ECD: return 'o';
    case 0x00F4: return 'o';
    case 0x1ED3: return 'o';
    case 0x1ED5: return 'o';
    case 0x1ED7: return 'o';
    case 0x1ED1: return 'o';
    case 0x1ED9: return 'o';
    case 0x01A1: return 'o';
    case 0x1EDD: return 'o';
    case 0x1EDF: return 'o';
    case 0x1EE1: return 'o';
    case 0x1EDB: return 'o';
    case 0x1EE3: return 'o';
    case 0x00D2: return 'O';
    case 0x1ECE: return 'O';
    case 0x00D5: return 'O';
    case 0x00D3: return 'O';
    case 0x1ECC: return 'O';
    case 0x00D4: return 'O';
    case 0x1ED2: return 'O';
    case 0x1ED4: return 'O';
    case 0x1ED6: return 'O';
    case 0x1ED0: return 'O';
    case 0x1ED8: return 'O';
    case 0x01A0: return 'O';
    case 0x1EDC: return 'O';
    case 0x1EDE: return 'O';
    case 0x1EE0: return 'O';
    case 0x1EDA: return 'O';
    case 0x1EE2: return 'O';
    case 0x00F9: return 'u';
    case 0x1EE7: return 'u';
    case 0x0169: return 'u';
    case 0x00FA: return 'u';
    case 0x1EE5: return 'u';
    case 0x01B0: return 'u';
    case 0x1EEB: return 'u';
    case 0x1EED: return 'u';
    case 0x1EEF: return 'u';
    case 0x1EE9: return 'u';
    case 0x1EF1: return 'u';
    case 0x00D9: return 'U';
    case 0x1EE6: return 'U';
    case 0x0168: return 'U';
    case 0x00DA: return 'U';
    case 0x1EE4: return 'U';
    case 0x01AF: return 'U';
    case 0x1EEA: return 'U';
    case 0x1EEC: return 'U';
    case 0x1EEE: return 'U';
    case 0x1EE8: return 'U';
    case 0x1EF0: return 'U';
    case 0x1EF3: return 'y';
    case 0x1EF7: return 'y';
    case 0x1EF9: return 'y';
    case 0x00FD: return 'y';
    case 0x1EF5: return 'y';
    case 0x1EF2: return 'Y';
    case 0x1EF6: return 'Y';
    case 0x1EF8: return 'Y';
    case 0x00DD: return 'Y';
    case 0x1EF4: return 'Y';
    // Punctuation & Quotes
    case 0x2018: case 0x2019: return '\'';
    case 0x201C: case 0x201D: return '"';
    case 0x2013: case 0x2014: return '-';
    case 0x00A0: return ' '; // Non-breaking space
    default: return ' ';
  }
}

inline String removeDiacritics(const char* src) {
  if (!src) return String();
  String out;
  out.reserve(strlen(src));
  const char* p = src;
  while (*p) {
    uint32_t cp = nextUtf8Char(p);
    if (!cp) break;
    char a = unicodeToAscii(cp);
    if ((a >= 32 && a <= 126) || a == '\n') {
      out += a;
    }
  }
  return out;
}

inline String removeDiacritics(const String& src) {
  return removeDiacritics(src.c_str());
}

} // namespace TextUtils
