#!/bin/bash

# Parse arguments with default
output="runs"
save=false
gt=false
while [[ $# -gt 0 ]]; do
    case $1 in
        --dataset|-d)
            dataset="$2"
            shift 2
            ;;
        --split|-s)
            split="$2"
            shift 2
            ;;
        --config|-c)
            config="$2"
            shift 2
            ;;
        --output|-o)
            output="$2"
            shift 2
            ;;
        --save)
            save=true
            shift
            ;;
        --gt)
            gt=true
            shift
            ;;
        *)
            echo "Usage: $0 --dataset|-d <dataset> --split|-s <split> --config|-c <config> [--output|-o <output>] [--save] [--gt]"
            echo "  dataset: MOT15, MOT16, MOT17, MOT20"
            echo "  split: train or test"
            echo "  config: path to config file (e.g. config/sort.toml)"
            echo "  output: path to output folder (default: runs)"
            echo "  save: enable saving of visualization"
            echo "  gt: use ground truth (only works with train split)"
            exit 1
            ;;
    esac
done

# Validate required arguments
if [ -z "$dataset" ] || [ -z "$split" ] || [ -z "$config" ]; then
    echo "Error: Missing required arguments"
    echo "Usage: $0 --dataset|-d <dataset> --split|-s <split> --config|-c <config> [--output|-o <output>] [--save] [--gt]"
    exit 1
fi

# Validate gt flag only used with train split
if [ "$gt" = true ] && [ "$split" != "train" ]; then
    echo "Error: --gt can only be used with train split"
    exit 1
fi

# Locate mot binary
if command -v mot &> /dev/null; then
    MOT_BIN="mot"
elif [ -f "./build/app/mot" ]; then
    MOT_BIN="./build/app/mot"
else
    echo "Error: mot binary not found. Compile the project first:"
    echo "meson compile -C build"
    exit 1
fi

if [ -f "venv/bin/python3" ]; then
    PYTHON_BIN="venv/bin/python3"
else
    echo "Error: python venv binary not found. Create the venv first:"
    echo "python3 -m venv venv && ./venv/bin/pip install -r requirements.txt"
    exit 1
fi

# Get all sequence directories
seq_dir="$dataset/$split"
if [ ! -d "$seq_dir" ]; then
    echo "Error: Split directory $seq_dir not found!"
    exit 1
fi

# Create experiment directory
timestamp=$(date +%Y%m%d_%H%M%S)
exp_dir="$output/exp_${timestamp}"
output_dir="$exp_dir/output"

mkdir -p "$output_dir"

# Save experiment command
cp "$config" "$exp_dir/"
{
    echo "Dataset: $dataset"
    echo "Split: $split" 
    echo "Config: $config"
    echo "GT enabled: $gt"
    echo "Save video: $save"
    echo "Command: $0 $@"
} > "$exp_dir/cli.txt"

# Run tracker on each sequence
for seq in $seq_dir/*; do
    if [ -d "$seq" ]; then
        seq=${seq%/}
        echo "Processing sequence: $seq"
        cmd="$MOT_BIN --input $seq --config $config --output $output_dir"
        [ "$save" = true ] && cmd="$cmd --save"
        [ "$gt" = true ] && cmd="$cmd --gt"
        $cmd
    fi
done

# Run evaluation only for train split
if [ "$split" = "train" ]; then
    echo "Running evaluation..."
    $PYTHON_BIN -m motmetrics.apps.eval_motchallenge "$dataset/$split" "$output_dir" 2>&1 | tee "$exp_dir/metrics.txt"
else
    echo "Skipping evaluation for test split (no ground truth available)"
fi