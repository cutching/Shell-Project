# Makefile for project 3
# By Gavin Cutchin
# 4/30/2022
# CMSC 257 - 001
CC = gcc
CFLAGS = -pthread
TARGET = sh257

all: $(TARGET)

$(TARGET): csapp.c shellex.c
	$(CC) $(CFLAGS) -o $(TARGET) csapp.c shellex.c

.PHONY: clean

clean:
	$(RM) $(TARGET)