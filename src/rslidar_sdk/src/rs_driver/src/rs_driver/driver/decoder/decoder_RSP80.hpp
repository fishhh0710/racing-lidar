/*********************************************************************************************************************
Copyright (c) 2020 RoboSense
All rights reserved

By downloading, copying, installing or using the software you agree to this license. If you do not agree to this
license, do not download, install, copy or use the software.

License Agreement
For RoboSense LiDAR SDK Library
(3-clause BSD License)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the names of the RoboSense, nor Suteng Innovation Technology, nor the names of other contributors may be used
to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************************************************/

#pragma once

#include <rs_driver/driver/decoder/decoder_mech.hpp>

namespace robosense
{
namespace lidar
{
#pragma pack(push, 1)
typedef struct
{
  uint8_t id[1];
  uint8_t ret_id;
  uint16_t azimuth;
  RSChannel channels[80];
} RSP80MsopBlock;

typedef struct
{
  RSMsopHeaderV2 header;
  RSP80MsopBlock blocks[4];
  uint8_t reserved[192];
} RSP80MsopPkt;

typedef struct
{
  uint8_t id[8];
  uint16_t rpm;
  RSEthNetV2 eth;
  RSFOV fov;
  uint8_t reserved1[2];
  uint16_t phase_lock_angle;
  RSVersionV2 version;
  uint8_t reserved2[229];
  RSSN sn;
  uint16_t zero_cali;
  uint8_t return_mode;
  RSTimeInfo time_info;
  RSStatusV1 status;
  uint8_t reserved3[5];
  RSDiagnoV1 diagno;
  uint8_t gprmc[86];
  RSCalibrationAngle vert_angle_cali[128];
  RSCalibrationAngle horiz_angle_cali[128];
  uint8_t reserved4[10];
  uint16_t tail;
} RSP80DifopPkt;

#pragma pack(pop)

template <typename T_PointCloud>
class DecoderRSP80 : public DecoderMech<T_PointCloud>
{
public:
  virtual void decodeDifopPkt(const uint8_t* pkt, size_t size);
  virtual bool decodeMsopPkt(const uint8_t* pkt, size_t size);
  virtual ~DecoderRSP80() = default;

  explicit DecoderRSP80(const RSDecoderParam& param);

#ifndef UNIT_TEST
protected:
#endif

  static RSDecoderMechConstParam& getConstParam();
  static RSEchoMode getEchoMode(uint8_t mode);

  void calcParam();
  template <typename T_BlockIterator>
  bool internDecodeMsopPkt(const uint8_t* pkt, size_t size);

  uint8_t lidar_model_;
};

template <typename T_PointCloud>
inline RSDecoderMechConstParam& DecoderRSP80<T_PointCloud>::getConstParam()
{
  static RSDecoderMechConstParam param = 
  {
    1248 // msop len
      , 1248 // difop len
      , 4 // msop id len
      , 8 // difop id len
      , {0x55, 0xAA, 0x05, 0x5A} // msop id
    , {0xA5, 0xFF, 0x00, 0x5A, 0x11, 0x11, 0x55, 0x55} // difop id
    , {0xFE} // block id
    , 80 // laser number
    , 4 // blocks per packet
      , 80 // channels per block
      , 0.4f // distance min
      , 250.0f // distance max
      , 0.005f // distance resolution
      , 0.0625f // temperature resolution

      // lens center
      , 0.02892f // RX
      , -0.013f // RY
      , 0.0f // RZ
  };

  float blk_ts = 55.56f;
  param.BLOCK_DURATION = blk_ts / 1000000;

  return param;
}

template <typename T_PointCloud>
inline void DecoderRSP80<T_PointCloud>::calcParam()
{
  float blk_ts = 55.56f;

  float firing_tss_80[80] = 
  {
    0.0f,    0.0f,    0.0f,    0.0f,    1.217f,  1.217f,  1.217f,  1.217f, 
    2.434f,  2.434f,  3.652f,  3.652f,  3.652f,  4.869f,  4.869f,  6.086f,  
    6.086f,  7.304f,  7.304f,  8.521f,  8.521f,  9.739f,  9.739f,  11.323f, 
    11.323f, 12.907f, 12.907f, 14.924f, 14.924f, 16.941f, 16.941f, 16.941f, 

    16.941f, 18.959f, 18.959f, 18.959f, 18.959f, 20.976f, 20.976f, 20.976f, 
    20.976f, 23.127f, 23.127f, 23.127f, 23.127f, 25.278f, 25.278f, 25.278f, 
    25.278f, 27.428f, 27.428f, 27.428f, 27.428f, 29.579f, 29.579f, 29.579f, 
    29.579f, 31.963f, 31.963f, 31.963f, 31.963f, 34.347f, 34.347f, 34.347f, 

    34.347f, 36.498f, 36.498f, 36.498f, 36.498f, 38.648f, 38.648f, 40.666f, 
    40.666f, 42.683f, 50.603f, 52.187f, 52.187f, 52.187f, 53.771f, 53.771f,
  };

  float firing_tss_80v[80] = 
  {
    0.0f,    0.0f,    0.0f,    0.0f,    1.217f,  1.217f,  1.217f,  1.217f, 
    2.434f,  2.434f,  2.434f,  2.434f,  3.652f,  3.652f,  3.652f,  3.652f,
    4.869f,  4.869f,  4.869f,  4.869f,  6.086f,  6.086f,  6.086f,  6.086f, 
    7.304f,  7.304f,  7.304f,  7.304f,  8.521f,  8.521f,  8.521f,  8.521f,

    9.739f,  9.739f,  9.739f,  9.739f, 11.323f, 11.323f, 11.323f, 11.323f,
    12.907f, 12.907f, 12.907f, 12.907f, 14.924f, 14.924f, 14.924f, 14.924f, 
    16.941f, 16.941f, 16.941f, 16.941f, 18.959f, 18.959f, 18.959f, 18.959f, 
    20.976f, 20.976f, 20.976f, 20.976f, 23.127f, 23.127f, 23.127f, 23.127f, 

    25.278f, 25.278f, 25.278f, 25.278f, 27.428f, 27.428f, 27.428f, 27.428f, 
    29.579f, 29.579f, 29.579f, 29.579f, 31.963f, 31.963f, 31.963f, 31.963f, 
  };

  float* firing_tss = firing_tss_80; // 80  - lidar_model = 0x02
  if (lidar_model_ == 0x03)          // 80v - lidar_model = 0x03
  {
    firing_tss = firing_tss_80v;
  }

  RSDecoderMechConstParam& param = this->mech_const_param_;
  for (uint16_t i = 0; i < 80; i++)
  {
    param.CHAN_TSS[i] = (double)firing_tss[i] / 1000000;
    param.CHAN_AZIS[i] = firing_tss[i] / blk_ts;
  }
}

template <typename T_PointCloud>
inline RSEchoMode DecoderRSP80<T_PointCloud>::getEchoMode(uint8_t mode)
{
  switch (mode)
  {
    case 0x00:
    case 0x01:
    case 0x02:
      return RSEchoMode::ECHO_SINGLE;
    case 0x03:
    case 0x04:
    case 0x05:
    default:
      return RSEchoMode::ECHO_DUAL;
  }
}

template <typename T_PointCloud>
inline void DecoderRSP80<T_PointCloud>::decodeDifopPkt(const uint8_t* packet, size_t size)
{
  const RSP80DifopPkt& pkt = *(const RSP80DifopPkt*)(packet);
  this->template decodeDifopCommon<RSP80DifopPkt>(pkt);

  this->echo_mode_ = getEchoMode (pkt.return_mode);
  this->split_blks_per_frame_ = (this->echo_mode_ == RSEchoMode::ECHO_DUAL) ? 
    (this->blks_per_frame_ << 1) : this->blks_per_frame_;
}

template <typename T_PointCloud>
inline DecoderRSP80<T_PointCloud>::DecoderRSP80(const RSDecoderParam& param)
  : DecoderMech<T_PointCloud>(getConstParam(), param), 
    lidar_model_(0)
{
}

template <typename T_PointCloud>
inline bool DecoderRSP80<T_PointCloud>::decodeMsopPkt(const uint8_t* pkt, size_t size)
{
  if (this->echo_mode_ == RSEchoMode::ECHO_SINGLE)
  {
    return internDecodeMsopPkt<SingleReturnBlockIterator<RSP80MsopPkt>>(pkt, size);
  }
  else
  {
    return internDecodeMsopPkt<DualReturnBlockIterator<RSP80MsopPkt>>(pkt, size);
  }
}

template <typename T_PointCloud>
template <typename T_BlockIterator>
inline bool DecoderRSP80<T_PointCloud>::internDecodeMsopPkt(const uint8_t* packet, size_t size)
{
  const RSP80MsopPkt& pkt = *(const RSP80MsopPkt*)(packet);
  bool ret = false;

  uint8_t lidar_model = pkt.header.lidar_model;
  if (lidar_model_ != lidar_model)
  {
    lidar_model_ = lidar_model;
    calcParam();
  }

  this->temperature_ = parseTempInBe(&(pkt.header.temp)) * this->const_param_.TEMPERATURE_RES;
  this->is_get_temperature_ = true;
  double pkt_ts = 0;
  if (this->param_.use_lidar_clock)
  {
    pkt_ts = parseTimeUTCWithUs(&pkt.header.timestamp) * 1e-6;
  }
  else
  {
    uint64_t ts = getTimeHost();

    // roll back to first block to approach lidar ts as near as possible.
    pkt_ts = ts * 1e-6 - this->getPacketDuration();

    if (this->write_pkt_ts_)
    {
      createTimeUTCWithUs (ts, (RSTimestampUTC*)&pkt.header.timestamp);
    }
  }

  T_BlockIterator iter(pkt, this->const_param_.BLOCKS_PER_PKT, this->mech_const_param_.BLOCK_DURATION,
      this->block_az_diff_, this->fov_blind_ts_diff_);

  for (uint16_t blk = 0; blk < this->const_param_.BLOCKS_PER_PKT; blk++)
  {
    const RSP80MsopBlock& block = pkt.blocks[blk];

    if (memcmp(this->const_param_.BLOCK_ID, block.id, 1) != 0)
    {
      this->cb_excep_(Error(ERRCODE_WRONGMSOPBLKID));
      break;
    }

    int32_t block_az_diff;
    double block_ts_off;
    iter.get(blk, block_az_diff, block_ts_off);

    double block_ts = pkt_ts + block_ts_off;
    int32_t block_az = ntohs(block.azimuth);
    if (this->split_strategy_->newBlock(block_az))
    {
      this->cb_split_frame_(this->const_param_.LASER_NUM, this->cloudTs());
      this->first_point_ts_ = block_ts;
      ret = true;
    }

    for (uint16_t chan = 0; chan < this->const_param_.CHANNELS_PER_BLOCK; chan++)
    {
      const RSChannel& channel = block.channels[chan]; 

      double chan_ts = block_ts + this->mech_const_param_.CHAN_TSS[chan];
      int32_t angle_horiz = block_az + 
        (int32_t)((float)block_az_diff * this->mech_const_param_.CHAN_AZIS[chan]);

      int32_t angle_vert = this->chan_angles_.vertAdjust(chan);
      int32_t angle_horiz_final = this->chan_angles_.horizAdjust(chan, angle_horiz);
      float distance = ntohs(channel.distance) * this->const_param_.DISTANCE_RES;

      if (this->distance_section_.in(distance) && this->scan_section_.in(angle_horiz_final))
      {
        float x =  distance * COS(angle_vert) * COS(angle_horiz_final) + this->mech_const_param_.RX * COS(angle_horiz);
        float y = -distance * COS(angle_vert) * SIN(angle_horiz_final) - this->mech_const_param_.RX * SIN(angle_horiz);
        float z =  distance * SIN(angle_vert) + this->mech_const_param_.RZ;
        this->transformPoint(x, y, z);

        typename T_PointCloud::PointT point;
        setX(point, x);
        setY(point, y);
        setZ(point, z);
        setIntensity(point, channel.intensity);
        setTimestamp(point, chan_ts);
        setRing(point, this->chan_angles_.toUserChan(chan));

        this->point_cloud_->points.emplace_back(point);
      }
      else if (!this->param_.dense_points)
      {
        typename T_PointCloud::PointT point;
        setX(point, NAN);
        setY(point, NAN);
        setZ(point, NAN);
        setIntensity(point, 0);
        setTimestamp(point, chan_ts);
        setRing(point, this->chan_angles_.toUserChan(chan));

        this->point_cloud_->points.emplace_back(point);
      }

      this->prev_point_ts_ = chan_ts;
    }
  }

  this->prev_pkt_ts_ = pkt_ts;
  return ret;
}

}  // namespace lidar
}  // namespace robosense
