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
#define CLAP_THRESHOLD 1.15f // Increased threshold for noisy cheap microphones
#define MIN_CLAP_GAP_MS 100 // Minimum gap between claps (ms)
#define MAX_CLAP_GAP_MS 800 // Maximum gap between claps (ms)
#define SILENCE_THRESHOLD 0.6f // Threshold for silence detection

const std::string CLIENT_ID("AudioNodePublisher");
const std::string USERNAME("ppaudionode");
const std::string PASSWORD("audio");
const std::string TOPIC("commands/light");

// Single clap detection function
bool detect_single_clap(PaStream* stream, std::vector<float>& buffer, float& peakAmplitude, bool debug = false) {
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
    
    // Debug output to see actual levels
    if (debug && maxAmplitude > 0.05f) {
        std::cout << "Audio level: " << maxAmplitude << " (threshold: " << CLAP_THRESHOLD << ")" << std::endl;
    }
    
    return maxAmplitude > CLAP_THRESHOLD;
}

// Double-clap detection function with timing constraints
bool detect_double_clap(PaStream* stream, std::vector<float>& buffer) {
    using namespace std::chrono;
    float peakAmplitude = 0.0f;
    
    // Wait for first clap
    while (true) {
        if (detect_single_clap(stream, buffer, peakAmplitude)) {
            std::cout << "First clap detected! (Peak: " << peakAmplitude << ")" << std::endl;
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
        std::cerr << "Usage: " << argv[0] << " <master_ip>" << std::endl;
        return 1;
    }
    
    std::string master_ip = argv[1];
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
    
    // Print device info for debugging
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(inputParams.device);
    std::cout << "Using input device: " << deviceInfo->name << std::endl;
    std::cout << "Max input channels: " << deviceInfo->maxInputChannels << std::endl;
    std::cout << "Default sample rate: " << deviceInfo->defaultSampleRate << std::endl;

    try {
        std::cout << "Audio Node connecting to broker at " << SERVER_ADDRESS << "..." << std::endl;
        client.connect(connOpts)->wait();
        std::cout << "Connected to MQTT broker" << std::endl;
        std::cout << "Listening for DOUBLE claps. Press Ctrl+C to exit." << std::endl;
        std::cout << "Clap twice within " << MAX_CLAP_GAP_MS << "ms to trigger." << std::endl;
        
        // DEBUG MODE: Show audio levels for first 10 seconds
        std::cout << "\n=== DEBUG MODE: Showing audio levels for 10 seconds ===" << std::endl;
        std::cout << "Make some noise near the microphone..." << std::endl;
        auto debugStart = std::chrono::steady_clock::now();
        float peakAmplitude = 0.0f;
        
        while (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - debugStart).count() < 10) {
            detect_single_clap(stream, buffer, peakAmplitude, true);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "=== DEBUG MODE ENDED - Starting clap detection ===" << std::endl;
        std::cout << "Current threshold: " << CLAP_THRESHOLD << std::endl << std::endl;
        
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