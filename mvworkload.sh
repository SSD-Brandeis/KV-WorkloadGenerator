#!/bin/bash

# Check if workload.txt exists in the current directory
if [ ! -f "./workload.txt" ]; then
    echo "Error: workload.txt does not exist in the current directory."
    exit 1
fi

# Prompt the user for the destination directory
printf "\nEnter the absolute path to the destination directory: "
read -r dest_dir

# Function to move workload.txt to the user-specified directory
move_file() {
    if mv ./workload.txt "$dest_dir"; then
        echo "workload.txt has been successfully moved to $dest_dir."
    else
        echo "An error occurred. Please check if the destination directory exists and you have permission to write to it."
        exit 1
    fi
}

# Check if the destination directory exists
if [ -d "$dest_dir" ]; then
    move_file
else
    echo "The specified directory does not exist. Would you like to create it? (y/n)"
    read -n 1 choice
    echo # Move to a new line
    if [[ $choice = [Yy] ]]; then
        mkdir -p "$dest_dir" && move_file
    else
        echo "Operation cancelled by the user."
        exit 1
    fi
fi
