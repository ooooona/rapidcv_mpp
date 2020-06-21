#ifndef YT_HISI_RAPIDCV_MPP_H_
#define YT_HISI_RAPIDCV_MPP_H_

#include <stdint.h>

namespace rapidcv_mpp {

struct SysCapInfo {
  uint32_t maxWidth;
  uint32_t maxHeight;
};

/**
 * 采用屏幕坐标系，即左上角为坐标起点，横向向右侧为x轴，纵向向下为y轴。以像素为单位。
 * 坐标系形式如下：
 * ----------------> x
 * |
 * |
 * v
 * y
 */
struct Rect {
  int topX;   /* 左上角x坐标值。绝对像素坐标 */
  int topY;   /* 左上角y坐标值。绝对像素坐标 */
  int width;  /* 宽度。像素单位。 */
  int height; /* 高度。像素单位。 */
};

// todo(oona): set log level
enum LogLevel {
  ERROR = 0,
  WARN,
  INFO,
  DEBUG,
};

struct MmzBlockInfo {
  uint64_t phyAddr;  /* mmz物理地址 */
  uint8_t* pVirAddr; /* mmz虚拟地址 */

  uint32_t height; /* mmz高度，即图像高度，也是mmz存储空间的高度 */
  uint32_t width;  /* mmz宽度，即图像宽度 */
  uint32_t stride; /* mmz跨距，即mmz存储空间的宽度。一定有stride>=width */

  bool bCache; /* 是否为可cache的mmz。如果是可cache，确保用flush接口把数据从系统内存同步到mmz内存上 */
};

struct FrameInfo {
  struct MmzBlockInfo data; /* 帧数据内容 */
  uint64_t timestamp;       /* 帧时间 */
  uint32_t seq;             /* 帧序列号 */
};

struct JpegReqParam {
  struct FrameInfo frame; /* JPEG编码原始帧数据，如果是从海思取流得到的帧数据，请确保seq有效；否则置0。 */
  uint16_t quality; /* JPEG编码质量，取值范围:[1，99]，数值越高约清晰、图片尺寸越大 */
  bool bCrop;       /* 是否进行裁剪 */
  struct Rect rect; /* 若bCrop=true,则只编码rect指定的矩形区域，请确保rect区域在frame区域内；
                       否则，按照rect指定的长宽为目标编码长宽，注意若rect指定长宽和frame长宽不一致，则会进行缩放。
                     */
};

struct JpegOutInfo {
  uint32_t size;
  uint8_t* pVirAddr;  // virtual address of mmz
};

/**@brief InitSys
          初始化系统，申请必要系统资源
 * @return	 0成功，其他失败
 * @notice   在调用其他API之前必须确保该函数被成功调用
 */
int InitSys(const uint32_t& maxWidth = 0, const uint32_t& maxHeight = 0) noexcept;

/**@brief SetLogLevel
          设置日志级别
 * @return	 0成功，其他失败
 * @notice   目前暂不支持输出到文件
 */
int SetLogLevel(const LogLevel& _ll);

/**@brief GetSysCaps
          获取系统信息
 * @param [out] info 系统信息
 * @return	 0成功，其他失败
 * @notice   必须在InitSys之后调用，目前无法通过MPP获得sensor分辨率，只能通过外部传入；或者使用默认分辨率。
 *           默认分辨率：Hi3519AV100 - 3840X2160；Hi3516DV300 - 2560X1440。
 */
int GetSysCaps(SysCapInfo& info) noexcept;

/**@brief OpenJpeg
          初始化JPEG功能
 * @return	 0成功，其他失败
 * @notice   必须成功调用本函数才能使用JPEG功能。
 *           全局只需执行一次即可，若已经打开过本函数了，将返回0。
 */
int OpenJpegCap() noexcept;

/**@brief CloseJpegCap
          释放JPEG资源
 * @return	0成功，其他失败
 * @notice  调用OpenJpegCap函数后，必须在程序退出前调用本函数释放资源。
 *          若执行JPEG功能失败，建议调用本函数、配合OpenJpegCap恢复JPEG功能。
 */
int CloseJpegCap() noexcept;

/**@brief JpegEncode
          JPEG编码请求
 * @param [in] reqParam JPEG编码的原数据参数
 *                      frame格式必须是YUV420SP格式。
 *                      frame是分配在mmz上的有效数据，即必须确保frame每个字段为有效值。
 * @param [out] outInfo JPEG编码的输出数据。
 *                      请用户预先分配好mmz内存，即确保pVirAddr为有效值。
 *                      调用该API后，size数值将被修改为JPEG图像实际大小，单位为字节。
 * @param [in] timeout_ms 用户可以设置JPEG编码的超时时间，单位毫秒。默认100ms。
 * @param [in] bDump 是否保存JPEG图片
 * @param [in] filename 若bDump=true,则按filename保存图片
 * @return  0成功，其他失败
 */
int JpegEncode(const JpegReqParam& reqParam, JpegOutInfo& outInfo, const int& timeout_ms = 100,
               const bool bDump = false, const char* filename = "") noexcept;

/**@brief OpenYuvCap
          初始化YUV功能
 * @return	 0成功，其他失败
 * @notice   必须成功调用本函数才能使用YUV功能。
 *           全局只需执行一次即可，若已经打开过本函数了，将返回0。
 */
int OpenYuvCap() noexcept;

/**@brief CloseYuvCap
          释放YUV资源
 * @return	 0成功，其他失败
 * @notice  调用OpenYuvCap函数后，必须在程序退出前调用本函数释放资源。
 *          若调用ScaleYuv、CropYuv函数失败，建议调用本函数、配合OpenYuvCap恢复YUV功能。
 */
int CloseYuvCap() noexcept;

/**@brief ScaleYuv
          YUV缩放

 * @param [in] srcFrame YUV420SP格式。
 *                      用户预先申请好内存，即必须确保phyAddr、pVirAddr、height、width、stride有效。
 * @param [in] dstWidth 目标宽度
 * @param [in] dstHeight 目标高度
 * @param [out] dstFrame YUV缩放后的数据。
 *                      请用户预先申请好内存，即必须确保phyAddr、pVirAddr有效。

 * @return  0成功，其他失败
 */
int ScaleYuv(const MmzBlockInfo& srcFrame, const uint32_t& dstWidth, const uint32_t& dstHeight,
             MmzBlockInfo& dstFrame) noexcept;

/**@brief CropYuv
          YUV裁剪

 * @param [in] srcFrame YUV420SP格式原始数据。
 *                      用户预先申请好内存，即必须确保phyAddr、pVirAddr、height、width、stride有效。
 * @param [in] cropRect 裁剪矩形框。确保topX+width和topY+height分别小于等于srcFrame的width和height。
 * @param [out] dstFrame YUV缩放后的数据。
 *                      请用户预先申请好内存，即必须确保phyAddr、pVirAddr有效。
 *
 * @return  0成功，其他失败
 */
int CropYuv(const MmzBlockInfo& srcFrame, const Rect& cropRect, MmzBlockInfo& dstFrame) noexcept;

/**@brief CopyYuv
          YUV硬件加速拷贝

 * @param [in] srcFrame YUV420SP格式原始数据。
 *                      用户预先申请好内存，即必须确保phyAddr、pVirAddr、height、width、stride有效。
 * @param [out] dstFrame YUV缩放后的数据。
 *                      请用户预先申请好内存，即必须确保phyAddr、pVirAddr有效。
 *
 * @return  0成功，其他失败
 */
int CopyYuv(const MmzBlockInfo& srcFrame, MmzBlockInfo& dstFrame) noexcept;

/**@brief GetMmzBlock
          申请mmz内存
 * @param [out] mmzBlock  按照结构体内指定的width和height申请对应的mmz内存。
 *                        返回后phyAddr、pVirAddr、stride均被赋予有效值。
 *                        若bCache为true，则申请得到可cache的mmz内存；否则不可cache。
 * @return	 0成功，其他失败
 * @notice  申请成功后，请调用ReleaseMmzBlock函数进行资源释放
 */
int GetMmzBlock(MmzBlockInfo& mmzBlock) noexcept;

/**@brief FlushMmzBlock
          把数据刷回mmz
 * @param [out] mmzBlock  请务必确认phyAddr、pVirAddr、height、width和stride均被赋予有效值。
 * @return	 0成功，其他失败
 * @notice  如果申请的是可cache的mmz，建议在重要操作之前调用本函数。
 */
int FlushMmzBlock(MmzBlockInfo& mmzBlock) noexcept;

/**@brief ReleaseMmzBlock
          释放mmz内存
 * @param [out] mmzBlock  确保phyAddr、pVirAddr为有效值。
 * @return	 0成功，其他失败
 */
int ReleaseMmzBlock(MmzBlockInfo& mmzBlock) noexcept;

/**@brief MmzpVirAddr2MmzPhyAddr
          释放mmz内存
 * @param [in|out] mmzBlock  确保pVirAddr为有效值。
 * @return	 0成功，其他失败
 * @notice  若返回成功，则mmzBlock内部的phyAddr、bCache将被赋予有效值。
 */
int MmzpVirAddr2MmzPhyAddr(MmzBlockInfo& mmzBlock) noexcept;

/**@brief MmzpVirAddr2MmzPhyAddr
          释放mmz内存
 * @param [in|out] mmzBlock  确保pVirAddr为有效值。
 * @return	 0成功，其他失败
 * @notice  若返回成功，则mmzBlock内部的phyAddr、bCache将被赋予有效值。
 */
int MmzPhyAddr2MmzpVirAddr(MmzBlockInfo& mmzBlock) noexcept;

/**@brief DeinitSys
          释放资源
 * @return	 0成功，其他失败
 * @notice   在退出程序前请确保该函数正确被调用。
 */
int DeinitSys() noexcept;

/**@brief ReadYuvFromFile
          读取YUV420SP格式文件
 * @param [in] filename  文件名。
 * @param [out] frame  用户预先申请好内存，即必须确保pVirAddr、height、width、stride有效。
 *
 * @return	 0成功，其他失败
 * @notice  必须确保yuv文件的长宽和mmz的长宽一致
 */
int ReadYuvFromFile(const char* filename, MmzBlockInfo& frame) noexcept;

/**@brief DumpYuvToFile
          保存YUV420SP格式数据到文件
 * @param [in] filename  文件名。
 * @param [out] frame  用户预先申请好内存，即必须确保pVirAddr、height、width、stride有效。
 *
 * @return	 0成功，其他失败
 */
int DumpYuvToFile(const char* filename, const MmzBlockInfo& frame) noexcept;

/**@brief DumpYuvToFile
          保存JPEG格式数据到文件
 * @param [in] filename  文件名。
 * @param [out] frame  用户预先申请好内存，即必须确保pVirAddr有效。
 *
 * @return	 0成功，其他失败
 */
int DumpJpeg(const char* filename, const JpegOutInfo& jpeg) noexcept;

}  // namespace rapidcv_mpp
#endif  // YT_HISI_RAPIDCV_MPP_H_