#include <Arduino.h>
#include <M5Unified.h>
#include <arduinoFFT.h>

auto gfx = &M5.Display;

const int micPin = 35;                 // コンデンサマイクのADCピン (通常36番ピン)
const int samples = 256;               // FFTのサンプル数（2のべき乗）
const float samplingFrequency = 44100; // サンプリング周波数
const float maxVoltage = 3.3;          // 最大電圧 (3.3V)
const int adcResolution = 4095;        // ADCの分解能 (12ビット)

unsigned int samplingPeriodUs;
unsigned long microseconds;

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);        // 画面の回転方向を設定
  M5.Display.fillScreen(TFT_BLACK); // 画面を黒でクリア

  gfx->setFont(&fonts::Font4);
  gfx->setTextPadding(48);
  gfx->setTextColor(TFT_WHITE, TFT_BLACK);
  gfx->setTextDatum(textdatum_t::middle_center);

  // ADCの精度を設定
  analogReadResolution(12); // 12ビットの分解能 (0-4095)

  samplingPeriodUs = round(1000000 * (1.0 / samplingFrequency));

  // 周波数ラベルの描画
  int barWidth = 1;    // 各バーの幅
  int maxHeight = 200; // バーの最大高さ
  int startX = 30;     // バーの描画開始位置
  int startY = 220;    // バーの描画開始Y位置
  int numLabels = 5;   // ラベルの数
  for (int j = 0; j <= numLabels; j++)
  {
    int x = startX + (j * (samples / 2) / numLabels * (barWidth + 1));
    float freq = (j * (samplingFrequency / 2) / numLabels) / 1000.0; // kHzに変換
    gfx->drawString(String(freq, 1), x, startY + 10, 2);
  }
}

void loop()
{
  /* サンプリング */
  microseconds = micros();
  for (int i = 0; i < samples; i++)
  {
    vReal[i] = analogRead(micPin) * (maxVoltage / adcResolution);
    vImag[i] = 0;
    while (micros() - microseconds < samplingPeriodUs)
    {
      // サンプリング周期が終わるまで待つ
    }
    microseconds += samplingPeriodUs;
  }

  /* FFTを実行 */
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* ウィンドウ関数を適用 */
  FFT.compute(FFT_FORWARD);                        /* FFTを計算 */
  FFT.complexToMagnitude();                        /* 結果を複素数から絶対値に変換 */

  /* 結果を表示 */
  //  M5.Display.fillScreen(TFT_BLACK); // 画面をクリア

  int barWidth = 1;    // 各バーの幅
  int maxHeight = 200; // バーの最大高さ
  int startX = 30;     // バーの描画開始位置
  int startY = 215;    // バーの描画開始Y位置

  for (int i = 2; i < (samples / 2); i++)
  {                                                  // DC成分とNyquist成分は除外
    float barHeight = (vReal[i] / 50.0) * maxHeight; // バーの高さを計算
    if (barHeight > maxHeight)
      barHeight = maxHeight; // 高さを最大値に制限
    M5.Display.fillRect(startX + (i * (barWidth + 1)), startY - barHeight, barWidth, barHeight, TFT_GREEN);
    M5.Display.fillRect(startX + (i * (barWidth + 1)), startY - barHeight - 1, barWidth, -maxHeight, TFT_BLACK);
  }

  //  delay(10); // 更新周期
}
