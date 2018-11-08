#!/bin/bash

echo "gen flash=1 Mbytes"
touch user/user_main.c

boot=new
app=1
spi_speed=2
spi_mode=3
spi_size_map=6
make clean
make COMPILE=gcc BOOT=$boot APP=$app SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map