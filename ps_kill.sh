#!/bin/bash

for pid in $(ps -ef | grep "some search" | awk '{print $2}'); do kill -9 $pid; done
