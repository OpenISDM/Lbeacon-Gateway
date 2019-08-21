#!/bin/bash

ps -ef | grep Gateway.out | grep -v grep | awk '{print $2}' | xargs sudo kill -SIGINT
