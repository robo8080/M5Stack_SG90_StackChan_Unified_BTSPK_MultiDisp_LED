#if defined ( ARDUINO )
#include <Arduino.h>
//// If you use SD card, write this.
#include <SD.h>

//// If you use SPIFFS, write this.
//#include <SPIFFS.h>
#endif

#if defined(ARDUINO_M5STACK_Core2)
//  #include <M5Core2.h>
  #define SERVO_PIN_X 13
  #define SERVO_PIN_Y 14
#elif defined( ARDUINO_M5STACK_FIRE )
//  #include <M5Stack.h>
  #define SERVO_PIN_X 21
  #define SERVO_PIN_Y 22
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
//  #include <M5Stack.h>
  #define SERVO_PIN_X 21
  #define SERVO_PIN_Y 22
#endif
#include <ServoEasing.hpp> // https://github.com/ArminJo/ServoEasing       
#include <M5Unified.h>
#include <M5UnitOLED.h>
#include <M5UnitLCD.h>

#define USE_SD_UPDATER
//#ifdef USE_SD_UPDATER
#define SDU_APP_NAME "SG90_StackChan_BTSPK_LED"
#include <M5StackUpdater.h>
#endif

//M5UnitOLED oled;
LGFX_Device* gfx2 = nullptr;

int16_t lipsync_level = 0;

#include "Avatar.h"
#if defined( ARDUINO_M5STACK_FIRE ) || defined(ARDUINO_M5STACK_Core2)
#define USE_ATARU_FACE
#ifdef USE_ATARU_FACE  
#include "AtaruFace.h"
#endif
#endif
using namespace m5avatar;

Avatar avatar;
#ifdef USE_ATARU_FACE  
Face* face;
ColorPalette* cp;
#endif

#if 1
#include <FastLED.h>
// How many leds in your strip?
#define NUM_LEDS 10
#define DATA_PIN 25
// Define the array of leds
CRGB leds[NUM_LEDS];
CRGB led_table[NUM_LEDS/2] = {CRGB::Blue,CRGB::Green,CRGB::Yellow,CRGB::Orange,CRGB::Red};
void turn_off_led() {
  // Now turn the LED off, then pause
  for(int i=0;i<NUM_LEDS;i++) leds[i] = CRGB::Black;
  FastLED.show();  
}

void clear_led_buff() {
  // Now turn the LED off, then pause
  for(int i=0;i<NUM_LEDS;i++) leds[i] =  CRGB::Black;
}

void level_led(int level1, int level2) {  
if(level1 > 5) level1 = 5;
if(level2 > 5) level2 = 5;
  
  clear_led_buff(); 
  for(int i=0;i<level1;i++){
    leds[NUM_LEDS/2-1-i] = led_table[i];
  }
  for(int i=0;i<level2;i++){
    leds[i+NUM_LEDS/2] = led_table[i];
  }
  FastLED.show();
}

#else
#include <Adafruit_NeoPixel.h>
#define PIN 25 
#define NUMPIXELS 10 //LEDの数を指定
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //800kHzでNeoPixelを駆動
uint32_t led_table[NUM_LEDS/2] = {0x0000FF,0x008000,0xFFFF00,0xFFA500,0xFF0000};
//        Blue=0x0000FF,
//        Green=0x008000,
//        Yellow=0xFFFF00,
//        Orange=0xFFA500,
//        Red=0xFF0000,

void level_led(int level1, int level2) {  
  pixels.clear(); // NeoPixelのリセット
  for(int i=0;i<level1;i++){
    pixels.setPixelColor(NUM_LEDS/2-1-i, led_table[i]); // i番目に設定
  }
  for(int i=0;i<level2;i++){
    pixels.setPixelColor(i+NUM_LEDS/2, led_table[i]); // i番目に設定
  }
  pixels.show();   //LEDに色を反映
}
#endif

/// need ESP32-A2DP library. ( URL : https://github.com/pschatzmann/ESP32-A2DP/ )
#include <BluetoothA2DPSink.h>

/// set M5Speaker virtual channel (0-7)
static constexpr uint8_t m5spk_virtual_channel = 0;

/// set ESP32-A2DP device name
static constexpr char bt_device_name[] = "ESP32";

int bt_status = 0;

class BluetoothA2DPSink_M5Speaker : public BluetoothA2DPSink
{
public:
  BluetoothA2DPSink_M5Speaker(m5::Speaker_Class* m5sound, uint8_t virtual_channel = 0)
  : BluetoothA2DPSink()
  {
    is_i2s_output = false; // I2S control by BluetoothA2DPSink is not required.
  }

  // get rawdata buffer for FFT.
  const int16_t* getBuffer(void) const { return _flip_buf[_flip_index]; }

  const char* getMetaData(size_t id) { _meta_bits &= ~(1<<id); return (id < metatext_num) ? _meta_text[id] : nullptr; }

  uint8_t getMetaUpdateInfo(void) const { return _meta_bits; }

  void clear(void)
  {
    for (int i = 0; i < 2; ++i)
    {
      if (_flip_buf[i]) { memset(_flip_buf[i], 0, _flip_buf_size[i]); }
    }
  }

  static constexpr size_t metatext_size = 80;
  static constexpr size_t metatext_num = 3;

protected:
  int16_t* _flip_buf[2] = { nullptr, nullptr };
  size_t _flip_buf_size[2] = { 0, 0 };
  bool _flip_index = 0;
  char _meta_text[metatext_num][metatext_size];
  uint8_t _meta_bits = 0;
  size_t _sample_rate = 48000;

  void clearMetaData(void)
  {
    for (int i = 0; i < metatext_num; ++i)
    {
      _meta_text[i][0] = 0;
    }
    _meta_bits = (1<<metatext_num)-1;
  }

  void av_hdl_a2d_evt(uint16_t event, void *p_param) override
  {
    esp_a2d_cb_param_t* a2d = (esp_a2d_cb_param_t *)(p_param);

    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
      if (ESP_A2D_CONNECTION_STATE_CONNECTED == a2d->conn_stat.state)
      { // 接続
          avatar.setExpression(Expression::Neutral);
          delay(500);    
          avatar.setExpression(Expression::Doubt);    
          bt_status = 1;
      }
      else
      if (ESP_A2D_CONNECTION_STATE_DISCONNECTED == a2d->conn_stat.state)
      { // 切断
          avatar.setExpression(Expression::Sleepy);    
          bt_status = 0;
      }
      break;

    case ESP_A2D_AUDIO_STATE_EVT:
      if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state)
      { // 再生
        avatar.setExpression(Expression::Neutral);    
        bt_status = 2;
      } else
      if ( ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND == a2d->audio_stat.state
        || ESP_A2D_AUDIO_STATE_STOPPED        == a2d->audio_stat.state )
      { // 停止
        clearMetaData();
        avatar.setExpression(Expression::Doubt);    
        bt_status = 3;
      }
      break;

    case ESP_A2D_AUDIO_CFG_EVT:
      {
        esp_a2d_cb_param_t *a2d = (esp_a2d_cb_param_t *)(p_param);
        size_t tmp = a2d->audio_cfg.mcc.cie.sbc[0];
        size_t rate = 16000;
        if (     tmp & (1 << 6)) { rate = 32000; }
        else if (tmp & (1 << 5)) { rate = 44100; }
        else if (tmp & (1 << 4)) { rate = 48000; }
        _sample_rate = rate;
      }
      break;

    default:
      break;
    }

    BluetoothA2DPSink::av_hdl_a2d_evt(event, p_param);
  }

  void av_hdl_avrc_evt(uint16_t event, void *p_param) override
  {
    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(p_param);

    switch (event)
    {
    case ESP_AVRC_CT_METADATA_RSP_EVT:
      for (size_t i = 0; i < metatext_num; ++i)
      {
        if (0 == (rc->meta_rsp.attr_id & (1 << i))) { continue; }
        strncpy(_meta_text[i], (char*)(rc->meta_rsp.attr_text), metatext_size);
        _meta_bits |= rc->meta_rsp.attr_id;
        break;
      }
      break;

    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
      break;

    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
      break;

    default:
      break;
    }

    BluetoothA2DPSink::av_hdl_avrc_evt(event, p_param);
  }

  void audio_data_callback(const uint8_t *data, uint32_t length) override
  {
    /// When the queue is empty or full, delay processing is performed.
    if (M5.Speaker.isPlaying(m5spk_virtual_channel) != 1)
    {
//      vTaskDelay(5 / portTICK_RATE_MS);
//      while (M5.Speaker.isPlaying(m5spk_virtual_channel) == 2) { taskYIELD(); }
      do { vTaskDelay(1); } while (M5.Speaker.isPlaying(m5spk_virtual_channel) > 1);
    }
    bool flip = !_flip_index;
    if (_flip_buf_size[flip] < length)
    {
      _flip_buf_size[flip] = length;
      if (_flip_buf[flip] != nullptr) { heap_caps_free(_flip_buf[flip]); }
      auto tmp = (int16_t*)heap_caps_malloc(length, MALLOC_CAP_8BIT);
      _flip_buf[flip] = tmp;
      if (tmp == nullptr)
      {
        _flip_buf_size[flip] = 0;
        return;        
      }
    }
    memcpy(_flip_buf[flip], data, length);
    _flip_index = flip;
    M5.Speaker.playRAW(_flip_buf[flip], length >> 1, _sample_rate, true, 1, m5spk_virtual_channel);
  }
};


#define FFT_SIZE 256
class fft_t
{
  float _wr[FFT_SIZE + 1];
  float _wi[FFT_SIZE + 1];
  float _fr[FFT_SIZE + 1];
  float _fi[FFT_SIZE + 1];
  uint16_t _br[FFT_SIZE + 1];
  size_t _ie;

public:
  fft_t(void)
  {
#ifndef M_PI
#define M_PI 3.141592653
#endif
    _ie = logf( (float)FFT_SIZE ) / log(2.0) + 0.5;
    static constexpr float omega = 2.0f * M_PI / FFT_SIZE;
    static constexpr int s4 = FFT_SIZE / 4;
    static constexpr int s2 = FFT_SIZE / 2;
    for ( int i = 1 ; i < s4 ; ++i)
    {
    float f = cosf(omega * i);
      _wi[s4 + i] = f;
      _wi[s4 - i] = f;
      _wr[     i] = f;
      _wr[s2 - i] = -f;
    }
    _wi[s4] = _wr[0] = 1;

    size_t je = 1;
    _br[0] = 0;
    _br[1] = FFT_SIZE / 2;
    for ( size_t i = 0 ; i < _ie - 1 ; ++i )
    {
      _br[ je << 1 ] = _br[ je ] >> 1;
      je = je << 1;
      for ( size_t j = 1 ; j < je ; ++j )
      {
        _br[je + j] = _br[je] + _br[j];
      }
    }
  }

  void exec(const int16_t* in)
  {
    memset(_fi, 0, sizeof(_fi));
    for ( size_t j = 0 ; j < FFT_SIZE / 2 ; ++j )
    {
      float basej = 0.25 * (1.0-_wr[j]);
      size_t r = FFT_SIZE - j - 1;

      /// perform han window and stereo to mono convert.
      _fr[_br[j]] = basej * (in[j * 2] + in[j * 2 + 1]);
      _fr[_br[r]] = basej * (in[r * 2] + in[r * 2 + 1]);
    }

    size_t s = 1;
    size_t i = 0;
    do
    {
      size_t ke = s;
      s <<= 1;
      size_t je = FFT_SIZE / s;
      size_t j = 0;
      do
      {
        size_t k = 0;
        do
        {
          size_t l = s * j + k;
          size_t m = ke * (2 * j + 1) + k;
          size_t p = je * k;
          float Wxmr = _fr[m] * _wr[p] + _fi[m] * _wi[p];
          float Wxmi = _fi[m] * _wr[p] - _fr[m] * _wi[p];
          _fr[m] = _fr[l] - Wxmr;
          _fi[m] = _fi[l] - Wxmi;
          _fr[l] += Wxmr;
          _fi[l] += Wxmi;
        } while ( ++k < ke) ;
      } while ( ++j < je );
    } while ( ++i < _ie );
  }

  uint32_t get(size_t index)
  {
    return (index < FFT_SIZE / 2) ? (uint32_t)sqrtf(_fr[ index ] * _fr[ index ] + _fi[ index ] * _fi[ index ]) : 0u;
  }
};


static BluetoothA2DPSink_M5Speaker a2dp_sink = { &M5.Speaker, m5spk_virtual_channel };
static fft_t fft;
static bool fft_enabled = false;
static uint16_t prev_y[(FFT_SIZE/2)+1];
static uint16_t peak_y[(FFT_SIZE/2)+1];
static int header_height = 0;


uint32_t bgcolor(LGFX_Device* gfx, int y)
{
  auto h = gfx->height();
  auto dh = h - header_height;
  int v = ((h - y)<<5) / dh;
  if (dh > 40)
  {
    int v2 = ((h - y + 1)<<5) / dh;
    if ((v >> 2) != (v2 >> 2))
    {
      return 0x666666u;
    }
  }
  return gfx->color888(v + 2, v, v + 6);
}

void gfxSetup(LGFX_Device* gfx)
{
  if (gfx == nullptr) { return; }
  gfx->setRotation(3);
//  if (gfx->width() < gfx->height())
//  {
//    gfx->setRotation(gfx->getRotation()^1);
//  }
  gfx->setFont(&fonts::lgfxJapanGothic_12);
  gfx->setEpdMode(epd_mode_t::epd_fastest);
  gfx->setCursor(0, 8);
  gfx->print("BT A2DP : ");
  gfx->println(bt_device_name);
  gfx->setTextWrap(false);
  gfx->fillRect(0, 6, gfx->width(), 2, TFT_BLACK);

  header_height = 45;
  fft_enabled = !gfx->isEPD();
  if (fft_enabled)
  {
    for (int y = header_height; y < gfx->height(); ++y)
    {
      gfx->drawFastHLine(0, y, gfx->width(), bgcolor(gfx, y));
    }
  }

  for (int x = 0; x < (FFT_SIZE/2)+1; ++x)
  {
    prev_y[x] = INT16_MAX;
    peak_y[x] = INT16_MAX;
  }
}

void gfxLoop(LGFX_Device* gfx)
{
  if (gfx == nullptr) { return; }
  auto bits = a2dp_sink.getMetaUpdateInfo();
  if (bits)
  {
    gfx->startWrite();
    for (int id = 0; id < a2dp_sink.metatext_num; ++id)
    {
      if (0 == (bits & (1<<id))) { continue; }
      gfx->setCursor(0, 8 + id * 12);
      gfx->fillRect(0, 8 + id * 12, gfx->width(), 12, gfx->getBaseColor());
      gfx->print(a2dp_sink.getMetaData(id));
      gfx->print(" "); // Garbage data removal when UTF8 characters are broken in the middle.
    }
    gfx->display();
    gfx->endWrite();
  }

  if (!gfx->displayBusy())
  { // draw volume bar
    static int px;
    uint8_t v = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    int x = v * (gfx->width()) >> 8;
    if (px != x)
    {
      gfx->fillRect(x, 6, px - x, 2, px < x ? 0xAAFFAAu : 0u);
      gfx->display();
      px = x;
    }
  }

  if (fft_enabled && !gfx->displayBusy())
  {
    static int prev_x[2];
    static int peak_x[2];
    static bool prev_conn;
    bool connected = a2dp_sink.is_connected();
    if (prev_conn != connected)
    {
      prev_conn = connected;
      if (!connected)
      {
        a2dp_sink.clear();
      }
    }

    auto data = a2dp_sink.getBuffer();
    if (data)
    {
      level_led(abs(data[1])*10/INT16_MAX,abs(data[0])*10/INT16_MAX);
      gfx->startWrite();

      // draw stereo level meter
       uint16_t level[2] = { 0, 0 };
      for (int i = 0; i < 512; i += 16)
      {
        uint32_t lv = abs(data[i]);
        if (level[0] < lv) { level[0] = lv; }
        lv = abs(data[i+1]);
        if (level[1] < lv) { level[1] = lv; }
      }
      for (int i = 0; i < 2; ++i)
      {
        int x = (level[i] * gfx->width() - 4) / INT16_MAX;
        int px = prev_x[i];
        if (px != x)
        {
          gfx->fillRect(x, i * 3, px - x, 2, px < x ? 0xFF9900u : 0x330000u);
          prev_x[i] = x;
        }
        px = peak_x[i];
        if (px > x)
        {
          gfx->writeFastVLine(px, i * 3, 2, TFT_BLACK);
          px--;
        }
        else
        {
          px = x;
        }
        if (peak_x[i] != px)
        {
          peak_x[i] = px;
          gfx->writeFastVLine(px, i * 3, 2, TFT_WHITE);
        }
      }
      gfx->display();

      // draw FFT level meter
      fft.exec(data);
      int bw = gfx->width() / 60;
      if (bw < 3) { bw = 3; }
      int dsp_height = gfx->height();
      int fft_height = dsp_height - header_height - 1;
      int xe = gfx->width() / bw;
      if (xe > (FFT_SIZE/2)) { xe = (FFT_SIZE/2); }
      int32_t lipsync_temp = 0;
      for (int x = 0; x <= xe; ++x)
      {
        if (((x * bw) & 7) == 0) { gfx->display(); }
        int32_t f = fft.get(x);
        int y = (f * fft_height) >> 18;
        if (x > 0 and x < 10) { // 0〜31の範囲でlipsyncでピックアップしたい音域を選びます。
          int32_t f1 = f * 100;
          lipsync_temp = std::max(lipsync_temp, f1 >> 19); // 指定した範囲で最も高い音量を設定。
//            lipsync_temp += (f1 >> 18);
        }
        if (y > fft_height) { y = fft_height; }
        y = dsp_height - y;
        int py = prev_y[x];
        if (y != py)
        {
//          gfx->fillRect(x * bw, y, bw - 1, py - y, (y < py) ? 0x99AAFFu : 0x000033u);
            if (x > 0 and x < 10) { // 0〜31の範囲でlipsyncでピックアップしたい音域を選びます。
              gfx->fillRect(x * bw, y, bw - 1, py - y, (y < py) ? 0x00F800u : 0x000033u);
            } else {
              gfx->fillRect(x * bw, y, bw - 1, py - y, (y < py) ? 0x99AAFFu : 0x000033u);
            }
          prev_y[x] = y;
        }
        py = peak_y[x] + 1;
        if (py < y)
        {
          gfx->writeFastHLine(x * bw, py - 1, bw - 1, bgcolor(gfx, py - 1));
        }
        else
        {
          py = y - 1;
        }
        if (peak_y[x] != py)
        {
          peak_y[x] = py;
          gfx->writeFastHLine(x * bw, py, bw - 1, TFT_WHITE);
        }
      }
      lipsync_level = lipsync_temp; // リップシンクを設定
      gfx->display();
      gfx->endWrite();
    }
  }
}

#define START_DEGREE_VALUE_X 90
#define START_DEGREE_VALUE_Y 90
ServoEasing servo_x;
ServoEasing servo_y;

void lipSync(void *args)
{
  float gazeX, gazeY;
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
   for (;;)
  {
    //int level = a2dp_sink.audio_level;
//    printf("data=%d\n\r",lipsync_level);
    int level = abs(lipsync_level);
//    lipsync_level *= 3;
    if(level > 120)
    {
      level = 120;
    }
    float open = (float)level/120.0;
    avatar->setMouthOpenRatio(open);
    avatar->getGaze(&gazeY, &gazeX);
    avatar->setRotation(gazeX * 5);
    if(bt_status == 2)
    {
        servo_x.setEaseTo(START_DEGREE_VALUE_X + (int)(20.0 * gazeX));
        if(gazeY < 0) {
          int tmp = (int)(15.0 * gazeY + open * 15.0);
          if(tmp > 15) tmp = 15;
          servo_y.setEaseTo(START_DEGREE_VALUE_Y + tmp);
        } else {
          servo_y.setEaseTo(START_DEGREE_VALUE_Y + (int)(10.0 * gazeY) - open * 15.0);
        }
    } else {
       avatar->setRotation(gazeX * 5);
       float b = avatar->getBreath();
       servo_x.setEaseTo(START_DEGREE_VALUE_X); 
       servo_y.setEaseTo(START_DEGREE_VALUE_Y + b * 5);
    }
    synchronizeAllServosStartAndWaitForAllServosToStop();
     delay(50);
  }
}

void setup()
{
 auto cfg = M5.config();

  cfg.external_spk = true;    /// use external speaker (SPK HAT / ATOMIC SPK)
//cfg.external_spk_detail.omit_atomic_spk = true; // exclude ATOMIC SPK
//cfg.external_spk_detail.omit_spk_hat    = true; // exclude SPK HAT

  M5.begin(cfg);

#ifdef USE_SD_UPDATER
  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    5000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );
#endif

  { /// custom setting
    auto spk_cfg = M5.Speaker.config();
    /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
    spk_cfg.sample_rate = 96000; // default:48000 (48kHz)  e.g. 50000 , 80000 , 96000 , 100000 , 144000 , 192000
//    spk_cfg.task_pinned)core = 1;
//    spk_cfg.task_priprity = configMAX_PRIORITIS -2;
    spk_cfg.dma_buf_count = 16;
    spk_cfg.dma_buf_len = 256;
    M5.Speaker.config(spk_cfg);
  }


  M5.Speaker.begin();

  M5.Display.setBrightness(128);
  
  header_height = 48;
  for (int x = 0; x < (FFT_SIZE/2)+1; ++x)
  {
    prev_y[x] = INT16_MAX;
    peak_y[x] = INT16_MAX;
  }

  a2dp_sink.start(bt_device_name, false);
  
  if (servo_x.attach(SERVO_PIN_X, START_DEGREE_VALUE_X, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo x");
  }
  if (servo_y.attach(SERVO_PIN_Y, START_DEGREE_VALUE_Y, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo y");
  }
  servo_x.setEasingType(EASE_QUADRATIC_IN_OUT);
  servo_y.setEasingType(EASE_QUADRATIC_IN_OUT);
  setSpeedForAllServos(60);

#if 1
  FastLED.addLeds<SK6812, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(32);
  level_led(5, 5);
  delay(1000);
  turn_off_led();
#else
  pixels.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();            // Turn OFF all pixels ASAP
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  level_led(5, 5);
  delay(1000);
  pixels.clear(); // NeoPixelのリセット
  pixels.show();            // Turn OFF all pixels ASAP
#endif
  gfx2 = new M5UnitLCD();
  if(!gfx2->init()) {
    delete gfx2;
    gfx2 = new M5UnitOLED();
    gfx2->init();
  }
  gfxSetup(gfx2);
#ifdef USE_ATARU_FACE  
  face = new AtaruFace();
  cp = new ColorPalette();
  cp->set(COLOR_PRIMARY, TFT_BLACK);
  cp->set(COLOR_BACKGROUND, TFT_WHITE);
  cp->set(COLOR_SECONDARY, TFT_WHITE);
#endif
  avatar.init(); // start drawing
#ifdef USE_ATARU_FACE  
  avatar.setFace(face);
  avatar.setColorPalette(*cp);
#endif
  avatar.setExpression(Expression::Sleepy);
  avatar.addTask(lipSync, "lipSync");
}
int BatteryLevel = 0;
void loop(void)
{
  gfxLoop(gfx2);

  {
    static int prev_frame;
    int frame;
    while (prev_frame == (frame = millis() >> 3)) /// 8 msec cycle wait
    {
      vTaskDelay(1);
    }
    prev_frame = frame;
  }
  static int lastms = 0;
  if (millis()-lastms > 1000) {
    lastms = millis();
    BatteryLevel = M5.Power.getBatteryLevel();
//    printf("%d\n\r",BatVoltage);
   }

  M5.update();
  if (M5.BtnA.wasClicked())
  {
    M5.Speaker.tone(1000, 100);
    a2dp_sink.next();
  }
  if (M5.BtnA.isHolding() || M5.BtnB.isPressed() || M5.BtnC.isPressed())
  {
    size_t v = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    if (M5.BtnB.isPressed()) { --v; } else { ++v; }
    if (v <= 255 || M5.BtnA.isHolding())
    {
      M5.Speaker.setChannelVolume(m5spk_virtual_channel, v);
    }
  }
}

#if !defined ( ARDUINO )
extern "C" {
  void loopTask(void*)
  {
    setup();
    for (;;) {
      loop();
    }
    vTaskDelete(NULL);
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif
