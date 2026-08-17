#include "DxLib.h"
#include "MyDraw.h"

#include <cassert>
#include <array>
#include <Windows.h>
#include <vector>
#include <fstream>
#include <complex>
#include <cmath>
#include <algorithm>
#include <cstring>

namespace
{
    constexpr int kFFT_SIZE = 1024;
    constexpr float kPI = 3.14159265359f;
    constexpr int kSpectrumBands = 8;
}

//==================================================
// WAVデータ
//==================================================

struct WavData
{
    int sampleRate = 0;
    int channels = 0;
    int bitsPerSample = 0;

    std::vector<float> samples;
};

//==================================================
// 周波数データ
//==================================================

struct FrequencyData
{
    std::array<float, 8> spectrum{};
};

//==================================================
// ビート検出
//==================================================

struct BeatDetector
{
    // 低音の平均値
    float averageBass = 0.001f;

    // 現在のビートの強さ
    float pulse = 0.0f;

    // 最後にビートを検出した時間
    int lastBeatTime = -1000;

    // ビート同士の最低間隔(ms)
    int minBeatInterval = 180;

    // 平均値に対して何倍ならビートとするか
    float threshold = 1.35f;
};

//==================================================
// WAV読み込み
// 24bit PCM専用
//==================================================

bool LoadWav(const char* fileName, WavData& wav)
{
    std::ifstream file(fileName, std::ios::binary);

    if (!file)
    {
        return false;
    }

    char riff[4];

    file.read(riff, 4);

    if (std::strncmp(riff, "RIFF", 4) != 0)
    {
        return false;
    }

    file.seekg(4, std::ios::cur);

    char wave[4];

    file.read(wave, 4);

    if (std::strncmp(wave, "WAVE", 4) != 0)
    {
        return false;
    }

    short audioFormat = 0;
    short channels = 0;
    int sampleRate = 0;
    short bitsPerSample = 0;

    std::vector<char> pcmData;

    while (file)
    {
        char chunkId[4];
        int chunkSize = 0;

        file.read(chunkId, 4);

        if (file.gcount() != 4)
        {
            break;
        }

        file.read(
            reinterpret_cast<char*>(&chunkSize),
            4
        );

        // fmt
        if (std::strncmp(chunkId, "fmt ", 4) == 0)
        {
            file.read(
                reinterpret_cast<char*>(&audioFormat),
                2
            );

            file.read(
                reinterpret_cast<char*>(&channels),
                2
            );

            file.read(
                reinterpret_cast<char*>(&sampleRate),
                4
            );

            file.seekg(6, std::ios::cur);

            file.read(
                reinterpret_cast<char*>(&bitsPerSample),
                2
            );

            file.seekg(
                chunkSize - 16,
                std::ios::cur
            );
        }

        // data
        else if (std::strncmp(chunkId, "data", 4) == 0)
        {
            pcmData.resize(chunkSize);

            file.read(
                pcmData.data(),
                chunkSize
            );

            break;
        }

        // その他
        else
        {
            file.seekg(
                chunkSize,
                std::ios::cur
            );
        }
    }

    if (audioFormat != 1)
    {
        return false;
    }

    if (bitsPerSample != 24)
    {
        return false;
    }

    if (channels <= 0)
    {
        return false;
    }

    wav.sampleRate = sampleRate;
    wav.channels = channels;
    wav.bitsPerSample = bitsPerSample;

    constexpr int bytesPerSample = 3;

    const size_t totalSamples =
        pcmData.size() / bytesPerSample;

    const size_t frameCount =
        totalSamples / channels;

    wav.samples.resize(frameCount);

    for (size_t frame = 0; frame < frameCount; ++frame)
    {
        float mono = 0.0f;

        for (int channel = 0; channel < channels; ++channel)
        {
            const size_t index =
                (frame * channels + channel)
                * bytesPerSample;

            int32_t sample =
                static_cast<unsigned char>(
                    pcmData[index]
                    )
                |
                (
                    static_cast<int32_t>(
                        static_cast<unsigned char>(
                            pcmData[index + 1]
                            )
                        ) << 8
                    )
                |
                (
                    static_cast<int32_t>(
                        static_cast<unsigned char>(
                            pcmData[index + 2]
                            )
                        ) << 16
                    );

            // 符号拡張
            if (sample & 0x00800000)
            {
                sample |= 0xFF000000;
            }

            mono +=
                static_cast<float>(sample)
                / 8388608.0f;
        }

        mono /= static_cast<float>(channels);

        wav.samples[frame] = mono;
    }

    return true;
}

//==================================================
// FFT
//==================================================

using Complex = std::complex<float>;

void FFT(std::vector<Complex>& data)
{
    const size_t N = data.size();

    if (N <= 1)
    {
        return;
    }

    std::vector<Complex> even(N / 2);
    std::vector<Complex> odd(N / 2);

    for (size_t i = 0; i < N / 2; ++i)
    {
        even[i] = data[i * 2];
        odd[i] = data[i * 2 + 1];
    }

    FFT(even);
    FFT(odd);

    for (size_t k = 0; k < N / 2; ++k)
    {
        float angle =
            -2.0f * kPI *
            static_cast<float>(k) /
            static_cast<float>(N);

        Complex w(
            std::cos(angle),
            std::sin(angle)
        );

        Complex t = w * odd[k];

        data[k] =
            even[k] + t;

        data[k + N / 2] =
            even[k] - t;
    }
}

//==================================================
// FFT → 8バンド
//==================================================

FrequencyData AnalyzeFrequency(
    const std::vector<Complex>& fftData,
    int sampleRate)
{
    FrequencyData result;

    const int fftSize =
        static_cast<int>(fftData.size());

    const float minFreq[8] =
    {
        20.0f,
        80.0f,
        160.0f,
        320.0f,
        640.0f,
        1280.0f,
        2560.0f,
        5120.0f
    };

    const float maxFreq[8] =
    {
        80.0f,
        160.0f,
        320.0f,
        640.0f,
        1280.0f,
        2560.0f,
        5120.0f,
        10000.0f
    };

    std::array<int, 8> count{};

    for (int i = 1; i < fftSize / 2; ++i)
    {
        float frequency =
            static_cast<float>(i) *
            static_cast<float>(sampleRate) /
            static_cast<float>(fftSize);

        float magnitude =
            std::abs(fftData[i]);

        for (int band = 0; band < 8; ++band)
        {
            if (frequency >= minFreq[band] &&
                frequency < maxFreq[band])
            {
                result.spectrum[band] += magnitude;
                count[band]++;
                break;
            }
        }
    }

    for (int band = 0; band < 8; ++band)
    {
        if (count[band] > 0)
        {
            result.spectrum[band] /=
                static_cast<float>(count[band]);
        }

        result.spectrum[band] *= 0.02f;

        result.spectrum[band] =
            std::clamp(
                result.spectrum[band],
                0.0f,
                1.0f
            );
    }

    return result;
}

//==================================================
// 現在再生中のBGMをFFT
//==================================================

FrequencyData AnalyzeCurrentBGM(
    const WavData& wav,
    int bgmHandle)
{
    int currentTime =
        GetSoundCurrentTime(bgmHandle);

    size_t startSample =
        static_cast<size_t>(
            static_cast<double>(currentTime) *
            wav.sampleRate /
            1000.0
            );

    std::vector<Complex> fftData(kFFT_SIZE);

    for (int i = 0; i < kFFT_SIZE; ++i)
    {
        size_t index =
            startSample + i;

        if (index < wav.samples.size())
        {
            fftData[i] =
                wav.samples[index];
        }
        else
        {
            fftData[i] = 0.0f;
        }
    }

    FFT(fftData);

    return AnalyzeFrequency(
        fftData,
        wav.sampleRate
    );
}

//==================================================
// ビート検出
//==================================================

float DetectBeat(
    const FrequencyData& frequency,
    BeatDetector& detector,
    int currentTime)
{
    // 低音を中心にする
    //
    // 20～80Hz  : 70%
    // 80～160Hz : 30%
    float bass =
        frequency.spectrum[0] * 0.7f +
        frequency.spectrum[1] * 0.3f;

    // 現在の低音が平均より十分大きいか
    bool isBeat =
        bass >
        detector.averageBass *
        detector.threshold;

    // あまり短い間隔でビートを発生させない
    bool enoughTime =
        currentTime -
        detector.lastBeatTime >=
        detector.minBeatInterval;

    if (isBeat && enoughTime)
    {
        detector.lastBeatTime =
            currentTime;

        detector.pulse = 1.0f;
    }

    // 平均値を徐々に更新
    detector.averageBass =
        detector.averageBass * 0.95f +
        bass * 0.05f;

    // 0以下にならないようにする
    detector.averageBass =
        max(
            detector.averageBass,
            0.001f
        );

    // ビートの余韻
    detector.pulse *= 0.88f;

    return detector.pulse;
}

//==================================================
// ゲーミングカラー
//==================================================

std::array<float, 3> GetRainbowColor(float time)
{
    float hue =
        std::fmod(time, 1.0f);

    float h =
        hue * 6.0f;

    int i =
        static_cast<int>(h);

    float f =
        h - static_cast<float>(i);

    float r;
    float g;
    float b;

    switch (i)
    {
    case 0:
        r = 1.0f;
        g = f;
        b = 0.0f;
        break;

    case 1:
        r = 1.0f - f;
        g = 1.0f;
        b = 0.0f;
        break;

    case 2:
        r = 0.0f;
        g = 1.0f;
        b = f;
        break;

    case 3:
        r = 0.0f;
        g = 1.0f - f;
        b = 1.0f;
        break;

    case 4:
        r = f;
        g = 0.0f;
        b = 1.0f;
        break;

    default:
        r = 1.0f;
        g = 0.0f;
        b = 1.0f - f;
        break;
    }

    return { r, g, b };
}

//==================================================
// Main
//==================================================
//==================================================
// Main
//==================================================

int WINAPI WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int)
{
    //==============================================
    // DXライブラリ初期化
    //==============================================

    ChangeWindowMode(TRUE);

    if (DxLib_Init() == -1)
    {
        return -1;
    }

    DxLib::SetDrawScreen(DX_SCREEN_BACK);

    //==============================================
    // シェーダー読み込み
    //==============================================

    int psH =
        LoadPixelShader(
            L"MV1PixelShader_Simple.pso"
        );

    assert(psH != -1);

    int outLinePSH =
        LoadPixelShader(
            L"ToonShader_OutLine.pso"
        );

    assert(outLinePSH != -1);

    int vsH =
        LoadVertexShader(
            L"MV1VertexShader_Bone.vso"
        );

    assert(vsH != -1);

    int outLineVSH =
        LoadVertexShader(
            L"VertexShader_Outline.vso"
        );

    assert(outLineVSH != -1);

    //==============================================
    // モデル
    //==============================================

    int modelH =
        MV1LoadModel(
            L"Human.mv1"
        );

    assert(modelH != -1);

    MV1SetUseOrigShader(true);

    //==============================================
    // アニメーション
    //==============================================

    int animIndex =
        MV1AttachAnim(
            modelH,
            1
        );

    float animTime = 0.0f;

    // モデル回転
    float angle = 0.0f;

    //==============================================
    // アウトライン用VS定数バッファ
    //==============================================

    struct VSOutLine
    {
        float width = 0.0f;
        float bass = 0.0f;
        float time = 0.0f;
        float spikePower = 0.0f;

        // ビート関連
        float beatPulse = 0.0f;
        float beatStrength = 0.0f;

        float padding1 = 0.0f;
        float padding2 = 0.0f;

        // スペクトラム
        float spectrum[8][4]{};
    };

    int outlineVSCBH =
        CreateShaderConstantBuffer(
            sizeof(VSOutLine)
        );

    VSOutLine* vsPointer =
        static_cast<VSOutLine*>(
            GetBufferShaderConstantBuffer(
                outlineVSCBH
            )
            );

    //==============================================
    // アウトライン用PS定数バッファ
    //==============================================

    struct PSOutLine
    {
        float color1 = 0.0f;
        float color2 = 0.0f;
        float color3 = 0.0f;
        float color4 = 0.0f;
    };

    int outlinePSCBH =
        CreateShaderConstantBuffer(
            sizeof(PSOutLine)
        );

    PSOutLine* psPointer =
        static_cast<PSOutLine*>(
            GetBufferShaderConstantBuffer(
                outlinePSCBH
            )
            );

    //==============================================
    // 画面サイズ
    //==============================================

    int width;
    int height;

    GetDrawScreenSize(
        &width,
        &height
    );

    //==============================================
    // レンダーターゲット
    //==============================================

    int rt1 =
        MakeScreen(
            width,
            height,
            true
        );

    int rt2 =
        MakeScreen(
            width,
            height,
            true
        );

    //==============================================
    // WAV解析
    //==============================================

    WavData wav;

    if (!LoadWav("BGM.wav", wav))
    {
        MessageBoxA(
            nullptr,
            "BGM.wavの読み込みに失敗しました",
            "Error",
            MB_OK
        );

        DxLib_End();

        return -1;
    }

    if (wav.samples.size() < kFFT_SIZE)
    {
        MessageBoxA(
            nullptr,
            "BGM.wavのサンプル数が1024未満です",
            "Error",
            MB_OK
        );

        DxLib_End();

        return -1;
    }

    //==============================================
    // BGM再生
    //==============================================

    int bgmH =
        LoadSoundMem(
            L"BGM.wav"
        );

    if (bgmH == -1)
    {
        MessageBoxA(
            nullptr,
            "BGM.wavの再生準備に失敗しました",
            "Error",
            MB_OK
        );

        DxLib_End();

        return -1;
    }

    PlaySoundMem(
        bgmH,
        DX_PLAYTYPE_LOOP
    );

    //==================================================
    // ビート解析用変数
    //==================================================

    // 前回のBass
    float previousBass = 0.0f;

    // Bassの平均値
    float bassAverage = 0.0f;

    // 前回ビートが発生した時間
    double previousBeatTime = -1.0;

    // 次に予測しているビート時間
    double nextBeatTime = -1.0;

    // 推定BPM
    float estimatedBPM = 0.0f;

    // 安定してきたBPM
    float targetBPM = 0.0f;

    // ビート間隔
    float beatInterval = 0.0f;

    // ビートパルス
    float beatPulse = 0.0f;

    // ビート強度
    float beatStrength = 0.0f;

    // 前回解析した時間
    int previousSoundTime = 0;

    // ビート間隔を保存
    std::vector<float> beatIntervals;

    // 最大保存数
    constexpr int kMaxBeatIntervals = 8;

    //==================================================
    // ビート検出パラメータ
    //==================================================

    // Bassがこれ以上急上昇したらビート候補
    constexpr float kBeatThreshold = 0.025f;

    // Bassがこの値以上ならビート候補
    constexpr float kMinimumBass = 0.08f;

    // ビート同士の最小間隔(ms)
    // これを短くしすぎると倍テンポを拾いやすい
    constexpr double kMinBeatInterval = 250.0;

    // ビート同士の最大間隔(ms)
    constexpr double kMaxBeatInterval = 1000.0;

    // BPMの最小値
    constexpr float kMinBPM = 60.0f;

    // BPMの最大値
    constexpr float kMaxBPM = 240.0f;

    // ビートパルス減衰
    constexpr float kBeatDecay = 0.88f;

    // BPMの追従速度
    constexpr float kBPMFollowSpeed = 0.08f;

    //==================================================
    // フレーム時間計測
    //==================================================

    int lastTime = GetNowCount();

    //==================================================
    // メインループ
    //==================================================

    while (
        ProcessMessage() == 0 &&
        CheckHitKey(KEY_INPUT_ESCAPE) == 0
        )
    {
        //==============================================
        // デルタタイム
        //==============================================

        int currentFrameTime = GetNowCount();

        float deltaTime =
            static_cast<float>(
                currentFrameTime - lastTime
                ) / 1000.0f;

        lastTime = currentFrameTime;

        // 異常値防止
        deltaTime =
            std::clamp(
                deltaTime,
                0.001f,
                0.1f
            );

        //==============================================
        // 画面クリア
        //==============================================

        DxLib::ClearDrawScreen();

        //==============================================
        // 虹色アウトライン
        //==============================================

        auto color =
            GetRainbowColor(
                angle * 0.1f
            );

        psPointer->color1 =
            color[0];

        psPointer->color2 =
            color[1];

        psPointer->color3 =
            color[2];

        psPointer->color4 =
            1.0f;

        UpdateShaderConstantBuffer(
            outlinePSCBH
        );

        SetShaderConstantBuffer(
            outlinePSCBH,
            DX_SHADERTYPE_PIXEL,
            6
        );

        //==============================================
        // 現在のBGMをFFT解析
        //==============================================

        FrequencyData frequency =
            AnalyzeCurrentBGM(
                wav,
                bgmH
            );

        //==============================================
        // Bass取得
        //==============================================

        float bass =
            frequency.spectrum[0];

        // Bassを少し平滑化
        bassAverage =
            bassAverage * 0.90f +
            bass * 0.10f;

        //==============================================
        // 現在の音楽時間
        //==============================================

        int currentSoundTime =
            GetSoundCurrentTime(
                bgmH
            );

        double currentTime =
            static_cast<double>(
                currentSoundTime
                );

        //==============================================
        // 曲がループした場合
        //==============================================

        if (currentSoundTime < previousSoundTime)
        {
            previousBeatTime = -1.0;
            nextBeatTime = -1.0;

            beatIntervals.clear();

            estimatedBPM = 0.0f;
            targetBPM = 0.0f;
            beatInterval = 0.0f;
        }

        previousSoundTime =
            currentSoundTime;

        //==============================================
        // Bassの急上昇量
        //==============================================

        float bassDelta =
            bass - previousBass;

        previousBass =
            bass;

        //==============================================
        // ビート候補検出
        //==============================================

        bool beatDetected = false;

        if (bassDelta > kBeatThreshold &&
            bass > kMinimumBass)
        {
            // 前回ビートからの時間
            if (previousBeatTime >= 0.0)
            {
                double interval =
                    currentTime -
                    previousBeatTime;

                // 妥当なビート間隔か
                if (interval >= kMinBeatInterval &&
                    interval <= kMaxBeatInterval)
                {
                    float newInterval =
                        static_cast<float>(
                            interval
                            );

                    //==================================
                    // ビート間隔を保存
                    //==================================

                    beatIntervals.push_back(
                        newInterval
                    );

                    if (
                        beatIntervals.size()
                        >
                        kMaxBeatIntervals
                        )
                    {
                        beatIntervals.erase(
                            beatIntervals.begin()
                        );
                    }

                    //==================================
                    // 平均ビート間隔
                    //==================================

                    float averageInterval = 0.0f;

                    for (float value :
                    beatIntervals)
                    {
                        averageInterval += value;
                    }

                    averageInterval /=
                        static_cast<float>(
                            beatIntervals.size()
                            );

                    beatInterval =
                        averageInterval;

                    //==================================
                    // BPM計算
                    //==================================

                    float bpm =
                        60000.0f /
                        averageInterval;

                    bpm =
                        std::clamp(
                            bpm,
                            kMinBPM,
                            kMaxBPM
                        );

                    //==================================
                    // BPMを徐々に追従
                    //==================================

                    if (estimatedBPM <= 0.0f)
                    {
                        estimatedBPM = bpm;
                    }
                    else
                    {
                        estimatedBPM =
                            estimatedBPM +
                            (
                                bpm -
                                estimatedBPM
                                )
                            * kBPMFollowSpeed;
                    }

                    targetBPM =
                        estimatedBPM;

                    //==================================
                    // 次のビート予測
                    //==================================

                    if (estimatedBPM > 0.0f)
                    {
                        beatInterval =
                            60000.0f /
                            estimatedBPM;

                        nextBeatTime =
                            currentTime +
                            beatInterval;
                    }

                    beatDetected = true;
                }
            }
            else
            {
                // 最初のビート
                beatDetected = true;
            }

            previousBeatTime =
                currentTime;
        }

        //==============================================
        // 最初のビート
        //==============================================

        if (beatDetected)
        {
            // Bassの強さからビート強度を作る
            float strength =
                bassDelta * 8.0f;

            strength =
                std::clamp(
                    strength,
                    0.25f,
                    1.0f
                );

            beatStrength =
                strength;

            // ビートパルス発生
            beatPulse = 1.0f;
        }

        //==============================================
        // BPMが分かった後は
        // 次のビートを予測してパルスを発生
        //==============================================

        if (estimatedBPM > 0.0f &&
            beatInterval > 0.0f &&
            nextBeatTime > 0.0)
        {
            // 次のビートを通過した
            if (currentTime >= nextBeatTime)
            {
                beatPulse = 1.0f;

                beatStrength = 0.7f;

                // 次のビートへ
                nextBeatTime +=
                    beatInterval;

                // 遅れすぎた場合
                if (
                    nextBeatTime <
                    currentTime
                    )
                {
                    nextBeatTime =
                        currentTime +
                        beatInterval;
                }
            }
        }

        //==============================================
        // ビートパルス減衰
        //==============================================

        // フレームレートに依存しにくい減衰
        beatPulse *=
            std::pow(
                kBeatDecay,
                deltaTime * 60.0f
            );

        beatStrength *=
            std::pow(
                0.92f,
                deltaTime * 60.0f
            );

        //==============================================
        // アウトライン用定数
        //==============================================

        vsPointer->width =
            0.5f;

        vsPointer->bass =
            bass;

        vsPointer->time =
            angle;

        // スペクトラムのトゲ
        vsPointer->spikePower =
            5.0f;

        // ビート
        vsPointer->beatPulse =
            beatPulse;

        vsPointer->beatStrength =
            beatStrength;

        //==============================================
        // スペクトラム
        //==============================================

        for (int i = 0; i < 8; ++i)
        {
            vsPointer->spectrum[i][0] =
                frequency.spectrum[i];

            vsPointer->spectrum[i][1] =
                0.0f;

            vsPointer->spectrum[i][2] =
                0.0f;

            vsPointer->spectrum[i][3] =
                0.0f;
        }

        //==============================================
       // モデル回転
       //==============================================

        angle += 0.1f;

        MV1SetRotationYUseDir(
            modelH,
            VGet(0, 1, 0),
            angle * 0.01f
        );

        //==============================================
        // アニメーション
        //==============================================

        animTime += 0.1f;

        MV1SetAttachAnimTime(
            modelH,
            animIndex,
            animTime
        );

        float animTotalTime =
            MV1GetAttachAnimTotalTime(
                modelH,
                animIndex
            );

        if (animTime >= animTotalTime)
        {
            animTime = 0.0f;
        }


        //==============================================
        // 定数バッファ更新
        //==============================================

        UpdateShaderConstantBuffer(
            outlineVSCBH
        );

        SetShaderConstantBuffer(
            outlineVSCBH,
            DX_SHADERTYPE_VERTEX,
            7
        );

        //==============================================
        // アウトライン描画
        //==============================================

        SetDrawScreen(rt1);

        ClearDrawScreen();

        SetCameraPositionAndTargetAndUpVec(
            VGet(0, 100, -200),
            VGet(0, 100, 0),
            VGet(0, 1, 0)
        );

        SetUseVertexShader(
            outLineVSH
        );

        SetUsePixelShader(
            outLinePSH
        );

        MV1DrawModel(
            modelH
        );

        //==============================================
        // 通常モデル描画
        //==============================================

        SetDrawScreen(rt2);

        ClearDrawScreen();

        SetCameraPositionAndTargetAndUpVec(
            VGet(0, 100, -200),
            VGet(0, 100, 0),
            VGet(0, 1, 0)
        );

        SetUseVertexShader(
            vsH
        );

        SetUsePixelShader(
            psH
        );

        MV1DrawModel(
            modelH
        );

        //==============================================
        // 画面へ合成
        //==============================================

        SetDrawScreen(
            DX_SCREEN_BACK
        );

        ClearDrawScreen();

        SetCameraPositionAndTargetAndUpVec(
            VGet(0, 100, -200),
            VGet(0, 100, 0),
            VGet(0, 1, 0)
        );

        // アウトライン
        DrawGraph(
            0,
            0,
            rt1,
            true
        );

        // 通常モデル
        DrawGraph(
            0,
            0,
            rt2,
            true
        );

        //==============================================
        // デバッグ表示
        //==============================================

        DrawFormatString(
            10,
            10,
            GetColor(255, 255, 255),
            L"Bass : %.3f",
            bass
        );

        DrawFormatString(
            10,
            30,
            GetColor(255, 255, 255),
            L"BPM : %.1f",
            estimatedBPM
        );

        DrawFormatString(
            10,
            50,
            GetColor(255, 255, 255),
            L"Beat Interval : %.1f ms",
            beatInterval
        );

        DrawFormatString(
            10,
            70,
            GetColor(255, 255, 255),
            L"Beat Pulse : %.3f",
            beatPulse
        );

        DrawFormatString(
            10,
            90,
            GetColor(255, 255, 255),
            L"Beat Strength : %.3f",
            beatStrength
        );

        DrawFormatString(
            10,
            110,
            GetColor(255, 255, 255),
            L"Next Beat : %.0f",
            nextBeatTime
        );

        //==============================================
        // 画面更新
        //==============================================

        ScreenFlip();
    }

    //==============================================
    // 終了
    //==============================================

    StopSoundMem(
        bgmH
    );

    DeleteSoundMem(
        bgmH
    );

    DeleteShaderConstantBuffer(
        outlineVSCBH
    );

    DeleteShaderConstantBuffer(
        outlinePSCBH
    );

    DxLib_End();

    return 0;
}