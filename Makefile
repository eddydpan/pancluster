.PHONY: build clean audio master run-audio run-master

# Default IP for audio node
IP ?= 192.168.32.137

build:
	mkdir -p build
	cd build && cmake -DBUILD_FOLLOWER_NODES=ON .. && make -j4

clean:
	rm -rf build/*

master:
	cd build && cmake .. && make master_node -j4

audio:
	cd build && cmake -DBUILD_FOLLOWER_NODES=ON .. && make audio_node -j4

run-audio:
	./build/src/follower_nodes/audio_node/audio_node $(IP)

run-master:
	./build/src/master_node/master_node
