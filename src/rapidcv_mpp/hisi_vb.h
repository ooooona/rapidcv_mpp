#ifndef YT_HISI_RAPIDCV_MPP_VB_H_
#define YT_HISI_RAPIDCV_MPP_VB_H_

#include "common.h"

#define VB_DIR "/tmp/rpdcvmpp_vb/"

namespace rapidcv_mpp {

int initModVbPool(const uint16_t& blkCnt, const uint32_t& width, const uint32_t& height, const VB_UID_E& plid);
int deinitModVbPool(const VB_UID_E& plid);

uint32_t initPvtVbPool(const uint16_t& blkCnt, const uint32_t& width, const uint32_t& height);
int deinitPvtVbPool(HI_U32& plid);

int getVbBlock(VbBlockInfo& vbBlock);
int releaseVbBlock(VbBlockInfo& vbBlock);

int getVbBlkFromMmzBlk(const MmzBlockInfo& mmzBlock, const HI_U32& plId, VbBlockInfo& vbBlock);

int initVbPoolByFile();
int deinitVbPoolByFile();

int loadVbPoolFile(const HI_U32& plid);
int unloadVbPoolFile(const HI_U32& plid);

}  // namespace rapidcv_mpp

#endif  // YT_HISI_RAPIDCV_MPP_VB_H_