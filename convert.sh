#!/bin/bash

# Check if the video path is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <full_path_to_video>"
    exit 1
fi

videoPath="$1"

if [ -d "~/.videoascii/" ]; then
    echo "~/.videoascii/ does exist."
else
    echo "~/.videoascii/ does not exist, creating this folder."
    mkdir -p ~/.videoascii/
fi


fps=$(ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of default=noprint_wrappers=1:nokey=1 "$videoPath")
delay=$(echo "scale=4; 1/($fps)" | bc)


ffmpeg -i "$videoPath" -vsync 0 ~/.videoascii/frame_%04d.png

for frame in ~/.videoascii/frame_*.png; do
    ./img_to_txt/img_to_txt $frame
    sleep $delay
done

rm -rf ~/.videoascii/*
