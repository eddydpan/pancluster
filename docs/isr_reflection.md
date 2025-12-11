# C++ Robotics Practicum ISR Reflection

Eddy Pan
10 December 2025

I came into this ISR with high goals. In short, I felt like I didn’t achieve the goals I came in with, and instead ended up pivoting to new goals as my motivations shifted over the course of the semester. I was hoping to use this ISR as a platform to keep myself accountable working on a robotics project. 

While I had some ideas of what projects I wanted to work on going into this ISR, none of them stuck out. At the start of the semester, my learning objectives were the following:

- Creating a multi-node robotics system
- Gaining familiarity working with C++
- Relearning ROS2 and getting acquainted with that development environment
- Write software for an embodied robotics project (no simulation-only project)
- Build something practical
- Familiarize myself with “operations” skills
- Use existing hardware

However, when it came time to finally decide on a project, I realized I lacked the hardware to build a multi-node robotics system. I decided on a sort of Home Automation project. I had been holding onto my high school laptop that has been just sitting around. I imagined I would be able to turn it into a home server that could communicate with another device over the network to toggle the state of my smart LED lights in my room and perform some sort of physical task such as closing the door or opening/closing my window blinds.

I ended up hitting many roadblocks with the setup of my master server node. Configuring it with Debian Trixie ended up taking a good portion of the semester as I was also figuring out the full scope of my project. I dealt with time sinks like broken WiFi drivers, creating backups of my high school files, and overall inexperience with provisioning Linux machines.

Once I came across ODROID XU4s, I reconsidered my learning goals, which at this point shifted slightly—I wanted a platform that would support embedded systems learning. Therefore, the ODROID XU4s would work perfectly. I would still be working in a multi-node system—a distributed systems project. I learned networking the hard way as I provisioned each of my ODROIDs. Most of my time I spent on this ISR was not on C++, but on provisioning instead. Since it took so much time, I decided to switch away from ROS2 as middleware and instead using MQTT for simple pub-sub in C++. 

While the computational setup for the hardware was the most time-consuming, I felt like having the space for the ISR was helpful in keeping me accountable to put in time each week, even if it felt like I wasn’t making much linear progress.

My ISR was more robotics than it was C++. Had I known this at the start of the semester, I would’ve ironed out a plan and timeline for getting a C++ project done. The biggest roadblock I faced was hardware. Since I didn’t have access to any pre-existing hardware that felt truly multi-nodal and weren’t Neatos, I feel good about the decision I took with using these ODROIDs. I think there is a lot of future upside in building a platform where I can develop in DevOps and Robotics in a way that feels both personal and practical to me. I absolutely plan on continuing this project next year. Next steps include building out my services ecosystem on my master server utilizing technologies such as Docker, Kubernetes, Terraform, Grafana, Prometheus, and Ansible. Later down the line, I’ll build out my Vision Node and my Embedded Node architecture. The goal is to have it all running in my dorm by the first half of senior year. While I wish I had a project that exposed me to more ROS2, I am happy with the direction of my project and the future upside it has. I could have seen a different path with this ISR where instead of learning about infrastructure, I would do more “robotics”-type work of working with a mobile base and implementing a SLAM algorithm like the Kalman filter. I have high hopes that configuring these ODROIDs will bring high upside for robotics projects in the future.
