#include "hisi_venc.h"

#include "hisi_vb.h"
#include "log.h"

namespace rapidcv_mpp {

int initVenc() {
  int ret = 0;
  if (g_venc_vb_plid == VB_INVALID_POOLID) {
    g_venc_vb_plid = initPvtVbPool(2, g_max_width, g_max_height);
    if (g_venc_vb_plid == VB_INVALID_POOLID) {
      return 1;
    }
  }
  VENC_CHN_ATTR_S chnAttr = {0};
  chnAttr.stVencAttr.enType = PT_JPEG;
  chnAttr.stVencAttr.bByFrame = HI_TRUE;
  chnAttr.stVencAttr.u32PicWidth = g_max_width;
  chnAttr.stVencAttr.u32PicHeight = g_max_height;
  chnAttr.stVencAttr.u32MaxPicWidth = g_max_width;
  chnAttr.stVencAttr.u32MaxPicHeight = g_max_height;
  chnAttr.stVencAttr.u32BufSize = COMMON_GetPicBufferSize(g_max_width, g_max_height, PIXEL_FORMAT_YUV_SEMIPLANAR_420,
                                                          DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);

  ret = startVenc(RPCV_VENC_JPEG_CHN_ID, chnAttr);
  if (ret == HI_ERR_VENC_EXIST) {
    if (stopVenc(RPCV_VENC_JPEG_CHN_ID) != 0) {
      deinitPvtVbPool(g_venc_vb_plid);
      return ret;
    }
    ret = startVenc(RPCV_VENC_JPEG_CHN_ID, chnAttr);
    if (ret != 0) {
      deinitPvtVbPool(g_venc_vb_plid);
      return ret;
    }
  } else if (ret != 0) {
    deinitPvtVbPool(g_venc_vb_plid);
    return ret;
  }
  return 0;
}

int deinitVenc() {
  int ret = 0;
  if (g_venc_vb_plid != VB_INVALID_POOLID) {
    ret = deinitPvtVbPool(g_venc_vb_plid);
  }
  ret |= stopVenc(RPCV_VENC_JPEG_CHN_ID);
  if (ret != 0) {
    return ret;
  }
  return 0;
}

int startVenc(const VENC_CHN& chnId, const VENC_CHN_ATTR_S& chnAttr) {
  HI_S32 ret = HI_SUCCESS;
  ret = HI_MPI_VENC_CreateChn(chnId, &chnAttr);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VENC_EXIST != ret) {
      ERROR("HI_MPI_VENC_CreateChn failed with code 0x%X", ret);
    } else {
      WARN("HI_MPI_VENC_CreateChn failed with code 0x%X", ret);
    }
    return ret;

  } else {
    DEBUG(
        "venc chn attr: stVencAttr.enType=%d, stVencAttr.bByFrame=%d, stVencAttr.u32PicWidth=%d, "
        "stVencAttr.u32PicHeight=%d, stVencAttr.u32MaxPicWidth=%d, stVencAttr.u32MaxPicHeight=%d, "
        "stVencAttr.u32BufSize=%d",
        chnAttr.stVencAttr.enType, chnAttr.stVencAttr.bByFrame, chnAttr.stVencAttr.u32PicWidth,
        chnAttr.stVencAttr.u32PicHeight, chnAttr.stVencAttr.u32MaxPicWidth, chnAttr.stVencAttr.u32MaxPicHeight,
        chnAttr.stVencAttr.u32BufSize);
  }
  VENC_RECV_PIC_PARAM_S param = {0};
  param.s32RecvPicNum = -1;
  ret = HI_MPI_VENC_StartRecvFrame(chnId, &param);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VENC_EXIST != ret) {
      ERROR("HI_MPI_VENC_StartRecvFrame failed with code 0x%X", ret);
    } else {
      WARN("HI_MPI_VENC_StartRecvFrame failed with code 0x%X", ret);
    }
    return ret;
  }
  INFO("Start VENC, CHN ID=%d", chnId);
  return 0;
}

int stopVenc(const VENC_CHN& chnId) {
  int ret = 0;
  ret = HI_MPI_VENC_StopRecvFrame(chnId);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VENC_UNEXIST != ret) {
      ERROR("HI_MPI_VENC_StopRecvFrame failed with code 0x%X", ret);
    } else {
      WARN("HI_MPI_VENC_StopRecvFrame failed with code 0x%X", ret);
    }
  }
  ret = HI_MPI_VENC_DestroyChn(chnId);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VENC_UNEXIST != ret) {
      ERROR("HI_MPI_VENC_DestroyChn failed with code 0x%X", ret);
      return ret;
    } else {
      WARN("HI_MPI_VENC_DestroyChn failed with code 0x%X", ret);
    }
  }
  INFO("Stop VENC, CHN ID=%d", chnId);
  return 0;
}

int jpegEncode(const VbFrameInfo& vbFrame, int quality, const bool& bCrop, const Rect& rect, JpegOutInfo& infoOut,
               const int& timeout_ms /*= 100 */, const bool bDump /* = false*/, const char* filename /* = ""*/) {
  int ret = 0;
  ret = stopChn(RPCV_VENC_JPEG_CHN_ID);
  if (ret != 0) {
    return ret;
  }
  ret = startJpegChn(RPCV_VENC_JPEG_CHN_ID, quality, bCrop, rect);
  if (ret != 0) {
    return ret;
  }
  ret = sendFrame(RPCV_VENC_JPEG_CHN_ID, vbFrame);
  if (ret != 0) {
    return ret;
  }
  infoOut.size = rect.width * rect.height;
  ret = getFrame2(RPCV_VENC_JPEG_CHN_ID, infoOut.pVirAddr, infoOut.size, timeout_ms);
  if (ret != 0) {
    return ret;
  }
  ret = stopChn(RPCV_VENC_JPEG_CHN_ID);
  if (ret != 0) {
    return ret;
  }
  if (bDump) {
    DumpJpeg(filename, infoOut);
  }
  return 0;
}

int stopChn(const VENC_CHN& chnId) {
  int ret = 0;
  ret = HI_MPI_VENC_StopRecvFrame(chnId);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VENC_StopRecvFrame failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int startJpegChn(const VENC_CHN& chnId, const int& quality, const bool& bCrop, const Rect& rect) {
  int ret = 0;
  VENC_CHN_ATTR_S chnAttr;
  VENC_CHN_PARAM_S chnParam;
  VENC_JPEG_PARAM_S jpegParam;
  HI_MPI_VENC_GetChnAttr(chnId, &chnAttr);
  HI_MPI_VENC_GetChnParam(chnId, &chnParam);
  HI_MPI_VENC_GetJpegParam(chnId, &jpegParam);
  if (chnAttr.stVencAttr.u32PicWidth != rect.width || chnAttr.stVencAttr.u32PicHeight != rect.height) {
    chnAttr.stVencAttr.u32PicWidth = rect.width;
    chnAttr.stVencAttr.u32PicHeight = rect.height;
    ret = HI_MPI_VENC_SetChnAttr(chnId, &chnAttr);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VENC_SetChnAttr failed with code 0x%X, check u32PicWidth=%u, u32PicHeight=%u", ret,
            chnAttr.stVencAttr.u32PicWidth, chnAttr.stVencAttr.u32PicHeight);
      return ret;
    }
  }
  if (bCrop && (chnParam.stCropCfg.bEnable != HI_TRUE || chnParam.stCropCfg.stRect.s32X != rect.topX ||
                chnParam.stCropCfg.stRect.s32Y != rect.topY || chnParam.stCropCfg.stRect.u32Width != rect.width ||
                chnParam.stCropCfg.stRect.u32Height != rect.height)) {
    // if (bCrop) {
    chnParam.stCropCfg.bEnable = HI_TRUE;
    chnParam.stCropCfg.stRect.s32X = rect.topX;
    chnParam.stCropCfg.stRect.s32Y = rect.topY;
    chnParam.stCropCfg.stRect.u32Width = rect.width;
    chnParam.stCropCfg.stRect.u32Height = rect.height;
    ret = HI_MPI_VENC_SetChnParam(chnId, &chnParam);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VENC_SetChnParam failed with code 0x%X, check s32X=%u, s32Y=%u, u32Width=%u, u32Height=%u", ret,
            chnParam.stCropCfg.stRect.s32X, chnParam.stCropCfg.stRect.s32Y, chnParam.stCropCfg.stRect.u32Width,
            chnParam.stCropCfg.stRect.u32Height);
      return ret;
    }
  }
  if (jpegParam.u32Qfactor != (HI_U32)quality) {
    jpegParam.u32Qfactor = (HI_U32)quality;
    ret = HI_MPI_VENC_SetJpegParam(chnId, &jpegParam);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VENC_SetJpegParam failed with code 0x%X", ret);
      return ret;
    }
  }
  VENC_RECV_PIC_PARAM_S picParam;
  picParam.s32RecvPicNum = -1;
  ret = HI_MPI_VENC_StartRecvFrame(chnId, &picParam);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VENC_StartRecvFrame failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int sendFrame(const VENC_CHN& chnId, const VbFrameInfo& vbFrame) {
  int ret = 0;
  VIDEO_FRAME_INFO_S vdFrame;
  ret = vbFrameInfo2videoFrameInfo(vbFrame, vdFrame);
  if (ret != 0) {
    return ret;
  }
  // if (g_venc_frame_seq == 0) {
  //   if (vdFrame.stVFrame.u32TimeRef == 0) {
  //     vdFrame.stVFrame.u32TimeRef = 2;
  //   }
  //   g_venc_frame_seq = vdFrame.stVFrame.u32TimeRef;
  // } else {
  //   if (vdFrame.stVFrame.u32TimeRef <= g_venc_frame_seq) {
  //     vdFrame.stVFrame.u32TimeRef = g_venc_frame_seq + 2;
  //   }
  //   g_venc_frame_seq = vdFrame.stVFrame.u32TimeRef;
  // }
  g_venc_frame_seq += 2;
  vdFrame.stVFrame.u32TimeRef = g_venc_frame_seq;
  DEBUG("g_venc_frame_seq=%d, frame.u32TimeRef=%d", g_venc_frame_seq, vdFrame.stVFrame.u32TimeRef);
  ret = HI_MPI_VENC_SendFrame(chnId, &vdFrame, -1);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VENC_SendFrame failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int getFrame(const VENC_CHN& chnId, uint8_t* pVirAddr, uint32_t& size, const int& timeout_ms /*= 100 */) {
  int ret = 0;
  bool flag = false;
  VENC_STREAM_S stream;
  VENC_CHN_STATUS_S status;
  const uint32_t per_sleep_ms = 1;
  basic_timer<> timer;
  timer.tick();
  do {
    timer.tock();
    DEBUG("duration=%dms, timeout_ms=%dms", timer.elapse_ms(), timeout_ms);
    if (timer.elapse_ms() > timeout_ms) {
      break;
    }
    ret = HI_MPI_VENC_QueryStatus(chnId, &status);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VENC_QueryStatus failed with code 0x%X", ret);
      return ret;
    }
    if (0 == status.u32CurPacks && 0 == status.u32LeftStreamFrames) {
      DEBUG("null frame, waiting, u32CurPacks=%d, u32LeftStreamFrames=%d", status.u32CurPacks,
            status.u32LeftStreamFrames);
      usleep(per_sleep_ms * 100);
      continue;
    }

    stream.u32PackCount = status.u32CurPacks;
    stream.pstPack = (VENC_PACK_S*)malloc(sizeof(stream) * status.u32CurPacks);
    if (nullptr == stream.pstPack) {
      ERROR("VENC_STREAM_S.pstPack malloc failed");
      return 1;
    }
    ret = HI_MPI_VENC_GetStream(chnId, &stream, -1);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VENC_GetStream failed with code 0x%X", ret);
      free(stream.pstPack);
      stream.pstPack = nullptr;
      return ret;
    }
    DEBUG(
        "pack info:{u32PackCount=%d, u32Seq=%d}; jpeg info:{stJpegInfo.u32PicBytesNum=%d}, content:{u32Len=%d, "
        "u64PTS=%d, DataType=%d, u32Offset=%d, u32DataNum=%d, bFrameEnd=%d}",
        stream.u32PackCount, stream.u32Seq, stream.stJpegInfo.u32PicBytesNum, stream.stJpegInfo.u32UpdateAttrCnt,
        stream.stJpegInfo.u32Qfactor, stream.pstPack->u32Len, stream.pstPack->u64PTS, stream.pstPack->DataType,
        stream.pstPack->u32Offset, stream.pstPack->u32DataNum, stream.pstPack->bFrameEnd);
    size = 0;
    for (int i = 0; i < stream.u32PackCount; ++i) {
      memcpy(pVirAddr + size, stream.pstPack[i].pu8Addr + stream.pstPack[i].u32Offset,
             stream.pstPack[i].u32Len - stream.pstPack[i].u32Offset);
      size += stream.pstPack[i].u32Len - stream.pstPack[i].u32Offset;
    }
    ret = HI_MPI_VENC_ReleaseStream(chnId, &stream);
    free(stream.pstPack);
    stream.pstPack = nullptr;
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VENC_ReleaseStream failed with code 0x%X", ret);
      return ret;
    }
    if (0 != status.u32CurPacks && status.u32LeftStreamFrames != 0 && 0 == status.u32LeftPics) {
      flag = true;
      break;
    }
  } while (true);
  if (flag) {
    return 0;
  } else {
    ERROR("JPEG Encode timeout");
    return 1;
  }
}

int getFrame2(const VENC_CHN& chnId, uint8_t* pVirAddr, uint32_t& size, const int& timeout_ms /*= 100 */) {
  int ret = 0;
  HI_S32 fd = HI_MPI_VENC_GetFd(chnId);
  if (fd < 0) {
    ERROR("HI_MPI_VENC_GetFd faild with code 0x%X", fd);
    return fd;
  }
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(fd, &read_fds);
  struct timeval timeout;
  timeout.tv_sec = int(timeout_ms / 1000);
  timeout.tv_usec = 1000 * (timeout_ms - 1000 * timeout.tv_sec);
  ret = select(fd + 1, &read_fds, nullptr, nullptr, &timeout);
  if (ret < 0) {
    ERROR("JPEG Encode failed");
    return 1;
  } else if (ret == 0) {
    ERROR("JPEG Encode timeout");
    return 2;
  }
  if (!FD_ISSET(fd, &read_fds)) {
    WARN("JPEG Encode not ready");
    // return 3;
  }

  VENC_STREAM_S stream;
  VENC_CHN_STATUS_S status;
  ret = HI_MPI_VENC_QueryStatus(chnId, &status);
  if (ret != HI_SUCCESS) {
    ERROR("HI_MPI_VENC_QueryStatus failed with code %#x", ret);
    return ret;
  }
  if (0 == status.u32CurPacks) {
    ERROR("Current frame is NULL");
    return 4;
  }
  stream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * status.u32CurPacks);
  if (nullptr == stream.pstPack) {
    ERROR("VENC_STREAM_S.pstPack malloc failed");
    return 4;
  }
  stream.u32PackCount = status.u32CurPacks;
  ret = HI_MPI_VENC_GetStream(chnId, &stream, -1);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VENC_GetStream failed with code %#x", ret);
    free(stream.pstPack);
    stream.pstPack = nullptr;
    return ret;
  }
  size = 0;
  for (int i = 0; i < stream.u32PackCount; ++i) {
    memcpy(pVirAddr + size, stream.pstPack[i].pu8Addr + stream.pstPack[i].u32Offset,
           stream.pstPack[i].u32Len - stream.pstPack[i].u32Offset);
    size += stream.pstPack[i].u32Len - stream.pstPack[i].u32Offset;
  }
  ret = HI_MPI_VENC_ReleaseStream(chnId, &stream);
  free(stream.pstPack);
  stream.pstPack = nullptr;
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VENC_ReleaseStream failed with code %#x", ret);
    return ret;
  }
  return 0;
}

}  // namespace rapidcv_mpp