# Distributed Systems

Last updated: November 17, 2025 

<aside>
💡

Project planning document for a home-automation distributed systems project

</aside>

## Learning Goals

- [ ]  Gain experience working with distributed systems by building one up from scratch
- [ ]  Learn embedded topics (especially embedded linux)
- [ ]  Develop a platform for future learning while also having relevant

## System Architecture

I am using **Mosquitto (MQTT)** for my inter-process communication (IPC) protocol. It’s lightweight and there are libraries that I can patch together to build ‘drivers’ for my TP-Link Kasa smart LEDs. Another alternative is Redis, which feels more industry-standard than MQTT, but since the MQTT interface is so friendly, I can move fast and develop with it, ensuring that I keep my IPC configurable in its interface if I feel the desire to switch out the implementation. 

## Project Components

For this project, I plan on having three nodes: one leader node, and two followers. 

### Leader Node

The leader node will be hosted on my former Dell XPS laptop. I plan on having a Linux Distro like Debian Trixie running so that it’s lightweight but still easily configurable within the `apt` package manager. This leader node will receive **heartbeats** from the node and monitor the follower’s health. Additionally, it will keep track of the follower nodes’ provisioning, configuration, and maintenance through `Ansible`. It can also host all my plans I had in my [XTC Project Proposal](https://www.notion.so/XTC-Project-Proposal-29cb3684e97e8092a4a5c6ab8ea6d9fd?pvs=21)  of CI/CD self-hosted runners, health dashboard through Grafana/Prometheus, hosting my portfolio website, and acting as an endpoint for me to access an exposed API through my mobile devices. 

### Follower Nodes

Each follower node will be hosted on an ODROID-XU4

![image.png](Distributed%20Systems/image.png)

an *ARM Quad Core 2GHz A15, 2GB RAM single board computer (SBC)* akin to a Raspberry Pi 4. This SBC has more than enough processing power to handle DSP for tasks like audio input and basic CV.

Follower nodes are also responsible for publishing their health telemetry to the master node in two forms:  a *heartbeat* (every 5 seconds) and *performance metrics* (CPU, RAM, etc.) 

**Follower Node #1: CV Node**

For one of the follower nodes, I plan on having it run real-time CV (using a standard library like OpenCV) with a USB Camera peripheral. This could monitor something silly like bathroom usage throughout the day and by whom in my suite or perform person detection in my room.

**Follower Node #2: Audio Node**

This follower node will run some digital-signal processing on audio input from a USB Microphone or Microphone array connected to the ODROID’s GPIO pins. This node could be treated as a self-maintained Alexa, where I would have significantly less concern about having Amazon listening in while I’m in my own space. I could pair this with my light actuation and down-the-line, a door-closing robot (motor + wheel, mounted on a door).

**Follower Node #3: Embedded Node**

This node will act as a hub for my peripherals. Rather than being hosted on a WiFi-enabled ODROID XU4, this node could be a STM32 or some other microcontroller. Part of my learning goals is to develop with embedded linux. If there is an opportunity that *makes sense* to interface with peripherals through my own drivers or using memory-mapped registers with the Linux environment, then I will use a SBC for this node. Otherwise, it is likely that I would get by with something like an ESP32 or STM32 (although I could also put an embedded linux distro onto the STM32). 

Peripherals for this node could include: LEDs, servos/BLDC motors, temperature sensor, humidity sensor, load cells, 

---