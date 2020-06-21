#include <string.h>
#include <unistd.h>

#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "rapidcv_mpp.h"
#include "time_utils.h"

bool gBCache = true;
yt_utils::basic_timer<> timer;
std::vector<std::pair<uint32_t, uint32_t> > whPairs;

uint32_t max(uint32_t a, uint32_t b) { return a > b ? a : b; }
uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }

int testJpegSingle(const uint32_t& srcWidth, const uint32_t& srcHeight, const char* srcFilename, const int& quality,
                   const bool& bCrop, const uint32_t& topX, const uint32_t& topY, const uint32_t& dstWidth,
                   const uint32_t& dstHeight, const char* dstFilename) {
  printf("======> testJpegSingle: sw=%u, sh=%u, sf=%s, q=%d, bc=%s, x=%u, y=%u, dw=%u, dh=%u, df=%s <======\n",
         srcWidth, srcHeight, srcFilename, quality, bCrop ? "true" : "false", topX, topY, dstWidth, dstHeight,
         dstFilename);
  int ret = 0;
  rapidcv_mpp::MmzBlockInfo mmz_blk_src = {0};
  mmz_blk_src.width = srcWidth;
  mmz_blk_src.height = srcHeight;
  mmz_blk_src.bCache = gBCache;
  if ((ret = rapidcv_mpp::GetMmzBlock(mmz_blk_src)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  if ((ret = rapidcv_mpp::ReadYuvFromFile(srcFilename, mmz_blk_src)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  rapidcv_mpp::MmzBlockInfo mmz_blk_dst = {0};
  mmz_blk_dst.width = dstWidth;
  mmz_blk_dst.height = dstHeight;
  mmz_blk_dst.bCache = gBCache;
  if ((ret = rapidcv_mpp::GetMmzBlock(mmz_blk_dst)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_dst);
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  rapidcv_mpp::JpegReqParam param;
  param.quality = quality;
  param.bCrop = bCrop;
  param.rect.topX = topX;
  param.rect.topY = topY;
  param.rect.width = dstWidth;
  param.rect.height = dstHeight;
  param.frame.data = mmz_blk_src;
  rapidcv_mpp::JpegOutInfo info;
  info.pVirAddr = mmz_blk_dst.pVirAddr;
  timer.tick();
  ret = rapidcv_mpp::JpegEncode(param, info, 100, true, dstFilename);
  timer.tock();
  printf("JpegEncode cost time=%dms\n", timer.elapse_ms());
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_dst);
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  ret = rapidcv_mpp::ReleaseMmzBlock(mmz_blk_dst);
  ret |= rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
  if (ret != 0) {
    return ret;
  }
  return 0;
}

void testJpegAssembly(const uint32_t& srcWidth, const uint32_t& srcHeight) {
  int ret = 0;
  int quality;
  bool bCrop;
  uint32_t topX;
  uint32_t topY;
  uint32_t dstWidth;
  uint32_t dstHeight;
  char srcFilename[64];
  char dstFilename[64];
  // no crop with high quality
  quality = 90;
  bCrop = false;
  topX = 0;
  topY = 0;
  dstWidth = srcWidth;
  dstHeight = srcHeight;
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_nocrop_%d_dst.jpg", dstWidth, dstHeight, quality);
  ret = testJpegSingle(srcWidth, srcHeight, srcFilename, quality, bCrop, topX, topY, dstWidth, dstHeight, dstFilename);
  if (ret != 0) {
    printf("======> FAILED <======\n\n");
    sleep(10);
  } else {
    printf("======> SUCCEED <======\n\n");
  }
  // no crop with low quality
  quality = 10;
  bCrop = false;
  topX = 0;
  topY = 0;
  dstWidth = srcWidth;
  dstHeight = srcHeight;
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_nocrop_%d_dst.jpg", dstWidth, dstHeight, quality);
  ret = testJpegSingle(srcWidth, srcHeight, srcFilename, quality, bCrop, topX, topY, dstWidth, dstHeight, dstFilename);
  if (ret != 0) {
    printf("======> FAILED <======\n\n");
    sleep(10);
  } else {
    printf("======> SUCCEED <======\n\n");
  }
  // crop with high quality
  quality = 90;
  bCrop = true;
  topX = 16;
  topY = 16;
  dstWidth = (srcWidth - topX) / 2;
  dstHeight = (srcHeight - topY) / 2;
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_nocrop_%d_dst.jpg", dstWidth, dstHeight, quality);
  ret = testJpegSingle(srcWidth, srcHeight, srcFilename, quality, bCrop, topX, topY, dstWidth, dstHeight, dstFilename);
  if (ret != 0) {
    printf("======> FAILED <======\n\n");
    sleep(10);
  } else {
    printf("======> SUCCEED <======\n\n");
  }
  // crop with low quality
  quality = 10;
  bCrop = true;
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_nocrop_%d_dst.jpg", dstWidth, dstHeight, quality);
  ret = testJpegSingle(srcWidth, srcHeight, srcFilename, quality, bCrop, topX, topY, dstWidth, dstHeight, dstFilename);

  if (ret != 0) {
    printf("======> FAILED <======\n\n");
    sleep(10);
  } else {
    printf("======> SUCCEED <======\n\n");
  }
}

void testJpegContinous() {
  int order = 0;
  for (int z = 0; z < 1000; ++z) {
    for (int i = 1; i < (int)whPairs.size(); i++) {
      printf("*** order=%d ***\n", order++);
      testJpegAssembly(whPairs[i].first, whPairs[i].second);
    }
  }
}

void testJpegFullContinous() {
  int ret = 0;
  int quality;
  bool bCrop;
  uint32_t topX;
  uint32_t topY;
  uint32_t dstWidth;
  uint32_t dstHeight;
  uint32_t srcWidth;
  uint32_t srcHeight;
  char srcFilename[64];
  char dstFilename[64];
  srcWidth = 1920;
  srcHeight = 1080;
  quality = 90;
  bCrop = false;
  topX = 0;
  topY = 0;
  dstWidth = srcWidth;
  dstHeight = srcHeight;
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_nocrop_%d_dst.jpg", dstWidth, dstHeight, quality);
  int order = 0;
  bool flag = true;
  for (int i = 0; i < 1000; ++i) {
    printf("*** order=%d ***\n", order++);
    ret =
        testJpegSingle(srcWidth, srcHeight, srcFilename, quality, bCrop, topX, topY, dstWidth, dstHeight, dstFilename);
    if (ret != 0) {
      printf("======> FAILED <======\n\n");
      rapidcv_mpp::CloseJpegCap();
      rapidcv_mpp::OpenJpegCap();
      sleep(10);
    } else {
      printf("======> SUCCEED <======\n\n");
    }
  }
}

void testJpeg() {
  int ret = 0;
  if ((ret = rapidcv_mpp::OpenJpegCap()) != 0) {
    return;
  }

  int choice = 0;
  do {
    printf(
        "JPEG TEST, pls choose:\n1.160x456 assembley test\n2.960x544 assemble test\n3.1920x1080 assemble "
        "test\n4.2560x1440 assemble test\n5.3840x2160 assemble test\n6.continuous test\n7 full continous "
        "test\n0.quit\n\nEnter: ");
    scanf("%d", &choice);
    if (choice == 0) {
      break;
    }
    if (choice <= 5) {
      testJpegAssembly(whPairs[choice].first, whPairs[choice].second);
    } else if (choice == 6) {
      testJpegContinous();
    } else if (choice == 7) {
      testJpegFullContinous();
    }

  } while (true);

  if ((ret = rapidcv_mpp::CloseJpegCap()) != 0) {
    return;
  }
}

int testScaleSingle(const uint32_t& srcWidth, const uint32_t& srcHeight, const char* srcFilename,
                    const uint32_t& dstWidth, const uint32_t& dstHeight, const char* dstFilename) {
  printf("======> testScaleSingle: sw=%u, sh=%u, sf=%s, dw=%u, dh=%u, df=%s <======\n", srcWidth, srcHeight,
         srcFilename, dstWidth, dstHeight, dstFilename);
  int ret = 0;
  rapidcv_mpp::MmzBlockInfo mmz_blk_src = {0};
  mmz_blk_src.width = srcWidth;
  mmz_blk_src.height = srcHeight;
  mmz_blk_src.bCache = gBCache;
  if ((ret = rapidcv_mpp::GetMmzBlock(mmz_blk_src)) != 0) {
    return ret;
  }
  if ((ret = rapidcv_mpp::ReadYuvFromFile(srcFilename, mmz_blk_src)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  rapidcv_mpp::MmzBlockInfo mmz_blk_dst = {0};
  mmz_blk_dst.width = dstWidth;
  mmz_blk_dst.height = dstHeight;
  mmz_blk_dst.bCache = gBCache;
  if ((ret = rapidcv_mpp::GetMmzBlock(mmz_blk_dst)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  timer.tick();
  ret = rapidcv_mpp::ScaleYuv(mmz_blk_src, dstWidth, dstHeight, mmz_blk_dst);
  timer.tock();
  printf("ScaleYuv cost time=%dms\n", timer.elapse_ms());
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_dst);
    return ret;
  }
  rapidcv_mpp::DumpYuvToFile(dstFilename, mmz_blk_dst);
  ret = rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
  ret |= rapidcv_mpp::ReleaseMmzBlock(mmz_blk_dst);
  if (ret != 0) {
    return ret;
  }
  return 0;
}

void testScaleAssembly(const uint32_t& srcWidth, const uint32_t& srcHeight) {
  // shrink
  int ret = 0;
  uint32_t dstWidth;
  uint32_t dstHeight;
  char srcFilename[64];
  char dstFilename[64];
  dstWidth = max((srcWidth / 32) * 16, 64);
  dstHeight = max((srcHeight / 32) * 16, 64);
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_to_%ux%u_scale_dst.yuv", srcWidth, srcHeight, dstWidth, dstHeight);
  ret = testScaleSingle(srcWidth, srcHeight, srcFilename, dstWidth, dstHeight, dstFilename);
  if (ret != 0) {
    printf("======> FAILED <======\n\n");
  } else {
    printf("======> SUCCEED <======\n\n");
  }
  // enlarge
  dstWidth = max((srcWidth * 1.5 / 16) * 16, 64);
  dstHeight = max((srcHeight * 1.5 / 16) * 16, 64);
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_to_%ux%u_scale_dst.yuv", srcWidth, srcHeight, dstWidth, dstHeight);
  ret = testScaleSingle(srcWidth, srcHeight, srcFilename, dstWidth, dstHeight, dstFilename);
  if (ret != 0) {
    printf("======> FAILED <======\n\n");
  } else {
    printf("======> SUCCEED <======\n\n");
  }
}

void testScaleContinous() {
  int order = 0;
  for (int z = 0; z < 1000; ++z) {
    for (int i = 1; i < (int)whPairs.size(); i++) {
      printf("*** order=%d ***\n", order++);
      testScaleAssembly(whPairs[i].first, whPairs[i].second);
    }
  }
}

void testScaleFullContinous() {
  // shrink
  int ret = 0;
  uint32_t dstWidth;
  uint32_t dstHeight;
  char srcFilename[64];
  char dstFilename[64];
  uint32_t srcWidth = 160;
  uint32_t srcHeight = 456;
  // enlarge
  dstWidth = max((srcWidth * 1.5 / 16) * 16, 64);
  dstHeight = max((srcHeight * 1.5 / 16) * 16, 64);
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstFilename, "%ux%u_to_%ux%u_scale_dst.yuv", srcWidth, srcHeight, dstWidth, dstHeight);
  bool flag = true;
  int order = 0;
  for (int i = 0; i < 1000; ++i) {
    printf("*** order=%d ***\n", order++);
    ret = testScaleSingle(srcWidth, srcHeight, srcFilename, dstWidth, dstHeight, dstFilename);
    if (ret != 0) {
      printf("======> FAILED <======\n\n");
      rapidcv_mpp::CloseYuvCap();
      rapidcv_mpp::OpenYuvCap();
    } else {
      printf("======> SUCCEED <======\n\n");
    }
  }
  if (!flag) {
    sleep(20);
  }
}

void testScale() {
  int ret = 0;
  if ((ret = rapidcv_mpp::OpenYuvCap()) != 0) {
    return;
  }

  int choice;
  do {
    printf(
        "SCALE TEST, pls choose:\n1.160x456 assembley test\n2.960x544 assemble test\n3.1920x1080 assemble "
        "test\n4.2560x1440 assemble test\n5.3840x2160 assemble test\n6.continuous test\n7.full continous "
        "test\n0.quit\n\nEnter: ");
    scanf("%d", &choice);
    if (choice == 0) {
      break;
    }
    if (choice <= 5) {
      testScaleAssembly(whPairs[choice].first, whPairs[choice].second);
    } else if (choice == 6) {
      testScaleContinous();
    } else if (choice == 7) {
      testScaleFullContinous();
    }
  } while (true);

  if ((ret = rapidcv_mpp::CloseYuvCap()) != 0) {
    return;
  }
}

int testHwCopySingle(const uint32_t& srcWidth, const uint32_t& srcHeight, const char* srcFilename,
                     const uint32_t& dstWidth, const uint32_t& dstHeight, const char* dstFilename) {
  printf("======> testHwCopySingle: sw=%u, sh=%u, sf=%s, dw=%u, dh=%u, df=%s <======\n", srcWidth, srcHeight,
         srcFilename, dstWidth, dstHeight, dstFilename);
  int ret = 0;
  rapidcv_mpp::MmzBlockInfo mmz_blk_src = {0};
  mmz_blk_src.width = srcWidth;
  mmz_blk_src.height = srcHeight;
  mmz_blk_src.bCache = gBCache;
  ret = rapidcv_mpp::GetMmzBlock(mmz_blk_src);
  if (ret != 0) {
    return -1;
  }
  ret = rapidcv_mpp::ReadYuvFromFile(srcFilename, mmz_blk_src);
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return -1;
  }
  rapidcv_mpp::MmzBlockInfo mmz_blk_hw_dst = {0};
  mmz_blk_hw_dst.width = dstWidth;
  mmz_blk_hw_dst.height = dstHeight;
  mmz_blk_hw_dst.bCache = gBCache;
  ret = rapidcv_mpp::GetMmzBlock(mmz_blk_hw_dst);
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return -1;
  }
  timer.tick();
  ret = rapidcv_mpp::CopyYuv(mmz_blk_src, mmz_blk_hw_dst);
  timer.tock();
  printf("CopyYuv cost time=%dms\n", timer.elapse_ms());
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_hw_dst);
    return -1;
  }
  rapidcv_mpp::DumpYuvToFile(dstFilename, mmz_blk_hw_dst);
  ret = rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
  ret |= rapidcv_mpp::ReleaseMmzBlock(mmz_blk_hw_dst);
  if (ret != 0) {
    return -1;
  }
  return 0;
}

int testSwCopySingle(const uint32_t& srcWidth, const uint32_t& srcHeight, const char* srcFilename,
                     const uint32_t& dstWidth, const uint32_t& dstHeight, const char* dstFilename) {
  printf("======> testSwCopySingle: sw=%u, sh=%u, sf=%s, dw=%u, dh=%u, df=%s <======\n", srcWidth, srcHeight,
         srcFilename, dstWidth, dstHeight, dstFilename);
  int ret = 0;
  rapidcv_mpp::MmzBlockInfo mmz_blk_src = {0};
  mmz_blk_src.width = srcWidth;
  mmz_blk_src.height = srcHeight;
  mmz_blk_src.bCache = gBCache;
  ret = rapidcv_mpp::GetMmzBlock(mmz_blk_src);
  if (ret != 0) {
    return -1;
  }
  ret = rapidcv_mpp::ReadYuvFromFile(srcFilename, mmz_blk_src);
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return -1;
  }
  rapidcv_mpp::MmzBlockInfo mmz_blk_sw_dst = {0};
  mmz_blk_sw_dst.width = dstWidth;
  mmz_blk_sw_dst.height = dstHeight;
  mmz_blk_sw_dst.bCache = gBCache;
  ret = rapidcv_mpp::GetMmzBlock(mmz_blk_sw_dst);
  if (ret != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return -1;
  }
  timer.tick();
  memcpy(mmz_blk_sw_dst.pVirAddr, mmz_blk_src.pVirAddr, mmz_blk_sw_dst.stride * mmz_blk_sw_dst.height * 1.5);
  timer.tock();
  printf("memcpy cost time=%dms\n", timer.elapse_ms());
  rapidcv_mpp::DumpYuvToFile(dstFilename, mmz_blk_sw_dst);

  ret = rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
  ret |= rapidcv_mpp::ReleaseMmzBlock(mmz_blk_sw_dst);
  if (ret != 0) {
    return -1;
  }
  return 0;
}

void testCopyAssembly(const uint32_t& srcWidth, const uint32_t& srcHeight) {
  int ret = 0;
  uint32_t dstWidth;
  uint32_t dstHeight;
  char srcFilename[64];
  char dstHwFilename[64];
  char dstSwFilename[64];
  dstWidth = srcWidth;
  dstHeight = srcHeight;
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  sprintf(dstHwFilename, "hw_%ux%u_to_%ux%u_copy_dst.yuv", srcWidth, srcHeight, dstWidth, dstHeight);
  sprintf(dstSwFilename, "sw_%ux%u_to_%ux%u_copy_dst.yuv", srcWidth, srcHeight, dstWidth, dstHeight);
  // hw copy
  ret = testHwCopySingle(srcWidth, srcHeight, srcFilename, dstWidth, dstHeight, dstHwFilename);
  // sw copy
  ret |= testSwCopySingle(srcWidth, srcHeight, srcFilename, dstWidth, dstHeight, dstSwFilename);
  if (ret != 0) {
    printf("======> FAILED <======\n\n");
  } else {
    printf("======> SUCCEED <======\n\n");
  }
}

void testCopyContinous() {
  int order = 0;
  for (int z = 0; z < 1000; ++z) {
    for (int i = 1; i < (int)whPairs.size(); ++i) {
      printf("*** order=%d ***\n", order++);
      testCopyAssembly(whPairs[i].first, whPairs[i].second);
    }
  }
}

int testCopy() {
  int choice;
  do {
    printf(
        "COPY TEST, pls choose:\n1.160x456 assemble test\n2.960x544 assemble test\n3.1920x1080 assemble "
        "test\n4.2560x1440 assemble test\n5.3840x2160 assemble test\n6.continuous test\n0.quit\n\nEnter: ");
    scanf("%d", &choice);
    if (choice == 0) {
      break;
    }
    if (choice <= 5) {
      testCopyAssembly(whPairs[choice].first, whPairs[choice].second);
    } else {
      testCopyContinous();
    }
  } while (true);
}

int testMmzSingle(const uint32_t& srcWidth, const uint32_t& srcHeight) {
  printf("======> testMmzSingle: w=%u, h=%u <======\n", srcWidth, srcHeight);
  int ret = 0;
  rapidcv_mpp::MmzBlockInfo mmz_blk_src = {0};
  mmz_blk_src.width = srcWidth;
  mmz_blk_src.height = srcHeight;
  mmz_blk_src.bCache = gBCache;
  if ((ret = rapidcv_mpp::GetMmzBlock(mmz_blk_src)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  char srcFilename[64];
  sprintf(srcFilename, "data/%ux%u.yuv", srcWidth, srcHeight);
  if ((ret = rapidcv_mpp::ReadYuvFromFile(srcFilename, mmz_blk_src)) != 0) {
    rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
    return ret;
  }
  ret = rapidcv_mpp::ReleaseMmzBlock(mmz_blk_src);
  if (ret != 0) {
    return ret;
  }
  return 0;
}

void testMmz() {
  int ret = 0;
  int order = 0;
  for (int z = 0; z < 10000; ++z) {
    for (int i = 1; i < (int)whPairs.size(); ++i) {
      printf("*** order=%d ***\n", order++);
      ret = testMmzSingle(whPairs[i].first, whPairs[i].second);
      if (ret != 0) {
        sleep(10);
      }
    }
  }
}

void testCaps() {
  rapidcv_mpp::SysCapInfo info = {0};
  int ret = 0;
  ret = rapidcv_mpp::GetSysCaps(info);
  if (ret != 0) {
    printf("testCaps ERROR!\n");
    return;
  }
  printf("sys cap info: {maxW=%d, maxH=%d}\n", info.maxWidth, info.maxHeight);
}

int testLog() {
  printf("change log level\n");
  int ret = 0;
  rapidcv_mpp::LogLevel ll = rapidcv_mpp::LogLevel::ERROR;
  ret = rapidcv_mpp::SetLogLevel(ll);
  return ret;
}

int main() {
  int ret = 0;
  ret = rapidcv_mpp::InitSys();
  if (ret != 0) {
    printf("reinit mpp\n");
    rapidcv_mpp::DeinitSys();
    ret = rapidcv_mpp::InitSys();
    if (ret != 0) {
      printf("init mpp error\n");
      return -1;
    }
  }
  printf("===========> INIT MPP SUCCEED <===========\n\n");
  ret = testLog();
  if (ret != 0) {
    printf("===========> FAILED <===========\n\n");
  } else {
    printf("===========> SUCCEED <===========\n\n");
  }

  whPairs.push_back(std::make_pair(0, 0));
  whPairs.push_back(std::make_pair(160, 456));
  whPairs.push_back(std::make_pair(960, 544));
  whPairs.push_back(std::make_pair(1920, 1080));
  whPairs.push_back(std::make_pair(2560, 1440));
  whPairs.push_back(std::make_pair(3840, 2160));
  for (int i = 0; i < (int)whPairs.size(); ++i) {
    printf("i=%d: ~~w=%u, h=%u\n", i, whPairs[i].first, whPairs[i].second);
  }

  int choice;
  do {
    printf(
        "\ntest choice:\n1. test caps;\n2. test copy;\n3. test yuv scale;\n4. test jpeg encode;\n5. test mmz\n0. "
        "quite.\n\nEnter:");
    scanf("%d", &choice);
    switch (choice) {
      case 1:
        testCaps();
        if (ret != 0) {
          printf("===========> TEST CAPS FAILED <===========\n\n");
        } else {
          printf("===========> TEST CAPS SUCCEED <===========\n\n");
        }
        break;
      case 2:
        testCopy();
        if (ret != 0) {
          printf("===========> TEST COPY 1 FAILED <===========\n\n");
        } else {
          printf("===========> TEST COPY 1 SUCCEED <===========\n\n");
        }
        break;
      case 3:
        testScale();
        break;
      case 4:
        testJpeg();
        break;
      case 5:
        testMmz();
        break;
      default:
        break;
    }
    if (choice == 0) {
      break;
    }
  } while (true);

  ret = rapidcv_mpp::DeinitSys();
  if (ret != 0) {
    printf("deinit mpp error\n");
    return -1;
  }
  return 0;
}