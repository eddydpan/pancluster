# Distributed Systems -- Home Automation

Last updated: December 17, 2025 

<aside>
💡
Project planning document for a home-automation distributed systems project

</aside>

## Build Instructions

### Master Node
```bash
cmake -B build
cmake --build build
./build/src/master_node/master_node
```

### Follower Nodes (Audio/Visual)
Follower nodes require MQTT libraries:
```bash
cmake -B build -DBUILD_FOLLOWER_NODES=ON
cmake --build build
./build/src/follower_nodes/audio_node/audio_node <master_ip>
```

Example:
```bash
./build/src/follower_nodes/audio_node/audio_node 192.168.32.137
```

## Learning Goals

- [ ]  Gain experience working with distributed systems by building one up from scratch
- [ ]  Learn embedded topics (especially embedded linux)
- [ ]  Develop a platform for future learning while also having relevant

## System Architecture

I am using **Mosquitto (MQTT)** for my inter-process communication (IPC) protocol. It’s lightweight and there are libraries that I can patch together to build ‘drivers’ for my TP-Link Kasa smart LEDs. Another alternative is Redis, which feels more industry-standard than MQTT, but since the MQTT interface is so friendly, I can move fast and develop with it, ensuring that I keep my IPC configurable in its interface if I feel the desire to switch out the implementation. 

## Project Components

For this project, I plan on having three nodes: one leader node, and two followers. 

### Leader Node

The leader node is hosted on a Dell XPS 13 Laptop. I am running a minimal Debian Trixie for my OS so that it’s lightweight but still easily configurable within the `apt` package manager. This leader node will receive **heartbeats** from the node and monitor the follower’s health. Additionally, it will keep track of the follower nodes’ provisioning, configuration, and maintenance through `Ansible`. It can also host all my plans I had in my [XTC Project Proposal](https://www.notion.so/XTC-Project-Proposal-29cb3684e97e8092a4a5c6ab8ea6d9fd?pvs=21)  of CI/CD self-hosted runners, health dashboard through Grafana/Prometheus, hosting my portfolio website, and acting as an endpoint for me to access an exposed API in the form of a GUI/ Dashboard through my mobile devices. 

### Follower Nodes

Each follower node will be hosted on an ODROID-XU4

![image.png](Distributed%20Systems/image.png)

an *ARM Quad Core 2GHz A15, 2GB RAM single board computer (SBC)* akin to a Raspberry Pi 4. This SBC has more than enough processing power to handle DSP for tasks like audio input and basic CV.

Follower nodes are also responsible for publishing their health telemetry to the master node in two forms:  a *heartbeat* (every 5 seconds) and *performance metrics* (CPU, RAM, etc.) 

#### Follower Node #1: CV Node  
**[TO BE IMPLEMENTED]**

For one of the follower nodes, I plan on having it run real-time CV (using a standard library like OpenCV) with a USB Camera peripheral. This could monitor something simple like bathroom usage throughout the day, or run a person-detection model to collect data on my daily routines and how much time I spend in my room.

#### Follower Node #2: Audio Node  

This follower node will run some digital-signal processing on audio input from a USB Microphone. Due to the poor audio capture from the budget microphone, the audio data signal processing is constrained to simple tasks. Therefore, I've gone with a clap-detection that is connected to my smart LED drivers.

#### Follower Node #3: Embedded Node
**[TO BE IMPLEMENTED]**

This node will act as a hub for my peripherals. Rather than being hosted on a WiFi-enabled ODROID XU4, this node could be a STM32 or some other microcontroller. Part of my learning goals is to develop with embedded linux. If there is an opportunity that *makes sense* to interface with peripherals through my own drivers or using memory-mapped registers with the Linux environment, then I will use a SBC for this node. Otherwise, it is likely that I would get by with something like an ESP32 or STM32 (although I could also put an embedded linux distro onto the STM32). 

Peripherals for this node could include: LEDs, servos/BLDC motors, temperature sensor, humidity sensor, load cells, 

---
## Next Steps:
*As of 12.17.2025*


With the infrastructure in-place, I am looking to continue building the functionality of my follower nodes. Specifically, the Embedded Node and the Vision Node. There are so many applications of CV that I am leaving this one open until I find alignment in having a CV node be useful. Additionally, since I am planning on teaching an Embedded Software class during the Fall 2026 semester, I am planning on getting more hands-on experience with embedded programming. Therefore, I imagine I will be building out the architecture for the Embedded Node and have multiple of them.

Beyond the follower nodes' specific implementations, I am looking to develop more system-wide infrastructure. For example, I am planning on having a System Health module running on my Master Node with each of my follower nodes sending Heartbeats over the network at a fixed rate. To tie it all together, I will create a Grafana dashboard on my Master Node to visualize system health over time. Additionally, I want to ensure I have configuration management over the follower nodes by connecting them with `Ansible`. The biggest challenge I've had with this project has been the lack of static IPs. If I were to continue working on this project during the semester and bringing my setup to Olin, I would want to reserve some static IPs through IT or figuring out the DHCP range of the OLIN-DEVICES or OLIN-ROBOTICS SSIDs. In addition, the OLIN-ROBOTICS SSID doesn't reach the dorms, which doesn't align with my "dorm automation" goals.

Lastly, a completely separate thread would be pivoting even further towards DevOps and setting up my HomeLab cluster on the Master Node. My goal here would be to gain experience working with K8s, Nginx, Docker, Grafana, and Prometheus to orchestrate containers for my health monitoring, self-hosted CI/CD runners, self-hosted portfolio website, and dashboards for system health and for a user GUI mobile app through a VPN tunnel. 