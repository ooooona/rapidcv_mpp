#ifndef YT_HISI_RAPIDCV_MPP_VENC_H_
#define YT_HISI_RAPIDCV_MPP_VENC_H_

#include <unistd.h>

#include "common.h"
#include "hi_comm_venc.h"
#include "mpi_venc.h"
#include "rapidcv_mpp.h"

namespace rapidcv_mpp {

int initVenc();
int deinitVenc();
int jpegEncode(const VbFrameInfo& vbFrame, int quality, const bool& bCrop, const Rect& rect, JpegOutInfo& infoOut,
               const int& timeout_ms = 100, const bool bDump = false, const char* filename = "");

int startVenc(const VENC_CHN& chnId, const VENC_CHN_ATTR_S& chnAttr);
int stopVenc(const VENC_CHN& chnId);

int stopChn(const VENC_CHN& chnId);
int startJpegChn(const VENC_CHN& chnId, const int& quality, const bool& bCrop, const Rect& rect);
int sendFrame(const VENC_CHN& chnId, const VbFrameInfo& vbFrame);
int getFrame(const VENC_CHN& chnId, uint8_t* pVirAddr, uint32_t& size, const int& timeout_ms = 100);
int getFrame2(const VENC_CHN& chnId, uint8_t* pVirAddr, uint32_t& size, const int& timeout_ms = 100);

}  // namespace rapidcv_mpp
#endif  // YT_HISI_RAPIDCV_MPP_VENC_H_