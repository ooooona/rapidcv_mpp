#ifndef YT_HISI_RAPIDCV_MPP_VPSS_H_
#define YT_HISI_RAPIDCV_MPP_VPSS_H_

#include "common.h"
#include "hi_comm_vpss.h"
#include "mpi_vpss.h"
#include "rapidcv_mpp.h"

namespace rapidcv_mpp {
int initVpss();
int deinitVpss();
int procYUV(const VbBlockInfo& vbBlock, const CropInfo& cropInfo, MmzBlockInfo& dstFrame);

int startGrp(const uint32_t& grpId, const uint32_t& width, const uint32_t& height);
int stopGrp(const uint32_t& grpId);

int startChn(const uint32_t& grpId, const VPSS_CHN& chnId);
int stopChn(const uint32_t& grpId, const VPSS_CHN& chnId);
int resetChn(const uint32_t& grpId, const VPSS_CHN& chnId, const CropInfo& cropInfo);
int attachVbPool(const uint32_t& grpId, const VPSS_CHN& chnId, const uint32_t& plId);
int detachVbPool(const uint32_t& grpId, const VPSS_CHN& chnId);

int sendFrame(const uint32_t& grpId, const VbBlockInfo& vbBlock);
int getFrame(const uint32_t& grpId, const VPSS_CHN& chnId, MmzBlockInfo& dstFrame);

int getGrpAttr(const uint32_t& grpId, uint32_t& width, uint32_t& height);

}  // namespace rapidcv_mpp
#endif  // YT_HISI_RAPIDCV_MPP_VPSS_H_