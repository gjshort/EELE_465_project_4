# MCU interface GUI

## Installation 

### Python

If you're on Linux or macOS, you likely already have python installed. If you're on Windows, use the latest [Python install manager](https://www.python.org/downloads/windows/).

### Python package manager

I suggest you use [uv](https://docs.astral.sh/uv/) to manage your python environment for this GUI.

### Dependencies

[Pyserial](https://pyserial.readthedocs.io/en/latest/pyserial.html) is the only package needed that isn't the standard library.

If you're using uv, all dependencies will be installed the first time you run the program.

If you're not using uv, you can just install `pyserial` globally using `pip`:
```bash
pip install pyserial
```

## Usage

To the run GUI, you will launch it from the command line.

With `uv`:
```bash
uv run mcu_interface.py
```

If you're not using `uv`, just run directly with python:
```bash
python3 mcu_interface.py
```