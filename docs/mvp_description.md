# Clap Detection and Visualization System (MVP)
    FA'25 Semester -- Robotics Practicum ISR

This document outlines the Minimum Viable Product (MVP) for an IoT system designed to detect a loud audio event (a clap) on a remote sensor (Audio Node) and trigger a visual change on a centralized display (Visual Node) via an MQTT Broker (Master Node).


## Learning Objectives
One of my goals for this project was to reuse as much existing hardware and components I could find throughout the school. I had been rejected by a Zipline Embedded Software Co-op role for SP26 and immediately got to work ideating on projects. At the time, I couldn't decide what topics to focus on. Eventually, I landed on distributed systems with an emphasis on topics I've been meaning to dig deeper into--**DevOps, Embedded Systems, Self-Hosting/Security, and Full-Stack Software**. 

I repurposed my highschool laptop to serve as my Master Node and to grow as a platform for future DevOps tools such as Terraform, Ansible, Grafana, Prometheus, and AWS S3. 

## 1. System Architecture: MQTT Pub/Sub

**System Architecture Overview:**

The system follows a star topology using MQTT (Message Queuing Telemetry Transport), relying on a central broker for all communication. This decouples the sensor from the display.

**Communication Protocol:** MQTT (Port 1883)

**Core Topic:** `audio/clap/detected`

| Node          | Role                     | Hardware                     | Network ID                               | Key Action                                                      |
|---------------|--------------------------|------------------------------|------------------------------------------|----------------------------------------------------------------|
| Master Node   | Broker / Central Server   | Dell XPS 13 (Debian Trixie) | Static IP (10.x.x.100) or Hostname (dellxps13-master) | Hosts the central MQTT Broker.                                 |
| Audio Node    | Publisher / Sensor        | ODROID (Armbian/Debian)     | DHCP                                     | Listens for claps and Publishes to audio/clap/detected.       |
| Visual Node    | Subscriber / Display      | ODROID (Armbian/Debian)     | DHCP                                     | Subscribes to audio/clap/detected and renders a visual change. |

* **Communication Protocol**: MQTT (Port 1883)
* **Core Topic**: audio/clap/detected

## 2. Master Node (MQTT Broker) Configuration

The Master Node acts as the central hub.

### 2.1 Hardware and OS

- **Hardware**: Dell XPS 13 laptop.
- **OS**: Debian Trixie (Kernel 6.12.43+).
- **Wi-Fi Chipset**: Intel (iwlwifi driver).

### 2.2 Network Setup

- **Interface**: wlp2s0 (Intel Wi-Fi)
- **Configuration**: DHCP with **Hostname Resolution** (dellxps13-master).
- **Local Hostname Fix**: The /etc/hosts file was edited to map the hostname to 127.0.0.1 and 127.0.1.1 to resolve the sudo: unable to resolve host error.

### 2.3 MQTT Broker Setup

- **Software**: Mosquitto MQTT Broker.
- **Security**: Anonymous connections are disabled. All connecting clients (Audio/Visual Nodes) must authenticate.
- **Password File**: /etc/mosquitto/passwordfile
- **Configuration Directives Added**:
    - password_file /etc/mosquitto/passwordfile
    - allow_anonymous false
    - bind_address dellxps13-master

## 3. Audio Node (Clap Sensor) Functionality

The Audio Node is the IoT client responsible for sensing the environment.

### 3.1 Setup and Hardware

- **Device**: ODROID XU4 single-board computer.
- **Network**: DHCP (dynamic IP).
- **Sensory Input**: USB Microphone (tiny).
- **Development Language**: C++ (using the Paho MQTT C++ Client).
- **Key Library**: PortAudio (used for cross-platform audio input/microphone stream management).

### 3.2 MVP Logic

The MVP focuses solely on reliable detection and publishing.

- **Audio Processing**: The C++ application continuously reads audio samples from the USB microphone via PortAudio.
- **Clap Detection (MVP)**: A simple threshold-based detection method is used:
    - AmplitudeCurrent > ThresholdNoise
    (The threshold should be empirically determined based on ambient noise.)
- **Publishing**: Upon detecting an amplitude spike above the threshold, the node immediately publishes a message to the broker.

| Action         | Topic                  | QoS                     | Payload                                   |
|----------------|------------------------|-------------------------|-------------------------------------------|
| Clap Detected   | audio/clap/detected    | 1 (At Least Once)       | {"event": "clap", "time": "current_timestamp"} |

Note: The high Quality of Service (QoS 1) ensures the clap event is delivered to the Visual Node even if the network is briefly interrupted.

## 4. Visual Node (Display) Functionality

The Visual Node is the client responsible for acting on the sensor data.

### 4.1 Setup and Configuration

- **Device**: ODROID XU4 single-board computer.
- **Network**: DHCP (dynamic IP).
- **Development Language**: C++ (using the Paho MQTT C++ Client).
- **Video Streaming**: Utilizes OpenCV .

### 4.2 MVP Logic

The Visual Node waits passively for messages from the broker.

- **Subscription**: The node subscribes to the trigger topic.
- **Subscribe to**: audio/clap/detected
- **Visual Trigger**: Upon receiving any message on the audio/clap/detected topic:
    - The payload is received and acknowledged.
    - The application triggers a simple, immediate visual change (e.g., a screen flash, a color shift, or a text update like "CLAP DETECTED").
    - The visual state resets after a short delay (e.g., 500ms) to prepare for the next event.
- **Connection Target**: The client connects using the Master Node's hostname: tcp://dellxps13-master:1883.

