# Multi Object Tracking
Yet another C++ adaptation of SOTA multi-object tracking algorithm

## Supported Trackers
- [SORT](https://github.com/abewley/sort) ![Support](https://img.shields.io/badge/support-yes-brightgreen.svg)
- [BoTSORT](https://github.com/NirAharon/BoT-SORT) ![Support](https://img.shields.io/badge/support-yes-brightgreen.svg)

## Dataset
Get the MOT dataset of your choice from [MOT website](https://motchallenge.net/)
```bash
cd data
wget https://motchallenge.net/data/MOT20.zip
unzip MOT20.zip && rm MOT20.zip
```

## Configure
In `config` folder, add your custom tracker config:

<details open>
    <summary>SORT</summary>

```json
{
"tracker": {
    "name": "sort",
    "kalman": {
        "time_step": 1,
        "process_noise_scale": 1.0,
        "measurement_noise_scale": 1.0
    },
    "max_time_lost": 15,
    "match_thresh": 0.3
}
}
```
</details>

<details>
    <summary>BoTSORT</summary>

```json
{
    "tracker": {
        "name": "botsort",
        "kalman": {
            "time_step": 1,
            "process_noise_scale": 1.0,
            "measurement_noise_scale": 1.0
        },
        "max_time_lost": 15,
        "track_high_thresh": 0.5,
        "track_low_thresh": 0.1,
        "new_track_thresh": 0.6,
        "first_match_thresh": 0.3,
        "second_match_thresh": 0.1,
        "unconfirmed_match_thresh": 0.2,
        "proximity_thresh": 0.5,
        "appearance_thresh": 0.9
    },
    "reid": {
        "engine": {
            "model_path": "/path/to/model.engine",
            "batch_size": 1,
            "precision": 16
        }
    }
}
```
</details>

## Compile

```shell
# reid only supported with BoTSORT
meson setup build -Denable_reid=false 
meson compile -C build
```

## Run
```shell
cd build/app
./mot -h

# Example
./mot -i data/MOT20/train/<seq-name> -c config/sort.json --display
```

## Evaluate
```shell
chmod +x mot.sh
./mot-eval.sh --dataset data/MOT20 --split train --config app/config/sort.json --save
# experiment output available in runs folder
```

## Run with your detector

https://github.com/tensorworksio/TensorRT-Vision/tree/main/app/mot