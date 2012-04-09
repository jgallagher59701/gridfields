#!/usr/bin/env sh
# For off-screen rendering
/usr/bin/env Xvfb :9 -screen 0 1024x1024x24 &
PYTHONPATH=../dist DISPLAY=:9 python gfserve.py

