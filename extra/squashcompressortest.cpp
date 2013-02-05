/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is squash.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011-2012 The Initial Developer.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#if _MSC_VER >= 1700
#include <chrono>
using namespace std::chrono;
#include <windows.h>
#undef min
#undef max
#endif

#include "../squash.h"
#include "../squish/squish.h"

using namespace squash;

bool optimizequick = false;

char tempbuf[1024];
char nodebarchar = '-';
string nodebarref;

FILE *repfile = NULL;
FILE *logfile = NULL;

vector<string> notes;
vector<string> errrs;

#pragma warning (disable : 4100)
#pragma warning (disable : 4706)

bool ga = false;
bool gc = false;
bool ts = false;
bool os = false;
bool fl = false;
bool op = false;

static bool Compress(std::string const& sourceFileName, std::string const& targetFileName, int flags) {
  void *inmem, *oumem;
  size_t insize, ousize;

  int ret; struct stat sinfo;
  if ((ret = stat(sourceFileName.c_str(), &sinfo)) != 0)
    return false;

  /* read in the file-contents */
  inmem  = NULL;
  insize =    0;
  FILE *file;
  if ((file = fopen(sourceFileName.c_str(), "rb")) != NULL) {
    inmem = malloc(insize = (UINT)sinfo.st_size);
    if (!inmem) {
      fclose(file);
      return false;
    }

    UINT rdsze = (UINT)
    fread(inmem, 1, insize, file);
    fclose(file);

    if (rdsze != insize) {
      free(inmem);
      return false;
    }
  }
  else
    return false;

  LPDIRECT3DTEXTURE base = NULL;
  HRESULT res;

#ifdef DX11
  D3DX11_IMAGE_LOAD_INFO load; TEXINFO info;
  memset(&load, D3DX11_DEFAULT, sizeof(load));

  squash::TextureInfo(inmem, insize, info);

  load.Width = info.Width;
  load.Height = info.Height;
  load.Usage = D3D11_USAGE_STAGING;
  load.BindFlags = 0;
  load.CpuAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  load.Filter = D3DX11_FILTER_TRIANGLE;
  load.MipLevels = info.MipLevels;
  load.MipFilter = D3DX11_FILTER_NONE;
  load.Format = info.Format;
  load.pSrcInfo = &info;

  if ((res = D3DX11CreateBaseTextureFromMemory(
    squash::pD3DDevice, inmem, insize, &load, NULL, &base, NULL
  )) == S_OK) {
#else
  TEXINFO info;
  squash::TextureInfo(inmem, insize, info);

  if ((res =
    D3DXCreateBaseTextureFromFileInMemoryEx(
    squash::pD3DDevice, inmem, (UINT)insize,
    info.Width, info.Height, info.MipLevels/*D3DX_DEFAULT*/,
    0, info.Format, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE/*D3DX_DEFAULT*/,
    D3DX_FILTER_NONE/*D3DX_DEFAULT*/, 0, NULL, NULL,
    &base
  )) == D3D_OK) {
#endif
    clock_t start = std::clock();
    bool success = false;

    manualflags = flags;

    if (ts) {
      /**/ if (flags & squish::kBtc2) success = squash::TextureCompressXYZD  (&base, 0);
      else if (flags & squish::kBtc3) success = squash::TextureCompressXYZD  (&base, 0);
      else if (flags & squish::kBtc1) success = squash::TextureCompressXYZ   (&base, 0);
      else if (flags & squish::kBtc5) success = squash::TextureCompressXY_Z  (&base, NULL, 0);
      else if (flags & squish::kBtc4) success = squash::TextureCompressXY_Z  (NULL, &base, 0);
    }
    else if (os) {
      /**/ if (flags & squish::kBtc2) success = squash::TextureCompress_XYZD (&base, 0);
      else if (flags & squish::kBtc3) success = squash::TextureCompress_XYZD (&base, 0);
      else if (flags & squish::kBtc1) success = squash::TextureCompress_XYZ  (&base, 0);
      else if (flags & squish::kBtc5) success = squash::TextureCompress_XY_Z (&base, NULL, 0);
      else if (flags & squish::kBtc4) success = squash::TextureCompress_XY_Z (NULL, &base, 0);
    }
    else if (fl) {
      /**/ if (flags & squish::kBtc2) success = squash::TextureCompressRGB_A (&base, 0, ga, gc);
      else if (flags & squish::kBtc3) success = squash::TextureCompressRGB_A (&base, 0, ga, gc);
      else if (flags & squish::kBtc1) success = squash::TextureCompressRGB   (&base, 0, ga);
    }
    else if (op) {
      /**/ if (flags & squish::kBtc2) success = squash::TextureCompressRGBA  (&base, 0, ga, gc);
      else if (flags & squish::kBtc3) success = squash::TextureCompressRGBA  (&base, 0, ga, gc);
      else if (flags & squish::kBtc1) success = squash::TextureCompressRGB   (&base, 0, ga);
    }
    else {
      /**/ if (flags & squish::kBtc2) success = squash::TextureCompressRGBH  (&base, 0, ga);
      else if (flags & squish::kBtc3) success = squash::TextureCompressRGBH  (&base, 0, ga);
      else if (flags & squish::kBtc1) success = squash::TextureCompressRGB   (&base, 0, ga);
    }

    clock_t end = std::clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "time: " << duration << " secs" << std::endl;
    std::cout << "comp. speed: " <<
      ((double) (info.Width * info.Height) / (duration * 1024 * 1024)) << " MPixel/s" << ", " <<
      ((double) TextureInfoSize(base) / (duration * 1024 * 1024 * 8)) << " MBytes/s" << std::endl;

    if (success) {
#ifdef DX11
      if (0)
	res = D3DX11SaveTextureToFile(squash::pD3DDeviceContext, base, D3DX11_IFF_TIFF, targetFileName.c_str());
      else if (0)
	res = D3DX11SaveTextureToFile(squash::pD3DDeviceContext, base, D3DX11_IFF_PNG , targetFileName.c_str());
      else
	res = D3DX11SaveTextureToFile(squash::pD3DDeviceContext, base, D3DX11_IFF_DDS , targetFileName.c_str());
#else
      /* ATI1/2 without hardware, mem-layout (alignment etc.) is the same as DXT1/3 */
      /**/ if ((info.Format == D3DFMT_ATI1) || (info.Format == D3DFMT_ATI2))
	res = SaveBC45TextureToFile(targetFileName.c_str(),              base      );
      else
	res = D3DXSaveTextureToFile(targetFileName.c_str(), D3DXIFF_DDS, base, NULL);
#endif

      success = (res == S_OK);
      
      if (success) {
	struct stat sinfo;

	if (!(ret = stat(targetFileName.c_str(), &sinfo))) {
	  oumem  = NULL;
	  ousize = sinfo.st_size;

	  fprintf(stderr, "%dx%d: %d to %d bytes, %.2f to %.2f bpp, %.2f%%\n",
	    info.Width, info.Height,
	    insize, ousize,
	    (8.0f * insize) / (info.Width * info.Height),
	    (8.0f * ousize) / (info.Width * info.Height),
	    (100.0f * ousize) / insize
	  );

	  fflush(stderr);
	}
      }
    }

    if (base)
      base->Release();

    return success;
  }

  return false;
}

static bool Decompress(std::string const& sourceFileName, std::string const& targetFileName) {
  return false;
}

static bool Benchmark(std::string const& targetFileName) {
  return false;
}

enum Mode
{
  kCompress,
  kDecompress,
  kBenchmark
};

bool verbose = false;
bool threaded = false;

int main(int argc, char **argv)
{
  if (!TextureInit())
    return 1;

  try {
    // parse the command-line
    std::string sourceFileName;
    std::string targetFileName;
    Mode mode = kCompress;
    int method = squish::kBtc1;//kBtc7;
    int metric = squish::kColourMetricPerceptual;
    int fit    = squish::kColourClusterFit;//squish::kColourClusterFit;
    int alpha  = 0;//squish::kAlphaIterativeFit;
    int extra  = 0;

    bool help = false;
    bool arguments = true;

    for (int i = 1; i < argc; ++i) {
      // check for options
      char const* word = argv[i];
      if (arguments && word[0] == '-') {
	for (int j = 1; word[j] != '\0'; ++j) {
	  switch (word[j]) {
	    case 'h': help = true; break;

	    case 'c': mode = kCompress; break;
	    case 'd': mode = kDecompress; break;
	    case 'b': mode = kBenchmark; break;

	    case '1': method = squish::kBtc1; break;
	    case '2': method = squish::kBtc2; break;
	    case '3': method = squish::kBtc3; break;
	    case '4': method = squish::kBtc4; break;
	    case '5': method = squish::kBtc5; break;
	    case '7': method = squish::kBtc7; break;

	    case 'u': metric = squish::kColourMetricUniform; break;
	    case 'm': metric = squish::kColourMetricUnit; break;

	    case 'a': alpha = squish::kAlphaIterativeFit; break;
	    case 'r': fit = squish::kColourRangeFit; break;
	    case 'i': fit = squish::kColourIterativeClusterFit; break;
	    case 'x': fit = squish::kColourClusterFit * 15; break;

	    case 'w': extra = squish::kWeightColourByAlpha; break;

	    case 'v': verbose = true; break;
//	    case 't': threaded = true; break;

	    case 't': ts = true; break;
	    case 'n': os = true; break;
	    case 'f': fl = true; break;
	    case 'o': op = true; break;
	    case 'q':
	      alphaopaqueness = 0.5f;
	      break;
	    case 's':
	      normalsteepness = 1;
	      break;
	    case 'z':
	      gc = true;
	      alphacontrast = 1.0f / 2.0f;
	      alphacontrastinv = 2.0f / 1.0f;
	      break;
	    case 'g':
	      ga = true;
	      colorgamma = 2.2f / 1.0f;
	      colorgammainv = 1.0f / 2.2f;
	      break;

	    case '-': arguments = false; break;
	    default:
	      std::cerr << "unknown option '" << word[j] << "'" << std::endl;
	      return -1;
	  }
	}
      }
      else {
	if (sourceFileName.empty())
	  sourceFileName.assign(word);
	else if (targetFileName.empty())
	  targetFileName.assign(word);
	else {
	  std::cerr << "unexpected argument \"" << word << "\"" << std::endl;
	}
      }
    }

    // check arguments
    if (help) {
      std::cout
	<< "SYNTAX" << std::endl
	<< "\tsquashcompressortest [-bcd] <source> <target>" << std::endl
	<< "OPTIONS" << std::endl
	<< "\t-c\tCompress source texture to target dds (default)" << std::endl
	<< "\t-123\tSpecifies whether to use DXT1/BC1, DXT3/BC2 or DXT5/BC3 compression" << std::endl
	<< "\t-45\tSpecifies whether to use ATI/BC4, ATI2/BC5 compression" << std::endl
	<< "\t-7\tSpecifies whether to use BC7 (default) compression" << std::endl
	<< "\t-u\tUse a uniform colour metric during colour compression" << std::endl
	<< "\t-m\tUse a unit normal metric during normal compression" << std::endl
	<< "\t-a\tUse the slow alpha/gray iterative compressor" << std::endl
	<< "\t-r\tUse the fast but inferior range-based colour compressor" << std::endl
	<< "\t-i\tUse the very slow but slightly better iterative colour compressor" << std::endl
	<< "\t-x\tUse the extreme slow but slightly better iterative colour compressor" << std::endl
	<< "\t-w\tWeight colour values by alpha in the cluster colour compressor" << std::endl
	<< "\t-d\tDecompress source dds to target texture" << std::endl
	<< "\t-b\tbenchmark the given dds file" << std::endl
	<< "\t-v\tbe verbose" << std::endl
//	<< "\t-t\tbe multi-threaded" << std::endl
	<< "\t-t\tpartal-derivative tangent-space normal-map" << std::endl
	<< "\t-n\tregular tangent-space or object-space normal-map" << std::endl
	<< "\t-f\tfoilage/coverage-map in alpha-channel" << std::endl
	<< "\t-o\topacity-map in alpha-channel" << std::endl
	<< "\t-q\tsharpen foilage/coverage-map while filtering" << std::endl
	<< "\t-s\tsteepen normal-map while filtering" << std::endl
	<< "\t-z\tcontrast opacity-map while filtering" << std::endl
	<< "\t-g\tgamma-correct colour-map while filtering" << std::endl
	;

      return 0;
    }

    if (sourceFileName.empty()) {
      std::cerr << "no source file given" << std::endl;
      return -1;
    }

    if (targetFileName.empty() && (mode != kBenchmark)) {
      std::cerr << "no target file given" << std::endl;
      return -1;
    }

    // do the work
    switch (mode) {
      case kCompress:
	Compress(sourceFileName, targetFileName, method + metric + fit + alpha + extra);
	break;

      case kDecompress:
 	Decompress(sourceFileName, targetFileName);
	break;

      case kBenchmark:
	Benchmark(sourceFileName);
	break;

      default:
	std::cerr << "unknown mode" << std::endl;
	throw std::exception();
    }
  }
  catch (std::exception& excuse) {
    // complain
    std::cerr << "squashcompressortest error: " << excuse.what() << std::endl;
    return -1;
  }

  TextureCleanup();

  // done
  return 0;
}
