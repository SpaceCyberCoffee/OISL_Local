#!/bin/bash

SOURCE="/home/jstar/Desktop/github-nos3/try.txt"
DEST="/home/jstar/.nos3/42/"
INTERVAL=5  # Check every 10 seconds (you can adjust this)

while true; do
    if [ -f "$SOURCE" ]; then
        rsync -av --delete "$SOURCE" "$DEST"
        #echo "File synchronized at $(date)"
    else
        # If the source file doesn't exist, delete the destination file
        if [ -f "$DEST/try.txt" ]; then
            rm "$DEST/try.txt"
           # echo "Source file deleted. Deleting destination file at $(date)"
        #else
            #echo "Source file does not exist. Checked at $(date)"
	fi
    fi
    sleep $INTERVAL
done

