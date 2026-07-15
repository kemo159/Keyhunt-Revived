#include "getopt.h"
#include <cstdio>
#include <cstring>

char *optarg = nullptr;
int optind = 1;
int opterr = 1;
int optopt = 0;

int getopt(int argc, char *const argv[], const char *optstring) {
  static const char *next = nullptr;
  optarg = nullptr;

  if (next == nullptr || *next == '\0') {
    if (optind >= argc)
      return -1;
    const char *argument = argv[optind];
    if (argument[0] != '-' || argument[1] == '\0')
      return -1;
    if (std::strcmp(argument, "--") == 0) {
      ++optind;
      return -1;
    }
    next = argument + 1;
  }

  const char option = *next++;
  optopt = (unsigned char)option;
  const char *definition = std::strchr(optstring, option);
  if (definition == nullptr || option == ':') {
    if (*next == '\0') {
      ++optind;
      next = nullptr;
    }
    if (opterr && optstring[0] != ':')
      std::fprintf(stderr, "Unknown option -%c\n", option);
    return '?';
  }

  if (definition[1] == ':') {
    if (*next != '\0') {
      optarg = const_cast<char *>(next);
      ++optind;
      next = nullptr;
    } else if (optind + 1 < argc) {
      optarg = argv[optind + 1];
      optind += 2;
      next = nullptr;
    } else {
      ++optind;
      next = nullptr;
      if (opterr && optstring[0] != ':')
        std::fprintf(stderr, "Option -%c requires an argument\n", option);
      return optstring[0] == ':' ? ':' : '?';
    }
  } else if (*next == '\0') {
    ++optind;
    next = nullptr;
  }

  return option;
}
