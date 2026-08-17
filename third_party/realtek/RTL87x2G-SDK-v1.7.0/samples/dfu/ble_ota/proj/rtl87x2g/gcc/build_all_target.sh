#!/bin/bash

mingw32-make TARGET=app clean
mingw32-make TARGET=app
mingw32-make TARGET=app_ns clean
mingw32-make TARGET=app_ns
