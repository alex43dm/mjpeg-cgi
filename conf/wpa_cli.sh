#!/bin/bash

case "$2" in
    CONNECTED)
        /etc/mjpeg-cgi/reconf
        ;;
    DISCONNECTED)
        ;;
esac
