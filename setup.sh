#! /bin/bash

set -eu

# source .env if exists
if [ -f .env ]; then
    source .env
fi

# ask ahc problem name
read -p "Enter AHC problem name: " PROBLEM_NAME

# ask language, default to cpp
read -p "Enter language (default: cpp): " LANGUAGE
LANGUAGE=${LANGUAGE:-cpp}

# ask root directory, default to ./../huristics
read -p "Enter root directory (default: ./../huristics): " ROOT_DIR
ROOT_DIR=${ROOT_DIR:-./../huristics}

# ask objective, default is max
read -p "Enter objective (default: max): " OBJECTIVE
OBJECTIVE=${OBJECTIVE:-max}

# ask is interactive problem, default is false
read -p "Is this an interactive problem? (default: no): " INTERACTIVE
INTERACTIVE=${INTERACTIVE:-no}

# check if ROOT_DIR/$PROBLEM_NAME exists
if [ -d "$ROOT_DIR/$PROBLEM_NAME" ]; then
    echo "Directory $ROOT_DIR/$PROBLEM_NAME already exists."

    # ask if remove it
    read -p "Do you want to remove $ROOT_DIR/$PROBLEM_NAME (y/n): " REMOVE_DIR
    if [ "$REMOVE_DIR" = "y" ]; then
        rm -rf "$ROOT_DIR/$PROBLEM_NAME"
        echo "Removed $ROOT_DIR/$PROBLEM_NAME"
    else
        echo "Exiting without removing."
        exit 1
    fi
fi

# create ROOT_DIR/$PROBLEM_NAME
mkdir -p "$ROOT_DIR/$PROBLEM_NAME"

# Copy all files from the current directory to ROOT_DIR/$PROBLEM_NAME, excluding .git
rsync -av --exclude='.git' --exclude='setup.sh' ./ "$ROOT_DIR/$PROBLEM_NAME"

# run pahcer init in $ROOT_DIR/$PROBLEM_NAME
cd "$ROOT_DIR/$PROBLEM_NAME"
INTERACTIVE_FLAG=""
if [ "$INTERACTIVE" = "yes" ]; then
    INTERACTIVE_FLAG="-i"
fi
pahcer init -p "$PROBLEM_NAME" -o "$OBJECTIVE" -l "$LANGUAGE" $INTERACTIVE_FLAG

# get tool zip file from atcoder web page
CONTEST_URL="https://atcoder.jp/contests/$PROBLEM_NAME/tasks/${PROBLEM_NAME}_a"
TOOL_URL=$(curl -s "$CONTEST_URL" | grep -oP '(?<=<a href=")[^"]*(?=">Local version)' | head -n 1)
if [ -z "$TOOL_URL" ]; then
    echo "Error: Unable to find tool URL. Please check the contest URL."
else
    # download tool zip file
    curl -L "$TOOL_URL" -o tool.zip

    # unzip tool.zip
    unzip -o tool.zip

    # remove tool.zip
    rm tool.zip

    # make directory tools/out, tools/err
    mkdir -p tools/out
    mkdir -p tools/err
fi

# initialize git repository
git init
git add .
git commit -m "Initial commit"
git branch -M main

# run code ROOT_DIR/$PROBLEM_NAME
if command -v code >/dev/null 2>&1; then
    code .
else
    echo "Visual Studio Code (code) is not installed or not in PATH."
fi
