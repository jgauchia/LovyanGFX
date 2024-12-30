/*----------------------------------------------------------------------------/
 *  Lovyan GFX - Graphics library for embedded devices.
 *
 * Original Source:
 * https://github.com/lovyan03/LovyanGFX/
 *
 * Licence:
 * [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)
 *
 * Author:
 * [lovyan03](https://twitter.com/lovyan03)
 *
 * Contributors:
 * [ciniml](https://github.com/ciniml)
 * [mongonta0716](https://github.com/mongonta0716)
 * [tobozo](https://github.com/tobozo)
 * /----------------------------------------------------------------------------*/
#include "Panel_RM690B0.hpp"

#include "../Bus.hpp"
#include "../misc/colortype.hpp"
#include "../misc/pixelcopy.hpp"
#include "../platforms/common.hpp"
#include "driver/spi_master.h"
#include "esp_log.h"

/**
 * @brief Bug list
 *
 *  > Write image (pushSprite) works fine, bugs down below are from writing
 * directly
 *
 *  1> Write function is block even with DMA (manual CS wait data)
 *  2> In spi 40MHz draw vertical line incomplete, but 10MHz OK (Likely because
 * my dupont line connection) 3> After implement write/draw pixel funcs,
 * "testFilledRects" stucks sometime, acts differently to the different sck freq
 *  4> Haven't find the way to set rotation by reg
 */

namespace lgfx {
inline namespace v1 {
//----------------------------------------------------------------------------

/* Panel init */
bool Panel_RM690B0::init(bool use_reset) {
  // ESP_LOGD("SH8601Z","pannel init %d", use_reset);

  if (!Panel_Device::init(use_reset)) {
    return false;
  }

  /* Store pannel resolution */
  _width = _cfg.panel_width;
  _height = _cfg.panel_height;

  struct writereg_t {
    uint16_t cmd;
    uint8_t data;
  };
  static constexpr const writereg_t regs[] = {
      {0xF000, 0x55},
      {0xF001, 0xAA},
      {0xF002, 0x52},
      {0xF003, 0x08},
      {0xF004, 0x01},
      // GAMMA SETING  RED
      {0xD100, 0x00},
      {0xD101, 0x00},
      {0xD102, 0x1b},
      {0xD103, 0x44},
      {0xD104, 0x62},
      {0xD105, 0x00},
      {0xD106, 0x7b},
      {0xD107, 0xa1},
      {0xD108, 0xc0},
      {0xD109, 0xee},
      {0xD10A, 0x55},
      {0xD10B, 0x10},
      {0xD10C, 0x2c},
      {0xD10D, 0x43},
      {0xD10E, 0x57},
      {0xD10F, 0x55},
      {0xD110, 0x68},
      {0xD111, 0x78},
      {0xD112, 0x87},
      {0xD113, 0x94},
      {0xD114, 0x55},
      {0xD115, 0xa0},
      {0xD116, 0xac},
      {0xD117, 0xb6},
      {0xD118, 0xc1},
      {0xD119, 0x55},
      {0xD11A, 0xcb},
      {0xD11B, 0xcd},
      {0xD11C, 0xd6},
      {0xD11D, 0xdf},
      {0xD11E, 0x95},
      {0xD11F, 0xe8},
      {0xD120, 0xf1},
      {0xD121, 0xfa},
      {0xD122, 0x02},
      {0xD123, 0xaa},
      {0xD124, 0x0b},
      {0xD125, 0x13},
      {0xD126, 0x1d},
      {0xD127, 0x26},
      {0xD128, 0xaa},
      {0xD129, 0x30},
      {0xD12A, 0x3c},
      {0xD12B, 0x4A},
      {0xD12C, 0x63},
      {0xD12D, 0xea},
      {0xD12E, 0x79},
      {0xD12F, 0xa6},
      {0xD130, 0xd0},
      {0xD131, 0x20},
      {0xD132, 0x0f},
      {0xD133, 0x8e},
      {0xD134, 0xff},
      // GAMMA SETING GREEN
      {0xD200, 0x00},
      {0xD201, 0x00},
      {0xD202, 0x1b},
      {0xD203, 0x44},
      {0xD204, 0x62},
      {0xD205, 0x00},
      {0xD206, 0x7b},
      {0xD207, 0xa1},
      {0xD208, 0xc0},
      {0xD209, 0xee},
      {0xD20A, 0x55},
      {0xD20B, 0x10},
      {0xD20C, 0x2c},
      {0xD20D, 0x43},
      {0xD20E, 0x57},
      {0xD20F, 0x55},
      {0xD210, 0x68},
      {0xD211, 0x78},
      {0xD212, 0x87},
      {0xD213, 0x94},
      {0xD214, 0x55},
      {0xD215, 0xa0},
      {0xD216, 0xac},
      {0xD217, 0xb6},
      {0xD218, 0xc1},
      {0xD219, 0x55},
      {0xD21A, 0xcb},
      {0xD21B, 0xcd},
      {0xD21C, 0xd6},
      {0xD21D, 0xdf},
      {0xD21E, 0x95},
      {0xD21F, 0xe8},
      {0xD220, 0xf1},
      {0xD221, 0xfa},
      {0xD222, 0x02},
      {0xD223, 0xaa},
      {0xD224, 0x0b},
      {0xD225, 0x13},
      {0xD226, 0x1d},
      {0xD227, 0x26},
      {0xD228, 0xaa},
      {0xD229, 0x30},
      {0xD22A, 0x3c},
      {0xD22B, 0x4a},
      {0xD22C, 0x63},
      {0xD22D, 0xea},
      {0xD22E, 0x79},
      {0xD22F, 0xa6},
      {0xD230, 0xd0},
      {0xD231, 0x20},
      {0xD232, 0x0f},
      {0xD233, 0x8e},
      {0xD234, 0xff},
      // GAMMA SETING BLUE
      {0xD300, 0x00},
      {0xD301, 0x00},
      {0xD302, 0x1b},
      {0xD303, 0x44},
      {0xD304, 0x62},
      {0xD305, 0x00},
      {0xD306, 0x7b},
      {0xD307, 0xa1},
      {0xD308, 0xc0},
      {0xD309, 0xee},
      {0xD30A, 0x55},
      {0xD30B, 0x10},
      {0xD30C, 0x2c},
      {0xD30D, 0x43},
      {0xD30E, 0x57},
      {0xD30F, 0x55},
      {0xD310, 0x68},
      {0xD311, 0x78},
      {0xD312, 0x87},
      {0xD313, 0x94},
      {0xD314, 0x55},
      {0xD315, 0xa0},
      {0xD316, 0xac},
      {0xD317, 0xb6},
      {0xD318, 0xc1},
      {0xD319, 0x55},
      {0xD31A, 0xcb},
      {0xD31B, 0xcd},
      {0xD31C, 0xd6},
      {0xD31D, 0xdf},
      {0xD31E, 0x95},
      {0xD31F, 0xe8},
      {0xD320, 0xf1},
      {0xD321, 0xfa},
      {0xD322, 0x02},
      {0xD323, 0xaa},
      {0xD324, 0x0b},
      {0xD325, 0x13},
      {0xD326, 0x1d},
      {0xD327, 0x26},
      {0xD328, 0xaa},
      {0xD329, 0x30},
      {0xD32A, 0x3c},
      {0xD32B, 0x4A},
      {0xD32C, 0x63},
      {0xD32D, 0xea},
      {0xD32E, 0x79},
      {0xD32F, 0xa6},
      {0xD330, 0xd0},
      {0xD331, 0x20},
      {0xD332, 0x0f},
      {0xD333, 0x8e},
      {0xD334, 0xff},
      // GAMMA SETING  RED
      {0xD400, 0x00},
      {0xD401, 0x00},
      {0xD402, 0x1b},
      {0xD403, 0x44},
      {0xD404, 0x62},
      {0xD405, 0x00},
      {0xD406, 0x7b},
      {0xD407, 0xa1},
      {0xD408, 0xc0},
      {0xD409, 0xee},
      {0xD40A, 0x55},
      {0xD40B, 0x10},
      {0xD40C, 0x2c},
      {0xD40D, 0x43},
      {0xD40E, 0x57},
      {0xD40F, 0x55},
      {0xD410, 0x68},
      {0xD411, 0x78},
      {0xD412, 0x87},
      {0xD413, 0x94},
      {0xD414, 0x55},
      {0xD415, 0xa0},
      {0xD416, 0xac},
      {0xD417, 0xb6},
      {0xD418, 0xc1},
      {0xD419, 0x55},
      {0xD41A, 0xcb},
      {0xD41B, 0xcd},
      {0xD41C, 0xd6},
      {0xD41D, 0xdf},
      {0xD41E, 0x95},
      {0xD41F, 0xe8},
      {0xD420, 0xf1},
      {0xD421, 0xfa},
      {0xD422, 0x02},
      {0xD423, 0xaa},
      {0xD424, 0x0b},
      {0xD425, 0x13},
      {0xD426, 0x1d},
      {0xD427, 0x26},
      {0xD428, 0xaa},
      {0xD429, 0x30},
      {0xD42A, 0x3c},
      {0xD42B, 0x4A},
      {0xD42C, 0x63},
      {0xD42D, 0xea},
      {0xD42E, 0x79},
      {0xD42F, 0xa6},
      {0xD430, 0xd0},
      {0xD431, 0x20},
      {0xD432, 0x0f},
      {0xD433, 0x8e},
      {0xD434, 0xff},
      // GAMMA SETING GREEN
      {0xD500, 0x00},
      {0xD501, 0x00},
      {0xD502, 0x1b},
      {0xD503, 0x44},
      {0xD504, 0x62},
      {0xD505, 0x00},
      {0xD506, 0x7b},
      {0xD507, 0xa1},
      {0xD508, 0xc0},
      {0xD509, 0xee},
      {0xD50A, 0x55},
      {0xD50B, 0x10},
      {0xD50C, 0x2c},
      {0xD50D, 0x43},
      {0xD50E, 0x57},
      {0xD50F, 0x55},
      {0xD510, 0x68},
      {0xD511, 0x78},
      {0xD512, 0x87},
      {0xD513, 0x94},
      {0xD514, 0x55},
      {0xD515, 0xa0},
      {0xD516, 0xac},
      {0xD517, 0xb6},
      {0xD518, 0xc1},
      {0xD519, 0x55},
      {0xD51A, 0xcb},
      {0xD51B, 0xcd},
      {0xD51C, 0xd6},
      {0xD51D, 0xdf},
      {0xD51E, 0x95},
      {0xD51F, 0xe8},
      {0xD520, 0xf1},
      {0xD521, 0xfa},
      {0xD522, 0x02},
      {0xD523, 0xaa},
      {0xD524, 0x0b},
      {0xD525, 0x13},
      {0xD526, 0x1d},
      {0xD527, 0x26},
      {0xD528, 0xaa},
      {0xD529, 0x30},
      {0xD52A, 0x3c},
      {0xD52B, 0x4a},
      {0xD52C, 0x63},
      {0xD52D, 0xea},
      {0xD52E, 0x79},
      {0xD52F, 0xa6},
      {0xD530, 0xd0},
      {0xD531, 0x20},
      {0xD532, 0x0f},
      {0xD533, 0x8e},
      {0xD534, 0xff},
      // GAMMA SETING BLUE
      {0xD600, 0x00},
      {0xD601, 0x00},
      {0xD602, 0x1b},
      {0xD603, 0x44},
      {0xD604, 0x62},
      {0xD605, 0x00},
      {0xD606, 0x7b},
      {0xD607, 0xa1},
      {0xD608, 0xc0},
      {0xD609, 0xee},
      {0xD60A, 0x55},
      {0xD60B, 0x10},
      {0xD60C, 0x2c},
      {0xD60D, 0x43},
      {0xD60E, 0x57},
      {0xD60F, 0x55},
      {0xD610, 0x68},
      {0xD611, 0x78},
      {0xD612, 0x87},
      {0xD613, 0x94},
      {0xD614, 0x55},
      {0xD615, 0xa0},
      {0xD616, 0xac},
      {0xD617, 0xb6},
      {0xD618, 0xc1},
      {0xD619, 0x55},
      {0xD61A, 0xcb},
      {0xD61B, 0xcd},
      {0xD61C, 0xd6},
      {0xD61D, 0xdf},
      {0xD61E, 0x95},
      {0xD61F, 0xe8},
      {0xD620, 0xf1},
      {0xD621, 0xfa},
      {0xD622, 0x02},
      {0xD623, 0xaa},
      {0xD624, 0x0b},
      {0xD625, 0x13},
      {0xD626, 0x1d},
      {0xD627, 0x26},
      {0xD628, 0xaa},
      {0xD629, 0x30},
      {0xD62A, 0x3c},
      {0xD62B, 0x4A},
      {0xD62C, 0x63},
      {0xD62D, 0xea},
      {0xD62E, 0x79},
      {0xD62F, 0xa6},
      {0xD630, 0xd0},
      {0xD631, 0x20},
      {0xD632, 0x0f},
      {0xD633, 0x8e},
      {0xD634, 0xff},

      // AVDD VOLTAGE SETTING
      {0xB000, 0x05},
      {0xB001, 0x05},
      {0xB002, 0x05},
      // AVEE VOLTAGE SETTING
      {0xB100, 0x05},
      {0xB101, 0x05},
      {0xB102, 0x05},

      // AVDD Boosting
      {0xB600, 0x34},
      {0xB601, 0x34},
      {0xB603, 0x34},
      // AVEE Boosting
      {0xB700, 0x24},
      {0xB701, 0x24},
      {0xB702, 0x24},
      // VCL Boosting
      {0xB800, 0x24},
      {0xB801, 0x24},
      {0xB802, 0x24},
      // VGLX VOLTAGE SETTING
      {0xBA00, 0x14},
      {0xBA01, 0x14},
      {0xBA02, 0x14},
      // VCL Boosting
      {0xB900, 0x24},
      {0xB901, 0x24},
      {0xB902, 0x24},
      // Gamma Voltage
      {0xBc00, 0x00},
      {0xBc01, 0xa0},  // vgmp=5.0
      {0xBc02, 0x00},
      {0xBd00, 0x00},
      {0xBd01, 0xa0},  // vgmn=5.0
      {0xBd02, 0x00},
      // VCOM Setting
      {0xBe01, 0x3d},  // 3
      // ENABLE PAGE 0
      {0xF000, 0x55},
      {0xF001, 0xAA},
      {0xF002, 0x52},
      {0xF003, 0x08},
      {0xF004, 0x00},
      // Vivid Color Function Control
      {0xB400, 0x10},
      // Z-INVERSION
      {0xBC00, 0x05},
      {0xBC01, 0x05},
      {0xBC02, 0x05},

      //*************** add on 20111021**********************//
      {0xB700, 0x22},  // GATE EQ CONTROL
      {0xB701, 0x22},  // GATE EQ CONTROL
      {0xC80B, 0x2A},  // DISPLAY TIMING CONTROL
      {0xC80C, 0x2A},  // DISPLAY TIMING CONTROL
      {0xC80F, 0x2A},  // DISPLAY TIMING CONTROL
      {0xC810, 0x2A},  // DISPLAY TIMING CONTROL
      //*************** add on 20111021**********************//
      // PWM_ENH_OE =1
      {0xd000, 0x01},
      // DM_SEL =1
      {0xb300, 0x10},
      // VBPDA=07h
      {0xBd02, 0x07},
      // VBPDb=07h
      {0xBe02, 0x07},
      // VBPDc=07h
      {0xBf02, 0x07},
      // ENABLE PAGE 2
      {0xF000, 0x55},
      {0xF001, 0xAA},
      {0xF002, 0x52},
      {0xF003, 0x08},
      {0xF004, 0x02},
      // SDREG0 =0
      {0xc301, 0xa9},
      // DS=14
      {0xfe01, 0x94},
      // OSC =60h
      {0xf600, 0x60},
      // TE ON
      {0x3500, 0x00},
      {0xFFFF, 0xFF},
  };

  startWrite();
  for (size_t idx = 0; regs[idx].cmd != 0xFFFF; ++idx) {
    writeRegister(regs[idx].cmd, regs[idx].data);
  }

  return true;
}

void Panel_RM690B0::setBrightness(uint8_t brightness) {
  // ESP_LOGD("SH8601Z","setBrightness %d", brightness);

  startWrite();

  /* Write Display Brightness	MAX_VAL=0XFF */
  cs_control(false);
  write_cmd(0x51);
  _bus->writeCommand(brightness, 8);
  _bus->wait();
  cs_control(true);

  endWrite();
}

void Panel_RM690B0::setRotation(uint_fast8_t r) {
  // ESP_LOGD("SH8601Z","setRotation %d", r);

  r &= 7;
  _rotation = r;
  // offset_rotationを加算 (0~3:回転方向、 4:上下反転フラグ);
  _internal_rotation =
      ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

  auto ox = _cfg.offset_x;
  auto oy = _cfg.offset_y;
  auto pw = _cfg.panel_width;
  auto ph = _cfg.panel_height;
  auto mw = _cfg.memory_width;
  auto mh = _cfg.memory_height;
  if (_internal_rotation & 1) {
    std::swap(ox, oy);
    std::swap(pw, ph);
    std::swap(mw, mh);
  }
  _width = pw;
  _height = ph;
  // _colstart = (_internal_rotation & 2)
  //         ? mw - (pw + ox) : ox;

  // _rowstart = ((1 << _internal_rotation) & 0b10010110) // case 1:2:4:7
  //         ? mh - (ph + oy) : oy;

  _xs = _xe = _ys = _ye = INT16_MAX;

  // update_madctl();
}

void Panel_RM690B0::setInvert(bool invert) {
  // ESP_LOGD("SH8601Z","setInvert %d", invert);

  cs_control(false);

  if (invert) {
    /* Inversion On */
    write_cmd(0x21);
  } else {
    /* Inversion Off */
    write_cmd(0x20);
  }
  _bus->wait();

  cs_control(true);
}

void Panel_RM690B0::setSleep(bool flg) {
  // ESP_LOGD("SH8601Z","setSleep %d", flg);

  cs_control(false);

  if (flg) {
    /* Sleep in */
    write_cmd(0x10);
  } else {
    /* Sleep out */
    write_cmd(0x11);
    delay(150);
  }
  _bus->wait();

  cs_control(true);
}

void Panel_RM690B0::setPowerSave(bool flg) {
  // ESP_LOGD("SH8601Z","setPowerSave");
}

void Panel_RM690B0::waitDisplay(void) {
  // ESP_LOGD("SH8601Z","waitDisplay");
}

bool Panel_RM690B0::displayBusy(void) {
  // ESP_LOGD("SH8601Z","displayBusy");
  return false;
}

color_depth_t Panel_RM690B0::setColorDepth(color_depth_t depth) {
  // ESP_LOGD("SH8601Z","setColorDepth %d", depth);

  /* 0x55: 16bit/pixel */
  /* 0x66: 18bit/pixel */
  /* 0x77: 24bit/pixel */
  uint8_t cmd_send = 0;
  if (depth == rgb565_2Byte) {
    cmd_send = 0x55;
  } else if (depth == rgb666_3Byte) {
    cmd_send = 0x66;
  } else if (depth == rgb888_3Byte) {
    cmd_send = 0x77;
  } else {
    return _write_depth;
  }
  _write_depth = depth;

  /* Set interface Pixel Format */
  startWrite();

  cs_control(false);
  write_cmd(0x3A);
  _bus->writeCommand(cmd_send, 8);
  _bus->wait();
  cs_control(true);

  endWrite();

  return _write_depth;
}

void Panel_RM690B0::write_cmd(uint8_t cmd) {
  uint8_t cmd_buffer[4] = {0x02, 0x00, 0x00, 0x00};
  cmd_buffer[2] = cmd;
  // _bus->writeBytes(cmd_buffer, 4, 0, false);
  for (int i = 0; i < 4; i++) {
    _bus->writeCommand(cmd_buffer[i], 8);
  }
}

void Panel_RM690B0::start_qspi() {
  /* Begin QSPI */
  cs_control(false);
  _bus->writeCommand(0x32, 8);
  _bus->writeCommand(0x00, 8);
  _bus->writeCommand(0x2C, 8);
  _bus->writeCommand(0x00, 8);
  _bus->wait();
}

void Panel_RM690B0::end_qspi() {
  /* Stop QSPI */
  _bus->writeCommand(0x32, 8);
  _bus->writeCommand(0x00, 8);
  _bus->writeCommand(0x00, 8);
  _bus->writeCommand(0x00, 8);
  _bus->wait();
  cs_control(true);
}

void Panel_RM690B0::beginTransaction(void) {
  // ESP_LOGD("SH8601Z","beginTransaction");
  if (_in_transaction) return;
  _in_transaction = true;
  _bus->beginTransaction();
}

void Panel_RM690B0::endTransaction(void) {
  // ESP_LOGD("SH8601Z","endTransaction");
  // if (!_in_transaction) return;
  // _in_transaction = false;
  // _bus->endTransaction();

  if (!_in_transaction) return;
  _in_transaction = false;

  if (_has_align_data) {
    _has_align_data = false;
    _bus->writeData(0, 8);
  }

  _bus->endTransaction();
}

void Panel_RM690B0::write_bytes(const uint8_t* data, uint32_t len,
                                bool use_dma) {
  start_qspi();
  _bus->writeBytes(data, len, true, use_dma);
  _bus->wait();
  end_qspi();
}

void Panel_RM690B0::setWindow(uint_fast16_t xs, uint_fast16_t ys,
                              uint_fast16_t xe, uint_fast16_t ye) {
  // ESP_LOGD("SH8601Z","setWindow %d %d %d %d", xs, ys, xe, ye);

  /* Set limit */
  if ((xe - xs) >= _width) {
    xs = 0;
    xe = _width - 1;
  }
  if ((ye - ys) >= _height) {
    ys = 0;
    ye = _height - 1;
  }

  /* Set Column Start Address */
  cs_control(false);
  write_cmd(0x2A);
  _bus->writeCommand(xs >> 8, 8);
  _bus->writeCommand(xs & 0xFF, 8);
  _bus->writeCommand(xe >> 8, 8);
  _bus->writeCommand(xe & 0xFF, 8);
  _bus->wait();
  cs_control(true);

  /* Set Row Start Address */
  cs_control(false);
  write_cmd(0x2B);
  _bus->writeCommand(ys >> 8, 8);
  _bus->writeCommand(ys & 0xFF, 8);
  _bus->writeCommand(ye >> 8, 8);
  _bus->writeCommand(ye & 0xFF, 8);
  _bus->wait();
  cs_control(true);

  /* Memory Write */
  cs_control(false);
  write_cmd(0x2C);
  _bus->wait();
  cs_control(true);
}

void Panel_RM690B0::writeBlock(uint32_t rawcolor, uint32_t len) {
  // ESP_LOGD("SH8601Z","writeBlock 0x%lx %ld", rawcolor, len);

  /* Push color */
  start_qspi();
  _bus->writeDataRepeat(rawcolor, _write_bits, len);
  _bus->wait();
  end_qspi();
}

void Panel_RM690B0::writePixels(pixelcopy_t* param, uint32_t len,
                                bool use_dma) {
  // ESP_LOGD("SH8601Z","writePixels %ld %d", len, use_dma);

  start_qspi();

  if (param->no_convert) {
    _bus->writeBytes(reinterpret_cast<const uint8_t*>(param->src_data),
                     len * _write_bits >> 3, true, use_dma);
  } else {
    _bus->writePixels(param, len);
  }
  if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1)) {
    _has_align_data = !_has_align_data;
  }

  _bus->wait();
  end_qspi();
}

void Panel_RM690B0::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y,
                                        uint32_t rawcolor) {
  // ESP_LOGD("SH8601Z","drawPixelPreclipped %d %d 0x%lX", x, y, rawcolor);

  setWindow(x, y, x, y);
  if (_cfg.dlen_16bit) {
    _has_align_data = (_write_bits & 15);
  }

  start_qspi();

  _bus->writeData(rawcolor, _write_bits);

  _bus->wait();
  end_qspi();
}

void Panel_RM690B0::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y,
                                            uint_fast16_t w, uint_fast16_t h,
                                            uint32_t rawcolor) {
  // ESP_LOGD("SH8601Z","writeFillRectPreclipped %d %d %d %d 0x%lX", x, y, w, h,
  // rawcolor);

  uint32_t len = w * h;
  uint_fast16_t xe = w + x - 1;
  uint_fast16_t ye = y + h - 1;

  setWindow(x, y, xe, ye);
  // if (_cfg.dlen_16bit) { _has_align_data = (_write_bits & 15) && (len & 1); }

  start_qspi();
  _bus->writeDataRepeat(rawcolor, _write_bits, len);
  _bus->wait();
  end_qspi();
}

void Panel_RM690B0::writeImage(uint_fast16_t x, uint_fast16_t y,
                               uint_fast16_t w, uint_fast16_t h,
                               pixelcopy_t* param, bool use_dma) {
  // ESP_LOGD("SH8601Z","writeImage %d %d %d %d %d", x, y, w, h, use_dma);
  // use_dma = false;

  auto bytes = param->dst_bits >> 3;
  auto src_x = param->src_x;

  if (param->transp == pixelcopy_t::NON_TRANSP) {
    if (param->no_convert) {
      auto wb = w * bytes;
      uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bytes;
      auto src = &((const uint8_t*)param->src_data)[i];
      setWindow(x, y, x + w - 1, y + h - 1);
      if (param->src_bitwidth == w || h == 1) {
        write_bytes(src, wb * h, use_dma);
      } else {
        auto add = param->src_bitwidth * bytes;
        if (use_dma) {
          if (_cfg.dlen_16bit && ((wb * h) & 1)) {
            _has_align_data = !_has_align_data;
          }
          do {
            _bus->addDMAQueue(src, wb);
            src += add;
          } while (--h);
          _bus->execDMAQueue();
        } else {
          do {
            write_bytes(src, wb, false);
            src += add;
          } while (--h);
        }
      }
    } else {
      if (!_bus->busy()) {
        static constexpr uint32_t WRITEPIXELS_MAXLEN = 32767;

        setWindow(x, y, x + w - 1, y + h - 1);
        // bool nogap = (param->src_bitwidth == w || h == 1);
        bool nogap =
            (h == 1) || (param->src_y32_add == 0 &&
                         ((param->src_bitwidth << pixelcopy_t::FP_SCALE) ==
                          (w * param->src_x32_add)));
        if (nogap && (w * h <= WRITEPIXELS_MAXLEN)) {
          writePixels(param, w * h, use_dma);
        } else {
          uint_fast16_t h_step = nogap ? WRITEPIXELS_MAXLEN / w : 1;
          uint_fast16_t h_len = (h_step > 1) ? ((h - 1) % h_step) + 1 : 1;
          writePixels(param, w * h_len, use_dma);
          if (h -= h_len) {
            param->src_y += h_len;
            do {
              param->src_x = src_x;
              writePixels(param, w * h_step, use_dma);
              param->src_y += h_step;
            } while (h -= h_step);
          }
        }
      } else {
        size_t wb = w * bytes;
        auto buf = _bus->getDMABuffer(wb);
        param->fp_copy(buf, 0, w, param);
        setWindow(x, y, x + w - 1, y + h - 1);
        write_bytes(buf, wb, true);
        _has_align_data =
            (_cfg.dlen_16bit && (_write_bits & 15) && (w & h & 1));
        while (--h) {
          param->src_x = src_x;
          param->src_y++;
          buf = _bus->getDMABuffer(wb);
          param->fp_copy(buf, 0, w, param);
          write_bytes(buf, wb, true);
        }
      }
    }
  } else {
    h += y;
    uint32_t wb = w * bytes;
    do {
      uint32_t i = 0;
      while (w != (i = param->fp_skip(i, w, param))) {
        auto buf = _bus->getDMABuffer(wb);
        int32_t len = param->fp_copy(buf, 0, w - i, param);
        setWindow(x + i, y, x + i + len - 1, y);
        write_bytes(buf, len * bytes, true);
        if (w == (i += len)) break;
      }
      param->src_x = src_x;
      param->src_y++;
    } while (++y != h);
  }
}

uint32_t Panel_RM690B0::readCommand(uint_fast16_t cmd, uint_fast8_t index,
                                    uint_fast8_t len) {
  // ESP_LOGD("SH8601Z","readCommand");
  return 0;
}

uint32_t Panel_RM690B0::readData(uint_fast8_t index, uint_fast8_t len) {
  // ESP_LOGD("SH8601Z","readData");
  return 0;
}

void Panel_RM690B0::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w,
                             uint_fast16_t h, void* dst, pixelcopy_t* param) {
  // ESP_LOGD("SH8601Z","readRect");
}

//----------------------------------------------------------------------------
}  // namespace v1
}  // namespace lgfx
