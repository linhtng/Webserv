#!/bin/bash
for i in {1..100}; do
    curl -s -o /dev/null http://localhost:8080/ &
done
