#ifndef YT_HISI_RAPIDCV_MPP_COMMON_H_
#define YT_HISI_RAPIDCV_MPP_COMMON_H_

#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>

#include "hi_buffer.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "rapidcv_mpp.h"

namespace rapidcv_mpp {
#define RPCV_VPSS_GRP_ID_0 0
#define RPCV_VPSS_YUV_PROC_GRP_ID 13
#define RPCV_VPSS_GRP_PIPE_ID 0
#define RPCV_VPSS_ENLARGE_CHN_ID 0
#define RPCV_VPSS_SHRINK_CHN_ID 1
#define RPCV_VPSS_CROP_CHN_ID 2

#define RPCV_VENC_JPEG_CHN_ID 13

#ifdef USE_3519
#define DEFAULE_SYS_MAX_WIDTH 3840
#define DEFAULE_SYS_MAX_HEIGHT 2160
#else
#define DEFAULE_SYS_MAX_WIDTH 2560
#define DEFAULE_SYS_MAX_HEIGHT 1440
#endif

#define IVE_MAX_WIDTH 1920
#define IVE_MAX_HEIGHT 1080

extern uint32_t g_max_width;
extern uint32_t g_max_height;

extern HI_U32 g_vpss_vb_plid;
extern HI_U32 g_venc_vb_plid;

extern HI_U32 g_venc_frame_seq;

struct VbBlockInfo {
  HI_U64 phyAddr;
  HI_U8* pVirAddr;

  HI_U32 height;
  HI_U32 width;

  HI_U32 size;
  HI_U32 plId;
};

struct VbFrameInfo {
  struct VbBlockInfo data;
  uint64_t timestamp;
  uint32_t seq;
};

struct CropInfo {
  bool bCrop;
  struct Rect rect;
};

int mmzBlk2vbBlk(const MmzBlockInfo& mmzBlock, VbBlockInfo& vbBlock);
int vbBlk2mmzBlk(const VbBlockInfo& vbBlock, MmzBlockInfo& mmzBlock);
int videoFrameInfoS2mmzBlk(const VIDEO_FRAME_INFO_S& frame, MmzBlockInfo& mmzBlock);
int vbBlk2videoFrameInfo(const VbBlockInfo& vbBlock, VIDEO_FRAME_INFO_S& frame);
int vbFrameInfo2videoFrameInfo(const VbFrameInfo& vbFrame, VIDEO_FRAME_INFO_S& frame);

int getMmzBlock(MmzBlockInfo& mmzBlock);
int flushMmzBlock(MmzBlockInfo& mmzBlock);
int releaseMmzBlock(const MmzBlockInfo& mmzBlock);
int getMmzPhyAddrByVirAddr(MmzBlockInfo& mmzBlock);
int getMmzVirAddrByPhyAddr(MmzBlockInfo& mmzBlock);

int DumpYuvToFile(const char* filename, const VIDEO_FRAME_INFO_S& frameInfo) noexcept;
int DumpYuvToFile(const char* filename, const VbBlockInfo& frame) noexcept;
int DumpYuvToFile(const char* filename, const uint64_t& pVirAddr, const uint32_t& width, const uint32_t& height,
                  const uint32_t& stride) noexcept;

int CopyYuv(const uint8_t* srcVirAddr, const uint64_t& srcPhyAddr, const uint32_t& srcWidth, const uint32_t& srcHeight,
            const uint32_t& srcStride, const uint8_t* dstVirAddr, const uint64_t& dstPhyAddr, const uint32_t& dstWidth,
            const uint32_t& dstHeight, const uint32_t& dstStride) noexcept;
int copyByDma(const uint8_t* srcVirAddr, const uint64_t& srcPhyAddr, const uint32_t& srcWidth,
              const uint32_t& srcHeight, const uint32_t& srcStride, const uint8_t* dstVirAddr,
              const uint64_t& dstPhyAddr, const uint32_t& dstWidth, const uint32_t& dstHeight,
              const uint32_t& dstStride);

template <typename Clock = std::chrono::high_resolution_clock>
class basic_timer {
 public:
  // clang-format off
  using timepoint_t = typename std::chrono::time_point<Clock, typename Clock::duration>;
  using ns = std::chrono::nanoseconds;
  using us = std::chrono::microseconds;
  using ms = std::chrono::milliseconds;
  using  s = std::chrono::seconds;
  using  m = std::chrono::minutes;
  using  h = std::chrono::hours;
  // clang-format on

 public:
  void tick() { this->m_tick = Clock::now(); }

  void tick(const timepoint_t& tp) { this->m_tick = tp; }

  void tock() { this->m_tock = Clock::now(); }

  void tock(const timepoint_t& tp) { this->m_tock = tp; }

  template <typename DurationType>
  int elapse() const {
    using namespace std::chrono;
    return this->elapse_dispatch<DurationType>();
  }

#define DEFINE_ELAPSE_HELPER(DURATION_TYPE) \
  int elapse_##DURATION_TYPE() const { return this->elapse_dispatch<DURATION_TYPE>(); }

  DEFINE_ELAPSE_HELPER(ns)
  DEFINE_ELAPSE_HELPER(us)
  DEFINE_ELAPSE_HELPER(ms)
  DEFINE_ELAPSE_HELPER(s)
  DEFINE_ELAPSE_HELPER(m)
  DEFINE_ELAPSE_HELPER(h)

 private:  // methods
  template <typename DurationType>
  int elapse_dispatch() const {
    using namespace std::chrono;
    return duration_cast<DurationType>(m_tock - m_tick).count();
  }

 private:
  timepoint_t m_tick;
  timepoint_t m_tock;
};

}  // namespace rapidcv_mpp
#endif  // YT_HISI_RAPIDCV_MPP_COMMON_H_