#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <mqtt/async_client.h>

#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 2048
#define CLAP_THRESHOLD 0.27f // Increased threshold for noisy cheap microphones
#define MIN_CLAP_GAP_MS 100 // Minimum gap between claps (ms)
#define MAX_CLAP_GAP_MS 800 // Maximum gap between claps (ms)
#define SILENCE_THRESHOLD 0.12f // Threshold for silence detection

const std::string CLIENT_ID("AudioNodePublisher");
const std::string USERNAME("ppaudionode");
const std::string PASSWORD("audio");
const std::string TOPIC("commands/light");

// Single clap detection function
bool detect_single_clap(PaStream* stream, std::vector<float>& buffer, float& peakAmplitude) {
    PaError err = Pa_ReadStream(stream, buffer.data(), FRAMES_PER_BUFFER);
    if (err == paInputOverflowed) {
        // Skip buffer data if overflowed
        return false;
    }
    if (err != paNoError) {
        std::cerr << "PortAudio read error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    // Simple Peak Detection for Clap
    float maxAmplitude = 0.0f;
    for (float sample : buffer) {
        float absSample = std::abs(sample);
        if (absSample > maxAmplitude) maxAmplitude = absSample;
    }

    peakAmplitude = maxAmplitude;
    return maxAmplitude > CLAP_THRESHOLD;
}

// Double-clap detection function with timing constraints
bool detect_double_clap(PaStream* stream, std::vector<float>& buffer) {
    using namespace std::chrono;
    float peakAmplitude = 0.0f;
    
    // Wait for first clap
    while (true) {
        if (detect_single_clap(stream, buffer, peakAmplitude)) {
            break;
        }
        // Small sleep to prevent tight loop CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    auto firstClapTime = steady_clock::now();
    
    // Wait for silence (avoid counting echoes/reverb)
    bool silenceDetected = false;
    while (!silenceDetected) {
        PaError err = Pa_ReadStream(stream, buffer.data(), FRAMES_PER_BUFFER);
        if (err == paInputOverflowed) continue; // Skip overflowed buffers
        if (err != paNoError) return false;
        
        float maxAmplitude = 0.0f;
        for (float sample : buffer) {
            float absSample = std::abs(sample);
            if (absSample > maxAmplitude) maxAmplitude = absSample;
        }
        
        if (maxAmplitude < SILENCE_THRESHOLD) {
            silenceDetected = true;
        }
        
        // Timeout if silence takes too long
        auto elapsed = duration_cast<milliseconds>(steady_clock::now() - firstClapTime).count();
        if (elapsed > MIN_CLAP_GAP_MS) {
            silenceDetected = true;
        }
    }
    
    // Look for second clap within time window
    while (true) {
        auto elapsed = duration_cast<milliseconds>(steady_clock::now() - firstClapTime).count();
        
        if (elapsed > MAX_CLAP_GAP_MS) {
            std::cout << "Second clap timeout (waited " << elapsed << "ms)" << std::endl;
            return false;
        }
        
        if (detect_single_clap(stream, buffer, peakAmplitude)) {
            std::cout << "Second clap detected! (Peak: " << peakAmplitude << ", Gap: " << elapsed << "ms)" << std::endl;
            return true;
        }
        
        // Small sleep to prevent tight loop
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <master_ip> [calibrate]" << std::endl;
        std::cerr << "  Add 'calibrate' to run calibration mode" << std::endl;
        return 1;
    }
    
    std::string master_ip = argv[1];
    bool calibrationMode = (argc > 2 && std::string(argv[2]) == "calibrate");
    std::string SERVER_ADDRESS = "tcp://" + master_ip + ":1883";
    
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);
    connOpts.set_user_name(USERNAME);
    connOpts.set_password(PASSWORD);

    // PortAudio initialization
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    if (inputParams.device == paNoDevice) {
        std::cerr << "No default input device found" << std::endl;
        Pa_Terminate();
        return 1;
    }
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = NULL;

    PaStream* stream;
    // Passing NULL for the callback enables blocking I/O (polling)
    err = Pa_OpenStream(&stream, &inputParams, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, NULL, NULL);
    if (err != paNoError) {
        std::cerr << "Failed to open stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }
    
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::vector<float> buffer(FRAMES_PER_BUFFER);
    std::cout << "PortAudio initialized successfully" << std::endl;

    // CALIBRATION MODE
    if (calibrationMode) {
        std::cout << "\n========== CALIBRATION MODE ==========" << std::endl;
        std::cout << "Step 1: Measuring ambient noise for 5 seconds..." << std::endl;
        std::cout << "        (Stay quiet)" << std::endl;
        
        float minNoise = 1.0f, maxNoise = 0.0f, avgNoise = 0.0f;
        int noiseCount = 0;
        auto startTime = std::chrono::steady_clock::now();
        
        while (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime).count() < 5) {
            PaError err = Pa_ReadStream(stream, buffer.data(), FRAMES_PER_BUFFER);
            if (err == paInputOverflowed) continue;
            if (err != paNoError) break;
            
            float maxAmp = 0.0f;
            for (float sample : buffer) {
                float absSample = std::abs(sample);
                if (absSample > maxAmp) maxAmp = absSample;
            }
            
            if (maxAmp < minNoise) minNoise = maxAmp;
            if (maxAmp > maxNoise) maxNoise = maxAmp;
            avgNoise += maxAmp;
            noiseCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        avgNoise /= noiseCount;
        std::cout << "   Noise floor: min=" << minNoise << ", avg=" << avgNoise << ", max=" << maxNoise << std::endl;
        
        std::cout << "\nStep 2: Clap detection test for 15 seconds..." << std::endl;
        std::cout << "        (Clap normally near the microphone)" << std::endl;
        
        float maxClapSeen = 0.0f;
        int clapCount = 0;
        startTime = std::chrono::steady_clock::now();
        
        while (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime).count() < 15) {
            PaError err = Pa_ReadStream(stream, buffer.data(), FRAMES_PER_BUFFER);
            if (err == paInputOverflowed) continue;
            if (err != paNoError) break;
            
            float maxAmp = 0.0f;
            for (float sample : buffer) {
                float absSample = std::abs(sample);
                if (absSample > maxAmp) maxAmp = absSample;
            }
            
            // Show any significant peaks
            if (maxAmp > avgNoise * 2) {
                std::cout << "   Peak detected: " << maxAmp;
                if (maxAmp > CLAP_THRESHOLD) {
                    std::cout << " [ABOVE THRESHOLD]";
                    clapCount++;
                } else {
                    std::cout << " [below threshold]";
                }
                std::cout << std::endl;
                
                if (maxAmp > maxClapSeen) maxClapSeen = maxAmp;
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Debounce
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "\n========== CALIBRATION RESULTS ==========" << std::endl;
        std::cout << "Ambient noise level: " << avgNoise << " (range: " << minNoise << " - " << maxNoise << ")" << std::endl;
        std::cout << "Highest clap seen: " << maxClapSeen << std::endl;
        std::cout << "Claps above threshold: " << clapCount << std::endl;
        std::cout << "\nCurrent settings:" << std::endl;
        std::cout << "  CLAP_THRESHOLD = " << CLAP_THRESHOLD << std::endl;
        std::cout << "  SILENCE_THRESHOLD = " << SILENCE_THRESHOLD << std::endl;
        std::cout << "\nRecommended settings:" << std::endl;
        
        float recommendedClap = maxClapSeen * 0.7f; // 70% of max clap seen
        float recommendedSilence = avgNoise + (maxNoise - avgNoise) * 1.5f; // Above noise floor
        
        std::cout << "  CLAP_THRESHOLD = " << recommendedClap << "f" << std::endl;
        std::cout << "  SILENCE_THRESHOLD = " << recommendedSilence << "f" << std::endl;
        
        if (avgNoise > 0.05f) {
            std::cout << "\n⚠ WARNING: High ambient noise detected!" << std::endl;
            std::cout << "  Consider moving to a quieter location or using a better microphone." << std::endl;
        }
        
        std::cout << "\nEdit the #define values in audio_main.cpp and rebuild." << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 0;
    }

    // NORMAL MODE
    try {
        std::cout << "Audio Node connecting to broker at " << SERVER_ADDRESS << "..." << std::endl;
        client.connect(connOpts)->wait();
        std::cout << "Connected to MQTT broker" << std::endl;
        std::cout << "Listening for DOUBLE claps. Press Ctrl+C to exit." << std::endl;
        std::cout << "Clap twice within " << MAX_CLAP_GAP_MS << "ms to trigger." << std::endl;
        
        while (true) {
            if (detect_double_clap(stream, buffer)) {
                std::string payload = "{\"action\":\"toggle\",\"source\":\"double_clap\"}";
                mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC, payload, 1, false);
                client.publish(pubmsg)->wait();
                std::cout << "[DOUBLE CLAP] Published command" << std::endl;
                
                // Add cooldown to prevent immediate re-triggering
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        client.disconnect()->wait();
        std::cout << "Disconnected" << std::endl;
    } catch (const mqtt::exception& exc) {
        std::cerr << "MQTT Error: " << exc.what() << std::endl;
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}