#!/bin/bash

# Check if the video path is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <full_path_to_video>"
    exit 1
fi

videoPath="$1"

# Step 2: Check if the directory exists, otherwise create it
if [ -d "~/.config/videoascii/" ]; then
    echo "~/.config/videoascii/ does exist."
else
    echo "~/.config/videoascii/ does not exist, creating this folder."
    mkdir -p ~/.config/videoascii/
fi

# Step 3: Run ffmpeg to extract frames
ffmpeg -i "$videoPath" -r 1 ~/.config/videoascii/frame_%d.png

# Step 4: Loop through each frame and run txt_to_img
for frame in ~/.config/videoascii/frame_*.png; do
    ./img_to_txt/img_to_txt $frame
done
