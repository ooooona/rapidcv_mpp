#include "hisi_vb.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"

namespace rapidcv_mpp {
int initModVbPool(const uint16_t& blkCnt, const uint32_t& width, const uint32_t& height, const VB_UID_E& plid) {
  HI_S32 ret = 0;
  ret = HI_MPI_VB_ExitModCommPool(plid);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VB_ExitModCommPool failed with code 0x%X", ret);
    return ret;
  }
  VB_CONFIG_S vbConf = {0};
  vbConf.u32MaxPoolCnt = 1;
  for (int i = 0; i < (int)vbConf.u32MaxPoolCnt; ++i) {
    vbConf.astCommPool[i].u32BlkCnt = blkCnt;
    vbConf.astCommPool[i].u64BlkSize =
        COMMON_GetPicBufferSize(width, height, PIXEL_FORMAT_YUV_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);
  }
  ret = HI_MPI_VB_SetModPoolConfig(plid, &vbConf);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VB_SetModPoolConfig failed with code 0x%X", ret);
    return ret;
  }
  ret = HI_MPI_VB_InitModCommPool(plid);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VB_InitModCommPool failed with code 0x%X", ret);
    return ret;
  }
  return 0;
}

int deinitModVbPool(const VB_UID_E& plid) {
  int ret = 0;
  ret = HI_MPI_VB_ExitModCommPool(plid);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VB_ExitModCommPool failed with code 0x%X", ret);
    if (HI_ERR_VB_UNEXIST != ret) {
      return ret;
    }
  }
  INFO("Exit Module Common Pool");
  return 0;
}

uint32_t initPvtVbPool(const uint16_t& blkCnt, const uint32_t& width, const uint32_t& height) {
  VB_POOL_CONFIG_S vbPoolCfg = {0};
  vbPoolCfg.u32BlkCnt = blkCnt;
  vbPoolCfg.u64BlkSize =
      COMMON_GetPicBufferSize(width, height, PIXEL_FORMAT_YUV_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);
  vbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
  uint32_t plid = HI_MPI_VB_CreatePool(&vbPoolCfg);
  if (VB_INVALID_POOLID == plid) {
    ERROR("HI_MPI_VB_CreatePool failed, chech info:u32BlkCnt=%u, u32Width=%u, u32Hheight=%u, u64BlkSize=%llu", blkCnt,
          width, height, vbPoolCfg.u64BlkSize);
  } else {
    loadVbPoolFile(plid);
    INFO("Create Private VB Pool: u32BlkCnt=%u, u32Width=%u, u32Hheight=%u, u64BlkSize=%llu, PoolId=%u", blkCnt, width,
         height, vbPoolCfg.u64BlkSize, plid);
  }
  return plid;
}

int deinitPvtVbPool(HI_U32& plid) {
  int ret = 0;
  ret = HI_MPI_VB_DestroyPool(plid);
  if (HI_SUCCESS != ret) {
    if (HI_ERR_VB_UNEXIST != ret) {
      ERROR("HI_MPI_VB_DestroyPool with plid=%d, failed with code 0x%X", plid, ret);
      return ret;
    } else {
      WARN("HI_MPI_VB_DestroyPool with plid=%d, failed with code 0x%X", plid, ret);
    }
  }
  INFO("Destroy Private VB Pool, Pool ID=%d", plid);
  unloadVbPoolFile(plid);
  plid = VB_INVALID_POOLID;
  return 0;
}

int getVbBlock(VbBlockInfo& vbBlock) {
  int ret = 0;
  vbBlock.size = COMMON_GetPicBufferSize(vbBlock.width, vbBlock.height, PIXEL_FORMAT_YUV_SEMIPLANAR_420,
                                         DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);
  VB_BLK bid = HI_MPI_VB_GetBlock(vbBlock.plId, vbBlock.size, HI_NULL);
  if (VB_INVALID_HANDLE == bid) {
    ERROR("HI_MPI_VB_GetBlock failed with poolId=%u, u32Width=%u, u32Height=%u, u32Size=%u", vbBlock.plId,
          vbBlock.width, vbBlock.height, vbBlock.size);
    return 1;
  }
  HI_U64 phyAddr = HI_MPI_VB_Handle2PhysAddr(bid);
  if (!phyAddr) {
    ERROR("HI_MPI_VB_Handle2PhysAddr failed with poolId=%u, u32Width=%u, u32Height=%u, u32Size=%u", vbBlock.plId,
          vbBlock.width, vbBlock.height, vbBlock.size);
    ret = HI_MPI_VB_ReleaseBlock(bid);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VB_ReleaseBlock failed with code 0x%X", ret);
    } else {
      WARN("HI_MPI_VB_ReleaseBlock succeed");
    }
    return 2;
  }
  HI_VOID* pVirAddr = HI_MPI_SYS_Mmap(phyAddr, vbBlock.size);
  if (!pVirAddr) {
    ERROR("HI_MPI_SYS_Mmap failed with poolId=%u, u32Width=%u, u32Height=%u, u32Size=%u", vbBlock.plId, vbBlock.width,
          vbBlock.height, vbBlock.size);
    ret = HI_MPI_VB_ReleaseBlock(bid);
    if (HI_SUCCESS != ret) {
      ERROR("HI_MPI_VB_ReleaseBlock failed with code 0x%X", ret);
    } else {
      WARN("HI_MPI_VB_ReleaseBlock succeed");
    }
    return 3;
  }
  vbBlock.phyAddr = phyAddr;
  vbBlock.pVirAddr = (HI_U8*)pVirAddr;
  INFO("VB Block info: poolId=%u, u32Width=%u, u32Height=%u, u32Size=%u, phyAddr=%lu, pVirAddr=%p", vbBlock.plId,
       vbBlock.width, vbBlock.height, vbBlock.size, vbBlock.phyAddr, *vbBlock.pVirAddr);
  return 0;
}

int getVbBlkFromMmzBlk(const MmzBlockInfo& mmzBlock, const HI_U32& plId, VbBlockInfo& vbBlock) {
  vbBlock.plId = plId;
  vbBlock.width = mmzBlock.width;
  vbBlock.height = mmzBlock.height;
  int ret = 0;
  ret = getVbBlock(vbBlock);
  if (ret != 0) {
    return ret;
  }
  ret = mmzBlk2vbBlk(mmzBlock, vbBlock);
  if (ret != 0) {
    releaseVbBlock(vbBlock);
    return ret;
  }
  return ret;
}

int releaseVbBlock(VbBlockInfo& vbBlock) {
  int ret = 0;
  ret = HI_MPI_SYS_Munmap(vbBlock.pVirAddr, vbBlock.size);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_SYS_Munmap failed with code 0x%X, check pVirAddr=%p, u32Size=%u", ret, vbBlock.pVirAddr,
          vbBlock.size);
  }
  VB_BLK bid = HI_MPI_VB_PhysAddr2Handle(vbBlock.phyAddr);
  if (VB_INVALID_HANDLE == bid) {
    ERROR("HI_MPI_VB_PhysAddr2Handle failed");
    return 1;
  }
  ret = HI_MPI_VB_ReleaseBlock(bid);
  if (HI_SUCCESS != ret) {
    ERROR("HI_MPI_VB_ReleaseBlock failed with code 0x%X", ret);
    return ret;
  }
  INFO("Release VB Block info: poolId=%u, u32Width=%u, u32Height=%u, vbSize=%u, phyAddr=%lu, pVirAddr=%p", vbBlock.plId,
       vbBlock.width, vbBlock.height, vbBlock.size, vbBlock.phyAddr, vbBlock.pVirAddr)
  return 0;
}

int initVbPoolByFile() {}

int deinitVbPoolByFile() {
  int ret = 0;
  HI_U32 plid = 0;
  char filepath[80];
  DIR* dpdf = nullptr;
  struct dirent* epdf = nullptr;
  if ((dpdf = opendir(VB_DIR)) != nullptr) {
    while (epdf = readdir(dpdf)) {
      if (!epdf->d_name || epdf->d_name[0] == '.') continue;
      plid = 0;
      for (int i = 0; i < strlen(epdf->d_name); ++i) plid = plid * 10 + epdf->d_name[i] - '0';
      ret |= deinitPvtVbPool(plid);
    }
  }
  closedir(dpdf);
  return ret;
}

int loadVbPoolFile(const HI_U32& plid) {
  struct stat st = {0};
  if (stat(VB_DIR, &st) == -1) {
    mkdir(VB_DIR, 0700);
    DEBUG("create directory=%s", VB_DIR);
  }
  char filepath[80];
  snprintf(filepath, sizeof(filepath), "%s%d", VB_DIR, plid);
  FILE* fd = open(filepath, O_WRONLY | O_CREAT);
  if (!fd) {
    WARN("Failed to create filepath=%s", filepath);
    return 1;
  }
  DEBUG("create filepath=%s", filepath);
  close(fd);
  return 0;
}

int unloadVbPoolFile(const HI_U32& plid) {
  char filepath[80];
  snprintf(filepath, sizeof(filepath), "%s%d", VB_DIR, plid);
  int ret = remove(filepath);
  if (ret != 0) {
    WARN("Failed to remove filepath=%s, err: %s", filepath, strerror(errno));  // No such file or directory
  } else {
    DEBUG("remove filepath=%s", filepath);
  }
  return ret;
}

}  // namespace rapidcv_mpp