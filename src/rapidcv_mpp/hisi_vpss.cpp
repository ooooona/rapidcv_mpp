#include "hisi_vpss.h"

#include "hisi_vb.h"
#include "log.h"

namespace rapidcv_mpp {

int initVpss() {
  int ret = 0;
  if (g_vpss_vb_plid == VB_INVALID_POOLID) {
    g_vpss_vb_plid = initPvtVbPool(2, g_max_width, g_max_height);
    if (g_vpss_vb_plid == VB_INVALID_POOLID) {
      return 1;
    }
  }
  ret = startGrp(RPCV_VPSS_YUV_PROC_GRP_ID, g_max_width, g_max_height);
  if (HI_SUCCESS != ret) {
    if (g_vpss_vb_plid != VB_INVALID_POOLID) {
      deinitPvtVbPool(g_vpss_vb_plid);
    }
    return ret;
  }
  return 0;
}

int deinitVpss() {
  int ret = 0;
  if (g_vpss_vb_plid != VB_INVALID_POOLID) {
    ret = deinitPvtVbPool(g_vpss_vb_plid);
  }
  ret |= stopGrp(RPCV_VPSS_YUV_PROC_GRP_ID);
  if (HI_SUCCESS != ret) {
    return ret;
  }
  return 0;
}

int startGrp(const uint32_t& grpId, const uint32_t& width, const uint32_t& height) {
  int ret = 0;
  VPSS_GRP_ATTR_S grpAttr = {0};
  grpAttr.u32MaxW = width;
  grpAttr.u32MaxH = height;
  grpAttr.bNrEn = HI_FALSE;
  grpAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
  grpAttr.stFrameRate.s32SrcFrameRate = -1;
  grpAttr.stFrameRate.s32DstFrameRate = -1;
  ret = HI_MPI_VPSS_CreateGrp(grpId, &grpAttr);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VPSS_EXIST != ret) {
      ERROR("HI_MPI_VPSS_CreateGrp failed with code 0x%X", ret);
      return ret;
    } else {
      WARN("HI_MPI_VPSS_CreateGrp failed with code 0x%X", ret);
    }
  }
  CropInfo cropInfo = {0};
  cropInfo.rect.width = width;
  cropInfo.rect.height = height;
  for (HI_S32 i = 0; i < VPSS_MAX_PHY_CHN_NUM && i < 3; ++i) {
    cropInfo.bCrop = (i == RPCV_VPSS_CROP_CHN_ID) ? true : false;
    ret = resetChn(grpId, i, cropInfo);
    if (ret != 0) {
      continue;
    }
    // ret = HI_MPI_VPSS_EnableChn(grpId, i);
    // if (HI_SUCCESS != ret) {
    //   WARN("HI_MPI_VPSS_EnableChn failed with code 0x%X", ret);
    // }
  }
  ret = HI_MPI_VPSS_StartGrp(grpId);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_StartGrp failed with code 0x%X", ret);
    return ret;
  }
  INFO("Start VPSS, GRP ID=%d", grpId);
  return 0;
}

int stopGrp(const uint32_t& grpId) {
  int ret = 0;
  for (HI_S32 i = 0; i < VPSS_MAX_PHY_CHN_NUM; ++i) {
    ret = HI_MPI_VPSS_DisableChn(grpId, i);
    if (HI_SUCCESS != ret) {
      if (HI_ERR_VPSS_UNEXIST != ret) {
        ERROR("HI_MPI_VPSS_DisableChn failed with code 0x%X", ret);
      } else {
        WARN("HI_MPI_VPSS_DisableChn failed with code 0x%X", ret);
      }
      continue;
    }
  }
  ret = HI_MPI_VPSS_StopGrp(grpId);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VPSS_UNEXIST != ret) {
      ERROR("HI_MPI_VPSS_StopGrp failed with code 0x%X", ret);
    } else {
      WARN("HI_MPI_VPSS_StopGrp failed with code 0x%X", ret);
    }
  }
  ret = HI_MPI_VPSS_DestroyGrp(grpId);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VPSS_UNEXIST != ret) {
      ERROR("HI_MPI_VPSS_DestroyGrp failed with code 0x%X", ret);
      return ret;
    } else {
      WARN("HI_MPI_VPSS_DestroyGrp failed with code 0x%X", ret);
    }
  }
  INFO("Stop VPSS, GRP ID=%d", grpId);
  return 0;
}

int getGrpAttr(const uint32_t& grpId, uint32_t& width, uint32_t& height) {
  int ret = 0;
  VPSS_GRP_ATTR_S grpAttr = {0};
  ret = HI_MPI_VPSS_GetGrpAttr((VPSS_GRP)grpId, &grpAttr);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_GetGrpAttr failed with code 0x%X", ret);
    return ret;
  }
  width = grpAttr.u32MaxW;
  height = grpAttr.u32MaxH;
  return 0;
}

int procYUV(const VbBlockInfo& vbBlock, const CropInfo& cropInfo, MmzBlockInfo& dstFrame) {
  int ret = 0;
  VPSS_CHN chnId = 0;
  if (cropInfo.bCrop) {
    chnId = RPCV_VPSS_CROP_CHN_ID;
  } else {
    if (cropInfo.rect.width >= vbBlock.width || cropInfo.rect.height >= vbBlock.height) {
      chnId = RPCV_VPSS_ENLARGE_CHN_ID;
    } else {
      chnId = RPCV_VPSS_SHRINK_CHN_ID;
    }
  }
  ret = stopChn(RPCV_VPSS_YUV_PROC_GRP_ID, chnId);
  if (ret != 0) {
    return ret;
  }
  ret = startChn(RPCV_VPSS_YUV_PROC_GRP_ID, chnId);
  if (ret != 0) {
    return ret;
  }
  ret = resetChn(RPCV_VPSS_YUV_PROC_GRP_ID, chnId, cropInfo);
  if (ret != 0) {
    return ret;
  }
  ret = sendFrame(RPCV_VPSS_YUV_PROC_GRP_ID, vbBlock);
  if (ret != 0) {
    return ret;
  }
  ret = getFrame(RPCV_VPSS_YUV_PROC_GRP_ID, chnId, dstFrame);
  if (ret != 0) {
    return ret;
  }
  ret = stopChn(RPCV_VPSS_YUV_PROC_GRP_ID, chnId);
  if (ret != 0) {
    return ret;
  }
  return HI_SUCCESS;
}

int startChn(const uint32_t& grpId, const VPSS_CHN& chnId) {
  int ret = 0;
  ret = HI_MPI_VPSS_EnableChn(grpId, chnId);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_EnableChn failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int stopChn(const uint32_t& grpId, const VPSS_CHN& chnId) {
  int ret = 0;
  ret = HI_MPI_VPSS_DisableChn(grpId, chnId);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VPSS_UNEXIST != ret) {
      ERROR("HI_MPI_VPSS_DisableChn failed with code 0x%X", ret);
      return ret;
    } else {
      WARN("HI_MPI_VPSS_DisableChn failed with code 0x%X", ret);
    }
  }
  return 0;
}

int resetChn(const uint32_t& grpId, const VPSS_CHN& chnId, const CropInfo& cropInfo) {
  int ret = 0;
  VPSS_CHN_ATTR_S chnAttr;
  ret = HI_MPI_VPSS_GetChnAttr(grpId, chnId, &chnAttr);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_GetChnAttr failed with code 0x%X with GrpId=%u, ChnId=%u", ret, grpId, chnId);
    return ret;
  }
  chnAttr.u32Width = cropInfo.rect.width;
  chnAttr.u32Height = cropInfo.rect.height;
  chnAttr.enChnMode = VPSS_CHN_MODE_USER;
  chnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
  chnAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
  chnAttr.enCompressMode = COMPRESS_MODE_NONE;
  chnAttr.bMirror = HI_FALSE;
  chnAttr.bFlip = HI_FALSE;
  chnAttr.u32Depth = 1;
  ret = HI_MPI_VPSS_SetChnAttr(grpId, chnId, &chnAttr);
  if (HI_SUCCESS != ret) {
    ERROR(
        "HI_MPI_VPSS_SetChnAttr failed with code 0x%X with GrpId=%u, ChnId=%u, u32Width=%u, u32Height=%u, topX=%u, "
        "topY=%u, bCrop=%s",
        ret, grpId, chnId, cropInfo.rect.width, cropInfo.rect.height, cropInfo.rect.topX, cropInfo.rect.topY,
        cropInfo.bCrop ? "true" : "false");
    return ret;
  }
  if (cropInfo.bCrop) {
    VPSS_CROP_INFO_S info;
    ret = HI_MPI_VPSS_GetChnCrop(grpId, chnId, &info);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VPSS_GetChnCrop failed with code 0x%X with GrpId=%u, ChnId=%u", ret, grpId, chnId);
      return ret;
    }
    info.bEnable = HI_TRUE;
    info.enCropCoordinate = VPSS_CROP_ABS_COOR;
    info.stCropRect.s32X = cropInfo.rect.topX;
    info.stCropRect.s32Y = cropInfo.rect.topY;
    info.stCropRect.u32Width = cropInfo.rect.width;
    info.stCropRect.u32Height = cropInfo.rect.height;
    ret = HI_MPI_VPSS_SetChnCrop(grpId, chnId, &info);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VPSS_SetChnCrop failed with code 0x%X with GrpId=%u, ChnId=%u", ret, grpId, chnId);
      return ret;
    }
  }
  DEBUG("init vpss chn: GrpId=%u, ChnId=%u, u32Width=%u, u32Height=%u, topX=%u, topY=%u, bCrop=%s", grpId, chnId,
        cropInfo.rect.width, cropInfo.rect.height, cropInfo.rect.topX, cropInfo.rect.topY,
        cropInfo.bCrop ? "true" : "false");
  return 0;
}

int attachVbPool(const uint32_t& grpId, const VPSS_CHN& chnId, const uint32_t& plId) {
  int ret = 0;
  ret = HI_MPI_VPSS_AttachVbPool(grpId, chnId, plId);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_AttachVbPool failed with code 0x%X, check GRP_ID=%u, CHN_ID=%u, POOL_ID=%u", ret, grpId, chnId,
          plId);
    return ret;
  }
  return 0;
}

int detachVbPool(const uint32_t& grpId, const VPSS_CHN& chnId) {
  int ret = 0;
  ret = HI_MPI_VPSS_DetachVbPool(grpId, chnId);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_DetachVbPool failed with code 0x%X, check GRP_ID=%u, CHN_ID=%u", ret, grpId, chnId);
    return ret;
  }
  return 0;
}

int getFrame(const uint32_t& grpId, const VPSS_CHN& chnId, MmzBlockInfo& dstFrame) {
  int ret = 0;
  VIDEO_FRAME_INFO_S frame = {0};
  frame.u32PoolId = VB_INVALID_POOLID;
  ret = HI_MPI_VPSS_GetChnFrame(grpId, chnId, &frame, 50);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_GetChnFrame failed with code 0x%X", ret);
    return ret;
  }
  HI_U32 u32Size = frame.stVFrame.u32Stride[0] * frame.stVFrame.u32Height * 1.5;
  HI_VOID* pVirAddr = HI_MPI_SYS_MmapCache(frame.stVFrame.u64PhyAddr[0], u32Size);
  frame.stVFrame.u64VirAddr[0] = pVirAddr;
  frame.stVFrame.u64VirAddr[1] = pVirAddr + frame.stVFrame.u32Stride[0] * frame.stVFrame.u32Height;
  frame.stVFrame.u64VirAddr[2] = pVirAddr + frame.stVFrame.u32Stride[0] * frame.stVFrame.u32Height;
  ret = videoFrameInfoS2mmzBlk(frame, dstFrame);
  if (ret != 0) {
    return ret;
  }

  ret = HI_MPI_SYS_Munmap(pVirAddr, frame.stVFrame.u32Stride[0] * frame.stVFrame.u32Height * 1.5);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_SYS_Munmap failed with code 0x%X, check pVirAddr=%p, u32Size=%u", ret, pVirAddr, u32Size);
  }
  ret = HI_MPI_VPSS_ReleaseChnFrame(grpId, 0, &frame);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_ReleaseChnFrame failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int sendFrame(const uint32_t& grpId, const VbBlockInfo& vbBlock) {
  int ret = 0;
  VIDEO_FRAME_INFO_S frame;
  ret = vbBlk2videoFrameInfo(vbBlock, frame);
  if (ret != 0) {
    return ret;
  }
  ret = HI_MPI_VPSS_SendFrame(grpId, RPCV_VPSS_GRP_PIPE_ID, &frame, -1);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VPSS_SendFrame failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

}  // namespace rapidcv_mpp
