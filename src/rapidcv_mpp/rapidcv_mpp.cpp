#include "rapidcv_mpp.h"

#include "common.h"
#include "hisi_vb.h"
#include "hisi_venc.h"
#include "hisi_vpss.h"
#include "log.h"

namespace rapidcv_mpp {

int InitSys(const uint32_t& maxWidth /*= 0 */, const uint32_t& maxHeight /*= 0 */) noexcept {
  int ret = 0;
  if (maxWidth != 0 && maxHeight != 0) {
    g_max_width = maxWidth;
    g_max_height = maxHeight;
  }
  // else {
  //   getGrpAttr(RPCV_VPSS_GRP_ID_0, g_max_width, g_max_height);
  // }
  if (g_max_width == 0 || g_max_height == 0) {
    g_max_width = DEFAULE_SYS_MAX_WIDTH;
    g_max_height = DEFAULE_SYS_MAX_HEIGHT;
  }
  MPP_VERSION_S version;
  ret = HI_MPI_SYS_GetVersion(&version);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_SYS_GetVersion failed with code 0x%X", ret);
  } else {
    INFO("HISI MPP VERSION: %s, maxWidth=%d, maxHeight=%d", version.aVersion, g_max_width, g_max_height);
  }
  deinitVbPoolByFile();
  return 0;
}

int SetLogLevel(const LogLevel& _ll) {
  ll = _ll;
  return 0;
}

int GetSysCaps(SysCapInfo& info) noexcept {
  info.maxWidth = g_max_width;
  info.maxHeight = g_max_height;
  return 0;
}

int DeinitSys() noexcept {
  int ret = 0;
  ret = deinitVenc();
  ret |= deinitVpss();
  return ret;
}

int OpenJpegCap() noexcept { return initVenc(); }
int CloseJpegCap() noexcept { return deinitVenc(); }

int JpegEncode(const JpegReqParam& reqParam, JpegOutInfo& outInfo, const int& timeout_ms /*= 100 */,
               const bool bDump /* = false*/, const char* filename /* = ""*/) noexcept {
  int ret = 0;
  VbBlockInfo vbBlock = {0};
  ret = getVbBlkFromMmzBlk(reqParam.frame.data, g_venc_vb_plid, vbBlock);
  if (ret != 0) {
    return ret;
  }
  VbFrameInfo vbFrame = {0};
  vbFrame.data = vbBlock;
  vbFrame.seq = reqParam.frame.seq;
  vbFrame.timestamp = reqParam.frame.timestamp;
  ret = jpegEncode(vbFrame, reqParam.quality, reqParam.bCrop, reqParam.rect, outInfo, timeout_ms, bDump, filename);
  releaseVbBlock(vbBlock);
  return ret;
}

int OpenYuvCap() noexcept { return initVpss(); }
int CloseYuvCap() noexcept { return deinitVpss(); }

int ScaleYuv(const MmzBlockInfo& srcFrame, const uint32_t& dstWidth, const uint32_t& dstHeight,
             MmzBlockInfo& dstFrame) noexcept {
  int ret = 0;
  VbBlockInfo vbBlock = {0};
  ret = getVbBlkFromMmzBlk(srcFrame, g_vpss_vb_plid, vbBlock);
  if (ret != 0) {
    return ret;
  }
  CropInfo cropInfo;
  cropInfo.bCrop = false;
  Rect cropRect = {0};
  cropRect.width = dstWidth;
  cropRect.height = dstHeight;
  cropInfo.rect = cropRect;
  ret = procYUV(vbBlock, cropInfo, dstFrame);
  releaseVbBlock(vbBlock);
  return ret;
}

int CropYuv(const MmzBlockInfo& srcFrame, const Rect& cropRect, MmzBlockInfo& dstFrame) noexcept {
  int ret = 0;
  VbBlockInfo vbBlock = {0};
  ret = getVbBlkFromMmzBlk(srcFrame, g_vpss_vb_plid, vbBlock);
  if (ret != 0) {
    return ret;
  }
  CropInfo cropInfo;
  cropInfo.bCrop = true;
  cropInfo.rect = cropRect;
  ret = procYUV(vbBlock, cropInfo, dstFrame);
  releaseVbBlock(vbBlock);
  return ret;
}

int CopyYuv(const MmzBlockInfo& srcFrame, MmzBlockInfo& dstFrame) noexcept {
  return CopyYuv(srcFrame.pVirAddr, srcFrame.phyAddr, srcFrame.width, srcFrame.height * 1.5, srcFrame.stride,
                 dstFrame.pVirAddr, dstFrame.phyAddr, dstFrame.width, dstFrame.height * 1.5, dstFrame.stride);
}

int GetMmzBlock(MmzBlockInfo& mmzBlock) noexcept { return getMmzBlock(mmzBlock); }

int FlushMmzBlock(MmzBlockInfo& mmzBlock) noexcept { return flushMmzBlock(mmzBlock); }

int ReleaseMmzBlock(MmzBlockInfo& mmzBlock) noexcept { return releaseMmzBlock(mmzBlock); }

int MmzVirAddr2MmzPhyAddr(MmzBlockInfo& mmzBlock) noexcept { return getMmzPhyAddrByVirAddr(mmzBlock); }

int MmzPhyAddr2MmzVirAddr(MmzBlockInfo& mmzBlock) noexcept {}

int ReadYuvFromFile(const char* filename, MmzBlockInfo& mmzBlock) noexcept {
  FILE* fin = fopen(filename, "rb");
  if (!fin) {
    ERROR("Read file=%s fail", filename);
    return 1;
  }
  int totalSize = 0;
  for (int i = 0; i < mmzBlock.height * 1.5; ++i) {
    totalSize += fread(mmzBlock.pVirAddr + i * mmzBlock.stride, sizeof(unsigned char), mmzBlock.width, fin);
  }
  fclose(fin);
  DEBUG("total read bytes=%d from filename=%s", totalSize, filename);
  return 0;
}

int DumpYuvToFile(const char* filename, const MmzBlockInfo& mmzBlock) noexcept {
  FILE* fout = fopen(filename, "wb");
  if (!fout) {
    ERROR("Open file=%s fail", filename);
    return 1;
  }
  int fileSize = 0;
  for (int i = 0; i < mmzBlock.height * 1.5; ++i) {
    fileSize += fwrite(mmzBlock.pVirAddr + mmzBlock.stride * i, 1, mmzBlock.width, fout);
  }
  fclose(fout);
  DEBUG("total write bytes=%d to file=%s", fileSize, filename);
  return 0;
}

int DumpJpeg(const char* filename, const JpegOutInfo& jpeg) noexcept {
  FILE* fout = fopen(filename, "wb");
  if (!fout) {
    ERROR("Open file=%s fail", filename);
    return 1;
  }
  int fileSize = fwrite(jpeg.pVirAddr, 1, jpeg.size, fout);
  fclose(fout);
  DEBUG("jpeg size=%d, total write bytes=%d to file=%s", jpeg.size, fileSize, filename);
  return 0;
}

}  // namespace rapidcv_mpp