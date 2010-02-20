#include "dev.h"
#include <string.h>

dev::dev() {
}

dev::dev(int wi, int he, int de, int r, int g, int b) {
  initialize(wi, he, de, r, g, b);
}

dev::dev(int wi, int he, int de) {
  initialize(wi, he, de, 0, 1, 2);
}

dev::~dev() {
  delete [] image;
}

void dev::setRGB(int r, int g, int b) {
  int rgb_order[MAXDEPTH] = {r, g, b};
  for (int d = 0; d < depth; d++)
    // set offsets for RGB
    rgb[d] = rgb_order[d];
}

int dev::initialize(int wi, int he, int de, int r, int g, int b) {
  width = wi;
  height = he;
  depth = de;
  int rgb_order[MAXDEPTH] = {r, g, b};
  for (int d = 0; d < depth; d++){
    // set offsets for RGB
    rgb[d] = rgb_order[d];	
	}

  image = new unsigned char [width * height * depth];
  memset(image, 0, width * height * depth);
  return 0;
}

unsigned char dev::getByte(int position) {
  return image[position];
}
