#include "common.h"

#include "hi_comm_ive.h"
#include "log.h"
#include "mpi_ive.h"

namespace rapidcv_mpp {

uint32_t g_max_width = 0;
uint32_t g_max_height = 0;

HI_U32 g_vpss_vb_plid = VB_INVALID_POOLID;
HI_U32 g_venc_vb_plid = VB_INVALID_POOLID;

HI_U32 g_venc_frame_seq = 0;

int mmzBlk2vbBlk(const MmzBlockInfo& mmzBlock, VbBlockInfo& vbBlock) {
  CopyYuv(mmzBlock.pVirAddr, mmzBlock.phyAddr, mmzBlock.width, mmzBlock.height * 1.5, mmzBlock.stride, vbBlock.pVirAddr,
          vbBlock.phyAddr, vbBlock.width, vbBlock.height * 1.5, vbBlock.width);
}

int vbBlk2mmzBlk(const VbBlockInfo& vbBlock, MmzBlockInfo& mmzBlock) {
  CopyYuv(vbBlock.pVirAddr, vbBlock.phyAddr, vbBlock.width, vbBlock.height * 1.5, vbBlock.width, mmzBlock.pVirAddr,
          mmzBlock.phyAddr, mmzBlock.width, mmzBlock.height * 1.5, mmzBlock.stride);
}

int videoFrameInfoS2mmzBlk(const VIDEO_FRAME_INFO_S& frame, MmzBlockInfo& mmzBlock) {
  CopyYuv(frame.stVFrame.u64VirAddr[0], frame.stVFrame.u64PhyAddr[0], frame.stVFrame.u32Width,
          frame.stVFrame.u32Height * 1.5, frame.stVFrame.u32Stride[0], mmzBlock.pVirAddr, mmzBlock.phyAddr,
          mmzBlock.width, mmzBlock.height * 1.5, mmzBlock.stride);
  // CopyYuv(frame.stVFrame.u64VirAddr[1], frame.stVFrame.u64PhyAddr[1], frame.stVFrame.u32Width,
  //         frame.stVFrame.u32Height * 0.5, frame.stVFrame.u32Stride[1],
  //         mmzBlock.pVirAddr + mmzBlock.stride * frame.stVFrame.u32Height,
  //         mmzBlock.phyAddr + mmzBlock.stride * frame.stVFrame.u32Height, mmzBlock.width, mmzBlock.height * 0.5,
  //         mmzBlock.stride);
  return 0;
}

int vbBlk2videoFrameInfo(const VbBlockInfo& vbBlock, VIDEO_FRAME_INFO_S& frame) {
  frame.enModId = HI_ID_VB;
  frame.u32PoolId = vbBlock.plId;
  frame.stVFrame.u32Width = vbBlock.width;
  frame.stVFrame.u32Height = vbBlock.height;
  frame.stVFrame.u32Stride[0] = vbBlock.width;
  frame.stVFrame.u32Stride[1] = vbBlock.width;
  frame.stVFrame.u32Stride[2] = vbBlock.width;
  frame.stVFrame.u64PhyAddr[0] = vbBlock.phyAddr;
  frame.stVFrame.u64PhyAddr[1] = vbBlock.phyAddr + vbBlock.width * vbBlock.height;
  frame.stVFrame.u64PhyAddr[2] = vbBlock.phyAddr + vbBlock.width * vbBlock.height;
  frame.stVFrame.u64VirAddr[0] = vbBlock.pVirAddr;
  frame.stVFrame.u64VirAddr[1] = vbBlock.pVirAddr + vbBlock.width * vbBlock.height;
  frame.stVFrame.u64VirAddr[2] = vbBlock.pVirAddr + vbBlock.width * vbBlock.height;
  frame.stVFrame.enField = VIDEO_FIELD_FRAME;
  frame.stVFrame.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
  frame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
  frame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
  frame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
  frame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
  return 0;
}

int vbFrameInfo2videoFrameInfo(const VbFrameInfo& vbFrame, VIDEO_FRAME_INFO_S& vdFrame) {
  vbBlk2videoFrameInfo(vbFrame.data, vdFrame);
  vdFrame.stVFrame.u32TimeRef = vbFrame.seq;
  return 0;
}

int getMmzBlock(MmzBlockInfo& mmzBlock) {
  int ret = 0;
  HI_U64 phyAddr = 0;
  HI_VOID* pVirAddr = nullptr;
  mmzBlock.stride = mmzBlock.width;
  uint32_t size = mmzBlock.width * mmzBlock.height * 1.5;
  if (mmzBlock.bCache) {
    ret = HI_MPI_SYS_MmzAlloc_Cached((HI_U64*)&phyAddr, (HI_VOID**)&pVirAddr, "RPDCV_MPP_CACHE", HI_NULL, (HI_U32)size);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_SYS_MmzAlloc_Cached failed with code 0x%X, check u32Width=%u, u32Height=%u, u32Len=%u", ret,
            mmzBlock.width, mmzBlock.height, size);
      return ret;
    }
  } else {
    ret = HI_MPI_SYS_MmzAlloc((HI_U64*)&phyAddr, (HI_VOID**)&pVirAddr, "RPDCV_MPP", HI_NULL, (HI_U32)size);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_SYS_MmzAlloc failed with code 0x%X, check u32Width=%u, u32Height=%u, u32Len=%u", ret,
            mmzBlock.width, mmzBlock.height, size);
      return ret;
    }
  }
  mmzBlock.phyAddr = (uint64_t)phyAddr;
  mmzBlock.pVirAddr = (uint8_t*)pVirAddr;
  INFO("MMZ Block Info: u32Width=%u, u32Height=%u, bCache=%s, u32Len=%u, u64PhyAddr=%lu, u64VirAddr=%p", mmzBlock.width,
       mmzBlock.height, mmzBlock.bCache ? "true" : "false", size, mmzBlock.phyAddr, mmzBlock.pVirAddr);
  return 0;
}

int flushMmzBlock(MmzBlockInfo& mmzBlock) {
  int ret = 0;
  ret = HI_MPI_SYS_MmzFlushCache(mmzBlock.phyAddr, mmzBlock.pVirAddr, mmzBlock.height * mmzBlock.stride * 1.5);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_SYS_MmzFlushCache failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int releaseMmzBlock(const MmzBlockInfo& mmzBlock) {
  int ret = 0;
  ret = HI_MPI_SYS_MmzFree(mmzBlock.phyAddr, mmzBlock.pVirAddr);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_SYS_MmzFree failed with code 0x%X", ret);
    return 1;
  }
  INFO("Release MMZ Block with u32Width=%u, u32Height=%u, bCache=%s, u64PhyAddr=%lu, u64VirAddr=%p", mmzBlock.width,
       mmzBlock.height, mmzBlock.bCache ? "true" : "false", mmzBlock.phyAddr, mmzBlock.pVirAddr);
  return 0;
}

int getMmzPhyAddrByVirAddr(MmzBlockInfo& mmzBlock) {
  int ret = 0;
  SYS_VIRMEM_INFO_S info;
  ret = HI_MPI_SYS_GetVirMemInfo(mmzBlock.pVirAddr, &info);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_SYS_GetVirMemInfo failed with code 0x%X", ret);
    return ret;
  }
  mmzBlock.phyAddr = info.u64PhyAddr;
  mmzBlock.bCache = (info.bCached == HI_TRUE) ? true : false;
  return 0;
}

int getMmzVirAddrByPhyAddr(MmzBlockInfo& mmzBlock) {
  HI_VOID* pVirAddr = nullptr;
  uint32_t size = mmzBlock.stride * mmzBlock.height * 1.5;
  if (mmzBlock.bCache) {
    if (!(pVirAddr = (uint32_t*)HI_MPI_SYS_MmapCache(mmzBlock.phyAddr, size))) {
      ERROR("HI_MPI_SYS_MmapCache failed, check u32Width=%u, u32Height=%u, bCache=%s, u64PhyAddr=%lu", mmzBlock.width,
            mmzBlock.height, mmzBlock.bCache, mmzBlock.phyAddr);
      return 1;
    }
  } else {
    if (!(pVirAddr = (uint32_t*)HI_MPI_SYS_Mmap(mmzBlock.phyAddr, size))) {
      ERROR("HI_MPI_SYS_Mmap failed, check u32Width=%u, u32Height=%u, bCache=%s, u64PhyAddr=%lu", mmzBlock.width,
            mmzBlock.height, mmzBlock.bCache, mmzBlock.phyAddr);
      return 1;
    }
  }
  mmzBlock.pVirAddr = pVirAddr;
  return 0;
}

int DumpYuvToFile(const char* filename, const VIDEO_FRAME_INFO_S& frameInfo) noexcept {
  FILE* fout = fopen(filename, "wb");
  if (nullptr == fout) {
    ERROR("Open filename=%s failed", filename);
    return 1;
  }
  int fileSize = 0;
  for (int i = 0; i < frameInfo.stVFrame.u32Height; ++i) {
    fileSize += fwrite(frameInfo.stVFrame.u64VirAddr[0] + i * frameInfo.stVFrame.u32Stride[0], 1,
                       frameInfo.stVFrame.u32Width, fout);
  }
  for (int i = 0; i < frameInfo.stVFrame.u32Height / 2; ++i) {
    fileSize += fwrite(frameInfo.stVFrame.u64VirAddr[1] + i * frameInfo.stVFrame.u32Stride[1], 1,
                       frameInfo.stVFrame.u32Width, fout);
  }
  fclose(fout);
  DEBUG("total write bytes=%d to file=%s", fileSize, filename);
  return 0;
}

int DumpYuvToFile(const char* filename, const VbBlockInfo& frame) noexcept {
  FILE* fout = fopen(filename, "wb");
  if (!fout) {
    ERROR("Open file=%s fail", filename);
    return 1;
  }
  int fileSize = 0;
  for (int i = 0; i < frame.height * 1.5; ++i) {
    fileSize += fwrite(frame.pVirAddr + frame.width * i, 1, frame.width, fout);
  }
  fclose(fout);
  DEBUG("total write bytes=%d to file=%s", fileSize, filename);
  return 0;
}

int DumpYuvToFile(const char* filename, const uint64_t* pVirAddr, const uint32_t& width, const uint32_t& height,
                  const uint32_t& stride) noexcept {
  FILE* fout = fopen(filename, "wb");
  if (!fout) {
    ERROR("Open filename=%s fail", filename);
    return 1;
  }
  int fileSize = 0;
  for (int i = 0; i < height * 1.5; ++i) {
    fileSize += fwrite(pVirAddr + stride * i, 1, width, fout);
  }
  fclose(fout);
  DEBUG("total write bytes=%d to file=%s", fileSize, filename);
  return 0;
}

int CopyYuv(const uint8_t* srcVirAddr, const uint64_t& srcPhyAddr, const uint32_t& srcWidth, const uint32_t& srcHeight,
            const uint32_t& srcStride, const uint8_t* dstVirAddr, const uint64_t& dstPhyAddr, const uint32_t& dstWidth,
            const uint32_t& dstHeight, const uint32_t& dstStride) noexcept {
  int ret = 0;
  if (srcWidth <= IVE_MAX_WIDTH && srcHeight <= IVE_MAX_HEIGHT && dstWidth <= IVE_MAX_WIDTH &&
      dstHeight <= IVE_MAX_HEIGHT) {
    ret = copyByDma(srcVirAddr, srcPhyAddr, srcWidth, srcHeight, srcStride, dstVirAddr, dstPhyAddr, dstWidth, dstHeight,
                    dstStride);
  } else {
    int loop = (srcWidth * srcHeight) / (IVE_MAX_WIDTH * IVE_MAX_HEIGHT);
    int hr = (srcWidth * srcHeight / IVE_MAX_WIDTH) % IVE_MAX_HEIGHT;
    int r = (srcWidth * srcHeight) % IVE_MAX_WIDTH;
    uint64_t _srcVirAddr = srcVirAddr;
    uint64_t _srcPhyAddr = srcPhyAddr;
    uint64_t _dstVirAddr = dstVirAddr;
    uint64_t _dstPhyAddr = dstPhyAddr;
    for (int i = 0; i < loop; ++i) {
      ret |= copyByDma(_srcVirAddr, _srcPhyAddr, IVE_MAX_WIDTH, IVE_MAX_HEIGHT, IVE_MAX_WIDTH, _dstVirAddr, _dstPhyAddr,
                       IVE_MAX_WIDTH, IVE_MAX_HEIGHT, IVE_MAX_WIDTH);
      _srcVirAddr += IVE_MAX_WIDTH * IVE_MAX_HEIGHT;
      _srcPhyAddr += IVE_MAX_WIDTH * IVE_MAX_HEIGHT;
      _dstVirAddr += IVE_MAX_WIDTH * IVE_MAX_HEIGHT;
      _dstPhyAddr += IVE_MAX_WIDTH * IVE_MAX_HEIGHT;
    }
    if (hr) {
      ret |= copyByDma(_srcVirAddr, _srcPhyAddr, IVE_MAX_WIDTH, hr, IVE_MAX_WIDTH, _dstVirAddr, _dstPhyAddr,
                       IVE_MAX_WIDTH, hr, IVE_MAX_WIDTH);
      _srcVirAddr += IVE_MAX_WIDTH * hr;
      _srcPhyAddr += IVE_MAX_WIDTH * hr;
      _dstVirAddr += IVE_MAX_WIDTH * hr;
      _dstPhyAddr += IVE_MAX_WIDTH * hr;
    }
    if (r) {
      memcpy(_dstVirAddr, _srcVirAddr, r);
    }
  }
  return ret;
}

int copyByDma(const uint8_t* srcVirAddr, const uint64_t& srcPhyAddr, const uint32_t& srcWidth,
              const uint32_t& srcHeight, const uint32_t& srcStride, const uint8_t* dstVirAddr,
              const uint64_t& dstPhyAddr, const uint32_t& dstWidth, const uint32_t& dstHeight,
              const uint32_t& dstStride) {
  int ret = 0;
  IVE_DATA_S src;
  IVE_DST_DATA_S dst;
  IVE_HANDLE handler;
  IVE_DMA_CTRL_S dmaCtrl = {IVE_DMA_MODE_DIRECT_COPY, 0};

  src.u64VirAddr = (HI_U64)srcVirAddr;
  src.u64PhyAddr = (HI_U64)srcPhyAddr;
  src.u32Width = srcWidth;
  src.u32Height = srcHeight;
  src.u32Stride = srcStride;

  dst.u64VirAddr = (HI_U64)dstVirAddr;
  dst.u64PhyAddr = (HI_U64)dstPhyAddr;
  dst.u32Width = dstWidth;
  dst.u32Height = dstHeight;
  dst.u32Stride = dstStride;

  HI_MPI_SYS_MmzFlushCache(src.u64PhyAddr, src.u64VirAddr, src.u32Stride * src.u32Height);
  HI_MPI_SYS_MmzFlushCache(dst.u64PhyAddr, dst.u64VirAddr, dst.u32Stride * dst.u32Height);

  ret = HI_MPI_IVE_DMA(&handler, &src, &dst, &dmaCtrl, HI_TRUE);
  if (HI_SUCCESS != ret) {
    ERROR(
        "HI_MPI_IVE_DMA fail with code 0x%X, check srcWidth=%u, srcHeight=%u, srcStride=%u, srcVirAddr=%p, "
        "srcPhyAddr=%p, dstWidth=%u, dstHeight=%u, dstStride=%u, dstVirAddr=%p, dstPhyAddr=%p",
        ret, src.u32Width, src.u32Height, src.u32Stride, src.u64VirAddr, src.u64PhyAddr, dst.u32Width, dst.u32Height,
        dst.u32Stride, dst.u64VirAddr, dst.u64PhyAddr);
    return ret;
  }
  HI_BOOL bFinish = HI_FALSE;
  while (HI_ERR_IVE_QUERY_TIMEOUT == (ret = HI_MPI_IVE_Query(handler, &bFinish, HI_TRUE))) {
    DEBUG("copying, continue wait");
    usleep(10);
  }
  return 0;
}

}  // namespace rapidcv_mpp