#!/bin/sh

# arguments: filename and parameter for raspistill

Date=$( date +%Y%m%d_%H%M%S )
FILE=$1$Date
PARAM=$2

raspistill -o "${FILE}_.jpg" "${PARAM}"
echo "${FILE}_.jpg";
