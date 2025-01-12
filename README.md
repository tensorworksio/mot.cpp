# Multi Object Tracking
Yet another C++ adaptation of SOTA multi-object tracking algorithm

## Supported Trackers
- [SORT](https://github.com/abewley/sort) ![Support](https://img.shields.io/badge/support-yes-brightgreen.svg)
- [BoTSORT](https://github.com/NirAharon/BoT-SORT) ![Support](https://img.shields.io/badge/support-yes-brightgreen.svg)

## Dataset
Get the MOT dataset of your choice from [MOT website](https://motchallenge.net/)
```bash
wget https://motchallenge.net/data/MOT15.zip
unzip MOT15.zip -d data && rm MOT15.zip
```

## Configure
In `config` folder, add your custom tracker config:

<details>
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

<details open>
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
}
}
```
</details>

## Compile

```shell
meson setup build
meson compile -C build
```

## Run
```shell
cd build/app
./mot -i data/MOT15/train/<seq-name> -c config/<config-name>.json --display
```

## Evaluate
```shell
chmod +x mot.sh
./mot.sh --dataset data/MOT15 --split train --config app/config/<config-name>.json --save
```
